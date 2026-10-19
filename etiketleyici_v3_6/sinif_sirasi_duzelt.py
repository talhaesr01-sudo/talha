"""
Sınıf sırasını Teknofest resmi sırasına çevirir: tasit=0, insan=1.

Şu anki etiketler insan=0, tasit=1 sırasında. Bu betik tüm .txt etiket
dosyalarında 0 ile 1'i yer değiştirir ve classes.txt'yi düzeltir.

Güvenlik:
  - Varsayılan olarak SADECE RAPOR verir, hiçbir şeyi değiştirmez.
  - --uygula verilirse önce etiket klasörünün tam yedeğini alır.
  - classes.txt zaten "tasit" ile başlıyorsa durur (iki kez çalıştırılıp
    etiketlerin geri bozulmasını önler).

Kullanım (önce etiketleyiciyi kapat):
    python sinif_sirasi_duzelt.py --labels "C:/.../veriseti"            (deneme)
    python sinif_sirasi_duzelt.py --labels "C:/.../veriseti" --uygula   (uygula)
"""
import argparse
import shutil
from datetime import datetime
from pathlib import Path

SWAP = {"0": "1", "1": "0"}

ap = argparse.ArgumentParser()
ap.add_argument("--labels", required=True)
ap.add_argument("--uygula", action="store_true")
a = ap.parse_args()

lab = Path(a.labels).resolve()
cls_file = lab / "classes.txt"
if not cls_file.exists():
    raise SystemExit(f"classes.txt bulunamadı: {cls_file}")
classes = [l.strip() for l in cls_file.read_text(encoding="utf-8").splitlines() if l.strip()]
print("Şu anki sınıflar:", classes)
if classes[:2] != ["insan", "tasit"]:
    raise SystemExit("classes.txt 'insan, tasit' sırasında değil; zaten düzeltilmiş olabilir. Hiçbir şey yapılmadı.")

# Etiketler resimlerle aynı klasördeyse sadece bir resme ait .txt dosyalarına dokunulur (not dosyaları korunur).
RESIM = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}
resim_kokleri = {p.stem for p in lab.iterdir() if p.suffix.lower() in RESIM}
files = [f for f in sorted(lab.glob("*.txt"))
         if f.name != "classes.txt" and (not resim_kokleri or f.stem in resim_kokleri)]
n0 = n1 = other = 0
for f in files:
    for line in f.read_text(encoding="utf-8").splitlines():
        c = line.split()[0] if line.strip() else None
        if c == "0": n0 += 1
        elif c == "1": n1 += 1
        elif c is not None: other += 1
print(f"{len(files)} etiket dosyası | insan(0): {n0} kutu | tasit(1): {n1} kutu | diğer: {other}")

if not a.uygula:
    print("\nDENEME modu: hiçbir dosya değişmedi. Uygulamak için sonuna --uygula ekle.")
    raise SystemExit

backup = lab.parent / f"{lab.name}_etiket_yedek_{datetime.now():%Y%m%d_%H%M%S}"
backup.mkdir()
for f in files + [cls_file]:  # sadece etiket dosyaları yedeklenir (resimler kopyalanmaz)
    shutil.copy2(f, backup / f.name)
print("Yedek alındı:", backup)

for f in files:
    out = []
    for line in f.read_text(encoding="utf-8").splitlines():
        p = line.split()
        if p and p[0] in SWAP:
            p[0] = SWAP[p[0]]
        out.append(" ".join(p))
    f.write_text("\n".join(out) + ("\n" if out else ""), encoding="utf-8")

cls_file.write_text("\n".join(["tasit", "insan"] + classes[2:]) + "\n", encoding="utf-8")
print(f"Tamam. Artık tasit=0 ({n1} kutu), insan=1 ({n0} kutu).")
