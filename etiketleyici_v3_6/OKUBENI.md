# Etiketleyici (sürüm 3.7): model destekli etiketleme + aktif öğrenme

Sen kutuları düzeltirsin, model arka planda öğrenir. Belli sayıda kare etiketlenince YOLO kendiliğinden eğitilir,
etiketsiz karelere ön etiket koyar ve en kararsız olduğu kareleri sana önce gösterir.

## v3.7'de neler değişti (Faz 1)

- **Büyüteç kaldırıldı.** Kutu çizerken ve köşe tutarken imlecin yanında çıkan büyütülmüş görüntü artık yok; fare normal
  imleç olarak kalır. Kenarı piksel piksel oturtmak için tekerlekle yakınlaştır ya da kutuyu seçip ok tuşlarını kullan
  (1 piksel, Shift ile 10; Alt + ok ile boyut). B tuşu artık bir şey yapmaz.
- **Sol menü sadeleşti: üç kategori.** Her kare bunlardan birindedir, sayıları yanında yazar. Kategoriye tek tıklayınca
  o listenin ilk karesi açılır.
  - **MODEL ÖNERDİ**: modelin kutu önerdiği etiketsiz kareler ve M kareleri. Modelin en kararsız olduğu kare en üstte.
  - **ETİKETSİZ**: henüz ne etiketi ne model önerisi olan kareler. Önce test kareleri (T), sonra tohum kareler, sonra
    zaman sırası. Model yokken çalışma bu listede yürür.
  - **ONAYLANDI**: senin etiketlediğin ya da D ile onayladığın kareler.

  Test seti, tampon, denetim ve öncelik eskisi gibi çalışır; yalnızca ayrı liste olarak gösterilmez. Rozetler (T, D, M, –)
  ve görüntünün üstündeki şerit karenin türünü söyler. Tampon kareler hiç kullanılmadığı için listelerde görünmez.
- **Daha az tıklama.** Başlıktaki **✓ Onayla · sonraki** düğmesi D ile aynıdır. Bir iş listesi bitince (ör. MODEL ÖNERDİ)
  program öteki iş listesine (ETİKETSİZ) kendiliğinden geçer.
- **Davranış kaydı.** Her kaydetmede `<çalışma klasörü>/davranis.jsonl` dosyasına bir satır yazılır: karede geçen
  süre, eklenen / model adayından eklenen / silinen / taşınan / boyutu değişen / sınıfı değişen kutu sayısı ve geri
  alma sayısı (ayrı ayrı). Ayrıca karenin açılıştaki kutuları kayıttakilerle karşılaştırılır: modelin her kutusu için
  "onaylandı / düzeltildi / sınıfı değişti / silindi", kullanıcının çizdiği her yeni kutu için "model kaçırdı".
  Model sekmesindeki "davranış kaydı CSV" bağlantısıyla Excel'e alınır. Sonraki sürümlerde öncelik puanı ve sınıfa
  göre ayarlar bu kayıttan öğrenecek. Birden çok kişi etiketliyorsa `--kullanici ad` ile başlat (varsayılan:
  bilgisayardaki oturum adı). Eski `emek.jsonl` ve "İnsan emeği" özeti aynen sürer.
- Başlıkta "v3.7" yazar.

## v3.6'da neler değişti

- **Donma düzeltildi.** Arayüz her 2 saniyede bir durum bilgisi ister. v3.5'te bu istek, etiket klasörünü her resim için
  bir kez baştan tarıyordu (1.000 karede yaklaşık 1.100 tarama) ve bu sırada kaydetme ile kare geçişi bekliyordu.
  Artık klasör 2 saniyede en fazla bir kez taranır. 1.000 karelik denemede istek 887 ms'den 6 ms'ye indi.
- **Kılavuz çizgileri hafifledi.** Görüntü üç katmanda çizilir: resim, kutular, kılavuz çizgileri ve büyüteç.
  Fare kıpırdayınca yalnızca en üst katman çizilir; 4K resim yalnızca yakınlaştırma ve kaydırmada yeniden çizilir.
- **Kopya ve grup kutular azaldı.** Model önerileri sana gösterilmeden önce iki filtreden geçer:
  grup kutusu filtresi (yan yana iki nesneyi birden içine alan büyük kutuyu atar) ve kopya kutu sınırı
  (aynı sınıftan %50'den fazla örtüşen iki öneriden düşük güvenlisini atar; v3.5'te bu sınır %70 idi).
  Eski modellerin önerilerine de uygulanır; M etiketlerine uygulanması için Model sekmesinde "M etiketlerini yenile"ye bas.
  Parça birleştirme de daha temkinli: kesik bir nesne yanındaki başka bir nesneyle tek kutuda birleşmez.
  Öğrenme eğrisi eski turlarla karşılaştırılabilsin diye ham model çıktısıyla ölçülmeye devam eder.
- **Kutuyu düzgün çizmek kolaylaştı.** Büyüteç (B ile aç/kapat), ok tuşlarıyla 1 ve 10 piksel kaydırma,
  Alt + ok ile kenar boyutlandırma, Tab ile üst üste binmiş kutular arasında gezinme.
  Dikkat: kutu seçiliyken → ve ← artık kutuyu kaydırır; sonraki kare için D'ye bas (ya da önce Esc ile seçimi kaldır).
- **Kılavuz sayfası.** Model sekmesindeki "Kılavuz" düğmesi yeni sekmede açılır: TP, FP, FN, precision, recall, F1,
  AP, mAP50, mAP50-95, karışıklık tablosu, MC Dropout ve Deep Ensemble; hepsi senin modelinin test sonuçlarıyla.
  Güven eşiği kaydırıcısıyla her eşikte sonuçları görürsün. v3.5 ile eğitilmiş modeller test tahminlerini
  saklamıyordu: sayfadaki "Test tahminlerini hazırla" düğmesi son modelle test karelerini bir kez tahmin eder
  (40 karede yaklaşık 1-2 dakika). Sonraki turlarda bu kendiliğinden yapılır.
- **Küçük işler.** Model sekmesinin başında sade bir "Kısaca" özeti var. Öneri listesindeki puanın üstüne gelince
  hesabı görünür (öncelik = 0,7 × kararsızlık + 0,3 × titreme). Başlıkta "v3.6" yazar.
- **Yeni ayarlar** (Gelişmiş): `oneri_nms` (kopya kutu sınırı, 0,5) ve `grup_filtre` (açık).

## Dosyalar

- app.py: sunucu (arayüz, ayarlar, otomatik eğitim tetikleme)
- egitim.py: eğitim işçisi (sunucu kendisi başlatır, elle çalıştırma)
- ortak.py: ortak hesaplar (parçalama, mAP, titreme)
- davranis.py: kullanıcı davranışı kaydı (v3.7): kutu farkı, davranis.jsonl, sınıf özeti
- testler/: birim testleri (`python -m pytest testler -q`, pytest gerekir)
- static/index.html: arayüz
- static/kilavuz.html: Kılavuz sayfası (ölçütler ve canlı sonuçlar)
- sinif_sirasi_duzelt.py: sınıf sırasını Teknofest sırasına (tasit=0, insan=1) çevirir

## Çalıştırma

    python app.py --images "C:/veri/resimler"

Tarayıcı: http://127.0.0.1:5000

Etiketler resimlerle AYNI klasörde durur: kare_1.jpg yanında kare_1.txt, sınıf listesi classes.txt.
Ayrı etiket klasörü istersen --labels ile verebilirsin. Modeller, tahminler ve ayarlar
`<resim klasörü>_calisma` klasörüne yazılır (değiştirmek için --calisma).

## LabelImg (.xml) etiketleri

Klasörde kare_1.jpg yanında kare_1.xml (Pascal VOC, LabelImg'in varsayılanı) ya da kare_1.json (LabelMe) varsa program onu kendiliğinden
YOLO formatına (kare_1.txt) çevirir; .xml olduğu gibi kalır. Sınıf adları classes.txt ile eşlenir
("Taşıt" = "tasit", "person" = "insan", "car" = "tasit" gibi); tanımadığı ad (örn. UAP) yeni sınıf olarak eklenir.
Bir seferde 5'ten fazla kare içe aktarılırsa otomatik eğitim bekletilir: birkaç kareye bakıp sınıflar doğruysa
Model sekmesinde "Otomatik eğitimi sürdür"e bas. .xml'i sonradan değiştirirsen aynı adlı .txt'yi sil, yeniden aktarılır.

## "Etiketleyeyim mi?" (toplu model etiketi)

Model eğitildikten sonra Model sekmesinde "Model, etiketsiz N kareye kutu önerdi. Bu kareleri etiketleyeyim mi?"
kartı çıkar. Evet dersen bu kareler modelin kutularıyla etiketlenir ve M ile işaretlenir. M kareler sen açıp
D ile onaylayana kadar eğitimde kullanılmaz (model kendi hatalarını öğrenmesin diye). Her yeni modelden sonra M kareler
yeni önerilerle kendiliğinden güncellenir; eşiği değiştirince "M etiketlerini yenile" ile hemen güncelleyebilirsin. MODEL ÖNERDİ listesi M kareleri
en şüpheliden başlayarak gösterir. Test, tampon ve denetim kareleri asla model tarafından etiketlenmez.

## Test seti nasıl seçilir

Program ilk açılışta sorar: test seti kaç kare olsun, nereden (etiketli karelerinden ya da tüm karelerden) ve
hangi kısımdan (sondan, baştan ya da karışık: videonun 4 farklı yerinden bloklar). Model sekmesi > Sayılar > "değiştir"
ile sonradan da değiştirilebilir; o zaman öğrenme eğrisi sıfırlanır.
Onaylayana kadar eğitim başlamaz; onaylayınca test seti sabitlenir. Her test karesinin önündeki ve arkasındaki
10 kare "tampon"dur, hiç kullanılmaz (test karesine çok benzeyen kareler eğitime girmesin).

## Hazır etiketli veri eklemek

Etiketli resimleri (aynı adlı .jpg + .txt çiftleri) uygulama açıkken bile klasöre atabilirsin.
Birkaç saniye içinde listeye girerler; açınca kutuları görünür ve eğitimde kullanılırlar.
Sınıf numaraları classes.txt ile aynı sırada olmalı (şu an insan=0, tasit=1).
Eşikler (ilk eğitim, tur büyüklüğü) elle değiştirmediysen güncel resim sayısına göre kendiliğinden ayarlanır.
Test seti: test karelerinden hiçbirini henüz etiketlemediysen, resim ekledikçe son %10'a göre yeniden seçilir;
etiketlemeye başladığın an ve ilk başarılı turdan sonra sabitlenir.

## Akış

1. Test seti: son %10 kare (ETİKETSİZ listesinin en üstünde, T rozetli). Elle, dikkatle etiketle. Eğitime hiç girmez,
   sadece her turdan sonra başarıyı (mAP) ölçmek için kullanılır. Hemen önündeki 10 kare "tampon"dur, hiç kullanılmaz.
2. Tohum kareler: ETİKETSİZ listesi test karelerinden sonra eşit aralıklarla seçilmiş 100 kare gösterir. Bunları elle etiketle.
3. Eşik dolunca (ve test seti tamamsa) ilk eğitim kendiliğinden başlar. Beklemeden başlatmak için Model sekmesinde "Şimdi eğit".
4. Model bitince kareler MODEL ÖNERDİ listesine geçer: en kararsız kareler önce. Model önerileri kesikli çizgiyle gelir.
   Yanlışları sil, eksikleri ekle, D ile onayla. Boş bir yere tıklarsan modelin o noktadaki aday kutusu eklenir.
5. Her "tur büyüklüğü" kadar yeni karede model önceki modelden devam ederek yeniden eğitilir.
   Eğitim sürerken etiketlemeye devam edebilirsin.
6. Model sekmesindeki öğrenme eğrisi düzleşince (art arda 3 turda artış 0,01'den az) otomatik eğitim durur.

Kare türleri: T = test, – = tampon, D = denetim (rastgele seçilir, model önerisi bilerek gizlenir).

## Kısayollar

W: tek kutu çiz (LabelImg gibi; kutu bitince düzenleme moduna döner) · S: düzenle modu · Q / E: önceki / sonraki sınıf
X: kılavuz çizgileri (imleçten resmin iki ucuna uzanan çizgiler) · C: model adayları · D: kaydet + sonraki
Ok tuşları: seçili kutuyu 1 piksel kaydır (Shift: 10) · Alt + ok: sağ / alt kenarı büyüt-küçült
Tab / Shift + Tab: kutular arasında gez (imleç üst üste kutuların üstündeyse yalnızca onlar arasında)

## Ayar komutu ve kopyalama

Ayarlar penceresinde "Ayarları kopyala" tüm ayarları okunur biçimde panoya kopyalar (yardım isterken yapıştır).
Alttaki "Ayar komutu" kutusuna `ayar = değer` satırları yapıştırıp "Uygula"ya basınca ayarlar değişir, sonuç satır satır yazılır.
Örnek:
    conf_on_etiket = 0.45
    tur_epoch = 30

## Ayarlar (Model sekmesi > Ayarlar)

Her ayarın altında açıklaması vardır. En önemlileri:

- İlk eğitim eşiği (100), tur büyüklüğü (verinin %5'i)
- İlk epoch (100) ve tur epoch (30) üst sınırdır: doğrulama başarısı 10 epoch üst üste artmazsa
  (erken durdurma sabrı) eğitim kendiliğinden biter, yani model öğrenmeyi bırakana kadar eğitilir.
- Model: yolo11n / yolo11s (6 GB için önerilen), parça boyutu (640), batch (-1 = otomatik)
- Ön etiket güven eşiği (0,25), seçim yöntemi (aktif / rastgele), kararsızlık ve titreme ağırlıkları (0,7 / 0,3)

Model, parça boyutu ya da sınıf listesi değişirse sonraki tur sıfırdan (ilk epoch ile) eğitilir.
Test seti ilk başarılı turdan sonra kilitlenir; öğrenme eğrisi hep aynı test setiyle ölçülsün diye.

## Rastgele ve aktif seçimi karşılaştırma deneyi

Etiket klasörünün iki kopyasını al (aynı test ve tohum etiketleriyle). Birini aktif, diğerini rastgele seçimle,
farklı resim klasörleriyle (kopya) çalıştır. Model sekmesinden iki eğriyi CSV olarak indirip Excel'de karşılaştır.

## Sorun giderme

- "Ekran kartı kullanılamıyor": PyTorch'un CUDA sürümü kurulu değil. Kontrol:
  python -c "import torch; print(torch.cuda.is_available())"  → True olmalı.
- yolo11n.pt'yi terminalden indirmek (yarıda kalırsa aynı komut kaldığı yerden devam eder):
  curl.exe -L -C - --retry 20 -o yolo11n.pt https://github.com/ultralytics/assets/releases/download/v8.4.0/yolo11n.pt
  Doğru boyut: 5613764 bayt. "Model dosyası bozuk" hatasında dosyayı silip yeniden indir.
- Bellek hatası: Ayarlar'da batch'i 4 yap ya da daha küçük model seç (işçi önce kendisi küçültmeyi dener).
- Bilgisayar eğitimde çok yavaşlıyorsa: Ayarlar'da workers değerini 2 ya da 0 yap.
- Her turun ayrıntılı kaydı: <labels>_calisma/turlar/tur_XXX/log.txt
