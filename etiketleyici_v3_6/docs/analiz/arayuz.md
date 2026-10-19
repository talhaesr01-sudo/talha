# Arayüz haritası: `static/index.html` (doğrulanmış ve düzeltilmiş)

## 0. Sürüm durumu ve satır numaraları

**Analiz yazıldıktan sonra durum değişti.** Yarım kalmış diff commit edildi.

- **HEAD artık `56f2a51`.** Commit mesajı: "Faz 1: buyutec kaldirildi, sol menu uc kategori, tek tikla onay, davranis kaydi" (2026-10-19 21:35 UTC).
  - `da8f5c0` (v3.6) artık HEAD~1.
  - `git status` boş, çalışma ağacı temiz.
  - Commit 7 dosyayı içeriyor: `.gitignore`, `OKUBENI.md`, `app.py`, `davranis.py` (yeni), `static/index.html`, `static/kilavuz.html`, `testler/test_davranis.py`.
- **Bu belgedeki index.html satırları v3.6'ya (`da8f5c0`, 1633 satır) göredir.**
  - `.../scratchpad/index_HEAD.html` dosyası `da8f5c0` sürümüyle bayt bayt aynı (`cmp`).
  - Doğrulama kopyaları `/tmp/claude-0/-home-user-talha/2c30178d-d993-51d1-9278-3f9d7242faf6/scratchpad/dogrula/` klasöründe: `idx_v36.html`, `app_v36.py`, `kil_v36.html`, `oku_v36.md`.
- **app.py satırları HEAD'e (`56f2a51`, 1300 satır) göredir.**
  - v3.6'dan (1268 satır) fark: `import davranis as D` satırı (36), `save_labels` değişikliği, `davranis_kaydet` (732–739), `/api/davranis.csv` (742–745), `/api/davranis_ozet` (748–754).
  - 699'dan önceki satırlar v3.6'ya göre +1 kayık. Örnek: `get_labels` koşulu v3.6'da 684, HEAD'de 685.
- **HEAD'deki index.html (1657 satır):**
  - Başlık v3.7 (6, 217).
  - `#bOnay` düğmesi (227).
  - Üç kategori düğmesi (233–235): `data-liste="oneri|etiketsiz|onay"`, sayaçları `#say-*`.
  - `$('#gorunum')` ve büyüteç JS'i yok. Büyüteç yalnız CSS yorumunda geçiyor: "v3.7: büyüteç kaldırıldı" (66).
  - JS'teki bütün `$('#id')` başvurularının HTML'de karşılığı var. `say-` bir birleştirme öneki.
- **"Sayfa açılmaz" iddiası** yalnızca incelemedeki ara durum için geçerliydi. Commit edilen sürümde geçerli değil.
  - O ara durum artık yok, doğrulanamaz.
  - Mantık v3.6 koduna göre doğru: `$('#gorunum').onchange` (1134) en üst düzeyde ve `resize()` / `init()` çağrılarından (1629–1630) önce çalışır. Öğe yoksa TypeError betiği burada durdurur.
  - 518 de hata verirdi, ama `init` hiç çalışmayacağı için bu satıra sıra gelmez.
- **v3.6 `listeKur` (595–600):**
  - Filtre haritası `etiketli`, `model` ve `etiketsiz` değerlerini tanır. `etiketsiz` yeni kategoriyle aynı adı taşıdığı için tanınır.
  - `oneri` ve `onay` ise `else` dalına (600) düşer ve kuyruk sırası kullanılır.

---

## 1. Büyüteç (v3.6)

| Yer | Satır | İçerik |
|---|---|---|
| CSS yorumu | 65–66 | "üst: kılavuz çizgileri ve büyüteç" |
| Durum alanı | 397 | `buyutec: true` |
| localStorage okuma | 516 | `localStorage.getItem('etiketleyici_buyutec') !== '0'` |
| localStorage yazma | 1123 | anahtar `etiketleyici_buyutec` |
| Üç tuval | HTML 239–241, CSS 67–69, JS 386–387 | `#cvImg` (resim), `#cv` (kutular ve fare olayları), `#cvUst` (`pointer-events:none`, 69) |
| `resize()` | 631–636 | Üç tuval birlikte boyutlanır, `S.gorselAnahtar = null` |
| Katmanlar | 647–659 `gorselCiz`, 662–714 `kutuCiz`, 717–736 `ustCiz`; 660 `draw()` | `draw` üçünü sırayla çağırır |
| `ustCiz` | 716 (yorum), 717–736 | Katmanı temizler. 721: `!S.img \|\| !S.fare` ise çıkar. 724–734: `S.arti && !S.pan` iken kılavuz çizgileri, yalnız imleç resmin içindeyse (726). 735: `S.buyutec && S.op` ve tür `resize` ya da `draw` ise `buyutecCiz(fx,fy)` |
| `buyutecCiz` | 738–739 (yorum), 740–774 | `L=176`, `kat=clamp(S.view.s*4,1.5,16)`. Yalnız `ctxUst`'a çizer (`c = ctxUst`, 741). `S.op.type`, `S.op.i`, `S.op.x0..y1`, `S.boxes`, `S.img`, `S.view`, `S.cls` okur |
| `ustCiz` çağrıları | 660, 897 (mouseleave), 899 (`ustIste`, rAF) | `ustIste` yalnız 948'de çağrılır (`!op` ve `S.arti`). İşlem sürerken 953/971 → `cizIste` (898) → `draw` → `ustCiz`. Kaydırma (943) ve tekerlek (1026) de `cizIste` kullanır |
| `S.fare` | 946 (atama), 897 (null), 721, 723, 1064–1065 | Kılavuz çizgileri ve `kutuGez` (Tab) kullanır, büyüteç kalkınca da kalmalı. İşlem sürerken tuval dışında da güncellenir (946) |
| B kısayolu | 1121–1125 | Toast 1124'te. `draw()` çağırmaz |
| Kısayol listesi | 283 | `<dt>B</dt>…` |
| OKUBENI.md (v3.6) | 11, 19, 101 | doğru |
| OKUBENI.md (HEAD) | 8, 36, 44 | 8: "Büyüteç kaldırıldı". 36 ve 44: v3.6 değişiklik notlarında hâlâ geçiyor |
| kilavuz.html | 314 | v3.6: "(büyüteç, ok tuşları)". HEAD: "(yakınlaştırma, ok tuşları)". Başka geçiş yok |
| app.py, egitim.py, ortak.py, davranis.py | — | `buyutec` / `büyüteç` hiç geçmiyor (grep) |

**İmleç:**
- **CSS kuralları:** 68 `#cv{cursor:crosshair}` ve 71 `#stage.panning #cv{cursor:grabbing}`.
  - Bu iki kural `init` sonrasında fiilen etkisiz: `setMode` (886) inline `crosshair` atar, `init` bunu 519'da çağırır. Inline stil ID seçicisini ezer (`!important` yok).
  - Boşlukla kaydırırken imleç `grab` olur (1106). Orta tuşla kaydırırken son değerde kalır.
- **`CURSOR` haritası** (480–481): doğru.
- **`updateCursor`** (1009–1017): yalnız 948'de, `!S.op && e.target===cv` iken çağrılır. Sıra:
  1. `S.space` ise çıkar.
  2. Çizim modu ya da Shift basılıysa `crosshair`.
  3. Seçili kutunun tutamağı üstündeyse `CURSOR[h]`.
  4. Herhangi bir kutunun içindeyse (`hitBox`) `move`.
  5. `adayBul` bir aday bulursa `cell`, yoksa `crosshair`.
- **Diğer atamalar:** 1106 `grab`, 1128 `crosshair`, 886 `crosshair`.
- **İşlem sürerken** imleç güncellenmez.
- **`cursor:none` iki sürümde de yok** (grep). `#cvUst`, `#banner` (150) ve `#toast` (76) `pointer-events:none`. Büyüteç kalkınca imleç için ek iş gerekmez.

---

## 2. Sol menü (v3.6)

**HTML (215–236):** `nav#sol` içinde şunlar var:
- `div.solust`: `select#gorunum` (217–224), `#gorunumBilgi` (225)
- `#list` (227)
- `#simgeler` (228–235): ●, ○, M, T, D, –

| value | Metin | HTML | Filtre |
|---|---|---|---|
| `kuyruk` | Öneri sırası (etiketlenecekler) | 218 | 600: `S.kuyruk.adlar` → `byName` |
| `tum` | Tüm kareler (zaman sırası) | 219 | 598 |
| `etiketli` | Etiketliler (senin) | 220 | 596 `labeled && !model` |
| `model` | Kontrol bekleyenler (M) | 221 | 596 `labeled && model` |
| `etiketsiz` | Etiketsizler | 222 | 596 `!labeled`. Test ve tampon kareleri ile önerisi olan etiketsiz kareler dahil, M kareleri hariç |
| `test` | Test seti | 223 | 599 `rol==='test'` |

**Doğrulanan "yok" iddiaları:**
- **"Tam test" yok.** Proje ve v3.6 kopyalarında hiç geçmiyor (grep).
- **"Öncelik" bir filtre değil.** Geçtiği yerler:
  - Sağdaki puan: 834–836, CSS 59–61
  - Satır ipucu: 840–842
  - `gorunumBilgi` metni: 861
  - Ayar yardımı: `agirlik_u` alanı, 1496–1497

**Liste davranışı:**
- **Seçim kalıcı:** 514 okur, 1136 yazar. İşleyici 1134–1138: `listeKur()` + `blur()`.
- **İlk açılan kare** (524): `S.order` içindeki ilk etiketsiz kare. Yoksa (`??`) tüm kareler içindeki ilk etiketsiz kare. O da yoksa 0. M kareleri `labeled` sayıldığı için atlanır.
- **`siradaki`** (603–616):
  - Öneri sırasında ileri giderken etiketli ve M olmayan kareleri atlar (610). Diğer listelerde atlamaz.
  - Açık kare listede yoksa (`pos=-1`) ileri gitmek listenin başını verir, geri gitmek `null` döner (614).
- **`git`** (617–628): Sonraki kare yoksa ve ileri yönde kare kirli ya da öneri ise kaydeder (620–622), ardından toast gösterir (623–624).
- **`kuyrukYukle`** (591–594): Liste yalnız `yenidenCiz && S.liste==='kuyruk'` iken yeniden kurulur.

**Sayaçlar:**
- `#progress` (205), metni 817–820'de üretilir. Formül doğru.
- `gorunumBilgi` (852–862): tohum hâli 860, tur hâli 861. `renderModel` bunu 1337'de yalnız kuyruk listesinde yeniden çağırır.

**`renderList`** (815–851): satır 826–827, rozet 829–832, sağ sütun 833–836, tıklama 843, boş liste 848. Hepsi doğru.
- Rozet CSS'i: `.rozet*` 115–119, `.simgeler .rozet` 121.

**`/api/info`** (app.py 648–663):
- Biçim: `{classes, images:[{name, labeled, count, rol, model}]}`.
- `labeled` M karelerini de kapsar (`etiketli_kume`, 135–153, bütün `.txt` dosyaları).
- İstemci `kisa` alanını ekler (508).
- Kaydetmede `labeled`, `count` ve `model` alanlarını (542), yüklemede yalnız `labeled` ve `model` alanlarını (581) değiştirir. **`count` yüklemede değişmez.**

**`/api/kuyruk`** (app.py 869–878):
- Dal koşulu **`t and DURUM.get("kuyruk")`** (874). Yani tahmin olsa bile kuyruk boşsa tohum yanıtı döner (878): `{tur:null, adlar:tohum_listesi, denetim:[], skor:{}, tohum:true}`. Bu yanıtta `detay` ve `agirlik` yok.
- Dolu dal 875–877. `agirlik.t` alanı `titreme` kapalıysa 0 olur (877).
- `adlar` kurulduğu anda yalnız havuz karelerini içerir:
  - `kuyruk_kur`: 365
  - `tohum_listesi`: 348
  - Tahmin listesi: 411. Kullanıcının etiketlemediği havuz kareleri, M dahil (`kume` 395 M'yi dışlar).
- **"Test ve tampon asla görünmez" iddiası fazla kesin.**
  - `test_seti_sec` (1170–1207) `kuyruk_kur` çağırmıyor. Model varken "sıfırla" ile test seti değişirse `DURUM["kuyruk"]` eski havuz adlarını taşır. Bu adlar sonraki tur sonuna (483) ya da ayar kaydına (1076) kadar kalır.
  - Denetim kareleri 374'te yalnız `a in kareler` ile süzülür, rol ile süzülmez.
  - Olağan akışta doğru.

**Test seti akışı:**
- **Otomatik soru** (1288): `!test_onay && !S.testSoruldu && s.toplam>0 && açık dialog yok`.
  - `S.testSoruldu` başlangıç nesnesinde yok (391–398) ve sıfırlanmıyor.
  - İlk `durumAl` 521'de çalışır, liste kurulmadan ve kare yüklenmeden önce. Görüntü yoksa 521'e hiç gelinmez (520).
- **Diğer girişler:**
  - Uyarı kartı: 1247–1249 → 1270
  - "değiştir" bağlantısı: 1311 → 1313
  - "Şimdi eğit": 1404, `test_onay` yoksa `testSor(true)` → onaydan sonra 1453
- **Dialog:**
  - HTML: 356–381
  - `testYardim` 1408–1421, `testSor` 1422–1435, `tKaynak.onchange` 1436–1440
  - Gönderim 1444–1455: ardından `bilgiYukle`, `S.testSurum=undefined`, `durumAl`, `kuyrukYukle(false)`, `listeKur`, toast (1451), rol ve şerit (1452).
  - Listeyi "test"e geçirmez.
- **Etiketleme gereği:**
  - Otomatik eğitim `test_eksik` false olana dek bekler: app.py 539, `test_eksik` 336.
  - Arayüzde bu iş için tıklanabilir bir yol yok, yalnız metin geçer:
    - Kısaca 1347: "test karelerini etiketle"
    - Adımlar 1318: "Test setini etiketle"
    - Uyarı 1265: "test seti tamamlanınca başlar"
    - Dialog 1413: "Etiketsiz olanları elle etiketlemen gerekir"
    - Şerit 780
- **Test karesi açılınca:**
  - Sunucu öneri ve aday göndermez: **app.py 685** (`if t and r == "havuz" and not denetim`).
  - Şerit metni 780.
- **`test_kaynak`** (app.py 905) seçilen kaynağı değil, durumu söyler: bütün test kareleri etiketliyse `"etiketli"` döner. 1311 bunu "etiketlilerden seçildi" diye gösterir. "tum" ile seçilip hepsi elle etiketlenmiş bir set de böyle görünür.
- **Test karelerine erişim (v3.6):**
  - `test` ve `tum` listeleri
  - `etiketsiz` (kare etiketsizse) ya da `etiketli` (kare etiketliyse)
  - A / D tuşları yalnız `S.order` içinde gezer.
- **HEAD (v3.7):** Ayrı bir test listesi yok, ama kareler erişilebilir:
  - Etiketsiz test kareleri ETİKETSİZ listesinin başına konur (HEAD 637–638).
  - Etiketliler ONAYLANDI listesinde.
  - Tampon kareler hiçbir listede yok (`kategori`, HEAD 616–620).
  - Eski `test` / `tum` seçimi `etiketsiz` olarak eşlenir (HEAD 405).

---

## 3. Kaydetme akışı (v3.6)

**D tuşu:**
- 1111: `if(!e.repeat) git(+1)`.
- ArrowRight buraya yalnız seçili kutu yokken ve işlem yokken ulaşır (1084, 1108).
- `git(+1)` → `load(j,true)` (627) → 551–553: `S.dirty || (onay && S.pre)` ise `save()`.
- Kirli olmayan ve öneri olmayan karede D kayıt yapmaz, emek kaydı da yazılmaz.
  - **Sonuç:** Önerisi olmayan boş bir kare (test, denetim, tahminsiz kare) "nesne yok" diye ancak Ctrl+S ile işaretlenir.
  - Öneri var ama 0 kutuysa `S.pre=true` olur (574). D boş dosya yazar.

**Diğer kaydetme yolları:**
- A / ArrowLeft (1112): yalnız kirliyse kaydeder.
- Listeden tıklama (843): yalnız kirliyse kaydeder.
- Liste sonu (620–622): kirli ya da öneri ise kaydeder.
- Ctrl+S (1094–1098): koşulsuz kaydeder (`S.idx<0` hariç, 530) ve "Kaydedildi" toast'ı gösterir.

**`save()`** (529–546):
- Gövde: `{boxes:[{cls,cx,cy,w,h}], emek:{sure_sn, eklenen, silinen, duzeltilen, sinif_degisen, kaynak, ilk_kutu}}`. Kutular 532–534'te üretilir, emek 538–539'da.
- `m` gönderilmez, `davranis` alanı yok.
- `S.emek` null ise `emek` alanı JSON'a girmez.
- `sure_sn = round(sure/100)/10`.
- Kayıttan sonra (541–545): sayaçlar sıfırlanır, `ilk=boxes.length`, `kaynak='duzenleme'`; görüntü işaretleri güncellenir; `b.m` silinir; `S.pre=false`; ardından yeniden çizim.

**`S.emek`** (579–580): `kaynak` değerleri:
- `model`: `lab.model`
- `duzenleme`: `lab.labeled`
- `oneri`: `S.pre`, yani `tahmin_var`
- `elle`: diğer durumlar

Test ve denetim karelerinde `tahmin_var` false olduğu için (app.py 685) bu kareler `elle` sayılır.

**`emekTik`** (901–905): `sure += min(Δ, 30000)`.
- Çağrıldığı yerler: mousedown (908), keydown (1081, yalnız 1080 süzgecinden sonra), `save` (536).
- mousemove, mouseup ve tekerlek süreye eklenmez.
- `son` alanı yüklemede ayarlanır (579). Açılıştan ilk olaya kadar geçen süre de en çok 30 sn olarak sayılır.

**Sunucu (app.py HEAD):**
- **`save_labels`** (699–729):
  - Değerler kırpılır, `w` ya da `h` ≤ 0 olan kutu atlanır (707–709).
  - Boş dosya "nesne yok" anlamına gelir (712–713).
  - Sırasıyla: `etiket_degisti` (714), `davranis_kaydet` (715), emek.jsonl satırı (716–724), M listesinden çıkarma (725–728).
  - Yanıt: `{ok, count}`.
- **`davranis_kaydet`** (732–739): `davranis` alanı sözlük değilse hiçbir şey yazmaz (734–735).
- **Emek fonksiyonları:**
  - `emek_kayitlari`: 757–761
  - `emek_ozeti`: 764–775, mtime ve boyuta göre önbellekli
  - `_emek_ozeti_hesapla`: 778–792. Yalnız sayısal `sure_sn` olan kayıtlar alınır (779). Son 50 kayıt üzerinden; `model` grubu = kaynak `model` ya da `oneri`, `elle` grubu = kaynak `elle`. `duzenleme` kaynaklı kayıtlar yalnız `son` grubunda sayılır.
  - CSV: 795–806, sütunlar doğru.
- **Arayüzde gösterim:** `#mdEmek` (319), metin 1281–1287.

**v3.7 sözleşmesi (davranis.py):**
- `kayit_olustur` 133–146 bekler: `oturum, liste, kaynak, rol, tahmin_tur, sure_sn, islem{…}, ilk`.
- `ISLEMLER` (42): `eklenen, aday_eklenen, silinen, tasinan, boyut, sinif_degisen, geri_al`.
- `kutu_listesi` (69–83): `{cls,cx,cy,w,h,m?}` biçiminde 0–1 aralığında kutular bekler.
- `fark` sunucuda, `ilk` ile yazılan kutular karşılaştırılarak hesaplanır (145).
- `MODEL_KAYNAKLARI=("oneri","model")` (44). M karelerinde açılış kutularında `m` yok (568–570). Bu yüzden `guven` None olur, ama `sinif_ozeti` (201–206) bu kutuları yine model kutusu sayar.

---

## 4. Kutu düzenleme olayları (v3.6)

| Olay | Yer | Emek sayacı | v3.7 `islem` |
|---|---|---|---|
| Yeni kutu | mouseup 995–998. Başlangıç: `startDraw` 920–923, çağrı 924 (çizim modu / Shift) ve 938. Hareket 953 | `eklenen++` 996 | `eklenen` |
| Aday ekleme | mouseup 985–991, yalnız düzenleme modunda. `adayBul` 446–455; kutu `m:a.s` ile eklenir | `eklenen++` 987 | `aday_eklenen` |
| Tıklama | 983–984 (seç), 993 (seçimi kaldır) | — | — |
| Silme | `removeBox` 875–880. Çağrılar: 1109, 810 | `silinen++` 878 | `silinen` |
| Sınıf değiştirme | `setClass` 866–874. Çağrılar: 1110, 1116–1118, 796 | `sinif++` 871. Sınıf aynı kalsa da `m` varsa artar (869) | `sinif_degisen` |
| Fareyle taşıma | 931–935, 958–962, onay 1000–1005 | `duzeltilen++` 1004. Tür ayırt edilmez | `tasinan` |
| Fareyle boyutlandırma | 926–929, 963–970, onay 1000–1005 | `duzeltilen++` 1004 | `boyut` |
| Ok tuşları | `kutuOkla` 1032–1057 (çağrı 1084–1087). Alt ile boyut 1041–1043, Alt'sız taşıma 1045–1047 | Yeni grup başında `duzeltilen++` 1038. Grup = aynı kutu ve **son basıştan** itibaren 800 ms; `t` her basışta yenilenir (1040), basılı tutmak grubu uzatır. Taşıma ile boyut aynı grupta birleşebilir | `tasinan` / `boyut` |
| Geri alma | Ctrl+Z 1099–1104 (işlem sürerken `cancelOp` 1101) | Sayaçlar azaltılmaz | `geri_al` |
| İptal | `cancelOp` 889–893. Çağrılar: Esc 1107, `load` 550, `setMode` 882 | — | — |
| Çok küçülen kutu | 1002–1003 | Sayılmaz | — |

**Ek bilgiler:**
- **`b.m` silindiği yerler:** 870, 1004, 1049, kayıtta 543.
  - `kutuOkla`, kutu kenara dayanıp hiç kımıldamasa bile `m`'yi siler ve sayacı artırır.
  - Fareyle taşıyıp aynı yere geri bırakmak da (`moved=true`) taşıma sayılır.
- **Geri alma yığını:** 414–415, en çok 100 adım.
- **`select` çağrıları** (865): 809, 918, 933, **937**, 984, 989, **993**, 998, 1071 (`kutuGez`), 1107.

---

## 5. Yoklama, toast, ayarlar (v3.6)

**Yoklama:**
- **Başlangıç:** `init` → `setTimeout(durumDongu,2000)`: görüntü yoksa 520, varsa 526.
- **Döngü:** `durumDongu` (1191) sıralı çalışır, istekler üst üste binmez.
  - **Ek:** `try/catch` yok. `durumAl` yalnız fetch hatasını yakalar (1151). `renderModel` ya da `klasorDegistiMi` içinde bir istisna olursa `setTimeout` hiç kurulmaz ve yoklama sayfa yenilenene dek durur.
- **`durumAl`** (1149–1166):
  - `tahmin_tur` değişince (ilk çağrı hariç): `kuyrukYukle(true)` çağrılır. Toast 5 sn gösterilir, yalnız `d.tahmin_tur` doluysa.
  - Kare dokunulmamışsa yeniden yüklenir (1162). Koşul: `!dirty && !pre && !labeled && !op`.
- **`klasorDegistiMi`** (1167–1190):
  - `S.op` varken atlanır.
  - Karşılaştırır: `toplam`, `etiketli_toplam` ile yerel `labeled` sayısı (M dahil), `test_surum`.
  - Değişiklik varsa: `bilgiYukle`, `kuyrukYukle`, `listeKur`, toast'lar (1183–1185), yeniden yükleme (1188–1189).
- **`/api/durum`** (app.py 881–909): 19 alan, liste doğru.
- **`sayimlar`** (app.py 311–342): 15 alan, liste doğru. Havuz ya da test olmayan her kare "tampon" sayılır (328–330).
- **`renderModel`** (1195–1338): bölüm listesi doğru. Ek olarak `#mdBarKutu`, `#bEgit` / `#bIptal` durumu (1229–1230) ve `kisacaHtml` 1341–1372, `egriSvg` 1374–1399.

**Toast:**
- Öğe `#toast` (244), CSS 70 ve 72–78.
- `toast()` (404–407): son mesaj öncekini ezer.
- Çağrı satırları grep ile birebir doğrulandı: 552, 561, 621, 623, 624, 990, 1003, 1054, 1096, 1115, 1118, 1120, 1124, 1126, 1160, 1183–1185, 1275, 1278, 1294, 1296, 1300, 1402, 1451, 1458, 1620, 1630.

**Ayarlar:**
- **HTML:** `dialog#dlg` 337–354.
- **`AYAR_FORM`** (1462–1513):
  - Temel ayarlar: 8 alan.
  - Gelişmiş ayarlar: 15 alan, `details` (üçüncü eleman `true`, 1512).
  - Listeler doğru.
- **Fonksiyonlar:**
  - `ayarFormuKur`: 1516–1542
  - İpucu: 1543–1551
  - `ayarlariAc`: 1552–1557 (`#bAyar` 1607)
  - `ayarMetni` 1559–1569, `panoya` 1570–1575, kopyala 1576–1578
- **`ayarUygula`** (1579–1606):
  - `k = v` ya da `k: v` satırları (1589) veya JSON alır. Ondalık virgülü noktaya çevirir (1592).
  - Yalnız bilinen anahtarları geçirir (1597).
  - Sonrasında `durumAl`, `ayarFormuKur`, `kuyrukYukle`, `listeKur` çağrılır. `bilgiYukle` yok.
- **Form gönderimi** (1609–1627): `bilgiYukle`, `durumAl`, `kuyrukYukle`, `listeKur`, ardından rol ve şerit (1625).

**Klavye:**
- 1080: dialog açıkken ya da odak INPUT / SELECT / TEXTAREA'dayken keydown hiçbir şey yapmaz. BUTTON ve DIV bu listede yok.
- keyup (1128) bu süzgeçten geçmez.

**Tab:**
- 1089: Tab yalnız `activeElement` `body` ya da `cv` iken çalışır. `cv`'nin tabindex'i olmadığı için pratikte yalnız `body`.
- **Ek:** Liste satırlarında `tabIndex=0` var (827). Bir satıra tıklayınca odak o satıra geçer, Tab kutuları değil sayfa odağını gezdirir.
- Odağı bırakan yerler: tuvale tıklama (910) ve select değişimi (1137).
- Select yerine düğme konursa aynı `blur()` gerekir. HEAD bunu yapıyor: 1161 ve 1162.

---

## Düzeltmeler:

1. **Sürüm durumu:** Yarım diff artık `56f2a51` olarak commit edildi, ağaç temiz. `da8f5c0` HEAD değil, HEAD~1. davranis.py git'te.
2. **"Sayfa açılmaz":** Commit edilen sürümde geçerli değil. `#gorunum` ve büyüteç JS'i kaldırılmış, bütün `$('#id')` başvuruları çözülüyor. İddia yalnız artık var olmayan ara durum için geçerliydi.
3. **"Yarım diff'te test için yol yok":** HEAD'de etiketsiz test kareleri ETİKETSİZ listesinin başında, etiketliler ONAYLANDI'da. Tampon hiçbir listede yok.
4. **app.py 682 → 685.** Öneri ve aday koşulu 685'te. 682 `t = DURUM.get("tahmin")` satırı.
5. **`/api/kuyruk` dal koşulu** "model var / yok" değil, `t and DURUM["kuyruk"]` (874). Model olsa da kuyruk boşsa tohum yanıtı döner.
6. **"Test ve tampon Öneri sırasında asla görünmez" fazla kesin.** `test_seti_sec` `kuyruk_kur` çağırmıyor. Sıfırlamadan sonra eski kuyruk kalabiliyor, denetim de rol ile süzülmüyor (374).
7. **Yüklemede (581)** `count` değişmez, yalnız `labeled` ve `model` değişir.
8. **Küçük satır kaymaları:** `/api/info` 648–663, `/api/kuyruk` 869–878, `/api/durum` 881–909, `sayimlar` 311–342, `davranis_kaydet` koşulu 734–735. davranis.py: 133–146, 42, 69–83.
9. **`select` çağrı listesine** 937 ve 993 eklendi.
10. **Ok tuşu grubu:** 800 ms son basıştan ölçülür (1040).
11. **Uyarı 1265** metni "test karelerini etiketle" değil, "test seti tamamlanınca başlar". Dialog metni 1413 eklendi.
12. **OKUBENI.md (HEAD)** büyüteci hâlâ 36 ve 44'te (v3.6 notları) anıyor, 8'de kaldırıldığını yazıyor.
13. **Eklenenler:**
    - `durumDongu`'da `try` yok, bir istisna yoklamayı kalıcı durdurur.
    - Liste satırı odak alır (`tabIndex=0`), bu da Tab'ı etkiler.
    - Test ve denetim kareleri emekte `elle` sayılır.
    - Önerisiz boş kare D ile kaydedilmez, Ctrl+S gerekir.
    - `test_kaynak` seçimi değil, durumu yansıtır.
    - M kutularında `m` yok ama `sinif_özeti` bunları model kutusu sayar.
    - CSS 68 de inline stil yüzünden etkisiz.