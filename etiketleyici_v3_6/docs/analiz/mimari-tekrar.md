# Mimari, kod tekrarı, magic number ve global durum denetimi (etiketleyici_v3_6), doğrulanmış sürüm

Hiçbir dosyayı değiştirmedim. Satır numaraları HEAD'e göre (`56f2a51`, "Faz 1: ... davranis kaydi"). Çalışma ağacı temiz (`git status` → "nothing to commit, working tree clean"). `davranis.py` ve `testler/test_davranis.py` bu commit'te git'e eklenmiş.

## 0. Mimari kısaca
- **Sunucu:** Flask `threaded=True` ile çalışıyor (app.py:1296). Ayrıca `izleyici` iş parçacığı var (app.py:628-639, başlatma 1294). Her turda `KILIT` altında beş adımı çalıştırıyor, sonra 2 sn uyuyor: xml aktarımı, test seti kontrolü, `is_kontrol`, `analiz_kontrol`, `oto_tetik` (631-636, 639).
- **Eğitim ve analiz işçisi:** `egitim.py`, `subprocess.Popen` ile ayrı süreç olarak başlıyor (app.py:438 `--is`, app.py:976 `--analiz`).
- **Süreçler arası iletişim:** Dosyalar ve çıkış kodu üzerinden yürüyor. Kod 527 ve 940'ta okunuyor; egitim.py:472 `sys.exit`.
  - Eğitim: `is.json`, `is_durumu.json`, `iptal.bayrak`, `sonuc.json` (app.py:423-433; egitim.py:118-130, 470).
  - Analiz: `analiz_is.json`, `analiz_durumu.json`, `analiz_iptal.bayrak`, `analiz_sonuc.json` (app.py:964-971; egitim.py:470).
  - Sonuçlar: `ozet.json`, `kareler/*.json`, `test_tahmin.json`.
  - **Analiz işi iptal edilemiyor.** `analiz_iptal.bayrak` yolu işçiye veriliyor (968) ama sunucu bu dosyayı hiçbir yerde yazmıyor, yalnızca siliyor (964). `is_kontrol`'deki 30 sn'lik zorla kapatmanın (516) analiz tarafında karşılığı yok (931-942). Analiz sürdükçe `tur_baslat` eğitimi reddediyor (388-389).
- **Arayüz:** `/api/durum` adresini bir istek bittikten 2 sn sonra yeniden yokluyor (index.html:1215; ilk çağrılar 535, 541). Kılavuz sayfası ayrıca 2000 ms ve 1500 ms ile yokluyor (kilavuz.html:241, 252).
- **Kalıcı durum:** `durum.json` (app.py:235-236, okuma 1271). Bunun yanında `ayarlar.json`, `emek.jsonl` ve `davranis.jsonl` da var.

## 1. Ayarlar

**Tanım yeri**
- `AYAR_TANIM` (app.py:56-85) tuple olarak tutuluyor:
  - `(tür, varsayılan, min, max)`: int ve float için.
  - `(tür, varsayılan, seçenekler | None)`: secim için.
  - `(tür, varsayılan)`: bool ve str için.
- Model seçenekleri dinamik: değer `None` gelince `model_secenekleri()` çağrılıyor (app.py:173-175, 218).
- Görünen adlar ayrı bir sözlükte: `AYAR_ADI` (app.py:86-96). Bu adlar formdaki etiketlerle uyuşmuyor ve sunucu hata mesajları `AYAR_ADI`'yı kullanıyor (200, 208, 213, 222):

  | Ayar | `AYAR_ADI` | Form etiketi |
  |---|---|---|
  | `secim` | "Seçim yöntemi" | "Öneri sırası" (index.html:1502) |
  | `workers` | "Workers" | "Veri yükleyici (workers)" (1510) |
  | `ortusme` | "Örtüşme" | "Parça örtüşmesi" (1514) |

- Açıklamalar, min/max, seçenekler ve gruplar yalnızca arayüzde: `AYAR_FORM` (index.html:1486-1537). Alanları `k, t, l, h, d, min, max, o`. Türler sunucudakinden farklı adlandırılmış: sunucuda `secim`, arayüzde `sel` / `model`.
- Varsayılan değerler yardım metninde düz yazı olarak tekrarlanıyor. Örnekler: "Öneri: 100" (1497) ↔ app.py:61, "Öneri: 0,7" (1521) ↔ app.py:75, "1.000 karede 100" (1491) ↔ `dinamik_varsayilan` formülü.

**Tek bir şema yok; üç yerde parçalı ve uyumsuz**
- Beş ayar formda hiç yok: `cihaz`, `conf_aday`, `dikey_cevirme`, `test_orani`, `test_tampon`.
  - Bu yüzden "Ayarları kopyala" çıktısında da görünmüyorlar, çünkü `ayarMetni` yalnızca `AYAR_FORM` üzerinde dönüyor (index.html:1588-1591).
  - Değiştirme yolları:
    - Ayar komutu: `k in bilinen` kontrolü (index.html:1621).
    - `cihaz` için `--device`.
    - `test_tampon` ve `test_orani` için dolaylı yol olarak test seti diyaloğu (tTampon/tSayi: index.html:380, 1453, 1471-1472). Ancak bu değerler `DURUM["test_secim"]` içine yazılıyor (app.py:1197) ve `ayar["test_tampon"]` değerini geçersiz kılıyor (app.py:267). Yani tampon için **iki ayrı doğruluk kaynağı** var.
- Formdaki sınırlar eksik:
  - Yalnızca `min` verilmiş, `max` olmayanlar:
    - `ilk_egitim_esigi` (1490; sunucuda 10 ile 1000000 arası)
    - `tur_buyuklugu` (1492)
    - `ilk_epoch`, `tur_epoch` (1496, 1498; sunucuda max 3000)
    - `patience` (1506; max 3000)
    - `batch` (1508; max 512)
    - `workers` (1510; max 32)
    - `plato_tur` (1528; max 100)
  - Form `novalidate` olduğu için (index.html:348) arayüzdeki min/max zaten hiçbir şeyi engellemiyor. Tek gerçek doğrulayıcı sunucu.
- Seçenekler iki yerde ayrı ayrı yazılmış:
  - `parca`: app.py:65 ↔ index.html:1512
  - `secim`: app.py:74 ↔ index.html:1502

**Doğrulama: `ayar_dogrula` (app.py:194-232)**
- Bilinmeyen anahtarlar sessizce atlanıyor (198).
- bool değerler metinden çözülüyor (204). `"AÇIK".lower()` sonucu `"açik"` olduğu için tanınmıyor.
- int değerler `int(float(v))` ile kesilerek alınıyor: 2.9 değeri 2 olur (206).
- **`"1e400"` gibi bir değer `OverflowError` veriyor** (206 ve 220). Bu hata `except (TypeError, ValueError)` (224) tarafından yakalanmıyor ve istek 500 ile düşüyor. `python3 -c "int(float('1e400'))"` ile doğrulandı.
- float için NaN kontrolü var (212).
- str değerler 40 karaktere kırpılıyor (216).
- secim için, seçenekler int ise değer int'e çevriliyor (219-220).
- İki alan arası kural yalnızca iki tane: `batch != 0` (228) ve `conf_aday <= conf_on_etiket` (230). `agirlik_u + agirlik_t` denetlenmiyor; ikisi de 0 olabilir ve bu durumda bütün skorlar 0 olur.
- Alanlar arası kontroller, diskten doğrulanmadan okunan değerlerle birleşmiş sözlük üzerinde çalışıyor (195).

**Doğrulanmayan yollar**
- `ayarlar_oku` diskteki `ayarlar.json` dosyasını yalnızca anahtar filtresinden geçiriyor (app.py:186-191).
- `--device` seçeneği `cihaz` değerini doğrulamadan, 40 karakter sınırı olmadan yazıyor. Diskteki ham sözlüğü de olduğu gibi geri yazıyor (app.py:1266-1269).

**Dinamik varsayılanlar: `dinamik_varsayilan` (app.py:179-183)**
- `ilk_egitim_esigi`: 500 ve üzeri karede 100, aksi halde `max(MIN_ETIKETLI, round(n*0.1))`.
- `tur_buyuklugu`: `max(5, round(n*0.05))`.
- Varsayılan sözlüğü iki yerde aynı şekilde kuruluyor: app.py:187-188 ve 1068-1069.
- Kayıtta yalnızca varsayılandan farklı değerler saklanıyor (1070). Kullanıcı bilerek o anki dinamik değeri girerse bu değer saklanmıyor ve resim sayısı değişince kayıyor.

## 2. Magic number'lar

| # | Yer | Değer | Anlamı |
|---|---|---|---|
| 1 | app.py:182-183 | 100, 500, 0.1, 5, 0.05 | Dinamik eşik ve tur büyüklüğü |
| 2 | app.py:274 (+docstring 258) + index.html:1449 | 200 | Test seti "etiketlilerden" seçilsin eşiği |
| 3 | app.py:244 + index.html:376, 1431, 1439 (metin) | blok=4 | Bloklu test seçimi |
| 4 | app.py:406-407 | 2 | En az doğrulama karesi |
| 5 | app.py:516 | 30 sn | İptalden sonra zorla kapatma (yalnızca eğitimde) |
| 6 | app.py:602 | 5 | XML içe aktarımda otomatik eğitimi bekletme eşiği |
| 7 | app.py:639; index.html:535, 541, 1215; kilavuz.html:241, 252 | 2 sn / 2000 ms / 1500 ms | Yoklama aralıkları |
| 8 | app.py:106, 143, 829 | 3 sn, 2.0 sn, 10 sn | Önbellek ömürleri (resim listesi, etiket kümesi, tarama) |
| 9 | app.py:1185-1186 ↔ app.py:82 | 1000 ↔ 1000000 | `test_tampon` sınırı iki yerde çelişkili |
| 10 | app.py:472 | `"surum": "3.6"` | Tur kaydına gömülü sürüm. Uygulama 3.7 (app.py:2, 1242; index.html:6, 217; OKUBENI.md:1). kilavuz.html:6 başlığı da hâlâ "v3.6" |
| 11 | app.py:782 (+765) | 50 | Emek özetinde kullanılan son kayıt sayısı |
| 12 | app.py:502 | 25 | Hata durumunda gösterilen log satırı |
| 13 | app.py:1026 | 16 | Analiz önbelleği üst sınırı |
| 14 | egitim.py:160 | 2 px, 0.25 | Parça etiketinde görünürlük eşiği |
| 15 | egitim.py:237, 256, 258 | 8, 3 | CPU / bellek hatasında batch, yeniden deneme sayısı |
| 16 | egitim.py:249-250 | seed=0, `min(10, E//2)` | close_mosaic |
| 17 | egitim.py:300 | iou=0.7 | Model içi NMS |
| 18 | egitim.py:315-316 | 2 px | Parça kenarında sayılma toleransı |
| 19 | egitim.py:343, 381 | conf=0.01 (yalnızca test, 343), max_det=100 (her ikisi) | Test ve aday tahmini |
| 20 | egitim.py:383 | 500 | Kare başına en çok kutu |
| 21 | egitim.py:415 | 16 / 4 | Tahmin batch'i (GPU / CPU) |
| 22 | ortak.py:248 | 0.5, 0.85, 0.5, 0.8 | Parça birleştirme eşikleri |
| 23 | ortak.py:504, 525 | 0.25, 0.8, 0.7, 0.3, 0.5, `0.5*s[i]` | Grup kutusu filtresi |
| 24 | ortak.py:563, 573 | en_cok=3, IoU 0.5 | Kararsızlık ve titreme |
| 25 | index.html:1381-1382 ve 1391-1392 | p<0.5, r<0.7 | "Kısaca" önerileri (iki kez yazılmış) |

Ek olarak:
- ortak.py:47, 52: 40 deneme × 0.05 sn.
- index.html:467 ve 732: aday gizleme IoU 0.5 (iki kez).
- index.html:1013: tıklama eşiği 4 px; `DRAG_MIN=3` (index.html:399) ile tutarsız.
- index.html:1055: zoom katsayıları 0.0015, 0.05, 40.
- index.html:431: geri alma sınırı 100.
- egitim.py:50: sabit indirme bağlantısı `v8.4.0/yolo11n.pt`.
- egitim.py:122: 1.0 sn durum yazma aralığı.
- egitim.py:200: doğrulama yoksa `//10` ödünç alma.
- egitim.py:278, 280: 2 iş parçacığı, 3 kare önden okuma.
- ortak.py:99: JPEG kalitesi 95.
- index.html:1352: tur tablosunda son 30 tur.
- index.html:1442, 1444, 1462: %10 ve %20 test önerisi; `test_orani` 0.10 (app.py:81) ile tekrar.
- app.py:216'daki 40 karakter ↔ davranis.py:38 `METIN_SINIRI = 40`.

Olumlu karşıt örnek: davranis.py:33-49 ve index.html:398-401 sabitleri adlandırıp açıklamışlar.

## 3. Tekrarlanan kod

| Mantık | Yerler |
|---|---|
| Alt süreç başlatma (log aç, env, PYTHONUTF8, Popen) | app.py:434-442 ↔ 972-980 |
| Süreç bitti mi, log kapat | app.py:511-529 ↔ 931-942 |
| Tur dizini `tur_{:03d}` | Satır içi: app.py:421, 474, 525, 1282. `_tur_dizini` yardımcısı (918-919) **var ve kullanılıyor**, ama yalnızca analiz kodunda (939, 962, 987, 1044) |
| "Onaylı etiketli küme" (`etiketli_kume() - model_etiket_kokleri()`) | app.py:271, 317, 395, 957, 1191 |
| Test karelerinden etiketli olanları süzme | app.py:410 ↔ 958 |
| Piksel → YOLO satırı | app.py:593, 1124-1126; egitim.py:161-163; index.html:924-926 (`yoloKutu`) |
| cxcywh → xyxy | ortak.py:139-140 (piksel); index.html:586-588 (piksel); davranis.py:79 (normalize, 0-1) |
| Etiket yolu | app.py:124-125 ↔ 400 ↔ 594 (satır içi) ↔ egitim.py:135-136 |
| `"\n".join(x) + ("\n" if x else "")` | app.py:594, 713, 1127; egitim.py:181 (benzeri 202) |
| `oneri_nms` / `grup_filtre` ile temizleme çağrısı | app.py:1014, 1138; egitim.py:332-333 |
| Kutuyu yuvarlanmış listeye çevirme | egitim.py:345-347 (1 ondalık), 384-385 (tamsayı); ortak.py:559-560 |
| `ozet.json` okuma | app.py:364, 479, 864 |
| CSV dışa aktarım (`;`, ondalık virgül, BOM) | app.py:795-806 ↔ 1219-1237 ↔ davranis.py:223-235. Ondalıklar üç farklı biçimde: `str().replace`, `:.4f`, `str().replace` |
| Ayar varsayılan sözlüğü | app.py:187-188 ↔ 1068-1069 |
| Ayar sınırları, seçenekleri, adları | app.py:56-96 ↔ index.html:1486-1537 |
| Eksik ayar için varsayılan | egitim.py:332-333, 390 (`.get(..., 0.5/True)`) ↔ app.py:83-84, 77 |
| IoU / kesişim hesabı | ortak.py:233-245, 282-287, 522-524; egitim.py:154-159; index.html:445-450 |
| Açgözlü eşleştirme (3 değişken) | ortak.py:349-356, 469-475; davranis.py:86-98 |
| PR zarfı + 101 noktalı interpolasyon | ortak.py:317-321 ↔ 441-443 |
| Eski "son" kaynağını "tum" olarak eşleme | app.py:268-269, 1177; index.html:1450. Sunucu bu eski değeri kendisi de üretiyor: `/api/durum` içinde `test_kaynak: ... else "son"` (app.py:905) |
| İki ayrı günlük (emek ve davranış, aynı sayaçlar) | app.py:715-724; index.html:552-557 |
| Arayüzde "yenile" dizisi (`bilgiYukle/kuyrukYukle/listeKur/durumAl`) | index.html:1300, 1319, 1474, 1628, 1645-1648 |
| Kaydet, sonra geç (try/catch) | index.html:570 ↔ 676 (benzeri 1128) |
| `torch.cuda.empty_cache` | egitim.py:260-261, 271-272, 307 (307'de `is_available` kontrolü yok) |
| Arayüz yardımcıları (`$`, `fmt`, `yuzde`, `esc`) | kilavuz.html:198-202 ↔ index.html:395, 440, 442, 1367 |
| Aynı istekte tekrarlanan `ayarlar_oku()` (her biri diskten okuyor) | `model_etiketi_yaz` her kare için (1125); bu yüzden `model_etiketle` N kare için N kez okuyor. Ayrıca `test_seti_sec` 1182 + 1204; `m_etiketlerini_yenile` 1146 |

**Ölü kod**
- `_ETK["surum"]` artırılıyor (app.py:150) ama hiçbir yerde okunmuyor; tek geçtiği yerler 47 ve 150. Oysa `_MEO` önbellek anahtarı için doğru adaydı (bkz. §4).
- `deger()` içindeki regex aynı metni geri yazıyor, yani hiçbir şey yapmıyor (index.html:1582).
- `/api/analiz` yanıtındaki `tur_bilgi` (app.py:1053), içindeki `surum` dahil, kilavuz.html tarafından hiç okunmuyor. Sayfa yalnızca `hazir, tur, metrik, siniflar, conf, son_tur, analiz` alanlarını kullanıyor (kilavuz.html:212-271).
- `analiz_iptal.bayrak` hiçbir zaman yazılmıyor (bkz. §0).
- `ortak.degerlendir` fonksiyonunun `ayrinti=True` dalı (ortak.py:415-417) kullanılmıyor. egitim.py:358 ve 364 bu parametre olmadan çağırıyor; app.py ise aynı işi `olcutler` ve `karisiklik` ile yeniden yazıyor (1018-1023).

## 4. Global durum ve thread güvenliği

**Değişkenler (app.py modül seviyesi)**
- `CFG` (41): yalnızca `main` içinde dolduruluyor (1261-1264). `davranis` nesnesinin kendi kilidi var (davranis.py:155).
- `KILIT` (42): `RLock`.
- `DURUM` (43): `durum.json` dosyasının bellekteki kopyası.
- `ROL` (44), `IS` (45), `ANALIZ` (46). `IS` ve `ANALIZ` her zaman kilit altında değişiyor.
- Önbellekler: `_ETK` (47), `_MEO` (48), `_EMEK` (49), `_ANALIZ_ONB` (50), `_ONB` (51), `_XML_HATALI` (549), `_TARAMA` (809), `_DETAY` (858).
- `durum_yaz` çağrılarının hepsi kilit altında ya da izleyici başlamadan önce `main` içinde.

**Arayüz tarafı**
- `S` nesnesi (index.html:407-414).
- Sonradan, tanımsız olarak eklenen alanlar: `S.cizBekliyor` (920), `S.ustBekliyor` (921), `S.testSurum` (1195-1198), `S.testSoruldu` (1312), `S.testSonraEgit` (1448). Fonksiyon özelliği olarak tutulan durum: `toast.h` (422).
- `byName` (415).
- localStorage anahtarları: 531-532, 652, 1084-1085, 1152.

**Kilitsiz erişimler**
- **`save_labels` (699-729):** Etiket dosyası yazımı (713), `etiket_degisti()` (714) ve `emek.jsonl` eklemesi (721) kilitsiz. `davranis_kaydet` (715) kendi kilidini kullanıyor. Yalnızca `DURUM` değişikliği kilit altında (725).
- **`metin_yaz` geçici dosya çakışması:**
  - Geçici dosya adı yalnızca PID'den üretiliyor (ortak.py:45).
  - `model_etiketi_yaz` kilit altında (1127; `m_etiketlerini_yenile` üzerinden izleyiciden de çağrılıyor) ve `save_labels` kilitsiz (713) aynı etiket dosyasını yazabilir. Bu durumda iki iş parçacığı aynı `.tmp` dosyasını kullanır.
  - İkinci `os.replace` çağrısı `FileNotFoundError` alır. Bu hata yakalanmıyor (yalnızca `PermissionError` yakalanıyor, ortak.py:51) ve istek 500 ile döner.
- **`classes.txt`:** Kilit altında ama atomik olmadan yazılıyor (599). Kilitsiz okuyan yerler: `get_labels` (688), `davranis_ozet` (753) ve `read_classes` (yoksa dosyayı oluşturuyor: 131). Yarım dosya okunabilir.
- **`/api/analiz`:** `_analiz_hesapla` kilit dışında çağrılıyor (1050) ve `_ANALIZ_ONB` önbelleğini temizleyip yazıyor (1026-1028).
- **`ayar_kaydet`:** `ayar_dogrula` kilit dışında (1059). Bu çağrı `goruntu_adlari()` üzerinden `_ONB` önbelleğini değiştiriyor (112-113). Doğrulama kilitten önce yapılıp yazma kilit içinde yapıldığı için, aynı anda gelen iki ayar kaydında birinin değişikliği kaybolabilir.
- **`get_labels`:** `DURUM` kilit dışında okunuyor (694).
- **`model_etiketlenebilir` önbellek anahtarı (1104):** `id(kume)` ve `id(adlar)` kullanılıyor.
  - `_ETK["kume"]` her yeniden taramada içerik aynı olsa bile yeni bir frozenset ile değiştiriliyor (151). `_ONB["adlar"]` da her 3 sn'de yeni liste oluyor (112).
  - Sonuç 1: önbellek pratikte ~2 sn'de bir, yani `/api/durum` yoklamasıyla hemen hemen aynı sıklıkta, boşa düşüyor.
  - Sonuç 2: `_MEO` yalnızca id saklıyor, nesnenin kendisini tutmuyor. Eski nesne serbest kalınca adresi yeniden kullanılabilir; bu da yanlış önbellek isabeti riski doğuruyor.
- **`/api/info`:** Kilit tutulurken her etiketli karenin dosyasını okuyor (650-662).
- **`etiketli_kume` yarışı:** İzleyici kilit altında taramayı bitirip `zaman` değerini yeniden yazarsa (151), kilitsiz `etiket_degisti` ile yapılan geçersiz kılma (157) ezilebilir. Sonuç en fazla yaklaşık 2 saniyelik bayat veri.

## 5. Büyük fonksiyonlar (60 satır ve üzeri)
- app.py `tur_baslat`: 384-457 (74)
- ortak.py `parca_birlestir`: 248-311 (64)
- index.html `renderModel`: 1219-1362 (144). Uyarılar, emek, sayılar, adımlar, eğri ve tur tablosu hepsi bu tek fonksiyonda.
- egitim.py `Isci` sınıfı bütünüyle 96-447 (352 satır).
- Sınıra yakın olanlar:
  - app.py: `xml_iceri_aktar` 552-607 (56), `main` 1241-1296 (56), `tur_sonucunu_isle` 460-508 (49), `_analiz_hesapla` 985-1029 (45)
  - egitim.py: `parcala` 139-183 (45)
  - ortak.py: `sinif_eslesmeleri` 325-366 (42)
  - index.html: `kutuCiz` 721-773 (53), keydown işleyicisi 1111-1154 (44)

## 6. Dosya yazma ve JSON formatları

**`metin_yaz` (ortak.py:41-53)**
- Geçici dosyaya yazıp `os.replace` ile yerine koyuyor.
- Eksikleri:
  - `fsync` yok.
  - Geçici ad yalnızca PID'den oluşuyor (§4).
  - Yalnızca `PermissionError` durumunda 40 kez yeniden deniyor.
  - Hata olursa `.tmp` dosyası silinmiyor.
- `json_yaz`: `indent=1`, `ensure_ascii=False` (57).
- `goruntu_yaz`: `str(yol)+".tmp"` ile ayrıca atomik yazıyor (99-107).

**Atomik yazılanlar**
- Etiket `.txt` dosyaları: app.py:594, 713, 1127
- `durum.json`: 236
- `ayarlar.json`: 1070, 1269
- `is.json` ve `analiz_is.json`: 433, 971
- `is_durumu.json` ve `analiz_durumu.json`: egitim.py:128
- `veri.yaml` (JSON olarak): egitim.py:206
- `test_tahmin.json`: egitim.py:348
- `tahmin/kareler/*.json`: egitim.py:386
- `ozet.json`: egitim.py:404
- `sonuc.json` ve `analiz_sonuc.json`: egitim.py:470

**Atomik olmayanlar**
- `classes.txt`: app.py:131, 599
- `emek.jsonl`: kilitsiz ekleme (721)
- `iptal.bayrak`: 1093
- Log dosyaları: 434, 972
- Parça etiketleri: egitim.py:181
- `egitim.txt` ve `dogrulama.txt`: egitim.py:202
- `davranis.jsonl`: ekleme ile yazılıyor ama kilitli ve yarım kalan satırı onarıyor (davranis.py:158-167)

**JSON formatları**
- **`ayarlar.json`:** yalnızca varsayılandan farklı anahtarlar (1070).
- **`durum.json`:**
  - `turlar[]`: alanları 444-449 ve 467-473'te; `eski_test` 1201'de ekleniyor.
  - `sonraki_tur`, `test[]`, `tampon[]`, `test_surum` (298-299)
  - `test_secim{sayi,kaynak,yerlesim,tampon}` (1197), `test_onay`
  - `son_basarili{tur,agirlik,model,siniflar,parca}` (476-477), `tahmin{tur,dizin}` (478)
  - `denetim[]` (482), `kuyruk[]`, `kuyruk_skor{}` (380)
  - `son_hata{tur,mesaj,log}`
  - `oto_beklet`: plato / iptal / hata / ice_aktarma / None
  - `aktif_is{tur}`, `son_baslangic_etiketli`, `model_etiket[]`
  - `xml_aktarim{sayi,kutu,yeni_siniflar,zaman}`
- **İş dosyaları:**
  - `is.json`: 425-432
  - `analiz_is.json`: 967-970
- **İşçi çıktıları:**
  - `is_durumu.json`: egitim.py:125-126 (+ `bilgi` + `epoch/epochs`)
  - `sonuc.json`: egitim.py:447 + 461-469. Başarıda `agirlik, metrik, tahmin_kare, durum`; `mesaj` yalnızca iptal ya da hatada geliyor. Hepsine `bilgi` (`tur, cihaz, batch, val_map50, egitim_parca, dogrulama_parca, epoch_yapilan`) ve `sure_sn` ekleniyor.
  - `analiz_sonuc.json`: `{test_kare}` + `bilgi` + `sure_sn` (428).
- **Tahminler ve ölçüm:**
  - `kareler/<kök>.json`: `{w,h,k:[[x1,y1,x2,y2,cls,conf]],temiz:true}`. Temizlenmiş, tamsayı koordinat, en çok 500 kutu.
  - `ozet.json`: `{tur,kareler:{ad:{u,t,n}}}`
  - `test_tahmin.json`: `{tur,kareler:{ad:{w,h,k}}}`. Ham (conf ≥ 0.01), temizlenmemiş, 1 ondalık, `temiz` bayrağı yok.
  - `metrik`: ortak.py:395-401 + `test_kare`, `temiz`
- **`emek.jsonl` satırı:** `{ad,zaman,kutu,sure_sn,eklenen,silinen,duzeltilen,sinif_degisen,kaynak,ilk_kutu}` (718-720)
- **`davranis.jsonl` satırı (davranis.py:138-146):** `{surum, ad, zaman, kullanici, oturum, liste, kaynak, rol, tahmin_tur, sure_sn, islem{7 sayaç}, fark{onay,duzelt,sinif,sil,yeni,kutular[{c,m,w,h,sonuc,iou?,yeni_c?}],yeniler[{c,w,h}]}}`

## 7. Yeni modül için doğal entegrasyon noktaları

**`davranis.py` benzeri bir loglama modülü (zaten uygulanmış)**
- Sunucu: import (app.py:36), kaydetme bağlantısı (715), `davranis_kaydet` (732-739), uç noktalar (742-754), `--kullanici` (1249), `CFG` içinde tekil nesne (1263-1264).
- Arayüz: `OTURUM` (index.html:402), istek gövdesi (551-557), `ilkKutular` (`emekBaslat` 927-931, satır 930), CSV bağlantısı (329).

Eksik kalan bağlantılar:
- `save_labels` dışında etiket yazan yollar günlüğe düşmüyor:
  - `xml_iceri_aktar` (594)
  - `model_etiketi_yaz` (1127; `model_etiketle` 1151-1167 toplu yazım)
- Eğitim olayları: `tur_baslat` (444-449), `tur_sonucunu_isle` (467-473). Ayar değişiklikleri: `ayar_kaydet` (1057-1078). Test seti seçimi: 1170-1207.
- İstemci işlem sayaçları: `emekSay` çağrıları (index.html:893, 900, 1019, 1028, 1036, 1070, 1134) davranış kaydına **toplam olarak** giriyor (`islem`, 555-556). Zaman damgalı olay akışı yok.
- **`denetim` bilgisi gönderilmiyor.** Gövde yalnızca `rol` içeriyor (554). `get_labels` bunu döndürüyor (695) ve `S.denetim` dolu (584). Denetim kareleri `kaynak: 'elle'` olarak kaydediliyor ve ancak `DURUM["denetim"]` ile karşılaştırılarak ayırt edilebiliyor.
- **M karelerinde güven değeri kayıp.** Etiket dosyasından yüklenen kutularda `m` alanı yok (586-588). Bu yüzden `kaynak:'model'` olan karelerde `fark.kutular[].m = null`.
- `/api/davranis_ozet` arayüzde kullanılmıyor (docstring 750-751).

**`oncelik.py` gibi bir skor kayıt defteri**
1. **Modülün yeri:** torch içermeyen, `ortak.py` gibi her iki süreçten de import edilebilen bir modül olmalı. Mevcut skorlar orada: `kararsizlik` (ortak.py:563-570) ve `titreme_skorlari` (586-612).
2. **Özellik üretimi (işçi tarafı):** `egitim.py` `tahmin_et` (371-405). Kare başına `kareler[ad]` sözlüğüne (389) yeni özellik eklenir; sonuç `ozet.json` dosyasına (404) yazılır.
3. **Skorun birleştirilmesi (sunucu tarafı):** `kuyruk_kur` (app.py:357-380).
   - Formül 367-368'de; `aktif` dalı 366-369, diğer dal 370-373.
   - **`else` dalı "aktif" dışındaki her değeri rastgele sayıyor.** Yeni bir `secim` değeri eklenirse sessizce rastgele sıraya düşer. Arayüz metni de `'rastgele'` değerine sabit bağlı (index.html:877).
   - Ağırlık değişince yeniden eğitim gerekmiyor; `ayar_kaydet` (1076) ve `tur_sonucunu_isle` (483) `kuyruk_kur`'u yeniden çağırıyor.
   - **İstisna:** `titreme` sonradan açılırsa bir sonraki tura kadar etkisi yok. `t` değeri yalnızca eğitim anında `titreme` açıksa hesaplanıyor (egitim.py:390); kapalıysa 0.0 kalıyor (389).
4. **Detay ve arayüz:**
   - `kuyruk_detay` sabit sıralı `[u,t,n]` listesi döndürüyor (app.py:865).
   - `/api/kuyruk` yalnızca `agirlik{u,t}` gönderiyor (877).
   - Arayüz ipucu metni `det[0..2]` ile sıraya bağlı (index.html:859-861). Bunların sözlüğe çevrilmesi gerekir.
5. **Ayarlar:**
   - Sunucu: `AYAR_TANIM` / `AYAR_ADI` içindeki `secim` ve ağırlıklar (app.py:74-76, 91-92).
   - Arayüz: `AYAR_FORM` (index.html:1502, 1520-1525).
   - Dinamik seçenek deseni zaten var: `None` → dinamik liste (app.py:218), `/api/durum` içinde `modeller` (906), arayüzde `d.modeller` (index.html:1553).
   - Tur kaydında `secim` (447) ve `egri.csv` (1221) da etkilenir.
6. **Davranış verisiyle besleme:** sınıf düzeyindeki oranlar `D.sinif_ozeti(kayitlar)` ile alınabilir (app.py:752 ve 754).
7. **İşçiye aktarım:**
   - Ayarlar `job["ayarlar"]` ile tamamen aktarılıyor (app.py:428; analizde 969).
   - Ancak işte çalışma klasörü yolu da davranış verisi de yok (425-432). Davranıştan türeyen sınıf istatistikleri işçide kullanılacaksa yeni bir iş alanı gerekir; yoksa sunucu tarafında `kareler/*.json` üzerinden hesaplanmalı. "Ek kanal gerekmez" yalnızca ayarlar için doğru.

---

## Düzeltmeler:
1. **Git durumu yanlıştı.** Çalışma ağacı temiz; `app.py`, `index.html`, `kilavuz.html` ve `davranis.py` `56f2a51` commit'inde. "Commit'lenmemiş, davranis.py takipsiz" iddiası geçersiz.
2. **`_tur_dizini` "kullanılmıyor" iddiası yanlıştı.** app.py:939, 962, 987 ve 1044'te kullanılıyor. Doğrusu: yalnızca eski 4 yer onu kullanmıyor.
3. **index.html satır numaraları yaklaşık 800. satırdan sonra 2 satır kaymıştı.** Düzeltilenler:

   | Öğe | Analizdeki satır | Doğru satır |
   |---|---|---|
   | Yoklama döngüsü | 1217 | 1215 |
   | `AYAR_FORM` | 1488-1539 | 1486-1537 |
   | Form sınırları (ilk_egitim_esigi / batch / workers) | 1492 / 1510 / 1512 | 1490 / 1508 / 1510 |
   | Parça seçenekleri | 1514 | 1512 |
   | Seçim yöntemi seçenekleri | 1504 | 1502 |
   | `ayarMetni` döngüsü | 1590-1593 | 1588-1591 |
   | Ayar komutu `k in bilinen` | 1623 | 1621 |
   | `deger()` | 1584 | 1582 |
   | `S.cizBekliyor` / `S.ustBekliyor` | 922 / 923 | 920 / 921 |
   | `S.testSurum` / `S.testSoruldu` / `S.testSonraEgit` | 1197 / 1314 / 1450 | 1195 / 1312 / 1448 |
   | localStorage | 1086, 1154 | 1084-1085, 1152 |
   | `renderModel` | 1221-1364 | 1219-1362 |
   | keydown işleyicisi | 1113-1156 | 1111-1154 |
   | Tıklama eşiği | 1015 | 1013 |
   | Zoom katsayıları | 1057 | 1055 |
   | "Kısaca" önerileri | 1383-1384 / 1393-1394 | 1381-1382 / 1391-1392 |
   | 200 eşiği | 1451 | 1449 |
   | "son" → "tum" | 1452 | 1450 |
   | Bloklu metin | 1433 | 1431 / 1439 |
   | Piksel → YOLO | 926-928 | 924-926 |
   | `ilkKutular` | 929-933 | 927-931 |
   | `emekSay` çağrıları | 895, 902, 1021, 1030, 1038, 1072, 1136 | 893, 900, 1019, 1028, 1036, 1070, 1134 |
   | İpucu metni `det` | 861-864 | 859-861 |
   | `'rastgele'` metni | 879 | 877 |
   | `d.modeller` | 1555 | 1553 |
   | Ağırlık alanları | 1522-1527 | 1520-1525 |
   | "Yenile" dizisi | 1302, 1321, 1476, 1630, 1647-1650 | 1300, 1319, 1474, 1628, 1645-1648 |

4. **Formdaki eksik `max` listesi eksikti.** `tur_buyuklugu`, `ilk_epoch`, `tur_epoch`, `patience` ve `plato_tur` eklendi. Formun `novalidate` olduğu, yani min/max'ın hiçbir şeyi engellemediği eklendi.
5. **"Yalnızca ayar komutuyla değiştirilebilir" iddiası düzeltildi.** `test_tampon` ve `test_orani` test seti diyaloğundan da `test_secim` üzerinden etkileniyor. Bu, iki ayrı doğruluk kaynağı demek.
6. **davranis.py:79 "YOLO → piksel" değil.** Dönüşüm normalize xyxy'ye yapılıyor.
7. **`sinif_ozeti` referansı düzeltildi.** Çağrı tek satırda değil: `kayitlar` 752'de, `sinif_ozeti` 754'te.
8. **Eklenen bulgular:**
   - `ayar_dogrula` içinde yakalanmayan `OverflowError` (206/220 → 500).
   - `metin_yaz`'da `FileNotFoundError` yakalanmıyor.
   - `classes.txt` atomik olmadan yazılıp kilitsiz okunuyor.
   - `ayar_kaydet`'te kayıp güncelleme riski.
   - `_MEO` önbelleği her yeniden taramada boşa düşüyor (151, 112); ölü `_ETK["surum"]` tam da bunun anahtarı olabilirdi.
   - Analiz işi iptal edilemiyor (ölü `analiz_iptal.bayrak`, 30 sn zorla kapatma yok).
   - Kılavuzun kullanmadığı `tur_bilgi`/`surum` alanları; kullanılmayan `degerlendir(ayrinti=True)` dalı.
   - kilavuz.html başlığı "v3.6".
   - `AYAR_ADI` ile form etiketleri uyumsuz; yardım metninde varsayılanlar tekrarlanıyor.
   - Yardımcı fonksiyonlar kilavuz.html ve index.html arasında kopyalanmış.
   - Eşleştirmenin üçüncü kopyası (ortak.py:349-356).
   - Onaylı küme hesabının ek yeri app.py:317; satır içi etiket yolu app.py:594.
   - Sunucu eski `"son"` değerini kendisi üretiyor (905).
   - Aynı istekte tekrarlanan `ayarlar_oku()` disk okumaları.
   - Eksik JSON şemaları: `davranis.jsonl`, `analiz_sonuc.json`; `test_tahmin.json` ile `kareler` arasındaki biçim farkı.
   - `kuyruk_kur`'un `else` dalı her yeni `secim` değerini rastgele sayıyor.
   - `titreme` sonradan açılırsa yeniden eğitim gerekiyor.
   - İşçiye davranış verisi aktaracak bir kanal yok.
   - Davranış kaydında `denetim` bayrağı ve M karelerinin güven değeri eksik.
   - Ek magic number'lar (egitim.py:122, 200, 278-280; ortak.py:99; index.html:1352, 1442-1462; kilavuz.html:241, 252).
9. **Doğrulanan iddialar.** Diğer app.py, egitim.py ve ortak.py satır numaraları, formüller (dinamik eşikler, öncelik = `a_u*u + a_t*t`, kararsızlık, bloklu seçim) ve "kilitsiz / atomik değil" iddiaları kodla birebir uyuşuyor.