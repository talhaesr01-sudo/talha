# Literatür eşlemesi: YOLO11 / RTX3050 6GB için hafif veri seti üretim aracı

**İşaretler:** **(W)** künye bu oturumda web aramasıyla doğrulandı. İşaretsiz olanlar alanın iyi bilinen künyeleri; web ile ayrıca kontrol edilmedi. **(doğrulanmadı)** künye teyit edilemedi. **[ek]** listede yoktu, ilgili olduğu için eklendi.

**Ortak hafiflik kuralları:**
- Görüntü başına en fazla 2 ileri geçiş yapılır: orijinal görüntü ve yatay flip.
- MC dropout ve ensemble kullanılmaz. Gerekirse 2 üyeli "komite" kurulur: mevcut model ile EMA ağırlıkları ya da bir önceki turun checkpoint'i. Bunun için ek eğitim gerekmez.
- YOLO11 sınıf skorları softmax değil, bağımsız sigmoid'dir.
  - Belirsizlik ölçüsü olarak şunlar kullanılır: `1-conf`, en yüksek iki sınıf skorunun farkı (top-2 margin) ya da en yüksek skorun ikili entropisi.
  - Top-2 için NMS öncesi sınıf vektörünün saklanması gerekir. NMS sonrası yalnızca en yüksek skor kalır.

---

## 1. Nesne algılamada aktif öğrenme (detection active learning)

**Belirsizlik (uncertainty)**

- **Haussmann, E., Fenzi, M., Chitta, K., ve ark.**, "Scalable Active Learning for Object Detection", *IEEE Intelligent Vehicles Symposium (IV)*, 2020. **(W)**
  - Yöntem: ensemble ve görüntü düzeyinde bilgi skoru.
  - Hafif uygulama: 5'li ensemble yerine 2 üyeli komite (model ve EMA) kullanılır. Görüntü skoru, en belirsiz top-k kutunun ortalamasıdır.
- **[ek] Brust, C.-A., Käding, C., Denzler, J.**, "Active Learning for Deep Object Detection", *VISAPP*, 2019. **(W)**
  - Yöntem: herhangi bir detektörle çalışan margin tabanlı seçim; sınıf dengesizliğini ağırlıklandırır.
  - Hafif uygulama: YOLO11 için en ucuz başlangıç noktası. Kutu başına top-2 margin hesaplanır, görüntüde toplam veya max alınır, nadir sınıflara ters frekans ağırlığı verilir.
- **Yu, W., Zhu, S., Yang, T., Chen, C.**, "Consistency-based Active Learning for Object Detection" (CALD), *CVPR Workshops (L3D-IVU)*, 2022. **(W)**
  - Not: yayın ana CVPR değil, CVPRW'dir.
  - Hafif uygulama: flip ile 2 geçiş yapılır. Eşleşen kutularda tutarsızlık skoru = `(1−IoU) + |Δskor| + sınıf uyuşmazlığı`. Bu skor hem sınıf hem konum belirsizliğini yakalar.
- **Choi, J., Elezi, I., Lee, H.-J., Farabet, C., Alvarez, J. M.**, "Active Learning for Deep Object Detection via Probabilistic Modeling", *ICCV*, 2021.
  - Mixture density network (MDN) başlığı mimari değişiklik gerektirdiği için varsayılan olarak kapalıdır.
  - Alınan fikir: sınıf ve konum belirsizliği ayrı ayrı ölçülür, normalize edilip birleştirilir. Konum belirsizliği için flip-IoU sapması kullanılır.
- **Yuan, T., Wan, F., Fu, M., ve ark.**, "Multiple Instance Active Learning for Object Detection" (MI-AOD), *CVPR*, 2021.
  - Karşıt (adversarial) sınıflandırıcı çifti varsayılan olarak kapalıdır.
  - Alınan fikir (multiple-instance): görüntü skoru tüm kutuların ortalaması değil, top-k kutudan hesaplanır. Böylece arka plan gürültüsü bastırılır.

**Çeşitlilik ve temsil (diversity / representativeness)**

- **Sener, O., Savarese, S.**, "Active Learning for Convolutional Neural Networks: A Core-Set Approach", *ICLR*, 2018.
  - Hafif uygulama: YOLO11 gömmesi (`model.embed()`, global average pooling) üzerinde greedy k-center çalıştırılır.
  - Aday havuzu önce belirsizlikle daraltılır (bütçenin 3–5 katı). Bu boyutta CPU yeterlidir.
- **Ash, J. T., Zhang, C., Krishnamurthy, A., Langford, J., Agarwal, A.**, "Deep Batch Active Learning by Diverse, Uncertain Gradient Lower Bounds" (BADGE), *ICLR*, 2020.
  - Hafif uygulama (kendi yaklaşımım): gradyan gömmesi yerine "gömme × görüntü belirsizliği" üzerinde k-means++ tohumlama yapılır. Ekstra geri yayılım (backward) gerekmez.
- **Yang, C., Huang, L., Crowley, E. J.**, "Plug and Play Active Learning for Object Detection" (PPAL), *CVPR*, 2024. **(W)**
  - Önerilen varsayılan seçim stratejisi. İki aşamalıdır:
    1. Zorluk katsayısıyla ağırlıklandırılmış belirsizliğe göre top-M aday seçilir.
    2. k-means++ ile çeşitli bir alt küme alınır.
  - CCMS benzerliği yerine sınıf histogramı ve gömmenin kosinüs benzerliği kullanılır.

## 2. Yarı denetimli algılama, pseudo-label ve öğretmen-öğrenci

- **Tarvainen, A., Valpola, H.**, "Mean teachers are better role models: Weight-averaged consistency targets improve semi-supervised deep learning results", *NeurIPS*, 2017.
  - Hafif uygulama: Ultralytics eğitimde zaten EMA tutar (`ModelEMA`). Öğretmen olarak EMA ağırlıkları kullanılır, ek eğitim maliyeti yoktur.
- **Sohn, K., Zhang, Z., Li, C.-L., Zhang, H., Lee, C.-Y., Pfister, T.**, "A Simple Semi-Supervised Learning Framework for Object Detection" (STAC), *arXiv:2005.04757*, 2020. **(W)**
  - Not: hakemli yayın yeri bulunamadı; arXiv olarak atıf yapılmalı.
  - Hafif uygulama: en ucuz şema budur. Pseudo-label bir kez, yüksek eşikle offline üretilir.
  - Bu araçta pseudo-label'lar öncelikle insanın doğrulayacağı ön-etiket olarak kullanılır.
- **Liu, Y.-C., Ma, C.-Y., He, Z., ve ark.**, "Unbiased Teacher for Semi-Supervised Object Detection", *ICLR*, 2021.
  - Hafif uygulama: nadir sınıflarda pseudo-label kıtlığı izlenir, eşik sınıf frekansına göre gevşetilir.
- **Xu, M., Zhang, Z., Hu, H., ve ark.**, "End-to-End Semi-Supervised Object Detection with Soft Teacher", *ICCV*, 2021.
  - YOLO'da RoI başlığı olmadığı için box jittering doğrudan uygulanamaz.
  - Ucuz karşılığı: flip-TTA'da kutu sapması. Sapması yüksek kutular "kutuyu kontrol et" kuyruğuna gider.
- **Wang, X., Yang, X., Zhang, S., ve ark.**, "Consistent-Teacher: Towards Reducing Inconsistent Pseudo-targets in Semi-supervised Object Detection", *CVPR*, 2023.
  - Hafif uygulama: sınıf başına 2 bileşenli GMM ile dinamik eşik belirlenir (sklearn, CPU). Sabit 0.5/0.7 eşiğinin yerini alır.
  - Bu araç için bu bölümdeki en yararlı fikir.
- **Zhou, H., Ge, Z., Liu, S., ve ark.**, "Dense Teacher: Dense Pseudo-Labels for Semi-supervised Object Detection", *ECCV*, 2022.
  - Özel kayıp fonksiyonu gerektirdiği için varsayılan olarak kapalıdır. Yalnızca "NMS'siz, top-%k yoğun skor" fikri not edilir.
- **[ek] Xu, B. ve ark.**, "Efficient Teacher: Semi-Supervised Object Detection for YOLOv5", *arXiv:2302.07577*, 2023. **(W)**
  - YOLO ailesine en yakın çalışma. Pseudo Label Assigner fikriyle üç bant kurulur:
    - Yüksek bant: otomatik kabul adayı (örneklemeli denetimle).
    - Orta bant: insan doğrulama kuyruğu.
    - Düşük bant: atılır.

## 3. Kalibrasyon

- **Guo, C., Pleiss, G., Sun, Y., Weinberger, K. Q.**, "On Calibration of Modern Neural Networks", *ICML*, 2017.
  - Hafif uygulama (temperature scaling):
    - `logit = ln(p/(1−p))`, kalibre skor `p' = σ(logit/T)`.
    - T, doğrulama kümesindeki TP/FP eşleşmeleri (IoU≥0.5) üzerinde NLL ile fit edilir.
  - Skor sıralaması değişmez, dolayısıyla mAP aynı kalır. Değişen, eşiklerin anlamıdır.
- **Platt, J. C.**, "Probabilistic Outputs for Support Vector Machines and Comparisons to Regularized Likelihood Methods", *Advances in Large Margin Classifiers*, MIT Press, 1999.
  - Hafif uygulama: `σ(a·logit + b)`, 2 parametre. Birkaç yüz eşleşmeyle bile kararlıdır; küçük veri setinde varsayılan budur.
- **Zadrozny, B., Elkan, C.**, "Transforming Classifier Scores into Accurate Multiclass Probability Estimates", *KDD*, 2002. **(W)**
  - Hafif uygulama: `IsotonicRegression` kullanılır. Bu yöntem çok veri ister; yaklaşık 1000'den fazla eşleşme olunca Platt yerine geçilir (pratik eşik, benim önerim).
- **Naeini, M. P., Cooper, G. F., Hauskrecht, M.**, "Obtaining Well Calibrated Probabilities Using Bayesian Binning", *AAAI*, 2015.
  - Hafif uygulama: ECE ve MCE 10–15 kutucukla raporlanır. BBQ gerekmez.
  - Az veride MCE gürültülüdür; az dolu kutucuklar raporda işaretlenmelidir.
- **Küppers, F., Kronenberger, J., Shantia, A., Haselhoff, A.**, "Multivariate Confidence Calibration for Object Detection", *CVPR Workshops*, 2020. **(W)**
  - Hafif uygulama: D-ECE için güven skoru ve kutu boyutu (küçük/orta/büyük) birlikte 2 boyutlu kutucuklanır. Her boyut grubuna ayrı T verilir.
  - Uyarı: farklı boyut veya kutucuk sayısıyla hesaplanmış D-ECE değerleri birbirleriyle kıyaslanamaz.
- **[ek] Kuzucu, S., Oksuz, K., Sadeghi, J., Dokania, P. K.**, "On Calibration of Object Detectors: Pitfalls, Evaluation and Baselines", *ECCV*, 2024. **(W)**
  - Bulgusu: post-hoc Platt ve isotonic, eğitim sırasında yapılan kalibrasyondan daha iyi sonuç veriyor. Bu, projenin yaklaşımını destekler.
  - Kalibrasyon, operasyon eşiğinden sonra kalan kutular üzerinde yapılmalıdır. Konum-farkında ECE (LaECE) de değerlendirilebilir.

## 4. Etiket gürültüsü ve etiketleme hatası tespiti

- **Northcutt, C. G., Jiang, L., Chuang, I. L.**, "Confident Learning: Estimating Uncertainty in Dataset Labels", *JAIR* 70, 2021.
  - Hafif uygulama: eşleşmiş GT kutusu–tahmin çiftlerinde sınıf düzeyinde CL uygulanır.
    - Sınıf eşiği `t_j`, j etiketli kutulardaki ortalama `p_j`'dir.
    - Karışıklık tablosunun köşegeni dışındaki sayımlar olası yanlış sınıfları gösterir.
  - Örneklem dışı (out-of-sample) tahmin gerekir. AL döngüsünde t. turda eklenen görüntüleri t−1 modeli hiç görmemiştir, bu yüzden k-fold'a gerek kalmaz.
- **Tkachenko, U., Thyagarajan, A., Mueller, J.**, "ObjectLab: Automated Diagnosis of Mislabeled Images in Object Detection Data", *ICML 2023 DMLR Workshop*, arXiv:2309.00832. **(W)**
  - Üç alt skor: kaçırılmış kutu, kötü konumlanmış kutu, yanlış sınıf. Bunlar görüntü kalite skorunda birleşir.
  - `cleanlab` kütüphanesinin `object_detection` modülü CPU'da YOLO çıktısıyla kullanılabilir. Kalibre edilmiş skorlarla beslenmelidir.
- **Schubert, M., Riedlinger, T., Kahl, K., Kröll, D., Schoenen, S., Šegvić, S., Rottmann, M.**, "Identifying Label Errors in Object Detection Datasets by Loss Inspection", *WACV*, 2024. **(W)**
  - Yöntem: GT kutusu başına kayba (sınıf + regresyon) göre sıralama. Makale iki aşamalı detektör kullanıyor.
  - YOLO yaklaşığı: `(1−p_sınıf) + (1−IoU_en iyi eşleşme)`. Yalnızca örneklem dışı tahminlerle hesaplanır.
- **Köksal, A., İnce, K. G., Alatan, A. A.**, "Effect of Annotation Errors on Drone Detection with YOLOv3", *CVPR Workshops*, 2020. **(W)**
  - Hafif uygulama: makaledeki hata simülasyonu, aracın hata dedektörünü test etmek için sentetik gürültü protokolü olarak kullanılır (eksik, kaymış, yanlış sınıflı kutu).
- **"Hu et al."**: görevde verilen künyeyle birebir eşleşen bir makale bulunamadı **(doğrulanmadı)**.
  - En yakın aday: **Wang, S., Gao, J., Li, B., Hu, W.**, "Narrowing the Gap: Improved Detector Training with Noisy Location Annotations", *arXiv:2206.05708*, 2022 **(W)**. IEEE TIP 2022'de yayımlandığı **(doğrulanmadı)**.
  - Hafif uygulama: flip-TTA ortalama kutusu kullanıcıya "önerilen düzeltme" olarak sunulur.

## 5. Zor örnek madenciliği (hard example mining)

- **Shrivastava, A., Gupta, A., Girshick, R.**, "Training Region-based Object Detectors with Online Hard Example Mining", *CVPR*, 2016.
  - YOLO kaybına dokunulmaz. Bunun yerine görüntü düzeyinde offline HEM yapılır: FN+FP sayısı yüksek görüntüler sonraki turda en fazla 2–3 kat tekrar örneklenir.
- **Lin, T.-Y., Goyal, P., Girshick, R., He, K., Dollár, P.**, "Focal Loss for Dense Object Detection", *ICCV*, 2017.
  - Focal kayba geçiş varsayılan değildir; YOLO11'in BCE ve TAL hedefleri korunur.
  - Fikir yalnızca öncelik puanında kullanılır: `(1−p)^γ` ağırlığı (γ≈2) kolay örnekleri bastırır.
- **[ek] Gupta, A., Dollár, P., Girshick, R.**, "LVIS: A Dataset for Large Vocabulary Instance Segmentation", *CVPR*, 2019.
  - Hafif uygulama (repeat factor sampling): `r_c = max(1, √(t/f_c))`. Görüntünün tekrar sayısı, içindeki sınıfların en büyük `r_c` değeridir.
- **Uyarı:** zor örnek ile gürültülü etiket aynı şey değildir. Tekrar örneklemeden önce bölüm 4'teki gürültü taramasından geçirilmelidir; yoksa hatalı etiketler pekiştirilir.

## 6. Unutma ve tahmin kararlılığı (self check)

- **Toneva, M., Sordoni, A., des Combes, R. T., Trischler, A., Bengio, Y., Gordon, G. J.**, "An Empirical Study of Example Forgetting during Deep Neural Network Learning", *ICLR*, 2019.
  - Tanım: bir GT kutusu, IoU≥0.5 ve doğru sınıfla τ üzerinde eşleştiğinde "öğrenilmiş" sayılır.
  - Birkaç checkpoint'te yalnızca inference yapılır (her 5 epoch'ta ya da her AL turunda). Kutu başına "öğrenildi → unutuldu" geçişleri sayılır.
  - Sık unutulan kutular zor örnek ya da hatalı etiket adayıdır.
- **[ek] Swayamdipta, S., Schwartz, R., Lourie, N., Wang, Y., Hajishirzi, H., Smith, N. A., Choi, Y.**, "Dataset Cartography: Mapping and Diagnosing Datasets with Training Dynamics", *EMNLP*, 2020. **(W)**
  - Hafif uygulama: aynı checkpoint tahminlerinden kutu başına güven ortalaması ve varyansı hesaplanır.
  - "Öğrenilmesi zor" bölge (düşük ortalama, düşük varyans) etiket hatasına işaret eder.
- **[ek] Pleiss, G., Zhang, T., Elenberg, E., Weinberger, K. Q.**, "Identifying Mislabeled Data Using the Area Under the Margin Ranking", *NeurIPS*, 2020. **(W)**
  - Hafif uygulama: margin = atanan sınıfın skoru − en yüksek diğer sınıf skoru. Checkpoint'ler boyunca ortalaması alınır; negatif AUM yanlış sınıf adayıdır.
- **Self check:** t ve t−1 turlarının tahminleri karşılaştırılır (eşleşme oranı, ortalama IoU, sınıf değişimi). Ek eğitim maliyeti yoktur; sonuç hem AL hem gürültü sinyali olarak kullanılır.

## 7. HCI ve insan-döngüde etiketleme

- **Papadopoulos, D. P., Uijlings, J. R. R., Keller, F., Ferrari, V.**, "Extreme Clicking for Efficient Object Annotation", *ICCV*, 2017. **(W)**
  - Bulgu: kutu başına yaklaşık 7 sn, geleneksel çizimden yaklaşık 5 kat hızlı.
  - Hafif uygulama: arayüze 4 uç nokta tıklama modu eklenir. Kutu bu noktaların min/max'ından çıkar; model gerekmez.
- **Papadopoulos, D. P., Uijlings, J. R. R., Keller, F., Ferrari, V.**, "We Don't Need No Bounding-Boxes: Training Object Class Detectors Using Only Human Verification", *CVPR*, 2016. **(W)**
  - Bulgu: yalnızca evet/hayır doğrulamasıyla 6–9 kat zaman tasarrufu.
  - Hafif uygulama: yüksek güvenli ön-etiketler tek tuşla kabul veya ret edilir; reddedilenler çizim kuyruğuna gider.
- **Konyushkova, K., Uijlings, J., Lampert, C. H., Ferrari, V.**, "Learning Intelligent Dialogs for Bounding Box Annotation", *CVPR*, 2018. **(W)**
  - Pekiştirmeli öğrenme ajanı yerine makaledeki daha basit "kabul olasılığı tahmini" varyantı kullanılır.
  - Kabul olasılığı P, kalibre güven, kutu boyutu ve sınıftan lojistik regresyonla tahmin edilir.
  - Karar kuralı: `P > t_v / t_d` ise doğrulama istenir, değilse çizim. `t_v` ve `t_d` kullanıcının ölçülen doğrulama ve çizim süreleridir.
- **Adhikari, B., Huttunen, H.**, "Iterative Bounding Box Annotation for Object Detection", *ICPR 2020*. **(W)**
  - Not: IEEE kaydında yıl 2021; DOI 10.1109/ICPR48806.2021.9412956.
  - Bulgu: manuel eforda %75'e varan azalma.
  - Aracın temel döngüsü bu çalışmayla aynıdır. Makaledeki simülasyon protokolü (insan yerine mevcut GT kullanmak), eşik ve strateji ayarı için offline A/B testi sağlar.

## 8. Kullanıcı ve annotator modelleme

- **Dawid, A. P., Skene, A. M.**, "Maximum Likelihood Estimation of Observer Error-Rates Using the EM Algorithm", *Journal of the Royal Statistical Society, Series C (Applied Statistics)*, 28(1), 1979.
  - Hafif uygulama: annotator başına K×K karışıklık matrisi tutulur; buna "arka plan / kaçırılmış" sınıfı da eklenir.
  - Tek annotator varsa model ikinci gözlemci olarak alınır. EM CPU'da anlık çalışır.
- **Raykar, V. C., Yu, S., Zhao, L. H., Valadez, G. H., Florin, C., Bogoni, L., Moy, L.**, "Learning From Crowds", *JMLR* 11:1297–1322, 2010. **(W)**
  - Hafif uygulama: ortak sınıflandırıcı ve annotator EM'i yerine yalnızca annotator'ın duyarlılık/özgüllüğü (veya ortalama IoU'su) tahmin edilir.
- **[ek] Kim, H.-C., Ghahramani, Z.**, "Bayesian Classifier Combination" (IBCC), *AISTATS* 2012, PMLR 22:619–627. **(W)**
  - Küçük örneklemde yumuşatma için Dirichlet önseli kullanılır: `π̂_jk = (n_jk + α_jk) / (n_j + Σα)`.
    - Laplace yumuşatma: α=1.
    - Köşegene ağırlık verilirse (`α_jj > α_jk`) yeni annotator "orta-iyi" kabul edilir.
  - Doğruluk için Beta-Binomial: `(k+a)/(n+a+b)`. a ve b ekip ortalamasından empirical Bayes ile alınır (ör. `a = μm`, `b = (1−μ)m`, m≈10).
  - Posterior alt %5 sınırı eşiğin altındaysa o annotator'ın denetim oranı artırılır.

---

## Varsayılan hafif reçete

1. **Seçim:** PPAL tipi iki aşama. Önce margin+flip tutarlılığına göre top-M aday, sonra YOLO gömmesi üzerinde k-center/k-means++.
2. **Ön-etiketleme:** EMA öğretmen, sınıf başına GMM eşiği ve üç bant (otomatik kabul / doğrula / at).
3. **Kalibrasyon:** önce Platt veya T; 1000'den fazla eşleşme olunca isotonic. ECE/MCE ve boyut gruplu D-ECE raporlanır.
4. **Kalite:** CL ve ObjectLab skorları, t−1 modelinin örneklem dışı tahminleriyle hesaplanır. Bunlara forgetting/AUM ve tur arası kararlılık eklenir.
5. **Arayüz:** kabul olasılığına göre doğrula veya çiz kararı; ayrıca extreme click modu.
6. **Kullanıcı modeli:** Dirichlet / Beta-Binomial ile yumuşatılmış Dawid-Skene.

## Doğrulama kaynakları
- [CALD (CVPRW 2022)](https://openaccess.thecvf.com/content/CVPR2022W/L3D-IVU/html/Yu_Consistency-Based_Active_Learning_for_Object_Detection_CVPRW_2022_paper.html)
- [PPAL (CVPR 2024)](https://openaccess.thecvf.com/content/CVPR2024/html/Yang_Plug_and_Play_Active_Learning_for_Object_Detection_CVPR_2024_paper.html)
- [Haussmann (IV 2020)](https://avg.is.mpg.de/publications/chitta_2020_1_cd)
- [Brust (VISAPP 2019)](https://scitepress.org/Papers/2019/72486/)
- [STAC (arXiv)](https://arxiv.org/abs/2005.04757v2)
- [Efficient Teacher](https://arxiv.org/pdf/2302.07577)
- [Küppers (CVPRW 2020)](https://openaccess.thecvf.com/content_CVPRW_2020/html/w20/Kuppers_Multivariate_Confidence_Calibration_for_Object_Detection_CVPRW_2020_paper.html)
- [Kuzucu (ECCV 2024)](https://www.ecva.net/papers/eccv_2024/papers_ECCV/html/3148_ECCV_2024_paper.php)
- [Zadrozny & Elkan (KDD 2002)](https://www.cs.cornell.edu/courses/cs678/2007sp/ZadroznyElkan.pdf)
- [ObjectLab](https://arxiv.org/pdf/2309.00832)
- [Schubert (WACV 2024)](https://openaccess.thecvf.com/content/WACV2024/html/Schubert_Identifying_Label_Errors_in_Object_Detection_Datasets_by_Loss_Inspection_WACV_2024_paper.html)
- [Köksal (CVPRW 2020)](https://openaccess.thecvf.com/content_CVPRW_2020/html/w69/Koksal_Effect_of_Annotation_Errors_on_Drone_Detection_With_YOLOv3_CVPRW_2020_paper.html)
- [Wang/Gao/Li/Hu 2022](https://arxiv.org/pdf/2206.05708)
- [AUM (NeurIPS 2020)](https://papers.nips.cc/paper/2020/hash/c6102b3727b2a7d8b1bb6981147081ef-Abstract.html)
- [Dataset Cartography](https://preview.aclanthology.org/setup/2020.emnlp-main.746)
- [Extreme clicking](https://openaccess.thecvf.com/content_iccv_2017/html/Papadopoulos_Extreme_Clicking_for_ICCV_2017_paper.html)
- [We don't need no bounding-boxes](https://openaccess.thecvf.com/content_cvpr_2016/html/Papadopoulos_We_Dont_Need_CVPR_2016_paper.html)
- [Konyushkova (CVPR 2018)](https://openaccess.thecvf.com/content_cvpr_2018/html/Konyushkova_Learning_Intelligent_Dialogs_CVPR_2018_paper.html)
- [Adhikari & Huttunen](https://arxiv.org/pdf/2007.00961)
- [Raykar (JMLR 2010)](https://jmlr.org/papers/v11/raykar10a.html)
- [IBCC (AISTATS 2012)](https://proceedings.mlr.press/v22/kim12.html)