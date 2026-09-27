// =====================================================
// ROBOLIG v9 - GOMULU WEB SAYFASI (3D ARAYUZ)
//
// Bu dosya ROBOLIG_v9_tek.ino ile AYNI KLASORDE olmali.
// Sayfa ayri bir .h dosyasinda tutulur: Arduino IDE .ino dosyalarini
// tararken ham metnin icindeki JavaScript "function ...(...){" satirlarini
// C++ fonksiyonu sanip prototip ekliyor ve derleme hata veriyordu.
// .h dosyalari bu taramadan gecmez.
// =====================================================

#pragma once
#include <Arduino.h>

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>ROBOLIG KONTROL</title>
<style>
:root{
  box-sizing:border-box;padding-top:env(safe-area-inset-top,0px);padding-bottom:env(safe-area-inset-bottom,0px);
  --bg1:#eef2ff;--bg2:#dbe4ff;--card:rgba(255,255,255,.78);--line:rgba(60,80,160,.2);
  --txt:#0f172a;--sub:#475569;--cyan:#0891b2;--mag:#a21caf;--grn:#059669;--yel:#b45309;--red:#e11d48;
  --btn1:rgba(255,255,255,.95);--btn2:rgba(220,228,255,.95);--glowA:#3b82f633;--glowB:#d946ef22;
  --canvas:radial-gradient(circle at 50% 30%,#12204a,#070b1c);
}
:root[data-theme="dark"]{
  --bg1:#050816;--bg2:#0b1330;--card:rgba(20,30,60,.55);--line:rgba(120,160,255,.18);
  --txt:#e8eeff;--sub:#8fa3d1;--cyan:#22d3ee;--mag:#e879f9;--grn:#34d399;--yel:#fbbf24;--red:#f43f5e;
  --btn1:rgba(60,90,170,.45);--btn2:rgba(30,45,95,.55);--glowA:#1e3a8a55;--glowB:#86198f44;
}
@media (prefers-color-scheme: dark){
  :root:not([data-theme="light"]){
    --bg1:#050816;--bg2:#0b1330;--card:rgba(20,30,60,.55);--line:rgba(120,160,255,.18);
    --txt:#e8eeff;--sub:#8fa3d1;--cyan:#22d3ee;--mag:#e879f9;--grn:#34d399;--yel:#fbbf24;--red:#f43f5e;
    --btn1:rgba(60,90,170,.45);--btn2:rgba(30,45,95,.55);--glowA:#1e3a8a55;--glowB:#86198f44;
  }
}
html{scroll-padding-top:env(safe-area-inset-top,0px)}
*{-webkit-tap-highlight-color:transparent}
*,*::before,*::after{box-sizing:border-box}
html,body{margin:0;min-height:100%}
body{
  background:radial-gradient(1200px 600px at 10% -10%,var(--glowA),transparent),
             radial-gradient(900px 500px at 110% 10%,var(--glowB),transparent),
             linear-gradient(180deg,var(--bg1),var(--bg2));
  background-attachment:fixed;
  color:var(--txt);font-family:system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif;
  user-select:none;padding-bottom:100px;
}
.wrap{max-width:1240px;margin:auto;padding:14px}
header{display:flex;flex-wrap:wrap;align-items:center;gap:10px;margin-bottom:12px}
.logo{font-weight:900;font-size:24px;letter-spacing:2px;
  background:linear-gradient(90deg,var(--cyan),var(--mag));-webkit-background-clip:text;background-clip:text;color:transparent}
.logo small{font-size:11px;letter-spacing:4px;color:var(--sub);-webkit-text-fill-color:var(--sub);margin-left:8px}
.pill{padding:6px 12px;border-radius:999px;font-size:12px;font-weight:700;border:1px solid var(--line);background:rgba(127,127,127,.1)}
.pill.ok{color:var(--grn);border-color:#34d39966}
.pill.bad{color:var(--red);border-color:#f43f5e66}
.pill.demo{color:#d97706;border-color:#fbbf2466}
.spacer{flex:1}

/* SEKMELER */
.nav{display:grid;grid-template-columns:repeat(6,1fr);gap:8px;margin-bottom:14px;position:sticky;top:env(safe-area-inset-top,0px);z-index:5}
.nav button{min-height:52px;font-size:14px;letter-spacing:1px;backdrop-filter:blur(8px);-webkit-backdrop-filter:blur(8px)}
.nav button.act{background:linear-gradient(90deg,#0891b2,#a21caf);color:#fff;border-color:transparent;box-shadow:0 6px 24px #a21caf55}
@media(max-width:640px){.nav{grid-template-columns:repeat(3,1fr)}.nav button{font-size:11px;min-height:44px;letter-spacing:0}}
.panel{display:none}
.panel.act{display:block;animation:fade .25s}
@keyframes fade{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:none}}

.grid{display:grid;grid-template-columns:1fr 1fr;gap:14px}
.g32{display:grid;grid-template-columns:1.6fr 1fr;gap:14px}
@media(max-width:900px){.grid,.g32{grid-template-columns:1fr}}
.card{background:var(--card);border:1px solid var(--line);border-radius:20px;padding:16px;
  backdrop-filter:blur(10px);-webkit-backdrop-filter:blur(10px);box-shadow:0 10px 40px rgba(0,0,0,.18)}
.card h2{margin:0 0 12px;font-size:14px;letter-spacing:3px;color:var(--cyan);display:flex;align-items:center;gap:8px}
.card h2::before{content:"";width:8px;height:8px;border-radius:50%;background:var(--cyan);box-shadow:0 0 10px var(--cyan)}
.sub{color:var(--sub);font-size:12px;line-height:1.45}
button{border:1px solid var(--line);border-radius:14px;color:var(--txt);font-weight:800;cursor:pointer;
  background:linear-gradient(180deg,var(--btn1),var(--btn2));touch-action:none;transition:transform .08s,box-shadow .15s}
button:active,button.on{transform:scale(.96);box-shadow:0 0 18px var(--cyan) inset}
button.tool.on{background:linear-gradient(90deg,#0891b2,#a21caf);color:#fff}
.row{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}
.row button{flex:1;min-height:44px;font-size:13px;padding:8px}
.tabs{display:flex;gap:6px;margin-bottom:10px;flex-wrap:wrap}
.tabs button{padding:8px 12px;font-size:12px;min-height:36px}
.tabs button.act{background:linear-gradient(90deg,#0891b2,#a21caf);color:#fff;border-color:transparent}
input[type=range]{width:100%;accent-color:var(--cyan)}
input[type=number]{width:84px;background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:10px;padding:6px}
canvas{width:100%;display:block;border-radius:14px;background:var(--canvas);touch-action:none}
.val{font-size:20px;font-weight:900;text-align:center;margin:6px}
.big{font-size:38px;font-weight:900;background:linear-gradient(90deg,var(--cyan),var(--mag));-webkit-background-clip:text;background-clip:text;color:transparent;line-height:1.1}
.stat{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:8px 0}
.stat div{background:rgba(127,127,127,.1);border-radius:12px;padding:8px}
.stat b{display:block;font-size:20px}
.bar{height:10px;border-radius:9px;background:rgba(127,127,127,.2);overflow:hidden;margin:8px 0}
.bar div{height:100%;width:0;background:linear-gradient(90deg,var(--cyan),var(--mag))}
.warn{color:var(--yel)}
.sl{display:grid;grid-template-columns:130px 1fr 60px;align-items:center;gap:8px;margin:6px 0;font-size:13px}
.pose{display:grid;grid-template-columns:52px 1fr auto auto;gap:6px;align-items:center;margin:6px 0;font-size:12px}
.pose button{padding:8px 10px;font-size:12px;min-height:36px}
.pose span{color:var(--sub)}

/* SURUS */
.pad{display:grid;grid-template-columns:repeat(3,1fr);grid-template-rows:repeat(3,86px);gap:10px;max-width:380px;margin:auto}
.pad button{font-size:26px}
.pad .stop{background:rgba(127,127,127,.35)}
.spin{display:grid;grid-template-columns:1fr 1fr;gap:10px;max-width:380px;margin:10px auto 0}
.spin button{height:64px;font-size:15px}
.keys{display:grid;grid-template-columns:repeat(4,52px);gap:6px;justify-content:center;margin:10px 0}
.key{height:52px;border-radius:10px;border:1px solid var(--line);display:flex;align-items:center;justify-content:center;font-weight:900;font-size:18px;background:rgba(127,127,127,.12);transition:all .08s}
.key.on{background:linear-gradient(180deg,#22d3ee,#0891b2);color:#fff;box-shadow:0 0 18px #22d3ee}
.key small{font-size:9px;font-weight:600;display:block}
.motors{display:grid;grid-template-columns:1fr 1fr;gap:16px;margin-top:6px}
.mbar{height:190px;border-radius:14px;background:rgba(127,127,127,.12);position:relative;overflow:hidden;border:1px solid var(--line)}
.mbar .fill{position:absolute;left:0;right:0;background:linear-gradient(180deg,#22d3ee,#a21caf);transition:all .08s}
.mbar .mid{position:absolute;left:0;right:0;top:50%;height:2px;background:var(--line)}
.mbar .lbl{position:absolute;left:0;right:0;bottom:8px;text-align:center;font-weight:900;font-size:15px}

/* LINEER */
#linCanvas{height:600px}
.linbtn{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:10px}
.linbtn button{height:78px;font-size:18px}
.up{background:linear-gradient(180deg,#34d399,#047857)!important;color:#fff!important}
.dn{background:linear-gradient(180deg,#fbbf24,#b45309)!important;color:#fff!important}
.fixbox{display:none;margin-top:10px;padding:10px;border-radius:12px;border:1px dashed var(--mag);background:rgba(232,121,249,.08)}
.fixbox.show{display:block}
.inl{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin-top:8px;font-size:13px}

/* KOL */
#armCanvas{height:520px}
#manualBox{display:none}
#manualBox.show{display:block}

/* CIZ-GIT */
#pathCanvas{height:78vh;min-height:520px;max-height:900px}

/* HAREKETLER */
.g3{display:grid;grid-template-columns:270px 1fr 340px;gap:14px}
@media(max-width:1100px){.g3{grid-template-columns:1fr 1fr}.g3>.card:last-child{grid-column:1/-1}}
@media(max-width:700px){.g3{grid-template-columns:1fr}}
.prog{min-height:260px;border:2px dashed var(--line);border-radius:14px;padding:8px}
.prog .empty{color:var(--sub);text-align:center;padding:40px 10px;font-size:13px}
.blk{display:flex;flex-wrap:wrap;align-items:center;gap:6px;padding:9px 10px;border-radius:12px;margin:6px 0;color:#fff;font-weight:800;font-size:13px;box-shadow:0 4px 12px rgba(0,0,0,.2);touch-action:none;cursor:grab;user-select:none}
.blk input,.blk select{width:64px;padding:4px 6px;border-radius:8px;border:none;background:rgba(255,255,255,.92);color:#0f172a;font-weight:800;touch-action:auto}
.blk .hd{font-size:16px;opacity:.8;padding-right:2px}
.blk .num{font-size:11px;opacity:.75;min-width:18px}
.blk .bt{margin-left:auto;display:flex;gap:4px}
.blk .bt button{padding:4px 8px;min-height:28px;font-size:12px;background:rgba(255,255,255,.22);color:#fff;border:none;border-radius:8px}
.blk .bx{margin-left:auto;padding:0 6px;opacity:.8;cursor:pointer}
.blk.run{outline:3px solid #fde047;box-shadow:0 0 22px #fde047}
.blk.done{opacity:.55}
.blk.ghost{position:fixed;pointer-events:none;z-index:50;opacity:.92;width:250px}
.ph{height:6px;border-radius:4px;background:var(--cyan);margin:4px 0;box-shadow:0 0 10px var(--cyan)}
.c-drive{background:linear-gradient(90deg,#2563eb,#1e40af)}
.c-turn{background:linear-gradient(90deg,#7c3aed,#5b21b6)}
.c-speed{background:linear-gradient(90deg,#0d9488,#115e59)}
.c-wait{background:linear-gradient(90deg,#64748b,#334155)}
.c-lin{background:linear-gradient(90deg,#059669,#065f46)}
.c-arm{background:linear-gradient(90deg,#c026d3,#86198f)}
.c-goto{background:linear-gradient(90deg,#ea580c,#9a3412)}
.pgrp{font-size:11px;letter-spacing:2px;color:var(--sub);margin:10px 0 2px;font-weight:800}
/* SERVO KALIBRASYON */
.calr{border:1px solid var(--line);border-radius:12px;padding:8px 10px;margin:8px 0;background:rgba(127,127,127,.08)}
.calh{display:flex;flex-wrap:wrap;gap:6px;align-items:center;font-size:13px}
.calh select,.calr select{background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:8px;padding:4px}
.zone{display:flex;gap:6px;align-items:center;font-size:13px;margin:4px 0}
.calg{display:grid;grid-template-columns:170px 1fr;gap:12px;align-items:center;margin-top:6px}
@media(max-width:520px){.calg{grid-template-columns:1fr}}
canvas.dial{width:170px;height:170px;border-radius:16px;margin:auto}
.dnum{font-size:28px;font-weight:900;color:var(--cyan)}
.dnum small{font-size:11px;color:var(--sub);font-weight:600}
.nud{display:flex;flex-wrap:wrap;gap:4px;margin:5px 0}
.nud button{padding:6px 9px;min-height:32px;font-size:12px}
input[type=file]{width:100%;margin-top:8px;color:var(--txt)}
/* SISTEM */
.hg{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.hi{background:rgba(127,127,127,.12);border-radius:10px;padding:8px;font-size:13px}
.okc{color:var(--grn);font-weight:800}.badc{color:var(--red);font-weight:800}
table.kb{border-collapse:collapse;width:100%;font-size:13px}
table.kb td{padding:5px 6px;border-bottom:1px solid var(--line)}
table.kb td:first-child{font-weight:900;width:110px}

.estop{position:fixed;left:50%;transform:translateX(-50%);bottom:calc(14px + env(safe-area-inset-bottom,0px));width:min(560px,92vw);height:62px;font-size:20px;letter-spacing:2px;
  background:linear-gradient(180deg,#e11d48,#881337)!important;color:#fff!important;border-color:#fb7185;box-shadow:0 0 30px #e11d4899;z-index:9}
.estop:active{transform:translateX(-50%) scale(.97)}
#toast{position:fixed;left:50%;transform:translateX(-50%);bottom:calc(90px + env(safe-area-inset-bottom,0px));background:rgba(15,23,42,.94);color:#e8eeff;padding:10px 16px;border-radius:12px;font-size:14px;opacity:0;transition:opacity .25s;pointer-events:none;z-index:10;max-width:90vw;text-align:center}
#toast.show{opacity:1}
</style>
</head>
<body>
<div class="wrap">

<header>
  <div class="logo">ROBOLIG<small>KONTROL MERKEZI</small></div>
  <div class="spacer"></div>
  <button id="hdrOta" style="padding:6px 12px;min-height:34px;font-size:12px">⬆ WEBDEN KOD YUKLE</button>
  <div id="demoPill" class="pill demo" style="display:none">DEMO MODU</div>
  <div id="connPill" class="pill bad">BAGLANTI YOK</div>
  <div id="healthPill" class="pill">SISTEM ?</div>
</header>

<nav class="nav" id="nav">
  <button data-p="drive" class="act">🚗 SURUS</button>
  <button data-p="moves">🧩 HAREKETLER</button>
  <button data-p="lin">⬆ LINEER</button>
  <button data-p="arm">🦾 ROBOT KOL</button>
  <button data-p="path">🗺 CIZ-GIT</button>
  <button data-p="sys">⚙ SISTEM</button>
</nav>

<!-- ================= SURUS ================= -->
<section class="panel act" id="p-drive">
  <div class="grid">
    <div class="card">
      <h2>SURUS KONTROLU</h2>
      <div class="pad">
        <button data-dr="wa">↖</button>
        <button data-dr="w">▲</button>
        <button data-dr="wd">↗</button>
        <button data-dr="a">◀</button>
        <button id="driveStop" class="stop">■</button>
        <button data-dr="d">▶</button>
        <button data-dr="sa">↙</button>
        <button data-dr="s">▼</button>
        <button data-dr="sd">↘</button>
      </div>
      <div class="spin">
        <button data-dr="q">⟲ KENDI EKSENINDE SOLA</button>
        <button data-dr="e">⟳ KENDI EKSENINDE SAGA</button>
      </div>
      <div class="sub" style="text-align:center;margin-top:10px">
        Klavye: <b>W</b> ileri, <b>S</b> geri, <b>A/D</b> sola/saga (W ya da S ile birlikte yay cizerek), <b>Q/E</b> kendi ekseninde don, <b>BOSLUK</b> dur
      </div>
      <div class="sub" style="text-align:center;margin-top:6px;color:var(--cyan);font-weight:700">
        🗺 Elle surerken CIZ-GIT haritasindaki robot da birlikte hareket eder.
      </div>
    </div>

    <div class="card">
      <h2>MOTOR DURUMU</h2>
      <div class="keys">
        <div></div><div class="key" id="k-w">W</div><div></div><div></div>
        <div class="key" id="k-a">A</div><div class="key" id="k-s">S</div><div class="key" id="k-d">D</div><div></div>
      </div>
      <div class="keys" style="grid-template-columns:repeat(2,70px)">
        <div class="key" id="k-q">Q<small>⟲ eksen</small></div><div class="key" id="k-e">E<small>⟳ eksen</small></div>
      </div>
      <div class="motors">
        <div><div class="sub" style="text-align:center">SOL MOTORLAR</div><div class="mbar"><div class="mid"></div><div class="fill" id="barL"></div><div class="lbl" id="lblL">0%</div></div></div>
        <div><div class="sub" style="text-align:center">SAG MOTORLAR</div><div class="mbar"><div class="mid"></div><div class="fill" id="barR"></div><div class="lbl" id="lblR">0%</div></div></div>
      </div>
      <div class="sl" style="margin-top:14px"><span>Azami hiz</span><input type="range" id="spMax" min="60" max="255" value="255"><b id="spMaxT">100%</b></div>
      <div class="sl"><span>Eksen donus hizi</span><input type="range" id="spTurn" min="60" max="255" value="200"><b id="spTurnT">78%</b></div>
      <div class="sl"><span>Yay keskinligi</span><input type="range" id="spArc" min="10" max="100" value="60"><b id="spArcT">60%</b></div>
      <div class="sub" id="tankInfo">-</div>
    </div>
  </div>
</section>

<!-- ================= HAREKETLER ================= -->
<section class="panel" id="p-moves">
  <div class="g3">
    <div class="card">
      <h2>KOD BLOKLARI</h2>
      <div class="sub">Bloga <b>dokunun</b>: programin sonuna eklenir. <b>Surukleyin</b>: istediginiz siraya birakin.</div>
      <div id="palette"></div>
      <div class="pgrp" style="margin-top:14px">KENDI BLOGUNU OLUSTUR</div>
      <div class="inl" style="margin-top:4px">
        <select id="nbType" style="background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:8px;padding:6px">
          <option value="fwd">⬆ Ileri</option><option value="back">⬇ Geri</option><option value="left">⟲ Sola don</option><option value="right">⟳ Saga don</option>
        </select>
        <input type="number" id="nbVal" step="any" value="1" style="width:70px">
        <select id="nbUnit" style="background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:8px;padding:6px"><option value="m">m</option><option value="cm">cm</option></select>
        <b id="nbDeg" style="display:none">°</b>
      </div>
      <div class="row"><button id="nbAdd">➕ BLOK OLUSTUR</button></div>
    </div>
    <div class="card">
      <h2>HAREKETLER</h2>
      <div id="prog" class="prog"><div class="empty" id="progEmpty">Bloklari buraya surukleyin ya da soldaki bloklara dokunun</div></div>
      <div class="sub" id="progInfo" style="margin-top:8px">-</div>
      <canvas id="miniMap" style="height:300px;margin-top:10px"></canvas>
      <div class="sub" style="margin-top:4px">Robotun sahadaki tahmini konumu. Baslangic konumu CIZ-GIT sekmesinden ayarlanir.</div>
      <div class="row"><button id="progRun" class="up">▶ SURUSU BASLAT</button><button id="progStop">■ DURDUR</button></div>
      <div class="row"><button id="progClear">TEMIZLE</button><button id="progFromPath">🗺 CIZ-GIT YOLUNU AKTAR</button></div>
      <h2 style="margin-top:14px">PROGRAMI KAYDET</h2>
      <div class="sub">Hazirladiginiz blok dizisine bir isim verip kaydedin (ornegin "A'dan Sinop'a"). Sonra listeden secip aninda geri yukleyebilirsiniz.</div>
      <div class="inl"><input type="text" id="progName" placeholder="ornek: A'dan Sinop'a" style="width:180px;background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:10px;padding:6px">
        <button id="progSave" style="min-height:36px;padding:6px 12px">💾 KAYDET</button></div>
      <div class="inl">Kayitli programlar: <select id="progSel"><option value="">- sec ve yukle -</option></select>
        <button id="progDel" style="min-height:36px;padding:6px 12px">SIL</button></div>
    </div>
    <div class="card">
      <h2>SENSOR (ARDUINO UNO)</h2>
      <div class="sub" id="sensState" style="font-weight:700">-</div>
      <div class="stat">
        <div><span class="sub">Onundeki mesafe</span><b id="sensCm">-</b></div>
        <div><span class="sub">Donus acisi</span><b id="sensDeg">-</b></div>
      </div>
      <div class="row"><button id="sensZero">ACIYI SIFIRLA</button><button id="sensCal">JIROSKOPU KALIBRE ET</button></div>
      <div class="sub" id="moveState" style="margin-top:8px">-</div>

      <h2 style="margin-top:16px">HAREKET YONTEMI</h2>
      <div class="inl"><label><input type="checkbox" id="useSensor" checked> Sensor varsa SENSORLE olc (yoksa sureye gore)</label></div>
      <div class="sub">
        <b>Ileri / geri:</b> robotun onunde duvar ya da levha varsa mesafe sensoruyle olculur.<br>
        <b>Donus:</b> jiroskopla olculur, her yerde calisir.<br>
        <b>Duz gitme:</b> jiroskop varsa robot kendini duzelterek dumduz gider.
      </div>

      <h2 style="margin-top:16px">OTOMATIK KALIBRASYON</h2>
      <div class="sub">Robot kendi kendine test yapip gercek hizini olcer. Sensorun 1 metreye kadar duzgun olctugu icin testler kisa mesafelerle yapilir.</div>
      <div class="stat">
        <div><span class="sub">Duz hiz (%100)</span><b id="calCmT">40 cm/sn</b></div>
        <div><span class="sub">Donus hizi (%100)</span><b id="calDegT">120 °/sn</b></div>
      </div>
      <div class="row"><button id="calAuto" class="up">📏 DUZ GITME TESTI (10 / 20 / 30 cm)</button></div>
      <div class="row"><button id="calTurnAuto" class="up">🔄 DONUS TESTI (90° / 180° / 360°)</button></div>
      <div class="row"><button id="calStop">■ TESTI DURDUR</button></div>
      <div class="sub" id="calAutoInfo" style="margin-top:6px">-</div>
      <div id="calTable" class="sub"></div>

      <h2 style="margin-top:16px">NEYI YANLIS YAPTI?</h2>
      <div class="sub">Robot komutu tam uygulamadiysa burada soyleyin, ayar kendiliginden duzelsin.</div>
      <select id="fixKind" style="width:100%;margin-top:6px;background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:10px;padding:8px">
        <option value="fwd">Ileri/geri gitti ama mesafe yanlis</option>
        <option value="turn">Dondu ama aci yanlis</option>
        <option value="drift">Duz giderken saga/sola kaydi</option>
        <option value="line">Sari cizgiyi gecti / duvara surttu</option>
        <option value="nomove">Hic hareket etmedi</option>
      </select>
      <div id="fixNum2" class="inl">Komut: <input type="number" id="fixCmd" step="1" value="100"> &nbsp; Gercekte: <input type="number" id="fixReal" step="1"> &nbsp;<span id="fixUnit">cm</span>
        <button id="fixApply" class="up" style="min-height:36px;padding:6px 12px">DUZELT</button></div>
      <div class="sub" id="fixHelp" style="margin-top:6px">-</div>

      <h2 style="margin-top:16px">ELLE AYAR</h2>
      <div class="inl">Duz hiz <input type="number" id="calCm" step="0.5" value="40"> cm/sn</div>
      <div class="inl">Donus hizi <input type="number" id="calDeg" step="1" value="120"> °/sn</div>
      <div class="sub">Bu degerler sensorun olcemedigi durumlarda (onde hedef yoksa) kullanilir.</div>
    </div>
  </div>
</section>

<!-- ================= LINEER ================= -->
<section class="panel" id="p-lin">
  <div class="g32">
    <div class="card">
      <h2>LINEER AKTUATOR 3D + ZIPLINE</h2>
      <div class="tabs">
        <button id="xrayBtn" class="act">ICINI GOSTER</button>
        <button id="linCamFront">ONDEN</button>
        <button id="linCamSide">YANDAN</button>
        <button id="linCamAuto" class="act">OTOMATIK DON</button>
      </div>
      <canvas id="linCanvas"></canvas>
      <div class="sub" style="margin-top:6px">Surukleyerek modeli dondurun. Klavye: <b>R</b> yukari, <b>F</b> asagi (basili tutun).
        Kulenin icinde: altta NEMA23, kaplin, vidali mil, somun ve somunun ittigi saft. Saftin ucunda kanca var.</div>
    </div>
    <div class="card">
      <h2>DURUM</h2>
      <div class="sub">Kanca yuksekligi (yerden)</div>
      <div class="big" id="linH">0.0 cm</div>
      <div class="stat">
        <div><span class="sub">Cikan</span><b id="linCm">0 cm</b></div>
        <div><span class="sub">Kalan</span><b id="linLeft">0 cm</b></div>
        <div><span class="sub">Zipline'a (90 cm)</span><b id="linZip">-</b></div>
        <div><span class="sub">Adim</span><b id="linPos">0</b></div>
        <div><span class="sub">Doluluk</span><b id="linPct">%0</b></div>
        <div><span class="sub">Anlik hiz</span><b id="linSpd">0</b></div>
      </div>
      <div class="bar"><div id="linBar"></div></div>
      <div class="linbtn">
        <button id="linearUp" class="up">▲ YUKARI</button>
        <button id="linearDown" class="dn">▼ ASAGI</button>
      </div>
      <div class="row"><button id="linearStop">■ DUR</button><button id="zeroBtn">BURASI 0</button></div>

      <div class="inl">Hedef yukseklik: <input type="number" id="gotoCm" step="0.5" value="90"> cm <button id="gotoBtn" style="flex:1;min-height:40px">➜ GIT</button></div>
      <div class="row"><button id="gotoZip">🎯 ZIPLINE'A CIK (90 cm)</button><button id="gotoHome">⬇ EN ALTA</button></div>

      <div style="margin-top:10px">
        <div class="sub">Hedef hiz: <b id="linTgt">2000</b> pulse/s</div>
        <input type="range" id="linearSpeed" min="300" max="10000" step="100" value="2000">
      </div>

      <h2 style="margin-top:14px">LINEER KALIBRASYONU</h2>
      <div class="sub" id="homeWarn" style="font-weight:700"></div>
      <div class="sub" style="margin-top:6px"><b>1)</b> ASAGI butonuyla lineeri <b>en alta</b> indirin ve <b>EN ALT = 0</b>'a basin.</div>
      <div class="row"><button id="calLinZero">⬇ EN ALT = 0</button></div>
      <div class="sub" style="margin-top:6px"><b>2)</b> YUKARI butonuyla <b>en uste</b> cikarin ve <b>EN UST</b>'e basin. O anki adim sayisi tam strok olarak kaydedilir.</div>
      <div class="row"><button id="calLinTop">⬆ EN UST = SU ANKI ADIM</button></div>
      <div class="inl">Tam strok: <input type="number" id="linMaxIn" step="100" value="16000"> adim <button id="linMaxSet" style="min-height:36px;padding:6px 12px">KAYDET</button></div>
      <label class="inl"><input type="checkbox" id="linLimit" checked> Yazilim siniri: lineer 0 ile en ust arasinda kendiliginden dursun</label>
      <div class="sub" style="margin-top:6px"><b>3)</b> Metreyle olcun: kanca en altta ve en ustte yerden kac cm?</div>
      <div class="inl">En altta (0 adim): <input type="number" id="h0Cm" step="0.5" value="62"> cm</div>
      <div class="inl">En ustte (<span id="linMax">16000</span> adim): <input type="number" id="h1Cm" step="0.5" value="90"> cm</div>
      <div class="sub" id="strokeInfo">Strok: 28 cm</div>

      <div class="row"><button id="fixBtn" class="tool">ADIM KACTI - DUZELT</button></div>
      <div id="fixBox" class="fixbox">
        <div class="sub">Pembe kancayi parmakla gercek yuksekligine surukleyin ya da degeri yazin.</div>
        <div class="inl">Yeni konum: <input type="number" id="fixNum" min="0" max="16000" value="0"> adim (kanca <span id="fixCm">0</span> cm)</div>
        <div class="row"><button id="fixSave">KAYDET</button><button id="fixCancel">VAZGEC</button></div>
      </div>
    </div>
  </div>
</section>

<!-- ================= ROBOT KOL ================= -->
<section class="panel" id="p-arm">
  <div class="g32">
    <div class="card">
      <h2>ROBOT KOL</h2>
      <div class="tabs" id="viewTabs">
        <button data-v="3d" class="act">3D</button>
        <button data-v="side">YANDAN</button>
        <button data-v="top">USTTEN</button>
      </div>
      <canvas id="armCanvas"></canvas>
      <div id="armInfo" class="sub" style="margin-top:6px">-</div>
    </div>
    <div class="card">
      <h2>KONTROL</h2>
      <div class="tabs" id="modeTabs">
        <button data-m="ik" class="act">SURUKLE</button>
        <button data-m="manual">MANUEL</button>
      </div>
      <div class="sub" id="modeHelp">Kolun ucundaki yesil noktayi surukleyin. 3D'de uca dokunmazsaniz kamera doner.</div>
      <div class="calr" style="margin-top:10px">
        <div class="calh"><b>KANAL ESLESTIRME</b> <span class="sub">Hangi eklem hangi kanalda takili?</span></div>
        <div class="calh" id="jointQuick" style="margin-top:6px"></div>
      </div>

      <div class="sl" style="margin-top:10px"><span>GRIPPER</span><input type="range" id="gripSl" min="0" max="100" value="50"><b id="gripTxt">%50</b></div>
      <div class="row"><button id="gripOpen">GRIPPER AC</button><button id="gripClose">GRIPPER KAPAT</button></div>
      <div class="sub" id="gripCalInfo" style="margin-top:4px">-</div>

      <div id="manualBox">
        <div class="sl"><span id="n4">CH4</span><input type="range" id="m4" min="0" max="180" value="90"><b id="t4">90°</b></div>
        <div class="sl"><span id="n5">CH5</span><input type="range" id="m5" min="0" max="180" value="90"><b id="t5">90°</b></div>
        <div class="sl"><span id="n6">CH6</span><input type="range" id="m6" min="0" max="180" value="90"><b id="t6">90°</b></div>
        <div class="sl"><span id="n7">CH7</span><input type="range" id="m7" min="0" max="180" value="90"><b id="t7">90°</b></div>
      </div>

      <h2 style="margin-top:16px">KAYITLI POZLAR</h2>
      <div class="sub">Kolu istediginiz aciya getirip KAYDET'e basin. GIT butonuyla ya da HAREKETLER sekmesindeki POZ blogu ile kol bu acilara gider.</div>
      <div class="pose"><b>AL</b><span id="pz-al">-</span><button data-ps="al">KAYDET</button><button data-pg="al">GIT</button></div>
      <div class="pose"><b>TASI</b><span id="pz-tasi">-</span><button data-ps="tasi">KAYDET</button><button data-pg="tasi">GIT</button></div>
      <div class="pose"><b>BIRAK</b><span id="pz-birak">-</span><button data-ps="birak">KAYDET</button><button data-pg="birak">GIT</button></div>

      <h2 style="margin-top:16px">SERVO KALIBRASYONU</h2>
      <div class="row"><button id="calModeBtn" class="tool">🔧 KALIBRASYON MODU</button></div>
      <div id="calBox" class="fixbox">
        <div class="sub">
          <b>1)</b> Servonun tipini secin: <b>180°</b> ya da <b>270°</b>.<br>
          <b>2)</b> HAM ACI kaydiricisiyla servoyu fiziksel olarak bildiginiz bir konuma getirin (ornegin kol tam dik).<br>
          <b>3)</b> O konumun kac derece olmasini istediginizi yazip <b>KALIBRE ET</b>'e basin. Bundan sonra butun hareketler bu konuma gore hesaplanir.<br>
          Servo ters donuyorsa yonu <b>ters</b> yapin. Sinirlar servonun zorlanmamasi icin kullanilir.
        </div>
        <div id="calRows"></div>
        <div class="row"><button id="calDone">✓ KALIBRASYONU BITIR</button><button id="calReset">VARSAYILANA DON</button></div>
      </div>

      <h2 style="margin-top:16px">EKRANDAKI KOLU GERCEK KOLA ESLE</h2>
      <div class="row"><button id="matchBtn" class="tool">🪞 EKRANI KOLA ESLE</button></div>
      <div id="matchBox" class="fixbox">
        <div class="sub"><b>Bu modda robota HICBIR ACI GONDERILMEZ.</b> Kol oldugu yerde durur, sadece ekrandaki kolu robottaki kolun su anki durusuna benzetirsiniz.</div>
        <div class="sub" style="margin-top:6px"><b>1)</b> Robottaki kol dimdik duruyorsa <b>⬆ EKRANDAKI KOLU DIMDIK YAP</b>'a basin. Degilse asagidaki kaydiricilarla ekrandaki kolu gercek kola benzetin.<br><b>2)</b> <b>✓ ESLEMEYI KAYDET</b>'e basin. Bundan sonra ekrandaki kol gercek kolla ayni hareket eder.<br><b>3)</b> Bir eklem ekranda ters yone gidiyorsa o eklemin <b>ters</b> kutusunu isaretleyip tekrar kaydedin.</div>
        <div class="row"><button id="matchUp" class="up">⬆ EKRANDAKI KOLU DIMDIK YAP</button></div>
        <div class="sl"><span>Taban yonu</span><input type="range" id="mjYaw" min="-90" max="90" step="1" value="0"><b id="mjYawT">0°</b></div>
        <div class="sl"><span>Omuz (yere gore)</span><input type="range" id="mjT1" min="-10" max="190" step="1" value="90"><b id="mjT1T">90°</b></div>
        <div class="sl"><span>Dirsek (kirilma)</span><input type="range" id="mjT2" min="-180" max="180" step="1" value="0"><b id="mjT2T">0°</b></div>
        <div class="inl">Ters yon: <label><input type="checkbox" id="mjDirB"> taban</label> <label><input type="checkbox" id="mjDirS"> omuz</label> <label><input type="checkbox" id="mjDirE"> dirsek</label></div>
        <div class="row"><button id="matchSave" class="up">✓ ESLEMEYI KAYDET</button><button id="matchCancel">VAZGEC</button></div>
        <div class="row"><button id="matchReset">ESLEMEYI SIFIRLA</button></div>
      </div>
      <div class="sub" style="margin-top:8px"><b>Acilis pozu:</b> Robot her acildiginda servolari bu konuma getirir. Kolu dimdik yapip asagidaki butona basarsaniz robot her acilista dimdik baslar.</div>
      <div class="row"><button id="bootPose">⏻ SU ANKI POZ = ACILIS POZU</button></div>

      <div class="row"><button id="armRuler" class="tool">📏 CETVEL (YANDAN / USTTEN)</button></div>
      <div class="sub" id="armRulerInfo"></div>

      <div class="row">
        <button id="armCenter">TUMU 90°</button>
        <button id="armOff">SERVOLARI KAPAT</button>
      </div>
      <div class="sub warn" style="margin-top:8px">Soluk kol = hedef, parlak kol = servonun gercek konumu.</div>
    </div>
  </div>
</section>

<!-- ================= CIZ-GIT ================= -->
<section class="panel" id="p-path">
  <div class="g32">
    <div class="card">
      <h2>TEKNOFEST UNIVERSITE SAHASI (550 x 350 cm)</h2>
      <div class="tabs">
        <button id="zIn">🔍 +</button>
        <button id="zOut">🔍 −</button>
        <button id="zFit">⟲ SIGDIR</button>
        <button class="tool" data-t="pan">✋ KAYDIR</button>
        <span class="pill" id="zoomTxt">%100</span>
      </div>
      <canvas id="pathCanvas"></canvas>
      <div class="sub" style="margin-top:6px">Buyutec: fare tekerlegi, iki parmakla sikistirma ya da + / − butonlari. Buyuttukce cizim ve cetvel daha hassas olur, olculer hep gercek cm'dir.</div>
    </div>
    <div class="card">
      <h2>KOLI ALMA</h2>
      <div class="sub">Sirayla basin. Her adimda robot durur ve sizi bekler.</div>
      <div class="row"><button id="goPickup" class="up">📦 1) KUTULARA GIT</button></div>
      <div class="row"><button id="pickupDone">✋ 2) ALDIM — GERI CEKIL</button></div>
      <div class="row"><button id="pickupScan" class="up">📡 3) ETIKETI OKUT</button><button id="pickupTurn">🔄 DIGER YUZ (90°)</button></div>
      <div class="row"><button id="pickupStop">■ DURDUR</button></div>
      <div class="inl">Durma mesafesi <input type="number" id="pickStop" step="5" value="25"> cm &nbsp; Geri cekilme <input type="number" id="pickBack" step="5" value="40"> cm</div>
      <div class="sub" id="pickupInfo" style="margin-top:6px">-</div>
      <div class="sub" id="rfidLive" style="margin-top:4px">-</div>

      <h2 style="margin-top:16px">TESLIMAT</h2>
      <div class="sub">Robot koliyi aldiktan sonra haritada sehre dokunun: robot sari yollardan gider ve sehre <b>30 cm kala durur</b>, koliyi kolla siz koyarsiniz.</div>
      <div class="row" id="cityBtns"></div>
      <div class="sub" id="deliverInfo" style="margin-top:6px">-</div>

      <h2 style="margin-top:16px">BASLANGIC</h2>
      <div class="row"><button id="startA" class="tool on">A ALANI</button><button id="startB" class="tool">B ALANI</button></div>
      <div class="row"><button class="tool" data-t="robot">🤖 ROBOTU TASI</button></div>
      <div class="row" id="robotDoneRow" style="display:none"><button id="robotDone" class="up">✓ TASIMAYI BITIR</button></div>
      <div class="inl">x <input type="number" id="poseX" step="1"> y <input type="number" id="poseY" step="1"> cm, yon <input type="number" id="poseH" step="1">°</div>
      <div class="sub">ROBOTU TASI acikken robotu surukleyin; yonu icin robotu surukledikten sonra yon kutusuna deger yazin.</div>
      <label class="inl"><input type="checkbox" id="poseFollow" checked> Yol bitince robotun yeni konumunu kullan</label>

      <h2 style="margin-top:16px">YOL</h2>
      <div class="row">
        <button class="tool on" data-t="draw">✏ CIZ</button>
        <button class="tool" data-t="pts">📍 NOKTA</button>
        <button class="tool" data-t="ruler">📏 CETVEL</button>
        <button class="tool" data-t="pan">✋ KAYDIR</button>
      </div>
      <div class="sub" id="toolHelp" style="margin-top:6px">-</div>
      <div class="row">
        <button id="pathGo" class="up">▶ YOLU SUR</button>
        <button id="pathStop">■ DURDUR</button>
      </div>
      <div class="row"><button id="pathFix">🛠 DUZELT</button><button id="pathUndo">↶ GERI AL</button><button id="pathClear">TEMIZLE</button></div>
      <div class="sub" id="pathInfo" style="margin-top:8px">-</div>
      <div class="sub" id="rulerInfo" style="margin-top:6px;font-size:14px"></div>

      <h2 style="margin-top:16px">SARI SINIR</h2>
      <div class="row"><button id="limitBtn" class="tool on">🟨 SARI SINIR: ACIK</button></div>
      <div class="sub" id="limitInfo" style="margin-top:6px;color:var(--yel);font-weight:700">⛔ Sari cizgilerin disina cikmak YASAK.</div>
      <div class="inl">Kontrol: <select id="chkMode" style="background:rgba(127,127,127,.15);border:1px solid var(--line);color:var(--txt);border-radius:8px;padding:5px">
        <option value="body">Robot govdesi sari cizgiye degmesin</option><option value="line">Sadece cizdigim cizgi</option></select></div>
      <div class="inl">Guvenlik payi: <input type="number" id="chkTol" step="1" value="0"> cm</div>

      <h2 style="margin-top:16px">HIZ</h2>
      <div class="inl">Normal hiz: <input type="number" id="normSpd" min="10" max="100" step="5" value="100"> %</div>
      <label class="inl"><input type="checkbox" id="kasisSlow" checked> Kasislerde hizi <input type="number" id="kasisSpd" min="10" max="100" step="5" value="85" style="width:60px"> %'e dusur</label>
      <div class="row"><button class="tool" data-t="slow">🐢 YAVAS ALAN EKLE</button></div>
      <div class="inl">Yeni alanin hizi: <input type="number" id="slowSpd" min="10" max="100" step="5" value="40"> %</div>
      <div id="zoneList"></div>

      <h2 style="margin-top:16px">ROBOT</h2>
      <div class="inl">Robot boyu <input type="number" id="robW" step="1" value="45"> cm</div>
      <div class="sub" id="calSummary">-</div>
      <div class="row"><button id="pathToBlocks">🧩 YOLU BLOKLARA AKTAR</button></div>
    </div>
  </div>
</section>

<!-- ================= SISTEM ================= -->
<section class="panel" id="p-sys">
  <div class="grid">
    <div class="card">
      <h2>SISTEM DURUMU</h2>
      <div class="hg">
        <div class="hi">OLED: <span id="hOled">-</span></div>
        <div class="hi">PCA9685: <span id="hPca">-</span></div>
        <div class="hi">RFID okuyucu: <span id="hRfid">-</span></div>
        <div class="hi">Wi-Fi: <span id="hWifi">-</span></div>
        <div class="hi">Son okunan kart: <b id="rfidUid">-</b></div>
        <div class="hi" style="grid-column:1/-1">RFID testi: <span id="rfidTest">-</span></div>
        <div class="hi">Kumanda: <span id="hRemote">-</span></div>
      </div>
      <div class="row"><button id="oledTest">OLED TEST</button></div>
    </div>
    <div class="card">
      <h2>KLAVYE KISAYOLLARI</h2>
      <table class="kb">
        <tr><td>W / S</td><td>Ileri / geri</td></tr>
        <tr><td>A / D</td><td>Sola / saga (W veya S ile yay cizerek)</td></tr>
        <tr><td>Q / E</td><td>Kendi ekseninde sola / saga</td></tr>
        <tr><td>BOSLUK</td><td>Surusu durdur</td></tr>
        <tr><td>R / F</td><td>Lineer yukari / asagi</td></tr>
        <tr><td>ESC</td><td>ACIL DURDUR</td></tr>
        <tr><td>1 - 6</td><td>Sekmeler arasi gecis</td></tr>
      </table>
    </div>
    <div class="card">
      <h2 id="otaCard">WEB UZERINDEN KOD YUKLE (OTA)</h2>
      <div class="sub warn">Web'den kod yuklediginizde robotun hafizasindaki ayarlar (kalibrasyonlar, programlar, eslemeler) SILINMEZ.</div>
      <div class="sub">Robottaki yazilim: <b id="fwVer">-</b></div>
      <ol class="sub" style="padding-left:18px">
        <li>Arduino IDE'de <b>Taslak → Derlenmis Binary'yi Disa Aktar</b> (Sketch → Export Compiled Binary) secin.</li>
        <li>Sketch klasorundeki <b>build</b> klasorunden adi <b>.ino.bin</b> ile biten dosyayi secin. Adinda <b>bootloader</b>, <b>partitions</b> ya da <b>merged</b> gecen dosyalari SECMEYIN.</li>
        <li><b>KODU YUKLE</b>'ye basin. Bitene kadar robotun gucunu kesmeyin ve sayfayi kapatmayin. Yukleme bitince robot kendiliginden yeniden baslar.</li>
      </ol>
      <input type="file" id="otaFile" accept=".bin">
      <div class="row"><button id="otaBtn" class="up">⬆ KODU YUKLE</button></div>
      <div class="bar"><div id="otaBar"></div></div>
      <div class="sub" id="otaTxt"></div>
    </div>
    <div class="card">
      <h2>AYARLARI KAYDET / YEDEKLE</h2>
      <div class="sub">Kalibrasyonlar, programlar, bloklar, yavas alanlar, kol eslemesi ve robot konumu her degisiklikte <b>otomatik</b> olarak bu cihaza ve robotun hafizasina kaydedilir. Yine de yarismadan once bir yedek alin.</div>
      <div class="sub" id="saveState" style="margin-top:6px;font-weight:700">-</div>
      <div class="row"><button id="saveNow" class="up">💾 SIMDI ROBOTA KAYDET</button></div>
      <div class="row"><button id="bkDown">⬇ YEDEK DOSYASI INDIR</button><button id="bkShow">📋 YEDEGI GOSTER / KOPYALA</button></div>
      <textarea id="bkText" style="display:none;width:100%;height:120px;margin-top:8px;background:rgba(127,127,127,.12);color:var(--txt);border:1px solid var(--line);border-radius:10px;padding:8px;font-size:11px" placeholder="Yedek metnini buraya yapistirin"></textarea>
      <div class="row"><button id="bkLoad">⬆ YAPISTIRILAN YEDEGI YUKLE</button></div>
      <div class="inl">ya da yedek dosyasi secin: <input type="file" id="bkFile" accept=".json,application/json" style="width:auto;margin:0"></div>
    </div>
    <div class="card">
      <h2>SON YAPILANLAR</h2>
      <div id="logList"></div>
    </div>
  </div>
</section>

</div>

<button id="emergency" class="estop">ACIL DURDUR</button>
<div id="toast"></div>

<script>
"use strict";
const $ = id => document.getElementById(id);
const RAD = Math.PI / 180;
function clamp(v,a,b){ return v<a?a:(v>b?b:v); }
function sleep(ms){ return new Promise(r=>setTimeout(r,ms)); }
function toast(t){ const e = $('toast'); e.textContent = t; e.classList.add('show'); clearTimeout(toast._t); toast._t = setTimeout(()=>e.classList.remove('show'), 2400); }
// Ayarlar: bu cihazda (localStorage) + robotun hafizasinda (/cfg) saklanir
const IS_ROBOT = location.hostname === '192.168.4.1';
let CFG = {};
try{ CFG = JSON.parse(localStorage.getItem('rl_cfg') || '{}') || {}; }catch(_){ CFG = {}; }
try{ for (let i = 0; i < localStorage.length; i++){ const k = localStorage.key(i); if (k && k.indexOf('rl_') === 0 && k !== 'rl_cfg' && !(k in CFG)) CFG[k] = localStorage.getItem(k); } }catch(_){}
if (IS_ROBOT){
  try{
    const x = new XMLHttpRequest(); x.open('GET', '/cfg', false); x.send();
    if (x.status === 200){ const rc = JSON.parse(x.responseText || '{}'); if (rc && typeof rc === 'object' && Object.keys(rc).length) CFG = Object.assign(CFG, rc); }
  }catch(_){}
}
let cfgTimer = null, robotCfgOK = true;
function lsGet(k, d){ return (k in CFG) ? CFG[k] : d; }
function lsSet(k, v){
  CFG[k] = String(v);
  try{ localStorage.setItem('rl_cfg', JSON.stringify(CFG)); }catch(_){}
  if (!IS_ROBOT) return;
  clearTimeout(cfgTimer);
  cfgTimer = setTimeout(() => {
    fetch('/cfg', { method: 'POST', headers: { 'Content-Type': 'text/plain' }, body: JSON.stringify(CFG) })
      .then(r => {
        const st = document.getElementById('saveState');
        if (r.status === 200){ if (st) st.textContent = '✓ Otomatik kaydedildi: ' + new Date().toLocaleTimeString('tr-TR'); }
        else if (robotCfgOK){ robotCfgOK = false; toast('Ayarlar robota kaydedilemedi (robot kodu eski olabilir), sadece bu cihazda saklandi'); }
      })
      .catch(()=>{});
  }, 1200);
}

// =====================================================
// ROBOT KOL AYARLARI (gerekirse burayi degistirin)
// =====================================================
const ARM = { H0: 8, L1: 20, L2: 20, G: 5 };   // cm: omuz yuksekligi, kol1, kol2, gripper
const JOINT = {
  base:     { ch: 4, j90: 0,   dir: 1, name: 'TABAN'   },  // MG995
  shoulder: { ch: 7, j90: 90,  dir: 1, name: 'OMUZ'    },  // QY3240MG
  elbow:    { ch: 6, j90: 0,   dir: 1, name: 'DIRSEK'  },  // MG996R  (servo 90 = on kol dimdik)
  grip:     { ch: 5, open: 40, close: 130, name: 'GRIPPER' } // MG945
};
const BOX = 9; // koli kenari (cm)
try{ const jm = JSON.parse(lsGet('rl_joint', 'null')); if (jm) for (const k of ['base','shoulder','elbow','grip']) if (jm[k]) JOINT[k].ch = +jm[k]; }catch(_){}
try{ const jg = JSON.parse(lsGet('rl_jgeo', 'null')); if (jg) for (const k of ['base','shoulder','elbow']) if (jg[k] && isFinite(jg[k].j90)){ JOINT[k].j90 = +jg[k].j90; JOINT[k].dir = jg[k].dir === -1 ? -1 : 1; } }catch(_){}
try{ const gp = JSON.parse(lsGet('rl_grip', 'null')); if (gp && isFinite(gp.open) && isFinite(gp.close)){ JOINT.grip.open = gp.open; JOINT.grip.close = gp.close; } }catch(_){}

// =====================================================
// DEMO MODU (robot yokken her sey simule edilir)
// =====================================================
const DEMO = location.hostname !== '192.168.4.1';
const SIM = { pos: 4200, dir: 0, cur: 500, tgtHz: 2000, spd: 255, gotoT: -1, L: 0, R: 0, max: 16000, homed: false, lim: true, cm: 120, deg: 0, mv: null, pkt: 0, uid: 'HENUZ YOK',
  us: {4:1500,5:1500,6:1500,7:1500}, tus: {4:1500,5:1500,6:1500,7:1500} };
if (DEMO){
  let last = performance.now();
  setInterval(() => {
    const now = performance.now(), dt = (now - last) / 1000; last = now;
    if (SIM.dir !== 0){
      let vmax = SIM.tgtHz;
      if (SIM.gotoT >= 0) vmax = Math.min(vmax, Math.sqrt(2*1200*Math.abs(SIM.gotoT - SIM.pos)) + 250);
      SIM.cur = SIM.cur < vmax ? Math.min(vmax, SIM.cur + 1200*dt) : Math.max(vmax, SIM.cur - 1200*dt);
      SIM.pos += SIM.dir * SIM.cur * dt;
      if (SIM.lim && SIM.homed && ((SIM.dir > 0 && SIM.pos >= SIM.max) || (SIM.dir < 0 && SIM.pos <= 0))){ SIM.pos = clamp(SIM.pos, 0, SIM.max); SIM.dir = 0; SIM.gotoT = -1; }
      if (SIM.gotoT >= 0 && ((SIM.dir > 0 && SIM.pos >= SIM.gotoT) || (SIM.dir < 0 && SIM.pos <= SIM.gotoT))){
        SIM.pos = SIM.gotoT; SIM.dir = 0; SIM.gotoT = -1;
      }
    } else SIM.cur = Math.min(500, SIM.tgtHz);
    for (const ch of [4,5,6,7]){
      const d = SIM.tus[ch] - SIM.us[ch], st = 650*dt;
      SIM.us[ch] += Math.abs(d) <= st ? d : Math.sign(d)*st;
    }
    SIM.pkt++;
    if (!SIM.mv && (SIM.L || SIM.R)){            // elle / sureye gore surus de sensoru etkilesin
      const f = (SIM.L + SIM.R) / 2 / 255, t = (SIM.R - SIM.L) / 2 / 255;
      SIM.cm = Math.max(5, SIM.cm - f * 40 * dt);
      SIM.deg += t * 120 * dt;
    }
    if (SIM.mv){
      const m = SIM.mv, k = m.sp / 100;
      m.done += (m.k <= 2 ? 40 : 120) * k * dt;          // demo hiz
      if (m.k === 1) SIM.cm = Math.max(5, SIM.cm - 40*k*dt);
      else if (m.k === 2) SIM.cm += 40*k*dt;
      else SIM.deg += (m.k === 3 ? 1 : -1) * 120*k*dt;
      if (m.done >= m.tgt) SIM.mv = null;
    }
  }, 20);
}
function simReply(p){
  const u = new URL(p, 'http://x'), q = u.searchParams, path = u.pathname;
  let txt = 'OK', obj = null;
  if (path === '/drive'){ const c = q.get('cmd'), s = SIM.spd; SIM.L = c==='forward'?s:c==='back'?-s:c==='left'?-s:c==='right'?s:0; SIM.R = c==='forward'?s:c==='back'?-s:c==='left'?s:c==='right'?-s:0; }
  else if (path === '/tank'){ SIM.L = +q.get('l'); SIM.R = +q.get('r'); }
  else if (path === '/speed') SIM.spd = +q.get('v');
  else if (path === '/linear'){ const c = q.get('cmd'); SIM.gotoT = -1; SIM.dir = c === 'up' ? 1 : (c === 'down' ? -1 : 0);
    if (SIM.lim && SIM.homed && ((SIM.dir > 0 && SIM.pos >= SIM.max) || (SIM.dir < 0 && SIM.pos <= 0))) SIM.dir = 0; }
  else if (path === '/lineargoto'){ const v = clamp(+q.get('v'), 0, SIM.max); SIM.gotoT = v; SIM.dir = v > SIM.pos ? 1 : (v < SIM.pos ? -1 : 0); if (!SIM.dir) SIM.gotoT = -1; }
  else if (path === '/zero'){ if (SIM.dir) txt = 'Lineer hareket ediyor. Once durdurun.'; else { SIM.pos = 0; SIM.homed = true; txt = 'Mevcut konum 0 olarak ayarlandi.'; } }
  else if (path === '/linmax'){ if (q.get('here')){ if (SIM.pos < 100) txt = 'HATA: once lineeri en uste cikarin'; else { SIM.max = Math.round(SIM.pos); txt = 'En ust = ' + SIM.max + ' adim'; } } else { SIM.max = clamp(+q.get('v'), 100, 200000); txt = 'Tam strok = ' + SIM.max + ' adim'; } }
  else if (path === '/linlimit'){ SIM.lim = q.get('on') === '1'; }
  else if (path === '/startup'){ txt = 'Acilis pozu kaydedildi (demo)'; }
  else if (path === '/move'){ SIM.mv = { k: +q.get('t'), tgt: +q.get('v'), done: 0, sp: +(q.get('sp') || 100) }; txt = 'SENSOR'; }
  else if (path === '/movestop'){ SIM.mv = null; txt = 'DURDU'; }
  else if (path === '/sensor'){ if (q.get('cmd') === 'zero') SIM.deg = 0; txt = 'Demo: komut alindi'; }
  else if (path === '/setpos'){ if (SIM.dir) txt = 'Lineer hareket ediyor. Once durdurun.'; else { SIM.pos = clamp(+q.get('v'), 0, SIM.max); SIM.homed = true; txt = 'Konum ' + Math.round(SIM.pos) + ' adim olarak ayarlandi.'; } }
  else if (path === '/linearspeed') SIM.tgtHz = clamp(+q.get('v'), 300, 10000);
  else if (path === '/pulse') SIM.tus[+q.get('ch')] = clamp(+q.get('us'), 500, 2500);
  else if (path === '/stopall'){ SIM.dir = 0; SIM.gotoT = -1; SIM.L = SIM.R = 0; for (const ch of [4,5,6,7]) SIM.tus[ch] = SIM.us[ch]; }
  else if (path === '/status'){
    obj = { position: Math.round(SIM.pos), linearMax: SIM.max, homed: SIM.homed, limits: SIM.lim, linearDir: SIM.dir, linearGoto: SIM.gotoT, driveSpeed: SIM.spd,
      linearSpeed: Math.round(SIM.cur), linearTarget: SIM.tgtHz,
      s4: Math.round(SIM.us[4]), s5: Math.round(SIM.us[5]), s6: Math.round(SIM.us[6]), s7: Math.round(SIM.us[7]),
      rfidVersion: '0x92', rfidUid: SIM.uid, oledOK: true, pcaOK: true, rfidOK: true, wifiOK: true, remote: false, fw: 'v9 (demo)',
      sensor: true, sCm: Math.round(SIM.cm*10)/10, sDeg: Math.round(SIM.deg*10)/10, sPkt: SIM.pkt,
      mv: SIM.mv ? SIM.mv.k : 0, mvDone: SIM.mv ? Math.round(SIM.mv.done*10)/10 : 0,
      mvTarget: SIM.mv ? SIM.mv.tgt : 0, mvSensor: true, mvNote: 'demo' };
  }
  return Promise.resolve({ ok: true, status: 200, json: () => Promise.resolve(obj), text: () => Promise.resolve(txt) });
}
function api(p){ return DEMO ? simReply(p) : fetch(p, {cache:'no-store'}); }
if (DEMO) $('demoPill').style.display = '';

// =====================================================
// SEKMELER
// =====================================================
let activePanel = 'drive';
function showPanel(p){
  activePanel = p;
  document.querySelectorAll('#nav button').forEach(b => b.classList.toggle('act', b.dataset.p === p));
  document.querySelectorAll('.panel').forEach(s => s.classList.toggle('act', s.id === 'p-' + p));
}
document.querySelectorAll('#nav button').forEach(b => b.addEventListener('click', () => showPanel(b.dataset.p)));

// =====================================================
// ORTAK: CANVAS + 3D
// =====================================================
function fit(c){
  const r = c.getBoundingClientRect(), d = window.devicePixelRatio || 1;
  const w = Math.max(1, Math.round(r.width * d)), h = Math.max(1, Math.round(r.height * d));
  if (c.width !== w || c.height !== h){ c.width = w; c.height = h; }
  const ctx = c.getContext('2d');
  ctx.setTransform(d, 0, 0, d, 0, 0);
  return { ctx, w: r.width, h: r.height };
}
function proj(cam, p){
  const cy = Math.cos(cam.yaw), sy = Math.sin(cam.yaw);
  const x = p[0]*cy - p[2]*sy;
  const z = p[0]*sy + p[2]*cy;
  const cp = Math.cos(cam.pitch), sp = Math.sin(cam.pitch);
  const y2 = p[1]*cp + z*sp;
  const z2 = -p[1]*sp + z*cp;
  const f = cam.dist / (cam.dist + z2);
  return [cam.cx + x*f*cam.scale, cam.cy - (y2 - cam.lift)*f*cam.scale, z2];
}
function shade(hex, k, a){
  const n = parseInt(hex.slice(1), 16);
  const r = Math.min(255, ((n>>16)&255)*k), g = Math.min(255, ((n>>8)&255)*k), b = Math.min(255, (n&255)*k);
  return 'rgba(' + (r|0) + ',' + (g|0) + ',' + (b|0) + ',' + (a===undefined?1:a) + ')';
}
function box(ctx, cam, c, s, color, alpha){
  const hx = s[0]/2, hy = s[1]/2, hz = s[2]/2, A = alpha===undefined?1:alpha;
  const P = [];
  for (let i=0;i<8;i++) P.push(proj(cam, [c[0] + (i&1?hx:-hx), c[1] + (i&2?hy:-hy), c[2] + (i&4?hz:-hz)]));
  const F = [[0,1,3,2,0.55],[4,5,7,6,0.8],[0,1,5,4,0.45],[2,3,7,6,1.15],[0,2,6,4,0.7],[1,3,7,5,0.9]];
  F.sort((a,b)=> (P[b[0]][2]+P[b[1]][2]+P[b[2]][2]+P[b[3]][2]) - (P[a[0]][2]+P[a[1]][2]+P[a[2]][2]+P[a[3]][2]));
  for (const f of F){
    ctx.beginPath(); ctx.moveTo(P[f[0]][0], P[f[0]][1]);
    for (let k=1;k<4;k++) ctx.lineTo(P[f[k]][0], P[f[k]][1]);
    ctx.closePath();
    ctx.fillStyle = shade(color, f[4], A); ctx.fill();
    ctx.strokeStyle = 'rgba(255,255,255,' + (0.14*A) + ')'; ctx.lineWidth = 1; ctx.stroke();
  }
}
function line3(ctx, cam, a, b, color, w){
  const A = proj(cam, a), B = proj(cam, b);
  ctx.beginPath(); ctx.moveTo(A[0], A[1]); ctx.lineTo(B[0], B[1]);
  ctx.strokeStyle = color; ctx.lineWidth = w; ctx.lineCap = 'round'; ctx.stroke();
}
function poly3(ctx, cam, pts, color, w){
  ctx.beginPath();
  pts.forEach((p, i) => { const A = proj(cam, p); if (i === 0) ctx.moveTo(A[0], A[1]); else ctx.lineTo(A[0], A[1]); });
  ctx.strokeStyle = color; ctx.lineWidth = w; ctx.lineCap = 'round'; ctx.lineJoin = 'round'; ctx.stroke();
}
function dot3(ctx, cam, p, r, color){
  const A = proj(cam, p);
  ctx.beginPath(); ctx.arc(A[0], A[1], r, 0, Math.PI*2); ctx.fillStyle = color; ctx.fill();
}
function text3(ctx, cam, p, t, color, font, align){
  const A = proj(cam, p);
  ctx.fillStyle = color; ctx.font = font || '11px system-ui'; ctx.textAlign = align || 'left'; ctx.fillText(t, A[0], A[1]);
}
function floorGrid(ctx, cam, size, step){
  for (let v=-size; v<=size+0.01; v+=step){
    line3(ctx, cam, [v,0,-size], [v,0,size], 'rgba(120,160,255,.13)', 1);
    line3(ctx, cam, [-size,0,v], [size,0,v], 'rgba(120,160,255,.13)', 1);
  }
}
// Ekran noktasini 3D duzleme cevir (Newton yontemi)
function solve2(fn, u, v, px, py){
  for (let it=0; it<12; it++){
    const f0 = fn(u, v); const ex = f0[0]-px, ey = f0[1]-py;
    if (Math.abs(ex) + Math.abs(ey) < 0.5) break;
    const h = 0.3, fu = fn(u+h, v), fv = fn(u, v+h);
    const a = (fu[0]-f0[0])/h, b = (fv[0]-f0[0])/h, c = (fu[1]-f0[1])/h, d = (fv[1]-f0[1])/h;
    const det = a*d - b*c;
    if (Math.abs(det) < 1e-6) return null;
    u -= ( d*ex - b*ey) / det;
    v -= (-c*ex + a*ey) / det;
  }
  return [u, v];
}

// =====================================================
// SURUS (WASD + tank)
// =====================================================
const keys = { w:false, a:false, s:false, d:false, q:false, e:false };
const btnDir = { w:false, a:false, s:false, d:false, q:false, e:false };
let tankOK = true, driveActive = false, curL = 0, curR = 0;

function driveFromInput(){
  const k = k2 => keys[k2] || btnDir[k2];
  const vmax = +$('spMax').value, vturn = +$('spTurn').value, arc = +$('spArc').value / 100;
  let L = 0, R = 0, mode = 'DUR';
  const thr = k('w') ? 1 : (k('s') ? -1 : 0);
  const st  = k('a') ? -1 : (k('d') ? 1 : 0);
  if (k('q')){ L = -vturn; R = vturn; mode = 'KENDI EKSENINDE SOLA'; }
  else if (k('e')){ L = vturn; R = -vturn; mode = 'KENDI EKSENINDE SAGA'; }
  else if (thr !== 0){
    L = thr * vmax; R = thr * vmax;
    if (st < 0){ L = thr * vmax * (1 - arc); mode = thr > 0 ? 'ILERI-SOL YAY' : 'GERI-SOL YAY'; }
    else if (st > 0){ R = thr * vmax * (1 - arc); mode = thr > 0 ? 'ILERI-SAG YAY' : 'GERI-SAG YAY'; }
    else mode = thr > 0 ? 'ILERI' : 'GERI';
  }
  else if (st !== 0){ L = st * vturn; R = -st * vturn; mode = st < 0 ? 'KENDI EKSENINDE SOLA' : 'KENDI EKSENINDE SAGA'; }
  return { L: Math.round(L), R: Math.round(R), mode };
}
function sendTank(L, R){
  if (tankOK){
    api('/tank?l=' + L + '&r=' + R).then(r => { if (r.status === 404){ tankOK = false; toast('Robot kodu eski: yay donus icin robotu guncelleyin'); } }).catch(()=>{});
  } else {
    let c = 'stop';
    if (L > 0 && R > 0) c = 'forward'; else if (L < 0 && R < 0) c = 'back'; else if (L < R) c = 'left'; else if (L > R) c = 'right';
    api('/drive?cmd=' + c).catch(()=>{});
  }
}
let lastTickMs = 0;
function driveTick(force){
  if (motionRun) return;
  const o = driveFromInput();
  const moving = o.L !== 0 || o.R !== 0;
  if (moving || driveActive || force){
    if (moving || o.L !== curL || o.R !== curR || force) sendTank(o.L, o.R);
    driveActive = moving;
  }
  curL = o.L; curR = o.R;
  showMotors(o.L, o.R, o.mode);
}
// Elle surerken (WASD / butonlar) haritadaki robot da hareket etsin.
// Kalibrasyon degerlerine gore tahmin edilir; sensor varsa robot zaten kendini duzeltir.
function manualPoseTick(){
  const now = performance.now();
  const dt = lastTickMs ? Math.min(0.2, (now - lastTickMs) / 1000) : 0;
  lastTickMs = now;
  if (motionRun || !dt) return;
  if (!curL && !curR) return;
  const f = (curL + curR) / 2 / 255;          // ileri bileseni (-1..1)
  const t = (curR - curL) / 2 / 255;          // donus bileseni (-1..1)
  const d = f * calCm() * dt;                 // cm
  robotPose.h += t * calDeg() * dt;           // derece (saat yonu tersi +)
  robotPose.x = clamp(robotPose.x + d * Math.cos(robotPose.h * RAD), 0, FW);
  robotPose.y = clamp(robotPose.y - d * Math.sin(robotPose.h * RAD), 0, FH);
  startPose = { x: robotPose.x, y: robotPose.y, h: normDeg(robotPose.h) };
  updPoseInputs();
}
setInterval(() => { driveTick(false); manualPoseTick(); }, 100);
function showMotors(L, R, mode){
  const set = (bar, lbl, v) => {
    const p = v / 255 * 50;
    bar.style.top = (p >= 0 ? 50 - p : 50) + '%';
    bar.style.height = Math.abs(p) + '%';
    lbl.textContent = Math.round(v / 255 * 100) + '%';
  };
  set($('barL'), $('lblL'), L); set($('barR'), $('lblR'), R);
  $('tankInfo').textContent = 'Hareket: ' + mode + '   |   Sol: ' + L + '   Sag: ' + R + ' (PWM)';
  ['w','a','s','d','q','e'].forEach(k => $('k-' + k).classList.toggle('on', keys[k] || btnDir[k]));
}
function stopDrive(){
  for (const k in keys) keys[k] = false;
  for (const k in btnDir) btnDir[k] = false;
  driveActive = false; curL = curR = 0;
  sendTank(0, 0);
  showMotors(0, 0, 'DUR');
  if (typeof startPose !== 'undefined' && !motionRun) lsSet('rl_pose', JSON.stringify(startPose));
}
document.querySelectorAll('[data-dr]').forEach(el => {
  const combo = el.dataset.dr.split('');
  el.addEventListener('pointerdown', e => {
    e.preventDefault(); try{ el.setPointerCapture(e.pointerId); }catch(_){}
    el.classList.add('on'); combo.forEach(k => btnDir[k] = true); driveTick(true);
  });
  const end = e => { if (e) e.preventDefault(); el.classList.remove('on'); combo.forEach(k => btnDir[k] = false); driveTick(true); };
  el.addEventListener('pointerup', end); el.addEventListener('pointercancel', end);
});
$('driveStop').addEventListener('click', stopDrive);
const spTxt = () => {
  $('spMaxT').textContent = Math.round($('spMax').value/255*100) + '%';
  $('spTurnT').textContent = Math.round($('spTurn').value/255*100) + '%';
  $('spArcT').textContent = $('spArc').value + '%';
};
['spMax','spTurn','spArc'].forEach(id => $(id).addEventListener('input', spTxt));
$('spMax').addEventListener('change', function(){ api('/speed?v=' + this.value).catch(()=>{}); });
spTxt();

// Klavye
let linKey = 0;
document.addEventListener('keydown', e => {
  if (e.target && (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA')) return;
  const k = e.key.toLowerCase();
  if (k in keys){ if (!keys[k]){ keys[k] = true; driveTick(true); } e.preventDefault(); }
  else if (k === ' '){ abortMotion(); stopDrive(); e.preventDefault(); }
  else if (k === 'escape'){ emergency(); }
  else if (k === 'r' && linKey !== 1){ linKey = 1; sendLinear('up'); }
  else if (k === 'f' && linKey !== -1){ linKey = -1; sendLinear('down'); }
  else if ('123456'.includes(k)){ showPanel(['drive','moves','lin','arm','path','sys'][+k - 1]); }
});
document.addEventListener('keyup', e => {
  const k = e.key.toLowerCase();
  if (k in keys){ keys[k] = false; driveTick(true); }
  if ((k === 'r' && linKey === 1) || (k === 'f' && linKey === -1)){ linKey = 0; stopLinear(); }
});

// =====================================================
// LINEER (kule + NEMA23 + kaplin + vidali mil + somun + saft + kanca)
// =====================================================
const ZIP_H = 90;                                   // zipline baslangic yuksekligi (sartname)
let linPos = 0, linMax = 16000, linDir = 0, linGoto = -1, linShown = 0, screwPhase = 0;
let fixMode = false, fixPos = 0, xray = true;
$('h0Cm').value = lsGet('rl_h0', '62'); $('h1Cm').value = lsGet('rl_h1', '90');
const h0 = () => parseFloat($('h0Cm').value) || 62;
const h1 = () => Math.max(h0() + 1, parseFloat($('h1Cm').value) || 90);
const stroke = () => h1() - h0();
const stepToCm = s => s / linMax * stroke();              // cikan mesafe
const hookH = s => h0() + stepToCm(s);                     // kanca yuksekligi (yerden)
const hToStep = H => Math.round(clamp((H - h0()) / stroke(), 0, 1) * linMax);
function updStroke(){ $('strokeInfo').textContent = 'Strok: ' + stroke().toFixed(1) + ' cm  |  1 cm = ' + Math.round(linMax / stroke()) + ' adim'; }
$('h0Cm').addEventListener('change', function(){ lsSet('rl_h0', this.value); updStroke(); });
$('h1Cm').addEventListener('change', function(){ lsSet('rl_h1', this.value); updStroke(); });
updStroke();

function sendLinear(c){ api('/linear?cmd=' + c).catch(()=>{}); }
function stopLinear(){ sendLinear('stop'); }
function holdBtn(id, onDown, onUp){
  const el = $(id);
  el.addEventListener('pointerdown', e => { e.preventDefault(); try{ el.setPointerCapture(e.pointerId); }catch(_){} el.classList.add('on'); onDown(); });
  const end = e => { if (e) e.preventDefault(); el.classList.remove('on'); onUp(); };
  el.addEventListener('pointerup', end); el.addEventListener('pointercancel', end);
}
holdBtn('linearUp',   ()=>sendLinear('up'),   stopLinear);
holdBtn('linearDown', ()=>sendLinear('down'), stopLinear);
$('linearStop').addEventListener('click', stopLinear);
$('zeroBtn').addEventListener('click', () => { api('/zero').then(r=>r.text()).then(toast).catch(()=>{}); });
$('calLinZero').addEventListener('click', () => { api('/zero').then(r=>r.text()).then(t => toast('EN ALT kaydedildi: ' + t)).catch(()=>{}); });
$('calLinTop').addEventListener('click', () => {
  api('/linmax?here=1').then(r => { if (r.status === 404){ toast('Robot kodu eski: once v6 kodunu yukleyin'); return ''; } return r.text(); })
    .then(t => { if (t){ toast(t); logEvent('Lineer en ust: ' + t); } }).catch(()=>{});
});
$('linMaxSet').addEventListener('click', () => {
  const v = Math.round(+$('linMaxIn').value || 0);
  if (v < 100){ toast('Gecerli bir adim sayisi yazin (ornegin 16000)'); return; }
  api('/linmax?v=' + v).then(r => r.status === 404 ? 'Robot kodu eski: once v6 kodunu yukleyin' : r.text()).then(toast).catch(()=>{});
});
$('linLimit').addEventListener('change', function(){ api('/linlimit?on=' + (this.checked ? 1 : 0)).catch(()=>{}); toast(this.checked ? 'Yazilim siniri acik' : 'Yazilim siniri KAPALI: mekanik sona dayandirmayin!'); });
function gotoHeight(H, quiet){
  if (H > h1() + 0.01){ if (!quiet) toast('Bu yukseklik stroktan fazla (en fazla ' + h1().toFixed(1) + ' cm)'); H = h1(); }
  const st = hToStep(H);
  api('/lineargoto?v=' + st).then(r => {
    if (r.status === 404) toast('Robot kodu eski: HEDEFE GIT icin robotu guncelleyin');
    else if (!quiet) toast('Kanca ' + hookH(st).toFixed(1) + ' cm yukseklige gidiyor (' + st + ' adim)');
  }).catch(()=>{});
}
$('gotoBtn').addEventListener('click', () => gotoHeight(parseFloat($('gotoCm').value) || h0()));
$('gotoZip').addEventListener('click', () => gotoHeight(ZIP_H));
$('gotoHome').addEventListener('click', () => gotoHeight(h0()));
$('linearSpeed').addEventListener('input', function(){ $('linTgt').textContent = this.value; });
$('linearSpeed').addEventListener('change', function(){ api('/linearspeed?v=' + this.value).catch(()=>{}); });

function setFix(on){
  fixMode = on;
  $('fixBox').classList.toggle('show', on);
  $('fixBtn').classList.toggle('on', on);
  if (on){ fixPos = Math.round(clamp(linPos, 0, linMax)); updFix(); stopLinear(); }
}
function updFix(){ $('fixNum').value = fixPos; $('fixCm').textContent = hookH(fixPos).toFixed(1); }
$('fixBtn').addEventListener('click', () => setFix(!fixMode));
$('fixCancel').addEventListener('click', () => setFix(false));
$('fixNum').addEventListener('input', function(){ fixPos = clamp(parseInt(this.value)||0, 0, linMax); $('fixCm').textContent = hookH(fixPos).toFixed(1); });
$('fixSave').addEventListener('click', () => {
  api('/setpos?v=' + Math.round(fixPos)).then(r=>r.text()).then(t=>{ toast(t); setFix(false); }).catch(()=>toast('Gonderilemedi'));
});

const linCanvas = $('linCanvas');
const linCam = { yaw: -0.75, pitch: 0.22, dist: 520, scale: 5, cx: 0, cy: 0, lift: 0 };
let linAutoRot = true;
function setLinAuto(on){ linAutoRot = on; $('linCamAuto').classList.toggle('act', on); }
$('xrayBtn').addEventListener('click', () => { xray = !xray; $('xrayBtn').classList.toggle('act', xray); });
$('linCamFront').addEventListener('click', () => { setLinAuto(false); linCam.yaw = 0; linCam.pitch = 0.12; });
$('linCamSide').addEventListener('click', () => { setLinAuto(false); linCam.yaw = -Math.PI/2; linCam.pitch = 0.12; });
$('linCamAuto').addEventListener('click', () => setLinAuto(!linAutoRot));
(function(){
  let down = false, lx = 0, ly = 0;
  linCanvas.addEventListener('pointerdown', e => {
    try{ linCanvas.setPointerCapture(e.pointerId); }catch(_){}
    if (fixMode){ linFixFromPointer(e); down = 'fix'; return; }
    down = 'cam'; lx = e.clientX; ly = e.clientY; setLinAuto(false);
  });
  linCanvas.addEventListener('pointermove', e => {
    if (down === 'fix'){ linFixFromPointer(e); return; }
    if (down !== 'cam') return;
    linCam.yaw += (e.clientX - lx) * 0.01;
    linCam.pitch = clamp(linCam.pitch + (e.clientY - ly) * 0.005, -0.2, 1.1);
    lx = e.clientX; ly = e.clientY;
  });
  const up = () => { down = false; };
  linCanvas.addEventListener('pointerup', up); linCanvas.addEventListener('pointercancel', up);
})();
function linFixFromPointer(e){
  const r = linCanvas.getBoundingClientRect(), py = e.clientY - r.top;
  const yb = proj(linCam, [0, h0(), 0])[1], yt = proj(linCam, [0, h1(), 0])[1];
  fixPos = Math.round(clamp((py - yb) / (yt - yb), 0, 1) * linMax); updFix();
}

// Cokgen prizma (sekizgen kule, silindir kaplin vb.)
function prism(ctx, cam, cx, cz, r, y0, y1, n, color, alpha, rot){
  const A = alpha === undefined ? 1 : alpha, ro = rot || 0;
  const bot = [], top = [];
  for (let i=0; i<n; i++){
    const a = ro + i * 2*Math.PI / n;
    bot.push(proj(cam, [cx + r*Math.cos(a), y0, cz + r*Math.sin(a)]));
    top.push(proj(cam, [cx + r*Math.cos(a), y1, cz + r*Math.sin(a)]));
  }
  const faces = [];
  for (let i=0; i<n; i++){
    const j = (i+1) % n;
    faces.push({ p: [bot[i], bot[j], top[j], top[i]], k: 0.55 + 0.45*Math.abs(Math.cos(ro + (i+0.5)*2*Math.PI/n - 0.8)) });
  }
  faces.push({ p: top, k: 1.15 });
  faces.sort((a,b) => b.p.reduce((s,q)=>s+q[2],0)/b.p.length - a.p.reduce((s,q)=>s+q[2],0)/a.p.length);
  for (const f of faces){
    ctx.beginPath(); ctx.moveTo(f.p[0][0], f.p[0][1]);
    for (let k=1; k<f.p.length; k++) ctx.lineTo(f.p[k][0], f.p[k][1]);
    ctx.closePath(); ctx.fillStyle = shade(color, f.k, A); ctx.fill();
    ctx.strokeStyle = 'rgba(255,255,255,' + (0.12*A) + ')'; ctx.lineWidth = 1; ctx.stroke();
  }
}

function drawHook(ctx, H, col, a, glow){
  // Ters U kanca: ic ust noktasi = H (zipline teline oturan nokta)
  const r = 1.8, pts = [[0, H - 7, 0], [0, H, 0]];
  for (let d = 180; d >= 0; d -= 12) pts.push([r + r*Math.cos(d*RAD), H + r*Math.sin(d*RAD), 0]);
  pts.push([2*r, H - 2.8, 0]);
  if (glow){ ctx.shadowColor = col; ctx.shadowBlur = 14; }
  ctx.globalAlpha = a; poly3(ctx, linCam, pts, col, 4); ctx.globalAlpha = 1;
  ctx.shadowBlur = 0;
}

function drawLinear(){
  const { ctx, w, h } = fit(linCanvas);
  ctx.clearRect(0, 0, w, h);
  linCam.cx = w * 0.5; linCam.cy = h - 28; linCam.scale = h / 118;
  if (linAutoRot && !fixMode) linCam.yaw += 0.0022;
  const moving = linDir !== 0;
  if (moving) screwPhase = (screwPhase + 0.09 * linDir) % 1;

  const H0 = h0(), S = stroke();
  const towerTop = H0 - 9;                                        // kanca 0'dayken saft 2 cm disarida
  const towerH = Math.max(18, Math.min(S + 18, towerTop - 6));
  const deck = towerTop - towerH;                                  // robot ust yuzeyi
  linShown += (linPos - linShown) * 0.25;
  const ext = clamp(stepToCm(linShown), -5, S + 5), Hk = H0 + ext;

  // zemin
  floorGrid(ctx, linCam, 60, 10);

  // ZIPLINE (Y cercevesi arkada, X cercevesi robotun ustunde)
  const zipZ = -100, px = 42.5;
  const reached = Math.abs(Hk - ZIP_H) <= 1;
  for (const [zz, hh, lbl] of [[zipZ, 70, 'Y (70 cm)'], [0, 90, 'X (90 cm)']]){
    box(ctx, linCam, [-px, hh/2, zz], [3, hh, 3], '#111827');
    box(ctx, linCam, [ px, hh/2, zz], [3, hh, 3], '#111827');
    box(ctx, linCam, [0, hh + 1.5, zz], [88, 3, 3], '#111827');
    text3(ctx, linCam, [px + 4, hh + 4, zz], lbl, '#cbd5e1', 'bold 11px system-ui');
  }
  ctx.shadowColor = '#22c55e'; ctx.shadowBlur = reached ? 22 : 6;
  line3(ctx, linCam, [0, ZIP_H, 0], [0, 70, zipZ], reached ? '#4ade80' : '#22c55e', reached ? 5 : 3.5);
  ctx.shadowBlur = 0;

  // yer cetveli (0 - 100 cm)
  const rx = -26, rz = 10;
  line3(ctx, linCam, [rx, 0, rz], [rx, 100, rz], 'rgba(226,232,240,.75)', 2);
  for (let c = 0; c <= 100; c += 5){
    const mj = c % 10 === 0;
    line3(ctx, linCam, [rx, c, rz], [rx - (mj ? 3 : 1.8), c, rz], 'rgba(226,232,240,.8)', mj ? 1.6 : 1);
    if (mj) text3(ctx, linCam, [rx - 4, c - 0.8, rz], c + '', 'rgba(226,232,240,.9)', '10px system-ui', 'right');
  }
  line3(ctx, linCam, [rx, ZIP_H, rz], [0, ZIP_H, 0], 'rgba(74,222,128,.35)', 1);
  if (linGoto >= 0){
    const Hg = hookH(linGoto);
    line3(ctx, linCam, [rx, Hg, rz], [0, Hg, 0], 'rgba(52,211,153,.8)', 1.5);
    text3(ctx, linCam, [rx + 2, Hg + 1.5, rz], 'HEDEF ' + Hg.toFixed(1), '#6ee7b7', 'bold 11px system-ui');
  }

  // robot govdesi
  box(ctx, linCam, [0, deck/2 + 2, 0], [44, Math.max(2, deck - 4), 32], '#1f2937');
  box(ctx, linCam, [0, deck - 0.6, 0], [46, 1.2, 34], '#0b0f19');
  for (const sx of [-17, 17]) for (const sz of [-17.5, 17.5]) box(ctx, linCam, [sx, 4.5, sz], [9, 9, 3.5], '#111827');

  // kule ici: NEMA23, kaplin, vidali mil, somun, saft
  const inner = () => {
    box(ctx, linCam, [0, deck + 3.2, 0], [5.7, 6.4, 5.7], '#0b0f19');
    text3(ctx, linCam, [3.2, deck + 3, 3.2], 'NEMA23', '#94a3b8', 'bold 9px system-ui');
    prism(ctx, linCam, 0, 0, 1.3, deck + 6.4, deck + 9, 10, '#a1a1aa', 1);
    text3(ctx, linCam, [2, deck + 8, 2], 'kaplin', '#d4d4d8', '9px system-ui');
    const sb = deck + 9, st = towerTop - 1;
    line3(ctx, linCam, [0, sb, 0], [0, st, 0], '#e2e8f0', 2.5);
    for (let y = sb + screwPhase * 1.2; y < st; y += 1.2) line3(ctx, linCam, [-0.7, y, 0], [0.7, y + 0.6, 0], 'rgba(148,163,184,.95)', 1);
    const nutY = sb + 2 + clamp(ext / S, 0, 1) * (st - sb - 5);
    box(ctx, linCam, [0, nutY, 0], [3.4, 2.2, 3.4], '#b45309');
    box(ctx, linCam, [3.2, nutY, 0], [2, 3, 1.6], '#78350f');     // donmeyi engelleyen kizak
    text3(ctx, linCam, [4.8, nutY, 1], 'somun', '#fcd34d', '9px system-ui');
    return nutY;
  };
  const shaftTop = Hk - 7;
  let nutY = 0;
  if (xray) nutY = inner();
  prism(ctx, linCam, 0, 0, 6, deck, towerTop, 8, '#111827', xray ? 0.28 : 1, Math.PI/8);
  if (!xray) nutY = deck + 11 + clamp(ext / S, 0, 1) * (towerH - 16);
  // saft (somundan yukari, kuleden disari)
  ctx.shadowColor = moving ? '#22d3ee' : 'transparent'; ctx.shadowBlur = moving ? 10 : 0;
  line3(ctx, linCam, [0.9, xray ? nutY : towerTop, 0.9], [0.9, shaftTop, 0.9], '#d6a756', 3.2);
  ctx.shadowBlur = 0;
  box(ctx, linCam, [0, towerTop + 0.4, 0], [5, 0.8, 5], '#1f2937');
  drawHook(ctx, Hk, reached ? '#4ade80' : '#fbbf24', 1, true);
  line3(ctx, linCam, [rx, Hk, rz], [0, Hk, 0], 'rgba(251,191,36,.55)', 1);
  text3(ctx, linCam, [6, Hk + 3, 0], 'KANCA ' + Hk.toFixed(1) + ' cm', reached ? '#86efac' : '#fde68a', 'bold 13px system-ui');
  if (moving) text3(ctx, linCam, [-5, Hk + 5, 0], linDir > 0 ? '▲' : '▼', '#22d3ee', 'bold 20px system-ui', 'center');

  if (fixMode){
    const Hf = hookH(fixPos);
    line3(ctx, linCam, [0.9, towerTop, 0.9], [0.9, Hf - 7, 0.9], 'rgba(232,121,249,.5)', 3);
    drawHook(ctx, Hf, '#e879f9', 0.8, true);
    text3(ctx, linCam, [6, Hf - 3, 0], fixPos + ' adim / ' + Hf.toFixed(1) + ' cm', '#f0abfc', 'bold 12px system-ui');
  }
  ctx.textAlign = 'left'; ctx.font = 'bold 13px system-ui';
  if (reached){ ctx.fillStyle = '#4ade80'; ctx.fillText('✓ KANCA ZIPLINE YUKSEKLIGINDE', 12, 22); }
  if (linPos > linMax){ ctx.fillStyle = '#fbbf24'; ctx.fillText('UYARI: tam strok ustunde!', 12, 42); }
}

// =====================================================
// ROBOT KOL
// =====================================================
const armCanvas = $('armCanvas');
const armCam = { yaw: -0.9, pitch: 0.42, dist: 230, scale: 6, cx: 0, cy: 0, lift: 12 };
let armView = '3d', armMode = 'ik', armTool = null;   // armTool: 'box' | 'ruler' | null
const tgt = { 4: 90, 5: 90, 6: 90, 7: 90 };
const act = { 4: 90, 5: 90, 6: 90, 7: 90 };
let servoSynced = false, ikUnreach = false;
const dirty = {};
let boxObj = null;          // { f, l, z, held }
const armRuler = { a: null, b: null, view: null };

// Servo kalibrasyonu: tip (180/270), yon, ofset, sinirlar
const SCFG_DEF = () => ({ range: 180, dir: 1, off: 0, min: 0, max: 180 });
let SCFG = {};
try{ SCFG = JSON.parse(lsGet('rl_scfg', '{}')) || {}; }catch(_){ SCFG = {}; }
[4,5,6,7].forEach(ch => { SCFG[ch] = Object.assign(SCFG_DEF(), SCFG[ch] || {}); });
function saveScfg(){ lsSet('rl_scfg', JSON.stringify(SCFG)); }
let matchMode = false;   // EKRANI KOLA ESLE modu: robota aci gonderilmez
function rawToUs(ch, raw){ const c = SCFG[ch]; return Math.round(500 + clamp(raw, 0, c.range) / c.range * 2000); }
function usToRaw(ch, us){ return (us - 500) / 2000 * SCFG[ch].range; }
function logToRaw(ch, L){ const c = SCFG[ch]; return c.off + c.dir * L; }
function rawToLog(ch, raw){ const c = SCFG[ch]; return (raw - c.off) / c.dir; }
function usToDeg(us, ch){ return rawToLog(ch, usToRaw(ch, us)); }
function degToUs(d, ch){ return rawToUs(ch, logToRaw(ch, d)); }
function jointFromDeg(j, d){ return j.j90 + j.dir * (d - 90); }
function degFromJoint(j, a){ return 90 + (a - j.j90) / j.dir; }
function setServo(ch, deg){ if (matchMode) return; const c = SCFG[ch]; deg = clamp(deg, c.min, c.max); if (Math.abs(tgt[ch] - deg) < 0.2) return; tgt[ch] = deg; dirty[ch] = true; }
setInterval(() => {
  for (const ch in dirty){ if (!dirty[ch]) continue; dirty[ch] = false; api('/pulse?ch=' + ch + '&us=' + degToUs(tgt[ch], +ch)).catch(()=>{}); }
}, 90);

function fk(d){
  return fkJ(jointFromDeg(JOINT.base, d[JOINT.base.ch]), jointFromDeg(JOINT.shoulder, d[JOINT.shoulder.ch]), jointFromDeg(JOINT.elbow, d[JOINT.elbow.ch]));
}
// eklem acilarindan (derece) kol noktalari
function fkJ(yawD, t1D, t2D){
  const yaw = yawD * RAD, t1 = t1D * RAD, t2 = t2D * RAD;
  const S = { r: 0, z: ARM.H0 };
  const E = { r: ARM.L1*Math.cos(t1), z: S.z + ARM.L1*Math.sin(t1) };
  const W = { r: E.r + ARM.L2*Math.cos(t1+t2), z: E.z + ARM.L2*Math.sin(t1+t2) };
  const T = { r: W.r + ARM.G*Math.cos(t1+t2),  z: W.z + ARM.G*Math.sin(t1+t2) };
  return { yaw, t1, t2, S, E, W, T };
}
function ik(r, z){
  const L1 = ARM.L1, L2 = ARM.L2 + ARM.G, dz = z - ARM.H0;
  let d = Math.hypot(r, dz); let unreach = false;
  const dMin = Math.abs(L1 - L2) + 0.5, dMax = L1 + L2 - 0.05;
  if (d > dMax){ d = dMax; unreach = true; }
  if (d < dMin){ d = dMin; unreach = true; }
  const c = clamp((d*d - L1*L1 - L2*L2) / (2*L1*L2), -1, 1);
  const t2 = -Math.acos(c);
  const t1 = Math.atan2(dz, r) - Math.atan2(L2*Math.sin(t2), L1 + L2*Math.cos(t2));
  const s = degFromJoint(JOINT.shoulder, t1 / RAD), e = degFromJoint(JOINT.elbow, t2 / RAD);
  const cs = SCFG[JOINT.shoulder.ch], ce = SCFG[JOINT.elbow.ch];
  if (s < cs.min || s > cs.max || e < ce.min || e > ce.max) unreach = true;
  return { s: clamp(s, cs.min, cs.max), e: clamp(e, ce.min, ce.max), unreach };
}
function applyIK(r, z){ const s = ik(r, Math.max(z, 0)); ikUnreach = s.unreach; setServo(JOINT.shoulder.ch, s.s); setServo(JOINT.elbow.ch, s.e); return s; }
function gripPercent(deg){ const g = JOINT.grip; return clamp((deg - g.open) / (g.close - g.open) * 100, 0, 100); }
function setGrip(p){ const g = JOINT.grip; setServo(g.ch, g.open + (g.close - g.open) * clamp(p,0,100) / 100); }
function to3(p, yaw){ return [p.r*Math.cos(yaw), p.z, -p.r*Math.sin(yaw)]; }
function tipWorld(k){ return { f: k.T.r*Math.cos(k.yaw), l: k.T.r*Math.sin(k.yaw), z: k.T.z }; }

// Kutu fizigi (tutma / birakma)
function updateBox(){
  if (!boxObj) return;
  const k = fk(act), tp = tipWorld(k), g = gripPercent(act[JOINT.grip.ch]);
  if (boxObj.held){
    if (g < 40){ boxObj.held = false; toast('Kutu birakildi'); }
    else { boxObj.f = tp.f; boxObj.l = tp.l; boxObj.z = tp.z; }
  } else {
    const dist = Math.hypot(tp.f - boxObj.f, tp.l - boxObj.l, tp.z - boxObj.z);
    if (g >= 70 && dist < 6.5){ boxObj.held = true; toast('Kutu tutuldu!'); }
    if (boxObj.z > BOX/2) boxObj.z = Math.max(BOX/2, boxObj.z - 1.2);   // yere dus
  }
}

function drawArm3D(ctx, k, a, glow, gripDeg){
  const S = to3(k.S, k.yaw), E = to3(k.E, k.yaw), W = to3(k.W, k.yaw);
  if (glow){ ctx.shadowColor = '#22d3ee'; ctx.shadowBlur = 14; }
  line3(ctx, armCam, [0,0,0], S, 'rgba(148,163,184,' + a + ')', 12);
  line3(ctx, armCam, S, E, 'rgba(34,211,238,' + a + ')', 10);
  line3(ctx, armCam, E, W, 'rgba(232,121,249,' + a + ')', 8);
  ctx.shadowBlur = 0;
  const ang = k.t1 + k.t2, open = (1 - gripPercent(gripDeg)/100) * 3.2 + 0.5;
  const nr = -Math.sin(ang), nz = Math.cos(ang);
  const P = (p, s) => to3({ r: p.r + nr*s, z: p.z + nz*s }, k.yaw);
  line3(ctx, armCam, P(k.W, open), P(k.W, -open), 'rgba(251,191,36,' + a + ')', 4);
  line3(ctx, armCam, P(k.W, open), P(k.T, open), 'rgba(251,191,36,' + a + ')', 4);
  line3(ctx, armCam, P(k.W, -open), P(k.T, -open), 'rgba(251,191,36,' + a + ')', 4);
  dot3(ctx, armCam, S, 7, 'rgba(226,232,240,' + a + ')');
  dot3(ctx, armCam, E, 6, 'rgba(226,232,240,' + a + ')');
  dot3(ctx, armCam, W, 5, 'rgba(226,232,240,' + a + ')');
}
function sideMap(w, h){ const sc = Math.min(w / 66, h / 58); return { ox: w*0.2, oy: h - 30, sc }; }
function topMap(w, h){ const sc = Math.min(w / 112, h / 62); return { ox: w/2, oy: h - 26, sc }; }
function topP(m, f, l){ return [m.ox - l*m.sc, m.oy - f*m.sc]; }

function drawRuler2D(ctx, A, B, label){
  ctx.strokeStyle = '#fbbf24'; ctx.fillStyle = '#fbbf24'; ctx.lineWidth = 2; ctx.setLineDash([]);
  ctx.beginPath(); ctx.arc(A[0], A[1], 5, 0, Math.PI*2); ctx.fill();
  if (!B) return;
  ctx.beginPath(); ctx.moveTo(A[0], A[1]); ctx.lineTo(B[0], B[1]); ctx.stroke();
  ctx.beginPath(); ctx.arc(B[0], B[1], 5, 0, Math.PI*2); ctx.fill();
  const mx = (A[0]+B[0])/2, my = (A[1]+B[1])/2;
  ctx.font = 'bold 14px system-ui'; ctx.textAlign = 'center';
  const tw = ctx.measureText(label).width + 12;
  ctx.fillStyle = 'rgba(15,23,42,.9)'; ctx.fillRect(mx - tw/2, my - 22, tw, 20);
  ctx.fillStyle = '#fde68a'; ctx.fillText(label, mx, my - 7);
}

function drawArm(){
  const { ctx, w, h } = fit(armCanvas);
  ctx.clearRect(0, 0, w, h);
  const kA = matchMode ? fkJ(MJ.yaw, MJ.t1, MJ.t2) : fk(act), kT = matchMode ? kA : fk(tgt), reach = ARM.L1 + ARM.L2 + ARM.G;

  if (armView === '3d'){
    armCam.cx = w/2; armCam.cy = h*0.64; armCam.scale = Math.min(w, h*1.4) / 100;
    floorGrid(ctx, armCam, 50, 10);
    const ring = []; for (let a=0; a<=360; a+=10) ring.push([reach*Math.cos(a*RAD), 0, reach*Math.sin(a*RAD)]);
    ctx.setLineDash([6,6]); poly3(ctx, armCam, ring, 'rgba(34,211,238,.35)', 1.5); ctx.setLineDash([]);
    box(ctx, armCam, [0, 2, 0], [16, 4, 16], '#334155');
    box(ctx, armCam, [0, 5.5, 0], [10, 3, 10], '#475569');
    if (boxObj){
      box(ctx, armCam, [boxObj.f, boxObj.z, -boxObj.l], [BOX, BOX, BOX], boxObj.held ? '#f59e0b' : '#3b82f6');
    }
    drawArm3D(ctx, kT, 0.25, false, tgt[JOINT.grip.ch]);
    drawArm3D(ctx, kA, 1, true, act[JOINT.grip.ch]);
    if (armMode === 'ik'){ dot3(ctx, armCam, to3(kT.T, kT.yaw), 9, ikUnreach ? '#f43f5e' : '#34d399'); }
  }
  else if (armView === 'side'){
    const m = sideMap(w, h), X = r => m.ox + r*m.sc, Y = z => m.oy - z*m.sc;
    ctx.strokeStyle = 'rgba(120,160,255,.15)'; ctx.lineWidth = 1;
    for (let r=-10; r<=55; r+=5){ ctx.beginPath(); ctx.moveTo(X(r), Y(0)); ctx.lineTo(X(r), Y(55)); ctx.stroke(); }
    for (let z=0; z<=55; z+=5){ ctx.beginPath(); ctx.moveTo(X(-10), Y(z)); ctx.lineTo(X(55), Y(z)); ctx.stroke(); }
    ctx.fillStyle = 'rgba(143,163,209,.8)'; ctx.font = '10px system-ui'; ctx.textAlign = 'center';
    for (let r=0; r<=50; r+=10) ctx.fillText(r + '', X(r), Y(0) + 14);
    ctx.setLineDash([6,6]); ctx.strokeStyle = 'rgba(34,211,238,.35)';
    ctx.beginPath(); ctx.arc(X(0), Y(ARM.H0), reach*m.sc, 0, Math.PI*2); ctx.stroke(); ctx.setLineDash([]);
    ctx.fillStyle = '#334155'; ctx.fillRect(X(-5), Y(ARM.H0), 10*m.sc, ARM.H0*m.sc);
    if (boxObj){
      const bd = Math.hypot(boxObj.f, boxObj.l), bang = Math.atan2(boxObj.l, boxObj.f);
      const inPlane = Math.abs(bang - kA.yaw) < 12*RAD || boxObj.held;
      ctx.globalAlpha = inPlane ? 1 : 0.3;
      ctx.fillStyle = boxObj.held ? '#f59e0b' : '#3b82f6';
      ctx.fillRect(X(bd - BOX/2), Y(boxObj.z + BOX/2), BOX*m.sc, BOX*m.sc);
      ctx.globalAlpha = 1;
    }
    const seg = (k, a, glow) => {
      if (glow){ ctx.shadowColor = '#22d3ee'; ctx.shadowBlur = 14; }
      ctx.lineCap = 'round';
      const L = (p, q, col, lw) => { ctx.strokeStyle = col; ctx.lineWidth = lw; ctx.beginPath(); ctx.moveTo(X(p.r), Y(p.z)); ctx.lineTo(X(q.r), Y(q.z)); ctx.stroke(); };
      L(k.S, k.E, 'rgba(34,211,238,' + a + ')', 10);
      L(k.E, k.W, 'rgba(232,121,249,' + a + ')', 8);
      L(k.W, k.T, 'rgba(251,191,36,' + a + ')', 5);
      ctx.shadowBlur = 0;
      ctx.fillStyle = 'rgba(226,232,240,' + a + ')';
      for (const p of [k.S, k.E, k.W]){ ctx.beginPath(); ctx.arc(X(p.r), Y(p.z), 5, 0, Math.PI*2); ctx.fill(); }
    };
    seg(kT, 0.28, false); seg(kA, 1, true);
    if (armMode === 'ik'){ ctx.fillStyle = ikUnreach ? '#f43f5e' : '#34d399'; ctx.beginPath(); ctx.arc(X(kT.T.r), Y(kT.T.z), 9, 0, Math.PI*2); ctx.fill(); }
    if (armRuler.view === 'side' && armRuler.a){
      const A = [X(armRuler.a[0]), Y(armRuler.a[1])], B = armRuler.b ? [X(armRuler.b[0]), Y(armRuler.b[1])] : null;
      drawRuler2D(ctx, A, B, armRuler.b ? Math.hypot(armRuler.b[0]-armRuler.a[0], armRuler.b[1]-armRuler.a[1]).toFixed(1) + ' cm' : '');
    }
  }
  else {
    const m = topMap(w, h);
    ctx.setLineDash([6,6]); ctx.strokeStyle = 'rgba(34,211,238,.35)';
    ctx.beginPath(); ctx.arc(m.ox, m.oy, reach*m.sc, Math.PI, 0); ctx.stroke(); ctx.setLineDash([]);
    ctx.strokeStyle = 'rgba(120,160,255,.15)'; ctx.lineWidth = 1;
    for (let a=-90; a<=90; a+=15){ const q = topP(m, reach*Math.cos(a*RAD), reach*Math.sin(a*RAD)); ctx.beginPath(); ctx.moveTo(m.ox, m.oy); ctx.lineTo(q[0], q[1]); ctx.stroke(); }
    for (let r=10; r<reach; r+=10){ ctx.beginPath(); ctx.arc(m.ox, m.oy, r*m.sc, Math.PI, 0); ctx.stroke(); }
    ctx.fillStyle = '#334155'; ctx.beginPath(); ctx.arc(m.ox, m.oy, 8*m.sc, 0, Math.PI*2); ctx.fill();
    if (boxObj){
      const c = topP(m, boxObj.f, boxObj.l);
      ctx.fillStyle = boxObj.held ? '#f59e0b' : '#3b82f6';
      ctx.fillRect(c[0] - BOX/2*m.sc, c[1] - BOX/2*m.sc, BOX*m.sc, BOX*m.sc);
    }
    const seg = (k, a, glow) => {
      const e = topP(m, k.E.r*Math.cos(k.yaw), k.E.r*Math.sin(k.yaw)), t = topP(m, k.T.r*Math.cos(k.yaw), k.T.r*Math.sin(k.yaw));
      if (glow){ ctx.shadowColor = '#22d3ee'; ctx.shadowBlur = 14; }
      ctx.lineCap = 'round';
      ctx.strokeStyle = 'rgba(34,211,238,' + a + ')'; ctx.lineWidth = 10;
      ctx.beginPath(); ctx.moveTo(m.ox, m.oy); ctx.lineTo(e[0], e[1]); ctx.stroke();
      ctx.strokeStyle = 'rgba(232,121,249,' + a + ')'; ctx.lineWidth = 8;
      ctx.beginPath(); ctx.moveTo(e[0], e[1]); ctx.lineTo(t[0], t[1]); ctx.stroke();
      ctx.shadowBlur = 0;
    };
    seg(kT, 0.28, false); seg(kA, 1, true);
    if (armRuler.view === 'top' && armRuler.a){
      const A = topP(m, armRuler.a[0], armRuler.a[1]), B = armRuler.b ? topP(m, armRuler.b[0], armRuler.b[1]) : null;
      drawRuler2D(ctx, A, B, armRuler.b ? Math.hypot(armRuler.b[0]-armRuler.a[0], armRuler.b[1]-armRuler.a[1]).toFixed(1) + ' cm' : '');
    }
  }

  const tp = tipWorld(kA);
  $('armInfo').textContent =
    'Uc: ileri ' + tp.f.toFixed(1) + ' cm, sol ' + tp.l.toFixed(1) + ' cm, yukseklik ' + tp.z.toFixed(1) + ' cm  |  ' +
    JOINT.base.name + ' ' + act[JOINT.base.ch].toFixed(0) + '°  ' + JOINT.shoulder.name + ' ' + act[JOINT.shoulder.ch].toFixed(0) + '°  ' +
    JOINT.elbow.name + ' ' + act[JOINT.elbow.ch].toFixed(0) + '°  ' + JOINT.grip.name + ' %' + gripPercent(act[JOINT.grip.ch]).toFixed(0) +
    (ikUnreach ? '   ⚠ ULASILAMAZ' : '');
}

// Kol etkilesimi
let armDrag = null, lastPX = 0, lastPY = 0;
function armLocal(e){ const rc = armCanvas.getBoundingClientRect(); return { px: e.clientX - rc.left, py: e.clientY - rc.top, w: rc.width, h: rc.height }; }
function placeBoxAt(f, l){ boxObj = { f, l, z: BOX/2, held: false }; setArmTool(null); toast('Kutu kondu'); }
function rulerPoint(view, p){
  if (armRuler.view !== view || (armRuler.a && armRuler.b)){ armRuler.view = view; armRuler.a = p; armRuler.b = null; $('armRulerInfo').textContent = 'Ikinci noktaya dokunun'; }
  else { armRuler.b = p; $('armRulerInfo').textContent = 'Mesafe: ' + Math.hypot(p[0]-armRuler.a[0], p[1]-armRuler.a[1]).toFixed(1) + ' cm'; }
}
armCanvas.addEventListener('pointerdown', e => {
  try{ armCanvas.setPointerCapture(e.pointerId); }catch(_){}
  const L = armLocal(e); lastPX = e.clientX; lastPY = e.clientY;
  if (armView === '3d'){
    if (!armTool && boxObj && !boxObj.held){
      const bs = proj(armCam, [boxObj.f, boxObj.z, -boxObj.l]);
      if (Math.hypot(bs[0] - L.px, bs[1] - L.py) < 26){ armDrag = 'box3d'; return; }
    }
    if (armTool === 'box'){
      const s = solve2((u, v) => proj(armCam, [u, 0, v]), 25, 0, L.px, L.py);
      if (s) placeBoxAt(s[0], -s[1]);
      return;
    }
    const kT = fk(tgt), tipS = proj(armCam, to3(kT.T, kT.yaw));
    if (armMode === 'ik' && Math.hypot(tipS[0] - L.px, tipS[1] - L.py) < 30){ armDrag = 'tip3d'; return; }
    armDrag = 'cam'; return;
  }
  if (armView === 'side'){
    const m = sideMap(L.w, L.h), r = (L.px - m.ox) / m.sc, z = (m.oy - L.py) / m.sc;
    if (armTool === 'ruler'){ rulerPoint('side', [r, z]); return; }
    if (armMode === 'ik'){ armDrag = 'side'; applyIK(r, z); }
    return;
  }
  const m = topMap(L.w, L.h), f = (m.oy - L.py) / m.sc, l = (m.ox - L.px) / m.sc;
  if (armTool === 'box'){ placeBoxAt(f, l); return; }
  if (armTool === 'ruler'){ rulerPoint('top', [f, l]); return; }
  if (boxObj && !boxObj.held && Math.hypot(f - boxObj.f, l - boxObj.l) < BOX * 0.8){ armDrag = 'boxTop'; return; }
  if (armMode === 'ik'){ armDrag = 'top'; setServo(JOINT.base.ch, degFromJoint(JOINT.base, clamp(Math.atan2(l, f) / RAD, -90, 90))); }
});
armCanvas.addEventListener('pointermove', e => {
  if (!armDrag) return;
  const L = armLocal(e);
  if (armDrag === 'cam'){
    armCam.yaw += (e.clientX - lastPX) * 0.01;
    armCam.pitch = clamp(armCam.pitch + (e.clientY - lastPY) * 0.006, -0.1, 1.2);
    lastPX = e.clientX; lastPY = e.clientY; return;
  }
  if (armDrag === 'box3d'){
    const s = solve2((u, v) => proj(armCam, [u, 0, v]), boxObj.f, -boxObj.l, L.px, L.py);
    if (s){ boxObj.f = s[0]; boxObj.l = -s[1]; }
    return;
  }
  if (armDrag === 'boxTop'){ const m = topMap(L.w, L.h); boxObj.f = (m.oy - L.py) / m.sc; boxObj.l = (m.ox - L.px) / m.sc; return; }
  if (armDrag === 'tip3d'){
    const kT = fk(tgt);
    const s = solve2((r, z) => proj(armCam, to3({ r, z }, kT.yaw)), kT.T.r, kT.T.z, L.px, L.py);
    if (s) applyIK(s[0], s[1]);
    return;
  }
  if (armDrag === 'side'){ const m = sideMap(L.w, L.h); applyIK((L.px - m.ox) / m.sc, (m.oy - L.py) / m.sc); return; }
  if (armDrag === 'top'){
    const m = topMap(L.w, L.h), f = (m.oy - L.py) / m.sc, l = (m.ox - L.px) / m.sc;
    setServo(JOINT.base.ch, degFromJoint(JOINT.base, clamp(Math.atan2(l, f) / RAD, -90, 90)));
  }
});
const armUp = () => { armDrag = null; };
armCanvas.addEventListener('pointerup', armUp); armCanvas.addEventListener('pointercancel', armUp);

document.querySelectorAll('#viewTabs button').forEach(b => b.addEventListener('click', () => {
  document.querySelectorAll('#viewTabs button').forEach(x => x.classList.remove('act')); b.classList.add('act'); armView = b.dataset.v;
}));
document.querySelectorAll('#modeTabs button').forEach(b => b.addEventListener('click', () => {
  document.querySelectorAll('#modeTabs button').forEach(x => x.classList.remove('act')); b.classList.add('act'); armMode = b.dataset.m;
  $('manualBox').classList.toggle('show', armMode === 'manual');
  $('modeHelp').textContent = armMode === 'ik' ? 'Kolun ucundaki yesil noktayi surukleyin. 3D\'de uca dokunmazsaniz kamera doner.' : 'Her servoyu kaydiriciyla ayri ayri kontrol edin.';
}));
function setArmTool(t){
  armTool = t;
  $('armRuler').classList.toggle('on', t === 'ruler');
  if (t === 'ruler'){ toast('YANDAN ya da USTTEN gorunumde iki noktaya dokunun'); if (armView === '3d') document.querySelector('#viewTabs [data-v="side"]').click(); }
}
$('armRuler').addEventListener('click', () => { setArmTool(armTool === 'ruler' ? null : 'ruler'); if (armTool !== 'ruler'){ armRuler.a = armRuler.b = null; $('armRulerInfo').textContent = ''; } });

const chName = {};
chName[JOINT.base.ch] = JOINT.base.name; chName[JOINT.shoulder.ch] = JOINT.shoulder.name;
chName[JOINT.elbow.ch] = JOINT.elbow.name; chName[JOINT.grip.ch] = JOINT.grip.name;
[4,5,6,7].forEach(ch => {
  $('n' + ch).textContent = 'CH' + ch + ' ' + (chName[ch] || '');
  $('m' + ch).addEventListener('input', function(){ const d = parseInt(this.value); $('t' + ch).textContent = d + '°'; setServo(ch, d); });
});
function syncManualUI(){
  [4,5,6,7].forEach(ch => { if (document.activeElement !== $('m' + ch)){ $('m' + ch).value = Math.round(tgt[ch]); $('t' + ch).textContent = Math.round(tgt[ch]) + '°'; } });
  if (document.activeElement !== $('gripSl')){ const p = Math.round(gripPercent(tgt[JOINT.grip.ch])); $('gripSl').value = p; $('gripTxt').textContent = '%' + p; }
}
$('gripSl').addEventListener('input', function(){ $('gripTxt').textContent = '%' + this.value; setGrip(+this.value); });
$('gripOpen').addEventListener('click', () => setGrip(0));
$('gripClose').addEventListener('click', () => setGrip(100));
$('armCenter').addEventListener('click', () => { [4,5,6,7].forEach(ch => setServo(ch, 90)); ikUnreach = false; });
$('armOff').addEventListener('click', () => { api('/offall').catch(()=>{}); });

// Kayitli pozlar
function defaultPose(r, z){ const s = ik(r, z); const p = {}; p[JOINT.base.ch] = 90; p[JOINT.shoulder.ch] = s.s; p[JOINT.elbow.ch] = s.e; return p; }
let POSES = { al: defaultPose(25, BOX/2), tasi: defaultPose(16, 28), birak: defaultPose(25, BOX/2 + 1) };
try{ const sv = JSON.parse(lsGet('rl_poses', 'null')); if (sv && sv.al && sv.tasi && sv.birak) POSES = sv; }catch(_){}
const POSE_CH = () => [JOINT.base.ch, JOINT.shoulder.ch, JOINT.elbow.ch];
function showPoses(){
  for (const k of ['al','tasi','birak']){
    const p = POSES[k];
    $('pz-' + k).textContent = JOINT.base.name[0] + ' ' + Math.round(p[JOINT.base.ch]) + '°  ' + JOINT.shoulder.name[0] + ' ' + Math.round(p[JOINT.shoulder.ch]) + '°  ' + JOINT.elbow.name[0] + ' ' + Math.round(p[JOINT.elbow.ch]) + '°';
  }
}
function goPose(k){ const p = POSES[k]; POSE_CH().forEach(ch => setServo(ch, p[ch])); ikUnreach = false; }
document.querySelectorAll('[data-ps]').forEach(b => b.addEventListener('click', () => {
  const k = b.dataset.ps, p = {};
  POSE_CH().forEach(ch => { p[ch] = Math.round(tgt[ch]); });
  POSES[k] = p; lsSet('rl_poses', JSON.stringify(POSES)); showPoses();
  toast(k.toUpperCase() + ' pozu kaydedildi');
}));
document.querySelectorAll('[data-pg]').forEach(b => b.addEventListener('click', () => goPose(b.dataset.pg)));
showPoses();

// =====================================================
// CIZ - GIT (TEKNOFEST UNIVERSITE SAHASI)
// =====================================================
const pathCanvas = $('pathCanvas');
const FW = 350, FH = 550;                             // cm (x: sag, y: asagi)
const CITY = { x: 7, y: 0, w: 336, h: 146 };
const CITIES = [ {n:'Sinop', x:167, y:10}, {n:'Nigde', x:87, y:40}, {n:'Tokat', x:251, y:35}, {n:'Aydin', x:47, y:99}, {n:'Elazig', x:291, y:96} ];
const LANES = [ {id:'A', x:12, dagX:100}, {id:'B', x:253, dagX:210} ];
const LANE_W = 85, LANE_Y = 289, LANE_H = 196;
const STARTS = { A: { x: 54.5, y: 520 }, B: { x: 295.5, y: 520 } };
// Haritadaki ogelerin konumlari (kasis hizi ve teslimat icin)
const KASIS = { y: 249, h: 40, w: 75 };
const ZIP_X_Y = LANE_Y, ZIP_Y_Y = LANE_Y + 100;

let startSel = 'A', pathTool = 'draw';
let pathPts = [], pathPlan = [], pathRunning = false, pathDrawing = false;
let robotPose = { x: STARTS.A.x, y: STARTS.A.y, h: 90 };
let startPose = { x: STARTS.A.x, y: STARTS.A.y, h: 90 };   // yolun baslayacagi konum (elle ayarlanabilir)
try{ const sp = JSON.parse(lsGet('rl_pose', 'null')); if (sp && isFinite(sp.x)) startPose = sp; }catch(_){}
robotPose = { ...startPose };
const ruler = { a: null, b: null };
let ZONES = [];                                           // yavas alanlar {x,y,w,h,sp}
try{ ZONES = JSON.parse(lsGet('rl_zones', '[]')) || []; }catch(_){ ZONES = []; }
let zoneDraft = null, robotDrag = false;

// Kalibrasyon (HAREKETLER sekmesinde)
$('calCm').value = lsGet('rl_cal_cm', '40'); $('calDeg').value = lsGet('rl_cal_deg', '120'); $('robW').value = lsGet('rl_robw', '45');
$('normSpd').value = lsGet('rl_norm', '100'); $('slowSpd').value = lsGet('rl_slow', '40');
$('kasisSpd').value = lsGet('rl_kasis', '85'); $('kasisSlow').checked = lsGet('rl_kasison', '1') === '1';
$('kasisSpd').addEventListener('change', function(){ lsSet('rl_kasis', this.value); buildPlan(false); });
$('kasisSlow').addEventListener('change', function(){ lsSet('rl_kasison', this.checked ? '1' : '0'); buildPlan(false); });
const calCm = () => Math.max(1, +$('calCm').value || 40);
const calDeg = () => Math.max(1, +$('calDeg').value || 120);
const normSpd = () => clamp(+$('normSpd').value || 100, 5, 100);
function updCalText(){
  $('calCmT').textContent = calCm().toFixed(1) + ' cm/sn'; $('calDegT').textContent = calDeg().toFixed(0) + ' °/sn';
  $('calSummary').textContent = 'Kalibrasyon: duz ' + calCm().toFixed(1) + ' cm/sn, donus ' + calDeg().toFixed(0) + ' °/sn (%100 hizda)';
}
$('calCm').addEventListener('change', function(){ lsSet('rl_cal_cm', this.value); updCalText(); updPathInfo(); updProgInfo(); });
$('calDeg').addEventListener('change', function(){ lsSet('rl_cal_deg', this.value); updCalText(); updPathInfo(); updProgInfo(); });
$('robW').addEventListener('change', function(){ lsSet('rl_robw', this.value); });
$('normSpd').addEventListener('change', function(){ lsSet('rl_norm', this.value); buildPlan(false); });
$('slowSpd').addEventListener('change', function(){ lsSet('rl_slow', this.value); });

const TOOLHELP = {
  draw: 'Robottan baslayarak parmakla serbest bir yol cizin.',
  pts: 'Sahaya dokunarak sirayla noktalar ekleyin. Robot noktalar arasinda duz gider.',

  ruler: 'Iki noktaya dokunun, aradaki mesafeyi cm olarak gosterir.',
  pan: 'Sahayi parmakla surukleyerek kaydirin (buyutulmusken).',
  robot: 'Robotu surukleyerek sahadaki gercek yerine koyun; yonunu yon kutusuna yazin.',
  slow: 'Sahada surukleyerek bir dikdortgen cizin. Robot bu alanin icinde yazdiginiz hizla gider.'
};
let prevTool = 'draw';
function setPathTool(t){
  if (t !== 'robot' && t !== 'pan') prevTool = t;
  pathTool = t;
  document.querySelectorAll('[data-t]').forEach(b => b.classList.toggle('on', b.dataset.t === t));
  $('toolHelp').textContent = TOOLHELP[t];
  $('robotDoneRow').style.display = t === 'robot' ? '' : 'none';
}
document.querySelectorAll('[data-t]').forEach(b => b.addEventListener('click', () => {
  const t = b.dataset.t;
  if ((t === 'robot' || t === 'pan') && pathTool === t) setPathTool(prevTool); else setPathTool(t);
}));
$('robotDone').addEventListener('click', () => { setPathTool(prevTool); toast('Robot konumu kaydedildi'); });
setPathTool('draw');
setTimeout(() => setLimit(limitOn), 0);
// yon: kullaniciya 0 = yukari (kuzey), saat yonu +; icerde h = 90 yukari, saat yonu tersi +
const normDeg = a => { a = a % 360; if (a > 180) a -= 360; if (a <= -180) a += 360; return a; };
function updPoseInputs(){
  for (const [id, v] of [['poseX', startPose.x], ['poseY', startPose.y], ['poseH', normDeg(90 - startPose.h)]]){
    if (document.activeElement !== $(id)) $(id).value = Math.round(v);
  }
}
function setStartPose(p, save){
  startPose = { x: clamp(p.x, 0, FW), y: clamp(p.y, 0, FH), h: normDeg(p.h) };
  if (!motionRun) robotPose = { ...startPose };
  updPoseInputs();
  if (save) lsSet('rl_pose', JSON.stringify(startPose));
  if (pathPts.length){ pathPts[0] = { x: startPose.x, y: startPose.y }; buildPlan(false); }
}
function setStart(s){
  if (motionRun) return;
  startSel = s; $('startA').classList.toggle('on', s === 'A'); $('startB').classList.toggle('on', s === 'B');
  pathPts = []; pathPlan = [];
  setStartPose({ x: STARTS[s].x, y: STARTS[s].y, h: 90 }, true);
  updPathInfo();
}
// =====================================================
// KOLI ALMA GOREVI (adim adim)
//   1) Dagitim merkezine git, kutulara donuk dur ve bekle
//   2) ALDIM -> geri cekil (kolu rahat kullanmak icin) ve bekle
//   3) ETIKETI OKUT -> kutuya donup RFID okur
//   4) DIGER YUZ -> 90 derece donup tekrar okur
// =====================================================
let pickTarget = null;
$('pickStop').value = lsGet('rl_pickstop', '25');
$('pickBack').value = lsGet('rl_pickback', '40');
$('pickStop').addEventListener('change', function(){ lsSet('rl_pickstop', this.value); });
$('pickBack').addEventListener('change', function(){ lsSet('rl_pickback', this.value); });

function nearestDag(){
  const d = Math.hypot(robotPose.x - 120, robotPose.y - 463);
  const e = Math.hypot(robotPose.x - 230, robotPose.y - 463);
  return TARGETS.find(t => t.id === (d <= e ? 'dagA' : 'dagB'));
}
async function faceTowards(x, y, label){
  if (!motionRun) return;
  const want = Math.atan2(robotPose.y - y, x - robotPose.x) / RAD;
  const d = normDeg(want - robotPose.h);
  if (Math.abs(d) < 4) return;
  $('pickupInfo').textContent = '▶ ' + label + ' (' + d.toFixed(0) + '°)';
  await execDrive(d > 0 ? 'left' : 'right', Math.abs(d), 60, trackPose(d > 0 ? 'left' : 'right', Math.abs(d)));
}
async function driveRouteTo(goal, label){
  const from = { x: robotPose.x, y: robotPose.y };
  const route = limitOn ? autoRoute(from, goal) : [from, goal];
  if (!route){ $('pickupInfo').textContent = '⛔ ' + label + ': sari yollardan ulasilamiyor'; return false; }
  for (const st of planFrom(route, robotPose.h)){
    if (!motionRun) return false;
    $('pickupInfo').textContent = '▶ ' + label + ' (' + (st.type === 'turn' ? st.deg.toFixed(0) + '°' : st.len.toFixed(0) + ' cm') + ')';
    if (st.type === 'turn') await execDrive(st.deg > 0 ? 'left' : 'right', Math.abs(st.deg), st.sp, trackPose(st.deg > 0 ? 'left' : 'right', Math.abs(st.deg)));
    else await execDrive('fwd', st.len, st.sp, trackPose('fwd', st.len));
    await waitMs(180);
  }
  return motionRun;
}
async function pickStep(fn){
  if (motionRun){ toast('Baska bir hareket calisiyor'); return; }
  stopDrive(); motionRun = true;
  try{ await fn(); }
  finally { motionRun = false; sendTank(0, 0); setStartPose(robotPose, true); }
}

// 1) KUTULARA GIT
$('goPickup').addEventListener('click', () => pickStep(async () => {
  pickTarget = nearestDag();
  const stop = clamp(+$('pickStop').value || 25, 5, 100);
  const goal = targetStopPoint({ ...pickTarget, stop }, { x: robotPose.x, y: robotPose.y });
  const ok = await driveRouteTo(goal, pickTarget.n);
  if (!ok){ $('pickupInfo').textContent = '■ Durduruldu'; return; }
  await faceTowards(pickTarget.x, pickTarget.y, 'kutulara donuluyor');
  const dist = Math.hypot(robotPose.x - pickTarget.x, robotPose.y - pickTarget.y);
  $('pickupInfo').innerHTML = '✓ <b>' + pickTarget.n + '</b> onunde bekliyor, kutulara <b>' + dist.toFixed(0) +
    ' cm</b> ve kutulara donuk. Koliyi kolla alin, sonra <b>✋ ALDIM</b>\'a basin.';
}));

// 2) ALDIM -> geri cekil
$('pickupDone').addEventListener('click', () => pickStep(async () => {
  const back = clamp(+$('pickBack').value || 40, 5, 150);
  $('pickupInfo').textContent = '▶ Geri cekiliyor (' + back + ' cm)...';
  await execDrive('back', back, 60, trackPose('back', back));
  if (!motionRun){ $('pickupInfo').textContent = '■ Durduruldu'; return; }
  $('pickupInfo').innerHTML = '✓ <b>' + back + ' cm geri cekildi.</b> Koliyi rahatca yerlestirin, sonra <b>📡 ETIKETI OKUT</b>\'a basin.';
}));

// RFID okuma bekleme
async function waitRfid(sec){
  rfidWaitUid = null;
  if (DEMO) setTimeout(() => { SIM.uid = 'A' + Math.floor(Math.random()*90+10) + ' B' + Math.floor(Math.random()*90+10) + ' C' + Math.floor(Math.random()*90+10); }, 2500);
  const t0 = Date.now();
  while (motionRun && Date.now() - t0 < sec * 1000){
    const kalan = Math.ceil(sec - (Date.now() - t0) / 1000);
    $('rfidLive').innerHTML = '📡 Etiket araniyor... <b>' + kalan + ' sn</b> (etiket okuyucuya 2-3 cm mesafede olmali)';
    if (rfidWaitUid) return rfidWaitUid;
    await sleep(200);
  }
  return null;
}

// 3) ETIKETI OKUT
$('pickupScan').addEventListener('click', () => pickStep(async () => {
  if (pickTarget) await faceTowards(pickTarget.x, pickTarget.y, 'kutuya donuluyor');
  const uid = await waitRfid(15);
  if (uid){
    $('rfidLive').innerHTML = '✓ <b>ETIKET OKUNDU:</b> ' + uid;
    $('pickupInfo').innerHTML = '✓ Okundu. Artik <b>TESLIMAT</b> bolumunden sehre dokunabilirsiniz.';
    logEvent('Koli etiketi okundu: ' + uid);
  } else {
    $('rfidLive').innerHTML = '⚠ Etiket okunamadi.';
    $('pickupInfo').innerHTML = '⚠ Okunamadi. Kutuyu okuyucuya yaklastirin ya da <b>🔄 DIGER YUZ</b> ile baska yuzu deneyin.';
  }
}));

// 4) DIGER YUZ: 90 derece don, tekrar oku
$('pickupTurn').addEventListener('click', () => pickStep(async () => {
  $('pickupInfo').textContent = '▶ Diger yuz icin 90° donuluyor...';
  await execDrive('left', 90, 60, trackPose('left', 90));
  if (!motionRun){ $('pickupInfo').textContent = '■ Durduruldu'; return; }
  const uid = await waitRfid(12);
  if (uid){
    $('rfidLive').innerHTML = '✓ <b>ETIKET OKUNDU:</b> ' + uid;
    $('pickupInfo').innerHTML = '✓ Okundu. Artik <b>TESLIMAT</b> bolumunden sehre dokunabilirsiniz.';
    logEvent('Koli etiketi okundu (diger yuz): ' + uid);
  } else {
    $('rfidLive').innerHTML = '⚠ Bu yuzde de okunamadi.';
    $('pickupInfo').innerHTML = '⚠ Okunamadi. <b>🔄 DIGER YUZ</b> ile bir yuz daha deneyebilirsiniz.';
  }
}));
$('pickupStop').addEventListener('click', () => { abortMotion(); $('pickupInfo').textContent = '■ Durduruldu'; $('rfidLive').textContent = '-'; });

// Teslimat: sehre tikla -> sari yollardan rota kur, hedefe 30 cm kala dur
let deliverTarget = null;
function deliverTo(id){
  const T = TARGETS.find(t => t.id === id);
  if (!T || motionRun) return;
  deliverTarget = id;
  document.querySelectorAll('#cityBtns button').forEach(b => b.classList.toggle('on', b.dataset.city === id));
  const from = { x: startPose.x, y: startPose.y };
  const goal = targetStopPoint(T, from);
  const route = limitOn ? autoRoute(from, goal) : [from, goal];
  if (!route){ $('deliverInfo').textContent = '⛔ ' + T.n + ': sari yollardan ulasilamiyor'; toast('Ulasilamiyor'); return; }
  pathPts = route; buildPlan(false);
  const tot = pathPlan.reduce((s2, x) => s2 + (x.type === 'go' ? x.len : 0), 0);
  $('deliverInfo').textContent = '🎯 ' + T.n + ' icin rota hazir: ' + (tot/100).toFixed(2) + ' m. Robot ' + T.stop + ' cm kala duracak. ▶ YOLU SUR';
}
function renderCityBtns(){
  const el = $('cityBtns'); el.innerHTML = '';
  TARGETS.filter(t => t.city).forEach(T => {
    const b = document.createElement('button');
    b.className = 'tool'; b.dataset.city = T.id; b.textContent = T.n;
    b.addEventListener('click', () => deliverTo(T.id));
    el.appendChild(b);
  });
  const d = document.createElement('button');
  d.className = 'tool'; d.textContent = 'Dagitim A'; d.dataset.city = 'dagA';
  d.addEventListener('click', () => deliverTo('dagA'));
  el.appendChild(d);
  const d2 = document.createElement('button');
  d2.className = 'tool'; d2.textContent = 'Dagitim B'; d2.dataset.city = 'dagB';
  d2.addEventListener('click', () => deliverTo('dagB'));
  el.appendChild(d2);
}
['poseX','poseY','poseH'].forEach(id => $(id).addEventListener('change', () => {
  setStartPose({ x: +$('poseX').value || 0, y: +$('poseY').value || 0, h: 90 - (+$('poseH').value || 0) }, true);
}));

// Yavas alanlar
// Kasisler: uzerinden gecerken hiz dusuk olmali (varsayilan %85)
function kasisZones(){
  const sp = clamp(+$('kasisSpd').value || 85, 5, 100);
  return LANES.map(L => ({ x: L.x, y: KASIS.y - 8, w: LANE_W, h: KASIS.h + 16, sp, kasis: true }));
}
function allZones(){ return $('kasisSlow').checked ? ZONES.concat(kasisZones()) : ZONES; }
function zoneSpeedAt(x, y){
  let sp = normSpd();
  for (const z of allZones()) if (x >= z.x && x <= z.x + z.w && y >= z.y && y <= z.y + z.h) sp = Math.min(sp, z.sp);
  return sp;
}
function saveZones(){ lsSet('rl_zones', JSON.stringify(ZONES)); renderZones(); buildPlan(false); }
function renderZones(){
  const el = $('zoneList');
  el.innerHTML = ZONES.length ? '' : '<div class="sub">Yavas alan yok.</div>';
  ZONES.forEach((z, i) => {
    const d = document.createElement('div'); d.className = 'zone';
    d.innerHTML = '🐢 <b>Alan ' + (i+1) + '</b> <input type="number" min="5" max="100" step="5" value="' + z.sp + '"> % <span class="sub">(' + Math.round(z.w) + ' x ' + Math.round(z.h) + ' cm)</span> <button style="min-height:30px;padding:4px 10px">✕</button>';
    d.querySelector('input').addEventListener('change', function(){ z.sp = clamp(+this.value || 40, 5, 100); saveZones(); });
    d.querySelector('button').addEventListener('click', () => { ZONES.splice(i, 1); saveZones(); });
    el.appendChild(d);
  });
}
$('startA').addEventListener('click', () => setStart('A'));
$('startB').addEventListener('click', () => setStart('B'));

const PZ = { k: 1, cx: FW/2, cy: FH/2 };             // buyutec: oran + gorunum merkezi (cm)
function fieldMap(w, h){ const base = Math.min((w - 16) / FW, (h - 16) / FH), sc = base * PZ.k; return { sc, ox: w/2 - PZ.cx*sc, oy: h/2 - PZ.cy*sc }; }
function fWraw(m, px, py){ return { x: (px - m.ox)/m.sc, y: (py - m.oy)/m.sc }; }
function clampView(){
  if (PZ.k <= 1.001){ PZ.k = 1; PZ.cx = FW/2; PZ.cy = FH/2; return; }
  PZ.cx = clamp(PZ.cx, 0, FW); PZ.cy = clamp(PZ.cy, 0, FH);
}
function zoomAt(factor, px, py){
  const rc = pathCanvas.getBoundingClientRect(), m = fieldMap(rc.width, rc.height);
  const p = fWraw(m, px, py);
  PZ.k = clamp(PZ.k * factor, 1, 16);
  const base = Math.min((rc.width - 16) / FW, (rc.height - 16) / FH), sc = base * PZ.k;
  PZ.cx = p.x - (px - rc.width/2) / sc; PZ.cy = p.y - (py - rc.height/2) / sc;
  clampView();
}
function zoomCenter(f){ const rc = pathCanvas.getBoundingClientRect(); zoomAt(f, rc.width/2, rc.height/2); }
const fS = (m, x, y) => [m.ox + x*m.sc, m.oy + y*m.sc];
function fW(m, px, py){ return { x: clamp((px - m.ox)/m.sc, 0, FW), y: clamp((py - m.oy)/m.sc, 0, FH) }; }

function rdp(pts, eps){
  if (pts.length < 3) return pts.slice();
  const a = pts[0], b = pts[pts.length-1];
  let idx = -1, dmax = 0;
  for (let i=1; i<pts.length-1; i++){
    const p = pts[i];
    const d = Math.abs((b.y-a.y)*p.x - (b.x-a.x)*p.y + b.x*a.y - b.y*a.x) / (Math.hypot(b.y-a.y, b.x-a.x) || 1);
    if (d > dmax){ dmax = d; idx = i; }
  }
  if (dmax > eps){ const l = rdp(pts.slice(0, idx+1), eps), r = rdp(pts.slice(idx), eps); return l.slice(0, -1).concat(r); }
  return [a, b];
}
function buildPlan(simplify){
  pathPlan = [];
  if (pathPts.length < 2){ updPathInfo(); return; }
  if (simplify) pathPts = rdp(pathPts, 5).filter((p, i, arr) => i === 0 || Math.hypot(p.x - arr[i-1].x, p.y - arr[i-1].y) > 8);
  pathPlan = planFrom(pathPts, startPose.h);
  updPathInfo();
}
// Nokta listesinden surus plani (donus + yavas alanlara gore parcalanmis duz gidis)
function planFrom(pts, hStart){
  const plan = [];
  let h = hStart;
  for (let i=1; i<pts.length; i++){
    const a = pts[i-1], b = pts[i];
    const len = Math.hypot(b.x - a.x, b.y - a.y);
    if (len < 0.5) continue;
    const hd = Math.atan2(a.y - b.y, b.x - a.x) / RAD;   // y asagi oldugu icin ters
    let dlt = hd - h; while (dlt > 180) dlt -= 360; while (dlt <= -180) dlt += 360;
    if (Math.abs(dlt) > 4) plan.push({ type: 'turn', deg: dlt, h0: h, at: a, sp: zoneSpeedAt(a.x, a.y) });
    // duz gidisi yavas alanlara gore parcala
    const P = t => ({ x: a.x + (b.x - a.x)*t, y: a.y + (b.y - a.y)*t });
    const n = Math.max(1, Math.ceil(len));
    let t0 = 0, cur = zoneSpeedAt(a.x, a.y);
    for (let k=1; k<=n; k++){
      const t = k / n, q = P(t), sp = zoneSpeedAt(q.x, q.y);
      if (sp !== cur){ plan.push({ type: 'go', len: (t - t0)*len, from: P(t0), to: P(t), h: hd, sp: cur }); t0 = t; cur = sp; }
    }
    if (1 - t0 > 1e-6) plan.push({ type: 'go', len: (1 - t0)*len, from: P(t0), to: b, h: hd, sp: cur });
    h = hd;
  }
  return plan;
}
function updPathInfo(){
  let tot = 0, turns = 0, sec = 0;
  const cm = calCm(), dg = calDeg();
  pathPlan.forEach(s => { const k = (s.sp || 100) / 100; if (s.type === 'go'){ tot += s.len; sec += s.len / (cm*k); } else { turns++; sec += Math.abs(s.deg) / (dg*k); } });
  const bad = pathBadSegs().length;
  const mode = $('chkMode').value === 'line' ? 'cizgi' : 'robot govdesi';
  $('pathInfo').textContent = pathPlan.length ? ((bad ? '⛔ ' + mode.toUpperCase() + ' SARI CIZGIYI GECIYOR (kirmizi kisimlar). 🛠 YOLU OTOMATIK DUZELT\'e basabilirsiniz. ' : (limitOn ? '✓ Yol sari sinirlarin icinde (' + mode + '). ' : '')) + 'Yol: ' + (tot/100).toFixed(2) + ' m, ' + turns + ' donus, tahmini sure ' + sec.toFixed(1) + ' sn.') : 'Henuz yol yok. Baslangic: ' + startSel;
}

const ptrs = new Map();
let pinch = null, panning = null, prevPts = [];
function pLocal(e){ const rc = pathCanvas.getBoundingClientRect(); return { x: e.clientX - rc.left, y: e.clientY - rc.top, w: rc.width, h: rc.height }; }
pathCanvas.addEventListener('pointerdown', e => {
  try{ pathCanvas.setPointerCapture(e.pointerId); }catch(_){}
  const L = pLocal(e);
  ptrs.set(e.pointerId, L);
  if (ptrs.size === 2){
    if (pathDrawing){ pathDrawing = false; pathPts = prevPts; buildPlan(false); }
    panning = null;
    const [p1, p2] = [...ptrs.values()];
    pinch = { d: Math.hypot(p1.x - p2.x, p1.y - p2.y), mx: (p1.x + p2.x)/2, my: (p1.y + p2.y)/2 };
    return;
  }
  if (ptrs.size > 2 || motionRun) return;
  if (pathTool === 'pan'){ panning = { x: L.x, y: L.y }; return; }
  const m = fieldMap(L.w, L.h), p = fW(m, L.x, L.y);
  const st = { x: startPose.x, y: startPose.y };
  if (pathTool === 'robot'){ robotDrag = true; setStartPose({ ...startPose, x: p.x, y: p.y }, false); return; }
  if (pathTool === 'slow'){ zoneDraft = { x0: p.x, y0: p.y, x1: p.x, y1: p.y }; return; }
  if (pathTool === 'ruler'){
    if (!ruler.a || ruler.b){ ruler.a = p; ruler.b = null; $('rulerInfo').textContent = '📏 Ikinci noktaya dokunun'; }
    else { ruler.b = p; const d = Math.hypot(p.x - ruler.a.x, p.y - ruler.a.y); $('rulerInfo').textContent = '📏 Mesafe: ' + d.toFixed(1) + ' cm (' + (d/100).toFixed(2) + ' m)'; }
    return;
  }
  robotPose = { ...startPose };
  if (pathTool === 'draw'){
    prevPts = pathPts.slice(); pathDrawing = true; pathPlan = [];
    // cizime robottan uzakta baslandiysa robottan oraya sari yollardan baglanti kur
    if (limitOn && Math.hypot(p.x - st.x, p.y - st.y) > 15 && !segOK(st, p, robR())){
      const conn = autoRoute(st, p);
      pathPts = conn ? conn : [st, p];
      if (conn) toast('Robottan cizimin basina yol otomatik baglandi');
    } else pathPts = [st, p];
  }
  else if (pathTool === 'pts'){
    if (pathPts.length === 0) pathPts = [st];
    const last = pathPts[pathPts.length - 1];
    if (limitOn && !segOK(last, p, robR())){
      const conn = autoRoute(last, p);
      if (conn){ pathPts = pathPts.concat(conn.slice(1)); toast('Nokta sari yollardan otomatik baglandi'); }
      else pathPts.push(p);
    } else pathPts.push(p);
    buildPlan(false);
  }

});
pathCanvas.addEventListener('pointermove', e => {
  if (!ptrs.has(e.pointerId)) return;
  const L = pLocal(e);
  ptrs.set(e.pointerId, L);
  if (pinch && ptrs.size >= 2){
    const [p1, p2] = [...ptrs.values()];
    const d = Math.hypot(p1.x - p2.x, p1.y - p2.y), mx = (p1.x + p2.x)/2, my = (p1.y + p2.y)/2;
    if (pinch.d > 10) zoomAt(d / pinch.d, mx, my);
    const m = fieldMap(L.w, L.h);
    PZ.cx -= (mx - pinch.mx) / m.sc; PZ.cy -= (my - pinch.my) / m.sc; clampView();
    pinch = { d, mx, my };
    return;
  }
  if (panning){
    const m = fieldMap(L.w, L.h);
    PZ.cx -= (L.x - panning.x) / m.sc; PZ.cy -= (L.y - panning.y) / m.sc; clampView();
    panning = { x: L.x, y: L.y };
    return;
  }
  if (robotDrag){ const m = fieldMap(L.w, L.h), p = fW(m, L.x, L.y); setStartPose({ ...startPose, x: p.x, y: p.y }, false); return; }
  if (zoneDraft){ const m = fieldMap(L.w, L.h), p = fW(m, L.x, L.y); zoneDraft.x1 = p.x; zoneDraft.y1 = p.y; return; }
  if (!pathDrawing) return;
  const m = fieldMap(L.w, L.h), p = fW(m, L.x, L.y), last = pathPts[pathPts.length-1];
  if (Math.hypot(p.x - last.x, p.y - last.y) > 3 / Math.max(1, PZ.k/2)) pathPts.push(p);
});
const endDraw = e => {
  ptrs.delete(e.pointerId);
  if (ptrs.size < 2) pinch = null;
  if (ptrs.size === 0) panning = null;
  if (robotDrag){ robotDrag = false; setStartPose(startPose, true); return; }
  if (zoneDraft){
    const z = { x: Math.min(zoneDraft.x0, zoneDraft.x1), y: Math.min(zoneDraft.y0, zoneDraft.y1), w: Math.abs(zoneDraft.x1 - zoneDraft.x0), h: Math.abs(zoneDraft.y1 - zoneDraft.y0), sp: clamp(+$('slowSpd').value || 40, 5, 100) };
    zoneDraft = null;
    if (z.w > 5 && z.h > 5){ ZONES.push(z); saveZones(); toast('Yavas alan eklendi: %' + z.sp); }
    return;
  }
  if (!pathDrawing) return;
  pathDrawing = false; buildPlan(true);
};
pathCanvas.addEventListener('pointerup', endDraw); pathCanvas.addEventListener('pointercancel', endDraw);
pathCanvas.addEventListener('wheel', e => {
  e.preventDefault();
  const L = pLocal(e);
  zoomAt(e.deltaY < 0 ? 1.18 : 1/1.18, L.x, L.y);
}, { passive: false });
$('zIn').addEventListener('click', () => zoomCenter(1.5));
$('zOut').addEventListener('click', () => zoomCenter(1/1.5));
$('zFit').addEventListener('click', () => { PZ.k = 1; clampView(); });

// ORTAK HAREKET MOTORU: sureye gore surus (kalibrasyon %100 hizda)
let motionRun = false, progRunning = false;
// Sensorlu hareket: robot kendi olcer (mesafe sensoru + jiroskop).
// Robot kodu eskiyse ya da kullanici kapattiysa sureye gore calisir.
let moveEndpointOK = true;
const MOVE_T = { fwd: 1, back: 2, left: 3, right: 4 };
const sensorWanted = () => $('useSensor').checked && sensorLive;
async function execDrive(kind, amount, sp, onStep){
  const k = clamp(sp || 100, 5, 100) / 100;
  amount = Math.abs(amount);

  // ---- 1) ROBOT KENDI OLCSUN (sensorlu) ----
  if (moveEndpointOK && $('useSensor').checked){
    let started = false;
    try{
      const r = await api('/move?t=' + MOVE_T[kind] + '&v=' + amount + '&sp=' + Math.round(k * 100) +
                          '&cm=' + calCm() + '&dg=' + calDeg());
      if (r.status === 404){ moveEndpointOK = false; toast('Robot kodu eski: sensorlu hareket icin v8 kodunu yukleyin'); }
      else if (r.status === 200){ started = true; mvMethod = await r.text(); }
    }catch(_){}
    if (started){
      const t0 = Date.now(), maxMs = 60000;
      while (motionRun && Date.now() - t0 < maxMs){
        await sleep(90);
        if (mvKind === 0 && Date.now() - t0 > 500) break;       // robot bitirdi
        if (onStep && mvTarget > 0) onStep(clamp(mvDone / mvTarget, 0, 1));
      }
      if (!motionRun) api('/movestop').catch(()=>{});
      if (onStep) onStep(1);
      showMotors(0, 0, 'DUR');
      return;
    }
  }

  // ---- 2) SUREYE GORE (yedek) ----
  const ms = (kind === 'fwd' || kind === 'back') ? amount / (calCm() * k) * 1000 : amount / (calDeg() * k) * 1000;
  const P = Math.round(255 * k);
  const L = kind === 'fwd' ? P : kind === 'back' ? -P : kind === 'left' ? -P : P;
  const R = kind === 'fwd' ? P : kind === 'back' ? -P : kind === 'left' ? P : -P;
  const t0 = performance.now(); let last = -1000;
  mvMethod = 'SURE';
  while (motionRun){
    const el = performance.now() - t0;
    if (el >= ms) break;
    if (el - last >= 150){ sendTank(L, R); showMotors(L, R, 'OTOMATIK'); last = el; }
    if (onStep) onStep(el / ms);
    await sleep(25);
  }
  sendTank(0, 0); showMotors(0, 0, 'DUR');
  if (onStep) onStep(1);
}
async function waitMs(ms){ const t0 = Date.now(); while (motionRun && Date.now() - t0 < ms) await sleep(40); }
async function waitServosRun(ms){
  const t0 = Date.now();
  while (motionRun && Date.now() - t0 < ms){
    if ([4,5,6,7].every(ch => Math.abs(act[ch] - tgt[ch]) < 1.5)) return;
    await sleep(60);
  }
}
async function waitLinear(ms){
  await waitMs(500);
  const t0 = Date.now();
  while (motionRun && Date.now() - t0 < ms){ if (linDir === 0 && linGoto < 0) return; await sleep(80); }
}
function abortMotion(){
  const was = motionRun;
  motionRun = false; pathRunning = false;
  if (was){ api('/movestop').catch(()=>{}); sendTank(0, 0); if (progRunning) stopLinear(); }
  progRunning = false;
}
async function runPath(){
  if (motionRun || pathPlan.length === 0){ if (!pathPlan.length) toast('Once bir yol cizin'); return; }
  if (pathBadSegs().length){ toast('⛔ Yol sari cizgilerin disina cikiyor. Yolu duzeltin ya da SARI SINIR\'i kapatin.'); return; }
  stopDrive(); motionRun = true; pathRunning = true;
  robotPose = { ...startPose };
  for (let i=0; i<pathPlan.length && motionRun; i++){
    const s = pathPlan[i];
    $('pathInfo').textContent = 'Adim ' + (i+1) + ' / ' + pathPlan.length + (s.type === 'turn' ? ' - DONUS ' + s.deg.toFixed(0) + '°' : ' - ILERI ' + s.len.toFixed(0) + ' cm') + ' (%' + s.sp + ')';
    if (s.type === 'turn') await execDrive(s.deg > 0 ? 'left' : 'right', Math.abs(s.deg), s.sp, f => { robotPose.h = s.h0 + s.deg * f; });
    else await execDrive('fwd', s.len, s.sp, f => { robotPose.x = s.from.x + (s.to.x - s.from.x) * f; robotPose.y = s.from.y + (s.to.y - s.from.y) * f; robotPose.h = s.h; });
    await waitMs(200);
  }
  const done = motionRun; motionRun = false; pathRunning = false;
  sendTank(0, 0);
  $('pathInfo').textContent = done ? '✓ Yol tamamlandi.' : '■ Yol DURDURULDU.';
  if (done && $('poseFollow').checked){ pathPts = []; pathPlan = []; setStartPose(robotPose, true); }
}
function abortPath(){ abortMotion(); }
function planToBlocks(){
  const out = []; let cur = 100;
  for (const s of pathPlan){
    if (s.sp !== cur){ out.push({ t: 'speed', v: s.sp }); cur = s.sp; }
    if (s.type === 'turn') out.push({ t: s.deg > 0 ? 'left' : 'right', v: Math.round(Math.abs(s.deg)) });
    else {
      const last = out[out.length - 1];
      if (last && last.t === 'fwd') last.v = Math.round((last.v + s.len) * 10) / 10;
      else out.push({ t: 'fwd', v: Math.round(s.len * 10) / 10 });
    }
  }
  if (cur !== 100) out.push({ t: 'speed', v: 100 });
  return out;
}
$('pathToBlocks').addEventListener('click', () => {
  if (!pathPlan.length){ toast('Once bir yol cizin'); return; }
  PROG = planToBlocks(); saveProg(); renderProg(); showPanel('moves');
  toast('Yol ' + PROG.length + ' bloga donusturuldu');
});
$('pathGo').addEventListener('click', runPath);
$('pathStop').addEventListener('click', abortPath);
$('pathUndo').addEventListener('click', () => { if (motionRun) return; if (pathPts.length > 2) pathPts.pop(); else pathPts = []; buildPlan(false); });
$('pathClear').addEventListener('click', () => { if (motionRun) return; pathPts = []; pathPlan = []; ruler.a = ruler.b = null; $('rulerInfo').textContent = ''; updPathInfo(); });

// ---------- SARI SINIRLI YOLLAR ----------
// Kesikli cizgiler yol yapildi: kasisten saga donup ortadaki dikey yoldan sehirler bolgesine cikilir.
const ROADS = [
  { t:'rect', x:7,   y:0,   w:336, h:146 },   // sehirler bolgesi
  { t:'rect', x:145, y:140, w:60,  h:137 },   // dikey yol (orta kesikli cizgi)
  { t:'rect', x:60,  y:217, w:230, h:60  },   // yatay yol (kasis hizasindaki kesikli cizgi)
  { t:'lane', x:12,  y:205, w:85,  h:280, r:38 },   // A seridi
  { t:'lane', x:253, y:205, w:85,  h:280, r:38 },   // B seridi
  { t:'rect', x:24.5,  y:478, w:60, h:72 },   // A baslangic
  { t:'rect', x:265.5, y:478, w:60, h:72 }    // B baslangic
];
let limitOn = lsGet('rl_limit', '1') === '1';
function inRoad(x, y){
  for (const r of ROADS){
    if (x < r.x || x > r.x + r.w || y < r.y || y > r.y + r.h) continue;
    if (r.t === 'rect') return true;
    if (y >= r.y + r.r) return true;
    const cx = x < r.x + r.r ? r.x + r.r : (x > r.x + r.w - r.r ? r.x + r.w - r.r : x);
    if (Math.hypot(x - cx, y - (r.y + r.r)) <= r.r || (x >= r.x + r.r && x <= r.x + r.w - r.r)) return true;
  }
  return false;
}
$('chkMode').value = lsGet('rl_chk', 'body'); $('chkTol').value = lsGet('rl_tol', '0');
$('chkMode').addEventListener('change', function(){ lsSet('rl_chk', this.value); updPathInfo(); });
$('chkTol').addEventListener('change', function(){ lsSet('rl_tol', this.value); updPathInfo(); });
// kontrol yaricapi: 'Sadece cizgi' = 0, 'Robot govdesi' = yarim genislik - tolerans
// Sari cizgiye DEGMEMESI icin govde yaricapi + guvenlik payi
const robR = () => $('chkMode').value === 'line' ? 0 : ((+$('robW').value || 45) / 2 + (+$('chkTol').value || 0));
const routeR = () => Math.max(robR(), 3);
function footOK(x, y, rr){
  if (!inRoad(x, y)) return false;
  for (let i=0; i<12; i++){ const a = i * Math.PI / 6; if (!inRoad(x + rr*Math.cos(a), y + rr*Math.sin(a))) return false; }
  return true;
}
function segOK(a, b, rr){
  const n = Math.max(1, Math.ceil(Math.hypot(b.x - a.x, b.y - a.y) / 2));
  for (let i=0; i<=n; i++){ const t = i / n; if (!footOK(a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t, rr)) return false; }
  return true;
}
function pathBadSegs(){
  const bad = [];
  if (!limitOn) return bad;
  const rr = robR();
  for (let i=1; i<pathPts.length; i++) if (!segOK(pathPts[i-1], pathPts[i], rr)) bad.push(i);
  return bad;
}
// Otomatik rota (A*): 5 cm izgara, robot genisligi kadar bosluk birakir
function autoRoute(from, to){
  const G = 5, nx = Math.ceil(FW / G), ny = Math.ceil(FH / G), rr = routeR();
  const free = new Uint8Array(nx * ny);
  for (let j=0; j<ny; j++) for (let i=0; i<nx; i++) free[j*nx + i] = footOK(i*G + G/2, j*G + G/2, rr) ? 1 : 0;
  const cell = p => [clamp(Math.floor(p.x / G), 0, nx-1), clamp(Math.floor(p.y / G), 0, ny-1)];
  const nearestFree = c => {
    if (free[c[1]*nx + c[0]]) return c;
    let best = null, bd = 1e9;
    for (let j=0; j<ny; j++) for (let i=0; i<nx; i++) if (free[j*nx + i]){ const d = (i-c[0])**2 + (j-c[1])**2; if (d < bd){ bd = d; best = [i, j]; } }
    return best;
  };
  const s = nearestFree(cell(from)), g = nearestFree(cell(to));
  if (!s || !g) return null;
  const idx = c => c[1]*nx + c[0];
  const gs = new Float32Array(nx * ny).fill(1e9), prev = new Int32Array(nx * ny).fill(-1), closed = new Uint8Array(nx * ny);
  const open = [[0, idx(s)]]; gs[idx(s)] = 0;
  const hfn = k => Math.hypot(k % nx - g[0], Math.floor(k / nx) - g[1]);
  const D = [[1,0,1],[-1,0,1],[0,1,1],[0,-1,1],[1,1,1.414],[1,-1,1.414],[-1,1,1.414],[-1,-1,1.414]];
  while (open.length){
    let bi = 0; for (let k=1; k<open.length; k++) if (open[k][0] < open[bi][0]) bi = k;
    const cur = open.splice(bi, 1)[0][1];
    if (closed[cur]) continue; closed[cur] = 1;
    if (cur === idx(g)) break;
    const cx = cur % nx, cy = Math.floor(cur / nx);
    for (const d of D){
      const x = cx + d[0], y = cy + d[1];
      if (x < 0 || y < 0 || x >= nx || y >= ny) continue;
      const k = y*nx + x; if (!free[k] || closed[k]) continue;
      if (d[0] && d[1] && (!free[cy*nx + x] || !free[y*nx + cx])) continue;   // kose kesme yok
      const ng = gs[cur] + d[2];
      if (ng < gs[k]){ gs[k] = ng; prev[k] = cur; open.push([ng + hfn(k), k]); }
    }
  }
  if (prev[idx(g)] < 0 && idx(g) !== idx(s)) return null;
  const raw = [];
  for (let k = idx(g); k >= 0; k = prev[k]){ raw.push({ x: (k % nx)*G + G/2, y: Math.floor(k / nx)*G + G/2 }); if (k === idx(s)) break; }
  raw.reverse();
  raw[0] = { x: from.x, y: from.y };
  if (footOK(to.x, to.y, rr) && segOK(raw[raw.length-1], to, rr)) raw.push({ x: to.x, y: to.y });
  // gorus hatti ile sadelestir
  const out = [raw[0]]; let i = 0;
  while (i < raw.length - 1){
    let j = raw.length - 1;
    while (j > i + 1 && !segOK(raw[i], raw[j], rr)) j--;
    out.push(raw[j]); i = j;
  }
  return out;
}
function fixPath(){
  if (pathPts.length < 2){ toast('Once bir yol cizin'); return; }
  const bad = new Set(pathBadSegs());
  if (!bad.size){ toast('Yol zaten sari sinirlarin icinde'); return; }
  const out = [pathPts[0]]; let fail = 0;
  for (let i = 1; i < pathPts.length; i++){
    if (!bad.has(i)){ out.push(pathPts[i]); continue; }
    const r = autoRoute(out[out.length - 1], pathPts[i]);
    if (r) out.push(...r.slice(1)); else { out.push(pathPts[i]); fail++; }
  }
  pathPts = out; buildPlan(false);
  toast(fail ? 'Bazi parcalar duzeltilemedi (hedef yolun disinda olabilir)' : 'Yol sari yollara gore duzeltildi');
}
$('pathFix').addEventListener('click', fixPath);
function setLimit(on){
  limitOn = on; lsSet('rl_limit', on ? '1' : '0');
  $('limitBtn').classList.toggle('on', on);
  $('limitBtn').textContent = on ? '🟨 SARI SINIR: ACIK' : '🚫 SARI YASAK KALDIRILDI';
  $('limitInfo').textContent = on ? '⛔ Sari cizgilerin disina cikmak YASAK. Robot sadece sari cerceveli yollarda gidebilir.'
                                  : '⚠ Sari sinir kapali: yol kontrol edilmiyor. Yarismada sari cizgilerin disina cikmak yasaktir!';
  updPathInfo();
}
$('limitBtn').addEventListener('click', () => setLimit(!limitOn));

function roadPath(ctx, m, r){
  const o = fS(m, r.x, r.y), w = r.w*m.sc, h = r.h*m.sc;
  ctx.beginPath();
  if (r.t === 'rect'){ ctx.rect(o[0], o[1], w, h); return; }
  const rr = r.r*m.sc;
  ctx.moveTo(o[0], o[1] + h); ctx.lineTo(o[0], o[1] + rr); ctx.arcTo(o[0], o[1], o[0] + rr, o[1], rr);
  ctx.lineTo(o[0] + w - rr, o[1]); ctx.arcTo(o[0] + w, o[1], o[0] + w, o[1] + rr, rr); ctx.lineTo(o[0] + w, o[1] + h); ctx.closePath();
}
function drawField(ctx, m){
  const R = (x, y, w, h) => { const a = fS(m, x, y); return [a[0], a[1], w*m.sc, h*m.sc]; };
  ctx.fillStyle = '#e8dcc4'; ctx.fillRect(...R(0, 0, FW, FH));
  ctx.fillStyle = '#6b7280'; ctx.fillRect(...R(7, 146, 336, 404));
  // yasak bolge taramasi
  if (limitOn){
    ctx.save(); const a0 = fS(m, 7, 146); ctx.beginPath(); ctx.rect(a0[0], a0[1], 336*m.sc, 404*m.sc); ctx.clip();
    ctx.strokeStyle = 'rgba(239,68,68,.28)'; ctx.lineWidth = 2;
    const st = Math.max(10, 12*m.sc);
    for (let d = -FH*m.sc; d < FW*m.sc + FH*m.sc; d += st){ ctx.beginPath(); ctx.moveTo(m.ox + d, m.oy); ctx.lineTo(m.ox + d - FH*m.sc, m.oy + FH*m.sc); ctx.stroke(); }
    ctx.restore();
  }
  // sari cerceveli yollar: once kalin sari kontur, sonra dolgu -> sadece dis sinir kalir
  const lw = clamp(3.5*m.sc, 3, 12);
  ctx.lineWidth = lw * 2; ctx.strokeStyle = '#facc15'; ctx.lineJoin = 'miter';
  for (const r of ROADS){ roadPath(ctx, m, r); ctx.stroke(); }
  ctx.fillStyle = '#d1d5db';
  for (const r of ROADS){ roadPath(ctx, m, r); ctx.fill(); }
  ctx.fillStyle = '#f8fafc'; ctx.fillRect(...R(CITY.x, CITY.y, CITY.w, CITY.h));
  ctx.strokeStyle = 'rgba(100,116,139,.25)'; ctx.lineWidth = 1;
  const gStep = m.sc > 6 ? 5 : 25;
  for (let x=0; x<=FW; x+=gStep){ const a = fS(m, x, 0), b = fS(m, x, FH); ctx.strokeStyle = x % 25 ? 'rgba(100,116,139,.12)' : 'rgba(100,116,139,.28)'; ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke(); }
  for (let y=0; y<=FH; y+=gStep){ const a = fS(m, 0, y), b = fS(m, FW, y); ctx.strokeStyle = y % 25 ? 'rgba(100,116,139,.12)' : 'rgba(100,116,139,.28)'; ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke(); }
  for (const c of CITIES){
    const TT = TARGETS.find(t2 => t2.n === c.n), sel = TT && deliverTarget === TT.id;
    ctx.fillStyle = sel ? '#16a34a' : '#2563eb'; ctx.fillRect(...R(c.x, c.y, 12, 12));
    if (sel){ ctx.strokeStyle = '#16a34a'; ctx.lineWidth = 2; ctx.strokeRect(...R(c.x - 4, c.y - 4, 20, 20)); }
    const p = fS(m, c.x + 6, c.y + 26); ctx.fillStyle = sel ? '#15803d' : '#0f172a'; ctx.font = 'bold ' + clamp(11*m.sc*1.8, 10, 34) + 'px system-ui'; ctx.textAlign = 'center'; ctx.fillText(c.n, p[0], p[1]);
  }
  ctx.strokeStyle = '#0f172a'; ctx.lineWidth = 3; ctx.setLineDash([10, 8]);
  let a = fS(m, 175, 146), b = fS(m, 175, 247); ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke();
  a = fS(m, 47, 247); b = fS(m, 303, 247); ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke();
  ctx.setLineDash([]);
  ctx.fillStyle = '#1f2937'; ctx.font = 'bold ' + clamp(12*m.sc*1.6, 10, 34) + 'px system-ui'; ctx.textAlign = 'left';
  let t = fS(m, 20, 170); ctx.fillText('A Bolgesi', t[0], t[1]);
  t = fS(m, 250, 162); ctx.fillText('C Bolgesi', t[0], t[1]);
  t = fS(m, 250, 180); ctx.fillText('B Bolgesi', t[0], t[1]);
  for (const L of LANES){
    ctx.fillStyle = '#f8fafc'; ctx.fillRect(...R(L.x, LANE_Y, LANE_W, LANE_H));
    const kx = L.x + (LANE_W - 75)/2;
    ctx.fillStyle = '#6b7280'; ctx.fillRect(...R(kx, 249, 75, 40));
    ctx.fillStyle = '#374151'; for (let i=0; i<3; i++) ctx.fillRect(...R(kx + 8 + i*22, 249, 10, 40));
    t = fS(m, kx + 37.5, 272); ctx.fillStyle = '#f8fafc'; ctx.font = 'bold ' + clamp(9*m.sc*1.6, 9, 34) + 'px system-ui'; ctx.textAlign = 'center'; ctx.fillText('KASIS', t[0], t[1]);
    ctx.fillStyle = '#111827';
    ctx.fillRect(...R(L.x - 2, LANE_Y - 2, LANE_W + 4, 4)); ctx.fillRect(...R(L.x - 2, LANE_Y + 100, LANE_W + 4, 4));
    ctx.fillRect(...R(L.x - 4, LANE_Y - 2, 4, 110)); ctx.fillRect(...R(L.x + LANE_W, LANE_Y - 2, 4, 110));
    ctx.strokeStyle = '#22c55e'; ctx.lineWidth = clamp(3*m.sc*1.5, 3, 34);
    a = fS(m, L.x + LANE_W/2, LANE_Y); b = fS(m, L.x + LANE_W/2, LANE_Y + 100); ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke();
    t = fS(m, L.x + LANE_W/2, LANE_Y + 150); ctx.fillStyle = '#334155'; ctx.font = clamp(9*m.sc*1.6, 9, 34) + 'px system-ui'; ctx.fillText('Zipline alani', t[0], t[1]);
    ctx.fillStyle = '#f8fafc'; ctx.fillRect(...R(L.dagX, 443, 40, 40)); ctx.strokeStyle = '#111827'; ctx.lineWidth = 2; ctx.strokeRect(...R(L.dagX, 443, 40, 40));
    ctx.fillStyle = '#3b82f6'; for (let i=0; i<3; i++) for (let j=0; j<3; j++) ctx.fillRect(...R(L.dagX + 4 + i*12.5, 447 + j*12.5, 9, 9));
    t = fS(m, L.dagX + 20, 496); ctx.fillStyle = '#111827'; ctx.font = clamp(8*m.sc*1.6, 8, 34) + 'px system-ui'; ctx.fillText('Dagitim ' + L.id, t[0], t[1]);
    const sx = STARTS[L.id].x - 30;
    ctx.fillStyle = '#f8fafc'; ctx.fillRect(...R(sx, 490, 60, 60)); ctx.strokeStyle = '#111827'; ctx.lineWidth = 3; ctx.strokeRect(...R(sx, 490, 60, 60));
    t = fS(m, sx + 30, 530); ctx.fillStyle = '#111827'; ctx.font = 'bold ' + clamp(26*m.sc*1.4, 14, 34) + 'px system-ui'; ctx.fillText(L.id, t[0], t[1]);
  }
  // yon oklari (kasisten saga don -> orta yol -> sehirler)
  ctx.fillStyle = 'rgba(15,23,42,.55)'; ctx.font = 'bold ' + clamp(14*m.sc*1.4, 12, 34) + 'px system-ui'; ctx.textAlign = 'center';
  for (const [x, y, ch] of [[118, 240, '➜'], [232, 240, '⬅'], [175, 195, '⬆']]){ const q = fS(m, x, y); ctx.fillText(ch, q[0], q[1]); }
}
function drawPath(){
  const { ctx, w, h } = fit(pathCanvas);
  ctx.clearRect(0, 0, w, h);
  const m = fieldMap(w, h);
  drawField(ctx, m);
  const drawZone = (z, draft) => {
    const a = fS(m, z.x, z.y), w2 = z.w*m.sc, h2 = z.h*m.sc;
    ctx.fillStyle = draft ? 'rgba(251,146,60,.2)' : 'rgba(251,146,60,.28)'; ctx.fillRect(a[0], a[1], w2, h2);
    ctx.setLineDash([8, 6]); ctx.strokeStyle = '#ea580c'; ctx.lineWidth = 2; ctx.strokeRect(a[0], a[1], w2, h2); ctx.setLineDash([]);
    ctx.fillStyle = '#7c2d12'; ctx.font = 'bold ' + clamp(10*m.sc*1.6, 11, 26) + 'px system-ui'; ctx.textAlign = 'center';
    ctx.fillText((z.kasis ? 'KASIS %' : '🐢 %') + z.sp, a[0] + w2/2, a[1] + h2/2 + 5);
  };
  allZones().forEach(z => drawZone(z, !!z.kasis));
  if (zoneDraft) drawZone({ x: Math.min(zoneDraft.x0, zoneDraft.x1), y: Math.min(zoneDraft.y0, zoneDraft.y1), w: Math.abs(zoneDraft.x1 - zoneDraft.x0), h: Math.abs(zoneDraft.y1 - zoneDraft.y0), sp: clamp(+$('slowSpd').value || 40, 5, 100) }, true);
  if (pathPts.length > 1){
    const badSet = new Set(pathDrawing ? [] : pathBadSegs());
    ctx.lineJoin = 'round'; ctx.lineCap = 'round';
    for (let i=1; i<pathPts.length; i++){
      const a = fS(m, pathPts[i-1].x, pathPts[i-1].y), b = fS(m, pathPts[i].x, pathPts[i].y), bad = badSet.has(i);
      ctx.shadowColor = bad ? '#ef4444' : '#e879f9'; ctx.shadowBlur = 10;
      ctx.strokeStyle = bad ? '#dc2626' : '#c026d3'; ctx.lineWidth = bad ? 6 : 4;
      ctx.beginPath(); ctx.moveTo(a[0], a[1]); ctx.lineTo(b[0], b[1]); ctx.stroke();
    }
    ctx.shadowBlur = 0;
    // robot genisligi seridi (ne kadar yer kapladigi)
    ctx.strokeStyle = 'rgba(34,211,238,.18)'; ctx.lineWidth = (+$('robW').value || 45) * m.sc;
    ctx.beginPath(); pathPts.forEach((p, i) => { const s = fS(m, p.x, p.y); if (i === 0) ctx.moveTo(s[0], s[1]); else ctx.lineTo(s[0], s[1]); }); ctx.stroke();
    if (!pathDrawing) pathPts.forEach((p, i) => { const s = fS(m, p.x, p.y); ctx.fillStyle = i === pathPts.length - 1 ? '#16a34a' : '#a21caf'; ctx.beginPath(); ctx.arc(s[0], s[1], 5, 0, Math.PI*2); ctx.fill(); });
  }
  if (ruler.a){
    const A = fS(m, ruler.a.x, ruler.a.y), B = ruler.b ? fS(m, ruler.b.x, ruler.b.y) : null;
    drawRuler2D(ctx, A, B, ruler.b ? Math.hypot(ruler.b.x - ruler.a.x, ruler.b.y - ruler.a.y).toFixed(1) + ' cm' : '');
  }
  const rp = fS(m, robotPose.x, robotPose.y), sz = (+$('robW').value || 45) * m.sc;
  ctx.save(); ctx.translate(rp[0], rp[1]); ctx.rotate(-(robotPose.h - 90) * RAD);
  ctx.shadowColor = '#22d3ee'; ctx.shadowBlur = motionRun ? 20 : (pathTool === 'robot' ? 16 : 8);
  ctx.fillStyle = 'rgba(14,116,144,.9)'; ctx.strokeStyle = '#22d3ee'; ctx.lineWidth = 2;
  ctx.fillRect(-sz/2, -sz/2, sz, sz); ctx.strokeRect(-sz/2, -sz/2, sz, sz);
  ctx.shadowBlur = 0; ctx.fillStyle = '#1f2937';
  ctx.fillRect(-sz/2 - 3, -sz/2 + 2, 4, sz*0.35); ctx.fillRect(sz/2 - 1, -sz/2 + 2, 4, sz*0.35);
  ctx.fillRect(-sz/2 - 3, sz/2 - 2 - sz*0.35, 4, sz*0.35); ctx.fillRect(sz/2 - 1, sz/2 - 2 - sz*0.35, 4, sz*0.35);
  ctx.fillStyle = '#e0f2fe'; ctx.beginPath(); ctx.moveTo(0, -sz/2 - 7); ctx.lineTo(-7, -sz/2 + 5); ctx.lineTo(7, -sz/2 + 5); ctx.closePath(); ctx.fill();
  ctx.restore();
  // olcek cubugu
  let Lc = [1, 2, 5, 10, 25, 50, 100].find(v => v * m.sc >= 60) || 100;
  const bx = 14, by = h - 16, bw = Lc * m.sc;
  ctx.fillStyle = 'rgba(15,23,42,.85)'; ctx.fillRect(bx - 6, by - 22, bw + 12, 30);
  ctx.strokeStyle = '#fde68a'; ctx.lineWidth = 3; ctx.beginPath(); ctx.moveTo(bx, by); ctx.lineTo(bx + bw, by); ctx.moveTo(bx, by - 5); ctx.lineTo(bx, by + 3); ctx.moveTo(bx + bw, by - 5); ctx.lineTo(bx + bw, by + 3); ctx.stroke();
  ctx.fillStyle = '#fde68a'; ctx.font = 'bold 12px system-ui'; ctx.textAlign = 'center'; ctx.fillText(Lc + ' cm', bx + bw/2, by - 8);
  $('zoomTxt').textContent = '%' + Math.round(PZ.k * 100);
}

// =====================================================
// HAREKETLER: KOD BLOKLARI
// =====================================================
const BLOCKS = {
  fwd:    { c: 'c-drive', ic: '⬆', name: 'Ileri git',     unit: 'cm', def: 100 },
  back:   { c: 'c-drive', ic: '⬇', name: 'Geri git',      unit: 'cm', def: 50 },
  left:   { c: 'c-turn',  ic: '⟲', name: 'Sola don',      unit: '°',  def: 90 },
  right:  { c: 'c-turn',  ic: '⟳', name: 'Saga don',      unit: '°',  def: 90 },
  speed:  { c: 'c-speed', ic: '⚡', name: 'Hiz',           unit: '%',  def: 60 },
  wait:   { c: 'c-wait',  ic: '⏱', name: 'Bekle',         unit: 'sn', def: 1 },
  lin:    { c: 'c-lin',   ic: '↕', name: 'Lineer kanca',  unit: 'cm', def: 90 },
  servo:  { c: 'c-arm',   ic: '🦾', name: 'Servo',         unit: '°',  def: 90 },
  pose:   { c: 'c-arm',   ic: '📌', name: 'Poz' },
  gopen:  { c: 'c-arm',   ic: '✋', name: 'Gripper ac' },
  gclose: { c: 'c-arm',   ic: '✊', name: 'Gripper kapat' }
};
// Otonom hedefler (saha uzerindeki noktalar, cm). Robot sari yollardan en yakin ulasilabilir noktaya gider.
// stop: hedefe kac cm kala duracak (koliyi kolla koyabilmek icin)
const TARGETS = [
  { id: 'sinop',  n: 'Sinop',  x: 173,   y: 16,  stop: 30, city: true },
  { id: 'nigde',  n: 'Nigde',  x: 93,    y: 46,  stop: 30, city: true },
  { id: 'tokat',  n: 'Tokat',  x: 257,   y: 41,  stop: 30, city: true },
  { id: 'aydin',  n: 'Aydin',  x: 53,    y: 105, stop: 30, city: true },
  { id: 'elazig', n: 'Elazig', x: 297,   y: 102, stop: 30, city: true },
  { id: 'dagA',   n: 'Dagitim A', x: 120, y: 463, stop: 25 },
  { id: 'dagB',   n: 'Dagitim B', x: 230, y: 463, stop: 25 },
  { id: 'zipA',   n: 'Zipline A', x: 54.5,  y: 300, stop: 0 },
  { id: 'zipB',   n: 'Zipline B', x: 295.5, y: 300, stop: 0 },
  { id: 'stA',    n: 'Baslangic A', x: 54.5,  y: 520, stop: 0 },
  { id: 'stB',    n: 'Baslangic B', x: 295.5, y: 520, stop: 0 }
];
// Hedefin durma noktasi: hedefe stop cm kala, gelis yonune gore
function targetStopPoint(T, from){
  if (!T.stop) return { x: T.x, y: T.y };
  const dx = T.x - from.x, dy = T.y - from.y, d = Math.hypot(dx, dy);
  if (d < T.stop + 1) return { x: from.x, y: from.y };
  const k = (d - T.stop) / d;
  return { x: from.x + dx * k, y: from.y + dy * k };
}
BLOCKS.goto = { c: 'c-goto', ic: '🧭', name: 'Hedefe git' };
const PALETTE = [
  ['ILERI / GERI', [ {t:'fwd', v:50, lbl:'50 cm ileri'}, {t:'fwd', v:100, lbl:'1 m ileri'}, {t:'fwd', v:200, lbl:'2 m ileri'}, {t:'fwd', v:300, lbl:'3 m ileri'},
                     {t:'back', v:50, lbl:'50 cm geri'}, {t:'back', v:100, lbl:'1 m geri'}, {t:'back', v:200, lbl:'2 m geri'} ]],
  ['SAGA / SOLA', [ {t:'left', v:90, lbl:'Sola 90°'}, {t:'right', v:90, lbl:'Saga 90°'}, {t:'left', v:45, lbl:'Sola 45°'}, {t:'right', v:45, lbl:'Saga 45°'},
                    {t:'left', v:180, lbl:'Geri don 180°'} ]]
];
// Kullanicinin olusturdugu bloklar
let MYBLOCKS = [];
try{ MYBLOCKS = JSON.parse(lsGet('rl_myblocks', '[]')) || []; }catch(_){ MYBLOCKS = []; }
function blkLabel(t, v){
  if (t === 'fwd' || t === 'back'){ const s = v >= 100 && v % 100 === 0 ? (v/100) + ' m' : (v >= 100 ? (v/100).toFixed(2).replace(/0+$/,'').replace(/\.$/,'') + ' m' : v + ' cm'); return s + (t === 'fwd' ? ' ileri' : ' geri'); }
  return (t === 'left' ? 'Sola ' : 'Saga ') + v + '°';
}
let PROG = [];
try{ PROG = JSON.parse(lsGet('rl_prog', '[]')) || []; }catch(_){ PROG = []; }
let PROGS = {};
try{ PROGS = JSON.parse(lsGet('rl_progs', '{}')) || {}; }catch(_){ PROGS = {}; }
function saveProg(){ lsSet('rl_prog', JSON.stringify(PROG)); updProgInfo(); }
const cloneBlk = b => { const o = { t: b.t }; if ('v' in b) o.v = b.low ? h0() : b.v; if ('ch' in b) o.ch = b.ch; if ('p' in b) o.p = b.p; return o; };

function blockText(b){
  const B = BLOCKS[b.t];
  if (b.t === 'servo') return B.ic + ' Servo CH' + b.ch + ' → ' + b.v + '°';
  if (b.t === 'pose') return B.ic + ' Poz: ' + String(b.p).toUpperCase();
  if (b.t === 'goto'){ const T = TARGETS.find(x => x.id === b.p); return B.ic + ' Git: ' + (T ? T.n : b.p); }
  if ('v' in b) return B.ic + ' ' + B.name + ' ' + b.v + ' ' + B.unit;
  return B.ic + ' ' + B.name;
}
function blockSec(b, sp){
  const k = sp / 100;
  if (b.t === 'fwd' || b.t === 'back') return Math.abs(+b.v) / (calCm() * k);
  if (b.t === 'left' || b.t === 'right') return Math.abs(+b.v) / (calDeg() * k);
  if (b.t === 'wait') return +b.v;
  if (b.t === 'lin') return 3;
  if (b.t === 'goto') return 8;
  return 1.5;
}
function updProgInfo(){
  if (progRunning) return;
  let sp = 100, sec = 0, dist = 0;
  PROG.forEach(b => { if (b.t === 'speed') sp = clamp(+b.v, 5, 100); else sec += blockSec(b, sp); if (b.t === 'fwd' || b.t === 'back') dist += Math.abs(+b.v); });
  const way = sensorWanted() ? 'SENSORLE olculecek' : 'SUREYE gore surulecek';
  $('progInfo').textContent = PROG.length ? (PROG.length + ' blok, toplam surus ' + (dist/100).toFixed(2) + ' m, tahmini sure ' + sec.toFixed(1) + ' sn — ' + way) : 'Program bos.';
}

function mkBlockEl(b, opts){
  const B = BLOCKS[b.t], el = document.createElement('div');
  el.className = 'blk ' + B.c;
  if (opts.palette){
    el.textContent = b.lbl ? (B.ic + ' ' + b.lbl) : blockText(b);
    if (opts.del){ const x = document.createElement('span'); x.className = 'bx'; x.textContent = '✕'; el.appendChild(x); }
    return el;
  }
  const idx = opts.idx;
  let html = '<span class="hd">⠿</span><span>' + B.ic + ' ' + B.name + '</span>';
  if (b.t === 'servo'){
    html += ' CH <select data-f="ch">' + [4,5,6,7].map(c => '<option value="' + c + '"' + (+b.ch === c ? ' selected' : '') + '>' + c + '</option>').join('') + '</select>';
  }
  if (b.t === 'goto'){
    html += ' <select data-f="p" style="width:130px">' + TARGETS.map(T => '<option value="' + T.id + '"' + (b.p === T.id ? ' selected' : '') + '>' + T.n + '</option>').join('') + '</select>';
  }
  if (b.t === 'pose'){
    html += ' <select data-f="p">' + ['al','tasi','birak'].map(p => '<option value="' + p + '"' + (b.p === p ? ' selected' : '') + '>' + p.toUpperCase() + '</option>').join('') + '</select>';
  }
  if ('v' in b) html += ' <input type="number" data-f="v" step="any" value="' + b.v + '"> ' + B.unit;
  html += '<span class="bt"><button data-a="del">✕</button></span>';
  el.innerHTML = html;
  el.querySelectorAll('[data-f]').forEach(inp => inp.addEventListener('change', function(){
    const f = this.dataset.f; b[f] = (f === 'p') ? this.value : +this.value; saveProg();
  }));
  el.querySelectorAll('[data-a]').forEach(btn => btn.addEventListener('click', e => {
    e.stopPropagation(); if (progRunning) return;
    const a = btn.dataset.a;
    if (a === 'del') PROG.splice(idx, 1);
    else if (a === 'up' && idx > 0) [PROG[idx-1], PROG[idx]] = [PROG[idx], PROG[idx-1]];
    else if (a === 'dn' && idx < PROG.length - 1) [PROG[idx+1], PROG[idx]] = [PROG[idx], PROG[idx+1]];
    saveProg(); renderProg();
  }));
  return el;
}
function renderPalette(){
  const pal = $('palette'); pal.innerHTML = '';
  const groups = PALETTE.slice();
  if (MYBLOCKS.length) groups.push(['BENIM BLOKLARIM', MYBLOCKS]);
  groups.forEach(([g, items]) => {
    const h = document.createElement('div'); h.className = 'pgrp'; h.textContent = g; pal.appendChild(h);
    const mine = items === MYBLOCKS;
    items.forEach((b, i) => {
      const el = mkBlockEl(b, { palette: true, del: mine });
      el.addEventListener('pointerdown', e => {
        if (mine && e.target.classList.contains('bx')){ e.preventDefault(); MYBLOCKS.splice(i, 1); lsSet('rl_myblocks', JSON.stringify(MYBLOCKS)); renderPalette(); return; }
        startDrag(e, b, -1, el);
      });
      pal.appendChild(el);
    });
  });
}
function nbUpd(){ const turn = $('nbType').value === 'left' || $('nbType').value === 'right'; $('nbUnit').style.display = turn ? 'none' : ''; $('nbDeg').style.display = turn ? '' : 'none'; }
$('nbType').addEventListener('change', () => { nbUpd(); $('nbVal').value = ($('nbType').value === 'left' || $('nbType').value === 'right') ? 90 : 1; });
$('nbAdd').addEventListener('click', () => {
  const t = $('nbType').value, turn = t === 'left' || t === 'right';
  let v = parseFloat($('nbVal').value);
  if (!(v > 0)){ toast('Pozitif bir deger yazin'); return; }
  if (!turn && $('nbUnit').value === 'm') v = v * 100;
  v = Math.round(v * 10) / 10;
  MYBLOCKS.push({ t, v, lbl: blkLabel(t, v) }); lsSet('rl_myblocks', JSON.stringify(MYBLOCKS)); renderPalette();
  toast('"' + blkLabel(t, v) + '" blogu olusturuldu');
});
nbUpd();
function renderProg(){
  const pr = $('prog'); pr.innerHTML = '';
  if (!PROG.length){ const e = document.createElement('div'); e.className = 'empty'; e.id = 'progEmpty'; e.textContent = 'Bloklari buraya surukleyin ya da soldaki bloklara dokunun'; pr.appendChild(e); }
  PROG.forEach((b, i) => {
    const el = mkBlockEl(b, { idx: i });
    el.addEventListener('pointerdown', e => { if (e.target.closest('input,select,button')) return; startDrag(e, b, i, el); });
    pr.appendChild(el);
  });
  updProgInfo();
}

// Surukle-birak (fare + dokunmatik)
let drag = null;
function startDrag(e, item, fromIdx, el){
  if (progRunning) return;
  e.preventDefault();
  drag = { item, fromIdx, el, sx: e.clientX, sy: e.clientY, moved: false, ghost: null, ph: null, idx: -1 };
  window.addEventListener('pointermove', onDragMove);
  window.addEventListener('pointerup', onDragEnd);
  window.addEventListener('pointercancel', onDragEnd);
}
function onDragMove(e){
  if (!drag) return;
  if (!drag.moved && Math.hypot(e.clientX - drag.sx, e.clientY - drag.sy) > 6){
    drag.moved = true;
    drag.ghost = drag.el.cloneNode(true); drag.ghost.classList.add('ghost'); document.body.appendChild(drag.ghost);
    if (drag.fromIdx >= 0) drag.el.style.opacity = '.3';
  }
  if (!drag.moved) return;
  drag.ghost.style.left = (e.clientX - 24) + 'px'; drag.ghost.style.top = (e.clientY - 18) + 'px';
  const pr = $('prog'), r = pr.getBoundingClientRect();
  const inside = e.clientX > r.left - 30 && e.clientX < r.right + 30 && e.clientY > r.top - 30 && e.clientY < r.bottom + 30;
  if (drag.ph){ drag.ph.remove(); drag.ph = null; }
  drag.idx = -1;
  if (!inside) return;
  const blks = [...pr.querySelectorAll('.blk')];
  let idx = blks.length;
  for (let i = 0; i < blks.length; i++){ const br = blks[i].getBoundingClientRect(); if (e.clientY < br.top + br.height / 2){ idx = i; break; } }
  drag.idx = idx;
  drag.ph = document.createElement('div'); drag.ph.className = 'ph';
  if (idx < blks.length) pr.insertBefore(drag.ph, blks[idx]); else pr.appendChild(drag.ph);
}
function onDragEnd(){
  window.removeEventListener('pointermove', onDragMove);
  window.removeEventListener('pointerup', onDragEnd);
  window.removeEventListener('pointercancel', onDragEnd);
  if (!drag) return;
  const d = drag; drag = null;
  if (d.ghost) d.ghost.remove();
  if (d.ph) d.ph.remove();
  if (d.el) d.el.style.opacity = '';
  if (!d.moved){
    if (d.fromIdx < 0){ PROG.push(cloneBlk(d.item)); saveProg(); renderProg(); }
    return;
  }
  if (d.idx < 0) return;
  let idx = d.idx;
  if (d.fromIdx >= 0){ const it = PROG.splice(d.fromIdx, 1)[0]; if (d.fromIdx < idx) idx--; PROG.splice(idx, 0, it); }
  else PROG.splice(idx, 0, cloneBlk(d.item));
  saveProg(); renderProg();
}

// Sahadaki robot konumunu surus sirasinda tahmin et (sureye gore)
function trackPose(kind, amount){
  const p0 = { ...robotPose };
  return f => {
    if (kind === 'fwd' || kind === 'back'){
      const d = (kind === 'fwd' ? 1 : -1) * amount * f;
      robotPose.x = p0.x + d * Math.cos(p0.h * RAD); robotPose.y = p0.y - d * Math.sin(p0.h * RAD); robotPose.h = p0.h;
    } else robotPose.h = p0.h + (kind === 'left' ? 1 : -1) * amount * f;
  };
}
async function gotoTarget(id, sp){
  const T = TARGETS.find(x => x.id === id);
  if (!T){ toast('Bilinmeyen hedef'); return; }
  const from = { x: robotPose.x, y: robotPose.y };
  const route = limitOn ? autoRoute(from, T) : [from, { x: T.x, y: T.y }];
  if (!route){ toast(T.n + ': sari yollardan ulasilamiyor'); return; }
  const plan = planFrom(route, robotPose.h);
  for (const s of plan){
    if (!motionRun) return;
    const k = Math.min(sp, s.sp);
    if (s.type === 'turn') await execDrive(s.deg > 0 ? 'left' : 'right', Math.abs(s.deg), k, trackPose(s.deg > 0 ? 'left' : 'right', Math.abs(s.deg)));
    else await execDrive('fwd', s.len, k, trackPose('fwd', s.len));
    await waitMs(150);
  }
}

// Program calistirma
async function runProg(){
  if (motionRun){ toast('Baska bir hareket calisiyor'); return; }
  if (!PROG.length){ toast('Once blok ekleyin'); return; }
  stopDrive(); motionRun = true; progRunning = true;
  robotPose = { ...startPose };
  const els = [...document.querySelectorAll('#prog .blk')];
  els.forEach(x => x.classList.remove('run', 'done'));
  let sp = 100;
  for (let i = 0; i < PROG.length && motionRun; i++){
    const b = PROG[i], el = els[i];
    if (el){ el.classList.add('run'); try{ el.scrollIntoView({ block: 'nearest', behavior: 'smooth' }); }catch(_){} }
    $('progInfo').textContent = '▶ ' + (i + 1) + ' / ' + PROG.length + ' : ' + blockText(b) + (b.t.match(/fwd|back|left|right/) ? ' (%' + sp + ', ' + (sensorWanted() ? 'sensorle' : 'sureye gore') + ')' : '');
    if (b.t === 'fwd' || b.t === 'back' || b.t === 'left' || b.t === 'right') await execDrive(b.t, +b.v, sp, trackPose(b.t, +b.v));
    else if (b.t === 'goto') await gotoTarget(b.p, sp);
    else if (b.t === 'speed') sp = clamp(+b.v, 5, 100);
    else if (b.t === 'wait') await waitMs((+b.v || 0) * 1000);
    else if (b.t === 'lin'){ gotoHeight(+b.v, true); await waitLinear(40000); }
    else if (b.t === 'servo'){ setServo(+b.ch, +b.v); await waitServosRun(8000); }
    else if (b.t === 'pose'){ goPose(b.p); await waitServosRun(8000); }
    else if (b.t === 'gopen'){ setGrip(0); await waitServosRun(6000); }
    else if (b.t === 'gclose'){ setGrip(100); await waitServosRun(6000); }
    if (el){ el.classList.remove('run'); el.classList.add('done'); }
    await waitMs(200);
  }
  const ok = motionRun;
  motionRun = false; progRunning = false; sendTank(0, 0);
  $('progInfo').textContent = ok ? '✓ Program tamamlandi' : '■ Program durduruldu';
  logEvent('Program ' + (ok ? 'tamamlandi' : 'durduruldu') + ' (' + PROG.length + ' blok)');
  if (ok && $('poseFollow').checked){ pathPts = []; pathPlan = []; setStartPose(robotPose, true); }
  setTimeout(() => { els.forEach(x => x.classList.remove('done')); updProgInfo(); }, 3000);
}
$('progRun').addEventListener('click', runProg);
$('progStop').addEventListener('click', () => { abortMotion(); stopLinear(); });
$('progClear').addEventListener('click', () => { if (progRunning) return; PROG = []; saveProg(); renderProg(); });
$('progFromPath').addEventListener('click', () => {
  if (!pathPlan.length){ toast('CIZ-GIT sekmesinde once bir yol cizin'); return; }
  PROG = planToBlocks(); saveProg(); renderProg(); toast('Yol ' + PROG.length + ' bloga donusturuldu');
});
function renderProgSel(){
  const sel = $('progSel'), keep = sel.value;
  sel.innerHTML = '<option value="">- sec -</option>' + Object.keys(PROGS).map(n => '<option value="' + n.replace(/"/g, '') + '">' + n.replace(/</g, '') + '</option>').join('');
  if (keep in PROGS) sel.value = keep;
}
$('progSave').addEventListener('click', () => {
  const n = ($('progName').value || $('progSel').value || '').trim();
  if (!n){ toast('Program icin bir isim yazin'); return; }
  PROGS[n] = JSON.parse(JSON.stringify(PROG)); lsSet('rl_progs', JSON.stringify(PROGS)); renderProgSel(); $('progSel').value = n;
  toast('"' + n + '" kaydedildi'); logEvent('Program kaydedildi: ' + n);
});
$('progSel').addEventListener('change', function(){
  if (!this.value || progRunning) return;
  PROG = JSON.parse(JSON.stringify(PROGS[this.value] || [])); $('progName').value = this.value; saveProg(); renderProg();
});
$('progDel').addEventListener('click', () => {
  const n = $('progSel').value; if (!n) return;
  delete PROGS[n]; lsSet('rl_progs', JSON.stringify(PROGS)); renderProgSel(); toast('"' + n + '" silindi');
});

// =====================================================
// NEYI YANLIS YAPTI? (hizli duzeltme)
// =====================================================
const FIX_HELP = {
  fwd:  'Ornek: "100 cm ileri git" dediniz, robot 50 cm gitti. Komut 100, Gercekte 50 yazip DUZELT\'e basin; duz hiz ayari otomatik duzelir.',
  turn: 'Ornek: "90° don" dediniz, robot 60° dondu. Komut 90, Gercekte 60 yazip DUZELT\'e basin; donus hizi ayari otomatik duzelir.',
  drift: 'Robot duz giderken kayiyorsa: 1) Arduino sensor koprusunu takin — jiroskop varsa robot kendini duzeltir. 2) Tekerlek basinclari ve motor baglantilari esit mi bakin. 3) Zemin kaygansa hizi dusurun.',
  line:  'Sari cizgiye cok yaklasiyorsa: YOL bolumundeki "Guvenlik payi" degerini artirin (ornegin 5 cm). Robot boyu dogru girildi mi kontrol edin. Sapma cok fazlaysa once kalibrasyonu tekrarlayin.',
  nomove: 'Hic hareket etmiyorsa: 1) Motor gucu acik mi? 2) SURUS sekmesinde WASD ile hareket ediyor mu? 3) SISTEM sekmesinde robot BAGLI gorunuyor mu? 4) Hareket sirasinda ACIL DURDUR\'a basilmis olabilir.'
};
function fixUpd(){
  const k = $('fixKind').value;
  const num = k === 'fwd' || k === 'turn';
  $('fixNum2').style.display = num ? '' : 'none';
  $('fixUnit').textContent = k === 'turn' ? 'derece' : 'cm';
  if (k === 'turn' && +$('fixCmd').value === 100) $('fixCmd').value = 90;
  if (k === 'fwd' && +$('fixCmd').value === 90) $('fixCmd').value = 100;
  $('fixHelp').innerHTML = FIX_HELP[k];
}
$('fixKind').addEventListener('change', fixUpd);
$('fixApply').addEventListener('click', () => {
  const k = $('fixKind').value, cmd = parseFloat($('fixCmd').value), real = parseFloat($('fixReal').value);
  if (!(cmd > 0) || !(real > 0)){ toast('Komut ve gercek degeri yazin'); return; }
  const ratio = real / cmd;
  if (ratio < 0.2 || ratio > 5){ toast('Degerler cok farkli, tekrar kontrol edin'); return; }
  if (k === 'turn'){
    const nd = calDeg() * ratio;
    $('calDeg').value = nd.toFixed(1); lsSet('rl_cal_deg', $('calDeg').value);
    $('fixHelp').innerHTML = '✓ Donus hizi <b>' + nd.toFixed(0) + ' °/sn</b> olarak duzeltildi (%' + ((ratio - 1) * 100).toFixed(0) + ' sapma). Tekrar deneyin.';
    logEvent('Donus duzeltildi: ' + nd.toFixed(0) + ' °/sn');
  } else {
    const nv = calCm() * ratio;
    $('calCm').value = nv.toFixed(2); lsSet('rl_cal_cm', $('calCm').value);
    $('fixHelp').innerHTML = '✓ Duz hiz <b>' + nv.toFixed(1) + ' cm/sn</b> olarak duzeltildi (%' + ((ratio - 1) * 100).toFixed(0) + ' sapma). Tekrar deneyin.';
    logEvent('Duz hiz duzeltildi: ' + nv.toFixed(1) + ' cm/sn');
  }
  updCalText(); updPathInfo(); updProgInfo(); $('fixReal').value = '';
});

// =====================================================
// SENSOR + OTOMATIK KALIBRASYON
//   Duz  : 10 / 20 / 30 cm git-gel, mesafe sensoru olcer
//   Donus: 90 / 180 / 360 derece, jiroskop olcer
// =====================================================
$('sensZero').addEventListener('click', () => { api('/sensor?cmd=zero').then(r => r.status === 404 ? 'Robot kodu eski (v8 gerekli)' : r.text()).then(toast).catch(()=>{}); });
$('sensCal').addEventListener('click', () => { api('/sensor?cmd=cal').then(r => r.status === 404 ? 'Robot kodu eski (v8 gerekli)' : r.text()).then(toast).catch(()=>{}); });

let calRows = [];
function calShow(){
  $('calTable').innerHTML = calRows.length
    ? '<table class="kb" style="margin-top:6px">' + calRows.map(r =>
        '<tr><td>' + r.k + '</td><td>komut ' + r.cmd + '</td><td><b>gercek ' + r.real + '</b></td><td>' + r.err + '</td></tr>').join('') + '</table>'
    : '';
}
$('calStop').addEventListener('click', () => { motionRun = false; api('/movestop').catch(()=>{}); sendTank(0, 0); $('calAutoInfo').textContent = '■ Test durduruldu'; });

// --- DUZ GITME: 10 / 20 / 30 cm ileri git, olc, geri gel ---
$('calAuto').addEventListener('click', async () => {
  if (motionRun){ toast('Baska bir hareket calisiyor'); return; }
  if (!sensorLive){ toast('Arduino sensor koprusu bagli degil'); return; }
  if (!(sensCm > 45)){ toast('Robotu duvara/levhaya bakacak sekilde en az 50 cm uzaga koyun'); return; }
  const info = $('calAutoInfo'), keep = $('useSensor').checked;
  calRows = []; calShow();
  info.innerHTML = '<b class="warn">⚠ ROBOT ILERI GIDIP GERI GELECEK. Onunu ve cevresini bosaltin.</b> 2 sn sonra basliyor...';
  await sleep(2000);
  stopDrive(); motionRun = true;
  $('useSensor').checked = false;                 // test SUREYE gore surulur, sensor sadece OLCER
  const ratios = [];
  try{
    for (const cm of [10, 20, 30]){
      if (!motionRun) break;
      info.innerHTML = '📏 ' + cm + ' cm testi: ileri gidiliyor...';
      const d0 = sensCm;
      await execDrive('fwd', cm, 100);
      await waitMs(700);
      const gone = d0 - sensCm;
      if (gone > 0.5){
        const r = gone / cm; ratios.push(r);
        calRows.push({ k: 'Duz', cmd: cm + ' cm', real: gone.toFixed(1) + ' cm', err: (r >= 1 ? '+' : '') + ((r - 1) * 100).toFixed(0) + '%' });
        calShow();
      }
      if (!motionRun) break;
      info.innerHTML = '📏 ' + cm + ' cm testi: baslangica donuluyor...';
      await execDrive('back', cm, 100);
      await waitMs(700);
    }
    if (ratios.length){
      const avg = ratios.reduce((a, b) => a + b, 0) / ratios.length;
      const nv = calCm() * avg;
      $('calCm').value = nv.toFixed(2); lsSet('rl_cal_cm', $('calCm').value); updCalText();
      info.innerHTML = '✓ Duz gitme kalibre edildi: ortalama sapma %' + ((avg - 1) * 100).toFixed(0) + ' → <b>' + nv.toFixed(1) + ' cm/sn</b>';
      logEvent('Duz gitme kalibre edildi: ' + nv.toFixed(1) + ' cm/sn');
    } else info.innerHTML = '✗ Mesafe degisimi olculemedi. Robot duz bir yuzeye bakiyor mu?';
  } finally { $('useSensor').checked = keep; motionRun = false; sendTank(0, 0); updPathInfo(); updProgInfo(); }
});

// --- DONUS: 90 / 180 / 360 derece, her testten sonra geri doner ---
$('calTurnAuto').addEventListener('click', async () => {
  if (motionRun){ toast('Baska bir hareket calisiyor'); return; }
  if (!sensorLive){ toast('Arduino sensor koprusu bagli degil'); return; }
  const info = $('calAutoInfo'), keep = $('useSensor').checked;
  calRows = []; calShow();
  info.innerHTML = '<b class="warn">⚠ ROBOT KENDI EKSENINDE DONECEK. Cevresini bosaltin, kablolari sikistirmayin.</b> 2 sn sonra basliyor...';
  await sleep(2000);
  stopDrive(); motionRun = true;
  $('useSensor').checked = false;
  const ratios = [];
  try{
    for (const dg of [90, 180, 360]){
      if (!motionRun) break;
      info.innerHTML = '🔄 ' + dg + '° testi: sola donuluyor...';
      api('/sensor?cmd=zero').catch(()=>{});
      await waitMs(500);
      const a0 = sensDeg;
      await execDrive('left', dg, 100);
      await waitMs(700);
      const turned = Math.abs(sensDeg - a0);
      if (turned > 5){
        const r = turned / dg; ratios.push(r);
        calRows.push({ k: 'Donus', cmd: dg + '°', real: turned.toFixed(0) + '°', err: (r >= 1 ? '+' : '') + ((r - 1) * 100).toFixed(0) + '%' });
        calShow();
      }
      if (!motionRun) break;
      info.innerHTML = '🔄 ' + dg + '° testi: baslangica donuluyor...';
      await execDrive('right', dg, 100);
      await waitMs(700);
    }
    if (ratios.length){
      const avg = ratios.reduce((a, b) => a + b, 0) / ratios.length;
      const nd = calDeg() * avg;
      $('calDeg').value = nd.toFixed(1); lsSet('rl_cal_deg', $('calDeg').value); updCalText();
      info.innerHTML = '✓ Donus kalibre edildi: ortalama sapma %' + ((avg - 1) * 100).toFixed(0) + ' → <b>' + nd.toFixed(0) + ' °/sn</b>';
      logEvent('Donus kalibre edildi: ' + nd.toFixed(0) + ' °/sn');
    } else info.innerHTML = '✗ Donus olculemedi. Jiroskop bagli mi?';
  } finally { $('useSensor').checked = keep; motionRun = false; sendTank(0, 0); updPathInfo(); updProgInfo(); }
});

// =====================================================
// SERVO KALIBRASYONU (kadranli ekran)
// =====================================================
let calMode = false, sweepCh = 0;
const calRaw = {};
function applyServoRanges(){
  [4,5,6,7].forEach(ch => { const c = SCFG[ch]; $('m' + ch).min = c.min; $('m' + ch).max = c.max; });
  updGripInfo();
}
function updGripInfo(){
  const el = $('gripCalInfo'); if (!el) return;
  el.textContent = 'Gripper CH' + JOINT.grip.ch + ': ACIK = ' + Math.round(JOINT.grip.open) + '°, KAPALI = ' + Math.round(JOINT.grip.close) + '° (KALIBRASYON MODU\'ndan ayarlanir)';
}
function renderJointQuick(){
  const el = $('jointQuick'); if (!el) return;
  el.innerHTML = ['base','shoulder','elbow','grip'].map(k =>
    JOINT[k].name + ' <select data-jq="' + k + '">' + [4,5,6,7].map(c =>
      '<option value="' + c + '"' + (JOINT[k].ch === c ? ' selected' : '') + '>CH' + c + '</option>').join('') + '</select>').join(' &nbsp; ');
  el.querySelectorAll('[data-jq]').forEach(sel => sel.addEventListener('change', function(){
    const k = this.dataset.jq, nv = +this.value;
    const other = ['base','shoulder','elbow','grip'].find(x => x !== k && JOINT[x].ch === nv);
    if (other) JOINT[other].ch = JOINT[k].ch;       // kanallar takas edilir
    JOINT[k].ch = nv;
    lsSet('rl_joint', JSON.stringify({ base: JOINT.base.ch, shoulder: JOINT.shoulder.ch, elbow: JOINT.elbow.ch, grip: JOINT.grip.ch }));
    refreshJointNames(); if (calMode) renderCalRows();
    toast(JOINT[k].name + ' artik CH' + nv);
    logEvent('Kol kanali degisti: ' + JOINT[k].name + ' -> CH' + nv);
  }));
}
function refreshJointNames(){
  for (const k in chName) delete chName[k];
  chName[JOINT.base.ch] = JOINT.base.name; chName[JOINT.shoulder.ch] = JOINT.shoulder.name;
  chName[JOINT.elbow.ch] = JOINT.elbow.name; chName[JOINT.grip.ch] = JOINT.grip.name;
  [4,5,6,7].forEach(ch => { $('n' + ch).textContent = 'CH' + ch + ' ' + (chName[ch] || ''); });
  showPoses(); updGripInfo(); renderJointQuick();
}
const dialAng = (ch, raw) => (90 + SCFG[ch].range / 2 - raw) * RAD;   // ekran acisi (radyan)
function setRaw(ch, v, card){
  const c = SCFG[ch];
  calRaw[ch] = clamp(v, 0, c.range);
  tgt[ch] = rawToLog(ch, calRaw[ch]);
  api('/pulse?ch=' + ch + '&us=' + rawToUs(ch, calRaw[ch])).catch(()=>{});
  if (card) updCard(ch, card);
}
function updCard(ch, d){
  const c = SCFG[ch], q = k => d.querySelector('[data-k="' + k + '"]');
  if (document.activeElement !== q('raw')) q('raw').value = calRaw[ch];
  q('rawT').textContent = calRaw[ch].toFixed(1) + '°';
  q('logT').textContent = 'Kalibre aci: ' + rawToLog(ch, calRaw[ch]).toFixed(1) + '°   |   ofset ' + c.off.toFixed(1) + '°   |   sinyal ' + rawToUs(ch, calRaw[ch]) + ' us';
}
function renderCalRows(){
  const box = $('calRows'); box.innerHTML = '';
  // Eklem eslestirme
  const jm = document.createElement('div'); jm.className = 'calr';
  jm.innerHTML = '<div class="calh"><b>EKLEM ESLESTIRME</b> <span class="sub">Robotta hangi eklemin servosu hangi kanala takili?</span></div><div class="calh" style="margin-top:6px">' +
    ['base','shoulder','elbow','grip'].map(k => JOINT[k].name + ' <select data-j="' + k + '">' + [4,5,6,7].map(c => '<option value="' + c + '"' + (JOINT[k].ch === c ? ' selected' : '') + '>CH' + c + '</option>').join('') + '</select>').join(' &nbsp; ') + '</div>';
  jm.querySelectorAll('[data-j]').forEach(sel => sel.addEventListener('change', function(){
    const k = this.dataset.j, nv = +this.value, other = ['base','shoulder','elbow','grip'].find(x => x !== k && JOINT[x].ch === nv);
    if (other) JOINT[other].ch = JOINT[k].ch;   // iki eklem ayni kanali almasin: yer degistir
    JOINT[k].ch = nv;
    lsSet('rl_joint', JSON.stringify({ base: JOINT.base.ch, shoulder: JOINT.shoulder.ch, elbow: JOINT.elbow.ch, grip: JOINT.grip.ch }));
    refreshJointNames(); renderCalRows(); toast('Eklem eslestirmesi kaydedildi');
  }));
  box.appendChild(jm);

  [4,5,6,7].forEach(ch => {
    const c = SCFG[ch], isGrip = ch === JOINT.grip.ch;
    if (!(ch in calRaw)) calRaw[ch] = clamp(logToRaw(ch, act[ch]), 0, c.range);
    const d = document.createElement('div'); d.className = 'calr';
    const gotoBtns = [0, 90, 180].concat(c.range === 270 ? [270] : []);
    d.innerHTML =
      '<div class="calh"><b>CH' + ch + ' ' + (chName[ch] || '') + '</b>' +
      ' Tip <select data-k="range"><option value="180"' + (c.range === 180 ? ' selected' : '') + '>180°</option><option value="270"' + (c.range === 270 ? ' selected' : '') + '>270°</option></select>' +
      ' Yon <select data-k="dir"><option value="1"' + (c.dir === 1 ? ' selected' : '') + '>normal</option><option value="-1"' + (c.dir === -1 ? ' selected' : '') + '>ters</option></select></div>' +
      '<div class="calg"><canvas class="dial" data-ch="' + ch + '"></canvas><div>' +
        '<div class="dnum"><span data-k="rawT">0°</span> <small>ham aci</small></div>' +
        '<div class="sub" data-k="logT">-</div>' +
        '<div class="nud">' + [-10, -5, -1, 1, 5, 10].map(v => '<button data-n="' + v + '">' + (v > 0 ? '+' : '') + v + '°</button>').join('') + '</div>' +
        '<div class="nud">' + gotoBtns.map(v => '<button data-g="' + v + '">' + v + '°</button>').join('') + '<button data-g="mid">ORTA</button></div>' +
        '<input type="range" data-k="raw" min="0" max="' + c.range + '" step="0.5" value="' + calRaw[ch] + '">' +
      '</div></div>' +
      '<div class="inl">Bu konum = <input type="number" data-k="ref" step="1" value="' + Math.round(rawToLog(ch, calRaw[ch])) + '"> ° <button data-k="apply" class="up" style="min-height:34px;padding:6px 12px">KALIBRE ET</button> <button data-k="zero" style="min-height:34px;padding:6px 12px">BU KONUM = 0°</button></div>' +
      '<div class="inl">Sinir: en az <input type="number" data-k="min" step="1" value="' + c.min + '"> ° en cok <input type="number" data-k="max" step="1" value="' + c.max + '"> °' +
        ' <button data-k="setMin" style="min-height:32px;padding:4px 10px">SU AN = EN AZ</button> <button data-k="setMax" style="min-height:32px;padding:4px 10px">SU AN = EN COK</button></div>' +
      (isGrip ? '<div class="row"><button data-k="gOpen">✋ BU KONUM = GRIPPER ACIK</button><button data-k="gClose">✊ BU KONUM = GRIPPER KAPALI</button></div>' +
                '<div class="row"><button data-k="gTestO">TEST: AC</button><button data-k="gTestC">TEST: KAPAT</button></div>' : '') +
      '<div class="row"><button data-k="sweep">↔ TEST: EN AZ → EN COK → ORTA</button></div>';
    const q = k => d.querySelector('[data-k="' + k + '"]');
    q('range').addEventListener('change', function(){
      const nr = +this.value; if (c.max === c.range) c.max = nr; c.range = nr;
      calRaw[ch] = clamp(calRaw[ch], 0, nr); saveScfg(); applyServoRanges(); renderCalRows();
    });
    q('dir').addEventListener('change', function(){ c.dir = +this.value; saveScfg(); renderCalRows(); });
    q('raw').addEventListener('input', function(){ setRaw(ch, +this.value, d); });
    d.querySelectorAll('[data-n]').forEach(b => b.addEventListener('click', () => setRaw(ch, calRaw[ch] + (+b.dataset.n), d)));
    d.querySelectorAll('[data-g]').forEach(b => b.addEventListener('click', () => setRaw(ch, b.dataset.g === 'mid' ? c.range / 2 : +b.dataset.g, d)));
    const applyRef = Lref => { c.off = calRaw[ch] - c.dir * Lref; tgt[ch] = Lref; saveScfg(); renderCalRows(); toast('CH' + ch + ' kalibre edildi: bu konum artik ' + Lref + '°'); logEvent('Servo CH' + ch + ' kalibre edildi (' + Lref + '°)'); };
    q('apply').addEventListener('click', () => { const L = parseFloat(q('ref').value); if (!isFinite(L)){ toast('Bu konumun kac derece oldugunu yazin'); return; } applyRef(L); });
    q('zero').addEventListener('click', () => applyRef(0));
    q('min').addEventListener('change', function(){ c.min = +this.value; if (c.max < c.min) c.max = c.min; saveScfg(); applyServoRanges(); });
    q('max').addEventListener('change', function(){ c.max = +this.value; if (c.max < c.min) c.min = c.max; saveScfg(); applyServoRanges(); });
    q('setMin').addEventListener('click', () => { c.min = Math.round(rawToLog(ch, calRaw[ch])); if (c.max < c.min) c.max = c.min; saveScfg(); applyServoRanges(); renderCalRows(); toast('En az sinir = ' + c.min + '°'); });
    q('setMax').addEventListener('click', () => { c.max = Math.round(rawToLog(ch, calRaw[ch])); if (c.max < c.min) c.min = c.max; saveScfg(); applyServoRanges(); renderCalRows(); toast('En cok sinir = ' + c.max + '°'); });
    if (isGrip){
      const saveGrip = () => { lsSet('rl_grip', JSON.stringify({ open: JOINT.grip.open, close: JOINT.grip.close })); updGripInfo(); };
      q('gOpen').addEventListener('click', () => { JOINT.grip.open = Math.round(rawToLog(ch, calRaw[ch])); saveGrip(); toast('Gripper ACIK = ' + JOINT.grip.open + '°'); });
      q('gClose').addEventListener('click', () => { JOINT.grip.close = Math.round(rawToLog(ch, calRaw[ch])); saveGrip(); toast('Gripper KAPALI = ' + JOINT.grip.close + '°'); });
      q('gTestO').addEventListener('click', () => setRaw(ch, logToRaw(ch, JOINT.grip.open), d));
      q('gTestC').addEventListener('click', () => setRaw(ch, logToRaw(ch, JOINT.grip.close), d));
    }
    q('sweep').addEventListener('click', async () => {
      if (sweepCh){ sweepCh = 0; return; }
      sweepCh = ch; q('sweep').textContent = '■ TESTI DURDUR';
      const seq = [c.min, c.max, (c.min + c.max) / 2];
      let L = rawToLog(ch, calRaw[ch]);
      for (const T of seq){
        while (sweepCh === ch && Math.abs(L - T) > 0.6){ L += Math.sign(T - L) * 1; setRaw(ch, logToRaw(ch, L), d); await sleep(28); }
        if (sweepCh !== ch) break;
        await sleep(300);
      }
      sweepCh = 0; q('sweep').textContent = '↔ TEST: EN AZ → EN COK → ORTA';
    });
    updCard(ch, d);
    box.appendChild(d);
  });
}
function drawDials(){
  document.querySelectorAll('canvas.dial').forEach(cv => {
    const ch = +cv.dataset.ch, c = SCFG[ch]; if (!c) return;
    const { ctx, w, h } = fit(cv);
    ctx.clearRect(0, 0, w, h);
    const cx = w / 2, cy = h / 2 + 6, R = Math.min(w, h) / 2 - 16;
    const P = (a, r) => [cx + r * Math.cos(a), cy - r * Math.sin(a)];
    // servo govdesi
    ctx.fillStyle = '#111827'; ctx.fillRect(cx - R*0.55, cy - R*0.35, R*1.1, R*0.95);
    ctx.fillStyle = '#1f2937'; ctx.beginPath(); ctx.arc(cx, cy, R*0.3, 0, Math.PI*2); ctx.fill();
    // izinli bolge (yesil) ve disi (kirmizi)
    const rMin = clamp(logToRaw(ch, c.min), 0, c.range), rMax = clamp(logToRaw(ch, c.max), 0, c.range);
    const lo = Math.min(rMin, rMax), hi = Math.max(rMin, rMax);
    const arc = (r0, r1, col) => { ctx.strokeStyle = col; ctx.lineWidth = 6; ctx.beginPath(); ctx.arc(cx, cy, R, -dialAng(ch, r0), -dialAng(ch, r1)); ctx.stroke(); };
    arc(0, c.range, 'rgba(244,63,94,.55)');
    arc(lo, hi, 'rgba(52,211,153,.9)');
    // olcek
    ctx.fillStyle = '#cbd5e1'; ctx.font = '10px system-ui'; ctx.textAlign = 'center';
    for (let r = 0; r <= c.range + 0.1; r += 15){
      const a = dialAng(ch, r), mj = r % 90 === 0, A = P(a, R - 5), B = P(a, R + (mj ? 7 : 3));
      ctx.strokeStyle = 'rgba(226,232,240,.8)'; ctx.lineWidth = mj ? 2 : 1; ctx.beginPath(); ctx.moveTo(A[0], A[1]); ctx.lineTo(B[0], B[1]); ctx.stroke();
      if (mj){ const T = P(a, R + 14); ctx.fillText(r + '', T[0], T[1] + 3); }
    }
    // gercek konum (soluk) ve hedef (parlak) kol
    const horn = (raw, col, wd, glow) => {
      const a = dialAng(ch, raw), T = P(a, R * 0.85);
      if (glow){ ctx.shadowColor = col; ctx.shadowBlur = 12; }
      ctx.strokeStyle = col; ctx.lineWidth = wd; ctx.lineCap = 'round';
      ctx.beginPath(); ctx.moveTo(cx, cy); ctx.lineTo(T[0], T[1]); ctx.stroke();
      ctx.shadowBlur = 0;
      ctx.fillStyle = col; ctx.beginPath(); ctx.arc(T[0], T[1], wd * 0.9, 0, Math.PI*2); ctx.fill();
    };
    horn(clamp(logToRaw(ch, act[ch]), 0, c.range), 'rgba(226,232,240,.45)', 7, false);
    horn(calRaw[ch] !== undefined ? calRaw[ch] : clamp(logToRaw(ch, tgt[ch]), 0, c.range), '#22d3ee', 5, true);
    ctx.fillStyle = '#e2e8f0'; ctx.beginPath(); ctx.arc(cx, cy, 5, 0, Math.PI*2); ctx.fill();
    // kalibre aci yazisi
    ctx.fillStyle = '#fde68a'; ctx.font = 'bold 12px system-ui';
    ctx.fillText(rawToLog(ch, calRaw[ch] !== undefined ? calRaw[ch] : 0).toFixed(0) + '° (kalibre)', cx, cy + R*0.62);
    // gripper icin cene gorseli
    if (ch === JOINT.grip.ch){
      const p = gripPercent(rawToLog(ch, calRaw[ch] !== undefined ? calRaw[ch] : 0)) / 100, gap = (1 - p) * 14 + 2;
      ctx.fillStyle = '#fbbf24';
      ctx.fillRect(cx - gap - 5, 6, 5, 18); ctx.fillRect(cx + gap, 6, 5, 18); ctx.fillRect(cx - gap - 5, 6, gap*2 + 10, 3);
    }
  });
}
function setCalMode(on){
  calMode = on;
  $('calBox').classList.toggle('show', on);
  $('calModeBtn').classList.toggle('on', on);
  if (on){ [4,5,6,7].forEach(ch => { delete calRaw[ch]; }); renderCalRows(); toast('KALIBRASYON MODU: kadranlara bakarak servoyu konumlandirin'); }
  else { sweepCh = 0; }
}
$('calModeBtn').addEventListener('click', () => setCalMode(!calMode));
$('calDone').addEventListener('click', () => { setCalMode(false); toast('Kalibrasyon kaydedildi'); });
$('calReset').addEventListener('click', () => {
  [4,5,6,7].forEach(ch => { SCFG[ch] = SCFG_DEF(); delete calRaw[ch]; });
  saveScfg(); applyServoRanges(); renderCalRows(); toast('Servo ayarlari varsayilana dondu');
});

// =====================================================
// EKRANDAKI KOLU GERCEK KOLA ESLE (robota aci gondermez)
// =====================================================
const MJ = { yaw: 0, t1: 90, t2: 0 };
function mjShow(){
  $('mjYaw').value = MJ.yaw; $('mjYawT').textContent = Math.round(MJ.yaw) + '°';
  $('mjT1').value = MJ.t1;   $('mjT1T').textContent = Math.round(MJ.t1) + '°';
  $('mjT2').value = MJ.t2;   $('mjT2T').textContent = Math.round(MJ.t2) + '°';
}
function setMatch(on){
  matchMode = on;
  $('matchBox').classList.toggle('show', on);
  $('matchBtn').classList.toggle('on', on);
  if (on){
    const k = fk(act);
    MJ.yaw = Math.round(k.yaw / RAD); MJ.t1 = Math.round(k.t1 / RAD); MJ.t2 = Math.round(k.t2 / RAD);
    $('mjDirB').checked = JOINT.base.dir === -1; $('mjDirS').checked = JOINT.shoulder.dir === -1; $('mjDirE').checked = JOINT.elbow.dir === -1;
    mjShow();
    toast('ESLEME MODU: robota aci gonderilmiyor');
  }
}
$('matchBtn').addEventListener('click', () => setMatch(!matchMode));
$('matchCancel').addEventListener('click', () => setMatch(false));
$('matchUp').addEventListener('click', () => { MJ.yaw = 0; MJ.t1 = 90; MJ.t2 = 0; mjShow(); });
[['mjYaw','yaw'], ['mjT1','t1'], ['mjT2','t2']].forEach(([id, k]) => $(id).addEventListener('input', function(){ MJ[k] = +this.value; mjShow(); }));
$('matchSave').addEventListener('click', () => {
  // Gercek servo su an act[] acisinda; ekranda MJ acisi gorunmeli -> j90 = MJ - dir * (act - 90)
  const set = (j, ang, neg) => { j.dir = neg ? -1 : 1; j.j90 = ang - j.dir * (act[j.ch] - 90); };
  set(JOINT.base, MJ.yaw, $('mjDirB').checked);
  set(JOINT.shoulder, MJ.t1, $('mjDirS').checked);
  set(JOINT.elbow, MJ.t2, $('mjDirE').checked);
  lsSet('rl_jgeo', JSON.stringify({ base: { j90: JOINT.base.j90, dir: JOINT.base.dir }, shoulder: { j90: JOINT.shoulder.j90, dir: JOINT.shoulder.dir }, elbow: { j90: JOINT.elbow.j90, dir: JOINT.elbow.dir } }));
  [4,5,6,7].forEach(ch => { tgt[ch] = act[ch]; });     // hedefi mevcut konuma esitle: kol yerinden oynamaz
  setMatch(false);
  logEvent('Ekran kolu gercek kola eslendi');
  toast('Esleme kaydedildi: ekrandaki kol artik gercek kolla ayni');
});
$('matchReset').addEventListener('click', () => {
  JOINT.base.j90 = 0; JOINT.base.dir = 1; JOINT.shoulder.j90 = 90; JOINT.shoulder.dir = 1; JOINT.elbow.j90 = 0; JOINT.elbow.dir = 1;
  lsSet('rl_jgeo', 'null'); setMatch(false); toast('Esleme varsayilana dondu (servo 90° = dimdik)');
});
$('bootPose').addEventListener('click', () => {
  const q = [4,5,6,7].map(ch => 'c' + ch + '=' + Math.round(degToUs(act[ch], ch))).join('&');
  api('/startup?' + q).then(r => r.status === 404 ? 'Robot kodu eski: acilis pozu icin v7 kodunu yukleyin' : r.text()).then(t => { toast(t); logEvent('Acilis pozu kaydedildi'); }).catch(()=>{});
});

// =====================================================
// AYARLARI KAYDET / YEDEKLE + SON ISLEMLER
// =====================================================
let LOG = [];
try{ LOG = JSON.parse(lsGet('rl_log', '[]')) || []; }catch(_){ LOG = []; }
function logEvent(m){
  const d = new Date(), t = d.toLocaleDateString('tr-TR') + ' ' + d.toLocaleTimeString('tr-TR', { hour: '2-digit', minute: '2-digit' });
  LOG.unshift({ t, m }); LOG = LOG.slice(0, 30);
  lsSet('rl_log', JSON.stringify(LOG)); renderLog();
}
function renderLog(){
  const el = $('logList'); if (!el) return;
  el.innerHTML = LOG.length ? LOG.map(e => '<div class="hi" style="margin:4px 0"><b>' + e.t + '</b> — ' + String(e.m).replace(/</g, '') + '</div>').join('') : '<div class="sub">Henuz kayit yok.</div>';
}
function saveNow(){
  if (!IS_ROBOT){ $('saveState').textContent = '✓ Bu cihaza kaydedildi (demo: robot bagli degil)'; return; }
  fetch('/cfg', { method: 'POST', headers: { 'Content-Type': 'text/plain' }, body: JSON.stringify(CFG) })
    .then(r => { $('saveState').textContent = r.status === 200 ? '✓ Robota kaydedildi: ' + new Date().toLocaleTimeString('tr-TR') : '✗ Robota kaydedilemedi (robot kodu eski olabilir)'; })
    .catch(() => { $('saveState').textContent = '✗ Robota ulasilamadi'; });
}
$('saveNow').addEventListener('click', () => { saveNow(); logEvent('Ayarlar elle kaydedildi'); });
const bkJSON = () => JSON.stringify({ robolig: 'yedek', tarih: new Date().toISOString(), cfg: CFG }, null, 1);
$('bkShow').addEventListener('click', () => {
  const ta = $('bkText'); ta.style.display = ''; ta.value = bkJSON(); ta.select();
  try{ navigator.clipboard.writeText(ta.value).then(() => toast('Yedek panoya kopyalandi'), () => toast('Metni secip kopyalayin')); }catch(_){ toast('Metni secip kopyalayin'); }
});
$('bkDown').addEventListener('click', () => {
  try{
    const a = document.createElement('a');
    a.href = URL.createObjectURL(new Blob([bkJSON()], { type: 'application/json' }));
    a.download = 'robolig_yedek_' + new Date().toISOString().slice(0, 10) + '.json';
    document.body.appendChild(a); a.click(); a.remove();
    toast('Yedek dosyasi indiriliyor');
  }catch(_){ toast('Indirilemedi: YEDEGI GOSTER ile kopyalayin'); }
});
function loadBackup(txt){
  try{
    const o = JSON.parse(txt), c = o && o.cfg ? o.cfg : o;
    if (!c || typeof c !== 'object'){ toast('Gecersiz yedek'); return; }
    CFG = c;
    try{ localStorage.setItem('rl_cfg', JSON.stringify(CFG)); }catch(_){}
    const done = () => { toast('Yedek yuklendi, sayfa yenileniyor...'); setTimeout(() => location.reload(), 1200); };
    if (IS_ROBOT) fetch('/cfg', { method: 'POST', headers: { 'Content-Type': 'text/plain' }, body: JSON.stringify(CFG) }).then(done, done); else done();
  }catch(_){ toast('Yedek okunamadi: metnin tamamini yapistirdiginizdan emin olun'); }
}
$('bkLoad').addEventListener('click', () => { const t = $('bkText').value.trim(); if (!t){ $('bkText').style.display = ''; toast('Yedek metnini kutuya yapistirin'); return; } loadBackup(t); });
$('bkFile').addEventListener('change', function(){ const f = this.files && this.files[0]; if (!f) return; const r = new FileReader(); r.onload = () => loadBackup(String(r.result)); r.readAsText(f); });
renderLog();

// =====================================================
// WEB UZERINDEN KOD YUKLEME (OTA)
// =====================================================
$('otaBtn').addEventListener('click', () => {
  const f = $('otaFile').files && $('otaFile').files[0];
  if (!f){ toast('Once .bin dosyasini secin'); return; }
  if (!/\.bin$/i.test(f.name)){ toast('Sadece .bin dosyasi yuklenebilir'); return; }
  if (/bootloader|partitions|merged/i.test(f.name)){ toast('Yanlis dosya: adinda bootloader / partitions / merged gecmeyen .ino.bin dosyasini secin'); return; }
  abortMotion(); stopDrive(); stopLinear();
  const bar = $('otaBar'), txt = $('otaTxt');
  if (DEMO){
    let p = 0; txt.textContent = 'DEMO: yukleme taklit ediliyor...';
    const t = setInterval(() => { p += 5; bar.style.width = p + '%'; if (p >= 100){ clearInterval(t); txt.textContent = 'DEMO: gercek robotta bu noktada robot yeniden baslar.'; } }, 80);
    return;
  }
  const fd = new FormData(); fd.append('update', f, f.name);
  const x = new XMLHttpRequest();
  x.upload.onprogress = e => { if (e.lengthComputable){ const p = e.loaded / e.total * 100; bar.style.width = p + '%'; txt.textContent = 'Yukleniyor... %' + p.toFixed(0) + ' (gucu kesmeyin)'; } };
  x.onload = () => {
    if (x.status === 200 && /OK/.test(x.responseText)){ bar.style.width = '100%'; txt.textContent = '✓ Yukleme tamamlandi. Robot yeniden basliyor; Wi-Fi ag gelince sayfayi yenileyin.'; }
    else if (x.status === 404) txt.textContent = '✗ Robottaki kod OTA desteklemiyor. Bu seferlik yeni kodu USB ile yukleyin, sonraki guncellemeler buradan yapilabilir.';
    else txt.textContent = '✗ Yukleme hatasi: ' + x.status + ' ' + x.responseText;
  };
  x.onerror = () => { txt.textContent = '✗ Baglanti koptu. Robot yeniden basliyor olabilir; biraz bekleyip sayfayi yenileyin.'; };
  x.open('POST', '/update'); x.send(fd); logEvent('Web uzerinden kod yuklendi: ' + f.name);
  txt.textContent = 'Yukleniyor...';
});

$('hdrOta').addEventListener('click', () => { showPanel('sys'); setTimeout(() => { try{ $('otaCard').scrollIntoView({ behavior: 'smooth' }); }catch(_){} }, 50); });

function drawRobotAt(ctx, m, pose, glow){
  const rp = fS(m, pose.x, pose.y), sz = Math.max(8, (+$('robW').value || 45) * m.sc);
  ctx.save(); ctx.translate(rp[0], rp[1]); ctx.rotate(-(pose.h - 90) * RAD);
  ctx.shadowColor = '#22d3ee'; ctx.shadowBlur = glow ? 18 : 6;
  ctx.fillStyle = 'rgba(14,116,144,.9)'; ctx.strokeStyle = '#22d3ee'; ctx.lineWidth = 2;
  ctx.fillRect(-sz/2, -sz/2, sz, sz); ctx.strokeRect(-sz/2, -sz/2, sz, sz);
  ctx.shadowBlur = 0; ctx.fillStyle = '#e0f2fe';
  ctx.beginPath(); ctx.moveTo(0, -sz/2 - 6); ctx.lineTo(-6, -sz/2 + 4); ctx.lineTo(6, -sz/2 + 4); ctx.closePath(); ctx.fill();
  ctx.restore();
}
function drawMini(){
  const cv = $('miniMap'), { ctx, w, h } = fit(cv);
  ctx.clearRect(0, 0, w, h);
  const sc = Math.min((w - 10) / FW, (h - 10) / FH), m = { sc, ox: (w - FW*sc)/2, oy: (h - FH*sc)/2 };
  drawField(ctx, m);
  ZONES.forEach(z => { const a = fS(m, z.x, z.y); ctx.fillStyle = 'rgba(251,146,60,.3)'; ctx.fillRect(a[0], a[1], z.w*sc, z.h*sc); });
  drawRobotAt(ctx, m, motionRun ? robotPose : startPose, motionRun);
}

// =====================================================
// DURUM (robottan)
// =====================================================
let rfidWaitUid = null, rfidLastSeen = '';
let sensorLive = false, sensCm = -1, sensDeg = 0, mvKind = 0, mvDone = 0, mvTarget = 0, mvBySensor = false, mvMethod = '-';
let lastOK = 0;
function setH(id, ok, txt){ const e = $(id); e.textContent = txt || (ok ? 'OK' : 'HATA'); e.className = ok ? 'okc' : 'badc'; }
function poll(){
  api('/status').then(r => r.json()).then(d => {
    lastOK = Date.now();
    linPos = d.position; linMax = d.linearMax || 16000;
    if (document.activeElement !== $('linMaxIn')) $('linMaxIn').value = linMax;
    if (d.limits !== undefined && document.activeElement !== $('linLimit')) $('linLimit').checked = !!d.limits;
    const hw = $('homeWarn');
    if (d.homed === false){ hw.textContent = '⚠ Konum dogrulanmadi: lineeri en alta indirip EN ALT = 0\'a basin. Bu yapilmadan yazilim siniri calismaz.'; hw.style.color = 'var(--yel)'; }
    else if (d.homed === true){ hw.textContent = '✓ Konum dogrulandi. ' + (d.limits ? 'Yazilim siniri acik (0 - ' + linMax + ' adim).' : 'Yazilim siniri kapali.'); hw.style.color = 'var(--grn)'; }
    else hw.textContent = '';
    updStroke(); linDir = d.linearDir || 0; linGoto = (d.linearGoto === undefined) ? -1 : d.linearGoto;
    const cm = stepToCm(d.position), S = stroke(), Hk = hookH(d.position), dz = ZIP_H - Hk;
    $('linH').textContent = Hk.toFixed(1) + ' cm';
    $('linCm').textContent = cm.toFixed(1) + ' cm';
    $('linLeft').textContent = Math.max(0, S - cm).toFixed(1) + ' cm';
    $('linZip').textContent = Math.abs(dz) <= 1 ? 'ULASTI ✓' : (dz > 0 ? dz.toFixed(1) + ' cm kaldi' : (-dz).toFixed(1) + ' cm ustunde');
    $('linPos').textContent = d.position;
    $('linMax').textContent = linMax;
    const pct = clamp(d.position / linMax * 100, 0, 100);
    $('linBar').style.width = pct + '%'; $('linPct').textContent = '%' + pct.toFixed(1);
    $('linSpd').textContent = Math.round(d.linearSpeed) + ' p/s';
    if (document.activeElement !== $('linearSpeed')){ $('linearSpeed').value = d.linearTarget; $('linTgt').textContent = d.linearTarget; }
    $('fixNum').max = linMax;
    act[4] = usToDeg(d.s4, 4); act[5] = usToDeg(d.s5, 5); act[6] = usToDeg(d.s6, 6); act[7] = usToDeg(d.s7, 7);
    if (d.fw) $('fwVer').textContent = d.fw;
    // Arduino sensor koprusu
    sensorLive = !!d.sensor; sensCm = (d.sCm === undefined) ? -1 : d.sCm; sensDeg = d.sDeg || 0;
    mvKind = d.mv || 0; mvDone = d.mvDone || 0; mvTarget = d.mvTarget || 0; mvBySensor = !!d.mvSensor;
    const ss = $('sensState');
    if (d.sensor === undefined){ ss.textContent = '⚠ Robot kodu eski (v8 gerekli): sensor desteklenmiyor'; ss.style.color = 'var(--yel)'; }
    else if (sensorLive){ ss.textContent = '✓ Arduino bagli (' + (d.sPkt || 0) + ' paket)'; ss.style.color = 'var(--grn)'; }
    else { ss.textContent = '✗ Arduino\'dan veri gelmiyor: D8/D9 ve ortak GND kablolarini kontrol edin'; ss.style.color = 'var(--red)'; }
    $('sensCm').textContent = (sensorLive && sensCm >= 0) ? sensCm.toFixed(1) + ' cm' : '—';
    $('sensDeg').textContent = sensorLive ? sensDeg.toFixed(1) + '°' : '—';
    const mnames = ['-', 'ILERI', 'GERI', 'SOLA', 'SAGA'];
    $('moveState').textContent = mvKind ? ('▶ ' + mnames[mvKind] + ': ' + mvDone.toFixed(1) + ' / ' + mvTarget.toFixed(1) + (mvKind >= 3 ? '°' : ' cm') + '  (' + (mvBySensor ? 'SENSORLE' : 'SUREYE GORE') + ')')
                                        : ('Son hareket: ' + (d.mvNote || '-'));
    if (!servoSynced){ [4,5,6,7].forEach(ch => { tgt[ch] = act[ch]; }); servoSynced = true; }
    syncManualUI();
    setH('hOled', d.oledOK); setH('hPca', d.pcaOK); setH('hRfid', d.rfidOK, d.rfidOK ? 'OK ' + d.rfidVersion : 'HATA'); setH('hWifi', d.wifiOK);
    setH('hRemote', d.remote, d.remote ? 'BAGLI' : 'YOK');
    $('rfidUid').textContent = d.rfidUid;
    if (d.rfidUid && d.rfidUid !== 'HENUZ YOK' && d.rfidUid !== rfidLastSeen){ rfidLastSeen = d.rfidUid; rfidWaitUid = d.rfidUid; }
    const rt = $('rfidTest');
    if (!d.rfidOK) rt.innerHTML = '<span class="badc">✗ Okuyucu cevap vermiyor</span> — SDA(A6), RST(A7), SPI ve 3V3 kablolarini kontrol edin';
    else if (d.rfidUid && d.rfidUid !== 'HENUZ YOK') rt.innerHTML = '<span class="okc">✓ CALISIYOR</span> — en son okunan kart: <b>' + d.rfidUid + '</b>';
    else rt.innerHTML = '<span style="color:var(--yel);font-weight:800">⏳ Okuyucu hazir</span> — bir karti okuyucuya 2-3 cm yaklastirin, burada gorunecek';
    const all = d.oledOK && d.pcaOK && d.rfidOK && d.wifiOK;
    $('healthPill').textContent = all ? 'SISTEM OK' : 'SISTEMDE HATA'; $('healthPill').className = 'pill ' + (all ? 'ok' : 'bad');
  }).catch(()=>{});
}
setInterval(poll, 250); poll();
setInterval(() => { const ok = Date.now() - lastOK < 1500; $('connPill').textContent = ok ? 'ROBOT BAGLI' : 'BAGLANTI YOK'; $('connPill').className = 'pill ' + (ok ? 'ok' : 'bad'); }, 500);
$('oledTest').addEventListener('click', () => { api('/oledtest').catch(()=>{}); });

// =====================================================
// ACIL DUR + GUVENLIK
// =====================================================
function emergency(){ abortMotion(); stopDrive(); api('/stopall').catch(()=>{}); toast('ACIL DURDURULDU'); }
$('emergency').addEventListener('click', emergency);
window.addEventListener('blur', () => { abortMotion(); stopDrive(); if (linKey){ linKey = 0; stopLinear(); } });
document.addEventListener('visibilitychange', () => { if (document.hidden){ abortMotion(); stopDrive(); stopLinear(); } });

// =====================================================
// CIZIM DONGUSU
// =====================================================
function frame(){
  try{ updateBox(); }catch(e){}
  try{ if (activePanel === 'lin') drawLinear(); }catch(e){ console.log(e); }
  try{ if (activePanel === 'arm') drawArm(); }catch(e){ console.log(e); }
  try{ if (activePanel === 'arm' && calMode) drawDials(); }catch(e){ console.log(e); }
  try{ if (activePanel === 'path') drawPath(); }catch(e){ console.log(e); }
  try{ if (activePanel === 'moves') drawMini(); }catch(e){ console.log(e); }
  requestAnimationFrame(frame);
}
requestAnimationFrame(frame);
updCalText(); renderZones(); updPoseInputs(); applyServoRanges(); renderCityBtns(); renderJointQuick(); fixUpd();
renderPalette(); renderProg(); renderProgSel();
updPathInfo();
</script>
</body>
</html>
)rawliteral";
