"""
Kullanıcı davranışı kaydı (v3.7, Faz 1 altyapısı). Yalnızca numpy ve standart kütüphane kullanır.

Her kaydetmede davranis.jsonl dosyasına bir satır yazılır. Satır, karenin bu "ziyaretinin" (kare açılışından başka kareye
geçişe kadar) başından o ana kadarki TOPLAMI taşır; aynı ziyarette ara kayıt (Ctrl+S) olursa yalnızca sonuncusu sayılır
(ziyaret_sonlari). Satırda iki tür bilgi vardır:
  islem : istemcinin saydığı işlemler (eklenen, model adayından eklenen, silinen, taşınan, boyutu değişen,
          sınıfı değişen kutu, geri alma) ve karede geçen etkin süre. "Kullanıcı ne kadar uğraştı?"
  fark  : karenin AÇILIŞTAKİ kutuları ile KAYITTAKİ kutularının karşılaştırması. "Sonuçta ne değişti?"
          Geri almaya (Ctrl+Z) ve aynı kutuyu iki kez düzeltmeye karşı dayanıklıdır; sonraki fazlardaki öğrenme
          bunu kullanır. Açılıştaki her kutu için sonuç: onay / duzelt / sinif / sil; kayıtta olup açılışta
          olmayan her kutu "yeni"dir (modelin kaçırdığı nesne).

Neden: sonraki fazlarda öncelik puanı, sınıfa göre eşik ayarı ve zor örnek madenciliği "model hangi sınıfta,
hangi güvende yanılıyor ve kullanıcı bunu nasıl düzeltiyor?" sorusunun cevabına dayanır. Bu, Confident Learning'in
(Northcutt ve ark., JAIR 2021) "gözlenen etiket ile modelin tahmini arasındaki ortak dağılım" fikrinin, insan
düzeltmesinden beslenen hafif bir hâlidir.

Eşleştirme: kutular sınıftan bağımsız ve IoU'ya göre açgözlü (en yüksek IoU önce) eşleştirilir. Eşik 0,5,
PASCAL VOC (Everingham ve ark., IJCV 2010) ve mAP50 ile aynıdır. Sınıftan bağımsız eşleştirme sayesinde
"kutu yerinde kaldı ama sınıfı değişti" ayrı bir olay olarak görülür.
"""
import json
import os
import threading
import time
from pathlib import Path

import numpy as np

import ortak as O

KAYIT_SURUMU = 1
ESLESME_IOU = 0.5        # açılıştaki ve kayıttaki kutu bu IoU'dan fazla örtüşürse "aynı nesne" sayılır (VOC / mAP50)
DEGISMEDI_IOU = 0.95     # eşleşen kutu bundan fazla örtüşüyorsa geometrisi değişmemiş sayılır (≈ kenarlarda %2-3 kayma)
EN_COK_KUTU = 2000       # istemciden gelen kutu listesi için üst sınır (bozuk / kötü niyetli istek diske taşmasın)
EN_COK_SURE_SN = 6 * 3600
METIN_SINIRI = 40        # oturum, liste, kaynak gibi kısa metin alanlarının en çok uzunluğu
YUVARLAMA = 4            # koordinat ve IoU kaç basamak saklansın (4K'da 0,0001 ≈ 0,4 piksel)

# İstemcinin saydığı işlemler. Eski istemciler (v3.6) "duzeltilen" gönderir; o, taşınan + boyut toplamıdır.
ISLEMLER = ("eklenen", "aday_eklenen", "silinen", "tasinan", "boyut", "sinif_degisen", "geri_al")
SONUCLAR = ("onay", "duzelt", "sinif", "sil")
MODEL_KAYNAKLARI = ("oneri", "model")   # açılıştaki kutular modelden geliyorsa (öneri ya da M etiketi)

# Sınıf özeti için yumuşatma: küçük örneklemde aşırı uyum olmasın diye her sınıfın oranı, tüm sınıfların ortak
# oranına doğru çekilir (Bayesçi toplamsal yumuşatma; Beta önseli, empirical Bayes). ONSEL_GUCU kadar "sanal örnek"
# eklenir. Aynı fikir annotator modellemede Dirichlet önseli olarak geçer (Kim ve Ghahramani, "Bayesian Classifier
# Combination", AISTATS 2012). ONSEL_GUCU ve MIN_ORNEK değerleri literatürden değil, benim seçimimdir.
ONSEL_GUCU = 5.0
MIN_ORNEK = 20           # bir sınıfın oranı bu kadar model kutusundan önce "güvenilir" sayılmaz


# ------------------------------------------------------------------ kutu dönüşümü ve doğrulama
def _sayi(v, varsayilan=0.0):
    try:
        v = float(v)
    except (TypeError, ValueError):
        return varsayilan
    return v if v == v else varsayilan   # NaN gelirse varsayılan


def _tamsayi(v, alt=0, ust=10 ** 6):
    return int(min(max(_sayi(v), alt), ust))


def _metin(v):
    return str(v)[:METIN_SINIRI] if v is not None else None


def kutu_listesi(ham):
    """YOLO biçimindeki ({cls, cx, cy, w, h, m?}, 0-1 arası) kutuları [x1, y1, x2, y2] dizisine, sınıf ve güven
    listelerine çevirir. Bozuk ya da boyutsuz kutular atlanır."""
    xyxy, sinif, guven = [], [], []
    for b in (ham if isinstance(ham, list) else [])[:EN_COK_KUTU]:
        if not isinstance(b, dict):
            continue
        cx, cy, w, h = (min(max(_sayi(b.get(k)), 0.0), 1.0) for k in ("cx", "cy", "w", "h"))
        if w <= 0 or h <= 0:
            continue
        xyxy.append([cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2])
        sinif.append(_tamsayi(b.get("cls")))
        m = b.get("m")
        guven.append(round(min(max(_sayi(m), 0.0), 1.0), YUVARLAMA) if m is not None else None)
    return np.asarray(xyxy, dtype=np.float64).reshape(-1, 4), sinif, guven


def _eslestir(iou, esik):
    """Açgözlü IoU eşleştirmesi: en yüksek IoU'lu çift önce eşleşir, her kutu en çok bir kez. {ilk_i: son_j}."""
    eslesme, kullanilan = {}, set()
    if iou.size == 0:
        return eslesme
    for i, j in zip(*np.unravel_index(np.argsort(-iou, axis=None), iou.shape)):
        if iou[i, j] < esik:
            break
        if i in eslesme or j in kullanilan:
            continue
        eslesme[int(i)] = int(j)
        kullanilan.add(int(j))
    return eslesme


def kutu_farki(ilk, son, eslesme_iou=ESLESME_IOU, degismedi_iou=DEGISMEDI_IOU):
    """Açılıştaki (ilk) ve kayıttaki (son) kutuları karşılaştırır. İkisi de YOLO biçiminde kutu listesidir.

    Dönen sözlük: sonuç sayıları (onay, duzelt, sinif, sil, yeni), açılıştaki her kutunun sonucu ("kutular") ve
    yeni eklenen kutular ("yeniler"). Kutu kayıtlarında c = sınıf, m = model güveni (varsa), w / h = boyut (0-1)."""
    a, ca, ma = kutu_listesi(ilk)
    b, cb, _ = kutu_listesi(son)
    iou = O.iou_matrisi(a, b)
    eslesme = _eslestir(iou, eslesme_iou)
    sayi = dict.fromkeys(SONUCLAR, 0)
    kutular = []
    for i in range(len(a)):
        k = {"c": ca[i], "m": ma[i], "w": round(float(a[i, 2] - a[i, 0]), YUVARLAMA),
             "h": round(float(a[i, 3] - a[i, 1]), YUVARLAMA)}
        j = eslesme.get(i)
        if j is None:
            k["sonuc"] = "sil"
        else:
            k["iou"] = round(float(iou[i, j]), YUVARLAMA)
            if cb[j] != ca[i]:
                k["sonuc"], k["yeni_c"] = "sinif", cb[j]
            else:
                k["sonuc"] = "onay" if iou[i, j] >= degismedi_iou else "duzelt"
        sayi[k["sonuc"]] += 1
        kutular.append(k)
    eslesen = set(eslesme.values())
    yeniler = [{"c": cb[j], "w": round(float(b[j, 2] - b[j, 0]), YUVARLAMA), "h": round(float(b[j, 3] - b[j, 1]), YUVARLAMA)}
               for j in range(len(b)) if j not in eslesen]
    return {**sayi, "yeni": len(yeniler), "kutular": kutular, "yeniler": yeniler}


# ------------------------------------------------------------------ kayıt
def kayit_olustur(ad, kullanici, istemci, son_kutular, zaman=None):
    """Bir kaydetme isteğinden davranış kaydı oluşturur. istemci: isteğin "davranis" alanı (sözlük)."""
    istemci = istemci if isinstance(istemci, dict) else {}
    islem = istemci.get("islem") if isinstance(istemci.get("islem"), dict) else {}
    tur = istemci.get("tahmin_tur")
    return {
        "surum": KAYIT_SURUMU, "ad": str(ad), "zaman": round(zaman if zaman is not None else time.time(), 1),
        "kullanici": _metin(kullanici), "oturum": _metin(istemci.get("oturum")), "ziyaret": _metin(istemci.get("ziyaret")),
        "liste": _metin(istemci.get("liste")),
        "kaynak": _metin(istemci.get("kaynak")), "rol": _metin(istemci.get("rol")),
        "tahmin_tur": _tamsayi(tur) if tur is not None else None,
        "sure_sn": round(min(max(_sayi(istemci.get("sure_sn")), 0.0), EN_COK_SURE_SN), 1),
        "islem": {k: _tamsayi(islem.get(k)) for k in ISLEMLER},
        "fark": kutu_farki(istemci.get("ilk"), son_kutular),
    }


class Gunluk:
    """davranis.jsonl'e ekleme yapan ve okuyan nesne. Sunucu çok iş parçacıklıdır (threaded=True); iki kaydetme
    aynı anda gelirse satırlar karışmasın diye ekleme kilitle yapılır. Okuma, dosya değişmedikçe önbellekten döner."""

    def __init__(self, yol):
        self.yol = Path(yol)
        self._kilit = threading.Lock()
        self._onbellek = (None, [])

    def ekle(self, kayit):
        satir = (json.dumps(kayit, ensure_ascii=False, separators=(",", ":")) + "\n").encode("utf-8")
        with self._kilit, open(self.yol, "a+b") as f:
            # Önceki yazma yarıda kaldıysa (ör. elektrik kesildi) yeni kayıt o yarım satıra yapışıp kaybolmasın
            f.seek(0, os.SEEK_END)
            if f.tell() > 0:
                f.seek(-1, os.SEEK_END)
                if f.read(1) != b"\n":
                    satir = b"\n" + satir
            f.write(satir)

    def kayitlar(self):
        try:
            st = self.yol.stat()
        except OSError:
            return []
        anahtar = (st.st_mtime_ns, st.st_size)
        if self._onbellek[0] != anahtar:
            kayit = []
            with self._kilit:
                try:   # errors="replace": yarıda kesilmiş çok baytlı karakter okumayı çökertmesin, o satır atlanır
                    satirlar = self.yol.read_text(encoding="utf-8", errors="replace").splitlines()
                except OSError:
                    satirlar = []
            for s in satirlar:
                try:
                    k = json.loads(s)
                except ValueError:
                    continue   # yarım kalmış satır (ör. elektrik kesildi): atla
                if isinstance(k, dict):
                    kayit.append(k)
            self._onbellek = (anahtar, kayit)
        return self._onbellek[1]


# ------------------------------------------------------------------ özet ve dışa aktarma
def ziyaret_sonlari(kayitlar):
    """Her ziyaretin yalnızca son kaydı (kayıtlar ziyaret başından toplam taşır). Ziyaret alanı olmayan kayıt tektir."""
    son = {}
    for i, k in enumerate(kayitlar):
        son[k.get("ziyaret") or ("#", i)] = i
    return [kayitlar[i] for i in sorted(son.values())]


def sinif_ozeti(kayitlar, onsel_gucu=ONSEL_GUCU, min_ornek=MIN_ORNEK):
    """Sınıf başına: model kutularından kaçı onaylandı / düzeltildi / sınıfı değişti / silindi ve kullanıcının
    kaç yeni (modelin kaçırdığı) kutu eklediği. Yalnızca açılışta model kutusu olan kareler (öneri ya da M) sayılır;
    elle etiketlenen karede çizilen kutu "model kaçırdı" değildir. Oranlar küçük örneklemde tüm sınıfların ortak
    oranına doğru yumuşatılır; "guvenilir" alanı en az min_ornek model kutusu görülüp görülmediğini söyler."""
    sinif = {}
    bos = lambda: {"model_kutu": 0, **dict.fromkeys(SONUCLAR, 0), "yeni": 0}   # noqa: E731
    for k in ziyaret_sonlari(kayitlar):
        if k.get("kaynak") not in MODEL_KAYNAKLARI:
            continue
        fark = k.get("fark") or {}
        for kutu in fark.get("kutular", []):
            s = sinif.setdefault(kutu.get("c"), bos())
            s["model_kutu"] += 1
            if kutu.get("sonuc") in SONUCLAR:
                s[kutu["sonuc"]] += 1
        for kutu in fark.get("yeniler", []):
            sinif.setdefault(kutu.get("c"), bos())["yeni"] += 1
    toplam = sum(s["model_kutu"] for s in sinif.values())
    onsel = {r: (sum(s[r] for s in sinif.values()) / toplam if toplam else 0.0) for r in SONUCLAR}
    for s in sinif.values():
        n = s["model_kutu"]
        s["oran"] = {r: round((s[r] + onsel_gucu * onsel[r]) / (n + onsel_gucu), 3) for r in SONUCLAR}
        s["guvenilir"] = n >= min_ornek
    return {str(c): v for c, v in sorted(sinif.items(), key=lambda x: (x[0] is None, x[0] or 0))}


CSV_ALANLARI = ("ad", "zaman", "kullanici", "oturum", "ziyaret", "liste", "kaynak", "rol", "tahmin_tur", "sure_sn") \
    + tuple(f"islem_{k}" for k in ISLEMLER) + tuple(f"fark_{k}" for k in SONUCLAR + ("yeni",))


def csv_satirlari(kayitlar):
    """CSV_ALANLARI sırasıyla düz satırlar (ortak.csv_metni için)."""
    for k in kayitlar:
        duz = {**k, **{f"islem_{a}": (k.get("islem") or {}).get(a) for a in ISLEMLER},
               **{f"fark_{a}": (k.get("fark") or {}).get(a) for a in SONUCLAR + ("yeni",)}}
        if isinstance(duz.get("zaman"), (int, float)):
            duz["zaman"] = O.tarih_metni(duz["zaman"])
        yield [duz.get(a) for a in CSV_ALANLARI]
