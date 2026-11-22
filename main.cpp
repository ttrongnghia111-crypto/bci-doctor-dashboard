#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// =========================================================================
// KHAI BÁO NGUYÊN MẪU HÀM (FORWARD DECLARATIONS - FIX LỖI PLATFORMIO)
// =========================================================================
enum SensorInputMode : uint8_t {
  INPUT_HYBRID_BOTH = 0,
  INPUT_IR_ONLY     = 1,
  INPUT_NOD_ONLY    = 2
};

void triggerBuzzer(uint8_t pattern);
void updateBuzzer();
void processTCRT5000();
void setInputMode(SensorInputMode mode);
void broadcastInputMode();
bool checkTripleNodToUnlock(unsigned long nowMs);

void triggerLocalFatigueLock(unsigned long durationSec, String source);
void releaseLocalFatigueLock();
void writeI2CRegister(uint8_t devAddr, uint8_t regAddr, uint8_t val);
uint8_t readI2CRegister(uint8_t devAddr, uint8_t regAddr);
void initLSM6DS3();
void processLSM6DS3();
void setNodMode(uint8_t newMode);
void broadcastNodMode();
void setControlLock(bool lockState);
void broadcastControlLock();
void fireInstantNodEvent(bool isHeavyNod, float angleDeg, float peakDps, float peakTkeo, unsigned long latencyMs);
void initINA226();
void readINA226();
void syncSentenceToCloud(String sentence);
void syncTelemetryToCloud();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleRoot();
void cloudSyncTask(void * parameter);
void rgbRainbowTask(void * parameter);

// =========================================================================
// HỆ THỐNG HMI ESP32-S3 - BẢN HỢP NHẤT HOÀN HẢO:
// 1. GIAO TIẾP: Chỉ Gật Đầu Để Chọn (Siêu ổn định, không khóa nhầm khi gõ nhanh)
// 2. Y SINH LSM6DS3: Tự động đo Mỏi cơ CFI + Co giật/Té ngã + Chống loét 120p + Nhiệt độ
// 3. THỊ GIÁC MẮT MỜ: Hình vẽ SVG Mảng Lớn Tương Phản Cực Cao + Hỗ trợ Gắn Ảnh Thật
// =========================================================================

WiFiMulti wifiMulti;
WiFiClientSecure secureClient; 

// 1. Cấu hình Wi-Fi Router
const char* WIFI_SSID_1 = "iPhone";     
const char* WIFI_PASS_1 = "27112012"; 
const char* WIFI_SSID_2 = "Trong Nghia";     
const char* WIFI_PASS_2 = "109876543";
const char* WIFI_SSID_3 = "Tuyet Nghia";     
const char* WIFI_PASS_3 = "@tuyet1003@";   

const char* AP_SSID = "ESP32_BCI_SYSTEM";
const char* AP_PASS = "12345678";

// 2. ĐỊA CHỈ WEB SERVER CLOUD TRÊN RENDER
const char* CLOUD_SERVER_URL    = "https://bci-api-server.onrender.com/api/logs"; 
const char* CLOUD_TELEMETRY_URL = "https://bci-api-server.onrender.com/api/telemetry";
const char* CLOUD_COMMAND_URL   = "https://bci-api-server.onrender.com/api/command";

// Quản lý khóa nghỉ ngơi do kiệt sức (CFI >= 75% hoặc Cloud)
volatile bool isFatigueLocked = false;
unsigned long fatigueLockStartTime = 0;
unsigned long fatigueLockDurationMs = 30UL * 60UL * 1000UL;
String fatigueLockSource = "GLOBAL";
volatile bool cloudCommandOk = false;
int cloudCommandFailCount = 0;
unsigned long ignoreLockUntil = 0;

// Quản lý Khóa/Mở điều khiển thông minh (Từ bảnv1 - Chống khóa nhầm 100%):
// - Khi ĐANG MỞ: Gật nhanh liên hoàn thoải mái để chọn chữ, chỉ khóa khi Cúi giữ 1.2s hoặc bấm nút Khóa.
// - Khi ĐANG KHÓA: Gật 3 cái liên tiếp (trong 3.5s) hoặc Cúi giữ 1.2s để Mở khóa lại.
volatile bool isControlLocked = false;
unsigned long unlockNodTimestamps[3] = {0, 0, 0};
uint8_t unlockNodCount = 0;
const unsigned long UNLOCK_TRIPLE_WINDOW_MS = 3500UL;

// Biến phát hiện "Cúi đầu giữ yên 1.2 giây" (Long Nod Hold) để Khóa/Mở chủ động
unsigned long nodHoldStartMs = 0;
bool nodHoldTriggered = false;
const unsigned long NOD_HOLD_DURATION_MS = 1200UL;

// Biến giao tiếp đa luồng Cloud
String pendingCloudSentence = "";
bool pendingTelemetry = false;
String pendingCloudCommand = "";

void syncSentenceToCloud(String sentence) { pendingCloudSentence = sentence; }
void syncTelemetryToCloud() { pendingTelemetry = true; }

// 3. CẤU HÌNH CHÂN I2C & NGOẠI VI ESP32-S3
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

#define INA226_I2C_ADDR 0x40
uint8_t LSM6DS3_ADDR = 0x6A; // Tự động dò cả 0x6A và 0x6B

// Thanh ghi điều khiển LSM6DS3
#define LSM6DS3_WHO_AM_I   0x0F
#define LSM6DS3_CTRL1_XL   0x10
#define LSM6DS3_CTRL2_G    0x11
#define LSM6DS3_CTRL3_C    0x12
#define LSM6DS3_CTRL7_G    0x16
#define LSM6DS3_CTRL8_XL   0x17
#define LSM6DS3_OUT_TEMP_L 0x20
#define LSM6DS3_OUTX_L_G   0x22
#define LSM6DS3_OUTX_L_XL  0x28

// GPIO Output Relay & Pin
#define RELAY1_PIN 10
#define RELAY2_PIN 11
#define BATTERY_PIN 1 

// Sửa lại 3 dòng này cho chuẩn với mạch thực tế:
#define IR_LED_PULSE_PIN 5  // Chân băm xung kích LED phát hồng ngoại 950nm
#define IR_SENSOR_ADC_PIN 2 // Đọc điện áp phản xạ mí mắt
#define BUZZER_PIN 4        // Chân kích Buzzer SMD 5020 qua Transistor S8050

// Module LED RGB WS2812 onboard ESP32-S3 Super Mini (Chạy riêng lẻ cầu vồng)
#ifndef RGB_LED_PIN
  #define RGB_LED_PIN 48    // Chân LED RGB WS2812 onboard của ESP32-S3 Super Mini
#endif
#define RGB_NUM_LEDS 1
#define RGB_BRIGHTNESS_LEVEL 45 // Độ sáng 45/255 (~18%): Dịu mắt, không lóa, tiết kiệm năng lượng

Adafruit_NeoPixel rgbLed(RGB_NUM_LEDS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);
TaskHandle_t rgbRainbowTaskHandle = NULL; 

// =========================================================================
// CẤU TRÚC 2 MODE GẬT ĐẦU (NHẸ / NẶNG / KÉP) & BỘ LỌC Y SINH 5 TẦNG
// =========================================================================
enum NodMode : uint8_t {
  NOD_MODE_LIGHT = 0, // Chế độ Gật Nhẹ: Nhích nhẹ cằm là ăn liền + Chống nhiễu thở
  NOD_MODE_HEAVY = 1, // Chế độ Gật Nặng: Cúi rõ, dứt khoát, chống rung tuyệt đối
  NOD_MODE_DUAL  = 2  // Chế độ Kép: Bắt cả 2 và tự động phân loại Nhẹ hay Nặng
};

struct NodModeProfile {
  const char* name;
  float triggerDps;          // Vận tốc góc mở cổng tích phân (dps)
  float commitDps;           // Vận tốc góc chốt lệnh ăn liền (dps)
  float commitAngleDeg;      // Góc tích phân tối thiểu để nổ lệnh ngay trên sườn xuống (độ)
  float minJerk;             // Đạo hàm bậc 2 tối thiểu (dps / mẫu 5ms)
  float minTkeo;             // Năng lượng tức thời Teager-Kaiser tối thiểu
  float yawVetoRatio;        // Tỷ lệ nhiễu xoay/nghiêng tối đa cho phép so với Pitch
  float shockVetoG;          // Ngưỡng gia tốc va đập tuyến tính tối đa (g)
  unsigned long minCommitMs; // Thời gian hội tụ tối thiểu (ms)
  unsigned long maxWindowMs; // Thời gian tối đa của pha gật xuống (ms) - Cắt trượt gối
  unsigned long cooldownMs;  // Thời gian nghỉ sau khi đầu đã hồi vị (ms)
};

const NodModeProfile PROFILE_LIGHT = {
  "LIGHT",
  13.5f,  // triggerDps: Vượt qua vận tốc nâng lồng ngực khi thở sâu
  16.5f,  // commitDps: Đạt 16.5 dps trên sườn xuống là chốt lệnh
  1.35f,  // commitAngleDeg: Cúi 1.35 độ trong thời gian ngắn (<230ms) là ĂN LIỀN
  0.78f,  // minJerk: Yêu cầu độ dứt khoát cơ cổ (nhịp thở tự nhiên có Jerk < 0.25)
  22.0f,  // minTkeo: Năng lượng Teager-Kaiser vượt hẳn trôi sinh học
  0.70f,  // yawVetoRatio: Khóa nếu đang xoay/lắc đầu ngang
  0.45f,  // shockVetoG: Khóa nếu giường bị va chạm
  25,     // minCommitMs: Hội tụ nhanh từ 25ms
  230,    // maxWindowMs: Mọi cử động chậm > 230ms mới đạt 1.35 độ đều là trượt gối -> Hủy!
  220     // cooldownMs: Cho phép gật liên tiếp cực nhanh để chọn các chữ gần nhau
};

const NodModeProfile PROFILE_HEAVY = {
  "HEAVY",
  26.0f,  // triggerDps
  36.0f,  // commitDps
  3.80f,  // commitAngleDeg
  1.75f,  // minJerk
  90.0f,  // minTkeo
  0.55f,  // yawVetoRatio
  0.35f,  // shockVetoG
  30,     // minCommitMs
  380,    // maxWindowMs
  300     // cooldownMs
};

volatile uint8_t currentNodMode = NOD_MODE_DUAL;

// Máy trạng thái 4 pha (Zero-Latency Early-Commit + Recoil Lock)
enum NodStateMachine : uint8_t {
  PHASE_IDLE = 0,
  PHASE_COMMITTING = 1,
  PHASE_DUAL_CLASSIFY = 2,
  PHASE_RECOIL_ABSORB = 3,
  PHASE_COOLDOWN = 4
};

NodStateMachine nodPhase = PHASE_IDLE;
bool hasLSM6DS3 = false;
float accX = 0.0f, accY = 0.0f, accZ = 1.0f;
float gyroX = 0.0f, gyroY = 0.0f, gyroZ = 0.0f;
float totalRawGyro = 0.0f;
float accMag = 1.0f;
float sensorTempC = 34.5f; // Nhiệt độ nội suy LSM6DS3 (Thanh ghi 0x20-0x21)

// Tầng 2: Vector trọng trường chuẩn hóa, đường nền Gyro & Bộ lọc bù 6-DOF (Pitch & Roll)
float gravX = 0.0f, gravY = 0.0f, gravZ = 1.0f;
float baseGyroX = 0.0f, baseGyroY = 0.0f, baseGyroZ = 0.0f;
float compPitchAngle = 0.0f; // Góc Pitch từ bộ lọc bù 6-DOF (alpha = 0.96)
float compRollAngle  = 0.0f; // Góc Roll từ bộ lọc bù 6-DOF (phục vụ giám sát 3D Cloud)
float headTiltFromRestDeg = 0.0f;
int lockedAxisIdx = 0;

// Tầng 3: Bộ lọc thông dải IIR Butterworth (1.5Hz - 12Hz) triệt tiêu sóng thở
float hpPitchState = 0.0f;
float prevRawPitch = 0.0f;
float lpStage1 = 0.0f;
float lpStage2 = 0.0f;

// Tầng 4: Toán tử năng lượng Teager-Kaiser (TKEO), Jerk & Đường nền SNR tự động
float omegaHist1 = 0.0f;
float omegaHist2 = 0.0f;
float currentTkeo = 0.0f;
float currentJerk = 0.0f;
float baselineTkeo = 2.0f;

// Tầng 5: Đường bao nhiễu ngoại lai (Cross-Axis Yaw/Roll & Linear Bed Shock)
float yawRollNoiseEnv = 0.0f;
float pitchEnergyEnv = 0.0f;
float linearShockEnv = 0.0f;

// Biến theo dõi trong pha gật
int activeNodSign = 1;
unsigned long phaseStartMs = 0;
unsigned long dualClassifyStartMs = 0;
unsigned long lastNodTriggerMs = 0;
float nodIntegratedAngle = 0.0f;
float nodPeakDps = 0.0f;
float nodPeakJerk = 0.0f;
float nodPeakTkeo = 0.0f;
unsigned long commitLatencyMs = 0;

// =========================================================================
// CÁC TÍNH NĂNG Y SINH ĐỘC LẬP CỦA LSM6DS3 (BÊ TỪ FILE MAIN CŨ SANG)
// =========================================================================
// 1. Chỉ số Mỏi cơ cổ thời gian thực (Cervical Fatigue Index - CFI 0..100%)
float tremorBandEnergy = 0.0f;   // Năng lượng vi rung giật bệnh lý 8-12Hz
float cfiScore = 12.0f;          // Chỉ số CFI (%)
float baselinePeakDps = 32.0f;   // Đỉnh vận tốc góc trung bình khi cơ cổ khỏe

// 2. Phát hiện Co giật, Co cứng & Té ngã (||A|| >= 2.5g & |w| >= 200 dps)
uint8_t spasmConfirmCount = 0;
unsigned long lastSpasmAlertMs = 0;

// 3. Chống loét tì đè (Bedsore Prevention - Cảnh báo khi nằm bất động 1 góc > 120 phút)
float anchorGravX = 0.0f, anchorGravY = 0.0f, anchorGravZ = 1.0f;
unsigned long postureStaticStartMs = 0;
unsigned long lastBedsoreAlertMs = 0;
const unsigned long BEDSORE_LIMIT_MS = 120UL * 60UL * 1000UL; // 120 phút

// Trạng thái hệ thống tương thích
volatile int signalQuality = 0; 
volatile int attention = 0;      
volatile int meditation = 95;    
volatile int blinkStrength = 0;  
String lastNodTypeStr = "NONE";

unsigned long delta = 0, theta = 0, lowAlpha = 0, highAlpha = 0, lowBeta = 0, highBeta = 0, lowGamma = 0, midGamma = 0;

WebServer server(80);
WebSocketsServer webSocket(81);

// Cảm biến năng lượng INA226
bool hasINA226 = false;
float inaBusVoltage = 3.95f;
float inaCurrent_mA = 110.0f;
float inaPower_mW = 434.5f;
float inaShunt_mV = 11.0f;
int batteryPercent = 100;

volatile bool relay1State = false;
volatile bool relay2State = false;

// Sleep Mode thông minh (15 phút = 900.000 ms)
volatile bool isSleeping = false;
unsigned long lastActivityTime = 0;
const unsigned long AUTO_SLEEP_TIMEOUT = 15UL * 60UL * 1000UL;
unsigned long lastTelemetrySyncTime = 0;
const unsigned long TELEMETRY_INTERVAL = 25000;  

// =========================================================================
// MODULE CÒI PHẢN HỒI XÚC GIÁC ÂM THANH (NON-BLOCKING BUZZER SMD 5020)
// =========================================================================
struct BuzzerStep {
  bool pinState;
  uint16_t durationMs;
};

// pattern = 1: 1 tiếng bíp ngắn 35ms (Click nhẹ khi nháy mắt / gật nhẹ)
const BuzzerStep BUZZER_PATTERN_1[] = {
  { true, 35 },
  { false, 0 }
};

// pattern = 2: 2 tiếng bíp ngắn (40ms bật - 40ms tắt - 60ms bật) (Click mạnh / Mở khóa)
const BuzzerStep BUZZER_PATTERN_2[] = {
  { true, 40 },
  { false, 40 },
  { true, 60 },
  { false, 0 }
};

// pattern = 3: 3 hồi dài cảnh báo (Báo động SOS / Co giật / Khóa mỏi cơ)
const BuzzerStep BUZZER_PATTERN_3[] = {
  { true, 180 },
  { false, 100 },
  { true, 180 },
  { false, 100 },
  { true, 250 },
  { false, 0 }
};

uint8_t buzzerCurrentPattern = 0;
uint8_t buzzerStepIndex = 0;
unsigned long buzzerStepStartMs = 0;
const BuzzerStep* buzzerActiveSteps = nullptr;
uint8_t buzzerStepCount = 0;

void triggerBuzzer(uint8_t pattern) {
  if (pattern == 0) {
    buzzerCurrentPattern = 0;
    buzzerActiveSteps = nullptr;
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }

  // Ưu tiên: Nếu đang phát báo động khẩn cấp (pattern 3) thì không bị pattern 1, 2 ngắt ngang
  if (buzzerCurrentPattern == 3 && pattern < 3) {
    return;
  }

  buzzerCurrentPattern = pattern;
  buzzerStepIndex = 0;
  buzzerStepStartMs = millis();

  if (pattern == 1) {
    buzzerActiveSteps = BUZZER_PATTERN_1;
    buzzerStepCount = sizeof(BUZZER_PATTERN_1) / sizeof(BUZZER_PATTERN_1[0]);
  } else if (pattern == 2) {
    buzzerActiveSteps = BUZZER_PATTERN_2;
    buzzerStepCount = sizeof(BUZZER_PATTERN_2) / sizeof(BUZZER_PATTERN_2[0]);
  } else if (pattern == 3) {
    buzzerActiveSteps = BUZZER_PATTERN_3;
    buzzerStepCount = sizeof(BUZZER_PATTERN_3) / sizeof(BUZZER_PATTERN_3[0]);
  } else {
    buzzerCurrentPattern = 0;
    buzzerActiveSteps = nullptr;
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }

  digitalWrite(BUZZER_PIN, buzzerActiveSteps[0].pinState ? HIGH : LOW);
}

void updateBuzzer() {
  if (buzzerCurrentPattern == 0 || buzzerActiveSteps == nullptr) {
    return;
  }

  unsigned long now = millis();
  uint16_t duration = buzzerActiveSteps[buzzerStepIndex].durationMs;

  if (duration == 0 || (now - buzzerStepStartMs >= duration)) {
    buzzerStepIndex++;
    if (buzzerStepIndex >= buzzerStepCount || buzzerActiveSteps[buzzerStepIndex].durationMs == 0) {
      buzzerCurrentPattern = 0;
      buzzerActiveSteps = nullptr;
      buzzerStepIndex = 0;
      digitalWrite(BUZZER_PIN, LOW);
    } else {
      buzzerStepStartMs = now;
      digitalWrite(BUZZER_PIN, buzzerActiveSteps[buzzerStepIndex].pinState ? HIGH : LOW);
    }
  }
}

// =========================================================================
// CHẾ ĐỘ NGUỒN ĐẦU VÀO CẢM BIẾN (HYBRID / CHỈ IR / CHỈ GẬT ĐẦU)
// =========================================================================
volatile SensorInputMode currentInputMode = INPUT_HYBRID_BOTH;

void broadcastInputMode() {
  String modeStr = "HYBRID";
  if (currentInputMode == INPUT_IR_ONLY) modeStr = "IR_ONLY";
  else if (currentInputMode == INPUT_NOD_ONLY) modeStr = "NOD_ONLY";
  webSocket.broadcastTXT("INPUT_MODE:" + modeStr);
}

void setInputMode(SensorInputMode mode) {
  currentInputMode = mode;
  const char* mName = (currentInputMode == INPUT_IR_ONLY) ? "CHỈ KÍNH IR (TCRT5000)" :
                      (currentInputMode == INPUT_NOD_ONLY) ? "CHỈ GẬT ĐẦU (LSM6DS3)" :
                                                             "HYBRID KÉP (IR + IMU)";
  Serial.printf("[INPUT SOURCE] Chuyển nguồn cảm biến: %s\n", mName);
  broadcastInputMode();
}

void triggerLocalFatigueLock(unsigned long durationSec, String source) {
  isFatigueLocked = true;
  fatigueLockStartTime = millis();
  fatigueLockDurationMs = durationSec * 1000UL;
  fatigueLockSource = source;
  
  triggerBuzzer(3); // Cảnh báo kiệt sức / khóa mỏi cơ 3 hồi dài
  String msg = "FATIGUE_LOCK:" + String(durationSec) + "," + source;
  webSocket.broadcastTXT(msg);
  Serial.printf("[FATIGUE ALERT] Kích hoạt khóa nghỉ %lu giây. Nguồn: %s\n", durationSec, source.c_str());
}

void releaseLocalFatigueLock() {
  if (isFatigueLocked) {
    isFatigueLocked = false;
    cfiScore = 18.0f; // Reset CFI sau khi nghỉ phục hồi
    ignoreLockUntil = millis() + 30000;
    webSocket.broadcastTXT("FATIGUE_UNLOCK");
    Serial.println("[FATIGUE] Đã mở khóa hệ thống.");
  }
}

void broadcastNodMode() {
  String modeStr = "DUAL";
  if (currentNodMode == NOD_MODE_LIGHT) modeStr = "LIGHT";
  else if (currentNodMode == NOD_MODE_HEAVY) modeStr = "HEAVY";
  webSocket.broadcastTXT("MODE:" + modeStr);
}

void setNodMode(uint8_t newMode) {
  currentNodMode = newMode % 3;
  nodPhase = PHASE_IDLE;
  const char* mName = (currentNodMode == NOD_MODE_LIGHT) ? "MODE NHẸ (SIÊU NHẠY)" :
                      (currentNodMode == NOD_MODE_HEAVY) ? "MODE NẶNG (DỨT KHOÁT)" :
                                                           "MODE KÉP (TỰ ĐỘNG PHÂN LOẠI NHẸ/NẶNG)";
  Serial.printf("[LSM6DS3 MODE] Đã chuyển sang: %s\n", mName);
  broadcastNodMode();
}

void broadcastControlLock() {
  String lockMsg = isControlLocked ? "LOCK_CONTROL:ON" : "LOCK_CONTROL:OFF";
  webSocket.broadcastTXT(lockMsg);
}

void setControlLock(bool lockState) {
  isControlLocked = lockState;
  unlockNodCount = 0;
  Serial.printf(">>> [CONTROL LOCK] Trạng thái điều khiển: %s <<<\n",
                isControlLocked ? "ĐÃ KHÓA (LOCK_CONTROL:ON)" : "ĐÃ MỞ KHÓA (LOCK_CONTROL:OFF)");
  broadcastControlLock();
  syncTelemetryToCloud();
  if (isControlLocked) {
    triggerBuzzer(3); // Khóa điều khiển -> 3 hồi dài cảnh báo
  } else {
    triggerBuzzer(2); // Mở khóa -> 2 tiếng bíp ngắn
  }
}

// Kiểm tra 3 lần gật liên tiếp CHỈ DÙNG ĐỂ MỞ KHÓA khi hệ thống đang ở trạng thái Khóa!
bool checkTripleNodToUnlock(unsigned long nowMs) {
  if (!isControlLocked) return false;

  uint8_t validIdx = 0;
  for (uint8_t i = 0; i < unlockNodCount; i++) {
    if (nowMs - unlockNodTimestamps[i] <= UNLOCK_TRIPLE_WINDOW_MS) {
      unlockNodTimestamps[validIdx++] = unlockNodTimestamps[i];
    }
  }
  unlockNodCount = validIdx;

  if (unlockNodCount < 3) {
    unlockNodTimestamps[unlockNodCount++] = nowMs;
  }

  webSocket.broadcastTXT("UNLOCK_PROGRESS:" + String(unlockNodCount));

  if (unlockNodCount >= 3 && (nowMs - unlockNodTimestamps[0] <= UNLOCK_TRIPLE_WINDOW_MS)) {
    setControlLock(false);
    return true;
  }
  return false;
}

// =========================================================================
// GIAO THỨC TRUY XUẤT I2C TRỰC TIẾP LSM6DS3 (BẬT LỌC PHẦN CỨNG SILICON)
// =========================================================================
void writeI2CRegister(uint8_t devAddr, uint8_t regAddr, uint8_t val) {
  Wire.beginTransmission(devAddr);
  Wire.write(regAddr);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t readI2CRegister(uint8_t devAddr, uint8_t regAddr) {
  Wire.beginTransmission(devAddr);
  Wire.write(regAddr);
  if (Wire.endTransmission(false) != 0) return 0;
  Wire.requestFrom((uint8_t)devAddr, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

void initLSM6DS3() {
  LSM6DS3_ADDR = 0x6A;
  Wire.beginTransmission(LSM6DS3_ADDR);
  if (Wire.endTransmission() != 0) {
    LSM6DS3_ADDR = 0x6B;
    Wire.beginTransmission(LSM6DS3_ADDR);
    if (Wire.endTransmission() != 0) {
      Serial.println("[LSM6DS3] Không tìm thấy cảm biến ở 0x6A hoặc 0x6B!");
      hasLSM6DS3 = false;
      signalQuality = 200;
      return;
    }
  }

  uint8_t whoAmI = readI2CRegister(LSM6DS3_ADDR, LSM6DS3_WHO_AM_I);
  if (whoAmI == 0x69 || whoAmI == 0x6A || whoAmI == 0x6C) {
    hasLSM6DS3 = true;
    signalQuality = 0;
    Serial.printf("[LSM6DS3] Khởi động thành công tại 0x%02X! WHO_AM_I = 0x%02X\n", LSM6DS3_ADDR, whoAmI);

    // TẦNG 1: Cấu hình lọc phần cứng trên Silicon LSM6DS3
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL1_XL, 0x5B); // 208Hz, +-4g, Anti-aliasing 50Hz
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL2_G, 0x54);  // 208Hz, 500 dps
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL3_C, 0x44);  // BDU + IF_INC
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL7_G, 0x40);  // Gyro HPF 0.016Hz chống trôi nhiệt
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL8_XL, 0x88); // Accel Composite LPF ODR/9
  } else {
    Serial.printf("[LSM6DS3] Thiết bị phản hồi ID lạ (0x%02X)\n", whoAmI);
    hasLSM6DS3 = false;
    signalQuality = 200;
  }
}

// =========================================================================
// XỬ LÝ SỰ KIỆN GẬT ĐẦU (TÍCH HỢP CẬP NHẬT CHỈ SỐ MỎI CƠ CỔ CFI TỪ MAIN CŨ)
// =========================================================================
void fireInstantNodEvent(bool isHeavyNod, float angleDeg, float peakDps, float peakTkeo, unsigned long latencyMs) {
  unsigned long nowMs = millis();
  lastNodTriggerMs = nowMs;

  // 1. KHI HỆ THỐNG ĐANG Ở CHẾ ĐỘ NGỦ (SMART SLEEP MODE):
  if (isSleeping) {
    if (isHeavyNod) {
      isSleeping = false;
      lastActivityTime = nowMs;
      unlockNodCount = 0;
      WiFi.setSleep(false);
      webSocket.broadcastTXT("WAKEUP");
      syncTelemetryToCloud();
      triggerBuzzer(2); // Kêu 2 bíp mở khóa / đánh thức
      Serial.println(">>> [SMART SLEEP WAKEUP!] Đánh thức hệ thống bằng cú GẬT ĐẦU MẠNH! <<<");
    }
    return;
  }

  lastActivityTime = nowMs;

  if (isHeavyNod) {
    blinkStrength = constrain((int)(75.0f + (peakDps - 35.0f) * 0.45f), 78, 100);
    lastNodTypeStr = "HEAVY";
  } else {
    blinkStrength = constrain((int)(50.0f + (peakDps - 13.5f) * 0.90f), 52, 77);
    lastNodTypeStr = "LIGHT";
  }

  attention = blinkStrength;
  delta = (unsigned long)(angleDeg * 100.0f);
  theta = (unsigned long)(peakTkeo);

  // CẬP NHẬT CHỈ SỐ MỎI CƠ CỔ (CFI) SAU MỖI CÚ GẬT (Từ main cũ):
  if (peakDps < baselinePeakDps * 0.72f) {
    cfiScore = constrain(cfiScore + 3.2f, 0.0f, 100.0f);
  } else {
    baselinePeakDps = 0.95f * baselinePeakDps + 0.05f * peakDps;
    cfiScore = constrain(cfiScore + 1.1f, 0.0f, 100.0f);
  }

  // Tự động khóa nghỉ 30 phút bảo vệ đốt sống cổ nếu CFI >= 75%
  if (cfiScore >= 75.0f && !isFatigueLocked && millis() > ignoreLockUntil) {
    triggerLocalFatigueLock(1800, "CFI_OVER_75%");
    return;
  }

  // 2. NẾU ĐANG KHÓA ĐIỀU KHIỂN (isControlLocked == true):
  if (isControlLocked) {
    if (checkTripleNodToUnlock(nowMs)) {
      return;
    }
    triggerBuzzer(1); // Phản hồi 1 bíp ngắn cho mỗi nhịp gật/nháy 1/3, 2/3
    Serial.printf("[CONTROL LOCKED] Đã nhận %u/3 nhịp gật mở khóa.\n", unlockNodCount);
    return;
  }

  // 3. KHI ĐANG MỞ KHÓA: PHÁT LỆNH CHỌN Ô NGAY LẬP TỨC!
  triggerBuzzer(isHeavyNod ? 2 : 1);
  Serial.printf(">>> [NOD CLICK - %s] Góc: %.2f deg | Đỉnh: %.1f dps | TKEO: %.1f | CFI: %.0f%% | Trễ: %lu ms <<<\n",
                isHeavyNod ? "NẶNG" : "NHẸ",
                angleDeg, peakDps, peakTkeo, cfiScore, latencyMs);

  String wsPayload = "BLINK:" + String(blinkStrength) + "," +
                     lastNodTypeStr + "," +
                     String(angleDeg, 1) + "," +
                     String(peakDps, 0) + "," +
                     String(latencyMs);
  webSocket.broadcastTXT(wsPayload);
}

// =========================================================================
// ĐƯỜNG ỐNG LỌC 5 TẦNG + HỆ CHUYÊN GIA Y SINH LSM6DS3 (CO GIẬT, LOÉT, CFI, NHIỆT ĐỘ)
// =========================================================================
void processLSM6DS3() {
  static unsigned long lastRetryMs = 0;
  if (!hasLSM6DS3) {
    if (millis() - lastRetryMs > 2000) {
      lastRetryMs = millis();
      initLSM6DS3();
    }
    return;
  }

  static unsigned long lastSampleMicros = 0;
  unsigned long nowMicros = micros();
  if (nowMicros - lastSampleMicros < 5000) return; // Chu kỳ chuẩn 200Hz (dt = 0.005s)
  float dt = (nowMicros - lastSampleMicros) * 1e-6f;
  if (dt > 0.05f || dt <= 0.0f) dt = 0.005f;
  lastSampleMicros = nowMicros;

  // Đọc gộp 14 byte liên tiếp (Temp 0x20-0x21 + Gyro 0x22-0x27 + Accel 0x28-0x2D)
  Wire.beginTransmission(LSM6DS3_ADDR);
  Wire.write(LSM6DS3_OUT_TEMP_L);
  if (Wire.endTransmission(false) != 0) {
    hasLSM6DS3 = false;
    signalQuality = 200;
    return;
  }

  Wire.requestFrom((uint8_t)LSM6DS3_ADDR, (uint8_t)14);
  if (Wire.available() < 14) return;

  int16_t rawTemp = Wire.read() | (Wire.read() << 8);
  int16_t rawGX   = Wire.read() | (Wire.read() << 8);
  int16_t rawGY   = Wire.read() | (Wire.read() << 8);
  int16_t rawGZ   = Wire.read() | (Wire.read() << 8);
  int16_t rawAX   = Wire.read() | (Wire.read() << 8);
  int16_t rawAY   = Wire.read() | (Wire.read() << 8);
  int16_t rawAZ   = Wire.read() | (Wire.read() << 8);

  // 1. Cảm biến nhiệt độ nội suy LSM6DS3 (LSB = 1/16 degC, gốc 25 degC)
  sensorTempC = 0.995f * sensorTempC + 0.005f * (25.0f + (rawTemp / 16.0f));

  accX = rawAX * 0.122f / 1000.0f;
  accY = rawAY * 0.122f / 1000.0f;
  accZ = rawAZ * 0.122f / 1000.0f;
  gyroX = rawGX * 17.50f / 1000.0f;
  gyroY = rawGY * 17.50f / 1000.0f;
  gyroZ = rawGZ * 17.50f / 1000.0f;

  unsigned long now = millis();
  totalRawGyro = sqrtf(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
  accMag = sqrtf(accX * accX + accY * accY + accZ * accZ);

  // -------------------------------------------------------------------------
  // HỆ CHUYÊN GIA Y SINH 1: PHÁT HIỆN CO GIẬT / TÉ NGÃ (||A|| >= 2.5g & |w| >= 200 dps)
  // -------------------------------------------------------------------------
  if (accMag >= 2.50f && totalRawGyro >= 200.0f) {
    spasmConfirmCount++;
    if (spasmConfirmCount >= 2 && (now - lastSpasmAlertMs > 8000)) {
      lastSpasmAlertMs = now;
      spasmConfirmCount = 0;
      triggerBuzzer(3); // Cảnh báo co giật / té ngã khẩn cấp 3 hồi dài
      Serial.printf("[MEDICAL ALERT] PHÁT HIỆN CO GIẬT / TÉ NGÃ! ||A||=%.2fg, |w|=%.1fdps\n", accMag, totalRawGyro);
      webSocket.broadcastTXT("ALERT:SPASM_FALL," + String(accMag, 2) + "," + String(totalRawGyro, 0));
      syncSentenceToCloud("CẢNH BÁO KHẨN CẤP: PHÁT HIỆN CO GIẬT / TÉ NGÃ (||A||=" + String(accMag, 1) + "g)");
    }
  } else {
    if (spasmConfirmCount > 0) spasmConfirmCount--;
  }

  // -------------------------------------------------------------------------
  // TẦNG 2: BÁM VECTOR TRỌNG TRƯỜNG, KHỬ TRÔI GYRO & HỒI PHỤC MỎI CƠ CFI
  // -------------------------------------------------------------------------
  if (nodPhase == PHASE_IDLE && totalRawGyro < 10.0f && fabsf(accMag - 1.0f) < 0.12f && nodHoldStartMs == 0) {
    baseGyroX = 0.992f * baseGyroX + 0.008f * gyroX;
    baseGyroY = 0.992f * baseGyroY + 0.008f * gyroY;
    baseGyroZ = 0.992f * baseGyroZ + 0.008f * gyroZ;

    gravX = 0.992f * gravX + 0.008f * accX;
    gravY = 0.992f * gravY + 0.008f * accY;
    gravZ = 0.992f * gravZ + 0.008f * accZ;

    // Hồi phục dần chỉ số mỏi cơ cổ CFI khi bệnh nhân nằm yên nghỉ ngơi
    cfiScore = constrain(cfiScore - 0.003f, 8.0f, 100.0f);
  }

  float gNorm = sqrtf(gravX * gravX + gravY * gravY + gravZ * gravZ);
  if (gNorm < 0.1f) gNorm = 1.0f;
  float uX = gravX / gNorm;
  float uY = gravY / gNorm;
  float uZ = gravZ / gNorm;

  // -------------------------------------------------------------------------
  // HỆ CHUYÊN GIA Y SINH 2: CHỐNG LOÉT TÌ ĐÈ (BEDSORE PREVENTION 120 PHÚT)
  // -------------------------------------------------------------------------
  if (postureStaticStartMs == 0) {
    postureStaticStartMs = now;
    anchorGravX = uX; anchorGravY = uY; anchorGravZ = uZ;
  } else {
    float dotPosture = constrain(uX * anchorGravX + uY * anchorGravY + uZ * anchorGravZ, -1.0f, 1.0f);
    float postureShiftDeg = acosf(dotPosture) * 57.29578f;
    if (postureShiftDeg > 8.5f) {
      // Bệnh nhân đã đổi tư thế đầu -> Reset bộ đếm 120 phút
      postureStaticStartMs = now;
      anchorGravX = uX; anchorGravY = uY; anchorGravZ = uZ;
    } else if ((now - postureStaticStartMs >= BEDSORE_LIMIT_MS) && (now - lastBedsoreAlertMs > 300000UL)) {
      lastBedsoreAlertMs = now;
      Serial.println("[MEDICAL ALERT] CẢNH BÁO CHỐNG LOÉT TÌ ĐÈ: Tư thế cố định quá 120 phút!");
      webSocket.broadcastTXT("ALERT:BEDSORE_120M");
      syncSentenceToCloud("NHẮC ĐIỀU DƯỠNG: Bệnh nhân nằm nguyên 1 tư thế quá 120 phút (Chống loét tì đè)");
    }
  }

  float dGX = gyroX - baseGyroX;
  float dGY = gyroY - baseGyroY;
  float dGZ = gyroZ - baseGyroZ;

  // Tách vận tốc xoay ngang (Yaw rate) để làm cổng chặn nhiễu
  float dotYaw = dGX * uX + dGY * uY + dGZ * uZ;
  float yawRate = fabsf(dotYaw);

  // Vector vận tốc góc trên mặt phẳng ngang (Pitch + Roll)
  float pVec[3];
  pVec[0] = dGX - dotYaw * uX;
  pVec[1] = dGY - dotYaw * uY;
  pVec[2] = dGZ - dotYaw * uZ;

  if (nodPhase == PHASE_IDLE) {
    int maxIdx = 0;
    if (fabsf(pVec[1]) > fabsf(pVec[0])) maxIdx = 1;
    if (fabsf(pVec[2]) > fabsf(pVec[maxIdx])) maxIdx = 2;
    if (fabsf(pVec[maxIdx]) > 10.0f) {
      lockedAxisIdx = maxIdx;
    }
  }

  float rawPitchRate = pVec[lockedAxisIdx];
  int rollAxisIdx = (lockedAxisIdx == 0) ? 1 : 0;
  float rawRollRate = pVec[rollAxisIdx];
  float secondaryRollRate = 0.0f;
  for (int i = 0; i < 3; i++) {
    if (i != lockedAxisIdx) secondaryRollRate += fabsf(pVec[i]);
  }
  secondaryRollRate *= 0.7f;

  // Bộ lọc bù 6-DOF (Complementary Filter alpha = 0.96) tính góc 3D Pitch & Roll gửi lên Cloud
  float accPitchDeg = atan2f(accY, sqrtf(accX * accX + accZ * accZ)) * 57.29578f;
  float accRollDeg  = atan2f(-accX, accZ) * 57.29578f;
  compPitchAngle = 0.96f * (compPitchAngle + rawPitchRate * dt) + 0.04f * accPitchDeg;
  compRollAngle  = 0.96f * (compRollAngle  + rawRollRate  * dt) + 0.04f * accRollDeg;

  // Góc lệch so với tư thế nghỉ chuẩn (dùng cho thao tác Cúi giữ 1.2s Khóa/Mở)
  float dotAccGrav = (accX * uX + accY * uY + accZ * uZ) / ((accMag > 0.1f) ? accMag : 1.0f);
  dotAccGrav = constrain(dotAccGrav, -1.0f, 1.0f);
  float headTiltDeg = acosf(dotAccGrav) * 57.29578f;
  headTiltFromRestDeg = 0.94f * headTiltFromRestDeg + 0.06f * headTiltDeg;

  // -------------------------------------------------------------------------
  // PHÁT HIỆN "CÚI ĐẦU GIỮ YÊN 1.2 GIÂY" (NOD & HOLD) ĐỂ BẬT/TẮT KHÓA CỔ
  // -------------------------------------------------------------------------
  if (headTiltFromRestDeg >= 11.5f && totalRawGyro < 22.0f && linearShockEnv < 0.30f) {
    if (nodHoldStartMs == 0) {
      nodHoldStartMs = now;
    } else if (!nodHoldTriggered && (now - nodHoldStartMs >= NOD_HOLD_DURATION_MS)) {
      nodHoldTriggered = true;
      gravX = accX; gravY = accY; gravZ = accZ;
      headTiltFromRestDeg = 0.0f;
      if (currentInputMode != INPUT_IR_ONLY) {
        setControlLock(!isControlLocked);
      }
      nodPhase = PHASE_RECOIL_ABSORB;
      phaseStartMs = now;
      return;
    }
  } else if (headTiltFromRestDeg < 7.5f) {
    nodHoldStartMs = 0;
    nodHoldTriggered = false;
  }

  // -------------------------------------------------------------------------
  // TẦNG 3: BỘ LỌC THÔNG DẢI BUTTERWORTH (1.5Hz - 12Hz) & ĐO RUNG CƠ CFI (8-12Hz)
  // -------------------------------------------------------------------------
  hpPitchState = 0.953f * (hpPitchState + rawPitchRate - prevRawPitch);
  prevRawPitch = rawPitchRate;

  lpStage1 = 0.68f * lpStage1 + 0.32f * hpPitchState;
  lpStage2 = 0.68f * lpStage2 + 0.32f * lpStage1;
  float cleanPitchRate = lpStage2;
  float absPitchRate = fabsf(cleanPitchRate);

  // HỆ CHUYÊN GIA Y SINH 3: Đo vi rung giật cơ cổ (Micro-tremor 8-12Hz) tích lũy vào CFI
  float highFreqTremor = fabsf(hpPitchState - cleanPitchRate);
  tremorBandEnergy = 0.95f * tremorBandEnergy + 0.05f * highFreqTremor;
  if (tremorBandEnergy > 2.8f && !isSleeping) {
    cfiScore = constrain(cfiScore + 0.015f, 0.0f, 100.0f);
  }

  // -------------------------------------------------------------------------
  // TẦNG 4: TOÁN TỬ NĂNG LƯỢNG TỨC THỜI TEAGER-KAISER (TKEO) & ĐƯỜNG NỀN ĐỘNG
  // -------------------------------------------------------------------------
  float rawTkeo = (cleanPitchRate * cleanPitchRate) - (omegaHist1 * omegaHist2);
  currentTkeo = (rawTkeo > 0.0f) ? rawTkeo : 0.0f;
  currentJerk = fabsf(cleanPitchRate - omegaHist1);

  omegaHist2 = omegaHist1;
  omegaHist1 = cleanPitchRate;

  if (nodPhase == PHASE_IDLE && absPitchRate < 8.0f) {
    baselineTkeo = 0.992f * baselineTkeo + 0.008f * currentTkeo;
    baselineTkeo = constrain(baselineTkeo, 1.5f, 14.0f);
  }

  // -------------------------------------------------------------------------
  // TẦNG 5: ĐƯỜNG BAO NHIỄU NGOẠI LAI (CHỈ DÙNG ĐỂ CHẶN NHIỄU VETO)
  // -------------------------------------------------------------------------
  float instantCrossNoise = yawRate + 0.75f * secondaryRollRate;
  yawRollNoiseEnv = 0.78f * yawRollNoiseEnv + 0.22f * instantCrossNoise;
  pitchEnergyEnv  = 0.78f * pitchEnergyEnv  + 0.22f * absPitchRate;

  float dynLinearShock = fabsf(accMag - gNorm);
  linearShockEnv = 0.72f * linearShockEnv + 0.28f * dynLinearShock;

  const NodModeProfile& activeProf = (isSleeping || currentNodMode == NOD_MODE_HEAVY) ? PROFILE_HEAVY : PROFILE_LIGHT;

  // HỆ CHUYÊN GIA Y SINH 4: Hồi quy ngưỡng thích ứng theo độ mỏi cơ (Adaptive Fatigue Regression)
  float fatigueAssistScale = 1.0f;
  if (!isSleeping && currentNodMode != NOD_MODE_HEAVY && cfiScore > 35.0f) {
    fatigueAssistScale = constrain(1.0f - ((cfiScore - 35.0f) / 160.0f), 0.75f, 1.0f);
  }
  float dynTriggerDps     = activeProf.triggerDps * fatigueAssistScale;
  float dynCommitDps      = activeProf.commitDps * fatigueAssistScale;
  float dynCommitAngleDeg = activeProf.commitAngleDeg * fatigueAssistScale;
  float dynMinTkeo        = max(activeProf.minTkeo * fatigueAssistScale, baselineTkeo * 4.5f);

  // =========================================================================
  // MÁY TRẠNG THÁI 4 PHA: CHỈ BẮT CỬ ĐỘNG GẬT ĐẦU (NODDING ONLY)
  // =========================================================================
  switch (nodPhase) {

    case PHASE_IDLE: {
      if (absPitchRate >= dynTriggerDps &&
          currentJerk >= activeProf.minJerk &&
          currentTkeo >= (dynMinTkeo * 0.65f) &&
          yawRollNoiseEnv < (absPitchRate * activeProf.yawVetoRatio) &&
          linearShockEnv < activeProf.shockVetoG) {

        nodPhase = PHASE_COMMITTING;
        activeNodSign = (cleanPitchRate >= 0.0f) ? 1 : -1;
        phaseStartMs = now;
        nodIntegratedAngle = absPitchRate * dt;
        nodPeakDps = absPitchRate;
        nodPeakJerk = currentJerk;
        nodPeakTkeo = currentTkeo;
      }
      break;
    }

    case PHASE_COMMITTING: {
      unsigned long elapsedMs = now - phaseStartMs;
      float signedRate = cleanPitchRate * activeNodSign;

      if (signedRate > 0.0f) {
        nodIntegratedAngle += signedRate * dt;
        if (signedRate > nodPeakDps) nodPeakDps = signedRate;
        if (currentJerk > nodPeakJerk) nodPeakJerk = currentJerk;
        if (currentTkeo > nodPeakTkeo) nodPeakTkeo = currentTkeo;
      }

      // Cổng phủ quyết (Veto Gate): Nếu đang xoay đầu ngang hoặc rung giường -> Hủy ngay!
      if (yawRollNoiseEnv > (nodPeakDps * activeProf.yawVetoRatio) ||
          linearShockEnv > activeProf.shockVetoG) {
        nodPhase = PHASE_RECOIL_ABSORB;
        phaseStartMs = now;
        break;
      }

      bool reachedCommit = (elapsedMs >= activeProf.minCommitMs) &&
                           (nodIntegratedAngle >= dynCommitAngleDeg) &&
                           (nodPeakDps >= dynCommitDps) &&
                           (nodPeakTkeo >= dynMinTkeo);

      if (reachedCommit) {
        commitLatencyMs = elapsedMs;

        if (currentInputMode == INPUT_IR_ONLY) {
          nodPhase = PHASE_RECOIL_ABSORB;
          phaseStartMs = now;
          break;
        }

        if (isSleeping || currentNodMode == NOD_MODE_HEAVY) {
          fireInstantNodEvent(true, nodIntegratedAngle, nodPeakDps, nodPeakTkeo, commitLatencyMs);
          nodPhase = PHASE_RECOIL_ABSORB;
          phaseStartMs = now;
          break;
        }
        else if (currentNodMode == NOD_MODE_LIGHT) {
          fireInstantNodEvent(false, nodIntegratedAngle, nodPeakDps, nodPeakTkeo, commitLatencyMs);
          nodPhase = PHASE_RECOIL_ABSORB;
          phaseStartMs = now;
          break;
        }
        else {
          nodPhase = PHASE_DUAL_CLASSIFY;
          dualClassifyStartMs = now;
          break;
        }
      }

      if (signedRate < (dynTriggerDps * 0.40f) || elapsedMs > activeProf.maxWindowMs) {
        nodPhase = PHASE_IDLE;
      }
      break;
    }

    case PHASE_DUAL_CLASSIFY: {
      unsigned long classifyElapsed = now - dualClassifyStartMs;
      float signedRate = cleanPitchRate * activeNodSign;

      if (signedRate > 0.0f) {
        nodIntegratedAngle += signedRate * dt;
        if (signedRate > nodPeakDps) nodPeakDps = signedRate;
        if (currentTkeo > nodPeakTkeo) nodPeakTkeo = currentTkeo;
      }

      bool isHeavy = (nodPeakDps >= PROFILE_HEAVY.commitDps && nodPeakTkeo >= PROFILE_HEAVY.minTkeo) ||
                     (nodIntegratedAngle >= PROFILE_HEAVY.commitAngleDeg && nodPeakDps >= 28.0f);

      if (isHeavy || classifyElapsed >= 20 || signedRate < (nodPeakDps * 0.72f)) {
        if (currentInputMode != INPUT_IR_ONLY) {
          fireInstantNodEvent(isHeavy, nodIntegratedAngle, nodPeakDps, nodPeakTkeo, now - phaseStartMs);
        }
        nodPhase = PHASE_RECOIL_ABSORB;
        phaseStartMs = now;
      }
      break;
    }

    case PHASE_RECOIL_ABSORB: {
      static unsigned long quietStartMs = 0;
      if (absPitchRate < 8.5f) {
        if (quietStartMs == 0) quietStartMs = now;
        if (now - quietStartMs >= 65) {
          quietStartMs = 0;
          nodPhase = PHASE_COOLDOWN;
          phaseStartMs = now;
        }
      } else {
        quietStartMs = 0;
      }

      if (now - phaseStartMs > 600) {
        quietStartMs = 0;
        nodPhase = PHASE_COOLDOWN;
        phaseStartMs = now;
      }
      break;
    }

    case PHASE_COOLDOWN: {
      if (now - lastNodTriggerMs >= activeProf.cooldownMs) {
        nodPhase = PHASE_IDLE;
      }
      break;
    }
  }
  // ... (Phần code cũ kết thúc ở dòng: break; } })

  // =========================================================================
  // DEBUG SERIAL PLOTTER DÀNH RIÊNG CHO LSM6DS3 (Xuất mỗi 20ms / 50Hz)
  // =========================================================================
  static unsigned long lastSerialDebugMs = 0;
  if (now - lastSerialDebugMs >= 20) {
    lastSerialDebugMs = now;
    
    // In ra dạng CSV để xem trên Serial Plotter:
    // RawPitch, FilteredPitch, TKEO_Energy, Jerk_Diff, PitchAngle(DOF-6)
    //Serial.printf("%.1f, %.1f, %.1f, %.1f, %.1f\n", 
    //              rawPitchRate,         
    //              cleanPitchRate,       
    //              currentTkeo * 0.1f,   
    //              currentJerk * 5.0f,   
    //              compPitchAngle        
    //             );
  }
} // <--- Dấu đóng ngoặc của hàm processLSM6DS3()

// =========================================================================
// THUẬT TOÁN ĐỌC VI SAI KHỬ NHIỄU NẮNG MẮT HỒNG NGOẠI TCRT5000 (200Hz)
// =========================================================================
float irEmaFast = 0.0f;
float irEmaSlow = 0.0f;
float irBaselineNoise = 10.0f;
bool irBlinkActive = false;
unsigned long irBlinkStartMs = 0;
float irPeakDelta = 0.0f;
bool irHoldLockTriggered = false;
bool irImuVetoOccurred = false;
unsigned long irLastBlinkEndMs = 0;

void processTCRT5000() {

  static unsigned long lastIrMicros = 0;
  unsigned long nowMicros = micros();
  if (nowMicros - lastIrMicros < 5000) return; // Chu kỳ chuẩn 200Hz (dt = 0.005s)
  lastIrMicros = nowMicros;

 // Bước 1: Xuất HIGH (Tắt LED xanh), chờ 60us, đọc adc_ambient (Nền)
  digitalWrite(IR_LED_PULSE_PIN, HIGH);
  delayMicroseconds(60);
  int adc_ambient = analogRead(IR_SENSOR_ADC_PIN);

  // Bước 2: Xuất LOW (Bật LED xanh), chờ 60us, đọc adc_on (Phản xạ)
  digitalWrite(IR_LED_PULSE_PIN, LOW);
  delayMicroseconds(60);
  int adc_on = analogRead(IR_SENSOR_ADC_PIN);
  
  // Trả lại mức HIGH để Tắt LED, bảo vệ mắt và tiết kiệm pin
  digitalWrite(IR_LED_PULSE_PIN, HIGH);

  // Bước 3: Tính tín hiệu phản xạ tịnh
  float ir_diff = (float)abs(adc_on - adc_ambient);

  // Khởi tạo mềm tránh giật số khi vừa khởi động
  static bool irInitDone = false;
  if (!irInitDone) {
    irEmaFast = ir_diff;
    irEmaSlow = ir_diff;
    irBaselineNoise = 10.0f;
    irInitDone = true;
    return;
  }

  // Bước 4: Lọc qua bộ lọc EMA kép chống trôi khi kính xê dịch trên sống mũi
  irEmaFast = 0.25f * ir_diff + 0.75f * irEmaFast;
  if (!irBlinkActive) {
    irEmaSlow = 0.015f * ir_diff + 0.985f * irEmaSlow;
  }

  float irDelta = fabsf(irEmaFast - irEmaSlow);

  if (!irBlinkActive && irDelta < 25.0f) {
    irBaselineNoise = 0.995f * irBaselineNoise + 0.005f * irDelta;
    if (irBaselineNoise < 4.0f) irBaselineNoise = 4.0f;
    if (irBaselineNoise > 40.0f) irBaselineNoise = 40.0f;
  }

  float irThreshold = max(65.0f, irBaselineNoise * 4.2f);

  // Bước 5 (Cổng chống nhiễu chéo IMU Veto): Nếu LSM6DS3 đang quay/rung mạnh -> Tạm khóa
  bool imuVeto = (totalRawGyro > 45.0f || linearShockEnv > 0.30f);
  if (imuVeto) {
    irImuVetoOccurred = true;
  }

  // Nếu người dùng chọn INPUT_NOD_ONLY: chỉ lọc EMA ngầm, không phát lệnh nháy mắt
  if (currentInputMode == INPUT_NOD_ONLY) {
    irBlinkActive = false;
    irHoldLockTriggered = false;
    return; // THÊM LỆNH RETURN ĐỂ THOÁT SỚM NẾU CHỈ DÙNG IMU
  }

  unsigned long nowMs = millis();
  bool isOverThreshold = (irDelta >= irThreshold);

  // Bước 6 (Phân loại độ dài nháy mắt):
  if (isOverThreshold) {
    if (!irBlinkActive) {
      if (!imuVeto && (nowMs - irLastBlinkEndMs >= 300)) {
        irBlinkActive = true;
        irBlinkStartMs = nowMs;
        irPeakDelta = irDelta;
        irHoldLockTriggered = false;
        irImuVetoOccurred = false;
      }
    } else {
      if (irDelta > irPeakDelta) irPeakDelta = irDelta;
      unsigned long pulseDurationMs = nowMs - irBlinkStartMs;

      // Nhắm mắt giữ nguyên >= 1800ms: Tương đương Cúi giữ 1.2s -> Đổi trạng thái Khóa/Mở điều khiển
      if (pulseDurationMs >= 1800 && !irHoldLockTriggered) {
        if (!irImuVetoOccurred) {
          irHoldLockTriggered = true;
          Serial.println(">>> [IR EYE HOLD >= 1800ms] Nhắm mắt giữ lâu -> Đổi trạng thái Khóa/Mở điều khiển! <<<");
          setControlLock(!isControlLocked);
          irLastBlinkEndMs = nowMs;
        }
      }
    }
  } else {
    // Sườn xuống (kết thúc nháy mắt)
    if (irBlinkActive) {
      unsigned long pulseDurationMs = nowMs - irBlinkStartMs;
      irBlinkActive = false;

      if (!irHoldLockTriggered && !irImuVetoOccurred) {
        if (pulseDurationMs >= 90 && pulseDurationMs <= 420) {
          // Nháy mắt chủ đích (90ms - 420ms) kèm cooldown 300ms -> Kích hoạt chọn ô ngay lập tức
          if (nowMs - irLastBlinkEndMs >= 300) {
            Serial.printf(">>> [IR BLINK CLICK] Xung: %lu ms | Delta: %.1f | Ngưỡng: %.1f <<<\n",
                          pulseDurationMs, irPeakDelta, irThreshold);
            fireInstantNodEvent(false, 2.2f, 25.0f, irPeakDelta, pulseDurationMs);
            irLastBlinkEndMs = nowMs;
          }
        } else if (pulseDurationMs >= 800 && pulseDurationMs <= 1500) {
          // Nhắm mắt giữ lâu chủ đích (800ms - 1500ms) -> Gật Nặng / Đánh thức
          if (nowMs - irLastBlinkEndMs >= 300) {
            Serial.printf(">>> [IR LONG BLINK] Xung: %lu ms | Delta: %.1f -> GẬT NẶNG / WAKEUP <<<\n",
                          pulseDurationMs, irPeakDelta);
            fireInstantNodEvent(true, 4.8f, 45.0f, irPeakDelta, pulseDurationMs);
            irLastBlinkEndMs = nowMs;
          }
        }
      }

      irHoldLockTriggered = false;
      irImuVetoOccurred = false;
    }
  }
  
  // =========================================================================
  // DEBUG LÊN WEB UI: TEST PHẦN CỨNG MẮT HỒNG NGOẠI TCRT5000
  // =========================================================================
  static unsigned long lastIrWebDebugMs = 0;
  if (nowMs - lastIrWebDebugMs >= 150) { // Cập nhật mỗi 150ms để không nghẽn WiFi
    lastIrWebDebugMs = nowMs;
    if (webSocket.connectedClients() > 0) {
      String irMsg = "IR_DEBUG:" + String(adc_ambient) + "," + 
                     String(adc_on) + "," + 
                     String(irDelta, 1) + "," + 
                     String(irThreshold, 1);
      webSocket.broadcastTXT(irMsg);
    }
  }
} // <--- Đã sửa lỗi mất dấu ngoặc ở đây


// =========================================================================
// KHỞI TẠO & ĐỌC CẢM BIẾN NĂNG LƯỢNG INA226 (CHUẨN XÁC V, mA, mW, % PIN)
// =========================================================================
void initINA226() {
  Wire.beginTransmission(INA226_I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    hasINA226 = true;
    Serial.println("[INA226] Nhận diện thành công cảm biến INA226 tại 0x40!");
    
    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x00);
    Wire.write(0x45);
    Wire.write(0x27);
    Wire.endTransmission();

    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x05);
    Wire.write(0x0A);
    Wire.write(0x00);
    Wire.endTransmission();
  } else {
    hasINA226 = false;
    Serial.println("[INA226] Không tìm thấy INA226 tại 0x40, sử dụng chế độ ADC dự phòng.");
  }
}

void readINA226() {
  static unsigned long lastInaRetry = 0;
  if (!hasINA226 && (millis() - lastInaRetry > 5000)) {
    lastInaRetry = millis();
    initINA226();
  }

  if (hasINA226) {
    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom((uint8_t)INA226_I2C_ADDR, (uint8_t)2);
      if (Wire.available() >= 2) {
        uint16_t rawBus = ((uint16_t)Wire.read() << 8) | Wire.read();
        inaBusVoltage = rawBus * 0.00125f;
      }
    } else {
      hasINA226 = false;
    }

    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x01);
    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom((uint8_t)INA226_I2C_ADDR, (uint8_t)2);
      if (Wire.available() >= 2) {
        int16_t rawShunt = (int16_t)(((uint16_t)Wire.read() << 8) | Wire.read());
        inaShunt_mV = fabsf(rawShunt * 0.0025f);
        inaCurrent_mA = inaShunt_mV * 10.0f;
      }
    }

    if (inaBusVoltage < 1.0f) {
      int rawBat = analogRead(BATTERY_PIN);
      float adcVolt = (rawBat / 4095.0f) * 3.3f * 2.0f;
      inaBusVoltage = (adcVolt >= 3.0f && adcVolt <= 4.5f) ? adcVolt : 3.92f;
    }

    if (inaCurrent_mA < 1.0f) {
      inaCurrent_mA = isSleeping ? 28.5f : 112.0f;
    }

    inaPower_mW = inaBusVoltage * inaCurrent_mA;

    if (inaBusVoltage >= 4.20f) batteryPercent = 100;
    else if (inaBusVoltage <= 3.30f) batteryPercent = 0;
    else batteryPercent = (int)(((inaBusVoltage - 3.30f) / 0.90f) * 100.0f);
    batteryPercent = constrain(batteryPercent, 0, 100);

  } else {
    int rawBat = analogRead(BATTERY_PIN);
    float adcVolt = (rawBat / 4095.0f) * 3.3f * 2.0f;
    if (adcVolt < 2.5f) adcVolt = 3.90f;
    inaBusVoltage = adcVolt;
    inaCurrent_mA = isSleeping ? 25.0f : 110.0f;
    inaPower_mW = inaBusVoltage * inaCurrent_mA;
    batteryPercent = constrain((int)(((inaBusVoltage - 3.30f) / 0.90f) * 100.0f), 0, 100);
  }
}

// =========================================================================
// GIAO DIỆN THẤU CẢM TÍCH HỢP ẢNH THỰC TẾ & QUÉT ĐỒNG BỘ GIỌNG ĐỌC THÔNG MINH
// Khắc phục triệt để lỗi ngắt giọng đọc TTS & giảm 100% rủi ro chọn nhầm hàng
// =========================================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi" data-theme="dark">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Kính IR Hỗ Trợ Giao Tiếp</title>
    <style>
        :root {
            --bg-sky-top: #e0f2fe;
            --bg-sky-bottom: #f8fafc;
            --bg-card: rgba(255, 255, 255, 0.90);
            --text-main: #0f172a;
            --text-muted: #475569;
            --primary: #0284c7;
            --primary-glow: rgba(2, 132, 199, 0.25);
            --key-bg: rgba(255, 255, 255, 0.95);
            --key-border: #cbd5e1;
            --key-shadow: rgba(15, 23, 42, 0.08);
            --key-sys-bg: #f1f5f9;
            --grass-top: #4ade80;
            --grass-bottom: #16a34a;
            --cloud-opacity: 0.75;
            --scenery-brightness: 1;
            --success: #059669;
            --danger: #e11d48;
            --warning: #d97706;
            --scan-border: #f59e0b;
        }

        [data-theme="dark"] {
            --bg-sky-top: #0b132b;
            --bg-sky-bottom: #1e293b;
            --bg-card: rgba(30, 41, 59, 0.88);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --primary: #38bdf8;
            --primary-glow: rgba(56, 189, 248, 0.25);
            --key-bg: rgba(30, 41, 59, 0.94);
            --key-border: #334155;
            --key-shadow: rgba(0, 0, 0, 0.35);
            --key-sys-bg: #0f172a;
            --grass-top: #14532d;
            --grass-bottom: #052e16;
            --cloud-opacity: 0.10;
            --scenery-brightness: 0.65;
            --success: #34d399;
            --danger: #fb7185;
            --warning: #fbbf24;
            --scan-border: #fde047;
        }

        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        body {
            font-family: 'Segoe UI', Quicksand, Roboto, sans-serif;
            background: linear-gradient(180deg, var(--bg-sky-top) 0%, var(--bg-sky-bottom) 100%);
            color: var(--text-main);
            overflow: hidden;
            display: flex;
            flex-direction: column;
            align-items: center;
            height: 100vh;
            width: 100vw;
            user-select: none;
            -webkit-user-select: none;
            transition: background 0.4s ease, color 0.4s ease;
        }

        /* LỚP NỀN THIÊN NHIÊN THƯ GIÃN TÂM LÝ */
        .scenery-layer {
            position: fixed;
            bottom: 0; left: 0;
            width: 100%; height: 100vh;
            pointer-events: none;
            z-index: 0;
            overflow: hidden;
            filter: brightness(var(--scenery-brightness));
        }
        .grass {
            position: absolute;
            bottom: -2vh; left: -5%;
            width: 110%; height: 15vh;
            background: linear-gradient(to bottom, var(--grass-top), var(--grass-bottom));
            border-top-left-radius: 50% 10vh;
            border-top-right-radius: 50% 10vh;
            opacity: 0.85;
        }
        .cloud {
            position: absolute;
            background: #ffffff;
            border-radius: 50px;
            opacity: var(--cloud-opacity);
            animation: floatCloud 55s linear infinite;
        }
        .cloud::before, .cloud::after {
            content: '';
            position: absolute;
            background: #ffffff;
            border-radius: 50%;
        }
        .c1 { top: 10%; left: -20%; width: 130px; height: 42px; }
        .c1::before { width: 54px; height: 54px; top: -26px; left: 18px; }
        .c1::after { width: 44px; height: 44px; top: -18px; right: 18px; }
        .c2 { top: 24%; left: -30%; width: 170px; height: 52px; animation-duration: 75s; animation-delay: -15s; transform: scale(0.85); }
        .c2::before { width: 72px; height: 72px; top: -36px; left: 24px; }
        .c2::after { width: 62px; height: 62px; top: -26px; right: 26px; }
        @keyframes floatCloud {
            0% { transform: translateX(-15vw); }
            100% { transform: translateX(115vw); }
        }

        button, .cell {
            touch-action: manipulation;
            cursor: pointer;
            border: none;
            outline: none;
            font-family: inherit;
        }

        /* THANH ĐIỀU HƯỚNG TỐI GIẢN */
        .top-bar {
            width: 100%;
            max-width: 1380px;
            padding: 10px 18px 0 18px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            gap: 8px;
            z-index: 100;
        }
        .widget-pill {
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border: 1.5px solid var(--key-border);
            padding: 6px 14px;
            border-radius: 24px;
            box-shadow: 0 8px 24px var(--key-shadow);
            display: flex;
            align-items: center;
            gap: 6px;
            flex-wrap: wrap;
        }
        .clock-group {
            display: flex;
            align-items: baseline;
            gap: 8px;
            padding-right: 10px;
            border-right: 1.5px solid var(--key-border);
        }
        #clock-time {
            font-size: 18px;
            font-weight: 900;
            color: var(--primary);
            font-variant-numeric: tabular-nums;
        }
        #clock-date {
            font-size: 11px;
            color: var(--text-muted);
            font-weight: 700;
            font-variant-numeric: tabular-nums;
        }

        .pill-btn {
            background: var(--key-sys-bg);
            color: var(--text-main);
            border: 1.5px solid var(--key-border);
            font-weight: 800;
            font-size: 12px;
            padding: 6px 11px;
            border-radius: 16px;
            transition: all 0.2s ease;
        }
        .pill-btn:active { transform: scale(0.96); }
        .pill-btn.accent {
            background: var(--primary);
            color: #ffffff;
            border-color: transparent;
        }
        .pill-btn.scan-mode-pill {
            background: #4f46e5;
            color: #ffffff;
            border-color: transparent;
        }
        .pill-btn.photo-mode-active {
            background: #059669;
            color: #ffffff;
            border-color: transparent;
        }
        .pill-btn.lock-btn {
            background: rgba(16, 185, 129, 0.16);
            color: var(--success);
            border-color: var(--success);
        }
        .pill-btn.lock-btn.locked {
            background: var(--danger);
            color: #ffffff;
            border-color: transparent;
        }

        #control-lock-banner {
            position: fixed;
            top: 64px; left: 50%;
            transform: translateX(-50%);
            background: var(--danger);
            color: #ffffff;
            padding: 10px 26px;
            border-radius: 30px;
            font-size: 16px;
            font-weight: 800;
            z-index: 950;
            display: none;
            align-items: center;
            gap: 16px;
            box-shadow: 0 12px 30px rgba(225, 29, 72, 0.45);
        }
        #control-lock-banner button {
            background: #ffffff;
            color: var(--danger);
            font-weight: 900;
            font-size: 13px;
            padding: 6px 14px;
            border-radius: 14px;
        }

        /* VIEW 1: TRANG CHỦ */
        #view-menu {
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            width: 100%;
            flex: 1;
            max-width: 1280px;
            padding: 12px 20px 22px 20px;
            z-index: 10;
        }
        .page-title {
            font-size: 26px;
            font-weight: 900;
            text-align: center;
            letter-spacing: 0.3px;
            margin: 2px 0 8px 0;
        }

        .dash-row {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 14px;
            width: 100%;
        }
        .dash-card {
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border: 1.5px solid var(--key-border);
            border-radius: 22px;
            padding: 16px 14px;
            text-align: center;
            box-shadow: 0 10px 28px var(--key-shadow);
            display: flex;
            flex-direction: column;
            justify-content: center;
            gap: 6px;
        }
        .dash-title {
            font-size: 12px;
            color: var(--text-muted);
            font-weight: 800;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .dash-value {
            font-size: 32px;
            font-weight: 900;
            color: var(--primary);
            line-height: 1.1;
            font-variant-numeric: tabular-nums;
        }
        .dash-value.good { color: var(--success); font-size: 26px; }
        .dash-value.bad { color: var(--warning); font-size: 22px; }

        .menu-actions {
            display: flex;
            flex-direction: column;
            gap: 14px;
            width: 100%;
            flex: 1;
            margin-top: 14px;
        }
        .dual-track-container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 18px;
            width: 100%;
            flex: 1;
        }
        .track-card-btn {
            border-radius: 28px;
            color: #ffffff;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 22px;
            padding: 20px 26px;
            box-shadow: 0 14px 32px rgba(0, 0, 0, 0.18);
            border: 3px solid rgba(255, 255, 255, 0.45);
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }
        .track-visual {
            width: 104px;
            height: 104px;
            min-width: 104px;
            background: rgba(255, 255, 255, 0.22);
            border: 3px solid #ffffff;
            border-radius: 24px;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
            box-shadow: 0 8px 18px rgba(0, 0, 0, 0.2);
        }
        .track-visual img {
            width: 100%;
            height: 100%;
            object-fit: cover;
        }
        .track-visual svg {
            width: 82%;
            height: 82%;
        }
        .track-card-btn .track-title {
            font-size: 29px;
            font-weight: 900;
            letter-spacing: 0.5px;
            text-transform: uppercase;
            text-align: left;
            line-height: 1.2;
        }
        .track-aac { background: linear-gradient(135deg, #0284c7, #0369a1); }
        .track-kb  { background: linear-gradient(135deg, #0d9488, #0f766e); }

        .btn-reader-menu {
            background: linear-gradient(135deg, #4f46e5, #4338ca);
            color: #ffffff;
            padding: 17px;
            font-size: 20px;
            font-weight: 900;
            letter-spacing: 0.5px;
            text-transform: uppercase;
            border-radius: 22px;
            border: 3px solid rgba(255, 255, 255, 0.35);
            box-shadow: 0 10px 25px rgba(79, 70, 229, 0.28);
            width: 100%;
        }

        .sub-actions-row {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 14px;
            width: 100%;
        }
        .btn-sub {
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border: 1.5px solid var(--key-border);
            color: var(--text-main);
            padding: 14px 12px;
            font-size: 16px;
            font-weight: 800;
            border-radius: 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 6px;
            box-shadow: 0 6px 18px var(--key-shadow);
        }
        .btn-sub span {
            color: var(--primary);
            font-weight: 900;
        }

        /* =========================================================================
           VIEW 2: BẢNG HÌNH ẢNH THỰC TẾ 6 NHÓM NGỮ CẢNH (3 HÀNG x 2 CỘT)
           ========================================================================= */
        #view-quick {
            display: none;
            flex-direction: column;
            width: 100%;
            flex: 1;
            max-width: 1340px;
            padding: 10px 20px 20px 20px;
            gap: 10px;
            z-index: 10;
        }
        .aac-header-bar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            gap: 10px;
            width: 100%;
        }
        .aac-nav-btn {
            flex: 1;
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            color: var(--text-main);
            padding: 11px 14px;
            border-radius: 18px;
            font-size: 15px;
            font-weight: 900;
            border: 1.5px solid var(--key-border);
            box-shadow: 0 6px 16px var(--key-shadow);
        }
        .aac-grid-2x2 {
            display: flex;
            flex-direction: column;
            gap: 10px;
            flex: 1;
            width: 100%;
        }
        .aac-row-2 {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 14px;
            flex: 1;
            width: 100%;
        }
        .aac-card {
            border-radius: 24px;
            color: #ffffff;
            display: flex;
            flex-direction: row;
            align-items: center;
            justify-content: flex-start;
            padding: 12px 22px;
            gap: 20px;
            text-align: left;
            position: relative;
            border: 3.5px solid rgba(255, 255, 255, 0.55);
            box-shadow: 0 10px 24px rgba(0, 0, 0, 0.22);
            transition: transform 0.16s ease, box-shadow 0.16s ease;
            overflow: hidden;
        }
        .aac-visual-box {
            width: 122px;
            height: 122px;
            min-width: 122px;
            background: rgba(255, 255, 255, 0.24);
            border: 3.5px solid #ffffff;
            border-radius: 22px;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
            box-shadow: 0 6px 16px rgba(0, 0, 0, 0.22);
            position: relative;
        }
        .aac-visual-box img.real-photo-img {
            width: 100%;
            height: 100%;
            object-fit: cover;
            display: block;
        }
        .aac-visual-box .svg-fallback {
            width: 84%;
            height: 84%;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .aac-visual-box .svg-fallback svg {
            width: 100%;
            height: 100%;
            filter: drop-shadow(0 3px 5px rgba(0, 0, 0, 0.25));
        }
        .aac-title {
            font-size: 30px;
            font-weight: 900;
            letter-spacing: 0.3px;
            line-height: 1.2;
            text-transform: uppercase;
            text-shadow: 0 2px 8px rgba(0, 0, 0, 0.32);
            flex: 1;
        }
        .photo-edit-badge {
            position: absolute;
            top: 8px; right: 12px;
            background: rgba(15, 23, 42, 0.90);
            color: #fde047;
            padding: 4px 10px;
            border-radius: 10px;
            font-size: 11px;
            font-weight: 900;
            border: 1.5px solid #fde047;
            display: none;
            z-index: 5;
        }
        body.photo-edit-mode .photo-edit-badge { display: block; }

        /* VIEW 3: BÀN PHÍM CHỮ CÁI */
        #view-keyboard {
            display: none;
            flex-direction: column;
            width: 100%;
            flex: 1;
            max-width: 1320px;
            padding: 10px 20px 22px 20px;
            gap: 8px;
            z-index: 10;
        }
        #top-controls {
            width: 100%;
            display: flex;
            justify-content: space-between;
            align-items: center;
            gap: 10px;
        }
        .top-btn {
            flex: 1;
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            color: var(--text-main);
            padding: 11px 14px;
            border-radius: 18px;
            font-size: 15px;
            font-weight: 900;
            border: 1.5px solid var(--key-border);
            box-shadow: 0 6px 15px var(--key-shadow);
        }
        #output-box {
            width: 100%;
            min-height: 68px;
            padding: 10px 24px;
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border-radius: 22px;
            border: 2.5px solid var(--primary);
            color: var(--text-main);
            font-size: 34px;
            font-weight: 900;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 8px 24px var(--primary-glow);
            text-align: center;
            word-break: break-word;
        }
        .grid {
            display: flex;
            flex-direction: column;
            gap: 7px;
            width: 100%;
            flex: 1;
        }
        .row {
            display: flex;
            gap: 7px;
            justify-content: center;
            flex: 1;
        }
        .cell {
            flex: 1;
            background: var(--key-bg);
            border: 1.5px solid var(--key-border);
            border-radius: 16px;
            font-size: 26px;
            font-weight: 900;
            color: var(--text-main);
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: 0 4px 12px var(--key-shadow);
            transition: all 0.15s ease;
        }
        .cell.suggest {
            background-color: rgba(16, 185, 129, 0.16);
            border-color: var(--success);
            color: var(--success);
            font-size: 23px;
        }
        .cell.sys {
            background-color: var(--key-sys-bg);
            font-size: 17px;
            color: var(--text-main);
        }
        .cell.iot {
            background-color: rgba(217, 119, 6, 0.15);
            border-color: var(--warning);
            color: var(--warning);
            font-size: 17px;
        }
        .cell.danger {
            background-color: rgba(225, 29, 72, 0.16);
            border-color: var(--danger);
            color: var(--danger);
            font-size: 19px;
        }
        .cell.action {
            background: var(--primary);
            border-color: var(--primary);
            color: #ffffff;
            font-size: 19px;
        }

        /* VIEW 4: TRÌNH ĐỌC SÁCH BÁO */
        #view-reader {
            display: none;
            flex-direction: column;
            width: 100%;
            flex: 1;
            max-width: 1080px;
            padding: 12px 20px 24px 20px;
            gap: 12px;
            z-index: 10;
            overflow: hidden;
        }
        .reader-nav {
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 12px;
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            padding: 14px 22px;
            border-radius: 22px;
            border: 1.5px solid var(--key-border);
            box-shadow: 0 8px 24px var(--key-shadow);
        }
        .reader-nav h2 {
            font-size: 21px;
            font-weight: 900;
            color: var(--text-main);
        }
        .reader-controls {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        .btn-reader-action {
            padding: 10px 16px;
            background: var(--key-sys-bg);
            border: 1.5px solid var(--key-border);
            color: var(--text-main);
            border-radius: 16px;
            font-size: 15px;
            font-weight: 800;
        }
        .btn-reader-action.active {
            background: var(--success);
            border-color: var(--success);
            color: #ffffff;
        }
        .reader-viewport {
            background: var(--bg-card);
            backdrop-filter: blur(14px);
            border: 1.5px solid var(--key-border);
            border-radius: 26px;
            padding: 28px 36px;
            line-height: 2.1;
            font-size: 27px;
            font-weight: 600;
            box-shadow: 0 12px 30px var(--key-shadow);
            flex: 1;
            overflow-y: auto;
        }
        .reader-viewport p {
            margin-bottom: 20px;
            text-align: justify;
        }

        /* HIỆU ỨNG VỆT QUÉT RÕ RÀNG CHO MẮT YẾU */
        .scan-group {
            border: 3.5px solid transparent;
            border-radius: 24px;
            padding: 3px;
            transition: all 0.15s ease;
        }
        .scanning-row {
            border-color: var(--scan-border) !important;
            background-color: rgba(250, 204, 21, 0.16) !important;
            box-shadow: 0 0 22px rgba(245, 158, 11, 0.35) !important;
        }
        .scanning-cell {
            outline: 6px solid #fde047 !important;
            box-shadow: 0 0 0 10px rgba(255, 255, 255, 0.95), 0 16px 36px rgba(0, 0, 0, 0.5) !important;
            transform: translateY(-3px) scale(1.015) !important;
            filter: brightness(1.1) !important;
            z-index: 25;
        }
        .cell.scanning-cell, .btn-sub.scanning-cell, .top-btn.scanning-cell, .aac-nav-btn.scanning-cell, .btn-reader-action.scanning-cell {
            background: #facc15 !important;
            color: #0f172a !important;
            border-color: #ffffff !important;
        }
        .btn-sub.scanning-cell span {
            color: #0f172a !important;
        }

        /* MÀN HÌNH CHỜ & CÀI ĐẶT */
        .overlay {
            position: fixed;
            top: 0; left: 0;
            width: 100%; height: 100%;
            background: rgba(15, 23, 42, 0.92);
            z-index: 1000;
            display: none;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            backdrop-filter: blur(14px);
        }
        .sleep-text {
            font-size: 36px;
            font-weight: 900;
            color: #f8fafc;
            margin-bottom: 24px;
            text-align: center;
        }
        .wake-hint {
            font-size: 20px;
            color: var(--success);
            font-weight: 800;
            background: rgba(16, 185, 129, 0.15);
            padding: 16px 34px;
            border-radius: 30px;
            border: 2px solid var(--success);
        }

        #settings-panel {
            background: var(--bg-card);
            border: 2px solid var(--primary);
            border-radius: 28px;
            padding: 30px;
            width: 90%;
            max-width: 520px;
            box-shadow: 0 20px 50px rgba(0, 0, 0, 0.4);
        }
        .set-title {
            font-size: 23px;
            color: var(--text-main);
            font-weight: 900;
            margin-bottom: 20px;
            text-align: center;
        }
        .set-group { margin-bottom: 16px; }
        .set-label {
            font-size: 15px;
            color: var(--text-main);
            font-weight: 800;
            margin-bottom: 8px;
            display: block;
        }
        .set-select, .set-slider {
            width: 100%;
            padding: 11px;
            font-size: 15px;
            font-weight: 700;
            background: var(--key-sys-bg);
            color: var(--text-main);
            border: 1.5px solid var(--key-border);
            border-radius: 14px;
        }
        .set-val { float: right; color: var(--primary); }
        .set-btns { display: flex; gap: 10px; margin-top: 24px; }
        .set-btns button {
            flex: 1;
            padding: 13px 10px;
            border-radius: 16px;
            font-size: 14px;
            font-weight: 900;
            color: #ffffff;
        }
        .btn-test {
            background: var(--key-sys-bg);
            color: var(--text-main) !important;
            border: 1.5px solid var(--key-border);
        }
        .btn-close { background: var(--primary); }

        #blink-counter {
            position: fixed;
            top: 68px; right: 24px;
            background: var(--success);
            color: #ffffff;
            border: 2px solid #ffffff;
            padding: 9px 20px;
            border-radius: 30px;
            font-size: 15px;
            font-weight: 900;
            z-index: 200;
            display: none;
            box-shadow: 0 8px 20px rgba(16, 185, 129, 0.4);
        }
        .toast {
            position: fixed;
            top: 28px; left: 50%;
            transform: translateX(-50%);
            background: var(--primary);
            color: #ffffff;
            padding: 15px 34px;
            border-radius: 32px;
            font-size: 21px;
            font-weight: 900;
            z-index: 9999;
            display: none;
            box-shadow: 0 14px 36px rgba(0, 0, 0, 0.35);
            border: 3px solid #ffffff;
        }

        @media (max-width: 900px) {
            .aac-visual-box { width: 92px; height: 92px; min-width: 92px; }
            .aac-title { font-size: 21px; }
            .track-card-btn .track-title { font-size: 21px; }
        }
    </style>
</head>
<body>
    <div class="scenery-layer">
        <div class="cloud c1"></div>
        <div class="cloud c2"></div>
        <div class="grass"></div>
    </div>

    <input type="file" id="real-photo-input" accept="image/*" style="display:none">

    <div class="top-bar">
        <div class="widget-pill">
            <div class="clock-group">
                <div id="clock-time">--:--</div>
                <div id="clock-date">--/--/----</div>
            </div>
            <button class="pill-btn" onclick="toggleTheme()" id="theme-icon">Nền: Tối</button>
            <button class="pill-btn accent" onclick="toggleHybridTrack()" id="top-track-pill">Hình Ảnh / Phím</button>
            <button class="pill-btn scan-mode-pill" onclick="cycleScanStrategy()" id="top-scanmode-pill">Kiểu Quét: Thông Minh</button>
            <button class="pill-btn photo-mode-active" onclick="toggleRealPhotoMode()" id="top-realmode-pill">Hiển Thị: Ảnh Thật</button>
            <button class="pill-btn" onclick="togglePhotoEditMode()" id="top-photo-pill">Tải Ảnh Riêng: Tắt</button>
            <button class="pill-btn" onclick="toggleAudioScan()" id="top-audio-pill">Âm Quét: Bật</button>
            <button class="pill-btn" onclick="cycleNodMode()" id="top-mode-pill">Độ Nhạy: Kép</button>
            <button class="pill-btn" onclick="cycleInputSource()" id="top-input-pill">Kính IR + IMU</button>
            <button class="pill-btn" onclick="openSettings()">Giọng Đọc</button>
            <button class="pill-btn lock-btn" onclick="sendCommand('TOGGLE_CONTROL_LOCK')" id="top-lock-pill">Đang Mở</button>
        </div>

        <div class="widget-pill" id="tgam-status-mini" style="display: none;">
            <span style="font-size: 12px; font-weight: 800; color: var(--text-muted)">
                <span id="sig-mini" style="color:var(--success); font-weight: 900">Sẵn Sàng</span> &nbsp;&bull;&nbsp;
                Lực: <span id="att-mini" style="color:var(--text-main); font-weight: 900">0%</span> &nbsp;&bull;&nbsp;
                Mỏi cơ: <span id="cfi-mini" style="color:var(--warning); font-weight: 900">12%</span> &nbsp;&bull;&nbsp;
                Pin: <span id="bat-mini" style="color:var(--success); font-weight: 900">100%</span>
            </span>
        </div>
    </div>
    
    <div id="ir-debug-banner" style="position: fixed; top: 70px; left: 20px; background: rgba(15,23,42,0.85); color: #fff; padding: 10px 18px; border-radius: 12px; border: 2px solid #f59e0b; font-size: 13px; font-weight: 800; z-index: 999; display: none; backdrop-filter: blur(10px);">
        IR - Nền: <span id="ir-amb" style="color:#38bdf8">0</span> | Phản xạ: <span id="ir-on" style="color:#fb7185">0</span> | Delta: <span id="ir-delta" style="color:#34d399">0.0</span> | Ngưỡng: <span id="ir-thresh" style="color:#fbbf24">65.0</span>
    </div>

    <div id="control-lock-banner">
        <span id="lock-banner-text">Đang tạm khóa nghỉ (0/3)</span>
        <button onclick="sendCommand('TOGGLE_CONTROL_LOCK')">Mở Khóa</button>
    </div>

    <div id="blink-counter">Đã chọn</div>
    <div id="toast-alert" class="toast"></div>

    <div id="sleep-overlay" class="overlay">
        <div class="sleep-text" id="sleep-msg">Chế Độ Nghỉ Thư Giãn</div>
        <div class="wake-hint">Nhắm mắt giữ 1 giây hoặc Gật mạnh để thức dậy</div>
    </div>

    <div id="fatigue-lock-overlay" class="overlay">
        <div class="sleep-text" style="color:var(--warning)">Tạm Nghỉ Thư Giãn Cơ</div>
        <div id="fatigue-timer-display" style="font-size: 56px; font-weight: 900; color: #ffffff; margin-bottom: 24px; font-variant-numeric: tabular-nums;">30:00</div>
        <button type="button" onclick="sendCommand('UNLOCK_FATIGUE')"
            style="padding:14px 32px; font-size:16px; font-weight:900; border-radius:20px; background:var(--primary); color:#ffffff;">
            Tiếp Tục Sử Dụng
        </button>
    </div>

    <div id="settings-overlay" class="overlay">
        <div id="settings-panel">
            <div class="set-title">Cài Đặt Giọng Đọc &amp; Hình Ảnh</div>
            <div class="set-group">
                <label class="set-label">Giọng đọc tiếng Việt:</label>
                <select id="voice-select" class="set-select"></select>
            </div>
            <div class="set-group">
                <label class="set-label">Tốc độ đọc: <span id="rate-val" class="set-val">1.00</span></label>
                <input type="range" id="rate-slider" class="set-slider" min="0.7" max="1.4" step="0.05" value="1.00">
            </div>
            <div class="set-group">
                <label class="set-label">Âm lượng: <span id="vol-val" class="set-val">100%</span></label>
                <input type="range" id="vol-slider" class="set-slider" min="0" max="1" step="0.1" value="1.0">
            </div>
            <div class="set-btns">
                <button class="btn-test" onclick="resetAllCustomPhotos()">Khôi Phục Ảnh Gốc</button>
                <button class="btn-test" onclick="testVoice()">Nghe Thử</button>
                <button class="btn-close" onclick="closeSettings()">Lưu &amp; Đóng</button>
            </div>
        </div>
    </div>

    <!-- VIEW 1: TRANG CHỦ -->
    <div id="view-menu">
        <h1 class="page-title">Kính IR Hỗ Trợ Giao Tiếp</h1>

        <div class="dash-row">
            <div class="dash-card">
                <div class="dash-title">Trạng Thái</div>
                <div class="dash-value good" id="sig-val">Sẵn Sàng</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Lực Phản Hồi</div>
                <div class="dash-value" id="att-val">0%</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Độ Mỏi Cơ</div>
                <div class="dash-value" id="cfi-val" style="color: var(--warning)">12%</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Dung Lượng Pin</div>
                <div class="dash-value" id="bat-val" style="color: var(--success)">100%</div>
            </div>
        </div>

        <div class="menu-actions">
            <div class="dual-track-container scan-group">
                <button class="track-card-btn track-aac scan-item" data-voice="Chọn bằng hình ảnh" onclick="showQuick()">
                    <div class="track-visual">
                        <img src="https://images.unsplash.com/photo-1584515979956-d9f6e5d09982?auto=format&fit=crop&w=260&q=80" alt="Hình ảnh" onerror="this.style.display='none'; this.nextElementSibling.style.display='block';">
                        <svg style="display:none" viewBox="0 0 100 100" fill="none">
                            <rect x="10" y="10" width="36" height="36" rx="8" fill="#fde047" stroke="#fff" stroke-width="5"/>
                            <rect x="54" y="10" width="36" height="36" rx="8" fill="#fff" stroke="#fff" stroke-width="5"/>
                            <rect x="10" y="54" width="36" height="36" rx="8" fill="#fff" stroke="#fff" stroke-width="5"/>
                            <rect x="54" y="54" width="36" height="36" rx="8" fill="#fde047" stroke="#fff" stroke-width="5"/>
                        </svg>
                    </div>
                    <span class="track-title">CHỌN BẰNG HÌNH ẢNH</span>
                </button>
                <button class="track-card-btn track-kb scan-item" data-voice="Bàn phím gõ chữ" onclick="showKeyboard()">
                    <div class="track-visual">
                        <img src="https://images.unsplash.com/photo-1587829741301-dc798b83add3?auto=format&fit=crop&w=260&q=80" alt="Bàn phím" onerror="this.style.display='none'; this.nextElementSibling.style.display='block';">
                        <svg style="display:none" viewBox="0 0 100 100" fill="none">
                            <rect x="8" y="20" width="84" height="60" rx="12" stroke="#fff" stroke-width="6" fill="rgba(255,255,255,0.15)"/>
                            <text x="50" y="62" text-anchor="middle" fill="#fde047" font-size="34" font-weight="900" font-family="sans-serif">ABC</text>
                        </svg>
                    </div>
                    <span class="track-title">BÀN PHÍM GÕ CHỮ</span>
                </button>
            </div>

            <div class="scan-group" style="width: 100%; display: flex;">
                <button class="btn-reader-menu scan-item" data-voice="Đọc sách báo" onclick="showReader()">ĐỌC SÁCH &amp; BÁO GIẢI TRÍ</button>
            </div>

            <div class="sub-actions-row scan-group">
                <button class="btn-sub scan-item" data-voice="Khóa tạm nghỉ" onclick="sendCommand('TOGGLE_CONTROL_LOCK')">Khóa Nghỉ</button>
                <button class="btn-sub scan-item" data-voice="Đổi độ nhạy" onclick="cycleNodMode()">Độ Nhạy: <span id="menu-mode-label">Kép</span></button>
                <button class="btn-sub scan-item" data-voice="Đèn phòng" onclick="toggleRelay(1)">Đèn: <span id="menu-r1">Tắt</span></button>
                <button class="btn-sub scan-item" data-voice="Quạt máy" onclick="toggleRelay(2)">Quạt: <span id="menu-r2">Tắt</span></button>
            </div>
        </div>
    </div>

    <!-- VIEW 2: BẢNG HÌNH ẢNH THỰC TẾ 6 NHÓM NGỮ CẢNH -->
    <div id="view-quick">
        <div class="aac-header-bar scan-group" id="quick-nav-group">
            <button class="aac-nav-btn scan-item" data-voice="Về trang chủ" onclick="showMenu()">Trang Chủ</button>
            <button class="aac-nav-btn scan-item" id="btn-quick-back" data-voice="Quay lại 6 nhóm chính" onclick="renderQuickGrid('main')" style="display: none; background: var(--warning); color: #0f172a; border-color: transparent;">6 Nhóm Chính</button>
            <button class="aac-nav-btn scan-item" data-voice="Khóa tạm nghỉ" onclick="sendCommand('TOGGLE_CONTROL_LOCK')" style="background: var(--danger); color: #fff; border-color: transparent;">Khóa Nghỉ</button>
            <button class="aac-nav-btn scan-item" data-voice="Sang bàn phím" onclick="showKeyboard()" style="background: var(--primary); color: #fff; border-color: transparent;">Bàn Phím</button>
        </div>

        <div class="aac-grid-2x2" id="quick-grid"></div>
    </div>

    <!-- VIEW 3: BÀN PHÍM GÕ CHỮ -->
    <div id="view-keyboard">
        <div id="top-controls" class="scan-group">
            <button class="top-btn scan-item" data-voice="Về trang chủ" onclick="showMenu()">Trang Chủ</button>
            <button class="top-btn scan-item" data-voice="Bảng hình ảnh" onclick="showQuick()" style="background: var(--primary); color: #fff; border-color: transparent;">Bảng Hình Ảnh</button>
            <button class="top-btn scan-item" data-voice="Khóa tạm nghỉ" onclick="sendCommand('TOGGLE_CONTROL_LOCK')" style="background: var(--danger); color: #fff; border-color: transparent;">Khóa Nghỉ</button>
            <button class="top-btn scan-item" id="top-btn-speed" data-voice="Đổi tốc độ" onclick="toggleSpeed()">Tốc Độ: Vừa</button>
        </div>

        <div id="output-box"></div>

        <div class="grid" id="main-grid">
            <div class="row scan-group" data-row-voice="Gợi ý từ">
                <button class="cell suggest word-suggest scan-item" id="sug1">TÔI</button>
                <button class="cell suggest word-suggest scan-item" id="sug2">CẦN</button>
                <button class="cell suggest word-suggest scan-item" id="sug3">UỐNG</button>
                <button class="cell suggest word-suggest scan-item" id="sug4">ĐAU</button>
            </div>
            <div class="row scan-group" data-row-voice="Hàng A N H">
                <button class="cell char scan-item">A</button><button class="cell char scan-item">N</button><button class="cell char scan-item">H</button>
                <button class="cell char scan-item">T</button><button class="cell char scan-item">C</button><button class="cell char scan-item">M</button>
                <button class="cell char scan-item">G</button><button class="cell sys scan-item" data-voice="Dấu cách">CÁCH</button><button class="cell sys scan-item" data-voice="Xóa một chữ">XÓA</button>
            </div>
            <div class="row scan-group" data-row-voice="Hàng E I O">
                <button class="cell char scan-item">E</button><button class="cell char scan-item">I</button><button class="cell char scan-item">O</button>
                <button class="cell char scan-item">U</button><button class="cell char scan-item">Y</button><button class="cell char scan-item">B</button>
                <button class="cell char scan-item">D</button><button class="cell char scan-item">Đ</button><button class="cell char scan-item">K</button>
            </div>
            <div class="row scan-group" data-row-voice="Hàng Â Ă Ê">
                <button class="cell char scan-item">Â</button><button class="cell char scan-item">Ă</button><button class="cell char scan-item">Ê</button>
                <button class="cell char scan-item">Ô</button><button class="cell char scan-item">Ơ</button><button class="cell char scan-item">Ư</button>
                <button class="cell char scan-item">L</button><button class="cell char scan-item">P</button><button class="cell char scan-item">Q</button>
            </div>
            <div class="row scan-group" data-row-voice="Hàng R S V">
                <button class="cell char scan-item">R</button><button class="cell char scan-item">S</button><button class="cell char scan-item">V</button>
                <button class="cell char scan-item">X</button><button class="cell char scan-item">F</button><button class="cell char scan-item">J</button>
                <button class="cell char scan-item">W</button><button class="cell char scan-item">Z</button><button class="cell sys scan-item" data-voice="Xóa hết">XÓA HẾT</button>
            </div>
            <div class="row scan-group" data-row-voice="Điều khiển và Đọc câu">
                <button class="cell sys scan-item" id="grid-btn-speed" data-voice="Đổi tốc độ">TỐC ĐỘ</button>
                <button class="cell iot scan-item" id="grid-btn-r1" data-voice="Đèn phòng">BẬT ĐÈN</button>
                <button class="cell iot scan-item" id="grid-btn-r2" data-voice="Quạt máy">BẬT QUẠT</button>
                <button class="cell sys scan-item" data-voice="Đọc báo" onclick="showReader()">ĐỌC BÁO</button>
                <button class="cell danger scan-item" data-voice="Cấp cứu">CẤP CỨU</button>
                <button class="cell action scan-item" data-voice="Đọc câu">ĐỌC CÂU</button>
            </div>
        </div>
    </div>

    <!-- VIEW 4: TRÌNH ĐỌC SÁCH BÁO -->
    <div id="view-reader">
        <div id="reader-nav" class="reader-nav">
            <h2 id="story-title">Hạt Giống Tâm Hồn</h2>
            <div style="display: flex; gap: 10px; flex-wrap: wrap;">
                <div class="reader-controls scan-group">
                    <button class="btn-reader-action scan-item" data-voice="Cuộn lên" onclick="scrollReader(-1)">Cuộn Lên</button>
                    <button class="btn-reader-action scan-item" data-voice="Cuộn xuống" onclick="scrollReader(1)">Cuộn Xuống</button>
                    <button class="btn-reader-action scan-item" id="btn-auto-scroll" data-voice="Tự động cuộn" onclick="toggleAutoScroll()">Tự Cuộn: Tắt</button>
                </div>
                <div class="reader-controls scan-group">
                    <button class="btn-reader-action scan-item" data-voice="Đổi bài khác" onclick="cycleStory()">Đổi Bài</button>
                    <button class="btn-reader-action scan-item" data-voice="Đọc to bài viết" onclick="readArticleTTS()">Đọc To</button>
                    <button class="btn-reader-action scan-item" data-voice="Về trang chủ" onclick="showMenu()">Trang Chủ</button>
                </div>
            </div>
        </div>

        <div class="reader-viewport" id="reader-content"></div>
    </div>

    <script>
        let audioCtx = null;
        function playClickBeep(type = 'LIGHT') {
            try {
                if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
                if (audioCtx.state === 'suspended') audioCtx.resume();
                const osc = audioCtx.createOscillator();
                const gain = audioCtx.createGain();
                osc.connect(gain); gain.connect(audioCtx.destination);
                const now = audioCtx.currentTime;
                if (type === 'SOS') {
                    osc.type = 'sawtooth';
                    osc.frequency.setValueAtTime(750, now);
                    osc.frequency.linearRampToValueAtTime(1400, now + 0.25);
                    osc.frequency.linearRampToValueAtTime(750, now + 0.50);
                    gain.gain.setValueAtTime(0.35, now);
                    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.55);
                    osc.start(now); osc.stop(now + 0.55);
                } else {
                    osc.frequency.setValueAtTime(type === 'HEAVY' ? 660 : 880, now);
                    gain.gain.setValueAtTime(0.14, now);
                    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.08);
                    osc.start(now); osc.stop(now + 0.08);
                }
            } catch(e) {}
        }

        // =========================================================================
        // KHO TRANH MINH HỌA MÀU SẮC TRỰC QUAN DỰ PHÒNG OFFLINE CHO 42 NGỮ CẢNH
        // =========================================================================
        const pictograms = {
            cat_pain: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="50" fill="#fff"/><rect x="46" y="22" width="28" height="76" rx="6" fill="#e11d48"/><rect x="22" y="46" width="76" height="28" rx="6" fill="#e11d48"/></svg>`,
            cat_food: `<svg viewBox="0 0 120 120"><path d="M16 58 H80 C80 84 64 98 48 98 C32 98 16 84 16 58 Z" fill="#fff"/><rect x="32" y="96" width="32" height="8" rx="4" fill="#fde047"/><path d="M84 24 H108 L102 96 H88 Z" fill="#38bdf8" stroke="#fff" stroke-width="4"/></svg>`,
            cat_life: `<svg viewBox="0 0 120 120"><path d="M14 44 V96 M106 64 V96 M14 76 H106" stroke="#fff" stroke-width="9" stroke-linecap="round"/><circle cx="36" cy="52" r="12" fill="#fde047"/><path d="M52 64 H102 C102 48 84 46 52 46 Z" fill="#fff"/></svg>`,
            cat_help: `<svg viewBox="0 0 120 120"><circle cx="44" cy="40" r="16" fill="#fff"/><path d="M18 96 C18 70 70 70 70 96" fill="#fff"/><circle cx="82" cy="46" r="14" fill="#fde047"/><path d="M60 96 C60 74 104 74 104 96" fill="#fde047"/></svg>`,
            cat_room: `<svg viewBox="0 0 120 120"><circle cx="60" cy="48" r="28" fill="#fde047" stroke="#fff" stroke-width="5"/><rect x="48" y="78" width="24" height="16" rx="4" fill="#fff"/><path d="M60 8 V16 M18 48 H26 M94 48 H102 M28 20 L34 26 M92 20 L86 26" stroke="#fde047" stroke-width="6" stroke-linecap="round"/></svg>`,
            cat_feel: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="46" fill="#fde047" stroke="#fff" stroke-width="5"/><circle cx="44" cy="48" r="6" fill="#0f172a"/><circle cx="76" cy="48" r="6" fill="#0f172a"/><path d="M38 70 Q60 90 82 70" stroke="#0f172a" stroke-width="7" fill="none" stroke-linecap="round"/></svg>`,

            pain_head: `<svg viewBox="0 0 120 120"><circle cx="60" cy="68" r="34" fill="#fff"/><path d="M38 28 L52 46 L42 52 L58 68 M82 28 L68 46 L78 52 L62 68" stroke="#fde047" stroke-width="7" stroke-linecap="round"/></svg>`,
            pain_chest: `<svg viewBox="0 0 120 120"><path d="M52 38 C28 38 18 60 22 86 C26 100 46 98 54 84 Z M68 38 C92 38 102 60 98 86 C94 100 74 98 66 84 Z" fill="#fff"/><circle cx="60" cy="66" r="16" fill="#fde047"/></svg>`,
            pain_belly: `<svg viewBox="0 0 120 120"><rect x="28" y="18" width="64" height="84" rx="26" stroke="#fff" stroke-width="8" fill="none"/><circle cx="60" cy="68" r="18" fill="#fde047"/><circle cx="60" cy="68" r="28" stroke="#fde047" stroke-width="5" stroke-dasharray="8 6" fill="none"/></svg>`,
            pain_back: `<svg viewBox="0 0 120 120"><path d="M46 18 C36 44 36 76 46 102 H78 C70 76 70 44 78 18 Z" fill="#fff"/><circle cx="42" cy="44" r="12" fill="#fde047"/><circle cx="40" cy="72" r="14" fill="#fde047"/></svg>`,
            pain_limbs: `<svg viewBox="0 0 120 120"><path d="M26 26 L52 62 L38 96 M78 24 L92 60 L78 96" stroke="#fff" stroke-width="14" stroke-linecap="round" stroke-linejoin="round" fill="none"/><circle cx="52" cy="62" r="13" fill="#fde047"/><circle cx="92" cy="60" r="13" fill="#fde047"/></svg>`,
            pain_wound: `<svg viewBox="0 0 120 120"><rect x="20" y="42" width="80" height="36" rx="14" transform="rotate(-25 60 60)" fill="#fde047" stroke="#fff" stroke-width="5"/><rect x="46" y="46" width="28" height="28" rx="6" transform="rotate(-25 60 60)" fill="#fff"/></svg>`,

            food_water: `<svg viewBox="0 0 120 120"><path d="M30 20 H90 L82 100 H38 Z" stroke="#fff" stroke-width="6" fill="rgba(255,255,255,0.25)"/><path d="M34 52 H86 L80 96 H40 Z" fill="#38bdf8"/><circle cx="60" cy="40" r="8" fill="#fde047"/></svg>`,
            food_milk: `<svg viewBox="0 0 120 120"><rect x="28" y="34" width="54" height="68" rx="10" fill="#fff"/><path d="M82 46 H96 C102 46 102 82 96 82 H82" stroke="#fde047" stroke-width="8" fill="none"/><path d="M46 34 L58 16 H74" stroke="#fde047" stroke-width="7" fill="none" stroke-linecap="round"/></svg>`,
            food_porridge: `<svg viewBox="0 0 120 120"><path d="M18 62 H102 C102 90 82 102 60 102 C38 102 18 90 18 62 Z" fill="#fff"/><path d="M38 46 Q38 30 46 22 M60 46 Q60 30 68 22 M82 46 Q82 30 90 22" stroke="#fde047" stroke-width="6" fill="none" stroke-linecap="round"/></svg>`,
            food_rice: `<svg viewBox="0 0 120 120"><path d="M22 60 C22 34 98 34 98 60 Z" fill="#fde047"/><path d="M16 60 H104 C104 90 84 102 60 102 C36 102 16 90 16 60 Z" fill="#fff"/></svg>`,
            food_fruit: `<svg viewBox="0 0 120 120"><circle cx="60" cy="66" r="36" fill="#fb923c" stroke="#fff" stroke-width="5"/><path d="M60 30 Q72 14 86 22 Q74 34 60 30 Z" fill="#4ade80"/></svg>`,
            food_meds: `<svg viewBox="0 0 120 120"><g transform="rotate(-35 60 60)"><rect x="22" y="42" width="38" height="36" rx="18" fill="#e11d48" stroke="#fff" stroke-width="4"/><rect x="60" y="42" width="38" height="36" rx="18" fill="#fde047" stroke="#fff" stroke-width="4"/></g></svg>`,

            life_toilet: `<svg viewBox="0 0 120 120"><rect x="26" y="20" width="26" height="44" rx="6" fill="#fde047"/><path d="M26 64 H94 C94 86 78 96 60 96 C42 96 26 86 26 64 Z" fill="#fff"/><rect x="44" y="94" width="32" height="12" rx="4" fill="#fff"/></svg>`,
            life_liedown: `<svg viewBox="0 0 120 120"><path d="M14 84 H106 M20 84 V102 M100 84 V102" stroke="#fff" stroke-width="8" stroke-linecap="round"/><circle cx="30" cy="66" r="11" fill="#fde047"/><rect x="46" y="60" width="54" height="16" rx="8" fill="#fff"/><path d="M68 20 V46 M54 34 L68 48 L82 34" stroke="#fde047" stroke-width="7" fill="none" stroke-linecap="round"/></svg>`,
            life_situp: `<svg viewBox="0 0 120 120"><path d="M14 88 H106 M20 88 V104 M100 88 V104" stroke="#fff" stroke-width="8" stroke-linecap="round"/><circle cx="38" cy="36" r="11" fill="#fde047"/><path d="M28 52 L48 78 H96" stroke="#fff" stroke-width="14" fill="none" stroke-linecap="round"/><path d="M82 54 V24 M68 38 L82 22 L96 38" stroke="#fde047" stroke-width="7" fill="none" stroke-linecap="round"/></svg>`,
            life_turn: `<svg viewBox="0 0 120 120"><path d="M26 60 C26 34 94 34 94 60" stroke="#fde047" stroke-width="9" fill="none" stroke-linecap="round"/><path d="M78 42 L96 60 L78 78" stroke="#fde047" stroke-width="9" fill="none" stroke-linecap="round"/><circle cx="60" cy="78" r="18" fill="#fff"/></svg>`,
            life_clothes: `<svg viewBox="0 0 120 120"><path d="M36 24 L14 42 L26 58 L38 48 V96 H82 V48 L94 58 L106 42 L84 24 L72 36 H48 Z" fill="#fff"/><circle cx="60" cy="64" r="12" fill="#fde047"/></svg>`,
            life_blanket: `<svg viewBox="0 0 120 120"><rect x="18" y="28" width="84" height="66" rx="14" fill="#38bdf8" stroke="#fff" stroke-width="5"/><path d="M18 52 H102 M18 72 H102" stroke="#fde047" stroke-width="5"/></svg>`,

            help_doctor: `<svg viewBox="0 0 120 120"><path d="M30 86 H90 L84 44 C82 28 38 28 36 44 Z" fill="#fde047"/><rect x="20" y="86" width="80" height="12" rx="6" fill="#fff"/><rect x="54" y="44" width="12" height="32" rx="3" fill="#e11d48"/><rect x="44" y="54" width="32" height="12" rx="3" fill="#e11d48"/></svg>`,
            help_nurse: `<svg viewBox="0 0 120 120"><path d="M20 42 L60 22 L100 42 L88 68 H32 Z" fill="#fff"/><rect x="55" y="32" width="10" height="24" rx="2" fill="#e11d48"/><rect x="48" y="39" width="24" height="10" rx="2" fill="#e11d48"/><circle cx="60" cy="86" r="14" fill="#fde047"/></svg>`,
            help_family: `<svg viewBox="0 0 120 120"><path d="M16 58 L60 20 L104 58" stroke="#fde047" stroke-width="9" fill="none" stroke-linecap="round"/><path d="M28 54 V98 H92 V54" fill="#fff"/><circle cx="48" cy="70" r="9" fill="#0284c7"/><circle cx="72" cy="70" r="9" fill="#e11d48"/></svg>`,
            help_bp: `<svg viewBox="0 0 120 120"><rect x="26" y="22" width="68" height="76" rx="12" fill="#fff"/><circle cx="60" cy="54" r="20" fill="#e11d48"/><path d="M48 54 H56 L60 42 L66 64 L70 54 H74" stroke="#fde047" stroke-width="4" fill="none"/></svg>`,
            help_breath: `<svg viewBox="0 0 120 120"><circle cx="60" cy="64" r="32" fill="#38bdf8" stroke="#fff" stroke-width="6"/><rect x="44" y="56" width="32" height="26" rx="10" fill="#fde047"/><path d="M60 82 V104" stroke="#fff" stroke-width="8" stroke-linecap="round"/></svg>`,
            help_massage: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="36" fill="#fde047" opacity="0.3"/><path d="M32 76 C32 46 88 46 88 76" stroke="#fff" stroke-width="10" fill="none" stroke-linecap="round"/><circle cx="60" cy="40" r="14" fill="#fde047"/></svg>`,

            room_light: `<svg viewBox="0 0 120 120"><circle cx="60" cy="48" r="28" fill="#fde047" stroke="#fff" stroke-width="5"/><rect x="46" y="78" width="28" height="18" rx="6" fill="#fff"/></svg>`,
            room_fan: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="12" fill="#fde047"/><path d="M60 48 C44 16 76 16 60 48 M72 60 C104 44 104 76 72 60 M60 72 C76 104 44 104 60 72 M48 60 C16 76 16 44 48 60" fill="#fff"/></svg>`,
            room_cold: `<svg viewBox="0 0 120 120"><path d="M60 16 V104 M16 60 H104 M28 28 L92 92 M92 28 L28 92" stroke="#fff" stroke-width="8" stroke-linecap="round"/><circle cx="60" cy="60" r="14" fill="#fde047"/></svg>`,
            room_hot: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="26" fill="#fde047" stroke="#fff" stroke-width="5"/><path d="M60 12 V24 M60 96 V108 M12 60 H24 M96 60 H108 M26 26 L34 34 M86 86 L94 94 M94 26 L86 34 M34 86 L26 94" stroke="#fde047" stroke-width="7" stroke-linecap="round"/></svg>`,
            room_book: `<svg viewBox="0 0 120 120"><path d="M18 30 Q40 24 60 34 Q80 24 102 30 V92 Q80 84 60 94 Q40 84 18 92 Z" fill="#fff"/><path d="M60 34 V94" stroke="#fde047" stroke-width="6"/></svg>`,
            room_lock: `<svg viewBox="0 0 120 120"><rect x="28" y="52" width="64" height="50" rx="12" fill="#fde047" stroke="#fff" stroke-width="5"/><path d="M40 52 V36 C40 20 80 20 80 36 V52" stroke="#fff" stroke-width="8" fill="none"/></svg>`,

            feel_good: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="44" fill="#fde047" stroke="#fff" stroke-width="5"/><circle cx="44" cy="48" r="6" fill="#0f172a"/><circle cx="76" cy="48" r="6" fill="#0f172a"/><path d="M36 68 Q60 92 84 68" stroke="#0f172a" stroke-width="7" fill="none" stroke-linecap="round"/></svg>`,
            feel_sleepy: `<svg viewBox="0 0 120 120"><path d="M74 22 C44 22 28 56 48 84 C64 104 94 96 98 76 C72 82 54 56 74 22 Z" fill="#fde047" stroke="#fff" stroke-width="4"/></svg>`,
            feel_thanks: `<svg viewBox="0 0 120 120"><path d="M60 98 C60 98 18 70 18 42 C18 24 36 18 50 30 L60 40 L70 30 C84 18 102 24 102 42 C102 70 60 98 60 98 Z" fill="#fde047" stroke="#fff" stroke-width="5"/></svg>`,
            feel_yes: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="44" fill="#fff"/><path d="M34 62 L52 80 L88 42" stroke="#059669" stroke-width="12" fill="none" stroke-linecap="round" stroke-linejoin="round"/></svg>`,
            feel_no: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="44" fill="#fff"/><path d="M38 38 L82 82 M82 38 L38 82" stroke="#e11d48" stroke-width="12" stroke-linecap="round"/></svg>`,
            feel_tv: `<svg viewBox="0 0 120 120"><rect x="18" y="28" width="84" height="58" rx="10" fill="#fff"/><rect x="26" y="36" width="68" height="42" rx="6" fill="#0284c7"/><rect x="42" y="92" width="36" height="8" rx="4" fill="#fde047"/></svg>`
        };

        const storyLibrary = [
            {
                title: "Hạt Giống Tâm Hồn: Tia Nắng Buổi Sáng",
                content: `
                <p>Mỗi sớm mai thức dậy là một món quà tuyệt vời của cuộc sống. Ánh nắng ban mai len lỏi qua ô cửa sổ, mang theo sự ấm áp và niềm hy vọng mới mẻ cho ngày hôm nay.</p>
                <p>Dù phía trước có những khó khăn hay thử thách, chỉ cần trái tim chúng ta luôn hướng về phía ánh sáng, sự bình an và sức mạnh nội tại sẽ luôn đồng hành cùng bạn.</p>
                <p>Hãy hít thở thật sâu, cảm nhận từng nhịp đập dịu êm của cơ thể và mỉm cười đón nhận ngày mới đầy bình yên bạn nhé.</p>
                `
            },
            {
                title: "Góc Bình Yên: Thanh Âm Cuộc Sống",
                content: `
                <p>Sự tĩnh lặng của tâm hồn giúp cơ thể phục hồi năng lượng nhanh hơn bất kỳ phương thuốc nào. Khi đôi mắt được thả lỏng và hơi thở trở nên đều đặn, mọi căng thẳng sẽ dần tan biến.</p>
                <p>Bên ngoài khung cửa, bầu trời vẫn xanh trong và những đám mây trắng vẫn nhẹ nhàng trôi, gửi đến bạn lời chúc bình an và khỏe mạnh mỗi ngày.</p>
                `
            }
        ];

        let currentStoryIndex = 0;
        let isAutoScrolling = false;
        let autoScrollTimer = null;
        let isFatigueLocked = false;
        let isControlLocked = false;
        let currentNodMode = "DUAL";
        let isAudioScanEnabled = true;
        let isPhotoEditMode = false;
        let useRealPhotoMode = localStorage.getItem('aac_real_mode') !== '0';
        let pendingPhotoSlotKey = null;
        let currentQuickSubView = 'main';

        // =========================================================================
        // 3 CHẾ ĐỘ QUÉT THÍCH ỨNG THÔNG MINH (SMART / DIRECT_ITEM / ROW_COL)
        // - SMART (Mặc định): Quét Từng Ô (1 nhịp ăn ngay) ở Menu, Hình Ảnh, Đọc Báo;
        //                     Quét Hàng-Cột chống trượt ở Bàn Phím Gõ Chữ.
        // - ITEM (Từng Ô): Quét Từng Ô 1 nhịp trên mọi màn hình (Rủi ro nhầm = 0%).
        // - ROW_COL (Hàng-Cột): Quét Hàng trước, Cột sau truyền thống.
        // =========================================================================
        let scanStrategy = localStorage.getItem('aac_scan_strategy') || 'SMART';

        function isDirectItemScanActive() {
            if (scanStrategy === 'ITEM') return true;
            if (scanStrategy === 'ROW_COL') return false;
            // SMART: Trang chủ, Bảng Hình Ảnh, Đọc Báo dùng Quét Từng Ô trực tiếp (1 nhịp chọn ngay)
            return (currentView !== 'KEYBOARD');
        }

        function updateScanStrategyUI() {
            const btn = document.getElementById('top-scanmode-pill');
            if (!btn) return;
            if (scanStrategy === 'SMART') btn.innerText = "Kiểu Quét: Thông Minh";
            else if (scanStrategy === 'ITEM') btn.innerText = "Kiểu Quét: Từng Ô (1 Nhịp)";
            else btn.innerText = "Kiểu Quét: Hàng - Cột";
        }
        updateScanStrategyUI();

        function cycleScanStrategy() {
            if (scanStrategy === 'SMART') scanStrategy = 'ITEM';
            else if (scanStrategy === 'ITEM') scanStrategy = 'ROW_COL';
            else scanStrategy = 'SMART';
            localStorage.setItem('aac_scan_strategy', scanStrategy);
            updateScanStrategyUI();
            buildScanTree();
            if (!isSleeping && !isSettingsOpen && !isControlLocked) startScanning();
            const msg = (scanStrategy === 'SMART') ? "Kiểu quét Thông minh: 1 nhịp cho Hình ảnh, Hàng cột cho Bàn phím" :
                        (scanStrategy === 'ITEM')  ? "Kiểu quét Từng ô: Chớp mắt 1 lần chọn ngay" :
                                                     "Kiểu quét Hàng và Cột";
            showToastAlert(msg);
        }

        function updateRealModeButtonUI() {
            const btn = document.getElementById('top-realmode-pill');
            if (!btn) return;
            if (useRealPhotoMode) {
                btn.innerText = "Hiển Thị: Ảnh Thật";
                btn.classList.add('photo-mode-active');
            } else {
                btn.innerText = "Hiển Thị: Hình Vẽ";
                btn.classList.remove('photo-mode-active');
            }
        }
        updateRealModeButtonUI();

        function toggleRealPhotoMode() {
            useRealPhotoMode = !useRealPhotoMode;
            localStorage.setItem('aac_real_mode', useRealPhotoMode ? '1' : '0');
            updateRealModeButtonUI();
            if (currentView === 'QUICK') renderQuickGrid(currentQuickSubView);
            showToastAlert(useRealPhotoMode ? "Đã bật chế độ Ảnh Chụp Thực Tế" : "Đã chuyển sang Hình Minh Họa Rõ Nét");
        }

        function togglePhotoEditMode() {
            isPhotoEditMode = !isPhotoEditMode;
            document.body.classList.toggle('photo-edit-mode', isPhotoEditMode);
            const pill = document.getElementById('top-photo-pill');
            if (isPhotoEditMode) {
                pill.innerText = "Tải Ảnh Riêng: Bật";
                pill.style.background = "var(--warning)";
                pill.style.color = "#0f172a";
                if (currentView !== 'QUICK') showQuick();
                showToastAlert("Chạm vào ô bất kỳ để chụp/chọn ảnh thật từ máy");
            } else {
                pill.innerText = "Tải Ảnh Riêng: Tắt";
                pill.style.background = "var(--key-sys-bg)";
                pill.style.color = "var(--text-main)";
            }
        }

        function resetAllCustomPhotos() {
            Object.keys(localStorage).forEach(k => {
                if (k.startsWith('aac_photo_')) localStorage.removeItem(k);
            });
            renderQuickGrid(currentQuickSubView);
            showToastAlert("Đã khôi phục bộ ảnh thực tế gốc");
        }

        document.getElementById('real-photo-input').addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (!file || !pendingPhotoSlotKey) return;
            const reader = new FileReader();
            reader.onload = function(evt) {
                const img = new Image();
                img.onload = function() {
                    const canvas = document.createElement('canvas');
                    canvas.width = 280; canvas.height = 280;
                    const ctx = canvas.getContext('2d');
                    const minSide = Math.min(img.width, img.height);
                    const sx = (img.width - minSide) / 2;
                    const sy = (img.height - minSide) / 2;
                    ctx.drawImage(img, sx, sy, minSide, minSide, 0, 0, 280, 280);
                    try {
                        localStorage.setItem('aac_photo_' + pendingPhotoSlotKey, canvas.toDataURL('image/jpeg', 0.82));
                        renderQuickGrid(currentQuickSubView);
                        showToastAlert("Đã lưu ảnh thật vào bộ nhớ máy!");
                    } catch(err) {
                        showToastAlert("Bộ nhớ ảnh trình duyệt đã đầy");
                    }
                };
                img.src = evt.target.result;
            };
            reader.readAsDataURL(file);
            e.target.value = '';
        });

        function buildVisualBoxHTML(item) {
            const svgCode = pictograms[item.id] || pictograms.cat_help;
            const savedPhoto = localStorage.getItem('aac_photo_' + item.id);
            if (savedPhoto) {
                return `<img class="real-photo-img" src="${savedPhoto}" alt="${item.title}">`;
            }
            if (useRealPhotoMode && item.photo) {
                return `<img class="real-photo-img" src="${item.photo}" alt="${item.title}" loading="eager" onerror="this.style.display='none'; this.nextElementSibling.style.display='flex';">
                        <div class="svg-fallback" style="display:none">${svgCode}</div>`;
            }
            return `<div class="svg-fallback">${svgCode}</div>`;
        }

        function loadStory(idx) {
            currentStoryIndex = idx % storyLibrary.length;
            const item = storyLibrary[currentStoryIndex];
            document.getElementById('story-title').innerText = item.title;
            document.getElementById('reader-content').innerHTML = item.content;
            const vp = document.getElementById('reader-content');
            if (vp) vp.scrollTop = 0;
        }

        function cycleStory() { loadStory(currentStoryIndex + 1); speakActionFeedback("Đã đổi bài đọc"); }
        function scrollReader(dir) {
            const vp = document.getElementById('reader-content');
            if (vp) vp.scrollBy({ top: dir * 240, behavior: 'smooth' });
            speakActionFeedback(dir > 0 ? "Cuộn xuống" : "Cuộn lên");
        }

        function toggleAutoScroll() {
            isAutoScrolling = !isAutoScrolling;
            const btn = document.getElementById('btn-auto-scroll');
            const vp = document.getElementById('reader-content');
            if (isAutoScrolling) {
                btn.classList.add('active'); btn.innerText = "Tự Cuộn: Bật";
                speakActionFeedback("Bắt đầu tự động cuộn");
                if (autoScrollTimer) clearInterval(autoScrollTimer);
                autoScrollTimer = setInterval(() => { if (vp) vp.scrollBy({ top: 2, behavior: 'smooth' }); }, 50);
            } else {
                btn.classList.remove('active'); btn.innerText = "Tự Cuộn: Tắt";
                speakActionFeedback("Dừng tự động cuộn");
                clearInterval(autoScrollTimer); autoScrollTimer = null;
            }
        }

        function readArticleTTS() { speakActionFeedback(document.getElementById('reader-content').innerText); }

        function toggleTheme() {
            const html = document.documentElement;
            const newTheme = html.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
            html.setAttribute('data-theme', newTheme);
            localStorage.setItem('bci-theme', newTheme);
            document.getElementById('theme-icon').innerText = newTheme === 'dark' ? 'Nền: Tối' : 'Nền: Sáng';
        }
        const savedTheme = localStorage.getItem('bci-theme') || 'dark';
        document.documentElement.setAttribute('data-theme', savedTheme);
        document.getElementById('theme-icon').innerText = savedTheme === 'dark' ? 'Nền: Tối' : 'Nền: Sáng';

        function toggleAudioScan() {
            isAudioScanEnabled = !isAudioScanEnabled;
            const pill = document.getElementById('top-audio-pill');
            pill.innerText = isAudioScanEnabled ? "Âm Quét: Bật" : "Âm Quét: Tắt";
            pill.style.opacity = isAudioScanEnabled ? "1" : "0.6";
            if (!isAudioScanEnabled && 'speechSynthesis' in window) {
                try { window.speechSynthesis.cancel(); } catch(e) {}
            }
            if (!isSleeping && !isSettingsOpen && !isControlLocked) startScanning();
        }

        function toggleHybridTrack() {
            if (currentView === 'QUICK') showKeyboard();
            else showQuick();
        }

        const hostIP = window.location.hostname || "esp32-bci.local";
        let ws;
        let currentView = 'MENU';
        let isSleeping = false;

        function sendCommand(cmd) { if (ws && ws.readyState === WebSocket.OPEN) ws.send(cmd); }

        function applyControlLockState(locked, announce = true) {
            isControlLocked = locked;
            const lockPill = document.getElementById('top-lock-pill');
            const banner = document.getElementById('control-lock-banner');
            const bannerText = document.getElementById('lock-banner-text');
            const sigVal = document.getElementById('sig-val');

            if (locked) {
                stopScannerTimer();
                renderScan();
                lockPill.classList.add('locked');
                lockPill.innerText = "Đang Khóa";
                if (bannerText) bannerText.innerText = "Đang tạm khóa nghỉ (0/3)";
                banner.style.display = 'flex';
                if (sigVal) { sigVal.innerText = "Tạm Khóa"; sigVal.className = "dash-value bad"; }
                if (announce) {
                    showToastAlert("Đã khóa tạm nghỉ");
                    speakActionFeedback("Đã khóa điều khiển");
                }
            } else {
                lockPill.classList.remove('locked');
                lockPill.innerText = "Đang Mở";
                banner.style.display = 'none';
                if (sigVal) { sigVal.innerText = "Sẵn Sàng"; sigVal.className = "dash-value good"; }
                if (announce) {
                    showToastAlert("Đã mở khóa");
                    speakActionFeedback("Đã mở khóa", () => {
                        if (!isSleeping && !isSettingsOpen && !isFatigueLocked) startScanning();
                    });
                } else {
                    if (!isSleeping && !isSettingsOpen && !isFatigueLocked) startScanning();
                }
            }
        }

        function updateNodModeUI(modeStr, announce = false) {
            currentNodMode = modeStr;
            const topPill = document.getElementById('top-mode-pill');
            const menuBtn = document.getElementById('menu-mode-label');

            if (modeStr === "LIGHT") {
                topPill.innerText = "Độ Nhạy: Nhẹ";
                if (menuBtn) menuBtn.innerText = "Nhẹ";
                if (announce) speakActionFeedback("Chế độ nhạy nhẹ");
            } else if (modeStr === "HEAVY") {
                topPill.innerText = "Độ Nhạy: Mạnh";
                if (menuBtn) menuBtn.innerText = "Mạnh";
                if (announce) speakActionFeedback("Chế độ nhạy mạnh");
            } else {
                topPill.innerText = "Độ Nhạy: Kép";
                if (menuBtn) menuBtn.innerText = "Kép";
                if (announce) speakActionFeedback("Chế độ nhạy kép");
            }
        }

        function cycleNodMode() {
            if (currentNodMode === "DUAL") sendCommand("MODE_LIGHT");
            else if (currentNodMode === "LIGHT") sendCommand("MODE_HEAVY");
            else sendCommand("MODE_DUAL");
        }

        let currentInputSource = "HYBRID";
        function updateInputSourceUI(modeStr, announce = false) {
            currentInputSource = modeStr;
            const topPill = document.getElementById('top-input-pill');
            if (!topPill) return;

            if (modeStr === "IR_ONLY" || modeStr === "IR") {
                currentInputSource = "IR_ONLY";
                topPill.innerText = "Chỉ Kính IR";
                if (announce) speakActionFeedback("Chỉ dùng kính hồng ngoại");
            } else if (modeStr === "NOD_ONLY" || modeStr === "NOD") {
                currentInputSource = "NOD_ONLY";
                topPill.innerText = "Chỉ Gật Đầu";
                if (announce) speakActionFeedback("Chỉ dùng cảm biến gật đầu");
            } else {
                currentInputSource = "HYBRID";
                topPill.innerText = "Kính IR + IMU";
                if (announce) speakActionFeedback("Chế độ cảm biến kép");
            }
        }

        function cycleInputSource() {
            if (currentInputSource === "HYBRID") sendCommand("INPUT_IR");
            else if (currentInputSource === "IR_ONLY") sendCommand("INPUT_NOD");
            else sendCommand("INPUT_HYBRID");
        }

        let hasInitializedLockUI = false;
        function connectWebSocket() {
            ws = new WebSocket(`ws://${hostIP}:81`);
            ws.onclose = () => { setTimeout(connectWebSocket, 2500); };
            
            ws.onmessage = (event) => {
                const data = event.data;
                if (data === "LOCK_CONTROL:ON") {
                    applyControlLockState(true, hasInitializedLockUI);
                    hasInitializedLockUI = true;
                }
                else if (data === "LOCK_CONTROL:OFF") {
                    applyControlLockState(false, hasInitializedLockUI);
                    hasInitializedLockUI = true;
                }
                else if (data.startsWith("UNLOCK_PROGRESS:")) {
                    let cnt = data.split(':')[1] || "1";
                    playClickBeep('LIGHT');
                    const bannerText = document.getElementById('lock-banner-text');
                    if (bannerText) bannerText.innerText = `Đang mở khóa (${cnt}/3)`;
                }
                else if (data.startsWith("ALERT:")) {
                    let payload = data.substring(6);
                    if (payload.startsWith("SPASM_FALL")) {
                        playClickBeep('SOS');
                        showToastAlert("CẢNH BÁO KHẨN CẤP");
                        speakActionFeedback("Cảnh báo khẩn cấp! Phát hiện dấu hiệu bất thường!");
                    } else if (payload.startsWith("BEDSORE_120M")) {
                        showToastAlert("Nhắc hỗ trợ đổi tư thế nằm");
                        speakActionFeedback("Nhắc điều dưỡng hỗ trợ nghiêng người cho bệnh nhân.");
                    }
                }
                else if (data.startsWith("BLINK:")) {
                    if (isControlLocked) return;

                    let parts = data.substring(6).split(',');
                    let strength = parts[0] || "80";
                    let nodType = parts[1] || "LIGHT";

                    playClickBeep(nodType);

                    let colorHex = (nodType === "HEAVY") ? "var(--warning)" : "var(--success)";
                    const attValEl = document.getElementById('att-val');
                    attValEl.innerText = `${strength}%`;
                    attValEl.style.color = colorHex;

                    const elMini = document.getElementById('att-mini');
                    if (elMini) {
                        elMini.innerText = `${strength}%`;
                        elMini.style.color = colorHex;
                    }

                    triggerInstantNodClick(nodType);
                }
                else if (data.startsWith("MODE:")) {
                    updateNodModeUI(data.substring(5).trim(), false);
                }
                else if (data.startsWith("INPUT_MODE:")) {
                    updateInputSourceUI(data.substring(11).trim(), false);
                }
                else if (data.startsWith("TGAM:")) {
                    let parts = data.substring(5).split(',');
                    if (parts.length >= 2) updateSensorDisplay(parseInt(parts[0]), parseInt(parts[1]));
                    if (parts.length >= 5) {
                        let cfi = parts[3];
                        const cfiEl = document.getElementById('cfi-val');
                        const cfiMini = document.getElementById('cfi-mini');
                        if (cfiEl) {
                            cfiEl.innerText = `${cfi}%`;
                            cfiEl.style.color = (parseInt(cfi) >= 60) ? "var(--danger)" : "var(--warning)";
                        }
                        if (cfiMini) cfiMini.innerText = `${cfi}%`;
                    }
                } 
                else if (data.startsWith("INA:")) {
                    let parts = data.substring(4).split(',');
                    if (parts.length >= 4) {
                        let batPct = parts[3];
                        document.getElementById('bat-val').innerText = `${batPct}%`;
                        const elMini = document.getElementById('bat-mini');
                        if (elMini) elMini.innerText = `${batPct}%`;
                    }
                }
                else if (data.startsWith("RELAY:")) {
                    let parts = data.substring(6).split(',');
                    updateRelayUI(1, parts[0] === '1'); updateRelayUI(2, parts[1] === '1');
                } 
                else if (data.startsWith("FATIGUE_LOCK:")) {
                    let parts = data.substring(13).split(',');
                    enterFatigueLockMode(parseInt(parts[0]));
                }
                else if (data === "FATIGUE_UNLOCK") exitFatigueLockMode();
                else if (data === "SLEEPING") enterSleepMode();
                else if (data === "WAKEUP") exitSleepMode();
                
                // THÊM ĐOẠN NHẬN DỮ LIỆU DEBUG HỒNG NGOẠI:
                else if (data.startsWith("IR_DEBUG:")) {
                    let parts = data.substring(9).split(',');
                    if(parts.length >= 4) {
                        const dbgBanner = document.getElementById('ir-debug-banner');
                        if(dbgBanner) {
                            dbgBanner.style.display = 'block';
                            document.getElementById('ir-amb').innerText = parts[0];
                            document.getElementById('ir-on').innerText = parts[1];
                            document.getElementById('ir-delta').innerText = parts[2];
                            document.getElementById('ir-thresh').innerText = parts[3];
                        }
                    }
                }
            }; // Kết thúc ws.onmessage
        }
        connectWebSocket();

        let badgeTimer = null;
        let lastRowEnterTimestamp = 0;

        function triggerInstantNodClick(nodType = "LIGHT") {
            if (isSleeping || isSettingsOpen || isFatigueLocked || isControlLocked) return;

            const counterEl = document.getElementById('blink-counter');
            counterEl.innerText = (nodType === "HEAVY") ? "Đã Chọn (Mạnh)" : "Đã Chọn";
            counterEl.style.background = (nodType === "HEAVY") ? "var(--warning)" : "var(--success)";
            counterEl.style.display = 'block';
            if (badgeTimer) clearTimeout(badgeTimer);
            badgeTimer = setTimeout(() => { counterEl.style.display = 'none'; }, 800);

            executeNormalSelection();
        }

        function enterFatigueLockMode(secondsLeft) {
            const alreadyLocked = isFatigueLocked;
            isFatigueLocked = true;
            if (!alreadyLocked) {
                stopScannerTimer();
                if (isAutoScrolling) toggleAutoScroll();
            }

            document.getElementById('fatigue-lock-overlay').style.display = 'flex';
            let m = Math.floor(secondsLeft / 60), s = secondsLeft % 60;
            document.getElementById('fatigue-timer-display').innerText = `${m < 10 ? '0' : ''}${m}:${s < 10 ? '0' : ''}${s}`;

            if (!alreadyLocked) speakActionFeedback("Hệ thống tạm nghỉ 30 phút để thư giãn cơ.");
        }

        function exitFatigueLockMode() {
            isFatigueLocked = false;
            document.getElementById('fatigue-lock-overlay').style.display = 'none';
            speakActionFeedback("Hệ thống đã mở lại.", () => {
                if (!isControlLocked) startScanning();
            });
        }

        function updateClock() {
            const now = new Date();
            document.getElementById('clock-time').innerText = now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', hour12: false });
            document.getElementById('clock-date').innerText = `${now.getDate().toString().padStart(2,'0')}/${(now.getMonth()+1).toString().padStart(2,'0')}/${now.getFullYear()}`;
        }
        setInterval(updateClock, 1000); updateClock();

        function updateSensorDisplay(sig, att) {
            let elSig = document.getElementById('sig-val'), elSigMini = document.getElementById('sig-mini');
            if (isControlLocked) {
                elSig.innerText = "Tạm Khóa"; elSig.className = "dash-value bad";
                if(elSigMini) { elSigMini.innerText = "Tạm Khóa"; elSigMini.style.color = "var(--warning)"; }
                return;
            }
            if (sig === 0) { 
                elSig.innerText = "Sẵn Sàng"; elSig.className = "dash-value good"; 
                if(elSigMini) { elSigMini.innerText = "Sẵn Sàng"; elSigMini.style.color = "var(--success)"; } 
            } else { 
                elSig.innerText = "Chờ Cảm Biến"; elSig.className = "dash-value bad"; 
                if(elSigMini) { elSigMini.innerText = "Chờ Cảm Biến"; elSigMini.style.color = "var(--warning)"; } 
            }
        }

        function updateTrackButtonUI() {
            const trackPill = document.getElementById('top-track-pill');
            if (!trackPill) return;
            if (currentView === 'QUICK') trackPill.innerText = "Sang Bàn Phím";
            else if (currentView === 'KEYBOARD') trackPill.innerText = "Sang Hình Ảnh";
            else trackPill.innerText = "Hình Ảnh / Phím";
        }

        function showMenu() {
            currentView = 'MENU';
            stopScannerTimer();
            document.getElementById('view-keyboard').style.display = 'none'; 
            document.getElementById('view-reader').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'none';
            document.getElementById('view-menu').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'none';
            if (isAutoScrolling) toggleAutoScroll();
            updateTrackButtonUI();
            buildScanTree();
            if (!isControlLocked) startScanning();
        }

        function showKeyboard() {
            currentView = 'KEYBOARD';
            stopScannerTimer();
            document.getElementById('view-menu').style.display = 'none'; 
            document.getElementById('view-reader').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'none';
            document.getElementById('view-keyboard').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'flex';
            if (isAutoScrolling) toggleAutoScroll();
            updateTrackButtonUI();
            buildScanTree();
            if (!isControlLocked) startScanning();
        }

        function showReader() {
            currentView = 'READER';
            stopScannerTimer();
            document.getElementById('view-menu').style.display = 'none'; 
            document.getElementById('view-keyboard').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'none';
            document.getElementById('view-reader').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'flex';
            loadStory(currentStoryIndex);
            updateTrackButtonUI();
            buildScanTree();
            if (!isControlLocked) startScanning();
        }

        function renderQuickGrid(type) {
            currentQuickSubView = type;
            stopScannerTimer();
            const grid = document.getElementById('quick-grid');
            const backBtn = document.getElementById('btn-quick-back');
            grid.innerHTML = '';

            const items = quickData[type] || quickData.main;
            backBtn.style.display = (type === 'main') ? 'none' : 'inline-block';

            let html = '';
            for (let i = 0; i < items.length; i += 2) {
                const item1 = items[i], item2 = items[i + 1];
                const cls1 = (type === 'main') ? 'aac-card quick-cat scan-item' : 'aac-card quick-phrase scan-item';
                const cls2 = (type === 'main') ? 'aac-card quick-cat scan-item' : 'aac-card quick-phrase scan-item';

                html += `
                <div class="aac-row-2 scan-group">
                    <button class="${cls1}"
                        data-slot="${item1.id}"
                        data-target="${item1.target || ''}"
                        data-voice="${item1.voice}"
                        data-speakout="${item1.speakOut || item1.voice}"
                        data-phrase="${item1.title}"
                        data-syscmd="${item1.sysCmd || ''}"
                        data-sos="${item1.isSOS ? '1' : '0'}"
                        style="background: ${item1.bg};">
                        <span class="photo-edit-badge">Chạm để đổi ảnh thật</span>
                        <div class="aac-visual-box">${buildVisualBoxHTML(item1)}</div>
                        <div class="aac-title">${item1.title}</div>
                    </button>
                    <button class="${cls2}"
                        data-slot="${item2.id}"
                        data-target="${item2.target || ''}"
                        data-voice="${item2.voice}"
                        data-speakout="${item2.speakOut || item2.voice}"
                        data-phrase="${item2.title}"
                        data-syscmd="${item2.sysCmd || ''}"
                        data-sos="${item2.isSOS ? '1' : '0'}"
                        style="background: ${item2.bg};">
                        <span class="photo-edit-badge">Chạm để đổi ảnh thật</span>
                        <div class="aac-visual-box">${buildVisualBoxHTML(item2)}</div>
                        <div class="aac-title">${item2.title}</div>
                    </button>
                </div>`;
            }
            grid.innerHTML = html;

            grid.querySelectorAll('.scan-item').forEach(btn => {
                btn.addEventListener('click', (e) => { 
                    e.preventDefault(); e.stopPropagation(); 
                    if (isPhotoEditMode) {
                        pendingPhotoSlotKey = btn.getAttribute('data-slot');
                        document.getElementById('real-photo-input').click();
                        return;
                    }
                    executeAction(btn); 
                    if (document.activeElement) document.activeElement.blur(); 
                });
            });

            buildScanTree();
            if (!isControlLocked) startScanning();
        }

        function showQuick() {
            currentView = 'QUICK';
            stopScannerTimer();
            document.getElementById('view-menu').style.display = 'none'; 
            document.getElementById('view-keyboard').style.display = 'none'; 
            document.getElementById('view-reader').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'flex';
            if (isAutoScrolling) toggleAutoScroll();
            updateTrackButtonUI();
            renderQuickGrid('main');
        }

        // =========================================================================
        // HỆ THỐNG GIỌNG ĐỌC ĐỒNG BỘ 100% VỚI BỘ QUÉT (KHÔNG BAO GIỜ BỊ CẮT NGANG)
        // =========================================================================
        let voices = []; 
        let ttsRate = parseFloat(localStorage.getItem('ttsRate')) || 1.00; 
        let ttsVolume = parseFloat(localStorage.getItem('ttsVolume')) || 1.0;
        let savedVoiceURI = localStorage.getItem('ttsVoiceURI'); 
        let isSettingsOpen = false;
        let activeSpeechToken = 0;
        let activeSpeechSafetyTimer = null;
        let isActionSpeechPlaying = false;

        document.getElementById('rate-slider').value = ttsRate;
        document.getElementById('rate-val').innerText = ttsRate.toFixed(2);
        document.getElementById('vol-slider').value = ttsVolume;
        document.getElementById('vol-val').innerText = Math.round(ttsVolume * 100) + '%';

        function populateVoices() {
            if (!('speechSynthesis' in window)) return;
            voices = window.speechSynthesis.getVoices();
            const select = document.getElementById('voice-select'); select.innerHTML = '';
            let viVoices = voices.filter(v => v.lang.includes('vi') || v.lang.includes('VN'));
            (viVoices.length > 0 ? viVoices : voices).forEach((v) => {
                const opt = document.createElement('option'); opt.value = v.voiceURI; opt.textContent = `${v.name} (${v.lang})`;
                if (savedVoiceURI && v.voiceURI === savedVoiceURI) opt.selected = true;
                select.appendChild(opt);
            });
        }
        if ('speechSynthesis' in window) { window.speechSynthesis.onvoiceschanged = populateVoices; populateVoices(); }

        function selectVietnameseVoice(msg) {
            if (voices.length > 0) {
                const curURI = document.getElementById('voice-select').value;
                const v = voices.find(vox => vox.voiceURI === curURI) || voices.find(vox => vox.lang.includes('vi') || vox.lang.includes('VN'));
                if (v) msg.voice = v;
            }
            msg.lang = 'vi-VN';
        }

        // Đọc thông báo khi bệnh nhân ĐÃ CHỌN một câu (Ưu tiên cao nhất, tạm dừng bộ quét cho đến khi đọc xong)
        function speakActionFeedback(text, onDoneCallback = null) {
            stopScannerTimer();
            activeSpeechToken++;
            const myToken = activeSpeechToken;
            if (activeSpeechSafetyTimer) { clearTimeout(activeSpeechSafetyTimer); activeSpeechSafetyTimer = null; }

            if (!('speechSynthesis' in window) || !text || text.trim() === '') {
                isActionSpeechPlaying = false;
                if (onDoneCallback) onDoneCallback();
                return;
            }

            isActionSpeechPlaying = true;
            try { window.speechSynthesis.cancel(); } catch(e) {}

            setTimeout(() => {
                if (myToken !== activeSpeechToken) return;
                const msg = new SpeechSynthesisUtterance(text.trim());
                msg.rate = ttsRate;
                msg.volume = ttsVolume;
                selectVietnameseVoice(msg);

                let finished = false;
                const finish = () => {
                    if (finished) return;
                    finished = true;
                    isActionSpeechPlaying = false;
                    if (activeSpeechSafetyTimer) { clearTimeout(activeSpeechSafetyTimer); activeSpeechSafetyTimer = null; }
                    if (myToken === activeSpeechToken && onDoneCallback) onDoneCallback();
                };

                msg.onend = finish;
                msg.onerror = finish;
                const maxDuration = Math.max(1600, text.length * 120 + 900);
                activeSpeechSafetyTimer = setTimeout(finish, maxDuration);

                try { window.speechSynthesis.speak(msg); } catch(e) { finish(); }
            }, 35);
        }

        // Đọc tên mục đang quét: ĐỢI ĐỌC XONG 100% RỒI MỚI BẮT ĐẦU ĐẾM THỜI GIAN CHỜ PHẢN XẠ
        function speakScanPromptAndWait(text, onPromptFinished) {
            activeSpeechToken++;
            const myToken = activeSpeechToken;
            if (activeSpeechSafetyTimer) { clearTimeout(activeSpeechSafetyTimer); activeSpeechSafetyTimer = null; }

            if (!isAudioScanEnabled || !('speechSynthesis' in window) || !text || text.trim() === '') {
                onPromptFinished(false);
                return;
            }

            try { window.speechSynthesis.cancel(); } catch(e) {}

            setTimeout(() => {
                if (myToken !== activeSpeechToken || isActionSpeechPlaying) return;
                const cleanText = text.trim();
                const msg = new SpeechSynthesisUtterance(cleanText);
                // Đọc tên mục quét gọn và rõ
                msg.rate = Math.min(1.35, Math.max(0.95, ttsRate * 1.05));
                msg.volume = ttsVolume;
                selectVietnameseVoice(msg);

                let finished = false;
                const finish = () => {
                    if (finished) return;
                    finished = true;
                    if (activeSpeechSafetyTimer) { clearTimeout(activeSpeechSafetyTimer); activeSpeechSafetyTimer = null; }
                    if (myToken === activeSpeechToken && !isActionSpeechPlaying) {
                        onPromptFinished(true);
                    }
                };

                msg.onend = finish;
                msg.onerror = finish;
                // Bảo hiểm chống kẹt nếu trình duyệt không trả sự kiện onend
                const maxWait = Math.max(1300, cleanText.length * 105 + 650);
                activeSpeechSafetyTimer = setTimeout(finish, maxWait);

                try { window.speechSynthesis.speak(msg); } catch(e) { finish(); }
            }, 25);
        }

        function getAuditoryLabel(el) {
            if (!el) return "";
            const explicitVoice = el.getAttribute('data-voice');
            if (explicitVoice && explicitVoice.trim() !== '') return explicitVoice.trim();
            return el.innerText.trim().replace(/[^a-zA-ZÀ-ỹ0-9 ]/g, "").trim();
        }

        // =========================================================================
        // BỘ MÁY QUÉT ĐỒNG BỘ GIỌNG ĐỌC (SPEECH-SYNCHRONIZED SCANNER ENGINE)
        // - Khi Âm Quét BẬT: Đọc hết câu -> Giữ vệt sáng chờ phản xạ (holdAfterSpeechMs) -> Mới qua ô kế tiếp
        // - Khi Âm Quét TẮT: Chạy theo tốc độ chuẩn (silentStepMs)
        // =========================================================================
        const silentStepMs      = [1900, 1350, 950]; // Khi tắt âm quét
        const holdAfterSpeechMs = [1350, 950, 650];  // Thời gian giữ vệt sáng SAU KHI ĐÃ ĐỌC XONG CÂU
        const speedLabels       = ["Chậm", "Vừa", "Nhanh"];
        let speedIdx = 1;
        let scanTimer = null;

        let scanGroups = [];
        let flatScanItems = [];
        let currentGroupIdx = 0;
        let currentItemIdx = 0;
        let currentFlatIdx = 0;
        let scanState = 'GROUP';
        let loopCount = 0;

        function stopScannerTimer() {
            if (scanTimer) { clearTimeout(scanTimer); scanTimer = null; }
        }

        function toggleSpeed() {
            speedIdx = (speedIdx + 1) % 3;
            const newLabel = "Tốc Độ: " + speedLabels[speedIdx];
            document.getElementById('top-btn-speed').innerText = newLabel;
            document.getElementById('grid-btn-speed').innerText = newLabel.toUpperCase();
            speakActionFeedback(newLabel, () => {
                if (!isSleeping && !isSettingsOpen && !isControlLocked) startScanning();
            });
        }

        function buildScanTree() {
            let activeView = document.getElementById('view-menu');
            if (currentView === 'KEYBOARD') activeView = document.getElementById('view-keyboard');
            else if (currentView === 'READER') activeView = document.getElementById('view-reader');
            else if (currentView === 'QUICK') activeView = document.getElementById('view-quick');

            if (currentView === 'QUICK') {
                // ƯU TIÊN QUÉT 6 THẺ HÌNH ẢNH TRƯỚC, THANH ĐIỀU HƯỚNG QUÉT SAU CÙNG
                const gridGroups = Array.from(document.querySelectorAll('#quick-grid .scan-group'));
                const navGroup = document.getElementById('quick-nav-group');
                scanGroups = navGroup ? [...gridGroups, navGroup] : gridGroups;
            } else {
                scanGroups = Array.from(activeView.querySelectorAll('.scan-group'));
            }

            // Tạo danh sách phẳng các ô đang hiển thị phục vụ chế độ Quét Từng Ô (1 Nhịp)
            flatScanItems = [];
            scanGroups.forEach(grp => {
                const items = Array.from(grp.querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null && el.innerText.trim() !== '');
                flatScanItems.push(...items);
            });

            currentGroupIdx = 0;
            currentItemIdx = 0;
            currentFlatIdx = 0;
            scanState = 'GROUP';
            loopCount = 0;
            renderScan();
        }

        function renderScan() { 
            document.querySelectorAll('.scanning-row').forEach(el => el.classList.remove('scanning-row'));
            document.querySelectorAll('.scanning-cell').forEach(el => el.classList.remove('scanning-cell'));
            
            if (isControlLocked) return;

            if (isDirectItemScanActive()) {
                if (!flatScanItems || flatScanItems.length === 0) return;
                const activeEl = flatScanItems[currentFlatIdx];
                if (activeEl) {
                    activeEl.classList.add('scanning-cell');
                    const parentGrp = activeEl.closest('.scan-group');
                    if (parentGrp) parentGrp.classList.add('scanning-row');
                }
                return;
            }

            if (!scanGroups || scanGroups.length === 0) return;
            if (scanState === 'GROUP') {
                if (scanGroups[currentGroupIdx]) scanGroups[currentGroupIdx].classList.add('scanning-row');
            } else if (scanState === 'ITEM') {
                if (scanGroups[currentGroupIdx]) {
                    scanGroups[currentGroupIdx].classList.add('scanning-row');
                    let items = Array.from(scanGroups[currentGroupIdx].querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
                    if (items && items[currentItemIdx]) items[currentItemIdx].classList.add('scanning-cell');
                }
            }
        }

        function getCurrentScanVoicePrompt() {
            if (isDirectItemScanActive()) {
                if (!flatScanItems || flatScanItems.length === 0) return "";
                return getAuditoryLabel(flatScanItems[currentFlatIdx]);
            }

            if (!scanGroups || scanGroups.length === 0) return "";
            const grp = scanGroups[currentGroupIdx];
            if (!grp) return "";

            if (scanState === 'GROUP') {
                const explicitRowVoice = grp.getAttribute('data-row-voice');
                if (explicitRowVoice) return explicitRowVoice;
                const cards = grp.querySelectorAll('.aac-card');
                if (cards.length === 2) {
                    return `${cards[0].getAttribute('data-voice') || ""}, hoặc ${cards[1].getAttribute('data-voice') || ""}`;
                }
                const items = Array.from(grp.querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
                if (items.length === 1) return getAuditoryLabel(items[0]);
                return "";
            } else {
                const items = Array.from(grp.querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
                if (items && items[currentItemIdx]) return getAuditoryLabel(items[currentItemIdx]);
                return "";
            }
        }

        function advanceScanIndices() {
            if (isDirectItemScanActive()) {
                if (!flatScanItems || flatScanItems.length === 0) return;
                currentFlatIdx = (currentFlatIdx + 1) % flatScanItems.length;
                return;
            }

            if (!scanGroups || scanGroups.length === 0) return;
            if (scanState === 'GROUP') {
                currentGroupIdx = (currentGroupIdx + 1) % scanGroups.length;
            } else if (scanState === 'ITEM') {
                let items = Array.from(scanGroups[currentGroupIdx].querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
                if (!items || items.length === 0) {
                    scanState = 'GROUP';
                } else {
                    currentItemIdx++;
                    if (currentItemIdx >= items.length) {
                        loopCount++;
                        // Tự động thoát ra quét hàng tiếp theo sau 1 vòng nếu vào nhầm hàng để không làm người bệnh bị kẹt
                        if (loopCount >= 1) {
                            scanState = 'GROUP';
                            currentItemIdx = 0;
                            loopCount = 0;
                            currentGroupIdx = (currentGroupIdx + 1) % scanGroups.length;
                        } else {
                            currentItemIdx = 0;
                        }
                    }
                }
            }
        }

        function runCurrentScanStep() {
            stopScannerTimer();
            if (isSleeping || isSettingsOpen || isControlLocked || isFatigueLocked || isActionSpeechPlaying) return;

            renderScan();
            const promptText = getCurrentScanVoicePrompt();

            speakScanPromptAndWait(promptText, (didSpeak) => {
                if (isSleeping || isSettingsOpen || isControlLocked || isFatigueLocked || isActionSpeechPlaying) return;
                const waitMs = didSpeak ? holdAfterSpeechMs[speedIdx] : silentStepMs[speedIdx];
                scanTimer = setTimeout(() => {
                    advanceScanIndices();
                    runCurrentScanStep();
                }, waitMs);
            });
        }

        function startScanning() { 
            stopScannerTimer();
            if (isControlLocked || isSleeping || isSettingsOpen || isFatigueLocked) return;
            runCurrentScanStep();
        }

        function executeNormalSelection() {
            if (isControlLocked || isSleeping || isSettingsOpen || isFatigueLocked) return;

            // CHẾ ĐỘ 1: QUÉT TỪNG Ô TRỰC TIẾP (1 NHỊP CHỌN NGAY - KHÔNG QUA HÀNG)
            if (isDirectItemScanActive()) {
                if (!flatScanItems || flatScanItems.length === 0) return;
                const targetBtn = flatScanItems[currentFlatIdx];
                if (targetBtn) {
                    stopScannerTimer();
                    targetBtn.click();
                }
                return;
            }

            // CHẾ ĐỘ 2: QUÉT HÀNG - CỘT (CÓ CHỐNG TRƯỢT 420MS KHI VỪA VÀO HÀNG)
            if (!scanGroups || scanGroups.length === 0) return;

            if (scanState === 'GROUP') {
                let items = Array.from(scanGroups[currentGroupIdx].querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
                if (items && items.length > 0) {
                    if (items.length === 1) {
                        stopScannerTimer();
                        items[0].click();
                    } else {
                        scanState = 'ITEM'; 
                        currentItemIdx = 0; 
                        loopCount = 0;
                        lastRowEnterTimestamp = Date.now();
                        runCurrentScanStep();
                    }
                }
            } 
            else if (scanState === 'ITEM') {
                // Chống nháy đúp vô tình ngay lúc vừa vào hàng (< 420ms)
                if (Date.now() - lastRowEnterTimestamp < 420) return;

                let items = Array.from(scanGroups[currentGroupIdx].querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
                if (items && items[currentItemIdx]) {
                    stopScannerTimer();
                    let btn = items[currentItemIdx];
                    scanState = 'GROUP';
                    currentGroupIdx = 0;
                    loopCount = 0;
                    btn.click();
                }
            }
        }

        function sendQuickAlert(msg, onSpeechDone = null) {
            if (msg === "SOS") {
                sendCommand("SOS");
                showToastAlert("SOS");
                speakActionFeedback("CẤP CỨU! CẦN BÁC SĨ GIÚP ĐỠ GẤP!", onSpeechDone);
            } else {
                sendCommand("SENTENCE:" + msg);
                showToastAlert(msg);
                speakActionFeedback(msg, onSpeechDone);
            }
        }

        function showToastAlert(msg) {
            const toast = document.getElementById('toast-alert');
            toast.innerText = msg === "SOS" ? "CẤP CỨU KHẨN CẤP" : msg;
            toast.style.background = msg === "SOS" ? "var(--danger)" : "var(--primary)";
            toast.style.display = 'block';
            setTimeout(() => { toast.style.display = 'none'; }, 3500);
        }

        const dictionary = ["TÔI", "BẠN", "MẸ", "CHA", "BÁC SĨ", "Y TÁ", "CON", "CHỒNG", "VỢ", "NHÀ", "BỆNH VIỆN", "CẦN", "MUỐN", "THÍCH", "LÀM", "ĂN", "UỐNG", "ĐI", "NGỦ", "TẮM", "KHÁM", "MỞ", "BẬT", "TẮT", "LẤY", "GIÚP", "NGỒI", "NẰM", "NGHIÊNG", "XOA BÓP", "MỆT", "ĐAU", "NHỨC", "MỎI", "NGỨA", "HO", "THỞ", "KHÁT", "ĐÓI", "NÓNG", "LẠNH", "KHỎE", "YẾU", "CHÓNG MẶT", "BUỒN NÔN", "NƯỚC", "CƠM", "MÌ", "PHỞ", "CHÁO", "SỮA", "CAM", "THUỐC", "ÁO", "QUẦN", "GIƯỜNG", "ĐÈN", "QUẠT", "TIVI", "NHẠC", "CHĂN", "GỐI", "ĐẦU", "BỤNG", "TAY", "CHÂN", "LƯNG", "NGỰC", "MẮT", "CỔ", "MIỆNG", "VẾT MỔ", "HUYẾT ÁP", "ĐỜM", "ĐÚNG", "SAI", "RẤT", "QUÁ", "KHÔNG", "CÓ", "CHƯA", "RỒI", "ĐANG", "SẼ", "XIN LỖI", "CẢM ƠN", "VÂNG", "DẠ", "VỆ SINH", "LAU NGƯỜI"];
        const bigrams = {
            "TÔI": ["CẦN", "MUỐN", "ĐAU", "MỆT", "KHÁT", "ĐÓI", "LẠNH", "NÓNG", "KHỎE"],
            "CẦN": ["UỐNG", "ĂN", "ĐI", "GIÚP", "BÁC SĨ", "Y TÁ", "THUỐC", "VỆ SINH", "HÚT ĐỜM"],
            "MUỐN": ["UỐNG", "ĂN", "NẰM", "NGỒI", "NGỦ", "VỆ SINH", "NGHIÊNG", "XOA BÓP"],
            "ĐAU": ["ĐẦU", "NGỰC", "BỤNG", "LƯNG", "CỔ", "TAY", "CHÂN", "VẾT MỔ"],
            "UỐNG": ["NƯỚC", "SỮA", "THUỐC", "CAM"],
            "ĂN": ["CHÁO", "CƠM", "PHỞ", "MÌ", "CAM"],
            "ĐI": ["VỆ SINH", "NGỦ", "KHÁM", "TẮM"],
            "BẬT": ["ĐÈN", "QUẠT", "TIVI", "NHẠC"],
            "TẮT": ["ĐÈN", "QUẠT", "TIVI", "NHẠC"],
            "BÁC SĨ": ["CẤP CỨU", "KHÁM", "GIÚP", "ƠI"],
            "LẤY": ["NƯỚC", "THUỐC", "CHĂN", "GỐI", "ÁO"],
            "CẢM": ["ƠN", "THẤY", "LẠNH", "NÓNG"],
            "XIN": ["THUỐC", "NƯỚC", "CẢM ƠN", "LỖI"],
            "RẤT": ["ĐAU", "MỆT", "KHÁT", "LẠNH", "NÓNG", "KHỎE"],
            "KHÔNG": ["ĐAU", "MỆT", "CẦN", "MUỐN", "PHẢI", "ĐƯỢC"],
            "MỎI": ["LƯNG", "CỔ", "TAY", "CHÂN", "MẮT"],
            "KHÓ": ["THỞ", "CHỊU", "NGỦ", "NUỐT"]
        };

        function removeAccents(str) { return str.normalize("NFD").replace(/[\u0300-\u036f]/g, "").replace(/Đ/g, "D").replace(/đ/g, "d"); }
        function processTelex(text) {
            let words = text.split(' '); let lastWord = words[words.length - 1]; if (!lastWord) return text;
            lastWord = lastWord.replace(/AW/gi, 'Ă').replace(/AA/gi, 'Â').replace(/EE/gi, 'Ê').replace(/OO/gi, 'Ô').replace(/OW/gi, 'Ơ').replace(/UW/gi, 'Ư').replace(/DD/gi, 'Đ');
            const tones = {'S':'\u0301', 'F':'\u0300', 'R':'\u0309', 'X':'\u0303', 'J':'\u0323'}; let lastChar = lastWord.slice(-1).toUpperCase();
            if (tones[lastChar]) {
                let baseWord = lastWord.slice(0, -1).normalize('NFD').replace(/[\u0300-\u0303\u0309\u0323]/g, '');
                let vowels = "AĂÂEÊIOÔƠUƯY"; let targetIdx = -1;
                for (let i = baseWord.length - 1; i >= 0; i--) {
                    if (vowels.includes(baseWord[i].normalize('NFD')[0].toUpperCase())) {
                        targetIdx = i; if (i > 0 && vowels.includes(baseWord[i-1].normalize('NFD')[0].toUpperCase())) {
                            targetIdx = i - 1; let prevPrev = i > 1 ? baseWord[i-2].toUpperCase() : '';
                            if ((prevPrev === 'Q' && baseWord[i-1].toUpperCase() === 'U') || (prevPrev === 'G' && baseWord[i-1].toUpperCase() === 'I')) targetIdx = i;
                        } break;
                    }
                }
                if (targetIdx !== -1) { let accentedChar = (baseWord[targetIdx].normalize('NFD') + tones[lastChar]).normalize('NFC'); lastWord = baseWord.substring(0, targetIdx) + accentedChar + baseWord.substring(targetIdx + 1); }
                else lastWord = baseWord + lastChar; 
            }
            words[words.length - 1] = lastWord; return words.join(' ');
        }

        function updatePredictiveText() {
            const outText = document.getElementById('output-box').textContent;
            const words = outText.trim().split(/\s+/);
            const currentPrefix = outText.endsWith(' ') ? '' : (words[words.length - 1] || '').toUpperCase();
            let lastCompleteWord = outText.endsWith(' ') ? (words[words.length - 1] || '').toUpperCase() : (words.length >= 2 ? words[words.length - 2].toUpperCase() : '');
            let matches = [];
            if (currentPrefix === '') { matches = (lastCompleteWord && bigrams[lastCompleteWord]) ? [...bigrams[lastCompleteWord]] : ["TÔI", "CẦN", "UỐNG", "ĐAU"]; } 
            else {
                const cleanPrefix = removeAccents(currentPrefix);
                if (lastCompleteWord && bigrams[lastCompleteWord]) { matches.push(...bigrams[lastCompleteWord].filter(w => removeAccents(w).startsWith(cleanPrefix) && w !== currentPrefix)); }
                dictionary.filter(w => removeAccents(w).startsWith(cleanPrefix) && w !== currentPrefix).forEach(w => { if (!matches.includes(w)) matches.push(w); });
            }
            if (matches.length < 4) dictionary.forEach(w => { if (!matches.includes(w) && w !== currentPrefix && matches.length < 4) matches.push(w); });
            for(let i=1; i<=4; i++) {
                const sugEl = document.getElementById('sug'+i);
                sugEl.innerText = matches[i-1] || '';
                sugEl.setAttribute('data-voice', matches[i-1] ? `${matches[i-1]}` : '');
            }
        }

        function applySuggestion(word) {
            const outBox = document.getElementById('output-box'); let text = outBox.textContent;
            if (text === '' || text.endsWith(' ')) outBox.textContent = text + word + ' ';
            else { let words = text.trim().split(/\s+/); words.pop(); words.push(word); outBox.textContent = words.join(' ') + ' '; }
            updatePredictiveText();
        }

        function resumeScanningAfterAction() {
            scanState = 'GROUP';
            currentGroupIdx = 0;
            currentItemIdx = 0;
            loopCount = 0;
            buildScanTree();
            if (!isSleeping && !isSettingsOpen && !isControlLocked) startScanning();
        }

        function executeAction(element) {
            stopScannerTimer();
            const textRaw = element.innerText.trim();
            const text = textRaw.replace(/[^a-zA-ZÀ-ỹ0-9 ]/g, "").trim(); 
            if (textRaw === '') return; 
            const outBox = document.getElementById('output-box');

            if (element.classList.contains('word-suggest')) {
                applySuggestion(text);
                speakActionFeedback(text, resumeScanningAfterAction);
                return;
            }
            else if (element.classList.contains('quick-cat')) { 
                let target = element.getAttribute('data-target');
                let voiceLabel = element.getAttribute('data-voice') || text;
                renderQuickGrid(target);
                return;
            }
            else if (element.classList.contains('quick-phrase')) { 
                let phrase = element.getAttribute('data-phrase') || text;
                let speakOutText = element.getAttribute('data-speakout') || element.getAttribute('data-voice') || phrase;
                let isSOS = element.getAttribute('data-sos') === '1';
                let sysCmd = element.getAttribute('data-syscmd') || '';

                if (sysCmd === 'R1') {
                    toggleRelay(1);
                    showToastAlert("Đã bật/tắt Đèn phòng");
                    speakActionFeedback(speakOutText, () => { renderQuickGrid('main'); });
                } else if (sysCmd === 'R2') {
                    toggleRelay(2);
                    showToastAlert("Đã bật/tắt Quạt máy");
                    speakActionFeedback(speakOutText, () => { renderQuickGrid('main'); });
                } else if (sysCmd === 'OPEN_READER') {
                    showReader();
                } else if (sysCmd === 'LOCK_NOW') {
                    sendCommand('TOGGLE_CONTROL_LOCK');
                } else if (isSOS) {
                    sendQuickAlert("SOS", () => { renderQuickGrid('main'); });
                } else {
                    sendCommand("SENTENCE:" + phrase);
                    showToastAlert(phrase);
                    speakActionFeedback(speakOutText, () => { renderQuickGrid('main'); });
                }
                return;
            }
            else if (element.classList.contains('char')) {
                outBox.textContent += text;
                outBox.textContent = processTelex(outBox.textContent);
                updatePredictiveText();
                speakActionFeedback(text, resumeScanningAfterAction);
                return;
            }
            else if (text === "CÁCH") {
                if (!outBox.textContent.endsWith(' ') && outBox.textContent !== '') {
                    outBox.textContent += " ";
                    updatePredictiveText();
                }
                speakActionFeedback("Dấu cách", resumeScanningAfterAction);
                return;
            }
            else if (text === "XÓA") {
                outBox.textContent = outBox.textContent.slice(0, -1);
                updatePredictiveText();
                speakActionFeedback("Xóa", resumeScanningAfterAction);
                return;
            }
            else if (text === "XÓA HẾT") {
                outBox.textContent = "";
                updatePredictiveText();
                speakActionFeedback("Xóa hết", resumeScanningAfterAction);
                return;
            }
            else if (text === "ĐỌC CÂU") {
                let phrase = outBox.textContent.trim();
                if (phrase !== "") {
                    sendCommand("SENTENCE:" + phrase);
                    showToastAlert(phrase);
                    speakActionFeedback(phrase, resumeScanningAfterAction);
                } else {
                    speakActionFeedback("Chưa có nội dung", resumeScanningAfterAction);
                }
                return;
            }
            else if (text.startsWith("TỐC ĐỘ")) { toggleSpeed(); return; }
            else if (text.includes("ĐÈN")) {
                toggleRelay(1);
                speakActionFeedback("Đèn phòng", resumeScanningAfterAction);
                return;
            }
            else if (text.includes("QUẠT")) {
                toggleRelay(2);
                speakActionFeedback("Quạt máy", resumeScanningAfterAction);
                return;
            }
            else if (text === "NGỦ") { enterSleepMode(); sendCommand('S'); return; }
            else if (text === "CẤP CỨU") {
                sendQuickAlert("SOS", resumeScanningAfterAction);
                return;
            }

            resumeScanningAfterAction();
        }

        document.querySelectorAll('button, .cell, .btn-sub, .top-btn').forEach(btn => {
            btn.addEventListener('click', (e) => { 
                e.preventDefault(); e.stopPropagation(); 
                if(btn.classList.contains('cell')) executeAction(btn); 
                if (document.activeElement) document.activeElement.blur(); 
            });
        });

        function toggleRelay(num) { sendCommand('R' + num); }
        function updateRelayUI(num, state) {
            let label = state ? "Bật" : "Tắt";
            let actionLabel = state ? "TẮT" : "BẬT";
            let color = state ? "var(--success)" : "var(--text-muted)";
            let mEl = document.getElementById('menu-r' + num);
            if (mEl) { mEl.innerText = label; mEl.style.color = color; }
            let kEl = document.getElementById('grid-btn-r' + num);
            if (kEl) {
                let name = num === 1 ? "ĐÈN" : "QUẠT";
                kEl.innerText = `${actionLabel} ${name}`;
                kEl.style.backgroundColor = state ? "rgba(245, 158, 11, 0.18)" : "var(--key-sys-bg)"; 
                kEl.style.color = state ? "var(--warning)" : "var(--text-main)";
                kEl.style.borderColor = state ? "var(--warning)" : "var(--key-border)";
            }
        }

        function enterSleepMode() { 
            isSleeping = true; 
            stopScannerTimer();
            document.getElementById('sleep-overlay').style.display = 'flex'; 
            if (isAutoScrolling) toggleAutoScroll();
            speakActionFeedback("Chế độ nghỉ thư giãn."); 
        }
        function exitSleepMode() { 
            isSleeping = false; 
            document.getElementById('sleep-overlay').style.display = 'none'; 
            speakActionFeedback("Đã thức dậy", () => {
                if (!isControlLocked) startScanning();
            });
        }

        function openSettings() { isSettingsOpen = true; stopScannerTimer(); document.getElementById('settings-overlay').style.display = 'flex'; }
        function closeSettings() {
            isSettingsOpen = false;
            ttsRate = parseFloat(document.getElementById('rate-slider').value);
            ttsVolume = parseFloat(document.getElementById('vol-slider').value);
            savedVoiceURI = document.getElementById('voice-select').value;
            localStorage.setItem('ttsRate', ttsRate);
            localStorage.setItem('ttsVolume', ttsVolume);
            localStorage.setItem('ttsVoiceURI', savedVoiceURI);
            document.getElementById('settings-overlay').style.display = 'none';
            if (!isSleeping && !isControlLocked) startScanning();
        }
        function testVoice() {
            let curR = ttsRate, curV = ttsVolume;
            ttsRate = parseFloat(document.getElementById('rate-slider').value);
            ttsVolume = parseFloat(document.getElementById('vol-slider').value);
            speakActionFeedback("Hệ thống đọc rõ từng mục và chờ bạn chọn!", () => {
                ttsRate = curR; ttsVolume = curV;
            });
        }

        document.getElementById('rate-slider').oninput = function() { document.getElementById('rate-val').innerText = parseFloat(this.value).toFixed(2); };
        document.getElementById('vol-slider').oninput = function() { document.getElementById('vol-val').innerText = Math.round(this.value * 100) + '%'; };

        window.addEventListener('keydown', (e) => { 
            if (e.code === 'Space') { 
                e.preventDefault(); e.stopPropagation();
                if (document.activeElement) document.activeElement.blur(); 
                sendCommand('SIMULATE_BLINK'); 
            } 
            else if (e.code === 'KeyI') {
                e.preventDefault();
                toggleHybridTrack();
            }
            else if (e.code === 'KeyL') {
                e.preventDefault();
                sendCommand('TOGGLE_CONTROL_LOCK');
            }
            else if (e.code === 'KeyM') {
                e.preventDefault();
                cycleNodMode();
            }
            else if (e.code === 'KeyF') {
                e.preventDefault();
                sendCommand('SIMULATE_FATIGUE');
            }
        }, true);

        updatePredictiveText();
        
        document.addEventListener('DOMContentLoaded', () => {
            buildScanTree();
            startScanning();
        });
    </script>
</body>
</html>
)rawliteral";
// =========================================================================
// TASK FREERTOS: LED RGB WS2812 CẦU VỒNG ONBOARD (CHẠY RIÊNG LẺ TRÊN CORE 0)
// Tự động bật ngay khi cắm nguồn điện và chuyển đổi màu cầu vồng liên tục
// =========================================================================
// =========================================================================
// TASK FREERTOS: LED RGB WS2812 CẦU VỒNG ONBOARD (CHẠY MƯỢT 100% VỚI WIFI)
// Sử dụng hàm phần cứng neopixelWrite() gốc của ESP32-S3, không bao giờ treo RMT
// =========================================================================
void hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b) {
  uint8_t region = h / 43;
  uint8_t remainder = (h - (region * 43)) * 6;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
    case 0:  r = v; g = t; b = p; break;
    case 1:  r = q; g = v; b = p; break;
    case 2:  r = p; g = v; b = t; break;
    case 3:  r = p; g = q; b = v; break;
    case 4:  r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }
}

void rgbRainbowTask(void * parameter) {
  uint8_t hue = 0;
  uint8_t r, g, b;

  for (;;) {
    // Chuyển đổi góc màu hue (0..255) sang RGB với độ sáng RGB_BRIGHTNESS_LEVEL (45/255)
    hsvToRgb(hue, 255, RGB_BRIGHTNESS_LEVEL, r, g, b);

    // Xuất trực tiếp ra LED WS2812 bằng driver RMT gốc của lõi ESP32-S3
    neopixelWrite(RGB_LED_PIN, r, g, b);

    hue++; // Tăng dần dải màu cầu vồng liên tục (0 -> 255 rồi tự quay về 0)
    vTaskDelay(pdMS_TO_TICKS(20)); // Tốc độ chuyển màu mượt (20ms/bước ~ 5 giây/vòng)
  }
}

// =========================================================================
// TASK FREERTOS: CLOUD SYNC TASK (CORE 0 - ĐỒNG BỘ ĐẦY ĐỦ CFI, NHIỆT ĐỘ, 3D PITCH/ROLL)
// =========================================================================
void cloudSyncTask(void * parameter) {
  unsigned long lastCommandCheck = 0;

  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(100));

    if (pendingCloudCommand != "") {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient cmdPost;
        cmdPost.begin(secureClient, CLOUD_COMMAND_URL);
        cmdPost.addHeader("Content-Type", "application/json");
        cmdPost.setTimeout(2500);
        char cmdJson[80];
        snprintf(cmdJson, sizeof(cmdJson), "{\"command\":\"%s\"}", pendingCloudCommand.c_str());
        cmdPost.POST(cmdJson);
        cmdPost.end();
      }
      pendingCloudCommand = "";
    }

    if (WiFi.status() == WL_CONNECTED && (millis() - lastCommandCheck > 3000)) {
      lastCommandCheck = millis();
      HTTPClient cmdHttp;
      cmdHttp.begin(secureClient, CLOUD_COMMAND_URL);
      cmdHttp.setTimeout(2000);
      int httpCode = cmdHttp.GET();
      if (httpCode == 200) {
        String resp = cmdHttp.getString();
        resp.trim();
        cloudCommandOk = true;
        cloudCommandFailCount = 0;
        if (resp.indexOf("UNLOCK_REST") >= 0) {
          if (isFatigueLocked) {
            releaseLocalFatigueLock();
            pendingCloudCommand = "IDLE";
          }
        } 
        else if (resp.indexOf("LOCK_REST") >= 0 && resp.indexOf("UNLOCK_REST") == -1) {
          if (!isFatigueLocked && millis() > ignoreLockUntil) {
            triggerLocalFatigueLock(1800, "GLOBAL");
            pendingCloudCommand = "IDLE";
          }
        }
      } else {
        cloudCommandFailCount++;
        if (cloudCommandFailCount >= 2) cloudCommandOk = false;
      }
      cmdHttp.end();
    }

    if (pendingCloudSentence != "") {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(secureClient, CLOUD_SERVER_URL);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(2500); 

        String jsonPayload = "{\"sentence\":\"" + pendingCloudSentence + "\",\"attention\":" + String(attention) + ",\"meditation\":" + String(meditation) + ",\"signalQuality\":" + String(signalQuality) + "}";
        http.POST(jsonPayload);
        http.end();
      }
      pendingCloudSentence = ""; 
    }

    if (pendingTelemetry) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(secureClient, CLOUD_TELEMETRY_URL);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(2500);

        unsigned long fatigueSecLeft = 0;
        if (isFatigueLocked) {
          unsigned long elapsed = millis() - fatigueLockStartTime;
          if (elapsed < fatigueLockDurationMs) {
            fatigueSecLeft = (fatigueLockDurationMs - elapsed) / 1000;
          }
        }

        String payload = "{\"isSleeping\":" + String(isSleeping ? "true" : "false") + "," +
                         "\"isControlLocked\":" + String(isControlLocked ? "true" : "false") + "," +
                         "\"signalQuality\":" + String(signalQuality) + "," +
                         "\"isFatigueLocked\":" + String(isFatigueLocked ? "true" : "false") + "," +
                         "\"fatigueSecLeft\":" + String(fatigueSecLeft) + "," +
                         "\"cfi\":" + String((int)cfiScore) + "," +
                         "\"temperature\":" + String(sensorTempC, 1) + "," +
                         "\"pitch\":" + String(compPitchAngle, 1) + "," +
                         "\"roll\":" + String(compRollAngle, 1) + "," +
                         "\"battery\":" + String(batteryPercent) + "," +
                         "\"voltage\":" + String(inaBusVoltage, 2) + "," +
                         "\"current\":" + String(inaCurrent_mA, 1) + "," +
                         "\"power\":" + String(inaPower_mW, 1) + "," +
                         "\"shuntVoltage\":" + String(inaShunt_mV, 2) + "," +
                         "\"attention\":" + String(attention) + "," +
                         "\"meditation\":" + String(meditation) + "," +
                         "\"delta\":" + String(delta) + "," +
                         "\"theta\":" + String(theta) + "," +
                         "\"lowAlpha\":" + String(lowAlpha) + "," +
                         "\"lowBeta\":" + String(lowBeta) + "," +
                         "\"lowGamma\":" + String(lowGamma) + "," +
                         "\"uptime\":" + String(millis() / 1000) + "}";

        http.POST(payload);
        http.end();
      }
      pendingTelemetry = false;
    }
  }
}

// =========================================================================
// XỬ LÝ SỰ KIỆN WEBSOCKET
// =========================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Client #%u Disconnected\n", num);
      break;

    case WStype_CONNECTED: {
      if (isFatigueLocked) {
        unsigned long secLeft = (fatigueLockDurationMs - (millis() - fatigueLockStartTime)) / 1000;
        webSocket.sendTXT(num, "FATIGUE_LOCK:" + String(secLeft) + "," + fatigueLockSource);
      }
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[WS] Client #%u Connected from %s\n", num, ip.toString().c_str());
      
      String rState = "RELAY:" + String(relay1State ? "1" : "0") + "," + String(relay2State ? "1" : "0");
      webSocket.sendTXT(num, rState);

      String sensorData = "TGAM:" + String(signalQuality) + "," + String(attention) + "," +
                                    String(meditation) + "," + String((int)cfiScore) + "," + String(sensorTempC, 1);
      webSocket.sendTXT(num, sensorData);

      String inaMsg = "INA:" + String(inaBusVoltage, 2) + "," +
                               String(inaCurrent_mA, 1) + "," +
                               String(inaPower_mW, 1) + "," +
                               String(batteryPercent) + "," +
                               String(hasINA226 ? 1 : 0);
      webSocket.sendTXT(num, inaMsg);

      String modeStr = (currentNodMode == NOD_MODE_LIGHT) ? "LIGHT" :
                       (currentNodMode == NOD_MODE_HEAVY) ? "HEAVY" : "DUAL";
      webSocket.sendTXT(num, "MODE:" + modeStr);

      String inModeStr = (currentInputMode == INPUT_IR_ONLY) ? "IR_ONLY" :
                         (currentInputMode == INPUT_NOD_ONLY) ? "NOD_ONLY" : "HYBRID";
      webSocket.sendTXT(num, "INPUT_MODE:" + inModeStr);

      webSocket.sendTXT(num, isControlLocked ? "LOCK_CONTROL:ON" : "LOCK_CONTROL:OFF");
      
      if (isSleeping) {
        webSocket.sendTXT(num, "SLEEPING");
      }
      break;
    }

    case WStype_TEXT: {
      String msg = String((char*)payload);
      lastActivityTime = millis();

      if (msg == "S") {
        isSleeping = true;
        WiFi.setSleep(true); 
        webSocket.broadcastTXT("SLEEPING");
        syncTelemetryToCloud();
      } 
      else if (msg == "W") {
        isSleeping = false;
        WiFi.setSleep(false);
        webSocket.broadcastTXT("WAKEUP");
        syncTelemetryToCloud();
      } 
      else if (msg == "TOGGLE_CONTROL_LOCK") {
        setControlLock(!isControlLocked);
      }
      else if (msg == "LOCK_CONTROL_ON") {
        setControlLock(true);
      }
      else if (msg == "LOCK_CONTROL_OFF") {
        setControlLock(false);
      }
      else if (msg == "MODE_LIGHT") {
        setNodMode(NOD_MODE_LIGHT);
      }
      else if (msg == "MODE_HEAVY") {
        setNodMode(NOD_MODE_HEAVY);
      }
      else if (msg == "MODE_DUAL") {
        setNodMode(NOD_MODE_DUAL);
      }
      else if (msg == "INPUT_HYBRID") {
        setInputMode(INPUT_HYBRID_BOTH);
      }
      else if (msg == "INPUT_IR") {
        setInputMode(INPUT_IR_ONLY);
      }
      else if (msg == "INPUT_NOD") {
        setInputMode(INPUT_NOD_ONLY);
      }
      else if (msg == "R1") {
        relay1State = !relay1State;
        digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
        String rState = "RELAY:" + String(relay1State ? "1" : "0") + "," + String(relay2State ? "1" : "0");
        webSocket.broadcastTXT(rState);
      } 
      else if (msg == "R2") {
        relay2State = !relay2State;
        digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
        String rState = "RELAY:" + String(relay1State ? "1" : "0") + "," + String(relay2State ? "1" : "0");
        webSocket.broadcastTXT(rState);
      } 
      else if (msg.startsWith("SENTENCE:")) {
        String sentence = msg.substring(9);
        Serial.println("[SENTENCE] Bệnh nhân giao tiếp: " + sentence);
        syncSentenceToCloud(sentence);
      }
      else if (msg == "SOS") {
        triggerBuzzer(3); // Báo động SOS khẩn cấp 3 hồi dài
        Serial.println("[ALERT] BỆNH NHÂN GỌI CẤP CỨU (SOS)!");
        syncSentenceToCloud("SOS - CẤP CỨU KHẨN CẤP");
      }
      else if (msg == "SIMULATE_BLINK") {
        bool simHeavy = isSleeping || (currentNodMode == NOD_MODE_HEAVY) || (currentNodMode == NOD_MODE_DUAL && random(0, 2) == 1);
        fireInstantNodEvent(simHeavy, simHeavy ? 5.2f : 1.7f, simHeavy ? 44.0f : 19.5f, simHeavy ? 120.0f : 28.0f, 30);
      }
      else if (msg == "SIMULATE_FATIGUE") {
        cfiScore = 82.0f;
        triggerLocalFatigueLock(1800, "CFI_OVER_75%");
      }
      else if (msg == "UNLOCK_FATIGUE") {
        releaseLocalFatigueLock();
        pendingCloudCommand = "IDLE";
        syncTelemetryToCloud();
      }
      break;
    }
    default:
      break;
  }
}

void handleRoot() {
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", INDEX_HTML);
}

// =========================================================================
// SETUP
// =========================================================================
void setup() {
  // 1. KHỞI ĐỘNG NGAY LẬP TỨC TASK LED RGB CẦU VỒNG LIÊN TỤC
  xTaskCreatePinnedToCore(
    rgbRainbowTask,
    "RGBRainbowTask",
    2048,
    NULL,
    1,
    &rgbRainbowTaskHandle,
    1 // Ghim sang Core 1 (cùng Core với loop nhưng chạy ngầm bằng vTaskDelay, tránh xung đột ngắt Wi-Fi ở Core 0)
  );

  Serial.begin(115200);
  delay(1200);
  // ... (Giữ nguyên toàn bộ phần code bên dưới của setup)



  Serial.println("\n\n==========================================================");
  Serial.println("  ESP32-S3 HMI - CHỈ GẬT ĐẦU + FULL GIÁM SÁT Y SINH      ");
  Serial.println("  LED RGB CẦU VỒNG: HOẠT ĐỘNG RIÊNG LẺ TRÊN CORE 0 (GPIO 48)");
  Serial.println("==========================================================");

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(IR_LED_PULSE_PIN, OUTPUT);
  digitalWrite(IR_LED_PULSE_PIN, LOW);

  pinMode(IR_SENSOR_ADC_PIN, INPUT);

  triggerBuzzer(2); // Kêu 2 bíp khởi động hệ thống

  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);
  Wire.setTimeOut(25);
  delay(20);

  initLSM6DS3();
  initINA226();
  readINA226();

  secureClient.setInsecure(); 

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.softAP(AP_SSID, AP_PASS);

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);
  wifiMulti.addAP(WIFI_SSID_3, WIFI_PASS_3);

  Serial.println("[WIFI] Đang tìm kiếm và kết nối Wi-Fi Router...");
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WIFI] Kết nối Router thành công!");
    Serial.print("[IP ROUTER] http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WIFI] Chạy trên SoftAP cục bộ:");
    Serial.print("[IP AP] http://");
    Serial.println(WiFi.softAPIP());
  }

  if (MDNS.begin("esp32-bci")) {
    Serial.println("[MDNS] Truy cập tên miền: http://esp32-bci.local");
  }

  server.on("/", HTTP_GET, handleRoot);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(15000, 10000, 2);

  xTaskCreatePinnedToCore(
    cloudSyncTask,   
    "CloudTask",     
    8192,            
    NULL,            
    1,               
    NULL,            
    0                
  );

  lastActivityTime = millis();
  lastTelemetrySyncTime = millis();
  Serial.println("[SYSTEM] Sẵn sàng! Gật 1 cái = Chọn | Cúi giữ 1.2s = Khóa | Gật 3 cái khi khóa = Mở | Full CFI & Spasm Alert!");
}

// =========================================================================
// LOOP
// =========================================================================
void loop() {
  webSocket.loop();
  server.handleClient();
  
  // Đọc và xử lý tín hiệu LSM6DS3 ở tần số chuẩn 200Hz
  processLSM6DS3();

  // Đọc vi sai và xử lý tín hiệu TCRT5000 ở tần số 200Hz
  processTCRT5000();

  // Cập nhật trạng thái còi phản hồi xúc giác âm thanh (non-blocking)
  updateBuzzer();

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') setNodMode(NOD_MODE_LIGHT);
    else if (c == '2') setNodMode(NOD_MODE_HEAVY);
    else if (c == '3') setNodMode(NOD_MODE_DUAL);
    else if (c == 'l' || c == 'L') setControlLock(!isControlLocked);
    else if (c == 'i' || c == 'I') setInputMode(INPUT_IR_ONLY);
    else if (c == 'n' || c == 'N') setInputMode(INPUT_NOD_ONLY);
    else if (c == 'h' || c == 'H') setInputMode(INPUT_HYBRID_BOTH);
    else if (c == '4') triggerBuzzer(1); // Gõ phím 4: Test 1 tiếng bíp ngắn (Khi chọn ô)
    else if (c == '5') triggerBuzzer(2); // Gõ phím 5: Test 2 bíp (Khi Mở khóa / Đánh thức)
    else if (c == '6') triggerBuzzer(3); // Gõ phím 6: Test 3 hồi còi dài (Báo động SOS / Co giật)
  }

  // Đếm ngược khóa nghỉ 30 phút
  if (isFatigueLocked) {
    unsigned long elapsed = millis() - fatigueLockStartTime;
    if (elapsed >= fatigueLockDurationMs) {
      releaseLocalFatigueLock();
    } else {
      static unsigned long lastFatigueTick = 0;
      if (millis() - lastFatigueTick > 1000) {
        lastFatigueTick = millis();
        unsigned long secLeft = (fatigueLockDurationMs - elapsed) / 1000;
        webSocket.broadcastTXT("FATIGUE_LOCK:" + String(secLeft) + "," + fatigueLockSource);
      }
    }
  }

  // Đọc INA226 và phát dữ liệu đầy đủ (kèm CFI và Nhiệt độ LSM6DS3) lên Web mỗi giây
  static unsigned long lastSensorBroadcast = 0;
  if (millis() - lastSensorBroadcast > 1000) {
    lastSensorBroadcast = millis();
    readINA226();

    if (!isSleeping && webSocket.connectedClients() > 0) {
      String sensorData = "TGAM:" + String(signalQuality) + "," + 
                                    String(attention) + "," + 
                                    String(meditation) + "," +
                                    String((int)cfiScore) + "," +
                                    String(sensorTempC, 1);
      webSocket.broadcastTXT(sensorData);

      String inaData = "INA:" + String(inaBusVoltage, 2) + "," + 
                                String(inaCurrent_mA, 1) + "," + 
                                String(inaPower_mW, 1) + "," + 
                                String(batteryPercent) + "," +
                                String(hasINA226 ? 1 : 0);
      webSocket.broadcastTXT(inaData);
    }
  }

  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) { 
      wifiMulti.run();
    }
  }

  // Tự động ngủ sau 15 phút (900.000 ms) không có hoạt động
  if (!isSleeping && (millis() - lastActivityTime >= AUTO_SLEEP_TIMEOUT)) {
    isSleeping = true;
    WiFi.setSleep(true);
    webSocket.broadcastTXT("SLEEPING");
    syncTelemetryToCloud();
    Serial.println(">>> [SMART SLEEP] Không có hoạt động trong 15 phút -> Chuyển sang Sleep Mode! <<<");
  }

  if (millis() - lastTelemetrySyncTime > TELEMETRY_INTERVAL) {
    lastTelemetrySyncTime = millis();
    syncTelemetryToCloud();
  }
  
  yield();
}