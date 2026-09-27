/*
  ROBOLIG - TUM SISTEM KONTROL (3D WEB ARAYUZU v9)
  Deneyap Kart 1A V2

  !!! ONEMLI: web_sayfasi.h dosyasi bu .ino ile AYNI KLASORDE olmali !!!
      ROBOLIG_v9_tek/
        ├── ROBOLIG_v9_tek.ino
        └── web_sayfasi.h

  v2 YENILIKLER:
    - Sekmeli arayuz (Surus / Lineer / Robot Kol / Ciz-Git / Sistem)
    - WASD + yay donus + kendi ekseninde donus  -> /tank?l=..&r=..
    - Lineer "hedefe git" (yumusak yavaslayarak durur) -> /lineargoto?v=..
    - Teknofest universite sahasi, cetvel, kutu alma simulasyonu
  v3: gercek lineer mekanizmasi + zipline (90 cm), kayitli kol pozlari,
      kutu konumunu duzeltme, sahada buyutec (zoom)
  v4: ciz-git sari sinirli yollar (disina cikmak yasak), kesikli cizgiler yol,
      sari yasak kaldir butonu, hedefe otomatik rota
  v5: HAREKETLER (kod bloklari), surus kalibrasyon sihirbazi, servo kalibrasyonu
      (180/270 derece), yavas alanlar, robot konumu ayari,
      WEB UZERINDEN KOD YUKLEME (OTA) -> /update
      AYARLAR ROBOTTA SAKLANIR -> /cfg (Preferences / NVS)
  v6: WEB SAYFASI AYRI DOSYADA -> web_sayfasi.h (ayni klasorde olmali)
      lineer adim kalibrasyonu (en alt = 0, en ust = tam strok, varsayilan 16000)
      lineer yazilim siniri (0 ile en ust arasinda durur)  -> /linmax, /linlimit
  v7: ACILIS POZU robot hafizasinda saklanir (orn. kol dimdik)  -> /startup
      web: ekrani gercek kola esle, sade kod bloklari, ayar yedekleme
  v8: ARDUINO UNO SENSOR KOPRUSU (VL53L0X mesafe + MPU6050 aci) -> D8/D9 UART
      SENSORLU HAREKET: "1 m git" mesafe sensoruyle, "90 derece don" jiroskopla
      olculerek yapilir. Sensor yoksa sureye gore calismaya devam eder.
      Duz giderken jiroskopla yon duzeltme (robot dumduz gider).
      -> /move, /movestop, /sensor
  v9: web arayuzu yenilendi - saha olculeri sartnameye gore, sehre tiklayip
      30 cm kala durma, KOLI ALMA gorevi (git / geri cekil / etiketi okut /
      diger yuz), kasiste %85 hiz, otomatik kalibrasyon (10-20-30 cm ve
      90-180-360 derece), "neyi yanlis yapti" duzeltme, kol kanal eslestirme.
      ROBOT TARAFINDA DEGISIKLIK YOK: sadece gomulu web sayfasi guncellendi.
      Web sayfasi yine web_sayfasi.h dosyasinda (Arduino IDE .ino icindeki
      JavaScript'i C++ sanip "'function' does not name a type" hatasi veriyordu).

  SISTEMLER:
    1) 2 x L298N / 4 DC motor / tank surus
    2) TB6600 + NEMA23 lineer aktuator (0 - 15300 adim)
    3) RC522 RFID
    4) SSD1306 OLED
    5) PCA9685 + 4 servo (robot kol)
    6) 3D web arayuzu: http://192.168.4.1
       - Lineer 3D gorsel + "adim kacti" duzeltme
       - Robot kol 3D / yandan / ustten, uc noktayi surukleme
       - Ciz-git (deneysel)
    7) ESP-NOW kumanda alici (kumanda kullanilmasa da sorun olmaz)

  ARKA L298N: IN1 D0, IN2 D1, ENA D2, IN3 D3, IN4 D4, ENB D14
  ON L298N  : IN1 A0, IN2 A1, ENA A2, IN3 A3, IN4 A4, ENB A5
  RFID RC522: SDA A6, RST A7, SCK/MISO/MOSI kart SPI pinleri, 3V3, GND
  I2C       : SDA D10/GPIO47, SCL D11/GPIO21 (OLED 0x3C, PCA9685 0x41)
  TB6600    : PUL+ D12, DIR+ D13, PUL-/DIR- GND
  SERVO     : CH4 MG995, CH5 MG945, CH6 MG996R, CH7 QY3240MG
              50 Hz, 500-2500 us, 650 us/s rampa, acilista 1500 us HOLD
*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include "esp_timer.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PWMServoDriver.h>
#include <Update.h>        // YENI: web uzerinden kod yukleme (OTA)
#include <Preferences.h>   // YENI: ayarlari robot hafizasinda saklama
#include <memory>

const char* FW_VERSION = "v9";
Preferences prefs;

// =====================================================
// WIFI + WEB SERVER
// =====================================================

const char* WIFI_NAME = "ROBOLIG_KONTROL";
const char* WIFI_PASSWORD = "robolig35";

WebServer server(80);

// =====================================================
// ESP-NOW KUMANDA PAKETI (fonksiyonlardan ONCE olmali)
// =====================================================

typedef struct __attribute__((packed)) {
  uint8_t  surum;
  uint8_t  gecerli;     // bit0 joystick, bit1 lineer, bit2 CH4, bit3 CH5, bit4 CH6, bit5 CH7
  int8_t   joyX;
  int8_t   joyY;
  int8_t   lineer;
  uint16_t servoUs[4];
  uint8_t  sayac;
} KumandaPaketi;

// Servo durumu. Arduino IDE fonksiyon prototiplerini dosyanin basina ekledigi
// icin bu yapi da BUTUN FONKSIYONLARDAN ONCE tanimli olmali.
struct ServoState {
  uint8_t channel;
  const char* name;
  uint16_t minUs;
  uint16_t maxUs;
  float currentUs;
  float targetUs;
  bool active;
};

// =====================================================
// ARKA L298N
// =====================================================

#define REAR_LEFT_IN1   D0
#define REAR_LEFT_IN2   D1
#define REAR_LEFT_EN    D2

#define REAR_RIGHT_IN1  D3
#define REAR_RIGHT_IN2  D4
#define REAR_RIGHT_EN   D14

// =====================================================
// ON L298N
// =====================================================

#define FRONT_LEFT_IN1   A0
#define FRONT_LEFT_IN2   A1
#define FRONT_LEFT_EN    A2

#define FRONT_RIGHT_IN1  A3
#define FRONT_RIGHT_IN2  A4
#define FRONT_RIGHT_EN   A5

// =====================================================
// TB6600 / LINEER AKTUATOR
// =====================================================

#define STEP_PIN D12
#define DIR_PIN  D13

// Lineerin tam stroku (adim). Web'den kalibre edilir, robot hafizasinda saklanir.
volatile long linearMaxSteps = 16000;
// Yazilim siniri: acikken lineer 0 ile en ust arasinda kendiliginden durur.
// Sadece konum dogrulandiktan sonra (EN ALT = 0 ya da konum duzeltme) devreye girer.
volatile bool linearLimitsOn = true;
volatile bool linearHomed = false;

const uint32_t LINEAR_START_HZ = 500;
const uint32_t LINEAR_SPEED_MIN_HZ = 300;
const uint32_t LINEAR_SPEED_MAX_HZ = 10000;
const uint32_t LINEAR_TIMER_TICK_HZ = 10000;
const uint32_t LINEAR_TIMER_PERIOD_US = 1000000UL / LINEAR_TIMER_TICK_HZ;
const uint32_t LINEAR_PULSE_HIGH_US = 5;
const uint32_t LINEAR_ACCEL_HZ_PER_SEC = 1200;
const uint32_t LINEAR_RAMP_INTERVAL_MS = 20;

volatile uint32_t linearTargetPulseHz = 2000;

// Yukari yon ters ise -1 yap.
const int UP_DIRECTION = +1;

volatile long linearPosition = 0;
volatile int linearDirection = 0;
volatile bool linearPulseEnabled = false;
volatile uint32_t linearCurrentPulseHz = LINEAR_START_HZ;
volatile uint32_t linearPhaseAccumulator = 0;

esp_timer_handle_t linearPulseTimer = nullptr;

// Hedefe git: -1 = kapali, >=0 = bu adimda dur
volatile long linearGotoTarget = -1;

unsigned long lastLinearRampUpdate = 0;

// =====================================================
// ARDUINO UNO SENSOR KOPRUSU (UART)
//   Uno D3 --[1k]--+-- D8 (RX)      +--[2k]--GND   (gerilim bolucu SART)
//   Uno D2 --------- D9 (TX)
//   Uno GND -------- GND
// =====================================================

#define SENSOR_RX_PIN D8
#define SENSOR_TX_PIN D9
const long SENSOR_BAUD = 9600;
HardwareSerial SensorSerial(1);

volatile float sensorCm = -1;        // -1 = hedef yok / menzil disi
volatile float sensorDeg = 0;        // toplam donus acisi
volatile bool  sensorDistValid = false;
unsigned long lastSensorMs = 0;      // son gecerli paketin zamani
uint32_t sensorPackets = 0;
String sensorBuf;

bool sensorLive() { return (millis() - lastSensorMs) < 1000; }

void sensorSend(char c) { SensorSerial.write(c); SensorSerial.write('\n'); }

void updateSensorLink() {
  while (SensorSerial.available()) {
    char c = (char)SensorSerial.read();
    if (c == '\n' || c == '\r') {
      // Beklenen bicim: S,<mm>,<aci*10>,<gecerli>,<sayac>
      if (sensorBuf.length() > 4 && sensorBuf.charAt(0) == 'S') {
        int p1 = sensorBuf.indexOf(',');
        int p2 = sensorBuf.indexOf(',', p1 + 1);
        int p3 = sensorBuf.indexOf(',', p2 + 1);
        if (p1 > 0 && p2 > p1 && p3 > p2) {
          long mm  = sensorBuf.substring(p1 + 1, p2).toInt();
          long d10 = sensorBuf.substring(p2 + 1, p3).toInt();
          sensorDistValid = (mm >= 0);
          sensorCm = sensorDistValid ? (mm / 10.0f) : -1.0f;
          sensorDeg = d10 / 10.0f;
          lastSensorMs = millis();
          sensorPackets++;
        }
      }
      sensorBuf = "";
    }
    else if (sensorBuf.length() < 60) sensorBuf += c;
  }
}

// =====================================================
// SURUS AYARLARI
// =====================================================

int motorSpeed = 255;

unsigned long lastDriveCommand = 0;
const unsigned long DRIVE_TIMEOUT = 700;
bool driveMoving = false;

// Kalibrasyon (sensor yoksa sureye gore surus icin)
float driveCmPerSec = 40.0f;      // %100 hizda cm/sn
float driveDegPerSec = 120.0f;    // %100 hizda derece/sn

// =====================================================
// SENSORLU HAREKET (kapali cevrim)
// =====================================================

// Asagida kullanilan surus fonksiyonlari daha ilerde tanimli:
void tankDrive(int leftSpeed, int rightSpeed);
void stopDrive();

enum MoveKind { MV_NONE, MV_FWD, MV_BACK, MV_LEFT, MV_RIGHT };
MoveKind moveKind = MV_NONE;
bool moveBySensor = false;        // true: sensorle, false: sureye gore
float moveTarget = 0;             // hedef: cm ya da derece
float moveStartCm = 0, moveStartDeg = 0, moveDoneAmount = 0;
int movePower = 255;
unsigned long moveStartMs = 0, moveTimeoutMs = 0, moveEndMs = 0;
String moveNote = "-";
float headingHold = 0;            // duz giderken korunacak aci
bool headingHoldOn = false;

// Duz giderken yon duzeltme kazanci (PWM / derece)
const float HEADING_GAIN = 6.0f;
const int   HEADING_MAX_CORR = 90;

void stopMove(const char* why) {
  if (moveKind != MV_NONE) {
    Serial.print("HAREKET BITTI: ");
    Serial.println(why);
  }
  moveKind = MV_NONE;
  headingHoldOn = false;
  stopDrive();
  moveNote = why;
}

// tip: 1 ileri, 2 geri, 3 sola, 4 saga | miktar: cm ya da derece | hiz: 5-100
void startMove(int tip, float amount, int speedPct) {
  stopDrive();
  amount = fabsf(amount);
  speedPct = constrain(speedPct, 5, 100);
  movePower = 255 * speedPct / 100;

  moveKind = (tip == 1) ? MV_FWD : (tip == 2) ? MV_BACK : (tip == 3) ? MV_LEFT : MV_RIGHT;
  moveTarget = amount;
  moveDoneAmount = 0;
  moveStartMs = millis();
  moveStartCm = sensorCm;
  moveStartDeg = sensorDeg;

  bool live = sensorLive();
  if (moveKind == MV_LEFT || moveKind == MV_RIGHT) {
    // Donus: jiroskop her yerde calisir
    moveBySensor = live;
    moveTimeoutMs = (unsigned long)(amount / (driveDegPerSec * speedPct / 100.0f) * 1000.0f * 2.5f) + 1500;
    headingHoldOn = false;
  }
  else {
    // Ileri/geri: sadece karsida olculebilir bir hedef varsa sensorle
    moveBySensor = live && sensorDistValid && sensorCm > 5;
    moveTimeoutMs = (unsigned long)(amount / (driveCmPerSec * speedPct / 100.0f) * 1000.0f * 2.5f) + 1500;
    headingHold = sensorDeg;
    headingHoldOn = live;      // jiroskop varsa duz gitmeyi de duzeltiriz
  }

  driveMoving = true;
  lastDriveCommand = millis();

  Serial.printf("HAREKET BASLADI: tip=%d miktar=%.1f hiz=%d%% yontem=%s\n",
                tip, amount, speedPct, moveBySensor ? "SENSOR" : "SURE");
}

void updateMove() {
  if (moveKind == MV_NONE) return;

  unsigned long now = millis();
  lastDriveCommand = now;                 // surus watchdog'u tetiklenmesin
  float elapsed = (now - moveStartMs) / 1000.0f;

  // --- ne kadar ilerledik? ---
  if (moveKind == MV_LEFT || moveKind == MV_RIGHT) {
    if (moveBySensor && sensorLive()) moveDoneAmount = fabsf(sensorDeg - moveStartDeg);
    else moveDoneAmount = elapsed * driveDegPerSec * movePower / 255.0f;
  }
  else {
    if (moveBySensor && sensorLive() && sensorDistValid) {
      float d = (moveKind == MV_FWD) ? (moveStartCm - sensorCm) : (sensorCm - moveStartCm);
      if (d > -5) moveDoneAmount = d;     // hedefi kaybettiyse eski degeri koru
    }
    else moveDoneAmount = elapsed * driveCmPerSec * movePower / 255.0f;
  }

  // --- bitti mi? ---
  if (moveDoneAmount >= moveTarget) { stopMove("hedefe ulasildi"); return; }
  if (now - moveStartMs > moveTimeoutMs) { stopMove("sure asimi (guvenlik)"); return; }

  // --- yavaslama: son 20 cm / 25 derecede gucu dusur ---
  float remain = moveTarget - moveDoneAmount;
  float slowZone = (moveKind == MV_LEFT || moveKind == MV_RIGHT) ? 25.0f : 20.0f;
  int p = movePower;
  if (remain < slowZone) {
    int pmin = (moveKind == MV_LEFT || moveKind == MV_RIGHT) ? 150 : 120;
    p = pmin + (int)((movePower - pmin) * (remain / slowZone));
    if (p > movePower) p = movePower;
  }

  // --- motorlara uygula ---
  if (moveKind == MV_LEFT)       tankDrive(-p,  p);
  else if (moveKind == MV_RIGHT) tankDrive( p, -p);
  else {
    int corr = 0;
    if (headingHoldOn && sensorLive()) {
      float err = sensorDeg - headingHold;             // + ise sola kaymis
      corr = (int)constrain(err * HEADING_GAIN, (float)-HEADING_MAX_CORR, (float)HEADING_MAX_CORR);
    }
    if (moveKind == MV_FWD) tankDrive(constrain(p + corr, -255, 255), constrain(p - corr, -255, 255));
    else                    tankDrive(constrain(-p + corr, -255, 255), constrain(-p - corr, -255, 255));
  }
}

// =====================================================
// RFID RC522
// =====================================================

#define RFID_SS  A6
#define RFID_RST A7

MFRC522 rfid(RFID_SS, RFID_RST);

byte rfidVersion = 0;
bool rfidCommunication = false;
String lastRFIDUID = "HENUZ YOK";
unsigned long lastRFIDReadMs = 0;

// =====================================================
// ORTAK I2C: OLED + PCA9685
// =====================================================

TwoWire I2C_BUS = TwoWire(1);

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_BUS, OLED_RESET);

bool oledInitialized = false;
bool oledBusOK = false;
unsigned long oledTemporaryUntil = 0;

// =====================================================
// SISTEM SAGLIK
// =====================================================

bool pcaInitialized = false;
bool pcaBusOK = false;
bool wifiAPOK = false;

unsigned long lastHealthCheck = 0;
const unsigned long HEALTH_CHECK_INTERVAL = 1500;

// =====================================================
// PCA9685 + SERVOLAR
// =====================================================

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41, I2C_BUS);

#define CH_MG995     4
#define CH_MG945     5
#define CH_MG996R    6
#define CH_QY3240MG  7

const float PCA_FREQ_HZ = 50.0f;

const uint16_t GLOBAL_MIN_US = 500;
const uint16_t GLOBAL_MAX_US = 2500;

ServoState s4 = { CH_MG995,    "MG995",    500, 2500, 1500.0f, 1500.0f, false };
ServoState s5 = { CH_MG945,    "MG945",    500, 2500, 1500.0f, 1500.0f, false };
ServoState s6 = { CH_MG996R,   "MG996R",   500, 2500, 1500.0f, 1500.0f, false };
ServoState s7 = { CH_QY3240MG, "QY3240MG", 500, 2500, 1500.0f, 1500.0f, false };

const float SERVO_SPEED_US_PER_SEC = 650.0f;

const uint32_t MOTION_INTERVAL_MS = 20;
uint32_t lastMotionUpdate = 0;

const uint32_t SERVO_HOLD_REFRESH_MS = 500;
uint32_t lastServoHoldRefresh = 0;

// Acilis pozu (us). Web'den kaydedilir (/startup), robot hafizasinda saklanir.
uint16_t STARTUP_US_CH4 = 1500;
uint16_t STARTUP_US_CH5 = 1500;
uint16_t STARTUP_US_CH6 = 1500;
uint16_t STARTUP_US_CH7 = 1500;

// =====================================================
// OLED + SAGLIK YARDIMCILARI
// =====================================================

void tryRecoverPCA();

bool i2cAddressResponds(uint8_t address) {
  I2C_BUS.beginTransmission(address);
  return (I2C_BUS.endTransmission() == 0);
}

void oledWrite(String line1, String line2 = "", String line3 = "") {
  if (!oledInitialized || !oledBusOK) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 5);
  display.println(line1);
  display.setCursor(0, 25);
  display.println(line2);
  display.setCursor(0, 45);
  display.println(line3);
  display.display();
}

void showSystemHealthOnOLED() {
  if (!oledInitialized || !oledBusOK) return;

  if (!pcaInitialized || !pcaBusOK) {
    oledWrite("SISTEM UYARI", "PCA9685 YOK", "SERVO KONTROL");
    return;
  }

  if (!rfidCommunication) {
    oledWrite("SISTEM UYARI", "RFID SPI HATA", "BAGLANTI KONTROL");
    return;
  }

  String versionText = "RFID:0x" + String(rfidVersion, HEX);
  versionText.toUpperCase();
  oledWrite("ROBOLIG HAZIR", versionText, "KART OKUT");
}

void refreshSystemHealth(bool force = false) {
  unsigned long now = millis();
  if (!force && (now - lastHealthCheck < HEALTH_CHECK_INTERVAL)) return;
  lastHealthCheck = now;

  bool previousOLED = oledBusOK;
  bool previousPCA  = pcaBusOK;
  bool previousRFID = rfidCommunication;

  oledBusOK = oledInitialized && i2cAddressResponds(OLED_ADDRESS);

  tryRecoverPCA();

  rfidVersion = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  rfidCommunication = !(rfidVersion == 0x00 || rfidVersion == 0xFF);

  wifi_mode_t mode = WiFi.getMode();
  IPAddress apIP = WiFi.softAPIP();

  wifiAPOK =
    (mode == WIFI_AP || mode == WIFI_AP_STA) &&
    !(apIP[0] == 0 && apIP[1] == 0 && apIP[2] == 0 && apIP[3] == 0);

  if (oledTemporaryUntil == 0 &&
      (force || previousOLED != oledBusOK || previousPCA != pcaBusOK || previousRFID != rfidCommunication)) {
    showSystemHealthOnOLED();
  }
}

// =====================================================
// SERVO YARDIMCI FONKSIYONLARI
// =====================================================

ServoState* getServo(int ch) {
  if (ch == 4) return &s4;
  if (ch == 5) return &s5;
  if (ch == 6) return &s6;
  if (ch == 7) return &s7;
  return nullptr;
}

float angleToUs(const ServoState &s, float angle) {
  angle = constrain(angle, 0.0f, 180.0f);
  return (float)s.minUs + (angle / 180.0f) * ((float)s.maxUs - (float)s.minUs);
}

void writeServoPulse(const ServoState &s) {
  if (!pcaInitialized || !pcaBusOK) return;
  uint16_t pulse = (uint16_t)(constrain(s.currentUs, (float)s.minUs, (float)s.maxUs) + 0.5f);
  pwm.writeMicroseconds(s.channel, pulse);
}

void servoOff(ServoState &s) {
  if (pcaInitialized && pcaBusOK) pwm.setPWM(s.channel, 0, 0);
  s.active = false;
}

void servoOffAll() {
  servoOff(s4);
  servoOff(s5);
  servoOff(s6);
  servoOff(s7);
}

void applyAllServoPulsesRaw() {
  if (!pcaInitialized || !pcaBusOK) return;
  if (s4.active) pwm.writeMicroseconds(s4.channel, (uint16_t)(s4.currentUs + 0.5f));
  if (s5.active) pwm.writeMicroseconds(s5.channel, (uint16_t)(s5.currentUs + 0.5f));
  if (s6.active) pwm.writeMicroseconds(s6.channel, (uint16_t)(s6.currentUs + 0.5f));
  if (s7.active) pwm.writeMicroseconds(s7.channel, (uint16_t)(s7.currentUs + 0.5f));
}

void centerAllServosAtStartup() {
  s4.currentUs = STARTUP_US_CH4; s4.targetUs = STARTUP_US_CH4; s4.active = true;
  s5.currentUs = STARTUP_US_CH5; s5.targetUs = STARTUP_US_CH5; s5.active = true;
  s6.currentUs = STARTUP_US_CH6; s6.targetUs = STARTUP_US_CH6; s6.active = true;
  s7.currentUs = STARTUP_US_CH7; s7.targetUs = STARTUP_US_CH7; s7.active = true;

  for (int i = 0; i < 4; i++) {
    pwm.writeMicroseconds(CH_MG995,    STARTUP_US_CH4);
    pwm.writeMicroseconds(CH_MG945,    STARTUP_US_CH5);
    pwm.writeMicroseconds(CH_MG996R,   STARTUP_US_CH6);
    pwm.writeMicroseconds(CH_QY3240MG, STARTUP_US_CH7);
    delay(60);
  }

  lastServoHoldRefresh = millis();
}

void refreshServoHoldOutputs() {
  uint32_t now = millis();
  if (now - lastServoHoldRefresh < SERVO_HOLD_REFRESH_MS) return;
  lastServoHoldRefresh = now;
  if (pcaInitialized && pcaBusOK) applyAllServoPulsesRaw();
}

void tryRecoverPCA() {
  bool ack = i2cAddressResponds(0x41);

  if (!ack) {
    pcaBusOK = false;
    return;
  }

  if (!pcaInitialized || !pcaBusOK) {
    pcaInitialized = pwm.begin();
    delay(5);
    pcaBusOK = pcaInitialized && i2cAddressResponds(0x41);

    if (pcaBusOK) {
      pwm.setPWMFreq(PCA_FREQ_HZ);
      delay(10);
      applyAllServoPulsesRaw();
      Serial.println("PCA9685 yeniden baglandi -> servo HOLD geri yuklendi.");
    }
  }
  else {
    pcaBusOK = true;
  }
}

void holdAllServos() {
  if (s4.active) s4.targetUs = s4.currentUs;
  if (s5.active) s5.targetUs = s5.currentUs;
  if (s6.active) s6.targetUs = s6.currentUs;
  if (s7.active) s7.targetUs = s7.currentUs;
}

void setTargetAngle(ServoState &s, float angle) {
  angle = constrain(angle, 0.0f, 180.0f);
  s.targetUs = angleToUs(s, angle);

  if (!s.active) {
    s.currentUs = 1500.0f;
    s.active = true;
    writeServoPulse(s);
  }
}

void setTargetPulse(ServoState &s, int pulseUs) {
  pulseUs = constrain(pulseUs, (int)s.minUs, (int)s.maxUs);
  s.targetUs = (float)pulseUs;

  if (!s.active) {
    s.currentUs = 1500.0f;
    s.active = true;
    writeServoPulse(s);
  }
}

void updateOneServo(ServoState &s, float maxStepUs) {
  if (!s.active) return;

  float diff = s.targetUs - s.currentUs;

  if (diff > maxStepUs)       s.currentUs += maxStepUs;
  else if (diff < -maxStepUs) s.currentUs -= maxStepUs;
  else                        s.currentUs = s.targetUs;

  writeServoPulse(s);
}

void updateServoMotion() {
  uint32_t now = millis();
  if (now - lastMotionUpdate < MOTION_INTERVAL_MS) return;

  uint32_t elapsedMs = now - lastMotionUpdate;
  lastMotionUpdate = now;

  float maxStepUs = SERVO_SPEED_US_PER_SEC * ((float)elapsedMs / 1000.0f);

  updateOneServo(s4, maxStepUs);
  updateOneServo(s5, maxStepUs);
  updateOneServo(s6, maxStepUs);
  updateOneServo(s7, maxStepUs);
}

// =====================================================
// DC MOTOR KONTROL
// =====================================================

void setMotor(int in1, int in2, int en, int value) {
  value = constrain(value, -255, 255);

  if (value > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(en, value);
  }
  else if (value < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(en, abs(value));
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0);
  }
}

void tankDrive(int leftSpeed, int rightSpeed) {
  setMotor(REAR_LEFT_IN1, REAR_LEFT_IN2, REAR_LEFT_EN, leftSpeed);
  setMotor(FRONT_LEFT_IN1, FRONT_LEFT_IN2, FRONT_LEFT_EN, leftSpeed);

  setMotor(REAR_RIGHT_IN1, REAR_RIGHT_IN2, REAR_RIGHT_EN, rightSpeed);
  setMotor(FRONT_RIGHT_IN1, FRONT_RIGHT_IN2, FRONT_RIGHT_EN, rightSpeed);
}

void stopDrive() {
  tankDrive(0, 0);
  driveMoving = false;
}

// =====================================================
// LINEER AKTUATOR
// =====================================================

void stopLinear();

void linearPulseCallback(void* arg) {
  if (!linearPulseEnabled || linearDirection == 0) return;

  linearPhaseAccumulator += linearCurrentPulseHz;
  if (linearPhaseAccumulator < LINEAR_TIMER_TICK_HZ) return;
  linearPhaseAccumulator -= LINEAR_TIMER_TICK_HZ;

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(LINEAR_PULSE_HIGH_US);
  digitalWrite(STEP_PIN, LOW);

  if (linearDirection == UP_DIRECTION) linearPosition++;
  else linearPosition--;

  // Yazilim siniri
  if (linearLimitsOn && linearHomed) {
    if ((linearDirection == UP_DIRECTION && linearPosition >= linearMaxSteps) ||
        (linearDirection != UP_DIRECTION && linearPosition <= 0)) {
      linearPulseEnabled = false;
      linearDirection = 0;
      linearGotoTarget = -1;
      return;
    }
  }

  // Hedefe ulasildiysa dur
  if (linearGotoTarget >= 0) {
    bool up = (linearDirection == UP_DIRECTION);
    if ((up && linearPosition >= linearGotoTarget) || (!up && linearPosition <= linearGotoTarget)) {
      linearPulseEnabled = false;
      linearDirection = 0;
      linearGotoTarget = -1;
    }
  }
}

void startLinear(int direction) {
  if (direction != UP_DIRECTION && direction != -UP_DIRECTION) return;

  // Sinirdaysa disari dogru hareket etme
  if (linearLimitsOn && linearHomed) {
    if ((direction == UP_DIRECTION && linearPosition >= linearMaxSteps) ||
        (direction != UP_DIRECTION && linearPosition <= 0)) {
      stopLinear();
      return;
    }
  }
  if (linearPulseEnabled && linearDirection == direction) { linearGotoTarget = -1; return; }

  linearGotoTarget = -1;   // elle hareket hedefi iptal eder
  linearPulseEnabled = false;
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, direction == UP_DIRECTION ? HIGH : LOW);
  delayMicroseconds(20);

  linearDirection = direction;
  uint32_t target = linearTargetPulseHz;
  linearCurrentPulseHz = (target < LINEAR_START_HZ) ? target : LINEAR_START_HZ;
  linearPhaseAccumulator = 0;
  lastLinearRampUpdate = millis();
  linearPulseEnabled = true;
}

void updateLinearRamp() {
  if (!linearPulseEnabled || linearDirection == 0) return;

  unsigned long now = millis();
  if (now - lastLinearRampUpdate < LINEAR_RAMP_INTERVAL_MS) return;

  unsigned long elapsedMs = now - lastLinearRampUpdate;
  lastLinearRampUpdate = now;

  uint32_t target = linearTargetPulseHz;
  uint32_t current = linearCurrentPulseHz;

  // Hedefe yaklasirken yumusak yavasla: v = sqrt(2 * ivme * kalan)
  long gotoT = linearGotoTarget;
  if (gotoT >= 0) {
    long remaining = labs(gotoT - (long)linearPosition);
    uint32_t vmax = (uint32_t)sqrtf(2.0f * (float)LINEAR_ACCEL_HZ_PER_SEC * (float)remaining) + 250;
    if (target > vmax) target = vmax;
  }

  uint32_t delta = (LINEAR_ACCEL_HZ_PER_SEC * elapsedMs) / 1000UL;
  if (delta < 1) delta = 1;

  if (current < target) {
    uint32_t next = current + delta;
    linearCurrentPulseHz = (next > target) ? target : next;
  }
  else if (current > target) {
    uint32_t next = (current > delta) ? current - delta : target;
    linearCurrentPulseHz = (next < target) ? target : next;
  }
}

void stopLinear() {
  linearGotoTarget = -1;
  linearPulseEnabled = false;
  linearDirection = 0;
  linearCurrentPulseHz = (linearTargetPulseHz < LINEAR_START_HZ) ? linearTargetPulseHz : LINEAR_START_HZ;
  linearPhaseAccumulator = 0;
  digitalWrite(STEP_PIN, LOW);
}

// =====================================================
// RFID
// =====================================================

String getRFIDUID() {
  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uid += " ";
  }

  uid.toUpperCase();
  return uid;
}

void updateRFID() {
  if (!rfidCommunication) return;
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  lastRFIDUID = getRFIDUID();
  lastRFIDReadMs = millis();

  Serial.print("RFID UID = ");
  Serial.println(lastRFIDUID);

  oledWrite("KART OKUNDU!", "UID:", lastRFIDUID);
  oledTemporaryUntil = millis() + 2000UL;

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void updateOLEDTimeout() {
  if (oledTemporaryUntil != 0 && (long)(millis() - oledTemporaryUntil) >= 0) {
    oledTemporaryUntil = 0;
    showSystemHealthOnOLED();
  }
}

// =====================================================
// ACIL DUR
// =====================================================

void emergencyStop() {
  moveKind = MV_NONE;
  headingHoldOn = false;
  stopDrive();
  stopLinear();
  holdAllServos();
  Serial.println("!!! ACIL DUR - SURUS + LINEER DURDU, SERVOLAR HOLD !!!");
}

// =====================================================
// ESP-NOW KUMANDA (ESP32 -> DENEYAP)
// =====================================================

const uint8_t REMOTE_MAC[6] = {0x88, 0x57, 0x21, 0xBF, 0x1C, 0x60};
const uint8_t ESPNOW_CHANNEL = 1;

#define BIT_JOY    0x01
#define BIT_LINEER 0x02
#define BIT_CH4    0x04
#define BIT_CH5    0x08
#define BIT_CH6    0x10
#define BIT_CH7    0x20

const unsigned long REMOTE_TIMEOUT_MS = 500;
const int JOY_DEADZONE = 15;
const int REMOTE_SERVO_DEADBAND = 15;
const int REMOTE_DRIVE_MAX = 255;

portMUX_TYPE remoteMux = portMUX_INITIALIZER_UNLOCKED;
KumandaPaketi remoteBuffer;
volatile bool remoteNewPacket = false;

unsigned long lastRemotePacketMs = 0;
bool remoteConnected = false;
bool remoteDriving = false;
int remoteLinearDir = 0;
int lastRemoteServoUs[4] = {-10000, -10000, -10000, -10000};

void handleRemoteData(const uint8_t* mac, const uint8_t* data, int len) {
  if (len != (int)sizeof(KumandaPaketi)) return;
  if (memcmp(mac, REMOTE_MAC, 6) != 0) return;

  portENTER_CRITICAL(&remoteMux);
  memcpy(&remoteBuffer, data, sizeof(KumandaPaketi));
  remoteNewPacket = true;
  portEXIT_CRITICAL(&remoteMux);
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
void onRemoteRecv(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  handleRemoteData(info->src_addr, data, len);
}
#else
void onRemoteRecv(const uint8_t* mac, const uint8_t* data, int len) {
  handleRemoteData(mac, data, len);
}
#endif

void applyRemotePacket(const KumandaPaketi &p) {
  if (p.gecerli & BIT_JOY) {
    int x = constrain((int)p.joyX, -100, 100);
    int y = constrain((int)p.joyY, -100, 100);

    if (abs(x) < JOY_DEADZONE) x = 0;
    if (abs(y) < JOY_DEADZONE) y = 0;

    if (x == 0 && y == 0) {
      if (remoteDriving) {
        stopDrive();
        remoteDriving = false;
      }
    }
    else {
      int left  = constrain(y + x, -100, 100);
      int right = constrain(y - x, -100, 100);
      tankDrive(left * REMOTE_DRIVE_MAX / 100, right * REMOTE_DRIVE_MAX / 100);
      driveMoving = true;
      lastDriveCommand = millis();
      remoteDriving = true;
    }
  }

  if (p.gecerli & BIT_LINEER) {
    int dir = constrain((int)p.lineer, -1, 1);
    if (dir != remoteLinearDir) {
      if (dir == 0) stopLinear();
      else startLinear(dir > 0 ? UP_DIRECTION : -UP_DIRECTION);
      remoteLinearDir = dir;
    }
  }

  if (pcaInitialized && pcaBusOK) {
    const uint8_t bits[4] = {BIT_CH4, BIT_CH5, BIT_CH6, BIT_CH7};
    for (int i = 0; i < 4; i++) {
      if (!(p.gecerli & bits[i])) continue;
      int us = constrain((int)p.servoUs[i], (int)GLOBAL_MIN_US, (int)GLOBAL_MAX_US);
      if (abs(us - lastRemoteServoUs[i]) >= REMOTE_SERVO_DEADBAND) {
        ServoState* s = getServo(4 + i);
        if (s != nullptr) setTargetPulse(*s, us);
        lastRemoteServoUs[i] = us;
      }
    }
  }
}

void updateRemote() {
  if (remoteNewPacket) {
    KumandaPaketi p;

    portENTER_CRITICAL(&remoteMux);
    memcpy(&p, &remoteBuffer, sizeof(KumandaPaketi));
    remoteNewPacket = false;
    portEXIT_CRITICAL(&remoteMux);

    lastRemotePacketMs = millis();

    if (!remoteConnected) {
      remoteConnected = true;
      Serial.println("KUMANDA BAGLANDI (ESP-NOW)");
    }

    applyRemotePacket(p);
  }
  else if (remoteConnected && (millis() - lastRemotePacketMs > REMOTE_TIMEOUT_MS)) {
    remoteConnected = false;

    if (remoteDriving) {
      stopDrive();
      remoteDriving = false;
    }

    if (remoteLinearDir != 0) {
      stopLinear();
      remoteLinearDir = 0;
    }

    Serial.println("KUMANDA SINYALI KOPTU -> surus + lineer durdu, servolar HOLD");
  }
}

// =====================================================
// WEB SAYFASI (3D ARAYUZ)
// Sayfanin tamami web_sayfasi.h dosyasinda (ayni klasorde olmali).
// =====================================================

#include "web_sayfasi.h"

// =====================================================
// HTTP - SURUS
// =====================================================

void handleDrive() {
  String cmd = server.arg("cmd");
  lastDriveCommand = millis();
  if (moveKind != MV_NONE) stopMove("elle surus komutu geldi");

  if (cmd == "forward") {
    tankDrive(motorSpeed, motorSpeed);
    driveMoving = true;
  }
  else if (cmd == "back") {
    tankDrive(-motorSpeed, -motorSpeed);
    driveMoving = true;
  }
  else if (cmd == "left") {
    tankDrive(-motorSpeed, motorSpeed);
    driveMoving = true;
  }
  else if (cmd == "right") {
    tankDrive(motorSpeed, -motorSpeed);
    driveMoving = true;
  }
  else {
    stopDrive();
  }

  server.send(200, "text/plain", "OK");
}

// YENI: dogrudan sol/sag motor gucu (-255..255) - WASD ve yay donus icin
void handleTank() {
  if (moveKind != MV_NONE) stopMove("elle surus komutu geldi");
  int l = constrain(server.arg("l").toInt(), -255, 255);
  int r = constrain(server.arg("r").toInt(), -255, 255);
  lastDriveCommand = millis();

  if (l == 0 && r == 0) {
    stopDrive();
  }
  else {
    tankDrive(l, r);
    driveMoving = true;
  }

  server.send(200, "text/plain", "OK");
}

void handleSpeed() {
  if (server.hasArg("v")) {
    motorSpeed = constrain(server.arg("v").toInt(), 0, 255);
  }
  server.send(200, "text/plain", "OK");
}

// =====================================================
// HTTP - LINEER
// =====================================================

void handleLinear() {
  String cmd = server.arg("cmd");

  if (cmd == "up") startLinear(UP_DIRECTION);
  else if (cmd == "down") startLinear(-UP_DIRECTION);
  else stopLinear();

  server.send(200, "text/plain", "OK");
}

void handleZero() {
  if (linearPulseEnabled) {
    server.send(409, "text/plain", "Lineer hareket ediyor. Once durdurun.");
    return;
  }

  linearPosition = 0;
  linearHomed = true;
  Serial.println("LINEER POZISYON = 0 (EN ALT)");
  server.send(200, "text/plain", "Mevcut konum 0 olarak ayarlandi.");
}

// YENI: "Adim kacti" duzeltmesi - sayaci verilen degere ayarlar
void handleSetPos() {
  if (linearPulseEnabled) {
    server.send(409, "text/plain", "Lineer hareket ediyor. Once durdurun.");
    return;
  }

  if (!server.hasArg("v")) {
    server.send(400, "text/plain", "Eksik deger");
    return;
  }

  long v = server.arg("v").toInt();
  v = constrain(v, 0L, (long)linearMaxSteps);
  linearPosition = v;
  linearHomed = true;

  Serial.print("LINEER POZISYON DUZELTILDI = ");
  Serial.println(v);

  server.send(200, "text/plain", "Konum " + String(v) + " adim olarak ayarlandi.");
}

// YENI: lineeri verilen adima gotur, orada yumusakca dur
void handleLinearGoto() {
  if (!server.hasArg("v")) {
    server.send(400, "text/plain", "Eksik deger");
    return;
  }

  long v = constrain(server.arg("v").toInt(), 0L, (long)linearMaxSteps);
  long pos = linearPosition;

  if (v == pos) {
    server.send(200, "text/plain", "Zaten hedefte");
    return;
  }

  int dir = (v > pos) ? UP_DIRECTION : -UP_DIRECTION;
  startLinear(dir);          // elle hedefi sifirlar...
  linearGotoTarget = v;      // ...sonra yeni hedefi kur

  server.send(200, "text/plain", "Hedef " + String(v) + " adim");
}

// YENI: tam strok (en ust adim) ayari. ?here=1 -> su anki konum, ?v=16000 -> elle
void handleLinMax() {
  long v = -1;
  if (server.hasArg("here") && server.arg("here") == "1") {
    if (linearPulseEnabled) {
      server.send(409, "text/plain", "Once lineeri durdurun");
      return;
    }
    v = linearPosition;
    if (v < 100) {
      server.send(400, "text/plain", "HATA: once EN ALT = 0 yapip lineeri en uste cikarin");
      return;
    }
  }
  else if (server.hasArg("v")) {
    v = server.arg("v").toInt();
  }
  if (v < 100 || v > 400000) {
    server.send(400, "text/plain", "Gecersiz adim sayisi");
    return;
  }
  linearMaxSteps = v;
  prefs.begin("robolig", false);
  prefs.putLong("linmax", v);
  prefs.end();
  Serial.print("LINEER TAM STROK = ");
  Serial.println(v);
  server.send(200, "text/plain", "En ust = " + String(v) + " adim olarak kaydedildi");
}

// YENI: acilis pozu kaydet (?c4=1500&c5=1500&c6=1500&c7=1500, mikro saniye)
void handleStartup() {
  const char* keys[4] = {"c4", "c5", "c6", "c7"};
  const char* pk[4]   = {"st4", "st5", "st6", "st7"};
  uint16_t* vals[4]   = {&STARTUP_US_CH4, &STARTUP_US_CH5, &STARTUP_US_CH6, &STARTUP_US_CH7};
  prefs.begin("robolig", false);
  String msg = "Acilis pozu kaydedildi:";
  for (int i = 0; i < 4; i++) {
    if (!server.hasArg(keys[i])) continue;
    int us = constrain((int)server.arg(keys[i]).toInt(), (int)GLOBAL_MIN_US, (int)GLOBAL_MAX_US);
    *vals[i] = (uint16_t)us;
    prefs.putUShort(pk[i], (uint16_t)us);
    msg += " CH" + String(4 + i) + "=" + String(us);
  }
  prefs.end();
  Serial.println(msg);
  server.send(200, "text/plain", msg);
}

// YENI: yazilim siniri ac/kapat (?on=1 / ?on=0)
void handleLinLimit() {
  linearLimitsOn = server.arg("on") == "1";
  prefs.begin("robolig", false);
  prefs.putBool("linlim", linearLimitsOn);
  prefs.end();
  server.send(200, "text/plain", linearLimitsOn ? "Sinir acik" : "Sinir kapali");
}

void handleLinearSpeed() {
  if (server.hasArg("v")) {
    long v = server.arg("v").toInt();
    v = constrain(v, (long)LINEAR_SPEED_MIN_HZ, (long)LINEAR_SPEED_MAX_HZ);
    linearTargetPulseHz = (uint32_t)v;
  }

  String json = "{";
  json += "\"target\":" + String((uint32_t)linearTargetPulseHz);
  json += ",\"current\":" + String((uint32_t)linearCurrentPulseHz);
  json += "}";
  server.send(200, "application/json", json);
}

// =====================================================
// HTTP - SERVO
// =====================================================

void handleAngle() {
  if (!pcaInitialized || !pcaBusOK) {
    server.send(503, "text/plain", "PCA9685 HATA");
    return;
  }

  if (!server.hasArg("ch") || !server.hasArg("value")) {
    server.send(400, "text/plain", "Eksik parametre");
    return;
  }

  ServoState* s = getServo(server.arg("ch").toInt());

  if (s == nullptr) {
    server.send(400, "text/plain", "Gecersiz kanal");
    return;
  }

  setTargetAngle(*s, server.arg("value").toFloat());
  server.send(200, "text/plain", "OK");
}

void handlePulse() {
  if (!pcaInitialized || !pcaBusOK) {
    server.send(503, "text/plain", "PCA9685 HATA");
    return;
  }

  if (!server.hasArg("ch") || !server.hasArg("us")) {
    server.send(400, "text/plain", "Eksik parametre");
    return;
  }

  ServoState* s = getServo(server.arg("ch").toInt());

  if (s == nullptr) {
    server.send(400, "text/plain", "Gecersiz kanal");
    return;
  }

  setTargetPulse(*s, server.arg("us").toInt());
  server.send(200, "text/plain", "OK");
}

void handleOff() {
  if (!pcaInitialized || !pcaBusOK) {
    server.send(503, "text/plain", "PCA9685 HATA");
    return;
  }

  if (!server.hasArg("ch")) {
    server.send(400, "text/plain", "Eksik kanal");
    return;
  }

  ServoState* s = getServo(server.arg("ch").toInt());

  if (s == nullptr) {
    server.send(400, "text/plain", "Gecersiz kanal");
    return;
  }

  servoOff(*s);
  server.send(200, "text/plain", "OFF");
}

void handleOffAll() {
  if (!pcaInitialized || !pcaBusOK) {
    server.send(503, "text/plain", "PCA9685 HATA");
    return;
  }

  servoOffAll();
  server.send(200, "text/plain", "ALL OFF");
}

// =====================================================
// HTTP - OLED TEST
// =====================================================

void handleOLEDTest() {
  refreshSystemHealth(true);

  if (!oledInitialized || !oledBusOK) {
    server.send(503, "text/plain", "OLED HATA: 0x3C I2C cevabi yok");
    return;
  }

  oledWrite("WEB OLED TEST", "OLED CALISIYOR", "ROBOLIG");
  oledTemporaryUntil = millis() + 2000UL;
  server.send(200, "text/plain", "OLED OK");
}

// =====================================================
// HTTP - DURUM
// =====================================================

void handleStatus() {
  String versionText = "0x" + String(rfidVersion, HEX);
  versionText.toUpperCase();

  String json = "{";
  json += "\"position\":" + String((long)linearPosition);
  json += ",\"linearMax\":" + String((long)linearMaxSteps);
  json += ",\"homed\":" + String(linearHomed ? "true" : "false");
  json += ",\"limits\":" + String(linearLimitsOn ? "true" : "false");
  json += ",\"linearGoto\":" + String((long)linearGotoTarget);
  json += ",\"linearDir\":" + String(linearPulseEnabled ? (linearDirection == UP_DIRECTION ? 1 : -1) : 0);
  json += ",\"driveSpeed\":" + String(motorSpeed);
  json += ",\"linearSpeed\":" + String((uint32_t)linearCurrentPulseHz);
  json += ",\"linearTarget\":" + String((uint32_t)linearTargetPulseHz);
  json += ",\"s4\":" + String((int)s4.currentUs);
  json += ",\"s5\":" + String((int)s5.currentUs);
  json += ",\"s6\":" + String((int)s6.currentUs);
  json += ",\"s7\":" + String((int)s7.currentUs);
  json += ",\"rfidVersion\":\"" + versionText + "\"";
  json += ",\"rfidUid\":\"" + lastRFIDUID + "\"";
  json += ",\"oledOK\":" + String((oledInitialized && oledBusOK) ? "true" : "false");
  json += ",\"pcaOK\":" + String((pcaInitialized && pcaBusOK) ? "true" : "false");
  json += ",\"rfidOK\":" + String(rfidCommunication ? "true" : "false");
  json += ",\"wifiOK\":" + String(wifiAPOK ? "true" : "false");
  json += ",\"remote\":" + String(remoteConnected ? "true" : "false");
  json += ",\"fw\":\"" + String(FW_VERSION) + "\"";
  json += ",\"sensor\":" + String(sensorLive() ? "true" : "false");
  json += ",\"sCm\":" + String(sensorDistValid ? sensorCm : -1, 1);
  json += ",\"sDeg\":" + String(sensorDeg, 1);
  json += ",\"sPkt\":" + String(sensorPackets);
  json += ",\"mv\":" + String((int)moveKind);
  json += ",\"mvDone\":" + String(moveDoneAmount, 1);
  json += ",\"mvTarget\":" + String(moveTarget, 1);
  json += ",\"mvSensor\":" + String(moveBySensor ? "true" : "false");
  json += ",\"mvNote\":\"" + moveNote + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// =====================================================
// HTTP - AYARLAR (web arayuzu ayarlari robotun hafizasinda saklanir)
// =====================================================

void handleCfgGet() {
  String out = "{}";
  prefs.begin("robolig", true);
  size_t n = prefs.getBytesLength("cfg");
  if (n > 0 && n < 16000) {
    std::unique_ptr<char[]> buf(new char[n + 1]);
    prefs.getBytes("cfg", buf.get(), n);
    buf[n] = 0;
    out = String(buf.get());
  }
  prefs.end();
  server.send(200, "application/json", out);
}

void handleCfgSet() {
  String v = server.arg("plain");
  if (v.length() == 0 || v.length() > 12000) {
    server.send(413, "text/plain", "Ayar verisi cok buyuk");
    return;
  }
  prefs.begin("robolig", false);
  size_t w = prefs.putBytes("cfg", v.c_str(), v.length());
  prefs.end();
  if (w == v.length()) server.send(200, "text/plain", "OK");
  else server.send(500, "text/plain", "Kaydedilemedi");
}

// =====================================================
// HTTP - WEB UZERINDEN KOD YUKLEME (OTA)
// =====================================================

void handleUpdateDone() {
  bool ok = !Update.hasError();
  server.sendHeader("Connection", "close");
  server.send(ok ? 200 : 500, "text/plain", ok ? "OK - yeniden baslatiliyor" : "HATA: yukleme basarisiz");
  if (ok) {
    Serial.println("OTA TAMAM -> YENIDEN BASLATILIYOR");
    delay(500);
    ESP.restart();
  }
}

void handleUpdateUpload() {
  HTTPUpload& up = server.upload();

  if (up.status == UPLOAD_FILE_START) {
    // Guvenlik: yukleme sirasinda her sey dursun
    stopDrive();
    stopLinear();
    holdAllServos();
    Serial.print("OTA BASLADI: ");
    Serial.println(up.filename);
    oledWrite("KOD YUKLENIYOR", "GUCU KESMEYIN", "");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
  }
  else if (up.status == UPLOAD_FILE_WRITE) {
    if (Update.write(up.buf, up.currentSize) != up.currentSize) Update.printError(Serial);
  }
  else if (up.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.print("OTA BOYUT: ");
      Serial.println((unsigned long)up.totalSize);
    }
    else {
      Update.printError(Serial);
    }
  }
  else if (up.status == UPLOAD_FILE_ABORTED) {
    Update.abort();
    Serial.println("OTA IPTAL EDILDI");
  }
}

// =====================================================
// HTTP - SENSORLU HAREKET
// =====================================================

// /move?t=1..4&v=100&sp=100   (t: 1 ileri, 2 geri, 3 sola, 4 saga)
void handleMove() {
  int t = server.arg("t").toInt();
  float v = server.arg("v").toFloat();
  int sp = server.hasArg("sp") ? server.arg("sp").toInt() : 100;

  if (t < 1 || t > 4 || v <= 0) {
    server.send(400, "text/plain", "Gecersiz hareket");
    return;
  }
  if (server.hasArg("cm")) driveCmPerSec = max(1.0f, server.arg("cm").toFloat());
  if (server.hasArg("dg")) driveDegPerSec = max(1.0f, server.arg("dg").toFloat());

  startMove(t, v, sp);
  server.send(200, "text/plain", moveBySensor ? "SENSOR" : "SURE");
}

void handleMoveStop() {
  stopMove("kullanici durdurdu");
  server.send(200, "text/plain", "DURDU");
}

// /sensor?cmd=zero | cal
void handleSensorCmd() {
  String c = server.arg("cmd");
  if (c == "zero") { sensorSend('Z'); server.send(200, "text/plain", "Aci sifirlandi"); }
  else if (c == "cal") { sensorSend('K'); server.send(200, "text/plain", "Jiroskop kalibre ediliyor, robotu 2 sn oynatmayin"); }
  else server.send(400, "text/plain", "Bilinmeyen komut");
}

void handleStopAll() {
  emergencyStop();
  server.send(200, "text/plain", "STOP");
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("====================================");
  Serial.println("ROBOLIG TUM SISTEM BASLATILIYOR");
  Serial.println("====================================");

  // L298N PINLER
  pinMode(REAR_LEFT_IN1, OUTPUT);
  pinMode(REAR_LEFT_IN2, OUTPUT);
  pinMode(REAR_LEFT_EN, OUTPUT);
  pinMode(REAR_RIGHT_IN1, OUTPUT);
  pinMode(REAR_RIGHT_IN2, OUTPUT);
  pinMode(REAR_RIGHT_EN, OUTPUT);
  pinMode(FRONT_LEFT_IN1, OUTPUT);
  pinMode(FRONT_LEFT_IN2, OUTPUT);
  pinMode(FRONT_LEFT_EN, OUTPUT);
  pinMode(FRONT_RIGHT_IN1, OUTPUT);
  pinMode(FRONT_RIGHT_IN2, OUTPUT);
  pinMode(FRONT_RIGHT_EN, OUTPUT);

  stopDrive();

  // Arduino Uno sensor koprusu (UART1)
  SensorSerial.begin(SENSOR_BAUD, SERIAL_8N1, SENSOR_RX_PIN, SENSOR_TX_PIN);
  Serial.println("SENSOR KOPRUSU: D8(RX) / D9(TX), 9600 baud bekleniyor");

  // Kayitli lineer ayarlari
  prefs.begin("robolig", true);
  linearMaxSteps = prefs.getLong("linmax", 16000);
  linearLimitsOn = prefs.getBool("linlim", true);
  STARTUP_US_CH4 = constrain(prefs.getUShort("st4", 1500), 500, 2500);
  STARTUP_US_CH5 = constrain(prefs.getUShort("st5", 1500), 500, 2500);
  STARTUP_US_CH6 = constrain(prefs.getUShort("st6", 1500), 500, 2500);
  STARTUP_US_CH7 = constrain(prefs.getUShort("st7", 1500), 500, 2500);
  prefs.end();
  Serial.printf("ACILIS POZU: CH4=%u CH5=%u CH6=%u CH7=%u us\n", STARTUP_US_CH4, STARTUP_US_CH5, STARTUP_US_CH6, STARTUP_US_CH7);
  Serial.print("LINEER TAM STROK: ");
  Serial.print((long)linearMaxSteps);
  Serial.println(linearLimitsOn ? " adim | yazilim siniri ACIK (EN ALT = 0 yapilinca devreye girer)" : " adim | yazilim siniri KAPALI");

  // LINEER / TB6600
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  esp_timer_create_args_t linearTimerArgs = {};
  linearTimerArgs.callback = &linearPulseCallback;
  linearTimerArgs.arg = nullptr;
  linearTimerArgs.dispatch_method = ESP_TIMER_TASK;
  linearTimerArgs.name = "linearWebSpeed";

  esp_err_t timerCreateResult = esp_timer_create(&linearTimerArgs, &linearPulseTimer);
  if (timerCreateResult == ESP_OK) {
    esp_err_t timerStartResult = esp_timer_start_periodic(linearPulseTimer, LINEAR_TIMER_PERIOD_US);
    if (timerStartResult == ESP_OK) Serial.println("LINEER TIMER HAZIR");
    else { Serial.print("HATA: Lineer timer baslatilamadi, kod="); Serial.println((int)timerStartResult); }
  } else {
    Serial.print("HATA: Lineer timer olusturulamadi, kod=");
    Serial.println((int)timerCreateResult);
  }

  // ORTAK I2C
  I2C_BUS.begin(47, 21, 100000);
  delay(50);

  s4.currentUs = STARTUP_US_CH4; s4.targetUs = STARTUP_US_CH4; s4.active = true;
  s5.currentUs = STARTUP_US_CH5; s5.targetUs = STARTUP_US_CH5; s5.active = true;
  s6.currentUs = STARTUP_US_CH6; s6.targetUs = STARTUP_US_CH6; s6.active = true;
  s7.currentUs = STARTUP_US_CH7; s7.targetUs = STARTUP_US_CH7; s7.active = true;

  pcaInitialized = pwm.begin();
  delay(20);
  pcaBusOK = pcaInitialized && i2cAddressResponds(0x41);

  if (!pcaInitialized || !pcaBusOK) {
    Serial.println("HATA: PCA9685 0x41 bulunamadi / I2C cevabi yok.");
  }
  else {
    pwm.setPWMFreq(PCA_FREQ_HZ);
    delay(350);
    centerAllServosAtStartup();
    Serial.println("SERVOLAR: CH4-CH7 1500 us merkezde HOLD.");
  }

  if (i2cAddressResponds(OLED_ADDRESS)) {
    oledInitialized = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS, true, false);
  }
  else {
    oledInitialized = false;
  }

  oledBusOK = oledInitialized && i2cAddressResponds(OLED_ADDRESS);

  if (!oledInitialized || !oledBusOK) Serial.println("HATA: OLED 0x3C bulunamadi / I2C cevabi yok.");
  else oledWrite("ROBOLIG", "SISTEM", "BASLATILIYOR");

  lastMotionUpdate = millis();

  // RFID
  SPI.begin();
  delay(100);
  rfid.PCD_Init();
  delay(200);
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);

  rfidVersion = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  rfidCommunication = !(rfidVersion == 0x00 || rfidVersion == 0xFF);

  Serial.print("RC522 VERSION = 0x");
  Serial.println(rfidVersion, HEX);

  if (rfidCommunication) Serial.println("RFID SPI cevap veriyor. Kart bekleniyor.");
  else Serial.println("UYARI: RFID SPI haberlesmesi yok.");

  showSystemHealthOnOLED();

  // WIFI AP + ESP-NOW
  WiFi.mode(WIFI_AP_STA);

  if (!WiFi.softAP(WIFI_NAME, WIFI_PASSWORD, ESPNOW_CHANNEL)) {
    Serial.println("HATA: WiFi baslatilamadi.");
  }

  Serial.print("WiFi: ");
  Serial.println(WIFI_NAME);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(onRemoteRecv);
    Serial.print("ESP-NOW HAZIR. Deneyap MAC: ");
    Serial.println(WiFi.macAddress());
  }
  else {
    Serial.println("HATA: ESP-NOW baslatilamadi.");
  }

  refreshSystemHealth(true);

  // WEB ROUTES
  server.on("/", []() { server.send_P(200, "text/html; charset=utf-8", PAGE); });

  server.on("/drive", handleDrive);
  server.on("/tank", handleTank);
  server.on("/speed", handleSpeed);
  server.on("/linear", handleLinear);
  server.on("/zero", handleZero);
  server.on("/setpos", handleSetPos);
  server.on("/lineargoto", handleLinearGoto);
  server.on("/linmax", handleLinMax);
  server.on("/linlimit", handleLinLimit);
  server.on("/startup", handleStartup);
  server.on("/move", handleMove);
  server.on("/movestop", handleMoveStop);
  server.on("/sensor", handleSensorCmd);
  server.on("/linearspeed", handleLinearSpeed);
  server.on("/angle", handleAngle);
  server.on("/pulse", handlePulse);
  server.on("/off", handleOff);
  server.on("/offall", handleOffAll);
  server.on("/oledtest", handleOLEDTest);
  server.on("/status", handleStatus);
  server.on("/stopall", handleStopAll);
  server.on("/cfg", HTTP_GET, handleCfgGet);
  server.on("/cfg", HTTP_POST, handleCfgSet);
  server.on("/update", HTTP_POST, handleUpdateDone, handleUpdateUpload);

  server.begin();

  Serial.println("WEB SERVER HAZIR - http://192.168.4.1");
  Serial.println("====================================");
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  server.handleClient();

  updateRemote();

  // SURUS WATCHDOG
  if (moveKind == MV_NONE && driveMoving && millis() - lastDriveCommand > DRIVE_TIMEOUT) {
    stopDrive();
  }

  updateSensorLink();
  updateMove();

  updateLinearRamp();
  updateServoMotion();
  refreshServoHoldOutputs();
  refreshSystemHealth();
  updateRFID();
  updateOLEDTimeout();
}
