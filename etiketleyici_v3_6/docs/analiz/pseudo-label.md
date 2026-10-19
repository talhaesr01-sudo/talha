# Pseudo label (M etiketleri) ve güven mantığı: etiketleyici_v3_6 (doğrulanmış sürüm)

> **Sürüm notu.** Önceki analizdeki satır numaraları `da8f5c0` commit'ine uyuyor, yani orijinal v3.6'ya. Çalışma ağacı ise `56f2a51` commit'inde ("Faz 1"). Bu sürümde app.py kendini "sürüm 3.7" olarak tanıtıyor (app.py:2), yeni bir `davranis.py` dosyası var ve index.html büyük ölçüde değişmiş. Aşağıdaki tüm referanslar **güncel** dosyalara göredir. Hiçbir dosya değiştirilmedi.

## 1. Akış özeti

1. **Eğitim işçisi tahmin üretir** (egitim.py:371-405).
   - Girdi `job["tahmin"]`: havuzda olup etiketi olmayan ya da M olan kareler (app.py:395, 411).
   - Parçalı tahmin yapılır. Eşik `conf_aday`, varsayılanı 0,05 (app.py:73, egitim.py:381). YOLO'ya `iou=0.7` ve parça başına `max_det=100` verilir (egitim.py:300-301).
   - Sırasıyla `parca_birlestir` (egitim.py:327-328), `temizle` yani `oneri_temizle` (egitim.py:330-333, 382) ve skora göre ilk 500 kutu (egitim.py:383) uygulanır.
   - Sonuç `tur_xxx/tahmin/kareler/<kök>.json` dosyasına `{"w","h","k":[[x1,y1,x2,y2,sınıf,güven]],"temiz":True}` olarak yazılır. Koordinatlar tamsayıya, güven 3 basamağa yuvarlanır (egitim.py:384-386).
   - Kare özeti `ozet.json` içinde `{u,t,n}` olarak tutulur (egitim.py:389, 402-404).
2. **Tur sonunda sunucu şunları yapar** (app.py:475-487):
   - `DURUM["tahmin"]` son turun dizinini gösterir (478).
   - Denetim örneği seçilir (480-482).
   - Kuyruk kurulur (483).
   - M etiketleri yenilenir (485).
3. **`GET /api/labels`** (app.py:671-696):
   - `aday`: json'daki kutuların tümü, ama yalnızca `0<=sınıf<nc` olanlar (689-691).
   - `oneri`: yalnızca etiket dosyası yoksa ve güven ≥ `conf_on_etiket` ise (692-693).
   - İkisi de şu koşullarda verilir: `rol=="havuz"`, kare denetim değil, son turun json'u var (685-687).
4. **Toplu etiketleme ("Etiketleyeyim mi?"):** `POST /api/model_etiketle` (app.py:1151-1167).
5. **Arayüz (Faz 1):** Üç kategori var: MODEL ÖNERDİ / ETİKETSİZ / ONAYLANDI (index.html:404, 616-620).
   - M kareler ve kuyruktaki kareler MODEL ÖNERDİ'ye düşer (618).
   - Sıra kuyruk sırasıdır. Kuyrukta olmayan M kareler en sona eklenir (632-634).

## 2. Fonksiyonlar

### app.py

- **`model_etiketlenebilir`** (1098-1115)
  - Seçim koşulu: `rol=="havuz"`, denetim değil, etiket dosyası yok, son turun `kareler/` klasöründe json'u var (1111-1113).
  - Önbellek anahtarı: tur, dizin, `id(kume)`, `id(adlar)`, test_surum, denetim (1104).
  - Kararsızlık (u), titreme (t) ya da güven için filtre yok.
- **`model_etiketi_yaz`** (1118-1129)
  - Kutuları `oneri_kutulari` ile alır. Yalnızca `k[5]>=esik`, `0<=sınıf<nc`, `x2>x1` ve `y2>y1` olanları yazar (1124-1126).
  - Güven değeri .txt'ye yazılmaz.
  - Eşiği geçen kutu yoksa **boş dosya yazar ve True döner** (1127-1129).
  - False yalnızca json yoksa ya da `w` alanı eksikse döner (1120-1122).
- **`oneri_kutulari`** (1132-1138)
  - `temiz` True ise kutuları olduğu gibi döner (1136-1137).
  - Yalnızca eski v3.5 tahminlerine güncel `oneri_nms` ve `grup_filtre` ayarlarıyla `kutu_listesi_temizle` uygular (1138).
- **`m_etiketlerini_yenile`** (1141-1148)
  - `model_etiket` listesinde olup etiket dosyası hâlâ duran kareleri son turun json'u ve güncel `conf_on_etiket` ile yeniden yazar.
  - Çağrıldığı yerler: tur sonu (485) ve `{"yenile":true}` isteği (1161-1163).
  - Rol, denetim ya da test kontrolü yapmaz.
  - Son turda o karenin json'u yoksa (örneğin kare test ya da tampon olduğu için `tahmin` listesine girmediyse, 411) `model_etiketi_yaz` False döner. Bu durumda eski M etiketi olduğu gibi kalır.
- **`model_etiketle`** (1151-1167): Yazılan kareleri `model_etiket` listesine tekrarsız ekler ve `durum_yaz` çağırır (1164-1166).
- **`save_labels`** (699-729) ve **`davranis_kaydet`** (732-739, yeni)
  - .txt dosyasını kilit dışında yazar (713).
  - `davranis.jsonl` dosyasına kayıt ekler (715).
  - `emek.jsonl` dosyasına kayıt ekler (716-724).
  - Ardından kilit altında kareyi `model_etiket` listesinden çıkarır (725-728).

### ortak.py (iddiaların hepsi doğrulandı)

- **`parca_birlestir`** (248-311)
  - Kutular skora göre sıralanır (266).
  - Yalnızca farklı parçadan gelen aynı sınıf kutular eşleşir (280).
  - Eşleşme koşulu `IoU>=0,5` ya da `parcali` olmaktır (292-295). Parçalı sayılmak için:
    - kutular hizalı olmalı: bir eksende örtüşme / kısa kenar ≥ 0,8,
    - tek kutu kenardaysa IoS ≥ 0,85, iki kutu da kenardaysa IoS ≥ 0,5 olmalı.
  - Adaylar `iou+ios` değerine göre sıralanır ve her parçadan en fazla bir kutu birleşir (298-304).
  - Kutulardan biri kenardaysa birleşim kutusu alınır (305-307).
  - Kalan kutu en yüksek skoru taşır (309). Kenar tespiti: parçanın **iç** kenarına 2 pikselden yakın olmak (egitim.py:315-316).
- **`nms`** (484-501): Sınıf başına açgözlü NMS. IoU ≥ eşik olan düşük skorlu kutu silinir (500).
- **`grup_kutulari`** (504-535)
  - Kutu sayısı 3'ten azsa hiçbir şey yapmaz (515).
  - En az 2 aynı sınıf kutu gerekir (520).
  - İç kutu koşulu: IoS ≥ 0,8 (iç kutunun kendi alanına göre), alan ≤ 0,7×alan_i, skor ≥ max(0,25; 0,5·s_i) (524-525).
  - İç kutular skor sırasıyla açgözlü seçilir ve birbiriyle IoU < 0,3 olmalı (528-532).
  - İç kutuların toplam kesişimi alan_i'nin en az yarısı olmalı (533).
  - `ic_skor=0.25` varsayılan parametre olarak kalır. Hiçbir çağıran bu değeri geçmez (egitim.py:332-333, app.py:1014, ortak.py:558).
- **`oneri_temizle`** (538-550): Önce grup kutuları atılır, sonra NMS uygulanır. NMS yalnızca `nms_iou` doğru değerliyse (truthy) ve `<1` ise çalışır. Sonuç her yolda skor sırasıyla döner (549).
- **`kutu_listesi_temizle`** (553-560): Koordinatları tamsayıya, güveni 3 basamağa yuvarlar.
- **`kararsizlik`** (563-570): Her kutu için `u=1-|2p-1|`, kare değeri en kararsız 3 kutunun ortalaması. Kutu yoksa 0 döner.

### davranis.py (yeni, Faz 1)

- **`kutu_farki`** (101-129): Karenin açılıştaki kutularını kayıttaki kutularıyla karşılaştırır. Eşleştirme sınıftan bağımsız ve açgözlüdür, eşik IoU ≥ 0,5 (86-98, 108-109). Her açılış kutusu için sonuç:
  - aynı sınıf ve IoU ≥ 0,95 → `onay`,
  - aynı sınıf ve IoU < 0,95 → `duzelt`,
  - farklı sınıf → `sinif`,
  - eşleşmeyen → `sil`.
  - Kayıtta olup açılışta olmayan her kutu `yeni` sayılır (127-128).
- Kutunun model güveni `m` varsa saklanır (81-82, 113).
- **`sinif_ozeti`** (194-216): Sınıf başına onay / düzelt / sınıf / sil oranlarını Beta önseliyle yumuşatır (`ONSEL_GUCU=5`, `MIN_ORNEK=20`).
  - Yalnızca `/api/davranis_ozet` ile sunulur (app.py:748-754, docstring'e göre "Arayüz şimdilik göstermiyor").
  - Hiçbir karara girdi değildir.

## 3. M etiketleri: nerede, nasıl işaretli, eğitimden nasıl dışlanıyor

**Saklama**
- Etiket, kullanıcı etiketiyle aynı .txt dosyasına yazılır (`label_path`, app.py:124-125, yazım 1127). Varsayılan etiket klasörü resim klasörüdür (app.py:1257).
- M işareti yalnızca `DURUM["model_etiket"]` listesinde, yani `durum.json` dosyasında tutulur (app.py:1165-1166, 235-236).

**İşaretin kullanıldığı yerler**
- `model_etiket_kokleri()` (app.py:160-162).
- `/api/info` yanıtındaki `model` alanı (652, 662) ve `/api/labels` yanıtı (694-695).
- Arayüzde M rozeti (index.html:852), uyarı şeridi (index.html:803) ve kategori seçimi (index.html:617-618).

**Eğitimden dışlama**
- `tur_baslat` içinde `kume = etiketli_kume() - model_etiket_kokleri()` (app.py:395).
  - Eğitim ve doğrulama kümesi yalnızca bu kümeden kurulur (396-409).
  - Test kümesi de bu kümeyle süzülür (410).
- M kareler `tahmin` listesine girer (411). Bu yüzden yeni model onları yeniden tahmin eder, `ozet.json`'a ve kuyruğa girerler.
- egitim.py yalnızca `egitim` ve `dogrulama` listelerini parçalar (egitim.py:192).
- Titreme komşuları için kullanılan `etiketli_havuz` listesi M kareleri içermez (app.py:431, egitim.py:393).
- `havuz_etiketli` M kareleri saymaz (app.py:313-317). Bu yüzden M kareler eğitimi tetiklemez (333-335, 539-541).
- Şu yerler de M kareleri dışlar: test seti adayları (271), test seti üst sınırı (1191-1193), Kılavuz analizi (957).

## 4. D ile onay ve diğer kaydetme yolları

**D tuşunun zinciri**
1. D tuşu `git(+1)` çağırır. Kutu seçili değilken → tuşu da aynı işi yapar (index.html:1143; kutu seçiliyse oklar kutuyu kaydırır, 1116).
2. **Yeni:** başlıktaki "✓ Onayla · sonraki" düğmesi de aynı `git(+1)` çağrısını yapar (index.html:227, 1162).
3. `git` → `load(j, true)` (674). Liste bitince `S.dirty || S.pre` ise kaydeder (675-677). **Yeni:** ardından öteki iş listesine kendiliğinden geçer (679-685).
4. `load`: `S.dirty || (onay && S.pre)` ise `save()` çalışır (569-571).
5. `S.pre` değeri:
   - M karede `S.pre = !!lab.model` (589).
   - Etiketsiz karede `lab.tahmin_var` ise `S.pre = true` (590-592). Bu, **öneri listesi boş olsa bile** olur.

**`save()` ne yapar**
- `POST /api/labels` çağırır. Sunucu .txt dosyasını yeniden yazar (app.py:713).
- `emek.jsonl` dosyasına kare düzeyinde bir satır ekler (index.html:552-553, app.py:716-724). `kaynak` alanı M karede `model`, öneride `oneri`, etiketlide `duzenleme`, aksi halde `elle` olur (index.html:597).
- **Yeni:** `davranis.jsonl` dosyasına bir satır ekler: açılış kutuları, işlem sayaçları ve kutu kutu fark (index.html:554-558, 930; app.py:715).
- Kareyi `model_etiket` listesinden çıkarır ve `durum_yaz` çağırır (app.py:725-728).
- İstemcide `model:false` atanır, `b.m` silinir, `S.pre=false` olur (index.html:559-562).

**Sonrası**
- `etiket_degisti()` önbelleği geçersiz kılar (app.py:714).
- İzleyici iş parçacığı 2 saniyede bir `oto_tetik` çalıştırır (628-639). `sonraki_kalan==0` ise tur başlar (541-542).
- Bir sonraki turda kare, `kume_payi` değerine göre eğitim ya da doğrulama kümesine girer (405-409).

**Diğer onay yolları.** Her kaydetme M işaretini kaldırır:
- Ctrl+S (1126-1129),
- düzenlenmiş kareden listeden tıklayarak, A ile ya da kategori değiştirerek çıkmak (569),
- "Onayla · sonraki" düğmesi.

## 5. Güven eşikleri ve ayarlar

| Ayar | Varsayılan / aralık | Nerede etkili |
|---|---|---|
| `conf_on_etiket` | 0,25 / 0,01–0,99 (app.py:72) | Ekrandaki öneri (693), M yazımı (1126, 1146, 1164), `n` ve titreme varlığı (egitim.py:376, 387-389), test P/R (egitim.py:357-358, 364), Kılavuz P/R ve karışıklık (app.py:1001-1023) |
| `conf_aday` | 0,05 / 0,001–0,99 (app.py:73) | YOLO predict eşiği (egitim.py:381). `conf_aday ≤ conf_on_etiket` zorunlu (app.py:230-231) |
| `oneri_nms` | 0,5 / 0,3–0,95 (app.py:83) | egitim `temizle` (332), v3.5 tahminleri (app.py:1138), Kılavuz temiz modu (1013-1014) |
| `grup_filtre` | True (app.py:84) | Aynı yerler |
| Sabit değerler | — | YOLO `iou=0.7`, parça başına `max_det=100` (egitim.py:300, 343, 381); test eşiği `conf=0.01` (343); parça birleştirme 0,5 / 0,85 / 0,5 / 0,8 (ortak.py:248); grup filtresi 0,25 / 0,8 / 0,7 / 0,3 / 0,5 (504); ilk 500 kutu (egitim.py:383); en kararsız 3 kutu (ortak.py:563); tıkla-ekle ve C gösteriminde mevcut kutuyla IoU 0,5 (index.html:467, 732) |

**Canlılık:**
- `conf_on_etiket` canlı uygulanan yerler: ekrandaki öneri, yeni M yazımı, M yenileme ve Kılavuz.
- `conf_on_etiket` ile tur sırasında hesaplanıp dondurulan değerler: `n`, titreme ve tur kayıtlarındaki test P/R.
- `conf_aday`, `oneri_nms` ve `grup_filtre` tur başlarken iş dosyasına kopyalanır (app.py:428). Bu yüzden ancak bir **sonraki tur başlangıcında** geçerli olurlar. İstisnalar: v3.5 tahminleri ve Kılavuz temiz modu.
- `agirlik_u`, `agirlik_t`, `secim` ve `titreme` ayarları kaydedilince kuyruk hemen yeniden kurulur (app.py:1076).

## 6. Ham YOLO güveninin doğrudan karar verdiği yerler

1. Predict eşiği: tahminde `conf_aday`, testte 0,01 (egitim.py:300, 343, 381).
2. Parça birleştirmede sıralama. Kalan kutu en yüksek skoru taşır (ortak.py:266, 309).
3. Grup filtresinde iç kutu koşulu `s ≥ max(0,25; 0,5·s_i)` ve açgözlü seçim sırası (ortak.py:525, 528).
4. NMS'te hangi kutunun kalacağı (ortak.py:493, 500-501).
5. İlk 500 kutu kesimi (egitim.py:383).
6. Kararsızlık `u` (ortak.py:569-570, egitim.py:389). Hesaba **temizlenmiş ve 500 ile kesilmiş** kutuların tümü girer, yani conf_aday ve üstü.
7. Titreme varlığı ve `n` sayısı: yalnızca ≥ conf_on_etiket kutular (egitim.py:387-389).
8. Kuyruk önceliği `a_u·u + a_t·t`. Titreme kapalıysa `a_t=0` (app.py:367-368).
9. Ekranda gösterilen öneri (app.py:693).
10. M etiketi olarak yazılan kutular (app.py:1126).
11. Tıkla-ekle: tıklanan noktayı içeren ve mevcut kutularla IoU < 0,5 olan adaylardan en yüksek skorlu olan seçilir (index.html:462-471).
12. C ile gösterilen adayların saydamlığı `0,35+0,65·min(1, s/0,5)` (index.html:734).
13. Ölçüm: P/R conf_on_etiket ile kesilir (ortak.py:383). Karışıklık tablosu da öyle (464). Eşik eğrisi 0,05–0,95 arası (421, 424-449). Kılavuz her eşik için ayrı karışıklık tablosu üretir (app.py:1023).
- Güven ayrıca `davranis.jsonl` dosyasına `m` alanı olarak kaydedilir (davranis.py:82, 113). Hiçbir karar bunu kullanmaz.

## 7. Kalibrasyon ve kalite skoru

- **Kalibrasyon yok.** `.py`, `.html` ve `.md` dosyalarında `kalibr|calibr|temperature|platt|isoton` aramasının hiç sonucu yok.
  - Tüm sınıflar için tek bir genel eşik kullanılıyor.
  - Sınıf başına en iyi F1 eşiği `en_iyi_esik` hesaplanıyor (ortak.py:439, 449) ama yalnızca Kılavuz'da gösteriliyor (kilavuz.html:279, 367).
  - MC Dropout ve Ensemble için Kılavuz "v3.6'da yok" diyor (kilavuz.html:192, bölüm 170-193).
- **Kutu düzeyinde** tek kalite bilgisi ham güven `k[5]`.
- **Kare düzeyinde** u, t ve n var (egitim.py:389, 402-403).
  - u ve t yalnızca kuyruk sıralamasında kullanılıyor (app.py:368).
  - `n` yalnızca liste ipucunda görünüyor (app.py:865, index.html:861).
  - Hiçbiri M etiketleme ya da onay için eşik görevi görmüyor.
- **Model düzeyinde** şunlar var: test mAP50, mAP50-95, P, R ve temiz P/R (egitim.py:358-368), `val_map50` (223-225), Kılavuz ölçümleri (app.py:985-1029). Eğitim işçisi karışıklık tablosu **hesaplamıyor**, çünkü `ayrinti=False` (ortak.py:415-417).
- **Emek** özeti yalnızca raporlanıyor (app.py:757-792). `model` ve `oneri` kaynakları birlikte gruplanıyor (791).
- **Yeni:** `davranis.jsonl` ve `sinif_ozeti` ile sınıf başına model kutusu akıbeti tutuluyor. Bu bir veri; hiçbir eşiğe ya da seçime bağlanmıyor.

## 8. C: model adayları nereden geliyor

- Adaylar, conf_aday eşiğiyle üretilen ve temizlenmiş tahminlerdir (egitim.py:381-386). Ön etiket eşiğinin üstündeki kutuları da içerir.
- Sunucu bu listenin tamamını sınıf süzgeciyle `aday` olarak verir (app.py:689-691). Etiketli ya da M karede de verilir; yeter ki kare havuzda olsun, denetim olmasın ve son turda json'u bulunsun.
- **Tıklamada seçim:** boşa tıklanınca `adayBul` çalışır (index.html:1014-1024, 462-471).
- Eklenen kutu `m: a.s` taşır (1020) ve kesikli çizilir (747).
- `m` alanı şu durumlarda silinir: taşıma ya da boyutlandırma (1036), ok tuşu (1081), sınıf değişimi (891-892).
- C tuşu yalnızca görünürlüğü açıp kapatır (1153, çizim 729-738).

## 9. Zayıf noktalar ve riskler

1. **Denetim karelerinde M kutuları görünüyor.**
   - Denetim örneği `ozet.json` karelerinden çekiliyor (app.py:479-482). M kareler bu listede, çünkü `tahmin` listesine giriyorlar (411).
   - `get_labels` denetimde yalnızca `oneri` ve `aday` listelerini kapatıyor (685). Kutuları ise dosyadan, yani M etiketinden okuyor (674-678).
   - Bu sırada `S.pre=true` oluyor (index.html:589) ve uyarı şeridi "model önerisi bilerek gizlendi" diyor (802).
   - Tur sonunda denetim seçimi (482) M yenilemesinden (485) önce yapılıyor. Yenileme bu kareleri yeniden yazıyor (1147-1148).
2. **Boş etiket olumsuz örneğe dönüşüyor.**
   - Eşiği geçen kutu yoksa boş .txt yazılıyor (app.py:1124-1127).
   - Etiketsiz karede json varsa `S.pre=true` oluyor (index.html:590-592) ve D boş dosya kaydediyor (app.py:713).
   - Etki zayıflatılmış ama ortadan kalkmamış. Boş karenin parçaları yalnızca `bos_parca_orani` olasılığıyla (varsayılan 0,15) eğitime girer (egitim.py:148, 164).
   - Dolu karelerde kaçırılan nesneler, nesne içeren parçaların tümü kullanıldığı için tam ağırlıkla "arka plan" olarak öğrenilir (egitim.py:160-165).
3. **Onay için zorlayıcı bir koşul yok.**
   - D, "Onayla · sonraki" düğmesi, Ctrl+S ya da düzenlenmiş karede gezinme kareyi onaylıyor (index.html:569, 1126, 1143, 1162).
   - En kısa süre ya da kutu kutu onay gibi bir şart yok. `e.repeat` engelli olsa da hızlı basış ya da tıklama yeterli.
   - M kareler art arda gelmez. Kuyruk önceliğine göre etiketsiz karelerle karışık sıralanırlar (632-634, 664-665).
   - Süre ve "onay" sayısı `davranis.jsonl` dosyasına yazılıyor (davranis.py:123, 143) ama hiçbir şey bunu denetlemiyor.
4. **M karelerde güven ve kesikli çizgi görünmüyor.**
   - Dosyadan yüklenen kutularda `m` alanı yok (index.html:586-588). Bu yüzden düz çiziliyorlar ve listede güven gösterilmiyor (747, 751, 828).
   - Kod yorumu ise "modelden gelen ve henüz onaylanmamış olanlar kesikli" diyor (740).
   - Aynı nedenle M karelerin davranış kaydındaki açılış kutularında güven `null` kalıyor (index.html:930, davranis.py:81-82).
5. **Kaynak bilgisi .txt dosyasında yok, yalnızca günlüklerde var.**
   - Kare düzeyinde kaynak `emek.jsonl` dosyasında, kutu düzeyinde akıbet `davranis.jsonl` dosyasında tutuluyor. Bu kayıtlar etiket dosyasına bağlı değil ve eğitimde kullanılmıyor.
   - M işaretinin tek kaynağı `durum.json`. Yazma atomik (ortak.py:41-53). Ama dosya okunamazsa `{}` döner (app.py:1271, ortak.py:34-38). O durumda:
     - tüm M etiketleri sessizce eğitime girer,
     - test seti de yeniden seçilir (1274-1275).
   - Etiket klasörü dışarıda kullanılırsa onaylanmamış M .txt dosyaları insan etiketinden ayırt edilemez.
6. **Doğrulama kümesi model yanlılığı taşıyabilir.** Onaylanan M ya da öneri kareleri `kume_payi` ile doğrulamaya düşebilir (app.py:405). Erken durdurma ve best.pt seçimi modelin kendi kutularıyla yapılmış olur.
7. **Sıcak başlangıç döngüyü güçlendiriyor.** Her tur önceki best.pt üzerinden ince ayar yapıyor (app.py:413-417).
8. **NMS ya da grup ayarı değişince "M yenile" v3.6 tahminlerine etki etmiyor.**
   - Bu tahminler `temiz:True` kaydediliyor (egitim.py:386) ve olduğu gibi döndürülüyor (app.py:1136-1137).
   - Ayar açıklaması "Değişince M etiketlerini yenile" diyor (index.html:1533). Bu yanıltıcı.
   - OKUBENI.md:41'deki cümle ise v3.5 tahminleri için doğru.
   - Kılavuz temiz modu ham test tahminini güncel ayarla yeniden temizliyor (app.py:1013-1014). Bu yüzden Kılavuz'da görülen sonuç gerçek önerilerle tutmayabilir.
9. **Hiçbir şey görmeyen kare en sona düşüyor.**
   - Tespiti olmayan karede `u=0` (ortak.py:567-568).
   - Bunu yalnızca titreme telafi edebilir (ortak.py:608-611): ayar açıksa, `a_t>0` ise ve komşu kareler biliniyorsa.
   - Güven kalibre edilmeden "p≈0,5 kararsız" varsayımıyla kullanılıyor (ortak.py:569).
10. **Toplu M etiketlemede kalite filtresi yok.** En kararsız kareler de etiketleniyor (app.py:1111-1113).
11. **Sınıftan bağımsız NMS yok.**
    - `nms` ve `parca_birlestir` sınıf başına çalışıyor (ortak.py:280, 491). Grup filtresi de aynı sınıfla sınırlı (519).
    - Predict çağrısına `agnostic_nms` verilmiyor (egitim.py:300).
12. **Test seti sıfırlanınca M kareler korunmuyor. Sonuç: test kirlenmesi.**
    - `test_seti_sec` `model_etiket` listesine dokunmuyor (app.py:1170-1207). Kaynak "tum" seçilirse M kare test ya da tampon olabilir (275).
    - Böyle bir test karesinde olanlar:
      - Şerit "Model yardımı kapalı, dikkatle elle etiketle" diyor (index.html:800), oysa gösterilen kutular modelden geliyor (app.py:674-678) ve `S.pre=true` (index.html:589).
      - Kare test sayısında etiketsiz sayıldığı için `test_eksik` olur ve otomatik eğitim durur (app.py:336, 539).
      - D ile onaylanırsa **model kutuları test için doğru kabul edilen etiket olur**.
      - Bu kareler sonraki turlarda yenilenmez, çünkü `tahmin` listesinde değiller (411).
13. **Açık M kare otomatik yenilemede güncellenmiyor.**
    - Yeni tur gelince arayüz yalnızca `!S.pre` ve etiketsiz kareyi yeniden yükler (index.html:1186).
    - Açık M karede D'ye basılırsa eski modelin kutuları, yeni modelle yazılmış dosyanın üstüne yazılır.
    - "M yenile" düğmesi ise açık M kareyi yeniden yüklüyor (1301).
14. **Eşleştirme kök ada (stem) göre yapılıyor** (app.py:124-125, 160-162; egitim.py:386). Aynı köke sahip iki resim aynı etiket dosyasını ve aynı M durumunu paylaşır.
15. **Yarış durumu (yeni tespit).**
    - `save_labels` .txt dosyasını kilit dışında yazıyor (app.py:713) ve M işaretini sonra kaldırıyor (725-728).
    - Tur sonu yenilemesi kilit altında, izleyici iş parçacığında çalışıyor (631-634 → 485).
    - Bu iki adım arasındaki dar zaman penceresinde yenileme dosyayı model kutularıyla ezebilir. Ardından M işareti silinir. Sonuç: model kutuları sessizce "onaylı" sayılır.

**Okunan dosyalar:**
- /home/user/talha/etiketleyici_v3_6/app.py
- /home/user/talha/etiketleyici_v3_6/egitim.py
- /home/user/talha/etiketleyici_v3_6/ortak.py
- /home/user/talha/etiketleyici_v3_6/davranis.py
- /home/user/talha/etiketleyici_v3_6/static/index.html
- /home/user/talha/etiketleyici_v3_6/static/kilavuz.html
- /home/user/talha/etiketleyici_v3_6/OKUBENI.md

## Düzeltmeler:

1. **Satır numaraları eski sürüme göreydi.** Analiz `da8f5c0` (v3.6) commit'ine göre yazılmış. Çalışma ağacı `56f2a51` (Faz 1 / v3.7). Kaymalar:
   - app.py'de 36. satırdan sonra +1,
   - `save_labels` içinde +4,
   - 730. satırdan sonra +29.
   - index.html baştan değişmiş.
   - Tüm referanslar güncellendi. Örnekler: `model_etiketlenebilir` 1069→1098, `kume=…` 394→395, D tuşu index.html:1111→1143, `json_oku(durum.json)` 1239→1271, `conf_on_etiket` tanımı 71→72.
2. **kilavuz.html:194-195 her iki sürümde de yanlıştı.** Doğrusu 192.
3. **"egitim.py:357-358 karışıklık" yanlıştı.** Eğitim işçisi karışıklık tablosu hesaplamıyor (`ayrinti=False`). Bu tablo yalnızca app.py:1022-1023'te hesaplanıyor.
4. **`S.pre=true` koşulu yanlış verilmişti.** Koşul "öneri varsa" değil, `tahmin_var` (index.html:590-592). Öneri listesi boşken de geçerli.
5. **"Kuyruktaki M kareler art arda gelir" yanlıştı.** M kareler öncelik sırasına göre etiketsiz karelerle karışık geliyor. Kuyrukta olmayan M kareler sonda (index.html:632-634).
6. **"Kaynak bilgisi hiçbir yerde tutulmuyor" eksik ya da eskiydi.** Kare düzeyinde `emek.jsonl` vardı. Faz 1 ile kutu düzeyinde `davranis.jsonl` eklendi. Bilgi .txt dosyasında ve eğitimde yine yok.
7. **"Yalnızca conf_on_etiket canlı" ifadesine ayrıntı eklendi.**
   - `n`, titreme ve test P/R tur sırasında donar.
   - Diğer üç ayar tur başlangıcında iş dosyasına kopyalanır (app.py:428).
   - Kuyruk ağırlıkları canlıdır (app.py:1076).
8. **`aday` için sınıf süzgeci ve kararsızlık için kapsam eklendi.** `aday` listesine `0<=sınıf<nc` süzgeci uygulanıyor. Kararsızlık, temizlenmiş ve 500 ile kesilmiş kutular üzerinden hesaplanıyor.
9. **Onay yollarına iki ekleme yapıldı.** "Onayla · sonraki" düğmesi ve liste sonunda öteki listeye kendiliğinden geçiş eklendi.
10. **Risk 8 düzeltildi.** OKUBENI.md:41 (eski satır 16) v3.5 tahminleri için doğru. Yanıltıcı olan index.html:1533.
11. **Risk 2'ye ayrıntı eklendi.** Boş kare etkisi `bos_parca_orani` ile zayıflıyor. Dolu karelerde kaçırılan nesneler ise tam ağırlıkla arka plan oluyor.
12. **Risk 12 genişletildi.** Test kirlenmesi, yanıltıcı TEST şeridi, otomatik eğitimin durması ve yenilenmeyen M test kareleri eklendi.
13. **Yeni bulgular eklendi.**
    - `save_labels` ile tur sonu M yenilemesi arasındaki yarış durumu (risk 15).
    - M karelerin davranış kaydında güven olmaması (risk 4).
    - `m_etiketlerini_yenile` json yoksa eski etiketi bırakıyor.
    - `davranis.py` / `sinif_ozeti` var ama hiçbir karara bağlı değil.
14. **Doğrulanan iddialar.** Formüller (u, grup ve parça birleştirme koşulları, NMS), "kalibrasyon yok", "sınıftan bağımsız NMS yok", `ic_skor` sabit kodlanmış, boş dosya yazılınca True dönmesi, denetim karesinde M sızıntısı, stem eşleştirmesi ve sıcak başlangıç kodla doğrulandı. Bunlarda yalnızca satır numaraları güncellendi.