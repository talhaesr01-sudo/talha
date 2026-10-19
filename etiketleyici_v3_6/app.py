"""
Etiketleyici - sürüm 3.6: model destekli etiketleme + aktif öğrenme

Akış:
  1) Dondurulmuş test seti (son %10 kare) elle etiketlenir. Eğitime hiç girmez, sadece ölçüm içindir.
  2) Tohum kareler (seyrek seçilmiş ~100 kare) elle etiketlenir.
  3) Eşik dolunca model arka planda eğitilir (ayrı süreç; arayüz donmaz).
  4) Model etiketsiz karelere ön etiket koyar; en kararsız kareler önce gösterilir. Kullanıcı düzeltir.
  5) Her "tur büyüklüğü" kadar yeni karede model yeniden eğitilir; test setinde mAP ölçülür, eğri çizilir.
  6) Durma kuralı: art arda N turda mAP artışı eşikten küçükse otomatik eğitim durur.

Çalıştırma:
    python app.py --images "C:/veri/images"
Etiketler (.txt) varsayılan olarak resimlerle AYNI klasörde durur (kare_1.jpg + kare_1.txt).
Tarayıcı: http://127.0.0.1:5000
"""
import argparse
import csv
import io
import json
import math
import os
import random
import re
import subprocess
import sys
import threading
import time
import traceback
from pathlib import Path

import numpy as np

from flask import Flask, Response, abort, jsonify, request, send_file, send_from_directory

import ortak as O

BASE = Path(__file__).resolve().parent
app = Flask(__name__, static_folder=str(BASE / "static"))
CFG = {}
KILIT = threading.RLock()
DURUM = {}
ROL = {"test": set(), "tampon": set()}
IS = {"proc": None, "tur": None, "log": None, "baslangic": 0.0, "iptal_zamani": None}
ANALIZ = {"proc": None, "tur": None, "log": None, "baslangic": 0.0, "hata": None}   # Kılavuz için test tahmini işi
_ETK = {"zaman": 0.0, "kume": frozenset(), "surum": 0}   # etiket dosyası önbelleği (v3.6: donma düzeltmesi)
_MEO = {"anahtar": None, "sonuc": []}                    # model_etiketlenebilir önbelleği
_EMEK = {"anahtar": None, "sonuc": None}                  # emek özeti önbelleği
_ANALIZ_ONB = {}                                           # /api/analiz sonuç önbelleği
_ONB = {"zaman": 0.0, "adlar": []}
MIN_ETIKETLI = 10

MODELLER = ["yolo11n.pt", "yolo11s.pt", "yolo11m.pt", "yolov8n.pt", "yolov8s.pt", "yolov8m.pt"]
# ad: (tür, varsayılan, [min, max] ya da seçenekler)
AYAR_TANIM = {
    "oto_egitim": ("bool", True),
    "ilk_egitim_esigi": ("int", 100, 10, 1000000),
    "tur_buyuklugu": ("int", 50, 5, 1000000),
    "model": ("secim", "yolo11n.pt", None),
    "ilk_epoch": ("int", 100, 1, 3000),
    "tur_epoch": ("int", 30, 1, 3000),
    "patience": ("int", 10, 0, 3000),
    "batch": ("int", -1, -1, 512),
    "parca": ("secim", 640, [320, 480, 512, 640, 800, 960, 1024, 1280]),
    "ortusme": ("float", 0.2, 0.0, 0.5),
    "bos_parca_orani": ("float", 0.15, 0.0, 1.0),
    "dogrulama_orani": ("float", 0.15, 0.05, 0.5),
    "dikey_cevirme": ("float", 0.5, 0.0, 1.0),
    "workers": ("int", 4, 0, 32),
    "cihaz": ("str", ""),
    "conf_on_etiket": ("float", 0.25, 0.01, 0.99),
    "conf_aday": ("float", 0.05, 0.001, 0.99),
    "secim": ("secim", "aktif", ["aktif", "rastgele"]),
    "agirlik_u": ("float", 0.7, 0.0, 1.0),
    "agirlik_t": ("float", 0.3, 0.0, 1.0),
    "titreme": ("bool", True),
    "denetim_orani": ("float", 0.05, 0.0, 0.5),
    "plato_tur": ("int", 3, 1, 100),
    "plato_esik": ("float", 0.01, 0.0, 0.5),
    "test_orani": ("float", 0.10, 0.0, 0.5),
    "test_tampon": ("int", 10, 0, 1000000),
    "oneri_nms": ("float", 0.5, 0.3, 0.95),
    "grup_filtre": ("bool", True),
}
AYAR_ADI = {
    "oto_egitim": "Otomatik eğitim", "ilk_egitim_esigi": "İlk eğitim eşiği", "tur_buyuklugu": "Tur büyüklüğü",
    "model": "Model", "ilk_epoch": "İlk eğitim epoch", "tur_epoch": "Tur epoch", "patience": "Erken durdurma sabrı",
    "batch": "Batch", "parca": "Parça boyutu", "ortusme": "Örtüşme", "bos_parca_orani": "Boş parça oranı",
    "dogrulama_orani": "Doğrulama oranı", "dikey_cevirme": "Dikey çevirme", "workers": "Workers", "cihaz": "Cihaz",
    "conf_on_etiket": "Ön etiket güven eşiği", "conf_aday": "Aday güven eşiği", "secim": "Seçim yöntemi",
    "agirlik_u": "Kararsızlık ağırlığı", "agirlik_t": "Titreme ağırlığı", "titreme": "Titreme",
    "denetim_orani": "Denetim oranı", "plato_tur": "Durma kuralı tur sayısı", "plato_esik": "Durma kuralı eşiği",
    "test_orani": "Test oranı", "test_tampon": "Test tamponu",
    "oneri_nms": "Kopya kutu sınırı (NMS)", "grup_filtre": "Grup kutusu filtresi",
}


# ================================================================ yardımcılar
def calisma(*p):
    return CFG["calisma"].joinpath(*p)


def goruntu_adlari():
    simdi = time.time()
    if simdi - _ONB["zaman"] > 3:
        try:
            adlar = [e.name for e in os.scandir(CFG["images"])
                     if os.path.splitext(e.name)[1].lower() in O.IMG_EXTS and e.is_file()]
        except OSError:
            adlar = []
        _ONB["adlar"] = O.dogal_sirala(adlar)
        _ONB["zaman"] = simdi
    return _ONB["adlar"]


def find_image(name):
    p = CFG["images"] / name
    if p.parent != CFG["images"] or p.suffix.lower() not in O.IMG_EXTS or not p.exists():
        abort(404)
    return p


def label_path(img_path):
    return CFG["labels"] / (Path(img_path).stem + ".txt")


def read_classes():
    f = CFG["classes"]
    if not f.exists():
        f.write_text("insan\ntasit\n", encoding="utf-8")
    return [l.strip() for l in f.read_text(encoding="utf-8").splitlines() if l.strip()]


def etiketli_kume():
    """Etiket dosyası olan karelerin kök adları (uzantısız).

    v3.6: Sonuç 2 saniye önbellekte tutulur. v3.5'te /api/durum her 2 saniyede bu klasörü yüzlerce kez baştan
    tarıyordu (bir liste kurarken her resim için bir kez) ve bu sırada kilidi tuttuğu için kaydetme ve kare
    geçişi bekliyordu. Program kendisi etiket yazınca etiket_degisti() önbelleği hemen geçersiz kılar; klasöre
    dışarıdan eklenen dosyalar en geç 2 saniyede görülür. Dönen küme değiştirilemez (frozenset)."""
    simdi = time.time()
    if simdi - _ETK["zaman"] > 2.0:
        try:
            kume = frozenset(e.name[:-4] for e in os.scandir(CFG["labels"])
                             if e.name.endswith(".txt") and e.name != "classes.txt" and e.is_file())
        except OSError:
            kume = frozenset()
        if kume != _ETK["kume"]:
            _ETK["surum"] += 1
        _ETK.update(zaman=simdi, kume=kume)
    return _ETK["kume"]


def etiket_degisti():
    """Program bir etiket dosyası yazdı ya da sildi: sonraki etiketli_kume() klasörü yeniden tarasın."""
    _ETK["zaman"] = 0.0


def model_etiket_kokleri():
    """Modelin toplu etiketlediği ama kullanıcının henüz kontrol etmediği kareler (kök adlar)."""
    return {Path(a).stem for a in DURUM.get("model_etiket", [])}


def rol(ad):
    if ad in ROL["test"]:
        return "test"
    if ad in ROL["tampon"]:
        return "tampon"
    return "havuz"


def model_secenekleri():
    yerel = sorted(p.name for p in BASE.glob("*.pt") if not p.name.lower().startswith(("sam", "mobile_sam")))
    return MODELLER + [m for m in yerel if m not in MODELLER]


# ================================================================ ayarlar ve durum
def dinamik_varsayilan():
    """Elle değiştirilmeyen eşikler güncel resim sayısına göre hesaplanır (klasöre sonradan resim eklenebilir)."""
    n = len(goruntu_adlari())
    return {"ilk_egitim_esigi": 100 if n >= 500 else max(MIN_ETIKETLI, round(n * 0.1)),
            "tur_buyuklugu": max(5, round(n * 0.05))}


def ayarlar_oku():
    ayar = {k: v[1] for k, v in AYAR_TANIM.items()}
    ayar.update(dinamik_varsayilan())
    kayitli = O.json_oku(calisma("ayarlar.json"), {}) or {}
    ayar.update({k: v for k, v in kayitli.items() if k in AYAR_TANIM})
    return ayar


def ayar_dogrula(yeni):
    ayar = ayarlar_oku()
    hatalar = []
    for k, v in (yeni or {}).items():
        if k not in AYAR_TANIM:
            continue
        tanim, ad = AYAR_TANIM[k], AYAR_ADI.get(k, k)
        tur = tanim[0]
        try:
            if tur == "bool":
                v = v if isinstance(v, bool) else str(v).strip().lower() in ("1", "true", "evet", "on", "açık", "acik")
            elif tur == "int":
                v = int(float(v))
                if not tanim[2] <= v <= tanim[3]:
                    hatalar.append(f"{ad} {tanim[2]} ile {tanim[3]} arasında olmalı.")
                    continue
            elif tur == "float":
                v = float(v)
                if v != v or not tanim[2] <= v <= tanim[3]:
                    hatalar.append(f"{ad} {tanim[2]} ile {tanim[3]} arasında olmalı.")
                    continue
            elif tur == "str":
                v = str(v).strip()[:40]
            elif tur == "secim":
                secenek = tanim[2] if tanim[2] is not None else model_secenekleri()
                if isinstance(secenek[0], int):
                    v = int(float(v))
                if v not in secenek:
                    hatalar.append(f"{ad} için geçersiz seçim.")
                    continue
        except (TypeError, ValueError):
            hatalar.append(f"{ad} sayı olmalı.")
            continue
        ayar[k] = v
    if ayar["batch"] == 0:
        hatalar.append("Batch 0 olamaz (-1 = otomatik).")
    if ayar["conf_aday"] > ayar["conf_on_etiket"]:
        hatalar.append("Aday güven eşiği, ön etiket güven eşiğinden büyük olamaz.")
    return ayar, hatalar


def durum_yaz():
    O.json_yaz(calisma("durum.json"), DURUM)


def roller_guncelle():
    ROL["test"] = set(DURUM.get("test", []))
    ROL["tampon"] = set(DURUM.get("tampon", []))


def _bloklu(havuz, sayi, blok=4):
    """Havuzu 'blok' eşit parçaya böler; her parçanın ortasından art arda kareler alır."""
    blok = max(1, min(blok, sayi))
    adet = [sayi // blok + (1 if i < sayi % blok else 0) for i in range(blok)]
    secilen, n = [], len(havuz)
    for i, k in enumerate(adet):
        bas, son = i * n // blok, (i + 1) * n // blok
        orta = max(bas, (bas + son - k) // 2)
        secilen += havuz[orta:orta + k]
    return list(dict.fromkeys(secilen))


def test_seti_hesapla(ayar, sayi=None, kaynak=None, yerlesim=None, tampon=None):
    """Test seti = zamansal olarak ayrık bir blok.
    En az 200 etiketli kare varsa: ETİKETLİ karelerin son %10'u (ek etiketleme gerekmez).
    Yoksa: tüm karelerin son %10'u (bunları elle etiketlemek gerekir).
    Her test karesinin önündeki ve arkasındaki 'tampon' kadar kare hiç kullanılmaz (ardışık kareler neredeyse
    aynı olduğu için test karelerine çok benzeyen kareler eğitime girmesin)."""
    adlar = goruntu_adlari()
    secim = DURUM.get("test_secim") or {}
    sayi = secim.get("sayi") if sayi is None else sayi
    kaynak = secim.get("kaynak") if kaynak is None else kaynak
    yerlesim = secim.get("yerlesim", "son") if yerlesim is None else yerlesim
    tampon = secim.get("tampon", ayar["test_tampon"]) if tampon is None else tampon
    if kaynak == "son":  # eski sürümlerle uyum: "son kareler" = tüm kareler + sondan
        kaynak = "tum"
    n, oran, tmp = len(adlar), ayar["test_orani"], int(tampon)
    kume = etiketli_kume() - model_etiket_kokleri()
    etiketli = [a for a in adlar if Path(a).stem in kume]
    if kaynak not in ("etiketli", "tum"):
        kaynak = "etiketli" if len(etiketli) >= 200 else "tum"
    havuz = etiketli if kaynak == "etiketli" else adlar
    if sayi is None:
        sayi = max(1, int(round(len(havuz) * oran))) if oran > 0 else 0
    sayi = max(0, min(int(sayi), len(havuz)))
    if n == 0 or sayi == 0:
        return [], []
    if yerlesim == "bas":
        test = havuz[:sayi]
    elif yerlesim == "bloklu":
        test = _bloklu(havuz, sayi)
    else:
        test = havuz[-sayi:]
    konum = {a: i for i, a in enumerate(adlar)}
    tset = set(test)
    yakin = set()
    for a in test:  # her test karesinin ±tmp komşusu tampon (test karesine çok benzeyen kareler eğitime girmesin)
        i = konum[a]
        yakin.update(range(max(0, i - tmp), min(n, i + tmp + 1)))
    tampon = [adlar[j] for j in sorted(yakin) if adlar[j] not in tset]
    return test, tampon


def test_seti_kur(ayar):
    DURUM["test"], DURUM["tampon"] = test_seti_hesapla(ayar)
    DURUM["test_surum"] = int(DURUM.get("test_surum", 0)) + 1  # arayüz listeyi yenilesin diye
    roller_guncelle()


def test_onayli():
    return bool(DURUM.get("test_onay")) or bool(basarili_turlar())


def basarili_turlar():
    return [t for t in DURUM.get("turlar", []) if t.get("durum") == "tamam"]


def sayimlar(ayar=None):
    ayar = ayar or ayarlar_oku()
    kume = etiketli_kume()
    mk = model_etiket_kokleri()
    s = {"toplam": 0, "havuz": 0, "havuz_etiketli": 0, "test": 0, "test_etiketli": 0, "tampon": 0,
         "tampon_etiketli": 0, "model_etiketli": len(mk & kume)}
    kume = kume - mk
    for ad in goruntu_adlari():
        r = rol(ad)
        et = Path(ad).stem in kume
        s["toplam"] += 1
        if r == "havuz":
            s["havuz"] += 1
            s["havuz_etiketli"] += et
        elif r == "test":
            s["test"] += 1
            s["test_etiketli"] += et
        else:
            s["tampon"] += 1
            s["tampon_etiketli"] += et
    s["ilk_model_var"] = bool(DURUM.get("son_basarili"))
    if not s["ilk_model_var"]:
        s["sonraki_kalan"] = max(0, ayar["ilk_egitim_esigi"] - s["havuz_etiketli"])
    else:
        s["sonraki_kalan"] = max(0, ayar["tur_buyuklugu"] - (s["havuz_etiketli"] - DURUM.get("son_baslangic_etiketli", 0)))
    s["test_eksik"] = s["test"] > 0 and s["test_etiketli"] < s["test"]
    s["etiketli_toplam"] = s["havuz_etiketli"] + s["test_etiketli"] + s["tampon_etiketli"] + s["model_etiketli"]
    s["etiketsiz"] = s["toplam"] - s["etiketli_toplam"]
    t = DURUM.get("tahmin")
    s["model_etiketlenebilir"] = len(model_etiketlenebilir()) if t else 0
    s["test_surum"] = DURUM.get("test_surum", 0)
    return s


# ================================================================ kuyruk (öneri sırası)
def tohum_listesi(ayar):
    """İlk model yokken etiketlenecek kareler: havuzdan eşit aralıklarla (örn. her 9-10 karede bir)."""
    havuz = [a for a in goruntu_adlari() if rol(a) == "havuz"]
    k = min(len(havuz), int(ayar["ilk_egitim_esigi"]))
    if k <= 0:
        return []
    if k == 1:
        return [havuz[0]]
    return [havuz[i] for i in sorted({int(round(i * (len(havuz) - 1) / (k - 1))) for i in range(k)})]


def kuyruk_kur(ayar):
    """Son tahminlerden öneri sırası: aktif seçimde öncelik = a_u*kararsızlık + a_t*titreme (büyükten küçüğe),
    rastgele seçimde karışık sıra. Denetim kareleri ilk 'tur büyüklüğü' kadar karenin arasına serpiştirilir."""
    t = DURUM.get("tahmin")
    if not t:
        DURUM["kuyruk"], DURUM["kuyruk_skor"] = [], {}
        return
    kareler = (O.json_oku(Path(t["dizin"]) / "ozet.json", {}) or {}).get("kareler", {})
    adlar = [a for a in O.dogal_sirala(kareler) if rol(a) == "havuz"]
    if ayar["secim"] == "aktif":
        wt = ayar["agirlik_t"] if ayar["titreme"] else 0.0
        skor = {a: round(ayar["agirlik_u"] * kareler[a]["u"] + wt * kareler[a]["t"], 4) for a in adlar}
        sira = sorted(adlar, key=lambda a: -skor[a])
    else:
        skor = {}
        sira = adlar[:]
        random.Random(t["tur"]).shuffle(sira)
    den = [a for a in DURUM.get("denetim", []) if a in kareler]
    dset = set(den)
    sira = [a for a in sira if a not in dset]
    b = int(ayar["tur_buyuklugu"])
    for i, a in enumerate(den):
        sira.insert(min(len(sira), int(round((i + 1) * b / (len(den) + 1))) + i), a)
    DURUM["kuyruk"], DURUM["kuyruk_skor"] = sira, skor


# ================================================================ eğitim işi
def tur_baslat(manuel):
    """Yeni bir eğitim turu başlatır. (başarılı mı, mesaj) döner."""
    if IS["proc"] is not None:
        return False, "Zaten bir eğitim sürüyor."
    if ANALIZ["proc"] is not None:
        return False, "Kılavuz için test tahmini sürüyor; bir dakika içinde biter, sonra tekrar dene."
    if not test_onayli():
        return False, "Önce test setini onayla (Model sekmesi > Test setini seç)."
    ayar = ayarlar_oku()
    siniflar = read_classes()
    adlar = goruntu_adlari()
    kume = etiketli_kume() - model_etiket_kokleri()   # kontrol edilmemiş model etiketleri eğitime girmez
    havuz = [a for a in adlar if rol(a) == "havuz"]
    etiketli = [a for a in havuz if Path(a).stem in kume]
    if len(etiketli) < MIN_ETIKETLI:
        return False, f"Eğitim için en az {MIN_ETIKETLI} etiketli kare gerekli (şu an {len(etiketli)})."
    kutu = sum(len(O.etiket_oku(CFG["labels"] / (Path(a).stem + ".txt"))) for a in etiketli)
    if kutu == 0:
        return False, "Etiketli karelerde hiç kutu yok."

    # Doğrulama kümesi: ada göre sabit seçilir; aynı kare her turda aynı kümede kalır.
    dogrulama = [a for a in etiketli if O.kume_payi(a) < ayar["dogrulama_orani"]]
    if len(dogrulama) < 2:
        dogrulama = sorted(etiketli, key=O.kume_payi)[:2]
    dset = set(dogrulama)
    egitim = [a for a in etiketli if a not in dset]
    test = [a for a in DURUM.get("test", []) if Path(a).stem in kume]
    tahmin = [a for a in havuz if Path(a).stem not in kume]

    sb = DURUM.get("son_basarili")
    sicak = bool(sb and sb.get("model") == ayar["model"] and sb.get("siniflar") == siniflar
                 and sb.get("parca") == ayar["parca"] and Path(sb.get("agirlik", "")).exists())
    agirlik = sb["agirlik"] if sicak else ayar["model"]
    epochs = ayar["tur_epoch"] if sicak else ayar["ilk_epoch"]

    tur = int(DURUM.get("sonraki_tur", 1))
    DURUM["sonraki_tur"] = tur + 1
    tdir = calisma("turlar", f"tur_{tur:03d}")
    tdir.mkdir(parents=True, exist_ok=True)
    for f in (calisma("iptal.bayrak"), calisma("is_durumu.json")):
        f.unlink(missing_ok=True)
    job = {
        "tur": tur, "cikti": str(tdir), "durum_dosyasi": str(calisma("is_durumu.json")),
        "iptal_dosyasi": str(calisma("iptal.bayrak")), "images": str(CFG["images"]), "labels": str(CFG["labels"]),
        "siniflar": siniflar, "ayarlar": ayar,
        "parca_kok": str(calisma("parcalar", f"{ayar['parca']}_{ayar['ortusme']:.2f}")),
        "egitim": egitim, "dogrulama": dogrulama, "test": test, "tahmin": tahmin,
        "tum_sira": adlar, "etiketli_havuz": etiketli, "baslangic_agirlik": agirlik, "epochs": epochs,
    }
    O.json_yaz(tdir / "is.json", job)
    log = open(tdir / "log.txt", "w", encoding="utf-8")
    env = os.environ.copy()
    env.update(PYTHONUTF8="1", PYTHONIOENCODING="utf-8")
    try:
        proc = subprocess.Popen([sys.executable, "-u", str(BASE / "egitim.py"), "--is", str(tdir / "is.json")],
                                cwd=str(BASE), stdout=log, stderr=subprocess.STDOUT, env=env)
    except OSError as e:
        log.close()
        return False, f"Eğitim süreci başlatılamadı: {e}"
    IS.update(proc=proc, tur=tur, log=log, baslangic=time.time(), iptal_zamani=None)
    DURUM.setdefault("turlar", []).append({
        "tur": tur, "durum": "calisiyor", "baslangic": time.time(), "etiketli": len(etiketli),
        "egitim_kare": len(egitim), "dogrulama_kare": len(dogrulama), "test_kare": len(test), "kutu": kutu,
        "epochs": epochs, "model": ayar["model"], "sicak": sicak, "secim": ayar["secim"],
        "parca": ayar["parca"], "tetik": "elle" if manuel else "otomatik",
    })
    DURUM["son_baslangic_etiketli"] = len(etiketli)
    DURUM["aktif_is"] = {"tur": tur}
    if manuel:
        DURUM["oto_beklet"] = None
    durum_yaz()
    print(f"[Eğitim] Tur {tur} başladı ({'sıcak başlangıç' if sicak else agirlik}, {epochs} epoch, "
          f"{len(egitim)} eğitim + {len(dogrulama)} doğrulama karesi).")
    return True, f"Tur {tur} başladı."


def tur_sonucunu_isle(tur, sonuc):
    ayar = ayarlar_oku()
    kayit = next((t for t in DURUM.get("turlar", []) if t["tur"] == tur), None)
    if kayit is None:
        kayit = {"tur": tur}
        DURUM.setdefault("turlar", []).append(kayit)
    m = sonuc.get("metrik") or {}
    kayit.update({
        "durum": sonuc.get("durum", "hata"), "bitis": time.time(), "mesaj": sonuc.get("mesaj"),
        "sure_sn": sonuc.get("sure_sn"), "epoch_yapilan": sonuc.get("epoch_yapilan"), "cihaz": sonuc.get("cihaz"),
        "batch": sonuc.get("batch"), "val_map50": sonuc.get("val_map50"),
        "map50": m.get("map50"), "map5095": m.get("map5095"), "p": m.get("p"), "r": m.get("r"),
        "sinif": m.get("sinif"), "olculen_test": m.get("test_kare"), "temiz": m.get("temiz"), "surum": "3.6",
    })
    tdir = calisma("turlar", f"tur_{tur:03d}")
    if kayit["durum"] == "tamam":
        DURUM["son_basarili"] = {"tur": tur, "agirlik": sonuc["agirlik"], "model": kayit.get("model"),
                                 "siniflar": read_classes(), "parca": kayit.get("parca")}
        DURUM["tahmin"] = {"tur": tur, "dizin": str(tdir / "tahmin")}
        kareler = (O.json_oku(tdir / "tahmin" / "ozet.json", {}) or {}).get("kareler", {})
        k = math.ceil(ayar["denetim_orani"] * ayar["tur_buyuklugu"]) if ayar["denetim_orani"] > 0 else 0
        aday = O.dogal_sirala(kareler)
        DURUM["denetim"] = random.Random(tur).sample(aday, min(k, len(aday)))
        kuyruk_kur(ayar)
        DURUM["son_hata"] = None
        yenilenen = m_etiketlerini_yenile()  # kontrol bekleyen M kareler yeni modelin önerileriyle güncellenir
        if yenilenen:
            print(f"[Eğitim] {yenilenen} M etiketi yeni modelle güncellendi.")
        # Yer kazanmak için önceki tamamlanmış turların last.pt dosyalarını sil (best.pt kalır).
        for f in calisma("turlar").glob("tur_*/egitim*/weights/last.pt"):
            if not str(f).startswith(str(tdir)):
                f.unlink(missing_ok=True)
        # Durma kuralı
        olcum = [t["map50"] for t in basarili_turlar() if t.get("map50") is not None and not t.get("eski_test")]
        n = int(ayar["plato_tur"])
        if len(olcum) > n and all(olcum[-i] - olcum[-i - 1] < ayar["plato_esik"] for i in range(1, n + 1)):
            DURUM["oto_beklet"] = "plato"
        print(f"[Eğitim] Tur {tur} bitti. mAP50={kayit['map50']}")
    else:
        DURUM["oto_beklet"] = "iptal" if kayit["durum"] == "iptal" else "hata"
        son = []
        try:
            son = (tdir / "log.txt").read_text(encoding="utf-8", errors="replace").splitlines()[-25:]
        except OSError:
            pass
        DURUM["son_hata"] = {"tur": tur, "mesaj": kayit.get("mesaj"), "log": son} if kayit["durum"] == "hata" else None
        print(f"[Eğitim] Tur {tur}: {kayit['durum']} - {kayit.get('mesaj')}")
    DURUM["aktif_is"] = None
    durum_yaz()


def is_kontrol():
    p = IS["proc"]
    if p is None:
        return
    if p.poll() is None:
        if IS["iptal_zamani"] and time.time() - IS["iptal_zamani"] > 30:
            p.kill()  # nazik iptal 30 sn içinde olmadıysa zorla kapat
        return
    tur = IS["tur"]
    try:
        IS["log"].close()
    except Exception:  # noqa: BLE001
        pass
    IS.update(proc=None, tur=None, log=None, iptal_zamani=None)
    sonuc = O.json_oku(calisma("turlar", f"tur_{tur:03d}", "sonuc.json"))
    if not sonuc:
        sonuc = {"durum": "hata", "mesaj": f"Eğitim süreci beklenmedik şekilde kapandı (kod {p.returncode}). "
                                           "Ayrıntı için turun log.txt dosyasına bak."}
    tur_sonucunu_isle(tur, sonuc)


def oto_tetik():
    if IS["proc"] is not None or ANALIZ["proc"] is not None or DURUM.get("oto_beklet"):
        return
    ayar = ayarlar_oku()
    if not ayar["oto_egitim"]:
        return
    s = sayimlar(ayar)
    if not test_onayli() or s["test_eksik"] or s["havuz_etiketli"] < MIN_ETIKETLI:
        return
    if s["sonraki_kalan"] == 0:
        ok, mesaj = tur_baslat(manuel=False)
        if not ok:
            DURUM["oto_beklet"] = "hata"
            DURUM["son_hata"] = {"tur": None, "mesaj": mesaj, "log": []}
            durum_yaz()


_XML_HATALI = set()


def xml_iceri_aktar():
    """Klasörde .xml etiketi olup .txt karşılığı olmayan her resim için YOLO .txt yazar (.xml olduğu gibi kalır).
    Sınıf adları classes.txt ile eşlenir ('Taşıt' = 'tasit', 'person' = 'insan' gibi); bilinmeyen ad yeni sınıf olur.
    Çok sayıda etiket birden gelince otomatik eğitim, kullanıcı kontrol edene kadar bekletilir."""
    try:
        xmller = {}
        for e in os.scandir(CFG["labels"]):
            ad = e.name.lower()
            if (ad.endswith(".xml") or ad.endswith(".json")) and e.is_file():
                xmller.setdefault(e.name.rsplit(".", 1)[0], e.path)
    except OSError:
        return 0
    if not xmller:
        return 0
    kume = etiketli_kume()
    adlar = {Path(a).stem: a for a in goruntu_adlari()}
    bekleyen = O.dogal_sirala(k for k in xmller if k in adlar and k not in kume and k not in _XML_HATALI)
    if not bekleyen:
        return 0
    siniflar = read_classes()
    anahtar = {O.sinif_anahtar(c): i for i, c in enumerate(siniflar)}
    yeni_sinif, sayi, kutu = [], 0, 0
    for k in bekleyen:
        try:
            W, H, nesneler = (O.labelme_oku(xmller[k]) if xmller[k].lower().endswith(".json") else O.voc_oku(xmller[k]))
            if W <= 0 or H <= 0:
                W, H = O.goruntu_boyutu(CFG["images"] / adlar[k])
        except Exception as e:  # noqa: BLE001
            _XML_HATALI.add(k)
            print(f"[XML] Okunamadı, atlandı: {Path(xmller[k]).name} ({e})")
            continue
        satirlar = []
        for ad, x1, y1, x2, y2 in nesneler:
            i = O.sinif_bul(ad, anahtar)
            if i is None:  # bilinmeyen sınıf adı: listeye ekle
                siniflar.append(ad)
                i = anahtar[O.sinif_anahtar(ad)] = len(siniflar) - 1
                yeni_sinif.append(ad)
            x1, y1, x2, y2 = max(0.0, x1), max(0.0, y1), min(float(W), x2), min(float(H), y2)
            if x2 - x1 < 1 or y2 - y1 < 1:
                continue
            satirlar.append(f"{i} {(x1 + x2) / 2 / W:.6f} {(y1 + y2) / 2 / H:.6f} {(x2 - x1) / W:.6f} {(y2 - y1) / H:.6f}")
        O.metin_yaz(CFG["labels"] / f"{k}.txt", "\n".join(satirlar) + ("\n" if satirlar else ""))
        etiket_degisti()
        sayi += 1
        kutu += len(satirlar)
    if yeni_sinif:
        CFG["classes"].write_text("\n".join(siniflar) + "\n", encoding="utf-8")
    if sayi:
        DURUM["xml_aktarim"] = {"sayi": sayi, "kutu": kutu, "yeni_siniflar": yeni_sinif, "zaman": time.time()}
        if sayi >= 5:
            DURUM["oto_beklet"] = "ice_aktarma"
        durum_yaz()
        print(f"[XML] {sayi} karenin etiketi .xml/.json dosyalarından içe aktarıldı ({kutu} kutu). Sınıflar: {siniflar}"
              + (f"  (yeni eklenen: {yeni_sinif})" if yeni_sinif else ""))
    return sayi


def test_seti_kontrol():
    """İlk başarılı turdan önce, test setindeki hiçbir kare henüz etiketlenmemişse test setini güncel resim
    listesine göre yeniden seçer (örn. klasöre sonradan 1.000 fotoğraf eklendiyse)."""
    if basarili_turlar() or DURUM.get("test_onay"):
        return  # kullanıcı onayladı ya da ölçüm başladı: test seti sabit
    ayar = ayarlar_oku()
    mevcut = DURUM.get("test", [])
    kume = etiketli_kume()
    if any(Path(a).stem in kume for a in mevcut):
        return  # test setinde etiketli kare var (elle etiketlenmiş ya da etiketlilerden seçilmiş): sabit kalsın
    yeni, _ = test_seti_hesapla(ayar)
    if yeni == mevcut:
        return
    test_seti_kur(ayar)
    durum_yaz()
    print(f"[Bilgi] Resim sayısı değişti, test seti yeniden seçildi ({len(DURUM['test'])} kare).")


def izleyici():
    while True:
        try:
            with KILIT:
                xml_iceri_aktar()
                test_seti_kontrol()
                is_kontrol()
                analiz_kontrol()
                oto_tetik()
        except Exception:  # noqa: BLE001
            traceback.print_exc()
        time.sleep(2)


# ================================================================ sayfalar
@app.get("/")
def index():
    return send_from_directory(app.static_folder, "index.html")


@app.get("/api/info")
def info():
    with KILIT:
        kume = etiketli_kume()
        mset = set(DURUM.get("model_etiket", []))
        imgs = []
        for ad in goruntu_adlari():
            lab = Path(ad).stem in kume
            count = 0
            if lab:
                try:
                    count = sum(1 for l in label_path(ad).read_text(encoding="utf-8").splitlines() if l.strip())
                except OSError:
                    pass
            imgs.append({"name": ad, "labeled": lab, "count": count, "rol": rol(ad), "model": ad in mset})
    return jsonify(classes=read_classes(), images=imgs)


@app.get("/api/image/<path:name>")
def image(name):
    return send_file(find_image(name), max_age=3600)


@app.get("/api/labels/<path:name>")
def get_labels(name):
    p = find_image(name)
    lp = label_path(p)
    boxes = []
    if lp.exists():
        for c, cx, cy, w, h in O.etiket_oku(lp):
            boxes.append({"cls": c, "cx": cx, "cy": cy, "w": w, "h": h})
    with KILIT:
        r = rol(name)
        denetim = name in set(DURUM.get("denetim", []))
        t = DURUM.get("tahmin")
        ayar = ayarlar_oku()
    oneri, aday, tahmin_var = [], [], False
    if t and r == "havuz" and not denetim:
        veri = O.json_oku(Path(t["dizin"]) / "kareler" / (p.stem + ".json"))
        if veri:
            nc = len(read_classes())
            kutular = [k for k in oneri_kutulari(veri, ayar) if 0 <= k[4] < nc]
            tahmin_var = True
            aday = kutular
            if not lp.exists():
                oneri = [k for k in kutular if k[5] >= ayar["conf_on_etiket"]]
    model = name in set(DURUM.get("model_etiket", []))
    return jsonify(boxes=boxes, labeled=lp.exists(), model=model, rol=r, denetim=denetim, tahmin_var=tahmin_var,
                   tahmin_tur=t["tur"] if t else None, oneri=oneri, aday=aday)


@app.post("/api/labels/<path:name>")
def save_labels(name):
    p = find_image(name)
    boxes = (request.get_json(force=True) or {}).get("boxes", [])
    emek = (request.get_json(silent=True) or {}).get("emek")
    lines = []
    for b in boxes:
        vals = [min(max(float(b[k]), 0.0), 1.0) for k in ("cx", "cy", "w", "h")]
        if vals[2] <= 0 or vals[3] <= 0:
            continue
        lines.append(f"{int(b['cls'])} " + " ".join(f"{v:.6f}" for v in vals))
    # Boş dosya da yazılır: "bu görüntüde nesne yok" anlamına gelir.
    O.metin_yaz(label_path(p), "\n".join(lines) + ("\n" if lines else ""))
    etiket_degisti()
    if isinstance(emek, dict):
        try:
            kayit = {"ad": name, "zaman": round(time.time()), "kutu": len(lines),
                     **{k: emek.get(k) for k in ("sure_sn", "eklenen", "silinen", "duzeltilen", "sinif_degisen",
                                                  "kaynak", "ilk_kutu")}}
            with open(calisma("emek.jsonl"), "a", encoding="utf-8") as f:
                f.write(json.dumps(kayit, ensure_ascii=False) + "\n")
        except (OSError, TypeError, ValueError):
            pass
    with KILIT:
        if name in DURUM.get("model_etiket", []):   # kullanıcı kontrol edip kaydetti: artık onun etiketi
            DURUM["model_etiket"] = [a for a in DURUM["model_etiket"] if a != name]
            durum_yaz()
    return jsonify(ok=True, count=len(lines))


def emek_kayitlari():
    try:
        return [json.loads(l) for l in calisma("emek.jsonl").read_text(encoding="utf-8").splitlines() if l.strip()]
    except (OSError, ValueError):
        return []


def emek_ozeti():
    """Son 50 kayıttan: kare başına süre ve düzeltme sayıları, kaynağa göre (model önerisi / elle).
    v3.6: dosya değişmedikçe yeniden okunmaz (eskiden her 2 saniyede bütün dosya okunuyordu)."""
    try:
        st = calisma("emek.jsonl").stat()
        anahtar = (st.st_mtime_ns, st.st_size)
    except OSError:
        return None
    if anahtar == _EMEK["anahtar"]:
        return _EMEK["sonuc"]
    _EMEK.update(anahtar=anahtar, sonuc=_emek_ozeti_hesapla())
    return _EMEK["sonuc"]


def _emek_ozeti_hesapla():
    kayit = [k for k in emek_kayitlari() if isinstance(k.get("sure_sn"), (int, float))]
    if not kayit:
        return None
    son = kayit[-50:]

    def ozet(ks):
        n = len(ks)
        return {"kare": n, "sure": round(sum(k["sure_sn"] for k in ks) / n, 1),
                "eklenen": round(sum(k.get("eklenen") or 0 for k in ks) / n, 1),
                "silinen": round(sum(k.get("silinen") or 0 for k in ks) / n, 1),
                "duzeltilen": round(sum((k.get("duzeltilen") or 0) + (k.get("sinif_degisen") or 0) for k in ks) / n, 1)}
    return {"toplam": len(kayit), "son": ozet(son),
            "model": ozet(m) if (m := [k for k in son if k.get("kaynak") in ("model", "oneri")]) else None,
            "elle": ozet(e) if (e := [k for k in son if k.get("kaynak") == "elle"]) else None}


@app.get("/api/emek.csv")
def emek_csv():
    alanlar = ["ad", "zaman", "kaynak", "sure_sn", "ilk_kutu", "kutu", "eklenen", "silinen", "duzeltilen", "sinif_degisen"]
    buf = io.StringIO()
    w = csv.writer(buf, delimiter=";")
    w.writerow(alanlar)
    for k in emek_kayitlari():
        w.writerow([time.strftime("%d.%m.%Y %H:%M:%S", time.localtime(k["zaman"])) if a == "zaman" and k.get("zaman")
                    else (str(k.get(a)).replace(".", ",") if isinstance(k.get(a), float) else ("" if k.get(a) is None else k.get(a)))
                    for a in alanlar])
    return Response("\ufeff" + buf.getvalue(), mimetype="text/csv; charset=utf-8",
                    headers={"Content-Disposition": "attachment; filename=emek.csv"})


_TARAMA = {"zaman": 0.0, "sonuc": None}


def etikete_benziyor(yol, uzanti):
    """Dosyanın içi gerçekten etiket mi (not dosyalarını uyarıya katmamak için)?"""
    try:
        with open(yol, encoding="utf-8", errors="ignore") as f:
            bas = f.read(4096)
    except OSError:
        return False
    if uzanti == "xml":
        return "<annotation" in bas
    if uzanti == "json":
        return '"shapes"' in bas
    satir = next((l for l in bas.splitlines() if l.strip()), "")
    return bool(re.fullmatch(r"\s*\d+(\s+-?[\d.eE+-]+){4}\s*", satir)) or satir == ""


def etiket_taramasi():
    """Okunmayan etiketleri bulur: hiçbir resimle eşleşmeyen .txt/.xml/.json dosyaları ve yan klasörlerdeki etiketler."""
    if time.time() - _TARAMA["zaman"] < 10 and _TARAMA["sonuc"] is not None:
        return _TARAMA["sonuc"]
    kokler = {Path(a).stem for a in goruntu_adlari()}
    kume = etiketli_kume()
    eslesmeyen = []
    try:
        for e in os.scandir(CFG["labels"]):
            u = e.name.rsplit(".", 1)
            if len(u) == 2 and u[1].lower() in ("txt", "xml", "json") and e.name != "classes.txt" \
                    and u[0] not in kokler and e.is_file() and etikete_benziyor(e.path, u[1].lower()):
                eslesmeyen.append(e.name)
    except OSError:
        pass
    dis = []
    im = CFG["images"]
    for aday in {im.parent / (im.name + "_labels"), im.parent / "labels", im / "labels", im.parent / "Annotations"}:
        if aday.is_dir() and aday.resolve() != CFG["labels"].resolve():
            try:
                n = sum(1 for e in os.scandir(aday) if e.name.rsplit(".", 1)[0] in kokler
                        and e.name.rsplit(".", 1)[0] not in kume and e.name.lower().endswith((".txt", ".xml", ".json")))
            except OSError:
                n = 0
            if n:
                dis.append({"klasor": str(aday), "sayi": n})
    _TARAMA.update(zaman=time.time(), sonuc={"eslesmeyen": len(eslesmeyen), "ornek": O.dogal_sirala(eslesmeyen)[:3],
                                            "dis": dis})
    return _TARAMA["sonuc"]


_DETAY = {"anahtar": None, "sonuc": {}}


def kuyruk_detay(t):
    """Öneri sırasındaki her karenin öncelik parçaları: [kararsızlık, titreme, öneri sayısı]."""
    if _DETAY["anahtar"] != t["dizin"]:
        kareler = (O.json_oku(Path(t["dizin"]) / "ozet.json", {}) or {}).get("kareler", {})
        _DETAY.update(anahtar=t["dizin"], sonuc={a: [v.get("u", 0), v.get("t", 0), v.get("n", 0)] for a, v in kareler.items()})
    return _DETAY["sonuc"]


@app.get("/api/kuyruk")
def kuyruk():
    with KILIT:
        ayar = ayarlar_oku()
        t = DURUM.get("tahmin")
        if t and DURUM.get("kuyruk"):
            return jsonify(tur=t["tur"], adlar=DURUM["kuyruk"], denetim=DURUM.get("denetim", []),
                           skor=DURUM.get("kuyruk_skor", {}), detay=kuyruk_detay(t), tohum=False,
                           agirlik={"u": ayar["agirlik_u"], "t": ayar["agirlik_t"] if ayar["titreme"] else 0.0})
        return jsonify(tur=None, adlar=tohum_listesi(ayar), denetim=[], skor={}, tohum=True)


@app.get("/api/durum")
def durum():
    with KILIT:
        ayar = ayarlar_oku()
        is_bilgi = None
        if IS["proc"] is not None:
            d = O.json_oku(calisma("is_durumu.json"), {}) or {}
            if d.get("tur") != IS["tur"]:
                d = {}
            is_bilgi = {"tur": IS["tur"], "asama": d.get("asama", "baslatiliyor"), "ilerleme": d.get("ilerleme"),
                        "mesaj": d.get("mesaj", "Başlatılıyor"), "epoch": d.get("epoch"), "epochs": d.get("epochs"),
                        "val_map50": d.get("val_map50"), "cihaz": d.get("cihaz"), "batch": d.get("batch"),
                        "gecen_sn": round(time.time() - IS["baslangic"]),
                        "iptal_istendi": IS["iptal_zamani"] is not None}
        model_yolu = BASE / ayar["model"]
        kume = etiketli_kume()
        sayac = sayimlar(ayar)
        return jsonify({
            "is": is_bilgi, "turlar": DURUM.get("turlar", []), "ayarlar": ayar, "sayac": sayac,
            "analiz": analiz_durumu(),
            "oto_beklet": DURUM.get("oto_beklet"), "son_hata": DURUM.get("son_hata"),
            "xml_aktarim": DURUM.get("xml_aktarim"), "tarama": etiket_taramasi(),
            "test_onay": test_onayli(), "test_secim": DURUM.get("test_secim"), "emek": emek_ozeti(),
            "etiketli_sayi": sayac["etiketli_toplam"] - sayac["model_etiketli"],
            "test_kaynak": "etiketli" if (DURUM.get("test") and all(Path(a).stem in kume for a in DURUM["test"])) else "son",
            "tahmin_tur": (DURUM.get("tahmin") or {}).get("tur"), "modeller": model_secenekleri(),
            "model_dosyasi_var": model_yolu.exists() or bool(DURUM.get("son_basarili")),
            "test_kilitli": bool(basarili_turlar()), "calisma": str(CFG["calisma"]),
        })


# ================================================================ kılavuz ve analiz (v3.6)
@app.get("/kilavuz")
def kilavuz_sayfasi():
    return send_from_directory(app.static_folder, "kilavuz.html")


def _tur_dizini(tur):
    return calisma("turlar", f"tur_{int(tur):03d}")


def analiz_durumu():
    """Kılavuz için test tahmini işinin durumu (sunucu kilidi tutulurken çağrılır)."""
    if ANALIZ["proc"] is not None:
        d = O.json_oku(calisma("analiz_durumu.json"), {}) or {}
        return {"calisiyor": True, "tur": ANALIZ["tur"], "ilerleme": d.get("ilerleme"), "mesaj": d.get("mesaj", "Başlatılıyor"),
                "gecen_sn": round(time.time() - ANALIZ["baslangic"])}
    return {"calisiyor": False, "hata": ANALIZ["hata"]}


def analiz_kontrol():
    p = ANALIZ["proc"]
    if p is None or p.poll() is None:
        return
    try:
        ANALIZ["log"].close()
    except Exception:  # noqa: BLE001
        pass
    sonuc = O.json_oku(_tur_dizini(ANALIZ["tur"]) / "analiz_sonuc.json", {}) or {}
    ANALIZ["hata"] = None if sonuc.get("durum") == "tamam" else (sonuc.get("mesaj") or f"Analiz süreci kapandı (kod {p.returncode}).")
    print(f"[Kılavuz] Tur {ANALIZ['tur']} test tahmini: {sonuc.get('durum', 'hata')}")
    ANALIZ.update(proc=None, log=None)


@app.post("/api/analiz_hazirla")
def analiz_hazirla():
    """Eski (v3.5) turlarda test tahminleri saklanmıyordu: son modelle test karelerini bir kez tahmin eder.
    Kısa sürer (40 karede yaklaşık 1-2 dakika); eğitim sürerken çalışmaz."""
    with KILIT:
        if IS["proc"] is not None:
            return jsonify(error="Eğitim sürerken yapılamaz; tur bitince tekrar dene."), 409
        if ANALIZ["proc"] is not None:
            return jsonify(ok=True, mesaj="Zaten hazırlanıyor.")
        sb = DURUM.get("son_basarili")
        if not sb or not Path(sb.get("agirlik", "")).exists():
            return jsonify(error="Eğitilmiş model bulunamadı. Önce bir tur eğit."), 400
        kume = etiketli_kume() - model_etiket_kokleri()
        test = [a for a in DURUM.get("test", []) if Path(a).stem in kume]
        if not test:
            return jsonify(error="Test setinde etiketli kare yok."), 400
        tur = int(sb["tur"])
        tdir = _tur_dizini(tur)
        tdir.mkdir(parents=True, exist_ok=True)
        for f in (calisma("analiz_durumu.json"), calisma("analiz_iptal.bayrak"), tdir / "analiz_sonuc.json"):
            f.unlink(missing_ok=True)
        ayar = ayarlar_oku()
        job = {"tur": tur, "cikti": str(tdir), "durum_dosyasi": str(calisma("analiz_durumu.json")),
               "iptal_dosyasi": str(calisma("analiz_iptal.bayrak")), "images": str(CFG["images"]),
               "labels": str(CFG["labels"]), "siniflar": read_classes(), "ayarlar": {**ayar, "parca": sb.get("parca", ayar["parca"])},
               "test": test, "agirlik": sb["agirlik"]}
        O.json_yaz(calisma("analiz_is.json"), job)
        log = open(tdir / "analiz_log.txt", "w", encoding="utf-8")
        env = os.environ.copy()
        env.update(PYTHONUTF8="1", PYTHONIOENCODING="utf-8")
        try:
            proc = subprocess.Popen([sys.executable, "-u", str(BASE / "egitim.py"), "--analiz", str(calisma("analiz_is.json"))],
                                    cwd=str(BASE), stdout=log, stderr=subprocess.STDOUT, env=env)
        except OSError as e:
            log.close()
            return jsonify(error=f"Süreç başlatılamadı: {e}"), 500
        ANALIZ.update(proc=proc, tur=tur, log=log, baslangic=time.time(), hata=None)
    return jsonify(ok=True, mesaj=f"Tur {tur} modeliyle test kareleri tahmin ediliyor.")


def _analiz_hesapla(tur, temiz, ayar, siniflar):
    """test_tahmin.json + güncel test etiketlerinden ayrıntılı ölçüm. Etiket ya da ayar değişmedikçe önbellekten."""
    yol = _tur_dizini(tur) / "test_tahmin.json"
    try:
        st = yol.stat()
    except OSError:
        return None
    veri = O.json_oku(yol, {}) or {}
    kareler = veri.get("kareler", {})
    imza = []
    for ad in kareler:
        try:
            imza.append(label_path(ad).stat().st_mtime_ns)
        except OSError:
            imza.append(0)
    nc = len(siniflar)
    conf = float(ayar["conf_on_etiket"])
    anahtar = (tur, temiz, st.st_mtime_ns, tuple(imza), nc, float(ayar["oneri_nms"]), bool(ayar["grup_filtre"]), conf)
    if anahtar in _ANALIZ_ONB:
        return _ANALIZ_ONB[anahtar]
    tahmin, gercek = {}, {}
    for ad, k in kareler.items():
        lp = label_path(ad)
        if not lp.exists():
            continue
        gercek[ad] = O.etiket_piksel(lp, k["w"], k["h"], nc)
        a = np.asarray(k.get("k") or np.zeros((0, 6)), dtype=np.float64).reshape(-1, 6)
        b, sk, c = a[:, :4], a[:, 5], a[:, 4].astype(np.int64)
        if temiz:
            b, sk, c = O.oneri_temizle(b, sk, c, nms_iou=float(ayar["oneri_nms"]), grup=bool(ayar["grup_filtre"]))
        tahmin[ad] = (b, sk, c)
    if not gercek:
        return None
    es = O.sinif_eslesmeleri(tahmin, gercek, nc)
    m = O.olcutler(es, conf, ayrinti=True)
    if m:
        m["conf"] = conf
        m["karisiklik"] = O.karisiklik(tahmin, gercek, nc, conf=conf)
        m["karisiklik_esik"] = {f"{t:.2f}": O.karisiklik(tahmin, gercek, nc, conf=t) for t in O.ESIKLER}
        m["test_kare"] = len(gercek)
        m["kutu_ortalama"] = round(sum(len(v[1]) for v in gercek.values()) / len(gercek), 2)
    if len(_ANALIZ_ONB) > 16:
        _ANALIZ_ONB.clear()
    _ANALIZ_ONB[anahtar] = m
    return m


@app.get("/api/analiz")
def analiz():
    """Kılavuz sayfası için canlı ölçümler: sınıf başına TP/FP/FN, P, R, F1, AP, PR eğrisi, eşik eğrisi ve
    her eşik için karışıklık tablosu. Ham (temiz=0) ya da temizlenmiş öneriler (temiz=1, varsayılan)."""
    with KILIT:
        ayar = ayarlar_oku()
        siniflar = read_classes()
        turlar = [t for t in basarili_turlar() if not t.get("eski_test")] or basarili_turlar()
        durum_ = analiz_durumu()
    temiz = request.args.get("temiz", "1") != "0"
    istek = request.args.get("tur", type=int)
    sirali = [t for t in turlar if istek is None or t["tur"] == istek]
    hedef = next((t for t in reversed(sirali) if (_tur_dizini(t["tur"]) / "test_tahmin.json").exists()), None)
    temel = {"siniflar": siniflar, "conf": ayar["conf_on_etiket"], "oneri_nms": ayar["oneri_nms"],
             "grup_filtre": ayar["grup_filtre"], "analiz": durum_, "temiz": temiz,
             "son_tur": turlar[-1]["tur"] if turlar else None}
    if hedef is None:
        return jsonify(hazir=False, **temel)
    m = _analiz_hesapla(hedef["tur"], temiz, ayar, siniflar)
    if not m:
        return jsonify(hazir=False, **temel)
    bilgi = {k: hedef.get(k) for k in ("tur", "etiketli", "map50", "map5095", "p", "r", "model", "temiz", "surum")}
    return jsonify(hazir=True, tur=hedef["tur"], tur_bilgi=bilgi, metrik=m, **temel)


@app.post("/api/ayarlar")
def ayar_kaydet():
    ayar, hatalar = ayar_dogrula(request.get_json(force=True))
    if hatalar:
        return jsonify(error=" ".join(hatalar)), 400
    with KILIT:
        eski = ayarlar_oku()
        test_degisti = ayar["test_orani"] != eski["test_orani"] or ayar["test_tampon"] != eski["test_tampon"]
        if test_degisti and basarili_turlar():
            return jsonify(error="Test seti ilk başarılı turdan sonra değiştirilemez (öğrenme eğrisi bozulur)."), 400
        # Sadece varsayılandan farklı olanlar saklanır; dokunulmayan eşikler resim sayısıyla güncellenmeye devam eder.
        varsayilan = {k: v[1] for k, v in AYAR_TANIM.items()}
        varsayilan.update(dinamik_varsayilan())
        O.json_yaz(calisma("ayarlar.json"), {k: v for k, v in ayar.items() if v != varsayilan.get(k)})
        if test_degisti:
            if ayar["test_orani"] != eski["test_orani"]:
                DURUM.pop("test_secim", None)
                DURUM["test_onay"] = False
            test_seti_kur(ayar)
        kuyruk_kur(ayar)
        durum_yaz()
    return jsonify(ok=True, ayarlar=ayar)


@app.post("/api/egit")
def egit():
    with KILIT:
        ok, mesaj = tur_baslat(manuel=True)
    return (jsonify(ok=True, mesaj=mesaj), 200) if ok else (jsonify(error=mesaj), 400)


@app.post("/api/iptal")
def iptal():
    with KILIT:
        if IS["proc"] is None:
            return jsonify(error="Çalışan bir eğitim yok."), 400
        calisma("iptal.bayrak").write_text("iptal", encoding="utf-8")
        IS["iptal_zamani"] = time.time()
    return jsonify(ok=True)


def model_etiketlenebilir():
    """Modelin önerisi olan, henüz etiketi olmayan havuz kareleri (test, tampon ve denetim kareleri hariç)."""
    t = DURUM.get("tahmin")
    if not t:
        return []
    kume, adlar = etiketli_kume(), goruntu_adlari()
    anahtar = (t["tur"], t["dizin"], id(kume), id(adlar), DURUM.get("test_surum"), tuple(DURUM.get("denetim", [])))
    if anahtar == _MEO["anahtar"]:
        return _MEO["sonuc"]
    try:  # v3.5'te her resim için ayrı ayrı dosya var mı diye bakılıyordu; klasörü bir kez listelemek yeter
        mevcut = {e.name[:-5] for e in os.scandir(Path(t["dizin"]) / "kareler") if e.name.endswith(".json")}
    except OSError:
        mevcut = set()
    den = set(DURUM.get("denetim", []))
    sonuc = [a for a in adlar if rol(a) == "havuz" and a not in den
             and Path(a).stem not in kume and Path(a).stem in mevcut]
    _MEO.update(anahtar=anahtar, sonuc=sonuc)
    return sonuc


def model_etiketi_yaz(ad, dizin, esik, nc):
    """Bir kare için son modelin önerilerini etiket dosyası olarak yazar. Öneri yoksa False döner."""
    veri = O.json_oku(Path(dizin) / "kareler" / (Path(ad).stem + ".json"))
    if not veri or not veri.get("w"):
        return False
    W, H = veri["w"], veri["h"]
    satir = [f"{int(k[4])} {(k[0] + k[2]) / 2 / W:.6f} {(k[1] + k[3]) / 2 / H:.6f} {(k[2] - k[0]) / W:.6f} "
             f"{(k[3] - k[1]) / H:.6f}" for k in oneri_kutulari(veri, ayarlar_oku())
             if k[5] >= esik and 0 <= k[4] < nc and k[2] > k[0] and k[3] > k[1]]
    O.metin_yaz(label_path(ad), "\n".join(satir) + ("\n" if satir else ""))
    etiket_degisti()
    return True


def oneri_kutulari(veri, ayar):
    """Bir karenin model kutuları. v3.5 modellerinin tahminleri temizlenmeden kaydedilmişti; onlara grup kutusu
    filtresi ve sınıf başına NMS burada uygulanır (yeniden eğitmeden düzelir). v3.6 tahminleri zaten temizdir."""
    k = veri.get("k", [])
    if veri.get("temiz"):
        return k
    return O.kutu_listesi_temizle(k, nms_iou=float(ayar["oneri_nms"]), grup=bool(ayar["grup_filtre"]))


def m_etiketlerini_yenile():
    """Kontrol edilmemiş M karelerini son modelin önerileri ve güncel eşikle yeniden yazar."""
    t = DURUM.get("tahmin")
    if not t:
        return 0
    esik, nc, kume = ayarlar_oku()["conf_on_etiket"], len(read_classes()), etiketli_kume()
    return sum(1 for ad in DURUM.get("model_etiket", [])
               if Path(ad).stem in kume and model_etiketi_yaz(ad, t["dizin"], esik, nc))


@app.post("/api/model_etiketle")
def model_etiketle():
    """'Etiketleyeyim mi?' sorusuna evet: son modelin önerilerini etiket dosyası olarak yazar. Bu kareler
    'M' (kontrol edilmedi) olarak işaretlenir; kullanıcı açıp D ile onaylayana kadar eğitimde kullanılmaz."""
    with KILIT:
        t = DURUM.get("tahmin")
        if not t:
            return jsonify(error="Önce bir modelin eğitilmesi gerekiyor."), 400
        ayar = ayarlar_oku()
        nc = len(read_classes())
        if (request.get_json(silent=True) or {}).get("yenile"):
            sayi = m_etiketlerini_yenile()
            return jsonify(ok=True, sayi=sayi, yenilendi=True)
        yazilan = [ad for ad in model_etiketlenebilir() if model_etiketi_yaz(ad, t["dizin"], ayar["conf_on_etiket"], nc)]
        DURUM["model_etiket"] = list(dict.fromkeys(DURUM.get("model_etiket", []) + yazilan))
        durum_yaz()
    return jsonify(ok=True, sayi=len(yazilan))


@app.post("/api/test_seti")
def test_seti_sec():
    """Kullanıcı test setinin kaç kare olacağını ve nereden seçileceğini belirler; onaylanınca sabitlenir."""
    veri = request.get_json(force=True) or {}
    with KILIT:
        if basarili_turlar() and not veri.get("sifirla"):
            return jsonify(error="Test seti kilitli. Değiştirmek için 'öğrenme eğrisini sıfırla' kutusunu işaretle."), 400
        kaynak = "tum" if veri.get("kaynak") == "son" else veri.get("kaynak")
        yerlesim = veri.get("yerlesim", "son")
        if kaynak not in ("etiketli", "tum") or yerlesim not in ("son", "bas", "bloklu"):
            return jsonify(error="Geçersiz seçim."), 400
        try:
            tampon = int(veri.get("tampon", ayarlar_oku()["test_tampon"]))
        except (TypeError, ValueError):
            return jsonify(error="Tampon bir sayı olmalı."), 400
        if not 0 <= tampon <= 1000:
            return jsonify(error="Tampon 0 ile 1000 arasında olmalı."), 400
        try:
            sayi = int(veri.get("sayi"))
        except (TypeError, ValueError):
            return jsonify(error="Kare sayısı bir sayı olmalı."), 400
        kume = etiketli_kume() - model_etiket_kokleri()
        etiketli = sum(1 for a in goruntu_adlari() if Path(a).stem in kume)
        ust = etiketli - MIN_ETIKETLI if kaynak == "etiketli" else len(goruntu_adlari()) - 1
        if sayi < 0 or sayi > max(0, ust):
            return jsonify(error=f"Kare sayısı 0 ile {max(0, ust)} arasında olmalı"
                                 + (f" (eğitime en az {MIN_ETIKETLI} etiketli kare kalmalı)." if kaynak == "etiketli" else ".")), 400
        DURUM["test_secim"] = {"sayi": sayi, "kaynak": kaynak, "yerlesim": yerlesim, "tampon": tampon}
        if basarili_turlar():  # sıfırlama: eski ölçümler yeni test setiyle karşılaştırılamaz
            for t in DURUM.get("turlar", []):
                if t.get("durum") == "tamam":
                    t["eski_test"] = True
            if DURUM.get("oto_beklet") == "plato":
                DURUM["oto_beklet"] = None
        test_seti_kur(ayarlar_oku())
        DURUM["test_onay"] = True
        durum_yaz()
        return jsonify(ok=True, test=len(DURUM["test"]), tampon=len(DURUM["tampon"]))


@app.post("/api/oto_devam")
def oto_devam():
    with KILIT:
        DURUM["oto_beklet"] = None
        DURUM["son_hata"] = None
        durum_yaz()
    return jsonify(ok=True)


@app.get("/api/egri.csv")
def egri_csv():
    alanlar = [("tur", "tur"), ("durum", "durum"), ("secim", "secim"), ("model", "model"), ("sicak", "sicak_baslangic"),
               ("etiketli", "etiketli_kare"), ("egitim_kare", "egitim_kare"), ("dogrulama_kare", "dogrulama_kare"),
               ("olculen_test", "test_kare"), ("epoch_yapilan", "epoch"), ("map50", "mAP50"),
               ("map5095", "mAP50_95"), ("p", "precision"), ("r", "recall"), ("val_map50", "val_mAP50"),
               ("sure_sn", "sure_sn")]
    buf = io.StringIO()
    w = csv.writer(buf, delimiter=";")
    w.writerow([b for _, b in alanlar])
    with KILIT:
        for t in DURUM.get("turlar", []):
            satir = []
            for a, _ in alanlar:
                v = t.get(a)
                satir.append(f"{v:.4f}".replace(".", ",") if isinstance(v, float) else ("" if v is None else v))
            w.writerow(satir)
    return Response("\ufeff" + buf.getvalue(), mimetype="text/csv; charset=utf-8",
                    headers={"Content-Disposition": "attachment; filename=ogrenme_egrisi.csv"})


# ================================================================ başlatma
def main():
    ap = argparse.ArgumentParser(description="Model destekli YOLO etiketleyici (sürüm 3.6)")
    ap.add_argument("--images", required=True, help="Görüntülerin bulunduğu klasör")
    ap.add_argument("--labels", help="Etiket klasörü (varsayılan: resimlerle aynı klasör)")
    ap.add_argument("--classes", help="Sınıf listesi dosyası (varsayılan: etiket klasöründeki classes.txt)")
    ap.add_argument("--calisma", help="Modellerin ve tahminlerin tutulacağı klasör (varsayılan: <labels>_calisma)")
    ap.add_argument("--port", type=int, default=5000)
    ap.add_argument("--device", help="Örn. 0 (ekran kartı) veya cpu. Boş = otomatik")
    ap.add_argument("--sam", help=argparse.SUPPRESS)   # eski komutlar bozulmasın diye kabul edilir
    ap.add_argument("--crop", help=argparse.SUPPRESS)
    a = ap.parse_args()

    images = Path(a.images).resolve()
    if not images.is_dir():
        raise SystemExit(f"Klasör bulunamadı: {images}")
    labels = Path(a.labels).resolve() if a.labels else images  # varsayılan: resimlerle aynı klasör
    labels.mkdir(parents=True, exist_ok=True)
    cal = Path(a.calisma).resolve() if a.calisma else labels.parent / (labels.name + "_calisma")
    cal.mkdir(parents=True, exist_ok=True)
    CFG.update(images=images, labels=labels, calisma=cal,
               classes=Path(a.classes).resolve() if a.classes else labels / "classes.txt")

    if a.device is not None:
        kayitli = O.json_oku(calisma("ayarlar.json"), {}) or {}
        kayitli["cihaz"] = a.device
        O.json_yaz(calisma("ayarlar.json"), kayitli)

    DURUM.update(O.json_oku(calisma("durum.json"), {}) or {})
    DURUM.setdefault("turlar", [])
    DURUM.setdefault("sonraki_tur", len(DURUM["turlar"]) + 1)
    if "test" not in DURUM:
        test_seti_kur(ayarlar_oku())
    roller_guncelle()
    xml_iceri_aktar()
    for f in (calisma("iptal.bayrak"), calisma("is_durumu.json")):
        f.unlink(missing_ok=True)
    yarim = DURUM.get("aktif_is")
    if yarim:  # uygulama eğitim sürerken kapanmış
        s = O.json_oku(calisma("turlar", f"tur_{yarim['tur']:03d}", "sonuc.json"))
        tur_sonucunu_isle(yarim["tur"], s or {"durum": "iptal", "mesaj": "Uygulama kapandığı için tur yarıda kaldı."})
    durum_yaz()

    if a.sam or a.crop:
        print("[Bilgi] SAM kaldırıldı; --sam ve --crop yok sayılıyor.")
    s = sayimlar()
    print(f"Görüntüler : {images}  ({s['toplam']} kare)")
    print(f"Etiketler  : {labels}")
    print(f"Sınıflar   : {CFG['classes']}  -> {read_classes()}")
    print(f"Çalışma    : {cal}")
    print(f"Test seti  : {s['test']} kare ({s['test_etiketli']} etiketli), tampon {s['tampon']} kare")
    threading.Thread(target=izleyici, daemon=True).start()
    print(f"\nTarayıcıda aç: http://127.0.0.1:{a.port}\n")
    app.run(host="127.0.0.1", port=a.port, debug=False, threaded=True)


if __name__ == "__main__":
    main()
