# Eğitim işçisi, arka plan işleri, GPU/VRAM ve önbellek: doğrulanmış kod okuma raporu

> **Satır numaraları:** Numaralar v3.6 commit'i `da8f5c0` içindir. Doğrulamayı bu commit'ten çıkardığım kopyalar üzerinde yaptım: `/tmp/claude-0/-home-user-talha/2c30178d-d993-51d1-9278-3f9d7242faf6/scratchpad/v36/`. Önceki analizde "çalışma kopyasında değişiyor" denen v3.7 değişiklikleri artık HEAD `56f2a51` olarak commit'li ve çalışma ağacı temiz.
> - `egitim.py` ve `ortak.py` iki sürümde aynı.
> - `kilavuz.html` içinde yalnızca 314. satır değişmiş ("büyüteç" yerine "yakınlaştırma"). b8 bölümü aynı.
> - HEAD'deki `/home/user/talha/etiketleyici_v3_6/app.py` satırları şu kadar kayıyor: 37–~697 arası **+1**, ~717–~730 arası **+4**, ~755–~1222 arası **+29**, ~1223–1235 arası **+30**, ~1236 sonrası **+32**. Örnekler: `tur_baslat` 383→384, `_analiz_hesapla` 956→985, `main` 1212→1241, yarım tur bloğu 1248→1280.
> - HEAD `OKUBENI.md` içinde "MC Dropout" satırı 23→48'e kaydı.
>
> Hiçbir proje dosyasını değiştirmedim.

## 1. `egitim.py`: `Isci` sınıfı ve tur akışı

- **Giriş:** `main()` `--is` ya da `--analiz` alır (`egitim.py:450-472`).
  - `--is` ile `calis()` çalışır (`egitim.py:430-447`), `--analiz` ile `analiz()` çalışır (`egitim.py:419-428`).
  - `Isci(...)` nesnesi `try` bloğunun **dışında** oluşturuluyor (`egitim.py:457`). İş dosyası okunamazsa `SystemExit` fırlar (`egitim.py:99-100`) ve `sonuc.json` yazılmaz. Sunucu bu durumu "beklenmedik kapanış" olarak görür (`app.py:525-527`).
- **Kurulum (`__init__`):** İş dosyasından `ayarlar`, `cikti`, `durum_dosyasi`, `iptal_dosyasi`, `images`, `labels`, `siniflar`, `P` (parça boyutu) ve `ov` (örtüşme) okunur (`egitim.py:97-115`).
- **Cihaz (`cihaz_hazirla`):**
  - Ayardaki cihaz kullanılır. Ayar boşsa CUDA varsa `"0"`, yoksa `"cpu"` seçilir (`egitim.py:411-412`).
  - `tahmin_batch` GPU'da 16, CPU'da 4 parçadır (`egitim.py:415`).
  - GPU'da tahmin FP16 yapılır. `DEFAULT_CFG_DICT` içinde `quantize` varsa `quantize:16`, yoksa `half:True` gönderilir (`egitim.py:61-73`, `egitim.py:416`).
  - Not: Görünen cihaz adı her zaman `get_device_name(0)` ile alınıyor (`egitim.py:413`). Cihaz `"1"` seçilse bile 0. kartın adı yazılır.
  - `--device` seçeneği `ayarlar.json` dosyasına yazılır (`app.py:1234-1237`).
- **1) Veri hazırlama ve parçalama:**
  - `veri_hazirla`, parça kökünde (`parca_kok`) bulduğu `*.cache` dosyalarını her turda siler (`egitim.py:190-191`). Ardından eğitim ve doğrulama karelerini parçalar (`egitim.py:192-196`).
  - Doğrulama parçası çıkmazsa eğitim listesinin **ilk** %10'u (en az 1) doğrulama olarak kullanılır (`egitim.py:199-200`). Bu parçalar eğitimde de kalır. Seçim rastgele değil ve doğrulama eğitimle örtüşür.
  - `egitim.txt`, `dogrulama.txt` ve JSON biçimli `veri.yaml` yazılır (`egitim.py:201-206`).
  - `parcala`:
    - Bir kutu parçada en az 2×2 piksel ve alanının en az %25'i görünüyorsa o parçaya etiket olarak yazılır (`egitim.py:159-163`).
    - Etiketsiz parçalar `r < bos_parca_orani` ile seçilir (`egitim.py:164`).
    - `r`, `guvenli_ad(ad, dosya boyutu)` ile tohumlanan bir rng'den her parça için çekilir (`egitim.py:146-147,151`). Bu yüzden seçim turlar arasında sabit kalır.
    - %25'ten az görünen nesne parçaları, parça seçilse bile etiketsiz kalır ve arka plan sayılır (`egitim.py:160`).
  - Parça ızgarası `O.parcalar` / `O.parca_konumlari` ile kurulur (`ortak.py:216-229`).
- **2) Eğitim (`egit`):**
  - Model `YOLO(baslangic_agirlik)` ile yüklenir (`egitim.py:242`).
  - Eğitim parametreleri (`egitim.py:246-251`): `imgsz=P`, `batch`, `workers`, `plots=False`, `patience`, `seed=0`, `deterministic=False`, `flipud`, `close_mosaic=min(10, E//2)`, `name=egitim{deneme}`.
  - Batch varsayılanı -1'dir (`app.py:63`). CPU'da -1 verilirse 8 kullanılır (`egitim.py:236-237`).
  - Callback'ler iptali her batch'te kontrol eder. Durum dosyasına en çok saniyede bir yazarlar; epoch sonunda yazma zorlanır (`egitim.py:212-227`).
  - OOM olursa en çok 3 kez yeniden dener; batch 1 ise denemez. Batch -1 ise 8'e iner, değilse yarıya iner (`egitim.py:255-262`).
  - Dönen ağırlık `best.pt`, o yoksa `last.pt` olur (`egitim.py:264-268`).
- **3) Test değerlendirmesi:**
  - `test_tahmin_et` test karelerini `conf=0.01`, `max_det=100` ile **bir kez** tahmin eder (`egitim.py:343`). `max_det` her `predict` görüntüsü için, yani parça başınadır.
  - Parçalar birleştirilir ama temizlenmez. Kutular 0,1 piksel ve 4 hane skorla `test_tahmin.json` dosyasına yazılır (`egitim.py:345-348`).
  - `test_degerlendir` önce ham metriği (`egitim.py:358`), sonra aynı tahminlere `temizle` uygulayarak "temiz" P/R/mAP50 değerini hesaplar (`egitim.py:364-368`). Yeni çıkarım yapmaz.
- **4) Etiketsiz kareleri tahmin (`tahmin_et`):**
  - Her kare `conf_aday` ile tahmin edilir; varsayılan 0,05 (`app.py:72`). Sonra `temizle` uygulanır (grup kutusu ve sınıf başına NMS) ve en çok 500 kutu tutulur (`egitim.py:381-383`).
  - Her kare için `tahmin/kareler/<stem>.json` yazılır: `{"w","h","k":[[x1,y1,x2,y2,cls,skor]],"temiz":true}`. Koordinatlar tam sayıya, skor 3 haneye yuvarlanır (`egitim.py:384-386`).
  - **Kararsızlık `u`:** Temizlenmiş, ≥`conf_aday` ve en çok 500 kutunun tamamından hesaplanır. Formül en kararsız 3 kutunun `1-|2p-1|` ortalamasıdır; kutu yoksa 0 (`egitim.py:389`, `ortak.py:563-570`).
  - `n` = skoru ≥`conf_on_etiket` olan kutu sayısı (`egitim.py:387,389`).
- **5) Titreme ve özet:**
  - Tahmin edilen karelerde varlık bilgisi için yalnızca ≥`conf_on_etiket` kutular kullanılır (`egitim.py:387-388`). Titreme ayarı açıksa hesaplanır (`egitim.py:390`).
  - Komşu kare `etiketli_havuz` içindeyse tahmin yerine insan etiketi kullanılır. Etiket dosyası o anda diskten okunur (`egitim.py:391-401`).
  - Skor `O.titreme_skorlari` ile hesaplanır (`egitim.py:402-403`, `ortak.py:586-612`). Formül: `t = (vyv + yvy) / (karedeki kutu + vyv)`. Burada vyv "var-yok-var", yvy "yok-var-yok" olay sayısıdır.
  - Sonuç `tahmin/ozet.json` dosyasına `{u, t, n}` olarak yazılır (`egitim.py:404`).
- **Sonuç dosyaları (`turlar/tur_XXX/` altında):**

  | Dosya | Yazıldığı yer |
  |---|---|
  | `is.json` | `app.py:432` |
  | `log.txt` (stdout ve stderr) | `app.py:433,438` |
  | `egitim.txt`, `dogrulama.txt`, `veri.yaml` | `egitim.py:202-206` |
  | `egitim{deneme}/weights/best.pt`, `last.pt` | `egitim.py:248,265` |
  | `test_tahmin.json` (analiz modunda **üzerine yazılır**) | `egitim.py:348,427` |
  | `tahmin/kareler/*.json`, `tahmin/ozet.json` | `egitim.py:386,404` |
  | `sonuc.json` ya da analiz modunda `analiz_sonuc.json` | `egitim.py:470` |
  | `analiz_log.txt` | `app.py:943` |

  Çalışma kökünde ayrıca şunlar bulunur:
  - `durum.json` (`app.py:235`)
  - `is_durumu.json` ve `iptal.bayrak` (`app.py:425-426`)
  - `analiz_durumu.json`, `analiz_iptal.bayrak` ve `analiz_is.json` (`app.py:938-942`)
  - Parça önbelleği `parcalar/<P>_<ov:.2f>/{images,labels}`, örneğin `640_0.20` (`app.py:428`, `egitim.py:187-189`)
- **Hata davranışı:**
  - `calis()` tüm adımları tek `try ... except BaseException` içinde koşar (`egitim.py:459-467`).
  - Eğitim başarılı olsa bile test ya da tahmin adımında hata çıkarsa tur `hata` olur ve `agirlik` alanı `sonuc` içinde kaybolur (`egitim.py:467`).
  - Bu durumda `son_basarili` güncellenmez (`app.py:474-476`) ve `best.pt` kullanılmaz. Dosya diskte kalır. O turun `last.pt` dosyası bir sonraki başarılı turda silinir (`app.py:488-490`).

## 2. `app.py` ile işçi arasındaki iletişim (dosya tabanlı)

- **`tur_baslat`** (`app.py:383-456`):
  - Önce `IS["proc"]` ya da `ANALIZ["proc"]` doluysa reddeder (`app.py:385-388`).
  - Sonra şunları kontrol eder: test onayı, en az 10 etiketli kare, en az 1 kutu (`app.py:389-401`).
  - Elle başlatılan turda test setinin tamamen etiketli olması gerekmiyor. Ölçüm yalnızca etiketli test karelerinde yapılır (`app.py:409`).
  - Listeler:
    - `etiketli` = havuz kareleri; kontrol edilmemiş M etiketli kareler hariç (`app.py:394-396`).
    - `tahmin` = havuzdaki, `kume` dışında kalan tüm kareler. M kareler bu listeye girer (`app.py:410`).
  - **Isınık başlangıç (`sicak`):** Model, sınıflar ve parça boyutu son başarılı turla aynı olmalı **ve** o turun ağırlık dosyası var olmalı (`app.py:413-414`). Öyleyse önceki `best.pt` ve `tur_epoch` kullanılır (`app.py:415-416`).
  - `iptal.bayrak` ve `is_durumu.json` silinir, `is.json` yazılır ve işçi `subprocess.Popen` ile başlatılır (`app.py:422-438`). Popen'a `start_new_session` ya da `creationflags` verilmiyor.
  - Ardından `IS` ve `DURUM["aktif_is"]` güncellenir (`app.py:442-453`).
- **Durum:**
  - İşçi `is_durumu.json` dosyasını en çok saniyede bir yazar; `zorla=True` çağrıları hariç (`egitim.py:118-130`).
  - Yazma atomiktir: önce geçici dosyaya yazılır, sonra `os.replace` yapılır (`ortak.py:41-57`).
  - `/api/durum` bu dosyayı okur ve tur numarası uyuşmazsa yok sayar (`app.py:857-865`). Dosyadaki `zaman` alanını kontrol etmez.
- **`is_kontrol`** (`app.py:510-528`):
  - Süreç yaşıyorken yalnızca iptalden 30 sn sonra `p.kill()` yapar (`app.py:515-516`).
  - Süreç bitince `sonuc.json` okunur. Dosya yoksa "beklenmedik kapanış" **hatası** üretilir (`app.py:524-527`).
  - Bu yüzden 30 sn sonra zorla öldürülen bir tur `iptal` değil `hata` olarak kaydedilir. `oto_beklet="hata"` ve `son_hata` ayarlanır (`app.py:498,504`).
- **`tur_sonucunu_isle`** (`app.py:459-507`). Başarılı turda:
  - `son_basarili` (ağırlık yolu) ve `DURUM["tahmin"]` yalnızca bu tura ayarlanır (`app.py:475-477`).
  - Denetim kareleri `ceil(denetim_orani × tur_buyuklugu)` adet, `random.Random(tur)` ile seçilir (`app.py:479-481`).
  - `kuyruk_kur` çalışır (`app.py:482`, `app.py:356-379`).
  - `m_etiketlerini_yenile` çalışır (`app.py:484`).
  - Diğer turların `last.pt` dosyaları silinir (`app.py:487-490`).
  - Plato kuralı uygulanır (`app.py:491-495`).
  - Başarısız ya da iptal edilen turda `oto_beklet` ayarlanır. Logun son 25 satırı yalnızca `hata` durumunda saklanır (`app.py:497-505`).
- **`oto_tetik`** (`app.py:531-545`):
  - Şu koşullarda çalışır: iş yok, analiz yok, `oto_beklet` boş, `oto_egitim` açık, test onaylı ve eksiksiz, havuzda en az 10 etiketli kare, `sonraki_kalan==0`.
  - Başlatma başarısız olursa `oto_beklet="hata"` olur.
- **İptal:**
  - `/api/iptal` bayrak dosyasını yazar ve `IS["iptal_zamani"]` değerini ayarlar (`app.py:1059-1066`).
  - İşçi her `durum()` çağrısında bayrağı kontrol eder ve `Iptal` fırlatır (`egitim.py:119-120`). Kontrol, saniyede bir sınırından önce yapılır.
  - Kontrol noktaları:
    - Eğitimde her batch (`egitim.py:215-219`).
    - Hazırlıkta her kare (`egitim.py:195`).
    - Testte her kare (`egitim.py:340`).
    - Tahminde her kare (`egitim.py:379`).
  - Titreme hesabında ve bir karenin parça parça tahmini sırasında kontrol yok.
  - 30 sn içinde kapanmazsa sunucu `kill` eder. Bu yalnızca Popen ile başlatılan işçi sürecini hedefler (`app.py:516`).
- **Analiz:**
  - `analiz_hazirla` eğitim sürerken reddedilir (`app.py:921-922`). Son başarılı turun ağırlığıyla `egitim.py --analiz` başlatılır (`app.py:916-953`).
  - İş dosyası `calisma/analiz_is.json`, çıktı o turun dizinidir. Parça boyutu `sb["parca"]` değerinden, örtüşme ve diğer ayarlar **güncel ayardan** alınır (`app.py:940`).
  - O turun `test_tahmin.json` dosyası, turun kendi örtüşmesinden farklı bir örtüşmeyle yeniden yazılabilir (`egitim.py:348,427`).
  - `analiz_kontrol` yalnızca bitişi işler (`app.py:902-913`).
  - **Analiz için iptal yolu yok.** `analiz_iptal.bayrak` yalnızca siliniyor (`app.py:935`); dosyayı yazan bir endpoint ya da arayüz çağrısı yok. Zaman aşımı ya da `kill` de yok.
- **İzleyici thread'i:**
  - Daemon thread olarak başlar (`app.py:1262`).
  - Her 2 sn'de bir `KILIT` tutarak sırayla şunları çağırır: `xml_iceri_aktar`, `test_seti_kontrol`, `is_kontrol`, `analiz_kontrol`, `oto_tetik` (`app.py:627-638`).
  - Flask `threaded=True` ile çalışır (`app.py:1264`).
- **Kilit ve global durum:**
  - Tek kilit var: `KILIT = threading.RLock()` (`app.py:41`).
  - Global nesneler: `CFG`, `DURUM`, `ROL`, `IS`, `ANALIZ`, `_ETK`, `_MEO`, `_EMEK`, `_ANALIZ_ONB`, `_ONB` (`app.py:40-50`), `_XML_HATALI` (bir küme, `app.py:548`), `_TARAMA` (`app.py:780`), `_DETAY` (`app.py:829`).
  - `/api/analiz` yalnızca ayar, sınıf ve tur listesini kilit altında okur (`app.py:1007-1011`). Ağır hesap kilit **dışında** yapılır (`app.py:1012-1021`) ve `_ANALIZ_ONB` kilitsiz yazılır (`app.py:997-999`).
  - `tur_sonucunu_isle` içinden çağrılan `m_etiketlerini_yenile`, kilit tutulurken her M kare için JSON okur ve etiket yazar (`app.py:1112-1119`). Her karede `ayarlar_oku()` da çağrılır (`app.py:1096`). Bu sürede arayüz istekleri bekler.

### GPU'yu aynı anda kim kullanabiliyor?

- **Sunucu GPU kullanmıyor.**
  - `app.py` importları arasında torch ya da ultralytics yok (`app.py:17-36`).
  - `ortak.py` yalnızca numpy kullanır; gerektiğinde PIL ve cv2'yi tembel import eder (`ortak.py:3,75,88,100`).
- **Bir sunucu örneğinde aynı anda en çok bir işçi süreci çalışır**, tur ya da analiz.
  - Kontroller `tur_baslat` (`app.py:385-388`), `oto_tetik` (`app.py:532`) ve `analiz_hazirla` (`app.py:921-924`) içinde.
  - Üçü de `KILIT` altında çalışır (`app.py:630`, `app.py:1054`, `app.py:920`).
  - Bir turda eğitim, test ve tahmin aynı süreçte sırayla yapılır (`egitim.py:436-445`).
- **Korunmayan durumlar:**
  - Süreçler arası ya da sistem çapında bir kilit dosyası yok. İki uygulama örneği aynı GPU'da aynı anda eğitim başlatabilir. Örnek: OKUBENI'deki aktif/rastgele deneyi (`OKUBENI.md:125-128`).
  - **Yeniden açılış:** Eski işçinin PID'i saklanmıyor ve öldürülmüyor. `main()` şunları yapar:
    - `iptal.bayrak` ve `is_durumu.json` dosyalarını siler (`app.py:1246-1247`).
    - `aktif_is` varsa o turun `sonuc.json` dosyasını okur. Dosya varsa sonucu olduğu gibi işler; `tamam` ise başarılı tur sayılır. Dosya yoksa turu `iptal` olarak işaretler (`app.py:1248-1251`).
    - Sonradan bitip `sonuc.json` yazan yetim işçinin sonucunu kimse işlemez.
    - `ANALIZ` bellekte tutulur; yeniden açılışta yarım kalan analiz işi hiç ele alınmaz.
  - **Yetim işçi ile yeni işçi aynı dosyaları paylaşır:**
    - Aynı `iptal.bayrak` ve `is_durumu.json` (`app.py:425-426`).
    - Aynı `parca_kok` (`app.py:428`).
    - Yeni turun `veri_hazirla` adımı, eski işçi eğitimdeyken `*.cache` dosyalarını siler ve parça etiketlerini yeniden yazar (`egitim.py:190-191,181`).
  - Kodda `atexit` ya da `signal` yok (grep ile doğrulandı).

## 3. Tahminler tek geçişte mi? Önbellek durumu

- Her parça bir kez tahmin edilir. TTA ya da `augment` yok; `iou=0.7` (`egitim.py:300-301`).
- Test kareleri bir kez tahmin edilir. Ham ve temiz metrikler aynı sonuçtan hesaplanır (`egitim.py:343,356-364`).
- **Etiketsiz tahmin her turda havuzdaki tüm etiketsiz karelerde yeniden yapılır** (`app.py:410`).
  - Bu küme M kareleri ve hâlâ etiketlenmemiş eski denetim karelerini içerir.
  - Artımlı çalışma ya da önceki turun tahminini yeniden kullanma yok.
  - Kareler her turda diskten yeniden çözülür. 2 thread çalışır; işlenen karenin yanında kuyrukta en çok 3 kare bekler (`egitim.py:276-290`).
  - Tahmin parçaları önbelleğe alınmaz (`egitim.py:295`).
- **İşçideki önbellek:**
  - Yalnızca eğitim **ve doğrulama** karelerinin parça görüntüleri önbellekte tutulur (`egitim.py:167-177`, `egitim.py:192`).
  - Dosya adı anahtarı `guvenli_ad(ad, dosya boyutu)__x_y` biçimindedir (`egitim.py:146,167`). mtime anahtara girmez. P ve ov klasör adındadır (`app.py:428`).
  - Parçalar JPEG kalite 95 ile yeniden kodlanır (`ortak.py:99-107`).
  - Parça etiketleri her turda yeniden yazılır (`egitim.py:179-181`). Ultralytics `.cache` dosyaları her turda silinir (`egitim.py:190-191`).
  - Parça klasörü hiç temizlenmez. Projede `rmtree` yok; `unlink` yalnızca `app.py:423,490,936,1247` ve `egitim.py:191` satırlarında.
- **Sunucudaki önbellekler:**

  | Önbellek | Ne tutuyor / geçerlilik | Satır |
  |---|---|---|
  | `_ONB` | Resim listesi, 3 sn | `app.py:103-113` |
  | `_ETK` | Etiketli küme, 2 sn; `etiket_degisti()` ile sıfırlanır | `app.py:134-156` |
  | `_MEO` | Modelin etiketleyebileceği kareler. Anahtarda `id(kume)` ve `id(adlar)` var | `app.py:1069-1086` |
  | `_EMEK` | Emek özeti; mtime ve boyut anahtarlı | `app.py:735-746` |
  | `_TARAMA` | Etiket taraması, 10 sn | `app.py:798-826` |
  | `_DETAY` | Tahmin dizini başına özet | `app.py:832-837` |
  | `_ANALIZ_ONB` | Analiz sonucu. 17 kayıt varken yeni kayıt gelince tamamen temizlenir | `app.py:956-1000` |

  - `_MEO` anahtarındaki nesne kimliği (`id()`) en geç 2-3 sn'de bir değişir, çünkü küme ve liste her yeniden taramada yeni nesneyle değiştirilir (`app.py:111,150`). Serbest bırakılan bir nesnenin adresi yeniden kullanılırsa yanlış önbellek isabeti teorik olarak mümkün.
  - `_analiz_hesapla`, önbellek anahtarına bakmadan önce `test_tahmin.json` dosyasının tamamını her istekte parse eder ve her test etiketine `stat` yapar (`app.py:963-970` → `app.py:973-975`).
  - Kılavuz sayfası her yüklemede bu isteği iki kez yapar: önce `temiz=1`, sonra `temiz=0` (`kilavuz.html:217,220`).

## 4. Önceki modelin tahminleri saklanıyor mu? (Teacher-Student)

- **Diskte kalanlar.** Silen kod yok; yalnızca `last.pt` siliniyor (`app.py:487-490`).
  - Her turun `best.pt` dosyası. Yolu o turun `sonuc.json` dosyasındaki `agirlik` alanında (`egitim.py:447,470`). Sıcak başlangıçlı turun `is.json` dosyasında da önceki turun yolu `baslangic_agirlik` olarak duruyor (`app.py:430`).
    - `DURUM["turlar"]` kayıtlarında ağırlık yolu yok (`app.py:443-448,466-472`); yalnızca son tur için `son_basarili.agirlik` var (`app.py:475`).
  - Her turun `tahmin/kareler/*.json` ve `ozet.json` dosyaları. İçerik: temizlenmiş, ≥`conf_aday`, en çok 500 kutu, koordinatlar tam sayıya yuvarlanmış (`egitim.py:381-386`).
  - v3.6 turlarının `test_tahmin.json` dosyası: birleştirilmiş ama temizlenmemiş, ≥0,01 (`egitim.py:343-348`). Analiz modunda son başarılı turunki üzerine yazılabilir (bkz. 2).
  - Her turun ayar anlık görüntüsü `is.json` içinde (`app.py:427,432`).
- **Kullanılan kısım:**
  - Etiketsiz kare tahminlerinde yalnızca son tur kullanılır: `DURUM["tahmin"]` (`app.py:477`), `kuyruk_kur` (`app.py:363`), `/api/labels` (`app.py:681-685`), `model_etiketlenebilir` ve `model_etiketi_yaz` (`app.py:1079,1091`).
  - **İstisna:** `/api/analiz?tur=N` önceki bir turun `test_tahmin.json` dosyasını okuyabilir (`app.py:1013-1015`). Kılavuz arayüzü bu parametreyi kullanmıyor (`kilavuz.html:217,220`).
- **Karşılaştırma yapılabilir mi:** İki ardışık turda da etiketsiz kalan karelerin her iki modelden JSON'u diskte var. Ek GPU çalıştırmadan son ve önceki model karşılaştırılabilir. Kısıtları:
  - Kayıtlı kutular temizlenmiş, ≥`conf_aday` ile eşiklenmiş, en çok 500 tane ve yuvarlanmış.
  - `conf_aday`, `oneri_nms` ve `ortusme` turlar arasında değişmiş olabilir (her turun `is.json` dosyasına bakılmalı).
  - v3.5 JSON'larında `temiz` alanı yok (`app.py:1103-1109`).
- **İşçi içinde öğretmen modeli:** Sıcak başlangıçta işçi önceki modelin yolunu zaten `job["baslangic_agirlik"]` olarak alıyor (`app.py:415`). Ama bunu tahmin için değil, eğitime başlangıç ağırlığı olarak kullanıyor (`egitim.py:242`). Sıcak değilse bu alan seçili model dosyasıdır; varsayılan `yolo11n.pt` (`app.py:59`).
- **Titreme Teacher-Student için değil.**
  - Titreme, **o turun modelinin** tahminlerini komşu karelerle karşılaştırır. Komşular i-1 ve i+1'dir; sıra, doğal sıralanmış tüm karelerdir (`app.py:430`, `app.py:111`).
  - Etiketli havuz komşularında **insan etiketi** kullanılır (`egitim.py:393-401`). Yani karşılaştırma model ile model ya da model ile insan arasında yapılır.
  - Olaylar "var-yok-var" ve "yok-var-yok"dur (`ortak.py:586-612`).
  - Şu durumlarda `t=0` olur: komşu bilinmiyorsa (test, tampon karesi), kare dizinin ilk ya da son karesiyse (`ortak.py:602-606`).

## 5. Kılavuz ve `/api/analiz`: MC Dropout ve Deep Ensemble

- **Yalnızca anlatım; kodda çalışan karşılığı yok.**
  - `kilavuz.html:169-194` (b8) statik metin. "Durum: ikisi de araştırma aşamasında, v3.6'da yok" diyor (`kilavuz.html:192`).
  - b8 içinde `id`'li ya da JS'in doldurduğu eleman yok. Script `kilavuz.html:197-400` arasında ve doldurduğu `id`'lerin hepsi b1-b7 ile üst çubukta.
  - `dropout` ve `ensemble` kelimeleri yalnızca `kilavuz.html` (170-193) ve `OKUBENI.md:23` içinde geçiyor. `app.py`, `egitim.py`, `ortak.py` ve `index.html` içinde yok.
  - `/api/analiz` yalnızca `test_tahmin.json` dosyasından şunları hesaplıyor (`app.py:1003-1025`, `app.py:956-1000`, `ortak.py:369-480`): TP/FP/FN, P/R/F1, AP/mAP, PR eğrisi, eşik eğrisi, seçili eşikteki karışıklık tablosu ve 19 eşiğin her biri için karışıklık tablosu.
- **OKUBENI yanıltıcı:** `OKUBENI.md:23` (HEAD'de 48) "MC Dropout ve Deep Ensemble; hepsi senin modelinin test sonuçlarıyla" diyor.
- **Kılavuzdaki sabit formül:** "öncelik = 0,7 × kararsızlık + 0,3 × titreme" sabit yazılmış (`kilavuz.html:175`).
  - Gerçek ağırlıklar ayardan geliyor. Titreme kapalıysa titreme ağırlığı 0 olur (`app.py:74-75,366-367`).
  - Ana arayüzdeki ipucu gerçek ağırlıkları gösteriyor (`index.html:839-841`, `app.py:848`).
  - `OKUBENI.md:28` de sabit 0,7/0,3 yazıyor.
- **"Ucuz sürüm" iddiası kodla tutarlı:** Önceki turun `best.pt` dosyası diskte kalıyor (`kilavuz.html:189`, `app.py:487-490`). Etiketsiz kareler için önceki turun JSON'ları da duruyor. Bu yüzden ikinci tahmin geçişi bile her zaman gerekmiyor (bkz. 4).

## 6. Bellek ve VRAM açısından riskli yerler

1. **Eğitimden sonra boşaltma etkisiz kalıyor.**
   - `empty_cache()` çağrıldığında (`egitim.py:271-272`), `tr = model.trainer` (`egitim.py:264`) yerel değişkeni hâlâ canlı. Bu yüzden trainer'ın tuttuğu bellek o anda serbest değil.
   - `egit` dönünce `tr` kapsam dışına çıkar. Başka referans yoksa bellek PyTorch'un önbellek ayırıcısına döner. Aynı süreçte yüklenen çıkarım modeli (`egitim.py:443`) bu belleği yeniden kullanabilir; ama VRAM sürücüye geri verilmez.
   - Kodda `gc.collect()` yok. Kütüphane içinde döngüsel referans varsa serbest bırakma gecikebilir; ultralytics kurulu olmadığı için bunu doğrulayamadım.
2. **OOM sonrası yeniden deneme.**
   - `except` bloğundaki `empty_cache()` (`egitim.py:260-261`) çağrıldığında başarısız denemenin `model` nesnesi (ve trainer'ı) hâlâ bağlı. Yeni `YOLO(...)` atanana kadar (`egitim.py:242`) eski deneme bellekte kalır.
   - Varsayılan batch -1 (AutoBatch, `app.py:63`). OOM olursa 8'e düşer.
   - Her denemenin çıktısı ayrı bir `egitim{deneme}` klasörüne gider. Mevcut turun klasörleri `last.pt` silme döngüsünün dışında kalır (`app.py:489`).
3. **Tahmin batch'i 16 parça** (`egitim.py:415`). OOM olursa yarıya iner, bir daha büyümez. Değer nesne seviyesinde tutulduğu için test aşamasından tahmin aşamasına taşınır (`egitim.py:303-308`).
4. **Kare başına RAM (hesap).** 3840×2160 kare, P=640, örtüşme 0,2 (varsayılan, `app.py:64-65`):
   - Adım 512 piksel, ızgara 8×4 = 32 parça (`ortak.py:216-229`).
   - Parçalar `ascontiguousarray` ile kopyalanır: yaklaşık 39 MB (`egitim.py:295`).
   - Kare yaklaşık 25 MB. Ön okuma kuyruğunda 3 kare daha tutulur (`egitim.py:280-285`).
5. **Test tahmininde kutu patlaması.**
   - `conf=0.01` ve parça başına en çok 100 kutu: 4K/640 için birleştirme öncesi en çok 3200 kutu (`egitim.py:343`).
   - `parca_birlestir`: n adımlık Python döngüsü ve her adımda vektör işlem, toplam O(n²) (`ortak.py:275-310`).
   - `grup_kutulari`: O(n²) döngü; içinde aday başına `iou_matrisi` çağrısı var (`ortak.py:518-534`).
   - `nms` sınıf başına tam n×n float64 IoU matrisi kurar. `iou_matrisi` birkaç n×n ara dizi oluşturur (`ortak.py:494`, `ortak.py:238-245`). n=3200 için matris başına yaklaşık 82 MB (hesap); ara dizilerle birlikte bunun birkaç katı.
   - Bu yol işçide (`egitim.py:364`) ve Flask sürecinde `/api/analiz?temiz=1` ile çalışıyor (`app.py:985`). Flask tarafında 20 karışıklık tablosu da hesaplanıyor (`app.py:993-994`).
6. **`test_tahmin.json` büyük.** ≥0,01 olan tüm kutular `indent=1` ile yazılıyor (`egitim.py:345-348`, `ortak.py:57`). Her `/api/analiz` isteğinde baştan parse ediliyor (`app.py:963`); sayfa başına 2 istek var.
7. **Disk büyümesi.** Hiçbir şey silinmiyor:
   - Her turda tüm etiketsiz kareler için yazılan JSON'lar.
   - Her turun `best.pt` dosyası.
   - Her (P, ov) çifti için ayrı parça klasörü (`app.py:428`).
   - Silme kodu yalnızca `last.pt` için var (`app.py:487-490`).
8. **Takılan süreçler.**
   - İşçi takılırsa sunucuda heartbeat ya da zaman aşımı yok; `is_durumu.json` içindeki `zaman` alanına bakılmıyor (`app.py:857-865`).
   - `kill` yalnızca iptal istendikten sonra yapılıyor (`app.py:515-516`), ve sonuç "hata" olarak kaydediliyor (bkz. 2).
   - Analiz işi hiç iptal edilemiyor (`app.py:935`; bayrağı yazan yok).
9. **Yeniden açılışta GPU ve dosya çakışması.**
   - Yetim işçi kontrolü yok (`app.py:1246-1251`). Yeni tur, hâlâ çalışan eski bir işçiyle aynı anda GPU kullanabilir.
   - İki işçi aynı parça klasörüne (`.cache` silme, etiket yazma), aynı durum dosyasına ve aynı iptal bayrağına dokunur (`app.py:425-428`, `egitim.py:181,190-191`).

---

**Düzeltmeler:**

1. **Başlık notu:** v3.7 değişiklikleri artık "işlenmemiş çalışma kopyası" değil, commit `56f2a51` olarak duruyor ve çalışma ağacı temiz. HEAD `app.py` için satır kaymaları eklendi; analizde sadece "yaklaşık 36. satırdan sonra kayıyor" deniyordu. `kilavuz.html` b8 bölümü gerçekten aynı, ama dosyanın 314. satırı değişmiş.
2. **Yeniden açılış (`app.py:1248-1251`):** Analizde "yarım tur sadece `iptal` olarak işaretlenir" deniyordu. Bu yanlış: önce `sonuc.json` okunuyor. Dosya varsa sonuç olduğu gibi işleniyor, `tamam` da olabilir; yalnızca dosya yoksa `iptal` yazılıyor. Yetim işçinin aynı bayrak, durum dosyası ve parça klasörünü paylaştığı eklendi.
3. **İptal sonrası zorla `kill`:** Bu durumda `sonuc.json` yazılmadığı için tur `iptal` değil `hata` olarak kaydediliyor (`app.py:525-527,498`). Analizde bu yoktu.
4. **"Hiçbir kod önceki turun tahminini okumuyor"** ifadesi daraltıldı: `/api/analiz?tur=N` önceki bir turun `test_tahmin.json` dosyasını okuyabiliyor (`app.py:1013-1015`). Etiketsiz kare tahminleri için ifade doğru.
5. **Parça önbelleği:** Analizde "yalnızca eğitim" deniyordu; doğrulama kareleri de önbelleğe giriyor (`egitim.py:192`). Klasör adı `<P>_<ov:.2f>`. Dosya adı anahtarında parça koordinatları da var.
6. **Doğrulama yedeği:** "%10 ödünç alır" ifadesi netleştirildi. Seçilen, eğitim listesinin **ilk** %10'u; rastgele değil ve bu parçalar eğitimde de kalıyor (`egitim.py:200`).
7. **Isınık başlangıç koşuluna** ağırlık dosyasının var olması eklendi (`app.py:414`).
8. **Titreme:** Analizde "aynı modelin tutarsızlığı" deniyordu. Etiketli komşularda insan etiketi kullanılıyor (`egitim.py:393-401`). Dizinin ilk ve son karesinde de `t=0` (`ortak.py:602`). Formül eklendi.
9. **VRAM maddesi 1 düzeltildi:** `tr` yalnızca `egit` dönene kadar canlı. Ondan sonra bellek PyTorch önbelleğine dönüyor ve aynı süreçteki çıkarım modeli onu yeniden kullanabiliyor. Asıl sorun, `empty_cache()` çağrısının boşa gitmesi ve VRAM'in sürücüye geri verilmemesi. Kütüphane içi döngüsel referanslar koddan doğrulanamaz.
10. **Analiz işi:** Örtüşmenin yanı sıra diğer ayarlar da güncel ayardan alınıyor. Analiz işi son başarılı turun `test_tahmin.json` dosyasının **üzerine yazıyor** (`egitim.py:348,427`).
11. **`/api/iptal`** yalnızca bayrak yazmıyor; `IS["iptal_zamani"]` değerini de ayarlıyor (`app.py:1065`). İptal kontrol noktaları kesinleştirildi; titreme hesabında ve tek karenin parça tahmini sırasında kontrol yok.
12. **Durum yazımı:** "Saniyede en çok bir kez" kuralının `zorla=True` çağrılarında aşıldığı eklendi (`egitim.py:122`).
13. **Önbellekler:**
    - `_ANALIZ_ONB` 17 kayıt varken yeni kayıt gelince temizleniyor (`len > 16`, `app.py:997`).
    - `_MEO` anahtarındaki `id()` kullanımı eklendi.
    - Kılavuz sayfası `/api/analiz`'i 2 kez çağırıyor (`kilavuz.html:217,220`).
    - `m_etiketlerini_yenile` her karede `ayarlar_oku()` çağırıyor (`app.py:1096`).
14. **Küçük eklemeler:**
    - `Isci` nesnesi `try` dışında oluşuyor (`egitim.py:457`).
    - `%25` kuralında 2 piksel alt sınır var ve kısmen görünen nesneler etiketsiz kalıyor (`egitim.py:160`).
    - rng tohumu dosya adı ile boyutundan geliyor, yalnızca addan değil (`egitim.py:146-147`).
    - Cihaz adı her zaman 0. karttan alınıyor (`egitim.py:413`).
    - Elle başlatılan tur eksik test setiyle de çalışabiliyor (`app.py:389,409`).
    - Kök klasördeki `durum.json` dosyası (`app.py:235`).
    - OOM sonrası her denemenin ayrı `egitim{deneme}` klasörü.
    - Kılavuzdaki sabit formülün `OKUBENI.md:28` içinde de geçmesi.
    - `nms` için matris boyutu hesabı (yaklaşık 82 MB) kesinleştirildi.
15. **Doğrulanan ve olduğu gibi kalanlar:** Şu satır referansları ve formüller doğru çıktı: `egitim.py` 61-73, 97-115, 118-130, 190-206, 212-227, 242-268, 276-308, 335-404, 411-416, 419-472; `ortak.py` 41-57, 216-229, 238-245, 275-310, 494, 518-534, 563-570, 586-612; `app.py` 40-50, 63, 383-456, 459-507, 510-545, 627-638, 735-746, 798-837, 857-865, 902-1025, 1059-1066, 1069-1119, 1262-1264; `kilavuz.html` 175, 189, 192; `OKUBENI.md` 23, 125-128. Şu hesaplar da doğrulandı: 8×4 = 32 parça, yaklaşık 39 MB ve 25 MB, 3200 kutu. Kodda `atexit` ya da `signal` olmadığı ve sunucunun torch ya da ultralytics import etmediği grep ile doğrulandı.