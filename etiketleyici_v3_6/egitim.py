"""
Eğitim işçisi. Sunucu (app.py) bunu AYRI bir süreç olarak başlatır; böylece eğitim çökse bile
etiketleme arayüzü çalışmaya devam eder ve iptal edilebilir.

Bir tur şu adımlardan oluşur:
  1) hazırlık : etiketli 4K kareler parçalara (tile) bölünür, YOLO veri dosyaları yazılır
  2) eğitim   : YOLO eğitilir (ilk turda COCO ağırlığından, sonraki turlarda önceki turun en iyi modelinden)
  3) test     : dondurulmuş test setinde tam görüntü üzerinde mAP, precision, recall ölçülür
  4) tahmin   : etiketsiz kareler için ön etiket ve aday kutular üretilir, kararsızlık ve titreme hesaplanır

Elle çalıştırmak gerekmez; sunucu kendisi çağırır:
    python egitim.py --is "<calisma>/turlar/tur_001/is.json"
    python egitim.py --analiz "<calisma>/analiz_is.json"   (yalnızca test tahmini: Kılavuz sayfası için)

v3.6: test tahminleri test_tahmin.json olarak saklanır (Kılavuz sayfası bunlardan karışıklık tablosu ve
PR eğrisi çizer); öneriler kaydedilmeden önce temizlenir (grup kutusu + sınıf başına NMS).
"""
import argparse
import itertools
import os
import random
import sys
import time
import traceback
from collections import deque
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np

import ortak as O


class Iptal(Exception):
    """Kullanıcı 'İptal' dedi."""


def bellek_hatasi(e):
    m = str(e).lower()
    return "out of memory" in m or "cuda error: out of memory" in m or type(e).__name__ == "OutOfMemoryError"


def hata_ozeti(e):
    m = str(e)
    ml = m.lower()
    if bellek_hatasi(e):
        return "Ekran kartı belleği yetmedi. Ayarlardan batch değerini küçült (örn. 4) ya da daha küçük model seç."
    if "not a loadable checkpoint" in ml or "unpickling" in ml or "corrupt" in ml or "truncated" in ml:
        return ("Model dosyası bozuk; büyük ihtimalle indirme yarıda kaldı. Uygulama klasöründeki .pt dosyasını silip "
                "yeniden indir (bağlantı: github.com/ultralytics/assets/releases/download/v8.4.0/yolo11n.pt). Ayrıntı: "
                + m[:160])
    if (".pt" in ml and ("no such file" in ml or "not found" in ml or "does not exist" in ml)) or "download" in ml \
            or isinstance(e, ConnectionError):
        return ("Model ağırlığı bulunamadı ya da indirilemedi. Seçtiğin ağırlık dosyasını (örn. yolo11n.pt) "
                "tarayıcıdan indirip uygulama klasörüne koy. Ayrıntı: " + m[:200])
    if "no module named" in ml:
        return "Eksik Python paketi: " + m[:200] + " (pip install ultralytics)"
    return f"{type(e).__name__}: {m[:400]}"


def yari_hassasiyet(cihaz):
    """Ekran kartında tahmini FP16 ile yap (2 kat hızlı). Ultralytics 8.4 'half' yerine 'quantize' kullanıyor;
    eski sürümlerde 'half' geçerli. Hangisi varsa o seçilir. İşlemcide FP32 kalır."""
    if cihaz == "cpu":
        return {}
    try:
        from ultralytics.cfg import DEFAULT_CFG_DICT

        if "quantize" in DEFAULT_CFG_DICT:
            return {"quantize": 16}
    except Exception:  # noqa: BLE001
        pass
    return {"half": True}


def ultralytics_ayarla():
    """Ultralytics her eğitimin başında yazı tipi (Arial.ttf) indirmeye çalışır; internet yarıda koparsa
    eğitimin tamamı hata verir. Yazı tipi sadece grafik çizimi için gerekli, bu yüzden indirme hatası yok sayılır."""
    try:
        import ultralytics.data.utils as du

        asil = du.check_font

        def guvenli(*a, **k):
            try:
                return asil(*a, **k)
            except Exception as e:  # noqa: BLE001
                print(f"[Bilgi] Yazı tipi indirilemedi, atlanıyor: {e}", flush=True)
                return None

        du.check_font = guvenli
    except Exception:  # noqa: BLE001
        pass


class Isci:
    def __init__(self, is_yolu):
        self.job = O.json_oku(is_yolu)
        if not self.job:
            raise SystemExit(f"İş dosyası okunamadı: {is_yolu}")
        j = self.job
        self.ayar = j["ayarlar"]
        self.cikti = Path(j["cikti"])
        self.durum_yolu = Path(j["durum_dosyasi"])
        self.iptal_yolu = Path(j["iptal_dosyasi"])
        self.images = Path(j["images"])
        self.labels = Path(j["labels"])
        self.siniflar = j["siniflar"]
        self.nc = len(self.siniflar)
        self.P = int(self.ayar["parca"])
        self.ov = float(self.ayar["ortusme"])
        self.son_yazma = 0.0
        self.t0 = time.time()
        self.bilgi = {"tur": j["tur"], "cihaz": None, "batch": None}
        self._parti = 0

    # -------------------------------------------------------------- durum / iptal
    def durum(self, asama, ilerleme=None, mesaj="", zorla=False, iptal_kontrol=True, **ek):
        if iptal_kontrol and self.iptal_yolu.exists():
            raise Iptal()
        simdi = time.time()
        if not zorla and simdi - self.son_yazma < 1.0:
            return
        self.son_yazma = simdi
        veri = {"tur": self.job["tur"], "asama": asama, "ilerleme": ilerleme, "mesaj": mesaj,
                "gecen_sn": round(simdi - self.t0), "zaman": simdi, **self.bilgi, **ek}
        try:
            O.json_yaz(self.durum_yolu, veri)
        except OSError:
            pass

    def log(self, *a):
        print(time.strftime("%H:%M:%S"), *a, flush=True)

    def etiket_yolu(self, ad):
        return self.labels / (Path(ad).stem + ".txt")

    # -------------------------------------------------------------- 1) hazırlık
    def parcala(self, ad, img_dir, lab_dir):
        """Bir etiketli kareyi parçalar. Nesne içeren tüm parçalar + boş parçaların bir kısmı kullanılır.
        Parça görüntüleri önbellekte saklanır; etiketleri her turda yeniden yazılır (etiket değişmiş olabilir)."""
        img_yol = self.images / ad
        W, H = O.goruntu_boyutu(img_yol)
        kutular, siniflar = O.etiket_piksel(self.etiket_yolu(ad), W, H, self.nc)
        alan = (kutular[:, 2] - kutular[:, 0]) * (kutular[:, 3] - kutular[:, 1])
        ad_kok = O.guvenli_ad(ad, img_yol.stat().st_size)
        rng = random.Random(ad_kok)
        bos_oran = float(self.ayar["bos_parca_orani"])
        secilen = []
        for (tx, ty, tw, th) in O.parcalar(W, H, self.P, self.ov):
            r = rng.random()  # her parça için çekilir: boş parça seçimi turlar arasında sabit kalsın
            satirlar = []
            if len(kutular):
                ix1 = np.maximum(kutular[:, 0], tx)
                iy1 = np.maximum(kutular[:, 1], ty)
                ix2 = np.minimum(kutular[:, 2], tx + tw)
                iy2 = np.minimum(kutular[:, 3], ty + th)
                gw, gh = ix2 - ix1, iy2 - iy1
                gorunen = np.clip(gw, 0, None) * np.clip(gh, 0, None) / np.maximum(alan, 1e-9)
                for m in np.nonzero((gw >= 2) & (gh >= 2) & (gorunen >= 0.25))[0]:
                    cx = ((ix1[m] + ix2[m]) / 2 - tx) / tw
                    cy = ((iy1[m] + iy2[m]) / 2 - ty) / th
                    satirlar.append(f"{int(siniflar[m])} {cx:.6f} {cy:.6f} {gw[m] / tw:.6f} {gh[m] / th:.6f}")
            if satirlar or r < bos_oran:
                secilen.append((tx, ty, tw, th, satirlar))

        eksik = [p for p in secilen if not (img_dir / f"{ad_kok}__{p[0]}_{p[1]}.jpg").exists()]
        if eksik:
            img = O.goruntu_oku(img_yol)
            if img is None:
                self.log(f"UYARI: okunamadı, atlandı: {ad}")
                return []
            if img.shape[1] != W or img.shape[0] != H:
                self.log(f"UYARI: boyut uyuşmazlığı, atlandı: {ad}")
                return []
            for (tx, ty, tw, th, _) in eksik:
                O.goruntu_yaz(img_dir / f"{ad_kok}__{tx}_{ty}.jpg", img[ty:ty + th, tx:tx + tw])
        yollar = []
        for (tx, ty, tw, th, satirlar) in secilen:
            ad_p = f"{ad_kok}__{tx}_{ty}"
            (lab_dir / f"{ad_p}.txt").write_text("\n".join(satirlar) + ("\n" if satirlar else ""), encoding="utf-8")
            yollar.append(str(img_dir / f"{ad_p}.jpg"))
        return yollar

    def veri_hazirla(self):
        kok = Path(self.job["parca_kok"])
        img_dir, lab_dir = kok / "images", kok / "labels"
        img_dir.mkdir(parents=True, exist_ok=True)
        lab_dir.mkdir(parents=True, exist_ok=True)
        for f in kok.rglob("*.cache"):  # Ultralytics etiket önbelleği: etiketler değişmiş olabilir, sil
            f.unlink(missing_ok=True)
        adlar = [("egitim", a) for a in self.job["egitim"]] + [("dogrulama", a) for a in self.job["dogrulama"]]
        listeler = {"egitim": [], "dogrulama": []}
        for i, (bolum, ad) in enumerate(adlar):
            self.durum("hazirlik", i / max(1, len(adlar)), f"Kareler parçalanıyor ({i + 1}/{len(adlar)})")
            listeler[bolum] += self.parcala(ad, img_dir, lab_dir)
        if not listeler["egitim"]:
            raise RuntimeError("Eğitim için hiç parça oluşmadı. Etiketli karelerde kutu var mı?")
        if not listeler["dogrulama"]:  # çok az veri: doğrulamayı eğitimden ödünç al
            listeler["dogrulama"] = listeler["egitim"][: max(1, len(listeler["egitim"]) // 10)]
        for bolum, yollar in listeler.items():
            (self.cikti / f"{bolum}.txt").write_text("\n".join(yollar) + "\n", encoding="utf-8")
        veri = {"path": str(kok), "train": str(self.cikti / "egitim.txt"), "val": str(self.cikti / "dogrulama.txt"),
                "names": self.siniflar}
        yaml_yol = self.cikti / "veri.yaml"
        O.json_yaz(yaml_yol, veri)  # JSON geçerli bir YAML'dır
        self.bilgi.update(egitim_parca=len(listeler["egitim"]), dogrulama_parca=len(listeler["dogrulama"]))
        self.log(f"Parçalar: eğitim {len(listeler['egitim'])}, doğrulama {len(listeler['dogrulama'])}")
        return yaml_yol

    # -------------------------------------------------------------- 2) eğitim
    def _cb_epoch_basi(self, tr):
        self._parti = 0

    def _cb_parti(self, tr):
        self._parti += 1
        nb = max(1, len(tr.train_loader))
        ep = (tr.epoch + min(self._parti, nb) / nb) / max(1, tr.epochs)
        self.durum("egitim", ep, f"Epoch {tr.epoch + 1}/{tr.epochs}", epoch=tr.epoch + 1, epochs=tr.epochs)

    def _cb_epoch_sonu(self, tr):
        m = getattr(tr, "metrics", None) or {}
        v = m.get("metrics/mAP50(B)")
        if v is not None:
            self.bilgi["val_map50"] = round(float(v), 4)
        self.durum("egitim", (tr.epoch + 1) / max(1, tr.epochs), f"Epoch {tr.epoch + 1}/{tr.epochs} bitti",
                   zorla=True, epoch=tr.epoch + 1, epochs=tr.epochs)

    def egit(self, yaml_yol):
        import torch
        from ultralytics import YOLO

        a = self.ayar
        E = int(self.job["epochs"])
        batch = int(a["batch"])
        if self.cihaz == "cpu" and batch == -1:
            batch = 8
        deneme = 0
        while True:
            try:
                self.bilgi["batch"] = "otomatik" if batch == -1 else batch
                model = YOLO(self.job["baslangic_agirlik"])
                model.add_callback("on_train_epoch_start", self._cb_epoch_basi)
                model.add_callback("on_train_batch_end", self._cb_parti)
                model.add_callback("on_fit_epoch_end", self._cb_epoch_sonu)
                model.train(
                    data=str(yaml_yol), epochs=E, patience=int(a["patience"]), imgsz=self.P, batch=batch,
                    workers=int(a["workers"]), device=self.cihaz, project=str(self.cikti), name=f"egitim{deneme}",
                    exist_ok=True, plots=False, verbose=False, seed=0, deterministic=False,
                    flipud=float(a["dikey_cevirme"]), close_mosaic=max(0, min(10, E // 2)),
                )
                break
            except Iptal:
                raise
            except Exception as e:  # noqa: BLE001
                if bellek_hatasi(e) and deneme < 3 and batch != 1:
                    deneme += 1
                    batch = 8 if batch == -1 else max(1, batch // 2)
                    self.log(f"Bellek yetmedi, batch={batch} ile yeniden deneniyor.")
                    if torch.cuda.is_available():
                        torch.cuda.empty_cache()
                    continue
                raise
        tr = model.trainer
        kayit = Path(tr.save_dir) / "weights"
        en_iyi = kayit / "best.pt" if (kayit / "best.pt").exists() else kayit / "last.pt"
        if not en_iyi.exists():
            raise RuntimeError("Eğitim bitti ama ağırlık dosyası bulunamadı.")
        self.bilgi["epoch_yapilan"] = int(tr.epoch) + 1
        del model
        if torch.cuda.is_available():
            torch.cuda.empty_cache()
        return en_iyi

    # -------------------------------------------------------------- 3-4) parçalı tahmin
    def goruntuler(self, adlar):
        """Kareleri arka planda önceden okur (4K jpeg çözmek yavaş; ekran kartı beklemesin)."""
        with ThreadPoolExecutor(2) as ex:
            it = iter(adlar)
            kuyruk = deque((a, ex.submit(O.goruntu_oku, self.images / a)) for a in itertools.islice(it, 3))
            while kuyruk:
                ad, f = kuyruk.popleft()
                sonraki = next(it, None)
                if sonraki is not None:
                    kuyruk.append((sonraki, ex.submit(O.goruntu_oku, self.images / sonraki)))
                img = f.result()
                if img is None:
                    self.log(f"UYARI: okunamadı, atlandı: {ad}")
                    continue
                yield ad, img

    def parcali_tahmin(self, model, img, conf, max_det):
        H, W = img.shape[:2]
        tiles = O.parcalar(W, H, self.P, self.ov)
        kesitler = [np.ascontiguousarray(img[ty:ty + th, tx:tx + tw]) for (tx, ty, tw, th) in tiles]
        B, S, C, T, K = [], [], [], [], []
        k = 0
        while k < len(kesitler):
            try:
                sonuc = model.predict(kesitler[k:k + self.tahmin_batch], imgsz=self.P, conf=conf, iou=0.7,
                                      max_det=max_det, device=self.cihaz, verbose=False, **self.hassasiyet)
            except Exception as e:  # noqa: BLE001
                if bellek_hatasi(e) and self.tahmin_batch > 1:
                    import torch

                    self.tahmin_batch = max(1, self.tahmin_batch // 2)
                    torch.cuda.empty_cache()
                    continue
                raise
            for j, r in enumerate(sonuc):
                tx, ty, tw, th = tiles[k + j]
                if r.boxes is None or len(r.boxes) == 0:
                    continue
                xyxy = r.boxes.xyxy.cpu().numpy().astype(np.float64)
                kenar = (((tx > 0) & (xyxy[:, 0] <= 2)) | ((tx + tw < W) & (xyxy[:, 2] >= tw - 2))
                         | ((ty > 0) & (xyxy[:, 1] <= 2)) | ((ty + th < H) & (xyxy[:, 3] >= th - 2)))
                xyxy[:, [0, 2]] += tx
                xyxy[:, [1, 3]] += ty
                B.append(xyxy)
                S.append(r.boxes.conf.cpu().numpy().astype(np.float64))
                C.append(r.boxes.cls.cpu().numpy().astype(np.int64))
                T.append(np.full(len(xyxy), k + j))
                K.append(kenar)
            k += self.tahmin_batch
        if not B:
            return np.zeros((0, 4)), np.zeros(0), np.zeros(0, dtype=np.int64)
        return O.parca_birlestir(np.concatenate(B), np.concatenate(S), np.concatenate(C),
                                 np.concatenate(T), np.concatenate(K))

    def temizle(self, b, s, c):
        """Önerileri temizler: grup kutusu + sınıf başına NMS (Ayarlar: oneri_nms, grup_filtre)."""
        return O.oneri_temizle(b, s, c, nms_iou=float(self.ayar.get("oneri_nms", 0.5)),
                               grup=bool(self.ayar.get("grup_filtre", True)))

    def test_tahmin_et(self, model):
        """Test karelerini tahmin eder; ham tahminleri test_tahmin.json'a yazar (Kılavuz sayfası bunu kullanır)."""
        test = self.job["test"]
        tahmin, gercek, kayit = {}, {}, {}
        for i, (ad, img) in enumerate(self.goruntuler(test)):
            self.durum("test", i / len(test), f"Test setinde ölçülüyor ({i + 1}/{len(test)})")
            H, W = img.shape[:2]
            gercek[ad] = O.etiket_piksel(self.etiket_yolu(ad), W, H, self.nc)
            b, sk, c = self.parcali_tahmin(model, img, conf=0.01, max_det=100)
            tahmin[ad] = (b, sk, c)
            kayit[ad] = {"w": W, "h": H, "k": [[round(float(x1), 1), round(float(y1), 1), round(float(x2), 1),
                                                round(float(y2), 1), int(cc), round(float(ss), 4)]
                                               for (x1, y1, x2, y2), ss, cc in zip(b, sk, c)]}
        O.json_yaz(self.cikti / "test_tahmin.json", {"tur": self.job["tur"], "kareler": kayit})
        return tahmin, gercek

    def test_degerlendir(self, model):
        test = self.job["test"]
        if not test:
            self.log("Test setinde etiketli kare yok, ölçüm atlandı.")
            return None
        tahmin, gercek = self.test_tahmin_et(model)
        conf = float(self.ayar["conf_on_etiket"])
        m = O.degerlendir(tahmin, gercek, self.nc, conf_pr=conf)
        if m:
            m["test_kare"] = len(gercek)
            self.log(f"Test: mAP50={m['map50']:.4f} mAP50-95={m['map5095']:.4f} P={m['p']:.3f} R={m['r']:.3f}")
            # Öğrenme eğrisi ham tahminle ölçülür (eski turlarla karşılaştırılabilsin); temizlenmiş önerilerin
            # (kullanıcının gördüğü kutuların) P/R değeri ayrıca kaydedilir.
            mt = O.degerlendir({a: self.temizle(*v) for a, v in tahmin.items()}, gercek, self.nc, conf_pr=conf)
            if mt:
                m["temiz"] = {"p": mt["p"], "r": mt["r"], "map50": mt["map50"],
                              "sinif": {k: {"p": v["p"], "r": v["r"]} for k, v in mt["sinif"].items()}}
                self.log(f"Temizlenmiş öneriler: P={mt['p']:.3f} R={mt['r']:.3f}")
        return m

    def tahmin_et(self, model):
        a = self.ayar
        adlar = self.job["tahmin"]
        kat = self.cikti / "tahmin" / "kareler"
        kat.mkdir(parents=True, exist_ok=True)
        conf_on = float(a["conf_on_etiket"])
        varlik, kareler = {}, {}
        for i, (ad, img) in enumerate(self.goruntuler(adlar)):
            self.durum("tahmin", i / max(1, len(adlar)), f"Ön etiketler hazırlanıyor ({i + 1}/{len(adlar)})")
            H, W = img.shape[:2]
            b, s, c = self.parcali_tahmin(model, img, conf=float(a["conf_aday"]), max_det=100)
            b, s, c = self.temizle(b, s, c)  # v3.6: grup kutusu ve kopya kutular atılır
            b, s, c = b[:500], s[:500], c[:500]  # birleştirme skor sırasıyla döner
            kutular = [[round(float(x1)), round(float(y1)), round(float(x2)), round(float(y2)), int(cc), round(float(ss), 3)]
                       for (x1, y1, x2, y2), ss, cc in zip(b, s, c)]
            O.json_yaz(kat / (Path(ad).stem + ".json"), {"w": W, "h": H, "k": kutular, "temiz": True})
            on = s >= conf_on
            varlik[ad] = (b[on], c[on])
            kareler[ad] = {"u": round(O.kararsizlik(s), 4), "t": 0.0, "n": int(on.sum())}
        if a.get("titreme", True):
            sira = self.job["tum_sira"]
            konum = {x: i for i, x in enumerate(sira)}
            etiketli = set(self.job["etiketli_havuz"])
            gerekli = {sira[j] for ad in kareler for j in (konum[ad] - 1, konum[ad] + 1)
                       if 0 <= j < len(sira) and sira[j] in etiketli}
            for ad in gerekli:
                try:
                    W, H = O.goruntu_boyutu(self.images / ad)
                except OSError:
                    continue
                varlik[ad] = O.etiket_piksel(self.etiket_yolu(ad), W, H, self.nc)
            for ad, t in O.titreme_skorlari(sira, varlik, list(kareler)).items():
                kareler[ad]["t"] = round(t, 4)
        O.json_yaz(self.cikti / "tahmin" / "ozet.json", {"tur": self.job["tur"], "kareler": kareler})
        return len(kareler)

    # -------------------------------------------------------------- ana akış
    def cihaz_hazirla(self):
        import torch

        istek = str(self.ayar.get("cihaz") or "").strip()
        self.cihaz = istek if istek else ("0" if torch.cuda.is_available() else "cpu")
        self.bilgi["cihaz"] = (torch.cuda.get_device_name(0) if self.cihaz != "cpu" and torch.cuda.is_available()
                               else "işlemci (CPU)")
        self.tahmin_batch = 16 if self.cihaz != "cpu" else 4
        self.hassasiyet = yari_hassasiyet(self.cihaz)
        ultralytics_ayarla()

    def analiz(self):
        """Yalnızca test tahmini: mevcut bir modelle test karelerini tahmin edip test_tahmin.json yazar."""
        from ultralytics import YOLO

        self.cihaz_hazirla()
        self.log(f"Analiz | cihaz: {self.bilgi['cihaz']} | model: {self.job['agirlik']} | test: {len(self.job['test'])} kare")
        self.durum("test", 0, "Model yükleniyor", zorla=True)
        model = YOLO(self.job["agirlik"])
        self.test_tahmin_et(model)
        return {"test_kare": len(self.job["test"])}

    def calis(self):
        self.cihaz_hazirla()
        self.log(f"Tur {self.job['tur']} | cihaz: {self.bilgi['cihaz']} | başlangıç: {self.job['baslangic_agirlik']}"
                 f" | epoch: {self.job['epochs']} | eğitim kare: {len(self.job['egitim'])}"
                 f" | doğrulama: {len(self.job['dogrulama'])} | test: {len(self.job['test'])}")
        self.durum("hazirlik", 0, "Hazırlanıyor", zorla=True)
        yaml_yol = self.veri_hazirla()
        self.durum("egitim", 0, "Eğitim başlıyor", zorla=True)
        en_iyi = self.egit(yaml_yol)
        self.log(f"Eğitim bitti: {en_iyi}")

        from ultralytics import YOLO

        model = YOLO(str(en_iyi))
        metrik = self.test_degerlendir(model)
        n = self.tahmin_et(model)
        self.log(f"{n} kare için ön etiket hazır.")
        return {"agirlik": str(en_iyi), "metrik": metrik, "tahmin_kare": n}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--is", dest="is_yolu")
    ap.add_argument("--analiz", dest="analiz_yolu")
    args = ap.parse_args()
    if not args.is_yolu and not args.analiz_yolu:
        ap.error("--is ya da --analiz gerekli")
    isci = Isci(args.is_yolu or args.analiz_yolu)
    sonuc = {}
    try:
        sonuc = isci.analiz() if args.analiz_yolu else isci.calis()
        sonuc["durum"] = "tamam"
    except Iptal:
        sonuc = {"durum": "iptal", "mesaj": "Kullanıcı iptal etti."}
        isci.log("İptal edildi.")
    except BaseException as e:  # noqa: BLE001
        traceback.print_exc()
        sonuc = {"durum": "hata", "mesaj": hata_ozeti(e)}
    sonuc.update(isci.bilgi)
    sonuc["sure_sn"] = round(time.time() - isci.t0)
    O.json_yaz(isci.cikti / ("analiz_sonuc.json" if args.analiz_yolu else "sonuc.json"), sonuc)
    isci.durum("bitti", 1, sonuc.get("mesaj", "Tamamlandı"), zorla=True, iptal_kontrol=False)
    sys.exit(0 if sonuc["durum"] == "tamam" else 1)


if __name__ == "__main__":
    main()
