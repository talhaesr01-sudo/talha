# Aktif öğrenme ve öneri sıralaması: kod analizi (etiketleyici, çalışma ağacı v3.7 "Faz 1")

**Satır numaraları hakkında:** Önceki analizdeki `app.py`, `index.html` ve `OKUBENI.md` satır numaraları ilk v3.6 commit'ine (`da8f5c0`) uyuyor. Diskteki kod ise HEAD `56f2a51` ("Faz 1"); `app.py:2` ve `OKUBENI.md:1` "sürüm 3.7" diyor. `app.py:36` satırına `import davranis as D` eklendiği için ondan sonraki her satır 1 kaydı; yaklaşık 730. satırdan sonra kayma +29/+32 oldu. `index.html` yeniden düzenlendi. `egitim.py` ve `ortak.py` değişmemiş, eski referansları doğru. Aşağıdaki tüm referanslar diskteki koda göre.

**Kısaca:**
- Öncelik puanı kare başına hesaplanıyor.
- Kararsızlık ham, kalibre edilmemiş güven değerinden geliyor ve formülde açık bir sınıf terimi yok.
- Kuyruk düz bir sıralama. Çeşitlilik ya da sınıf dengesi yok, iki tur arasında da yeniden hesaplanmıyor; ayar kaydedilince yalnızca yeniden ağırlıklanıyor.
- Denetim (D) karelerinde belgelerle çelişen iki sızıntı var (bölüm 6, madde 8). Test seti sıfırlanınca kuyruğa test ya da tampon karesi sızabiliyor (madde 7).

## 1. Genel akış

1. **İş dosyası.** `tur_baslat` (`app.py:384-457`):
   - `kume`: etiketli kareler eksi M kareleri (`app.py:395`).
   - `tahmin`: `kume` içinde olmayan havuz kareleri (`app.py:396,411`). Yani etiketsiz kareler artı kontrol edilmemiş M kareleri. Boş `.txt` dosyası olan kare etiketli sayılıyor.
   - `tum_sira`: tüm kareler. `etiketli_havuz`: insan etiketli havuz kareleri (`app.py:431`).
   - Bunlar `turlar/tur_XXX/is.json` dosyasına yazılıyor (`app.py:433`).
2. **Tahmin.** İşçi önce test setini ölçüyor, sonra `tahmin_et` ile `tahmin` listesinin tamamını parça parça tahmin ediyor (`egitim.py:444-445,371-405`). Her tur havuzun tamamı yeniden tahmin ediliyor; alt örnekleme yok. Okunamayan kare atlanıyor ve özete girmiyor (`egitim.py:287-289`).
3. **Sonuç işleme.** `tur_sonucunu_isle`, yalnızca başarılı turda:
   - `DURUM["tahmin"]={tur,dizin}` (`app.py:478`)
   - denetim seçimi (`app.py:479-482`)
   - `kuyruk_kur(ayar)` (`app.py:483`)
   - M karelerini yenileme (`app.py:485`)
4. **Kuyruğun kurulduğu yerler.** Yalnızca iki yer:
   - Tur bitince (`app.py:483`). Sunucu açılışında yarım kalmış bir tur işlenirse de bu yol çalışıyor (`app.py:1280-1283`).
   - Ayar kaydedilince (`app.py:1076`).
   
   Etiket kaydedince kuyruk kurulmuyor (`save_labels`, `app.py:699-729`). Normal açılışta da kurulmuyor; kuyruk `durum.json` dosyasından olduğu gibi yükleniyor (`app.py:1271`).

## 2. Tahmin sonrası skor üretimi (egitim.py)

- **Tahmin parametreleri:**
  - `conf=conf_aday`; varsayılan 0,05 (`app.py:73`).
  - Parça başına YOLO `iou=0.7`, `max_det=100`.
  - GPU'da FP16, `quantize:16` ya da `half` (`egitim.py:300-301,381,61-73`).
- **Güven değeri:** doğrudan `r.boxes.conf` (`egitim.py:320`).
  - **Birleştirme:** parçalar `O.parca_birlestir` ile birleşiyor (`egitim.py:327-328`). Yalnızca farklı parçalardan gelen aynı sınıf kutular eşleşiyor (`ortak.py:280`). Kalan skor en yüksek skorlu kutunun skoru (`ortak.py:266,309`). Kenarda kesilen kutularda geometri birleşim kutusu oluyor (`ortak.py:305-307`).
  - **Temizleme:** `temizle` önce grup kutusu filtresini (en az 3 kutu varsa, `ortak.py:515`), sonra sınıf başına NMS'i uyguluyor. NMS eşiği `oneri_nms`, varsayılan 0,5 (`egitim.py:382`, `ortak.py:538-550`, `app.py:83-84`). Skorlar değişmiyor, yalnızca kutular eleniyor. Dönen liste skor sıralı.
  - **Sınır:** sonra ilk 500 kutu alınıyor; bunlar en yüksek skorlu 500 kutu (`egitim.py:383`).
- **Kare dosyası:** `tahmin/kareler/<stem>.json` = `{"w","h","k":[[x1,y1,x2,y2,sınıf,güven(3 hane)]],"temiz":true}`. Koordinatlar tam piksele yuvarlanıyor (`egitim.py:384-386`).
- **Kare özeti:** `kareler[ad] = {"u": kararsizlik(s), "t": 0.0, "n": güven ≥ conf_on_etiket olan kutu sayısı}` (`egitim.py:387-389`).
  - `u`, `conf_aday` eşiğinin üstündeki tüm temizlenmiş kutuların yuvarlanmamış skorlarıyla hesaplanıyor.
  - Önerilmeyen düşük güvenli kutular da buna dahil. Bunlar öneri olarak gösterilmiyor ama "aday" olarak arayüze gidiyor (`app.py:691,693`).
- **Titreme:** yalnızca işin ayarlarında `titreme` açıksa hesaplanıyor (`egitim.py:390-403`).
  - Tahmin edilen karede `varlik` = güveni `conf_on_etiket` ve üstündeki kutular, sınıflarıyla. Güven değeri kullanılmıyor, yalnızca var/yok (`egitim.py:387-388`).
  - İnsan etiketli komşularda `varlik` = gerçek etiketler. Bunlar yalnızca bir tahmin karesinin ±1 komşusu olup `etiketli_havuz` içindeyse yükleniyor (`egitim.py:394-401`).
- **Özet dosyası:** `tahmin/ozet.json` = `{"tur":N,"kareler":{ad:{u,t,n}}}` (`egitim.py:404`).
- **Eklenen nokta: değerler tur başında sabitleniyor.** `n`, `t` ve titremenin açık olup olmadığı tur başındaki iş ayarlarından alınıyor (`egitim.py:376,387,390`).
  - Sonradan `conf_on_etiket` ya da `titreme` değiştirilirse `kuyruk_kur` aynı `ozet.json` dosyasını yalnızca yeni ağırlıklarla yeniden puanlıyor (`app.py:364-369`).
  - Örneğin titreme kapalı bir turdan sonra açılırsa `t` değerleri 0 olduğu için sıra değişmiyor.

## 3. Formüller (ortak.py)

**Kararsızlık** (`ortak.py:563-570`):
- Kutu başına `u = 1 − |2p − 1|`; p burada ham güven değeri.
- Kare değeri, en büyük 3 `u` değerinin ortalaması (`en_cok=3`). Kutu yoksa 0.
- Girdi yalnızca skorlar.
- Örnek değerler: p=0,05 için u=0,1; p=0,25 için 0,5; p=0,5 için 1; p=0,95 için 0,1.
- 3'ten az kutu varsa ortalama mevcut kutularla alınıyor. Tek kutusu p=0,5 olan kare 1,0 alıyor. Kutuları {0,5; 0,95; 0,95} olan kare (1+0,1+0,1)/3 = 0,4 alıyor.
- **Sınıf nüansı:** formülde sınıf yok. Ancak birleştirme ve NMS yalnızca aynı sınıftan kutuları eliyor (`ortak.py:280,491-500`). Aynı yere farklı sınıfla iki kutu konursa ikisi de `u` hesabına giriyor.

**Titreme** (`ortak.py:586-612`, `_eslesir` `ortak.py:573-583`):
- Eşleşme koşulu: aynı sınıf ve IoU ≥ 0,5.
- `vyv` (var-yok-var): A (önceki karede) olup B'de (sonraki karede) eşi bulunan, ama C'de (bu karede) eşi olmayan nesne sayısı (`ortak.py:608`).
- `yvy` (yok-var-yok): C'de olup ne A'da ne B'de eşi olan nesne sayısı (`ortak.py:609`).
- `t = (vyv+yvy)/(|C|+vyv)`; payda 0 ise 0. Değer 0 ile 1 arasında (`ortak.py:610-611`).
- C'de hiç kutu yoksa ve komşularda eşleşen nesne varsa `t = vyv/vyv = 1`.
- İlk ve son kare (`tum_sira` içinde test ve tampon dahil) 0 alıyor. Komşulardan biri `varlik` içinde değilse de 0; örneğin komşu test ya da tampon karesiyse (`ortak.py:602-606`).
- Zaman sırası doğal ad sıralamasından geliyor (`app.py:112,431`, `ortak.py:25-31`).

**Öncelik** (`app.py:366-369`):
- `skor = round(agirlik_u·u + wt·t, 4)`. Titreme açıksa `wt = agirlik_t`, kapalıysa 0 (`app.py:367`).
- Varsayılanlar 0,7 ve 0,3 (`app.py:75-76`), ikisinin aralığı da [0,1].
- Ağırlıklar normalize edilmiyor, toplam en fazla 2 olabilir. Arayüz puan çubuğunu 0–1 aralığına kırpıyor (`index.html:856`).
- Kılavuz ağırlıkları sabit "0,7 / 0,3" olarak yazıyor (`kilavuz.html:175`).
- Sıralama `sorted(adlar, key=-skor)` ile yapılıyor. Python'da sıralama kararlı ve `adlar` doğal sıralı olduğu için eşit skorlar zaman sırasında kalıyor (`app.py:365,369`). Skor 4 haneye yuvarlandığı için eşitlik olasılığı artıyor.
- **Birim:** puan kare başına. `u` kutu başına hesaplanıp kareye toplanıyor, `t` doğrudan kare başına.

## 4. Kuyruk fonksiyonları (app.py) ve arayüz

**`tohum_listesi`** (`app.py:346-354`):
- `rol=="havuz"` olan karelerden `k = min(|havuz|, ilk_egitim_esigi)` tane, eşit aralıklı indekslerle seçiyor: `round(i·(n−1)/(k−1))`.
- Etiketli kareleri dışlamıyor; dışlamayı arayüz yapıyor (`index.html:630`).
- `ilk_egitim_esigi` dinamik: tüm resim sayısı n ≥ 500 ise 100, değilse `max(10, round(n·0,1))` (`app.py:182`). Bu yalnızca kullanıcı farklı bir değer kaydetmemişse geçerli (`app.py:188-190`).
- **Eklenen nokta:** `tur_buyuklugu` da dinamik: `max(5, round(n·0,05))` (`app.py:183`). `AYAR_TANIM` içindeki 50 (`app.py:59`) bununla eziliyor.

**`kuyruk_kur`** (`app.py:357-380`):
- `ozet.json` dosyasını her çağrıda yeniden okuyor (`app.py:364`) ve `rol=="havuz"` süzgecini uyguluyor (`app.py:365`).
- **Aktif mod:** bölüm 3'teki skor ve sıralama.
- **Rastgele mod:** `random.Random(tur).shuffle`, yani tohum tur numarası; `skor={}` (`app.py:370-373`).
- **Denetim kareleri:** önce listeden çıkarılıyor, sonra `min(len, round((i+1)·b/(|D|+1)) + i)` konumlarına, yani ilk `b = tur_buyuklugu` kare arasına yerleştiriliyor (`app.py:374-379`). Bu iki modda da yapılıyor.
  - D yalnızca `a in kareler` ile süzülüyor, `rol` ile değil (`app.py:374`).
  - Aktif modda D karelerinin de skoru var, çünkü `skor` sözlüğü tüm havuz karelerini kapsıyor (`app.py:368`).
- Sonuç `DURUM["kuyruk"]` ve `DURUM["kuyruk_skor"]` alanlarına yazılıyor (`app.py:380`). Diske yazmayı çağıran yapıyor (`app.py:508,1077` → `durum_yaz`, `app.py:235-236`).

**`kuyruk_detay`** (`app.py:858-866`):
- Kare başına `[u, t, n]` döndürüyor.
- Önbellek anahtarı yalnızca `t["dizin"]` (`app.py:863`). Bir turun `ozet.json` dosyası sonradan yazılmadığı için bu sorun çıkarmıyor.

**`/api/kuyruk`** (`app.py:869-878`):
- Hem `tahmin` hem dolu bir `kuyruk` varsa şunları döndürüyor: `adlar`, `denetim`, `skor`, `detay`, `agirlik` (titreme kapalıysa `agirlik.t=0`).
- Aksi halde `tohum_listesi` döndürüyor ve `tohum=True` diyor. Yani model varken kuyruk boşsa da (örneğin tahmin listesi boştu) tohum listesine düşülüyor.

**Arayüz (v3.7 kategori sistemi):**
- **Kategoriler** (`index.html:616-620`):
  - etiketli ve M değil → "onay"
  - M ya da kuyrukta (tohum değil) → "oneri" (MODEL ÖNERDİ)
  - kalan: tampon ise hiçbir liste, değilse "etiketsiz"
- **MODEL ÖNERDİ listesi:** sıra sunucu kuyruğuna göre; kuyrukta olmayan M kareleri sonda (`index.html:632-634`). İleri giderken kategorisi değişen kareler (örneğin bu arada onaylananlar) atlanıyor (`index.html:662-666`). Liste bitince ETİKETSİZ listesine kendiliğinden geçiliyor (`index.html:679-685`).
- **ETİKETSİZ listesi:** önce test kareleri, sonra (tohumsa) tohum listesi, sonra zaman sırası (`index.html:635-638`).
- **Puan:** yalnızca MODEL ÖNERDİ listesinde, etiketsiz ya da M karelerinde ve skoru varsa gösteriliyor (`index.html:854`). İpucu metni: "Öncelik X = a × kararsızlık u + b × titreme t / Modelin önerdiği kutu: n" (`index.html:859-862`).
- **Kuyruğun yeniden çekildiği yerler:**
  - açılışta (`index.html:537`)
  - yeni tur gelince (`index.html:1179-1186`)
  - klasördeki resim ya da etiket sayısı veya test sürümü değişince (`index.html:1194-1205`)
  - M yenile ve M etiketle işlemlerinden sonra (`index.html:1300,1319`)
  - test seti onaylanınca (`index.html:1474`)
  - ayarlar uygulanınca ya da kaydedilince (`index.html:1628,1647`)
  
  Kuyruk yalnızca çekiliyor. Sunucu, ayar kaydı dışında bu anlarda kuyruğu yeniden kurmuyor.
- **Eklenen nokta:** kuyruk `n` değerine göre süzülmüyor (`app.py:365`). Modelin hiç kutu önermediği (n=0) kareler ve önerisi gizlenen D kareleri de "MODEL ÖNERDİ" listesinde çıkıyor (`index.html:615,618`). Bu, OKUBENI'deki "modelin kutu önerdiği etiketsiz kareler" tanımıyla tam örtüşmüyor (`OKUBENI.md:13`).

## 5. Roller, denetim, test/tampon ve M kareleri

**`rol()`** (`app.py:165-170`):
- `ROL["test"]` ve `ROL["tampon"]` kümelerine bakıyor; ikisinde de yoksa "havuz".
- Kümeler `DURUM["test"]` ve `DURUM["tampon"]` alanlarından kuruluyor (`app.py:239-241`).

**Test ve tampon** (`app.py:256-294`):
- Test seti zamansal bir blok: son, baş ya da bloklu seçim.
- Kaynak ya etiketli kareler (M hariç, `app.py:271`) ya da tüm kareler (`app.py:275`).
- Tampon, her test karesinin ±`tampon` komşusu (`app.py:290-293`). Değer `test_secim.tampon` alanından ya da `test_tampon` ayarından geliyor; varsayılan 10 (`app.py:82,267`).
- İkisi de tahmin listesine (`app.py:396,411`), yeniden kurulan kuyruğa (`app.py:365`) ve tohum listesine (`app.py:348`) girmiyor.
- Test tahminleri ayrı yapılıyor: `conf=0.01`, ham, `test_tahmin.json` dosyasına (`egitim.py:343-348`).

**Denetim (D)** (`app.py:479-482`):
- **Sayı:** `k = ceil(denetim_orani · tur_buyuklugu)`. `tur_buyuklugu` dinamik olduğu için örneğin n=1000 resimde 50 → 3 kare, n=200 resimde 10 → 1 kare, n=2000 resimde 100 → 5 kare.
- **Seçim:** aday listesi `ozet.json` içindeki tüm tahmin kareleri (M dahil); seçim `random.Random(tur).sample`.
- Her başarılı turda `DURUM["denetim"]` baştan yazılıyor (`app.py:482`, tek atama yeri). Önceki turun etiketlenmemiş D kareleri normal kareye dönüyor. Ayar kaydı D'yi yeniden çekmiyor.
- **D karelerinde ne oluyor:**
  - Öneriler ve adaylar gizleniyor (`app.py:681,685`).
  - Toplu model etiketine alınmıyorlar (`app.py:1111-1112`).
  - Arayüzde uyarı bandı çıkıyor (`index.html:802`).
- **Eklenen nokta:** kodda D karelerinden ayrı bir ölçüm hesaplanmıyor.
  - D işareti yalnızca yukarıdaki iki yerde kullanılıyor.
  - Davranış kaydında denetim alanı yok (`davranis.py:140-141`).
  - Emek kaydında D kareleri "elle" kaynağına düşüyor (`index.html:597`).

**M kareleri:**
- Kontrol edilmemiş model etiketleri `kume` dışında tutuluyor (`app.py:395`). Bu yüzden `tahmin` listesine giriyor, puan alıyor ve kuyrukta etiketsizlerle aynı skorla, karışık olarak sıralanıyor.
- Ayrı bir öncelikleri yok. Arayüz onları "oneri" kategorisinde sayıyor (`index.html:618`), M rozetiyle (`index.html:852`) ve puanla (`index.html:854`) gösteriyor.
- Her başarılı turdan sonra yeni önerilerle yeniden yazılıyorlar (`app.py:485,1141-1148`). Elle yenileme de var (`app.py:1161-1163`).
- `m_etiketlerini_yenile` ne `rol` ne `denetim` alanına bakıyor (`app.py:1147-1148`).

## 6. Zayıf noktalar (kodda görülen)

1. **Ham güven.**
   - `u` doğrudan `r.boxes.conf` değerinden hesaplanıyor (`egitim.py:320,389`). Kodda kalibrasyon ya da sıcaklık ölçekleme yok; `app.py`, `egitim.py`, `ortak.py` ve `davranis.py` içinde araması sonuç vermedi.
   - Açık bir sınıf karışıklığı terimi yok (`ortak.py:563-570`). Yalnızca farklı sınıflı çakışan kutuların ikisinin de `u` hesabına girmesiyle dolaylı olarak yansıyor.
   - Birleştirme, grup filtresi ve NMS `u` hesabından önce çalışıyor (`egitim.py:327,382`). Skorundan bağımsız olarak, aynı sınıftan daha yüksek skorlu bir kutuyla IoU ≥ `oneri_nms` olan kutular ve grup kutuları `u` hesabından düşüyor.
   - Kılavuz MC Dropout ve Deep Ensemble için "v3.6'da yok" diyor (`kilavuz.html:170-193`, ifade `kilavuz.html:192`).
2. **Kaçırma körlüğü (kısmi).**
   - `conf_aday` üstünde hiç kutusu olmayan kare `u=0` alıyor (`ortak.py:567-568`).
   - Titreme bunu kısmen telafi ediyor: `conf_on_etiket` üstünde kutusu olmayan ama iki komşusunda eşleşen nesne bulunan kare `t=1` alıyor (`ortak.py:608-611`). Varsayılan ağırlıklarla puanı en fazla 0,3·1 + 0,7·u oluyor.
   - Üç ardışık karede de kaçırma varsa ya da komşu test, tampon veya dizi ucuysa `t=0`.
3. **Top-3 asimetrisi.**
   - 3'ten az kutulu kareler az kutuyla ortalandığı için şişiyor; örnek: 1,0'a karşı 0,4 (`ortak.py:570`).
   - Kutu boyutu açık bir ağırlık değil. Kutu sayısı da ağırlık değil, ama en büyük 3 değer seçildiği için çok kutulu karelerde p≈0,5 olan kutu bulma olasılığı dolaylı olarak artıyor.
4. **Çeşitlilik yok.**
   - Sıralama düz bir sıralama (`app.py:369`). Ardışık, birbirine çok benzeyen kareler kuyrukta yan yana çıkabiliyor.
   - Zamansal yayma yalnızca tohum listesinde (`app.py:354`), test tamponunda (`app.py:290-293`) ve D yerleşiminde (`app.py:378-379`) var.
5. **Sınıf dengesi yok.**
   - Skorda sınıf terimi yok (`app.py:368`).
   - Eğitim çağrısında sınıf ağırlığı ya da örnekleme parametresi yok (`egitim.py:246-251`).
6. **Titreme kırılgan.**
   - Hareket telafisi yok; sabit IoU ≥ 0,5 kullanılıyor (`ortak.py:573-583`).
   - Güven ikili eşikle atılıyor (`egitim.py:387`).
   - Zaman sırası doğal ad sıralamasına dayanıyor (`app.py:112`).
   - Test ya da tampon komşusu olan kareler 0 alıyor (`ortak.py:605`).
   - Etiketli komşuda modeli gerçek etiketle, etiketsiz komşuda modeli modelle karşılaştırıyor (`egitim.py:388,401`).
7. **Bayat kuyruk.**
   - Kuyruk yalnızca tur sonunda ya da ayar kaydedilince kuruluyor (`app.py:483,1076`). Etiketleme sırasında komşuların titremesi yeniden hesaplanmıyor, sıra güncellenmiyor. Ayar kaydı yalnızca yeniden ağırlıklıyor (bölüm 2, son madde).
   - Test seti sıfırlanınca `test_seti_sec` yalnızca `test_seti_kur` çağırıyor (`app.py:1204`), `kuyruk_kur` çağırmıyor. Eski kuyrukta artık test ya da tampon olan kareler kalıyor.
   - Kuyruk yeniden kurulsa bile D kareleri `rol` ile süzülmediği için (`app.py:374`) test ya da tampon olmuş bir D karesi geri giriyor.
   - Arayüzde bu kareler kuyrukta oldukları için "oneri" kategorisine düşüyor. Tampon karesi bile, tampon için gizleme kuralından önce bu kategoriye giriyor (`index.html:618-619`).
8. **Denetim sızıntıları** (`OKUBENI.md:91`: "Test, tampon ve denetim kareleri asla model tarafından etiketlenmez", ve `OKUBENI.md:122`: "model önerisi bilerek gizlenir" ile çelişiyor):
   - **(a) Puan ve kutu sayısı görünüyor.** Aktif modda D karelerinin puanı listede gösteriliyor (`app.py:368`, `index.html:854`). İpucu `u`, `t` ve modelin önerdiği kutu sayısı `n` değerini veriyor (`app.py:865`, `index.html:859-862`).
   - **(b) M karesi D seçilebiliyor.** Denetim örneklemi M karelerini dışlamıyor (`app.py:481-482`).
     - Seçilirse etiket dosyasındaki model kutuları görünüyor ve öneri gibi işaretleniyor (`index.html:585-589`); bant yine de "DENETİM" yazıyor (`index.html:802`).
     - Aynı tur sonunda, D atandıktan hemen sonra, `m_etiketlerini_yenile` o kareyi yeni modelle yeniden yazıyor (`app.py:482→485`, `app.py:1147-1148`).
   - **(c) Eklenen nokta: M kareleri test ya da tampon olabiliyor.** Test seti `kaynak=tum` ile sıfırlanırsa havuz tüm kareler olduğu için (`app.py:275`) M kareleri test ya da tampon olabiliyor. `m_etiketlerini_yenile` rol kontrolü yapmadığı için bunları da model etiketiyle yeniden yazıyor (`app.py:1147-1148`).
9. **Önbellek.**
   - Tahminler turlar arasında önbelleğe alınmıyor; her tur havuzun tamamı yeniden tahmin ediliyor (`app.py:411`). Önbellekte tutulan yalnızca eğitim parçası görüntüleri (`egitim.py:167-177`).
   - Sunucu önbellekleri:

     | Önbellek | Süre / anahtar | Yer |
     |---|---|---|
     | `goruntu_adlari` | 3 sn | `app.py:106` |
     | `etiketli_kume` | 2 sn (`etiket_degisti` ile hemen geçersiz) | `app.py:143,157` |
     | `kuyruk_detay` | `dizin` | `app.py:863` |
     | `model_etiketlenebilir` | `tur`, `dizin`, `id(kume)`, `id(adlar)`, `test_surum`, `denetim` | `app.py:1104` |

   - `etiketli_kume` her taramada yeni bir frozenset oluşturuyor (`app.py:145`) ve atıyor (`app.py:151`); `goruntu_adlari` da 3 sn'de bir yeni liste atıyor (`app.py:112`). Bu yüzden `model_etiketlenebilir` önbelleği en fazla yaklaşık 2 sn geçerli kalıyor.
   - Bayat sonuç için iki `id` değerinin birden eski serbest kalmış nesnelerle çakışması gerekiyor; bu teorik bir durum.
   - `kuyruk_kur` her çağrıda `ozet.json` dosyasını yeniden okuyor (`app.py:364`).
10. **Eklenen nokta: davranış kaydı kuyruğu etkilemiyor.** v3.7 davranış kaydı (`davranis.jsonl`, `app.py:715,732-739`) yalnızca kaydediliyor. Öncelik hesabına girmiyor; yalnızca CSV ve özet uç noktalarında kullanılıyor (`app.py:742-754`). OKUBENI bunu "sonraki sürümlerde" diye belirtiyor (`OKUBENI.md:26-27`).

## 7. Okunan ve yazılan dosyalar

Çalışma klasörü varsayılan olarak `<labels klasörü>_calisma/`; etiket klasörünün kardeşi (`app.py:1259`).

| Dosya | Okuyan | Yazan | İçerik |
|---|---|---|---|
| `ayarlar.json` | `app.py:189` | `app.py:1070` (+ `--device` için `app.py:1266-1269`) | yalnızca statik ya da dinamik varsayılandan farklı ayarlar |
| `durum.json` | `app.py:1271` | `app.py:236` | `test`, `tampon`, `test_secim`, `test_onay`, `test_surum`, `turlar`, `sonraki_tur`, `tahmin{tur,dizin}`, `denetim`, `kuyruk`, `kuyruk_skor`, `model_etiket`, `son_basarili`, `aktif_is`, `oto_beklet`… |
| `is_durumu.json` / `iptal.bayrak` | `app.py:887` / `egitim.py:119` | `egitim.py:128` / `app.py:1093` | işçinin durumu / iptal bayrağı |
| `turlar/tur_XXX/is.json` | `egitim.py:98` | `app.py:433` | `egitim`, `dogrulama`, `test`, `tahmin`, `tum_sira`, `etiketli_havuz`, `ayarlar`, `parca_kok`, `baslangic_agirlik`, `epochs` |
| `…/tahmin/kareler/<stem>.json` | `app.py:686,1108,1120` | `egitim.py:386` | kare başına kutular |
| `…/tahmin/ozet.json` | `app.py:364,479,864` | `egitim.py:404` | `{u,t,n}` |
| `…/test_tahmin.json` | `app.py:987-992,1044` | `egitim.py:348` | ham test tahminleri |
| `…/sonuc.json` | `app.py:525,1282` | `egitim.py:470` | tur sonucu |
| `…/log.txt` | `app.py:502` | `app.py:434` (alt sürecin stdout'u) | tur günlüğü |
| `…/{egitim,dogrulama}.txt`, `veri.yaml`, `egitim*/weights/best.pt` | `egitim.py` | `egitim.py:202-206,265` | eğitim girdileri ve ağırlıklar |
| `parcalar/<parca>_<ortusme>/{images,labels}` | `egitim.py:186-187` | `egitim.py:177,181` | parça önbelleği (yol: `app.py:429`) |
| `emek.jsonl` | `app.py:759` | `app.py:721` | etiketleme emeği kaydı |
| `davranis.jsonl` (v3.7) | `app.py:744,752` | `app.py:737` (`app.py:1264`) | davranış kaydı; kuyruğu etkilemiyor |
| `<labels>/<stem>.txt`, `classes.txt` | `app.py:124-132,676-678` | `app.py:713` (kaydet), `app.py:1127` (model etiketi), `app.py:594` (XML/JSON içe aktarma), `app.py:131,599` (classes) | etiketler; varsayılan olarak resimlerle aynı klasör (`app.py:1257`) |

---

**Düzeltmeler:**

1. **Satır numaraları.** `app.py`, `index.html` ve `OKUBENI.md` referansları ilk v3.6 commit'ine (`da8f5c0`) göre yazılmıştı; diskteki kod v3.7 "Faz 1" (`56f2a51`). Tümü güncellendi. Başlıca örnekler:
   - `conf_aday` 72→73, ağırlıklar 74-75→75-76, `test_tampon` 81→82, dinamik eşik 181→182
   - `tohum_listesi` 345-353→346-354, `kuyruk_kur` 356-379→357-380, `tahmin` 410→411
   - tur sonucu 477-484→478-485, `save_labels` 698-725→699-729
   - `kuyruk_detay` 829-837→858-866, `/api/kuyruk` 840-849→869-878, ayar kaydı 1047→1076
   - `model_etiketlenebilir` 1075/1082-1084→1104/1111-1112, M yenileme 1112-1119→1141-1148, `test_seti_sec` 1175→1170-1207 (`test_seti_kur` 1204)
   - varsayılan klasörler 1227/1229→1257/1259
   - `index.html`: 836→856, 834-841→854-862, 782→802, 567-571→585-589, 603-615→615-671
   - `OKUBENI.md:64`→`91`, `kilavuz.html` "yok" ifadesi 192. satırda.
   
   `egitim.py` ve `ortak.py` referansları doğruydu.
2. **`tur_buyuklugu` varsayılanı.** 50 değil; dinamik `max(5, round(n·0,05))` (`app.py:183`). "Varsayılanlarla 3 D karesi" yalnızca yaklaşık 1000 resimde geçerli.
3. **Sızıntıların yeri.** Özetteki "maddeler 8 ve 9" yanlıştı. Belgeyle çelişen iki sızıntı madde 8'in içinde; madde 9 önbellek. Test/tampon sızıntısı madde 7'de. Madde 8'e üçüncü alt madde (M karesinin test/tampon olup model etiketiyle yeniden yazılması) eklendi.
4. **"Sınıfa bakmıyor" iddiası.** Farklı sınıflı çakışan kutuların silinmediği ve ikisinin de `u` hesabına girdiği nüansı eklendi.
5. **NMS ifadesi.** "0,5 civarındaki kopya kutular u'dan düşüyor" düzeltildi. Düşen kutular skordan bağımsız olarak, aynı sınıftan daha yüksek skorlu bir kutuyla IoU ≥ `oneri_nms` olanlar.
6. **Kaçırma körlüğü.** Titreme, kutusuz bir karede `t=1` verebiliyor. Bu durumun koşulları ve varsayılan ağırlıklarla puan sınırı eklendi.
7. **Arayüz bölümü v3.7'ye göre yeniden yazıldı.** Kategori sistemi, liste bitince otomatik geçiş, kuyruğun yeniden çekildiği tüm yerler. Ayrıca D ve n=0 karelerinin "MODEL ÖNERDİ" listesinde çıktığı, eski kuyruktaki tampon karesinin bile bu kategoriye düşebildiği eklendi.
8. **Diğer eklenen noktalar:**
   - `ozet` içindeki `n` ve `t` değerleri tur başındaki `conf_on_etiket` ve `titreme` ayarlarıyla sabitleniyor; ayar kaydı yalnızca yeniden ağırlıklıyor.
   - D her tur yeniden çekiliyor ve D'den ayrı bir ölçüm yok.
   - Kuyruk normal açılışta yeniden kurulmuyor.
   - `davranis.jsonl` kuyruğu etkilemiyor.
   - Top-3 seçimi kutu sayısının dolaylı etkisini taşıyor.
9. **`model_etiketlenebilir` önbelleği.** Frozenset 145. satırda oluşturulup 151. satırda atanıyor; 150. satır sürüm sayacı. Bayat sonuç için iki `id` değerinin birden çakışması gerektiği, yani teorik olduğu belirtildi.
10. **Dosya tablosu.** Satırlar güncellendi. `log.txt`, `davranis.jsonl`, `--device` ile `ayarlar.json` yazımı ve diğer etiket yazan yerler (XML içe aktarma, model etiketi) eklendi. `is.json` içerik listesi tamamlandı.