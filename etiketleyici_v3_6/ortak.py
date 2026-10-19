"""
Ortak yardımcılar: hem sunucu (app.py) hem eğitim işçisi (egitim.py) kullanır.
Burada torch / ultralytics YOKTUR; sunucu hızlı açılsın diye sadece numpy kullanılır.

Terimler:
  parça (tile)  : 4K görüntüden kesilen küçük kare (örn. 640x640). Küçük nesneler kaybolmasın diye
                  model görüntünün tamamını küçültmek yerine parçalarla eğitilir ve parçalarla tahmin eder.
  IoU           : iki kutunun kesişim alanı / birleşim alanı (0 = hiç örtüşmüyor, 1 = aynı kutu).
  IoS           : kesişim alanı / küçük kutunun alanı (küçük kutu büyüğün içindeyse 1 olur).
"""
import csv
import io
import json
import math
import os
import re
import threading
import time
import zlib
from pathlib import Path

import numpy as np

IMG_EXTS = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}


# ------------------------------------------------------------------ genel
def dogal_anahtar(s):
    """'kare_2' < 'kare_10' olacak şekilde sıralama anahtarı (sayıları sayı gibi karşılaştırır)."""
    return [int(t) if t.isdigit() else t.lower() for t in re.split(r"(\d+)", str(s))]


def dogal_sirala(ogeler):
    return sorted(ogeler, key=dogal_anahtar)


def json_oku(yol, varsayilan=None):
    try:
        return json.loads(Path(yol).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return varsayilan


def metin_yaz(yol, metin):
    """Önce geçici dosyaya yazar, sonra yerine koyar. Yazarken çökme olursa eski dosya bozulmaz.
    Windows'ta hedef dosya o an başka süreçte açıksa kısa süre bekleyip tekrar dener."""
    yol = Path(yol)
    # v3.7: geçici ad iş parçacığına da özgü; aynı dosyayı iki iş parçacığı (kaydetme + M yenileme) aynı anda yazarsa
    # ikisi aynı .tmp'yi paylaşıp birinin os.replace'i FileNotFoundError ile düşmesin
    gecici = yol.with_name(yol.name + f".{os.getpid()}.{threading.get_ident()}.tmp")
    gecici.write_text(metin, encoding="utf-8")
    for deneme in range(40):
        try:
            os.replace(gecici, yol)
            return
        except PermissionError:
            time.sleep(0.05)
    os.replace(gecici, yol)


def json_yaz(yol, veri):
    metin_yaz(yol, json.dumps(veri, ensure_ascii=False, indent=1))


def csv_metni(baslik, satirlar, ondalik=None):
    """Türkçe Excel'in doğrudan açtığı CSV: noktalı virgül ayraç, ondalık virgül, UTF-8 BOM. None boş hücre olur.
    ondalik verilirse ondalıklı sayılar o kadar basamakla yazılır."""
    buf = io.StringIO()
    w = csv.writer(buf, delimiter=";")
    w.writerow(baslik)
    for satir in satirlar:
        w.writerow(["" if v is None else ((f"{v:.{ondalik}f}" if ondalik is not None else str(v)).replace(".", ",")
                                          if isinstance(v, float) else v) for v in satir])
    return "\ufeff" + buf.getvalue()


def tarih_metni(zaman):
    return time.strftime("%d.%m.%Y %H:%M:%S", time.localtime(zaman))


def guvenli_ad(ad, ek=""):
    """Dosya adını sadece ASCII karakterli, çakışmasız bir ada çevirir (parça dosyaları için)."""
    sade = re.sub(r"[^A-Za-z0-9_-]+", "_", Path(ad).stem)[:60]
    return f"{sade}_{zlib.crc32((str(ad) + '|' + str(ek)).encode('utf-8')):08x}"


def kume_payi(ad):
    """Ada göre sabit, 0-1 arası sayı. Doğrulama kümesi seçiminde kullanılır: aynı kare her turda
    aynı kümede kalır, böylece önceki modelin gördüğü kare doğrulamaya kaçmaz."""
    return (zlib.crc32(str(ad).encode("utf-8")) % 100000) / 100000.0


# ------------------------------------------------------------------ görüntü
def goruntu_boyutu(yol):
    """(genişlik, yükseklik). Tarayıcı ve OpenCV gibi EXIF döndürmesini hesaba katar."""
    from PIL import Image

    with Image.open(yol) as im:
        w, h = im.size
        try:
            yon = im.getexif().get(0x0112, 1)
        except Exception:  # noqa: BLE001
            yon = 1
    return (h, w) if yon in (5, 6, 7, 8) else (w, h)


def goruntu_oku(yol):
    """Türkçe karakterli yollarda da çalışır. BGR numpy dizisi ya da None döner."""
    import cv2

    try:
        veri = np.fromfile(str(yol), dtype=np.uint8)
    except OSError:
        return None
    if veri.size == 0:
        return None
    return cv2.imdecode(veri, cv2.IMREAD_COLOR)


def goruntu_yaz(yol, img, kalite=95):
    import cv2

    ok, buf = cv2.imencode(".jpg", img, [cv2.IMWRITE_JPEG_QUALITY, kalite])
    if not ok:
        raise IOError(f"Görüntü yazılamadı: {yol}")
    gecici = Path(str(yol) + ".tmp")
    buf.tofile(str(gecici))
    os.replace(gecici, yol)


# ------------------------------------------------------------------ etiket
def etiket_oku(yol):
    """YOLO etiket dosyası -> [(sınıf, cx, cy, w, h), ...]. Bozuk satırlar atlanır."""
    try:
        metin = Path(yol).read_text(encoding="utf-8")
    except OSError:
        return []
    sonuc = []
    for satir in metin.splitlines():
        p = satir.split()
        if len(p) != 5:
            continue
        try:
            c = int(float(p[0]))
            cx, cy, w, h = (float(v) for v in p[1:])
        except ValueError:
            continue
        if c < 0 or w <= 0 or h <= 0:
            continue
        sonuc.append((c, cx, cy, w, h))
    return sonuc


def etiket_piksel(yol, W, H, nc=None):
    """Etiketleri piksel koordinatına çevirir: (kutular[n,4] x1y1x2y2, siniflar[n])."""
    kutular, siniflar = [], []
    for c, cx, cy, w, h in etiket_oku(yol):
        if nc is not None and c >= nc:
            continue
        x1, y1 = max(0.0, (cx - w / 2) * W), max(0.0, (cy - h / 2) * H)
        x2, y2 = min(float(W), (cx + w / 2) * W), min(float(H), (cy + h / 2) * H)
        if x2 - x1 >= 1 and y2 - y1 >= 1:
            kutular.append((x1, y1, x2, y2))
            siniflar.append(c)
    return np.array(kutular, dtype=np.float64).reshape(-1, 4), np.array(siniflar, dtype=np.int64)


# ------------------------------------------------------------------ Pascal VOC (LabelImg .xml)
_TR = str.maketrans("çğıöşüÇĞİÖŞÜâîû", "cgiosuCGIOSUaiu")
ES_ANLAMLI = {
    "insan": {"insan", "insanlar", "kisi", "yaya", "person", "people", "pedestrian", "human"},
    "tasit": {"tasit", "tasitlar", "arac", "araba", "otomobil", "car", "vehicle", "truck", "bus", "van", "motor",
              "motorcycle", "motosiklet", "bicycle", "bisiklet", "otobus", "kamyon"},
}


def sinif_anahtar(ad):
    """'Taşıt', 'TASIT ', 'taşıt' -> 'tasit' (Türkçe harf ve büyük/küçük farkı yok sayılır)."""
    return re.sub(r"[^a-z0-9]+", "", str(ad).translate(_TR).lower())


def sinif_bul(ad, anahtarlar):
    """XML'deki sınıf adını classes.txt'deki sıra numarasına çevirir; bulamazsa None."""
    k = sinif_anahtar(ad)
    if k in anahtarlar:
        return anahtarlar[k]
    for hedef, eslar in ES_ANLAMLI.items():
        if k in eslar and hedef in anahtarlar:
            return anahtarlar[hedef]
    return None


def voc_oku(yol):
    """LabelImg'in Pascal VOC .xml dosyası -> (genişlik, yükseklik, [(ad, x1, y1, x2, y2), ...])."""
    import xml.etree.ElementTree as ET

    kok = ET.parse(str(yol)).getroot()
    W = H = 0
    boyut = kok.find("size")
    if boyut is not None:
        try:
            W = int(float(boyut.findtext("width") or 0))
            H = int(float(boyut.findtext("height") or 0))
        except ValueError:
            W = H = 0
    nesneler = []
    for o in kok.iter("object"):
        ad = (o.findtext("name") or "").strip()
        bb = o.find("bndbox")
        if not ad or bb is None:
            continue
        try:
            x1, y1, x2, y2 = (float(bb.findtext(k)) for k in ("xmin", "ymin", "xmax", "ymax"))
        except (TypeError, ValueError):
            continue
        nesneler.append((ad, min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2)))
    return W, H, nesneler


def labelme_oku(yol):
    """LabelMe .json -> (genişlik, yükseklik, [(ad, x1, y1, x2, y2), ...]). Dikdörtgen ve çokgenlerin dış kutusu alınır."""
    veri = json.loads(Path(yol).read_text(encoding="utf-8"))
    if not isinstance(veri, dict) or "shapes" not in veri:
        raise ValueError("LabelMe biçimi değil")
    W, H = int(veri.get("imageWidth") or 0), int(veri.get("imageHeight") or 0)
    nesneler = []
    for sh in veri.get("shapes", []):
        ad, pts = str(sh.get("label", "")).strip(), sh.get("points") or []
        if not ad or len(pts) < 2:
            continue
        xs, ys = [float(p[0]) for p in pts], [float(p[1]) for p in pts]
        nesneler.append((ad, min(xs), min(ys), max(xs), max(ys)))
    return W, H, nesneler


# ------------------------------------------------------------------ parçalama
def parca_konumlari(uzunluk, boyut, ortusme):
    """Bir eksen boyunca parça başlangıçları. Parçalar eşit aralıklı dağıtılır, son parça kenara dayanır.
    Gerçek örtüşme istenen örtüşmeden az olmaz."""
    if uzunluk <= boyut:
        return [0]
    adim = max(1, int(round(boyut * (1 - ortusme))))
    n = math.ceil((uzunluk - boyut) / adim) + 1
    return sorted({int(round(i * (uzunluk - boyut) / (n - 1))) for i in range(n)})


def parcalar(W, H, boyut, ortusme):
    """[(x, y, w, h), ...] parça listesi."""
    pw, ph = min(boyut, W), min(boyut, H)
    return [(x, y, pw, ph) for y in parca_konumlari(H, boyut, ortusme) for x in parca_konumlari(W, boyut, ortusme)]


# ------------------------------------------------------------------ kutu geometrisi
def iou_matrisi(a, b):
    a = np.asarray(a, dtype=np.float64).reshape(-1, 4)
    b = np.asarray(b, dtype=np.float64).reshape(-1, 4)
    if len(a) == 0 or len(b) == 0:
        return np.zeros((len(a), len(b)))
    x1 = np.maximum(a[:, None, 0], b[None, :, 0])
    y1 = np.maximum(a[:, None, 1], b[None, :, 1])
    x2 = np.minimum(a[:, None, 2], b[None, :, 2])
    y2 = np.minimum(a[:, None, 3], b[None, :, 3])
    kes = np.clip(x2 - x1, 0, None) * np.clip(y2 - y1, 0, None)
    aa = (a[:, 2] - a[:, 0]) * (a[:, 3] - a[:, 1])
    ab = (b[:, 2] - b[:, 0]) * (b[:, 3] - b[:, 1])
    return kes / np.maximum(aa[:, None] + ab[None, :] - kes, 1e-9)


def parca_birlestir(kutular, skorlar, siniflar, parca_no, kenarda, iou_esik=0.5, ios_tek=0.85, ios_cift=0.5, hiza=0.8):
    """Parçalardan gelen tahminleri tek görüntüde birleştirir.

    Örtüşen parçalar aynı nesneyi iki kez bulabilir; parça kenarında kesilen nesne de iki yarım kutu olarak
    gelebilir. Kural: FARKLI parçalardan gelen, aynı sınıftaki iki kutu
      - IoU >= iou_esik ise aynı nesnedir: yüksek skorlu kutu kalır,
      - parça kenarında kesilmiş kutular aynı nesnenin parçaları olabilir; birleşim kutusu alınır. v3.6'da temkinli:
          * yalnızca biri kenardaysa: kesik parça diğerinin neredeyse tamamen içinde olmalı (IoS >= ios_tek, 0,85),
          * ikisi de kenardaysa (büyük nesne iki parçaya bölünmüş): IoS >= ios_cift (0,5),
          * ve iki kutu en az bir eksende hizalı olmalı (o eksendeki örtüşme / kısa kenar >= hiza, 0,8).
        Böylece kesik bir insan, yanındaki BAŞKA bir insanla tek kutuda birleşmez (grup kutusu oluşmaz).
    Aynı parçanın kendi kutularına dokunulmaz (onları model zaten kendi içinde ayıkladı).
    Her parçadan en fazla bir kutu birleştirilir, böylece yan yana iki insan tek kutuya dönmez.
    """
    kutular = np.asarray(kutular, dtype=np.float64).reshape(-1, 4)
    n = len(kutular)
    if n == 0:
        return kutular, np.zeros(0), np.zeros(0, dtype=np.int64)
    sira = np.argsort(-np.asarray(skorlar), kind="stable")
    b = kutular[sira].copy()
    s = np.asarray(skorlar, dtype=np.float64)[sira]
    c = np.asarray(siniflar, dtype=np.int64)[sira]
    t = np.asarray(parca_no, dtype=np.int64)[sira]
    k = np.asarray(kenarda, dtype=bool)[sira]
    alan = np.clip(b[:, 2] - b[:, 0], 0, None) * np.clip(b[:, 3] - b[:, 1], 0, None)
    canli = np.ones(n, dtype=bool)
    ob, os_, oc = [], [], []
    for i in range(n):
        if not canli[i]:
            continue
        canli[i] = False
        bi = b[i].copy()
        aday = np.nonzero(canli & (c == c[i]) & (t != t[i]))[0]
        if len(aday):
            ix1 = np.maximum(b[i, 0], b[aday, 0])
            iy1 = np.maximum(b[i, 1], b[aday, 1])
            ix2 = np.minimum(b[i, 2], b[aday, 2])
            iy2 = np.minimum(b[i, 3], b[aday, 3])
            kes = np.clip(ix2 - ix1, 0, None) * np.clip(iy2 - iy1, 0, None)
            iou = kes / np.maximum(alan[i] + alan[aday] - kes, 1e-9)
            ios = kes / np.maximum(np.minimum(alan[i], alan[aday]), 1e-9)
            ox, oy = np.clip(ix2 - ix1, 0, None), np.clip(iy2 - iy1, 0, None)
            gx = np.minimum(b[i, 2] - b[i, 0], b[aday, 2] - b[aday, 0])
            gy = np.minimum(b[i, 3] - b[i, 1], b[aday, 3] - b[aday, 1])
            hizali = np.maximum(ox / np.maximum(gx, 1e-9), oy / np.maximum(gy, 1e-9)) >= hiza
            iki_kenar, tek_kenar = k[i] & k[aday], k[i] ^ k[aday]
            parcali = hizali & ((iki_kenar & (ios >= ios_cift)) | (tek_kenar & (ios >= ios_tek)))
            esles = (iou >= iou_esik) | parcali
            if esles.any():
                sec = aday[esles]
                anahtar = (iou + ios)[esles]
                gorulen = set()
                for j in sec[np.argsort(-anahtar, kind="stable")]:
                    if t[j] in gorulen:
                        continue
                    gorulen.add(int(t[j]))
                    canli[j] = False
                    if k[i] or k[j]:
                        bi[0], bi[1] = min(bi[0], b[j, 0]), min(bi[1], b[j, 1])
                        bi[2], bi[3] = max(bi[2], b[j, 2]), max(bi[3], b[j, 3])
        ob.append(bi)
        os_.append(s[i])
        oc.append(c[i])
    return np.array(ob).reshape(-1, 4), np.array(os_), np.array(oc, dtype=np.int64)


# ------------------------------------------------------------------ ölçümler
def _ap(recall, precision):
    """Ultralytics ile aynı yöntem: precision zarfı + 101 noktalı interpolasyon + yamuk kuralı."""
    mrec = np.concatenate(([0.0], recall, [1.0]))
    mpre = np.concatenate(([1.0], precision, [0.0]))
    mpre = np.flip(np.maximum.accumulate(np.flip(mpre)))
    x = np.linspace(0, 1, 101)
    y = np.interp(x, mrec, mpre)
    return float(np.sum((x[1:] - x[:-1]) * (y[1:] + y[:-1]) / 2))


def sinif_eslesmeleri(tahmin, gercek, nc):
    """Ölçümün ağır kısmı: her sınıf için tahminleri skora göre sıralar ve 10 IoU eşiğinde (0,50-0,95)
    hangilerinin doğru olduğunu (TP) bulur. Güven eşiğinden bağımsızdır; bir kez hesaplanıp saklanabilir.
    Döner: {sınıf: (gerçek nesne sayısı, skorlar [büyükten küçüğe], tp [n, 10])}"""
    esikler = np.linspace(0.5, 0.95, 10)
    bos = (np.zeros((0, 4)), np.zeros(0), np.zeros(0, dtype=np.int64))
    sonuc = {}
    for kk in range(nc):
        n_gt = int(sum(int((np.asarray(g[1]) == kk).sum()) for g in gercek.values()))
        if n_gt == 0:
            continue
        skor_l, tp_l = [], []
        for ad, (gb, gc) in gercek.items():
            pb, ps, pc = tahmin.get(ad, bos)
            m = np.asarray(pc) == kk
            pbk, psk = np.asarray(pb).reshape(-1, 4)[m], np.asarray(ps)[m]
            if len(pbk) == 0:
                continue
            o = np.argsort(-psk, kind="stable")
            pbk, psk = pbk[o], psk[o]
            gbk = np.asarray(gb).reshape(-1, 4)[np.asarray(gc) == kk]
            tp = np.zeros((len(pbk), len(esikler)), dtype=bool)
            if len(gbk):
                iou = iou_matrisi(pbk, gbk)
                for ti, esik in enumerate(esikler):
                    kullanildi = np.zeros(len(gbk), dtype=bool)
                    for pi in np.nonzero(iou.max(axis=1) >= esik)[0]:  # hiçbir nesneyle örtüşmeyen tahmin FP kalır
                        aday = np.nonzero(~kullanildi & (iou[pi] >= esik))[0]
                        if len(aday):
                            j = aday[np.argmax(iou[pi, aday])]
                            kullanildi[j] = True
                            tp[pi, ti] = True
            skor_l.append(psk)
            tp_l.append(tp)
        if skor_l:
            skor = np.concatenate(skor_l)
            tp = np.concatenate(tp_l)
            o = np.argsort(-skor, kind="stable")
            sonuc[kk] = (n_gt, skor[o], tp[o])
        else:
            sonuc[kk] = (n_gt, np.zeros(0), np.zeros((0, len(esikler)), dtype=bool))
    return sonuc


def olcutler(es, conf_pr=0.25, ayrinti=False):
    """sinif_eslesmeleri() sonucundan mAP50, mAP50-95, precision ve recall (ve istenirse ayrıntı)."""
    sinif = {}
    for kk, (n_gt, skor, tp) in es.items():
        ap = np.zeros(tp.shape[1])
        p_k = r_k = 0.0
        ek = {}
        if len(skor):
            tpc = np.cumsum(tp, axis=0)
            fpc = np.cumsum(~tp, axis=0)
            rec = tpc / n_gt
            pre = tpc / np.maximum(tpc + fpc, 1)
            for ti in range(tp.shape[1]):
                ap[ti] = _ap(rec[:, ti], pre[:, ti])
            sec = skor >= conf_pr
            if sec.any():
                d = int(tp[sec, 0].sum())
                p_k, r_k = d / int(sec.sum()), d / n_gt
            if ayrinti:
                ek = _ayrinti(skor, tp[:, 0], rec[:, 0], pre[:, 0], n_gt, conf_pr)
        elif ayrinti:
            ek = _ayrinti(np.zeros(0), np.zeros(0, dtype=bool), np.zeros(0), np.zeros(0), n_gt, conf_pr)
        sinif[kk] = {"ap50": float(ap[0]), "ap5095": float(ap.mean()), "p": p_k, "r": r_k, "nesne": n_gt, **ek}
    if not sinif:
        return None
    v = list(sinif.values())
    return {
        "map50": float(np.mean([x["ap50"] for x in v])),
        "map5095": float(np.mean([x["ap5095"] for x in v])),
        "p": float(np.mean([x["p"] for x in v])),
        "r": float(np.mean([x["r"] for x in v])),
        "sinif": {str(kk): x for kk, x in sinif.items()},
    }


def degerlendir(tahmin, gercek, nc, conf_pr=0.25, ayrinti=False):
    """Tam görüntü üzerinde mAP@0.5, mAP@0.5:0.95, precision ve recall.

    tahmin: {ad: (kutular[n,4], skorlar[n], siniflar[n])}
    gercek: {ad: (kutular[m,4], siniflar[m])}
    Precision/recall, güven >= conf_pr olan kutularla ve IoU 0.5 ile hesaplanır
    (yani "ön etiket olarak gösterilen kutuların ne kadarı doğru, nesnelerin ne kadarı bulundu").
    ayrinti=True ise sınıf başına TP/FP/FN, F1, precision-recall eğrisi (IoU 0,5; 101 nokta), eşik eğrisi
    (0,05-0,95 güven eşiklerinde P, R, F1) ve tüm sınıflar için karışıklık tablosu da döner (Kılavuz sayfası için).
    """
    sonuc = olcutler(sinif_eslesmeleri(tahmin, gercek, nc), conf_pr, ayrinti)
    if sonuc and ayrinti:
        sonuc["karisiklik"] = karisiklik(tahmin, gercek, nc, conf=conf_pr)
        sonuc["conf"] = conf_pr
    return sonuc


ESIKLER = [round(x, 2) for x in np.arange(0.05, 0.951, 0.05)]


def _ayrinti(skor, tp50, rec50, pre50, n_gt, conf_pr):
    """Bir sınıf için: seçili eşikte TP/FP/FN ve F1, PR eğrisi (101 nokta) ve eşik eğrisi.
    skor büyükten küçüğe sıralı; tp50 = IoU 0,5'te doğru mu."""
    def say(esik):
        n = int(np.searchsorted(-skor, -esik, side="right"))  # skor >= esik olanların sayısı
        d = int(tp50[:n].sum()) if n else 0
        p = d / n if n else 0.0
        r = d / n_gt if n_gt else 0.0
        return d, n - d, p, r, (2 * p * r / (p + r) if p + r > 0 else 0.0)
    tp_, fp_, p_, r_, f1_ = say(conf_pr)
    egri = {"t": ESIKLER, "p": [], "r": [], "f1": [], "tp": [], "fp": []}
    for e in ESIKLER:
        d, y, p, r, f1 = say(e)
        egri["p"].append(round(p, 4)); egri["r"].append(round(r, 4)); egri["f1"].append(round(f1, 4))
        egri["tp"].append(d); egri["fp"].append(y)
    en_iyi = int(np.argmax(egri["f1"])) if egri["f1"] else 0
    if len(rec50):
        mrec = np.concatenate(([0.0], rec50, [1.0]))
        mpre = np.flip(np.maximum.accumulate(np.flip(np.concatenate(([1.0], pre50, [0.0])))))
        pr = np.interp(np.linspace(0, 1, 101), mrec, mpre)
        # Ulaşılamayan recall değerleri (modelin hiç bulamadığı kısım) 0 precision sayılır, eğri orada düşer.
        pr[np.linspace(0, 1, 101) > float(rec50.max()) + 1e-9] = 0.0
    else:
        pr = np.zeros(101)
    return {"tp": tp_, "fp": fp_, "fn": n_gt - tp_, "f1": f1_, "pr": [round(float(x), 4) for x in pr],
            "esik_egri": egri, "en_iyi_esik": ESIKLER[en_iyi], "en_iyi_f1": egri["f1"][en_iyi] if egri["f1"] else 0.0}


def karisiklik(tahmin, gercek, nc, conf=0.25, iou_esik=0.5):
    """Karışıklık tablosu: satır = gerçek sınıf (son satır: arka plan, yani orada nesne yok),
    sütun = tahmin edilen sınıf (son sütun: kaçırıldı, yani hiçbir kutu konmadı).
    Eşleştirme sınıftan bağımsızdır (IoU >= 0,5); aynı sınıftan eşleşme varsa o tercih edilir."""
    M = np.zeros((nc + 1, nc + 1), dtype=np.int64)
    bos = (np.zeros((0, 4)), np.zeros(0), np.zeros(0, dtype=np.int64))
    for ad, (gb, gc) in gercek.items():
        gb, gc = np.asarray(gb, dtype=np.float64).reshape(-1, 4), np.asarray(gc, dtype=np.int64)
        g_m = gc < nc
        gb, gc = gb[g_m], gc[g_m]
        pb, ps, pc = tahmin.get(ad, bos)
        pb, ps, pc = np.asarray(pb, dtype=np.float64).reshape(-1, 4), np.asarray(ps), np.asarray(pc, dtype=np.int64)
        m = (ps >= conf) & (pc < nc)
        pb, pc = pb[m], pc[m]
        kg, kp = np.zeros(len(gb), dtype=bool), np.zeros(len(pb), dtype=bool)
        if len(gb) and len(pb):
            iou = iou_matrisi(gb, pb)
            gi, pj = np.nonzero(iou >= iou_esik)
            sira = sorted(range(len(gi)), key=lambda q: (gc[gi[q]] != pc[pj[q]], -iou[gi[q], pj[q]]))
            for q in sira:
                i, j = gi[q], pj[q]
                if not kg[i] and not kp[j]:
                    kg[i] = kp[j] = True
                    M[gc[i], pc[j]] += 1
        for i in np.nonzero(~kg)[0]:
            M[gc[i], nc] += 1
        for j in np.nonzero(~kp)[0]:
            M[nc, pc[j]] += 1
    return M.tolist()


# ------------------------------------------------------------------ öneri temizleme (v3.6)
def nms(kutular, skorlar, siniflar, iou_esik=0.5):
    """Sınıf başına klasik NMS: aynı sınıftan, IoU'su eşiği aşan daha düşük skorlu kutular silinir.
    Saklanan kutuların sıra numaralarını skor sırasıyla döner."""
    b = np.asarray(kutular, dtype=np.float64).reshape(-1, 4)
    s = np.asarray(skorlar, dtype=np.float64)
    c = np.asarray(siniflar, dtype=np.int64)
    tut = []
    for k in np.unique(c):
        idx = np.nonzero(c == k)[0]
        idx = idx[np.argsort(-s[idx], kind="stable")]
        M = iou_matrisi(b[idx], b[idx])
        canli = np.ones(len(idx), dtype=bool)
        for a in range(len(idx)):
            if not canli[a]:
                continue
            tut.append(int(idx[a]))
            canli[a + 1:] &= M[a, a + 1:] < iou_esik
    return sorted(tut, key=lambda i: (-s[i], i))


def grup_kutulari(kutular, skorlar, siniflar, ic_skor=0.25, ios_esik=0.8, alan_orani=0.7, ayrik_iou=0.3, kaplama=0.5):
    """"Grup kutusu" bulur: aynı sınıftan en az iki AYRI kutuyu neredeyse tamamen içine alan büyük kutu
    (örn. yan yana iki insanı tek kutuya alan öneri). Silinecekler için True dizisi döner.
    Koşullar: içteki kutuların her biri %80'den fazla büyük kutunun içinde, alanı büyük kutunun %70'inden küçük,
    güveni en az max(ic_skor, büyük kutu güveninin yarısı); içtekiler birbiriyle az örtüşüyor (IoU < 0,3) ve
    birlikte büyük kutunun en az yarısını kaplıyor."""
    b = np.asarray(kutular, dtype=np.float64).reshape(-1, 4)
    s = np.asarray(skorlar, dtype=np.float64)
    c = np.asarray(siniflar, dtype=np.int64)
    n = len(b)
    sil = np.zeros(n, dtype=bool)
    if n < 3:
        return sil
    alan = np.clip(b[:, 2] - b[:, 0], 0, None) * np.clip(b[:, 3] - b[:, 1], 0, None)
    for i in range(n):
        ayni = (c == c[i]) & (np.arange(n) != i)
        if ayni.sum() < 2 or alan[i] <= 0:
            continue
        kes = (np.clip(np.minimum(b[i, 2], b[:, 2]) - np.maximum(b[i, 0], b[:, 0]), 0, None)
               * np.clip(np.minimum(b[i, 3], b[:, 3]) - np.maximum(b[i, 1], b[:, 1]), 0, None))
        ios = kes / np.maximum(alan, 1e-9)
        aday = np.nonzero(ayni & (ios >= ios_esik) & (alan <= alan_orani * alan[i]) & (s >= max(ic_skor, 0.5 * s[i])))[0]
        if len(aday) < 2:
            continue
        aday = aday[np.argsort(-s[aday], kind="stable")]
        secilen = []
        for j in aday:
            if not secilen or iou_matrisi(b[j], b[secilen]).max() < ayrik_iou:
                secilen.append(j)
        if len(secilen) >= 2 and kes[secilen].sum() / alan[i] >= kaplama:
            sil[i] = True
    return sil


def oneri_temizle(kutular, skorlar, siniflar, nms_iou=0.5, grup=True, ic_skor=0.25):
    """Model önerilerini temizler: önce grup kutularını atar, sonra sınıf başına NMS uygular (kopya kutular).
    Sıra önemli: NMS önce yapılsaydı grup kutusu içteki gerçek kutuları silebilirdi. Skor sırasıyla döner."""
    b = np.asarray(kutular, dtype=np.float64).reshape(-1, 4)
    s = np.asarray(skorlar, dtype=np.float64)
    c = np.asarray(siniflar, dtype=np.int64)
    if len(b) == 0:
        return b, s, c
    if grup:
        m = ~grup_kutulari(b, s, c, ic_skor=ic_skor)
        b, s, c = b[m], s[m], c[m]
    tut = nms(b, s, c, nms_iou) if nms_iou and nms_iou < 1 else sorted(range(len(b)), key=lambda i: -s[i])
    return b[tut], s[tut], c[tut]


def kutu_listesi_temizle(k, nms_iou=0.5, grup=True):
    """[[x1, y1, x2, y2, sınıf, güven], ...] listesine oneri_temizle uygular (sunucudaki eski tahminler için)."""
    if not k:
        return []
    a = np.asarray(k, dtype=np.float64)
    b, s, c = oneri_temizle(a[:, :4], a[:, 5], a[:, 4].astype(np.int64), nms_iou=nms_iou, grup=grup)
    return [[round(float(x1)), round(float(y1)), round(float(x2)), round(float(y2)), int(cc), round(float(ss), 3)]
            for (x1, y1, x2, y2), ss, cc in zip(b, s, c)]


def kararsizlik(skorlar, en_cok=3):
    """Görüntü kararsızlığı: her kutu için u = 1 - |2p - 1| (p=0.5 iken 1 = en kararsız);
    görüntünün değeri en kararsız 3 kutunun ortalaması. Kutu yoksa 0."""
    s = np.asarray(skorlar, dtype=np.float64)
    if s.size == 0:
        return 0.0
    u = 1.0 - np.abs(2.0 * s - 1.0)
    return float(np.sort(u)[::-1][:en_cok].mean())


def _eslesir(X, Y, esik=0.5):
    """X'teki her kutu için: Y'de aynı sınıftan IoU >= esik bir kutu var mı?"""
    xb, xc = X
    yb, yc = Y
    if len(xb) == 0:
        return np.zeros(0, dtype=bool)
    if len(yb) == 0:
        return np.zeros(len(xb), dtype=bool)
    iou = iou_matrisi(xb, yb)
    ayni = np.asarray(xc)[:, None] == np.asarray(yc)[None, :]
    return ((iou >= esik) & ayni).any(axis=1)


def titreme_skorlari(sira, varlik, hedefler):
    """Ardışık karelerde tutarsızlık (titreme).

    sira    : tüm karelerin zaman sırası
    varlik  : {ad: (kutular, siniflar)} -> bilinen kareler (tahmin ya da insan etiketi)
    hedefler: skoru hesaplanacak kareler
    Bir kare için olaylar:
      var-yok-var : önceki ve sonraki karede eşleşen nesne bu karede yok  (muhtemel kaçırma)
      yok-var-yok : bu karedeki nesne ne öncekinde ne sonrakinde var       (muhtemel yanlış alarm)
    t = olay sayısı / (bu karedeki nesne + var-yok-var). İki komşusu bilinmeyen karede t = 0.
    """
    konum = {ad: i for i, ad in enumerate(sira)}
    sonuc = {}
    for ad in hedefler:
        i = konum.get(ad)
        sonuc[ad] = 0.0
        if i is None or i == 0 or i == len(sira) - 1:
            continue
        a, b = sira[i - 1], sira[i + 1]
        if ad not in varlik or a not in varlik or b not in varlik:
            continue
        A, C, B = varlik[a], varlik[ad], varlik[b]
        vyv = int((_eslesir(A, B) & ~_eslesir(A, C)).sum())
        yvy = int((~_eslesir(C, A) & ~_eslesir(C, B)).sum())
        payda = len(C[0]) + vyv
        sonuc[ad] = (vyv + yvy) / payda if payda else 0.0
    return sonuc
