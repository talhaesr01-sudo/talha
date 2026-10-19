# Faz 1 raporu: kod analizi, arayüz sadeleştirme, davranış kaydı (v3.7)

Faz 1'in kapsamı: mevcut kodun analizi, büyütecin kaldırılması, sol menünün üç kategoriye indirilmesi, tek tıkla
onay ve kullanıcı davranışı kayıt altyapısı. Eğitim işçisine (`egitim.py`), modele, etiket dosyası biçimine ve
eğitim verisine dokunulmadı.

Ayrıntılı analizler `docs/analiz/` klasöründe (her biri ayrı bir ajanla çıkarıldı, ikinci bir ajanla koda karşı
doğrulandı): `aktif-ogrenme.md`, `pseudo-label.md`, `egitim-isci.md`, `arayuz.md`, `mimari-tekrar.md`,
`literatur.md`.

---

## 1. Mevcut kod analizi ne gösteriyor?

**Aktif öğrenme.** Öncelik kare başına `a_u × kararsızlık + a_t × titreme` (varsayılan 0,7 / 0,3).
- Kararsızlık, ham YOLO güveninden kutu başına `1 − |2p − 1|`; karenin değeri en kararsız 3 kutunun ortalaması.
- Titreme, ardışık karelerde "var-yok-var / yok-var-yok" olaylarının oranı (yalnızca video verisinde anlamlı).
- Çeşitlilik (diversity), sınıf dengesi ve temsil (representativeness) yok. Ardışık, neredeyse aynı kareler sırada
  yan yana çıkabiliyor.
- Kutu bulunamayan kare `u = 0` alıp sıranın sonuna düşüyor ("kaçırma körlüğü"); bunu yalnızca titreme kısmen telafi
  ediyor.
- Sıra yalnızca tur sonunda ve ayar kaydedilince kuruluyor; etiketleme sırasında güncellenmiyor.

**Pseudo label (M etiketleri).** "Etiketleyeyim mi?" ile model, `conf_on_etiket` (0,25) üstündeki kutularını
etiket dosyası olarak yazıyor ve kareyi M ile işaretliyor.
- M kareleri kullanıcı D ile onaylayana kadar eğitime girmiyor (iyi). Her yeni modelden sonra yeniden yazılıyorlar.
- Kalite skoru yok; toplu etiketlemede en kararsız kareler de etiketleniyor.
- M işaretinin tek kaynağı `durum.json`; dosya bozulursa M etiketleri sessizce eğitime girer.

**Güven (confidence).** Ham YOLO güveni her yerde doğrudan karar veriyor: öneri eşiği, aday eşiği, kararsızlık
hesabı, titreme varlığı, M etiketi yazımı. Kalibrasyon yok. MC Dropout ve Deep Ensemble kodda hiç yok; yalnızca
Kılavuz sayfasında anlatılıyor.

**Önerilerin sıralanması.** Düz sıralama (büyükten küçüğe). Denetim (D) kareleri ilk "tur büyüklüğü" kadar karenin
arasına serpiştiriliyor. M kareleri etiketsizlerle aynı skorla karışık sıralanıyor.

**Teacher-Student için hazır veri.** Her turun `best.pt` dosyası ve etiketsiz karelerin tahmin JSON'ları diskte
duruyor; hiçbiri silinmiyor. Son model ile önceki modelin karşılaştırması ek GPU çalıştırmadan yapılabilir. Faz 3
için en önemli bulgu bu.

**GPU.** Sunucu GPU kullanmıyor. Bir sunucuda aynı anda en çok bir işçi süreci (eğitim ya da Kılavuz analizi)
çalışıyor; eğitim, test ve tahmin aynı süreçte sırayla yapılıyor. Süreçler arası kilit yok: uygulama eğitim
sürerken kapatılıp açılırsa eski işçi ile yenisi GPU'yu paylaşabilir.

**Kod tekrarı.** En belirginleri: alt süreç başlatma (2 yer), açgözlü IoU eşleştirme (3 yer), YOLO ↔ piksel
dönüşümü (4+ yer), CSV dışa aktarımı (3 yer; Faz 1'de tek yardımcıya toplandı), ayar tanımlarının sunucu ve
istemcide ayrı ayrı yazılması.

## 2. Beş soru

**1. Mevcut kod analizi ne gösteriyor?** Yukarıdaki bölüm. Kısaca: karar mekanizmasının her yerinde ham güven var;
çeşitlilik ve sınıf dengesi yok; M etiketlerinin kalite skoru yok; Teacher-Student için gereken veri zaten diskte.

**2. Bu değişiklik mevcut yapıyı bozar mı?** Faz 1 eğitime, tahmine, etiket dosyası biçimine ve `durum.json`'a
dokunmuyor.
- `emek.jsonl`, `/api/emek.csv` ve "İnsan emeği" özeti aynen sürüyor.
- Eski sayfa (v3.6) açık kalmışsa sunucu kaydı kabul ediyor; yalnızca davranış kaydı yazılmıyor.
- localStorage'daki eski liste adları yeni kategorilere eşleniyor.
- **Bilerek yapılan tek davranış değişikliği:** D (ve ✓ düğmesi) dokunulmamış boş kareyi artık "nesne yok" diye
  onaylıyor. v3.6'da atlıyordu. Atlamak için → kullanılıyor; → v3.6'daki D ile aynı davranıyor.

**3. Daha performanslı bir yöntem var mı?**
- Davranış kaydı için SQLite düşünüldü. Yeni bağımlılık ve kilit sorunu getirir; ekleme-yalnız JSON Lines (JSONL)
  100 binlerce kayda kadar yeterli ve okuma dosya değişmedikçe önbellekten dönüyor.
- Kutu eşleştirmede Macar algoritması (Hungarian) en iyi eşleşmeyi verir. Ama VOC/COCO değerlendirmesi de
  açgözlü eşleştirme kullanıyor ve IoU ≥ 0,5 ile birkaç kutuda fark ihmal edilebilir. Açgözlü yöntem O(n·m) ve
  bağımlılıksız.
- Kategori sayımı O(N) ve ucuz (10.000 karede ≈ 0,1 ms). Pahalı olan sol listenin DOM'unu baştan kurmaktı (10.000
  satır ≈ 400 ms, her D'de iki kez). Çözüm: liste yalnızca değişince baştan çizilir, kare geçişinde yalnızca iki satır
  yenilenir (ölçüm aşağıda).

**4. Literatürde daha iyi bir yöntem var mı?** Faz 1 bir algoritma değil, altyapı. Kayıt biçimi şu yöntemlerin
ihtiyacına göre tasarlandı:
- Confident Learning (Northcutt ve ark., JAIR 2021): "verilen etiket – model tahmini" ortak dağılımı. Kayıt her
  model kutusunun akıbetini sınıf bazında tutuyor.
- Kabul olasılığına göre "doğrulat mı, çizdir mi?" kararı (Konyushkova ve ark., CVPR 2018): kullanıcının
  doğrulama ve çizim süreleri gerekiyor. Kayıt kare süresini ve işlem türlerini ayrı tutuyor.
- Unutma olayları / tahmin kararlılığı (Toneva ve ark., ICLR 2019): her kayıtta önerinin hangi turdan geldiği var.

**5. RTX3050 6 GB bunu kaldırır mı?** Faz 1 GPU kullanmıyor. Sunucu yalnızca numpy kullanıyor. Büyütecin
kaldırılması tarayıcıda sürükleme sırasında her fare hareketindeki 4K görüntü kesitinin yeniden çizimini kaldırdı.

---

## 3. Değişiklik raporu

### Amaç / neden gerekli
- **Büyüteç:** kullanıcı isteği. Kutu çizerken imlecin yanında açılan katman dikkat dağıtıyordu.
- **Üç kategori:** altı liste seçeneği (öneri sırası, tüm kareler, etiketliler, M, etiketsizler, test seti)
  kullanıcıya "şimdi neye bakmalıyım?" sorusunu bırakıyordu. Üç kategori, karenin iş akışındaki yerini söyler.
- **Tek tıkla onay ve otomatik liste geçişi:** "kullanıcı mümkün olduğunca az tıklasın".
- **Davranış kaydı:** sonraki fazlardaki öğrenme (öncelik, sınıf eşikleri, zor örnekler) kullanıcının düzeltmelerine
  dayanacak. Bu veri olmadan Faz 2–6 kör çalışır.

### Eklenen algoritmalar ve literatür gerekçesi
| Yöntem | Nerede | Kaynak |
|---|---|---|
| Açılıştaki ve kayıttaki kutuların sınıftan bağımsız açgözlü IoU eşleştirmesi, eşik 0,5 | `davranis.kutu_farki` | Everingham ve ark., "The PASCAL VOC Challenge", IJCV 2010 |
| Model kutusunun akıbetini sınıf bazında tutma (onay / düzeltme / sınıf değişimi / silme; kaçırılan nesne) | `davranis.kutu_farki`, `sinif_ozeti` | Northcutt, Jiang, Chuang, "Confident Learning", JAIR 2021 (fikir; algoritma Faz 5'te) |
| Sınıf oranlarında Beta önselli toplamsal yumuşatma + en az 20 örnek kuralı | `davranis.sinif_ozeti` | Standart empirical Bayes; bkz. Kim ve Ghahramani, "Bayesian Classifier Combination", AISTATS 2012 |
| Tek tuşla doğrulama (onay) akışının öne çıkarılması | `index.html` ✓ düğmesi, D | Papadopoulos ve ark., "We don't need no bounding-boxes", CVPR 2016 (doğrulama çizimden 6–9 kat hızlı) |
| Kare süresi ve işlem türlerinin ayrı ayrı ölçülmesi | `index.html` emek sayaçları | Papadopoulos ve ark., "Extreme clicking", ICCV 2017; Konyushkova ve ark., CVPR 2018 |

Literatüre dayanmayan, kendi seçimim olan sabitler: "geometrisi değişmedi" eşiği `DEGISMEDI_IOU = 0,95`
(kenarlarda ≈ %2–3 kayma), önsel gücü 5, en az örnek 20, onay koruması 350 ms.

### Neler değişti
- `static/index.html`
  - Büyüteç (çizim fonksiyonu, B kısayolu, ayarı) kaldırıldı. Fare imleçleri (artı, taşıma, boyutlandırma) aynen.
  - Sol menü: MODEL ÖNERDİ / ETİKETSİZ / ONAYLANDI, yanlarında sayıları. Kategoriye tıklayınca listenin ilk karesi
    açılır.
  - ETİKETSİZ sırası: önce test kareleri, sonra tohum kareleri, sonra zaman sırası. Tampon kareler gösterilmez.
  - Başlıkta "✓ Onayla · sonraki" düğmesi; başlık metni sadeleşti ("N fotoğraf · %X onaylandı").
  - Liste sonunda önce atlanan karelere dönülür, iş listesi gerçekten bitince öteki iş listesine geçilir.
  - Çift tık / hızlı D-D koruması; D'nin anlamı (bkz. soru 2).
  - Davranış sayaçları: eklenen, model adayından eklenen, silinen, taşınan, boyutu değişen, sınıfı değişen, geri alma.
- `davranis.py` (yeni): kutu farkı, kayıt oluşturma, `Gunluk` (kilitli ekleme, önbellekli okuma), ziyaret bazlı
  tekilleştirme, sınıf özeti, CSV satırları.
- `app.py`: kaydetmede davranış kaydı; `/api/davranis.csv`, `/api/davranis_ozet`; `--kullanici`; test seti
  değişince öneri sırası yeniden kurulur, denetim kareleri role göre süzülür; CSV dışa aktarımı ortak yardımcıda.
- `ortak.py`: `csv_metni`, `tarih_metni`; `metin_yaz` geçici dosya adı iş parçacığına özgü.
- `OKUBENI.md`, `static/kilavuz.html`: v3.7 anlatımı.
- `testler/test_davranis.py` (yeni): 15 birim testi.

### Performans etkisi
Ölçüm: 10.000 kare, model yok (tohum aşaması, ETİKETSİZ listesi 9.990 kare), başsız Chromium.

| İşlem | Önce (Faz 1 ilk hâli) | Sonra |
|---|---|---|
| D (onayla + sonraki), medyan | ≈ 1.000 ms (inceleyicinin ölçümü) | 67 ms |
| Kare geçişinde sol liste güncellemesi | ≈ 400 ms (tam çizim) | 11 ms |
| Kategori değiştirme (tam çizim) | ≈ 400 ms | 111 ms |

v3.6'da aynı aşamada varsayılan liste yalnızca 100 tohum karesiydi ve D 36–55 ms sürüyordu.

- Kaydetme başına davranış kaydı: n × m IoU matrisi (birkaç kutuda mikrosaniyeler) + bir satır ekleme.
- Sürüklerken her fare hareketindeki büyüteç çizimi (4K kaynaktan 176 × 176 kesit) kalktı.

### Bellek / VRAM, RTX3050 uygunluğu
VRAM etkisi yok. RAM: davranış kayıtları okunduğunda bellekte tutuluyor (10.000 kayıt ≈ birkaç MB). Disk: kayıt
başına ≈ 0,5–2 KB (kalabalık karede daha çok).

### Kod karmaşıklığı
Yeni modül ≈ 230 satır (tek sorumluluk: davranış kaydı). `index.html`'de büyüteç (≈ 40 satır) çıktı, kategori ve
gezinme mantığı (≈ 80 satır) girdi. Sabitler dosya başlarında adlandırıldı (`BOSTA_SINIR_MS`, `OK_GRUP_MS`,
`EN_AZ_GORME_MS`, `ESLESME_IOU`, `DEGISMEDI_IOU`, `ONSEL_GUCU`, `MIN_ORNEK`).

### Kullanıcı deneyimi etkisi ve tahmini etiketleme süresi kazancı
- Kutusuz karede "Ctrl+S, sonra D" yerine tek D.
- Liste sonunda elle liste değiştirme yerine otomatik geçiş.
- Kategori + kare yerine tek tık.
- Ölçülmüş bir süre kazancı yok; bunu ölçecek altyapı (davranış kaydı) bu fazda kuruldu. Asıl kazanç önerilerin
  daha iyi sıralanması ve güvenilir kutuların kısa kontrolle geçmesiyle (Faz 2–3) beklenir.

### Beklenen mAP etkisi
Doğrudan yok. Dolaylı: test seti değişince test / tampon karelerinin öneri listesine sızması engellendi; bu, test
setinin temiz kalmasına (doğru ölçüm) yardım eder.

### Riskler
- D'nin anlamı değişti (bkz. soru 2). Alışkanlıkla D'ye basıp boş kare atlayan kullanıcı o kareyi "nesne yok" diye
  onaylar. OKUBENI'de "Dikkat" maddesi var.
- 350 ms koruması kasıtlı çok hızlı bir onayı yok sayabilir (kullanıcı tekrar basar).
- Tampon kareler artık hiçbir listede görünmüyor (zaten kullanılmıyorlardı).
- Davranış kaydının doğruluğu istemcinin gönderdiği açılış kutularına dayanıyor. Sunucu sınırlıyor ve doğruluyor
  ama içeriği bağımsız olarak doğrulayamıyor.

### Test senaryoları
- Birim (15): kutu farkı (onay / düzeltme / sınıf / silme / yeni), açgözlü eşleştirmenin tekliği, bozuk girdi,
  alan sınırları, günlüğe ekleme-okuma, yarım satır ve yarım çok baytlı karakter, ziyaret tekilleştirme, ara kayıt,
  elle karedeki kutunun "kaçırıldı" sayılmaması, sınıf özeti yumuşatması, CSV biçimi.
- Tarayıcı uçtan uca (sahte veri: 40 kare, 5 kullanıcı etiketi, 2 M karesi, 4 test, 4 tampon, sahte tur 1),
  iki betik toplam 43 kontrol:
  - eski liste adının eşlenmesi, üç kategori ve sayıları, büyütecin yokluğu (kod ve çizim katmanı), B tuşunun etkisizliği;
  - tek tık onay, çift tıkta tek kayıt, D ile boş kare onayı, → ile atlama, liste sonunda başa sarma,
    otomatik liste geçişi, liste yeniden kurulduktan sonra A ile dönüş;
  - davranış kaydı alanları, işlem sayaçları, kutu farkı, ara kayıtta açılış tabanının korunması, emek.jsonl
    uyumluluğu, aynı sınıf tuşunun sınıf değişimi sayılmaması, ilerleme yüzdesi, JavaScript hatası olmaması.
  - Kalıcı bir başarısızlık yok. Bir kontrol zamanlamaya bağlı: arayüz durumu 2 sn'de bir yoklandığı için bazen eski
    sayıyı okuyor; sunucunun sayısı doğru (4/4).
- Gerçek YOLO eğitimi bu ortamda çalıştırılmadı (ekran kartı ve gerçek veri yok); Faz 1 eğitime dokunmuyor.

### Gelecekte genişletilebilirlik
- `davranis.sinif_ozeti` Faz 2'deki sınıf dengesine ve Faz 6'daki sınıfa göre eşik ayarına doğrudan girdi verir.
- Kayıtlar kullanıcı ve oturum taşıdığı için kişiselleştirme profili kullanıcı başına kurulabilir.
- Kutu kayıtlarında güven, boyut ve tur var: kalibrasyon (Faz 4) ve kabul olasılığı modeli için hazır veri.

---

## 4. Çapraz inceleme

Faz 1 değişiklikleri 4 boyutta incelendi (liste ve gezinme, davranış kaydı, geriye uyumluluk, arayüz ve performans).
Her bulgu ayrı bir ajanla çürütülmeye çalışıldı. 24 bulgudan 5'i ayakta kaldı, hepsi düzeltildi. Çürütülenlerin
bir kısmı v3.6'da da olan davranışlardı. Bunların en önemlileri (çift tıkta çift kayıt, boş kareyi onaylayamama,
aynı sınıf tuşunun sınıf değişimi sayılması) yine de düzeltildi.

| Bulgu | Önem | Durum |
|---|---|---|
| Liste sonu yalnızca ileriye bakıyor; kare kalmışken "liste bitti" deyip liste değiştiriyor | orta | düzeltildi: başa sarma |
| Ara kayıttan (Ctrl+S) sonraki düzeltmeler model istatistiğinden düşüyor | orta | düzeltildi: ziyaret bazlı kayıt |
| 10.000 karede her D ≈ 1 sn (sol liste baştan kuruluyor) | orta | düzeltildi: artımlı güncelleme |
| Kirli karede kategori değişince kayıt yanlış liste adıyla yazılıyor | düşük | düzeltildi: liste ziyaret başında alınır |
| Dar ekranda "Sadece çiz" düğmesi kırpılıyor | düşük | düzeltildi: `.modes { flex: none }` |

Analizden ayrıca düzeltilenler: test seti değişince eski öneri sırasının yeniden kurulmaması (test ve tampon kareleri
MODEL ÖNERDİ'ye düşüyordu), denetim karelerinin role göre süzülmemesi, `metin_yaz`'ın geçici dosya adının iş
parçacığına özgü olmaması (aynı dosyaya eş zamanlı yazımda 500 hatası).

## 5. Analizde bulunan, sonraki fazlara bırakılan sorunlar

| Sorun | Kaynak | Faz |
|---|---|---|
| Denetim (D) karelerinde öncelik puanı ve öneri sayısı görünüyor (önerinin gizlenmesi ilkesiyle çelişiyor) | `aktif-ogrenme.md` §6.8 | 2 (puan yerine yıldız; D'de gizli) |
| Kutu bulunamayan kare sıranın sonuna düşüyor (kaçırma körlüğü) | `aktif-ogrenme.md` §6.2 | 2 ve 5 |
| Çeşitlilik ve sınıf dengesi yok | `aktif-ogrenme.md` §6.4–5 | 2 |
| M karesi denetim karesi seçilebiliyor; test seti sıfırlanınca M karesi test karesi olabiliyor | `pseudo-label.md` §9.1, §9.12 | 3 |
| Toplu M etiketlemede kalite filtresi yok | `pseudo-label.md` §9.10 | 3 |
| Kaydetme ile M yenilemesi arasında yarış (model kutuları "onaylı" sayılabilir) | `pseudo-label.md` §9.15 | 3 |
| Açık M karesi yeni modelle yenilenmiyor | `pseudo-label.md` §9.13 | 3 |
| Ham güven her kararda | `pseudo-label.md` §6 | 4 |
| Uygulama yeniden açılınca yetim işçi süreci kontrolü yok; eğitimden sonra VRAM boşaltma etkisiz | `egitim-isci.md` §3, §6 | 6 (egitim.py değişirken) |
| Her turun tahminleri ve ağırlıkları hiç silinmiyor (disk büyümesi) | `egitim-isci.md` §6.7 | ayrı iş |
