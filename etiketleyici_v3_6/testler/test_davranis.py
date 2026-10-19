"""davranis.py birim testleri. Çalıştırma (proje klasöründe): python -m pytest testler -q"""
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import davranis as D  # noqa: E402


def kutu(c, cx, cy, w, h, m=None):
    k = {"cls": c, "cx": cx, "cy": cy, "w": w, "h": h}
    if m is not None:
        k["m"] = m
    return k


def test_degismeyen_kutu_onay_sayilir():
    f = D.kutu_farki([kutu(0, .5, .5, .2, .2, .8)], [kutu(0, .5, .5, .2, .2)])
    assert (f["onay"], f["duzelt"], f["sinif"], f["sil"], f["yeni"]) == (1, 0, 0, 0, 0)
    assert f["kutular"][0]["m"] == 0.8 and f["kutular"][0]["iou"] == 1.0


def test_kaydirilan_kutu_duzelt_sayilir():
    # IoU ≈ 0,82: aynı nesne (≥ 0,5) ama geometrisi değişmiş (< 0,95)
    f = D.kutu_farki([kutu(0, .5, .5, .2, .2)], [kutu(0, .52, .5, .2, .2)])
    assert f["duzelt"] == 1 and f["onay"] == 0


def test_sinif_degisimi_ayri_olaydir():
    f = D.kutu_farki([kutu(0, .5, .5, .2, .2)], [kutu(1, .5, .5, .2, .2)])
    assert f["sinif"] == 1 and f["kutular"][0]["yeni_c"] == 1


def test_silinen_ve_yeni_eklenen():
    f = D.kutu_farki([kutu(0, .2, .2, .1, .1)], [kutu(1, .8, .8, .1, .1)])
    assert f["sil"] == 1 and f["yeni"] == 1
    assert f["yeniler"][0]["c"] == 1


def test_acgozlu_eslestirme_her_kutuyu_bir_kez_kullanir():
    ilk = [kutu(0, .5, .5, .2, .2), kutu(0, .51, .5, .2, .2)]   # aynı nesneye iki öneri
    son = [kutu(0, .5, .5, .2, .2)]                            # kullanıcı kopyayı sildi
    f = D.kutu_farki(ilk, son)
    assert f["onay"] == 1 and f["sil"] == 1


def test_bozuk_girdi_cokertmez():
    f = D.kutu_farki("bozuk", [{"cls": "x", "cx": "nan", "cy": None, "w": 0, "h": 1}, 5])
    assert f["yeni"] == 0 and f["kutular"] == []
    assert D.kutu_farki(None, None)["sil"] == 0


def test_kayit_olustur_alanlari_sinirlar():
    k = D.kayit_olustur("a.jpg", "talha", {"oturum": "x" * 500, "sure_sn": 1e9, "islem": {"eklenen": -3, "silinen": "2"},
                                           "kaynak": "oneri", "tahmin_tur": "4", "ilk": [kutu(0, .5, .5, .2, .2, .9)]},
                        [kutu(0, .5, .5, .2, .2)], zaman=100)
    assert len(k["oturum"]) == D.METIN_SINIRI
    assert k["sure_sn"] == D.EN_COK_SURE_SN
    assert k["islem"]["eklenen"] == 0 and k["islem"]["silinen"] == 2 and k["islem"]["geri_al"] == 0
    assert k["tahmin_tur"] == 4 and k["fark"]["onay"] == 1
    json.dumps(k)   # JSON'a yazılabilir olmalı (numpy türü sızmamalı)


def test_gunluk_ekler_okur_ve_bozuk_satiri_atlar(tmp_path):
    g = D.Gunluk(tmp_path / "davranis.jsonl")
    assert g.kayitlar() == []
    g.ekle({"a": 1})
    with open(g.yol, "a", encoding="utf-8") as f:
        f.write('{"yarim": ')   # yarıda kalmış satır
    g.ekle({"a": 2})
    assert [k.get("a") for k in g.kayitlar()] == [1, 2]


def test_sinif_ozeti_yumusatir_ve_guvenilirligi_bildirir():
    kayitlar = []
    for _ in range(3):   # sınıf 1: 3 model kutusu, hepsi silindi
        kayitlar.append(D.kayit_olustur("a", "u", {"kaynak": "oneri", "ilk": [kutu(1, .5, .5, .2, .2, .3)]}, []))
    for _ in range(30):  # sınıf 0: 30 model kutusu, hepsi onaylandı
        kayitlar.append(D.kayit_olustur("b", "u", {"kaynak": "oneri", "ilk": [kutu(0, .5, .5, .2, .2, .9)]},
                                        [kutu(0, .5, .5, .2, .2)]))
    kayitlar.append(D.kayit_olustur("c", "u", {"kaynak": "elle", "ilk": [kutu(0, .5, .5, .2, .2)]}, []))   # elle: sayılmaz
    oz = D.sinif_ozeti(kayitlar)
    assert oz["1"]["model_kutu"] == 3 and oz["1"]["sil"] == 3
    assert 0 < oz["1"]["oran"]["sil"] < 1          # 3 örnekte %100'e atlamaz, ortak orana çekilir
    assert oz["1"]["guvenilir"] is False and oz["0"]["guvenilir"] is True
    assert oz["0"]["model_kutu"] == 30


def test_csv_basligi_ve_satiri():
    k = D.kayit_olustur("a.jpg", "u", {"sure_sn": 2.5, "ilk": []}, [kutu(0, .5, .5, .2, .2)], zaman=0)
    metin = D.csv_metni([k])
    satirlar = metin.lstrip("﻿").splitlines()
    assert satirlar[0].split(";") == list(D.CSV_ALANLARI)
    assert "2,5" in satirlar[1]
