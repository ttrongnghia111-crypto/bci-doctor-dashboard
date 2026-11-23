#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>
#include <math.h>

// File include/web_ui.h chuẩn cấu trúc PlatformIO (src/main.cpp + include/web_ui.h)
#if __has_include("web_ui.h")
  #include "web_ui.h"
#else
  #include "include/web_ui.h"
#endif

// =========================================================================
// CẤU HÌNH PHẦN CỨNG & TÍNH NĂNG (BẬT = 1, TẮT = 0)
// =========================================================================
#define USE_INA226       0  // Mạch hiện tại không gắn INA226 -> Tắt dò I2C 0x40
#define USE_BATTERY_ADC  0  // Chân GPIO 1 đang bỏ trống -> Tắt đọc ADC tránh nhiễu

// Cổng UDP giao tiếp thời gian thực (<5ms) với Gateway ESP32-C3
#define GW_UDP_PORT      4210
#define AP_FIXED_CHANNEL 6

// Hỗ trợ tách mật khẩu Wi-Fi sang file include/secrets.h khi đóng gói nộp hồ sơ
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  const char* WIFI_SSID_1 = "iPhone";
  const char* WIFI_PASS_1 = "27112012";
  const char* WIFI_SSID_2 = "Trong Nghia";
  const char* WIFI_PASS_2 = "109876543";
  const char* WIFI_SSID_3 = "Tuyet Nghia";
  const char* WIFI_PASS_3 = "@tuyet1003@";
  const char* AP_SSID     = "ESP32_BCI_SYSTEM";
  const char* AP_PASS     = "12345678";
#endif

// =========================================================================
// ENUM & NGUYÊN MẪU HÀM (FORWARD DECLARATIONS - PLATFORMIO)
// =========================================================================
enum SensorInputMode : uint8_t {
  INPUT_HYBRID_BOTH = 0,
  INPUT_IR_ONLY     = 1,
  INPUT_NOD_ONLY    = 2
};

enum GlassesNetMode : uint8_t {
  NET_TRY_ROUTER = 0,   // Khởi động: ưu tiên kết nối Router
  NET_ROUTER     = 1,   // Đang chạy trên Router LAN (+ Cloud)
  NET_LOCAL_AP   = 2    // Router hỏng -> Phát Web Local AP để điện thoại & Gateway nối vào
};

enum CloudCmdAction : uint8_t {
  CLOUD_CMD_NONE           = 0,
  CLOUD_CMD_LOCK_REST      = 1,
  CLOUD_CMD_UNLOCK_REST    = 2,
  CLOUD_CMD_LOCK_CONTROL   = 3,
  CLOUD_CMD_UNLOCK_CONTROL = 4,
  CLOUD_CMD_INPUT_HYBRID   = 5,
  CLOUD_CMD_INPUT_IR       = 6,
  CLOUD_CMD_INPUT_NOD      = 7,
  CLOUD_CMD_R1_ON          = 8,
  CLOUD_CMD_R1_OFF         = 9,
  CLOUD_CMD_R2_ON          = 10,
  CLOUD_CMD_R2_OFF         = 11
};

void triggerBuzzer(uint8_t pattern);
void updateBuzzer();
void processTCRT5000();
void setInputMode(SensorInputMode mode);
void broadcastInputMode();
void broadcastRelayState();
void sendNetStatusTo(uint8_t num);
void sendGatewayStatusTo(uint8_t num);
void broadcastNetStatus();
void broadcastGatewayStatus();
void applyRelayLocalAndGateway(uint8_t channel, bool state, bool notifyCloud);
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
void initPowerMonitor();
void readPowerMonitor();
void enqueueCloudSentence(const String& sentence, bool highPriority = false);
void syncSentenceToCloud(const String& sentence);
void syncCommandToCloud(const String& cmd);
void syncTelemetryToCloud();
void enqueueCloudAction(CloudCmdAction act, unsigned long durationSec = 0);
bool dequeueCloudAction(CloudCmdAction &act, unsigned long &durationSec);
void parseTelemetryResponseForCmd(const String& resp);
void sanitizeJsonString(const String& in, char* out, size_t outSize);
IPAddress getStaSubnetBroadcast();
void sendUdpRaw(const IPAddress& ip, const char* msg);
void queueGatewayRelayCmd(uint8_t channel, bool state);
void sendSyncToGateway();
void sendRejoinRouterToGateway();
void announceGlassesOnLan();
void processGatewayUdp();
void startGlassesMdns();
void activateLocalApFallback();
bool probeRouterRecovery();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleRoot();
void cloudSyncTask(void * parameter);
void hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t &r, uint8_t &g, uint8_t &b);
void rgbRainbowTask(void * parameter);

// =========================================================================
// HỆ THỐNG HMI ESP32-S3 - ROUTER-FIRST + AUTO LOCAL AP FAILOVER + GATEWAY
// =========================================================================
WiFiMulti wifiMulti;
WiFiClientSecure secureClient;
WiFiUDP gwUdp;

const char* CLOUD_SERVER_URL    = "https://bci-api-server.onrender.com/api/logs";
const char* CLOUD_TELEMETRY_URL = "https://bci-api-server.onrender.com/api/telemetry";
const char* CLOUD_COMMAND_URL   = "https://bci-api-server.onrender.com/api/command";

// Quản lý khóa nghỉ ngơi do kiệt sức (CFI >= 75% hoặc Cloud)
volatile bool isFatigueLocked = false;
unsigned long fatigueLockStartTime = 0;
unsigned long fatigueLockDurationMs = 30UL * 60UL * 1000UL;
String fatigueLockSource = "GLOBAL";
unsigned long ignoreLockUntil = 0;

// Trạng thái mạng (Core 0 cập nhật, Core 1 phát lên Web UI & rebind UDP)
volatile GlassesNetMode currentNetMode = NET_TRY_ROUTER;
volatile bool pendingNetBroadcast = false;
volatile bool pendingUdpRebind = false;
volatile bool pendingLanAnnounce = false;
volatile bool forceRouterRetry = false;
volatile uint8_t activeWsClientCount = 0;
char currentNetIpBuf[24] = "0.0.0.0";

// Gửi CMD:REJOIN_ROUTER lặp 3 lần (cách 250ms) chống rớt gói UDP
volatile uint8_t pendingRejoinRepeats = 0;
unsigned long lastRejoinRepeatMs = 0;

// Quản lý kết nối & hàng đợi gửi lệnh tin cậy tới Gateway ESP32-C3 (Core 1)
struct GatewayCmdSlot {
  bool active;
  bool targetState;
  uint16_t cmdId;
  uint8_t retries;
  unsigned long lastSentMs;
};

GatewayCmdSlot gwCmdSlots[2] = {
  { false, false, 0, 0, 0 },
  { false, false, 0, 0, 0 }
};
uint16_t gwCmdSeq = 1;
IPAddress gatewayIP(0, 0, 0, 0);
volatile bool gatewayOnline = false;
unsigned long lastGatewaySeenMs = 0;
bool relayDirtyForGateway = false;

// Quản lý Khóa/Mở điều khiển thông minh
volatile bool isControlLocked = false;
unsigned long unlockNodTimestamps[3] = {0, 0, 0};
uint8_t unlockNodCount = 0;
const unsigned long UNLOCK_TRIPLE_WINDOW_MS = 3500UL;

// Phát hiện "Cúi đầu giữ yên 1.2 giây" để Khóa/Mở chủ động
unsigned long nodHoldStartMs = 0;
bool nodHoldTriggered = false;
const unsigned long NOD_HOLD_DURATION_MS = 1200UL;

// =========================================================================
// HÀNG ĐỢI ĐA LUỒNG CLOUD AN TOÀN (RING BUFFER KHÔNG MẤT SOS)
// =========================================================================
SemaphoreHandle_t cloudMutex = NULL;

#define CLOUD_LOG_QUEUE_SIZE 6
struct CloudLogItem {
  char sentence[200];
  bool highPriority;
};
CloudLogItem cloudLogQueue[CLOUD_LOG_QUEUE_SIZE];
uint8_t cloudLogHead = 0;
uint8_t cloudLogCount = 0;

#define CLOUD_ACT_QUEUE_SIZE 8
struct CloudActionItem {
  CloudCmdAction action;
  unsigned long durationSec;
};
CloudActionItem cloudActQueue[CLOUD_ACT_QUEUE_SIZE];
uint8_t cloudActHead = 0;
uint8_t cloudActCount = 0;

String pendingCloudCommand = "";
volatile bool pendingTelemetry = false;

void enqueueCloudSentence(const String& sentence, bool highPriority) {
  if (sentence.length() == 0 || !cloudMutex) return;
  if (xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(25)) == pdTRUE) {
    uint8_t slotIdx;
    if (cloudLogCount < CLOUD_LOG_QUEUE_SIZE) {
      slotIdx = (cloudLogHead + cloudLogCount) % CLOUD_LOG_QUEUE_SIZE;
      cloudLogCount++;
    } else {
      // Hàng đợi đầy: tìm ô thường cũ nhất để ghi đè, không xóa SOS/khẩn cấp
      int replacePos = -1;
      for (uint8_t i = 0; i < CLOUD_LOG_QUEUE_SIZE; i++) {
        uint8_t idx = (cloudLogHead + i) % CLOUD_LOG_QUEUE_SIZE;
        if (!cloudLogQueue[idx].highPriority) {
          replacePos = idx;
          break;
        }
      }
      if (replacePos >= 0) {
        slotIdx = (uint8_t)replacePos;
      } else if (highPriority) {
        slotIdx = cloudLogHead;
        cloudLogHead = (cloudLogHead + 1) % CLOUD_LOG_QUEUE_SIZE;
      } else {
        xSemaphoreGive(cloudMutex);
        return;
      }
    }
    strncpy(cloudLogQueue[slotIdx].sentence, sentence.c_str(), sizeof(cloudLogQueue[slotIdx].sentence) - 1);
    cloudLogQueue[slotIdx].sentence[sizeof(cloudLogQueue[slotIdx].sentence) - 1] = '\0';
    cloudLogQueue[slotIdx].highPriority = highPriority;
    xSemaphoreGive(cloudMutex);
  }
}

void syncSentenceToCloud(const String& sentence) {
  bool isEmerg = (sentence.indexOf("SOS") >= 0 || sentence.indexOf("CO GIẬT") >= 0 || sentence.indexOf("KHẨN CẤP") >= 0);
  enqueueCloudSentence(sentence, isEmerg);
}

void syncCommandToCloud(const String& cmd) {
  if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    pendingCloudCommand = cmd;
    xSemaphoreGive(cloudMutex);
  }
}

void syncTelemetryToCloud() { pendingTelemetry = true; }

void enqueueCloudAction(CloudCmdAction act, unsigned long durationSec) {
  if (act == CLOUD_CMD_NONE || !cloudMutex) return;
  if (xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    if (cloudActCount < CLOUD_ACT_QUEUE_SIZE) {
      uint8_t idx = (cloudActHead + cloudActCount) % CLOUD_ACT_QUEUE_SIZE;
      cloudActQueue[idx].action = act;
      cloudActQueue[idx].durationSec = durationSec;
      cloudActCount++;
    }
    xSemaphoreGive(cloudMutex);
  }
}

bool dequeueCloudAction(CloudCmdAction &act, unsigned long &durationSec) {
  bool ok = false;
  if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    if (cloudActCount > 0) {
      act = cloudActQueue[cloudActHead].action;
      durationSec = cloudActQueue[cloudActHead].durationSec;
      cloudActHead = (cloudActHead + 1) % CLOUD_ACT_QUEUE_SIZE;
      cloudActCount--;
      ok = true;
    }
    xSemaphoreGive(cloudMutex);
  }
  return ok;
}

void sanitizeJsonString(const String& in, char* out, size_t outSize) {
  size_t j = 0;
  for (size_t i = 0; i < in.length() && j + 2 < outSize; i++) {
    char c = in[i];
    if (c == '"' || c == '\\') {
      out[j++] = '\\';
      out[j++] = c;
    } else if ((uint8_t)c >= 0x20) {
      out[j++] = c;
    }
  }
  out[j] = '\0';
}

// =========================================================================
// CẤU HÌNH CHÂN I2C & NGOẠI VI ESP32-S3 (PRO-CAD V8.2)
// =========================================================================
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

#if USE_INA226
  #define INA226_I2C_ADDR 0x40
#endif

uint8_t LSM6DS3_ADDR = 0x6A;

#define LSM6DS3_WHO_AM_I   0x0F
#define LSM6DS3_CTRL1_XL   0x10
#define LSM6DS3_CTRL2_G    0x11
#define LSM6DS3_CTRL3_C    0x12
#define LSM6DS3_CTRL7_G    0x16
#define LSM6DS3_CTRL8_XL   0x17
#define LSM6DS3_OUT_TEMP_L 0x20

#define RELAY1_PIN 10
#define RELAY2_PIN 11
#if USE_BATTERY_ADC
  #define BATTERY_PIN 1
#endif

#define IR_LED_PULSE_PIN  5
#define IR_SENSOR_ADC_PIN 2
#define BUZZER_PIN        4

#ifndef RGB_LED_PIN
  #define RGB_LED_PIN 48
#endif
#define RGB_BRIGHTNESS_LEVEL 45

bool enableIrDebug = false;

// =========================================================================
// CẤU TRÚC 2 MODE GẬT ĐẦU (NHẸ / NẶNG / KÉP) & BỘ LỌC Y SINH 5 TẦNG
// =========================================================================
enum NodMode : uint8_t {
  NOD_MODE_LIGHT = 0,
  NOD_MODE_HEAVY = 1,
  NOD_MODE_DUAL  = 2
};

struct NodModeProfile {
  float triggerDps;
  float commitDps;
  float commitAngleDeg;
  float minJerk;
  float minTkeo;
  float yawVetoRatio;
  float shockVetoG;
  unsigned long minCommitMs;
  unsigned long maxWindowMs;
  unsigned long cooldownMs;
};

const NodModeProfile PROFILE_LIGHT = {
  13.5f, 16.5f, 1.35f, 0.78f, 22.0f, 0.70f, 0.45f, 25, 230, 220
};

const NodModeProfile PROFILE_HEAVY = {
  26.0f, 36.0f, 3.80f, 1.75f, 90.0f, 0.55f, 0.35f, 30, 380, 300
};

volatile uint8_t currentNodMode = NOD_MODE_DUAL;

enum NodStateMachine : uint8_t {
  PHASE_IDLE = 0,
  PHASE_COMMITTING = 1,
  PHASE_DUAL_CLASSIFY = 2,
  PHASE_RECOIL_ABSORB = 3,
  PHASE_COOLDOWN = 4
};

NodStateMachine nodPhase = PHASE_IDLE;
unsigned long recoilQuietStartMs = 0;
bool hasLSM6DS3 = false;
bool gravInitialized = false;
float accX = 0.0f, accY = 0.0f, accZ = 1.0f;
float gyroX = 0.0f, gyroY = 0.0f, gyroZ = 0.0f;
float totalRawGyro = 0.0f;
float accMag = 1.0f;
float sensorTempC = 34.5f;

float gravX = 0.0f, gravY = 0.0f, gravZ = 1.0f;
float baseGyroX = 0.0f, baseGyroY = 0.0f, baseGyroZ = 0.0f;
float compPitchAngle = 0.0f;
float compRollAngle  = 0.0f;
float headTiltFromRestDeg = 0.0f;
int lockedAxisIdx = 0;

float hpPitchState = 0.0f;
float prevRawPitch = 0.0f;
float lpStage1 = 0.0f;
float lpStage2 = 0.0f;

float omegaHist1 = 0.0f;
float omegaHist2 = 0.0f;
float currentTkeo = 0.0f;
float currentJerk = 0.0f;
float baselineTkeo = 2.0f;

float yawRollNoiseEnv = 0.0f;
float linearShockEnv = 0.0f;

int activeNodSign = 1;
unsigned long phaseStartMs = 0;
unsigned long dualClassifyStartMs = 0;
unsigned long lastNodTriggerMs = 0;
float nodIntegratedAngle = 0.0f;
float nodPeakDps = 0.0f;
float nodPeakTkeo = 0.0f;

float tremorBandEnergy = 0.0f;
float cfiScore = 12.0f;
float baselinePeakDps = 32.0f;

uint8_t spasmConfirmCount = 0;
unsigned long lastSpasmAlertMs = 0;

float anchorGravX = 0.0f, anchorGravY = 0.0f, anchorGravZ = 1.0f;
unsigned long postureStaticStartMs = 0;
unsigned long lastBedsoreAlertMs = 0;
const unsigned long BEDSORE_LIMIT_MS = 120UL * 60UL * 1000UL;
volatile uint16_t staticPostureMinutes = 0;

volatile int signalQuality = 0;
volatile int attention = 0;
volatile int meditation = 95;
volatile int blinkStrength = 0;
volatile float lastIrDelta = 0.0f;
String lastNodTypeStr = "NONE";
unsigned long delta = 0, theta = 0;

WebServer server(80);
WebSocketsServer webSocket(81);

bool hasINA226 = false;
float inaBusVoltage = 5.00f;
float inaCurrent_mA = 110.0f;
float inaPower_mW = 550.0f;
float inaShunt_mV = 0.0f;
int batteryPercent = 100;

volatile bool relay1State = false;
volatile bool relay2State = false;

volatile bool isSleeping = false;
unsigned long lastActivityTime = 0;
const unsigned long AUTO_SLEEP_TIMEOUT = 15UL * 60UL * 1000UL;
unsigned long lastTelemetrySyncTime = 0;
const unsigned long TELEMETRY_INTERVAL_ACTIVE = 3000UL;   // 3 giây khi hoạt động
const unsigned long TELEMETRY_INTERVAL_SLEEP  = 15000UL;  // 15 giây khi ngủ

// =========================================================================
// MODULE CÒI PHẢN HỒI XÚC GIÁC ÂM THANH (NON-BLOCKING BUZZER SMD 5020)
// =========================================================================
struct BuzzerStep {
  bool pinState;
  uint16_t durationMs;
};

const BuzzerStep BUZZER_PATTERN_1[] = { { true, 35 }, { false, 0 } };
const BuzzerStep BUZZER_PATTERN_2[] = { { true, 40 }, { false, 40 }, { true, 60 }, { false, 0 } };
const BuzzerStep BUZZER_PATTERN_3[] = { { true, 180 }, { false, 100 }, { true, 180 }, { false, 100 }, { true, 250 }, { false, 0 } };

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
  if (buzzerCurrentPattern == 3 && pattern < 3) return;

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
  if (buzzerCurrentPattern == 0 || buzzerActiveSteps == nullptr) return;

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
// CHẾ ĐỘ NGUỒN ĐẦU VÀO CẢM BIẾN & HÀM PHÁT TRẠNG THÁI (CHỈ CHẠY TRÊN CORE 1)
// =========================================================================
volatile SensorInputMode currentInputMode = INPUT_HYBRID_BOTH;

void broadcastRelayState() {
  char buf[20];
  snprintf(buf, sizeof(buf), "RELAY:%u,%u", relay1State ? 1 : 0, relay2State ? 1 : 0);
  webSocket.broadcastTXT(buf);
}

void applyRelayLocalAndGateway(uint8_t channel, bool state, bool notifyCloud) {
  if (channel == 1) {
    relay1State = state;
    digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
  } else if (channel == 2) {
    relay2State = state;
    digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
  } else {
    return;
  }
  relayDirtyForGateway = true;
  queueGatewayRelayCmd(channel, state);
  broadcastRelayState();
  if (notifyCloud) syncTelemetryToCloud();
}

static void buildNetStatusPayload(char* buf, size_t bufSize) {
  char ipCopy[24] = "0.0.0.0";
  if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    strncpy(ipCopy, currentNetIpBuf, sizeof(ipCopy) - 1);
    ipCopy[sizeof(ipCopy) - 1] = '\0';
    xSemaphoreGive(cloudMutex);
  }
  const char* modeStr = (currentNetMode == NET_ROUTER)   ? "ROUTER" :
                        (currentNetMode == NET_LOCAL_AP) ? "LOCAL_AP" : "CONNECTING";
  snprintf(buf, bufSize, "NET:%s,%s", modeStr, ipCopy);
}

static void buildGatewayStatusPayload(char* buf, size_t bufSize) {
  if (gatewayOnline) {
    snprintf(buf, bufSize, "GW:ONLINE,%s", gatewayIP.toString().c_str());
  } else {
    snprintf(buf, bufSize, "GW:OFFLINE");
  }
}

void sendNetStatusTo(uint8_t num) {
  char buf[64];
  buildNetStatusPayload(buf, sizeof(buf));
  webSocket.sendTXT(num, buf);
}

void sendGatewayStatusTo(uint8_t num) {
  char buf[48];
  buildGatewayStatusPayload(buf, sizeof(buf));
  webSocket.sendTXT(num, buf);
}

void broadcastNetStatus() {
  char buf[64];
  buildNetStatusPayload(buf, sizeof(buf));
  webSocket.broadcastTXT(buf);
}

void broadcastGatewayStatus() {
  char buf[48];
  buildGatewayStatusPayload(buf, sizeof(buf));
  webSocket.broadcastTXT(buf);
}

void broadcastInputMode() {
  const char* modeStr = (currentInputMode == INPUT_IR_ONLY)  ? "INPUT_MODE:IR_ONLY" :
                        (currentInputMode == INPUT_NOD_ONLY) ? "INPUT_MODE:NOD_ONLY" :
                                                               "INPUT_MODE:HYBRID";
  webSocket.broadcastTXT(modeStr);
}

void setInputMode(SensorInputMode mode) {
  currentInputMode = mode;
  const char* mName = (currentInputMode == INPUT_IR_ONLY) ? "CHỈ KÍNH IR (TCRT5000)" :
                      (currentInputMode == INPUT_NOD_ONLY) ? "CHỈ GẬT ĐẦU (LSM6DS3)" :
                                                             "HYBRID KÉP (IR + IMU)";
  Serial.printf("[INPUT SOURCE] Chuyển nguồn cảm biến: %s\n", mName);
  broadcastInputMode();
  syncTelemetryToCloud();
}

void triggerLocalFatigueLock(unsigned long durationSec, String source) {
  isFatigueLocked = true;
  fatigueLockStartTime = millis();
  fatigueLockDurationMs = (durationSec > 0 ? durationSec : 1800UL) * 1000UL;
  fatigueLockSource = source;

  triggerBuzzer(3);
  webSocket.broadcastTXT("FATIGUE_LOCK:" + String(fatigueLockDurationMs / 1000UL) + "," + source);
  syncTelemetryToCloud();
  Serial.printf("[FATIGUE ALERT] Kích hoạt khóa nghỉ %lu giây. Nguồn: %s\n",
                fatigueLockDurationMs / 1000UL, source.c_str());
}

void releaseLocalFatigueLock() {
  if (isFatigueLocked) {
    isFatigueLocked = false;
    cfiScore = 18.0f;
    ignoreLockUntil = millis() + 30000;
    webSocket.broadcastTXT("FATIGUE_UNLOCK");
    syncTelemetryToCloud();
    Serial.println("[FATIGUE] Đã mở khóa hệ thống.");
  }
}

void broadcastNodMode() {
  const char* modeStr = (currentNodMode == NOD_MODE_LIGHT) ? "MODE:LIGHT" :
                        (currentNodMode == NOD_MODE_HEAVY) ? "MODE:HEAVY" : "MODE:DUAL";
  webSocket.broadcastTXT(modeStr);
}

void setNodMode(uint8_t newMode) {
  currentNodMode = newMode % 3;
  nodPhase = PHASE_IDLE;
  recoilQuietStartMs = 0;
  const char* mName = (currentNodMode == NOD_MODE_LIGHT) ? "MODE NHẸ (SIÊU NHẠY)" :
                      (currentNodMode == NOD_MODE_HEAVY) ? "MODE NẶNG (DỨT KHOÁT)" :
                                                           "MODE KÉP (TỰ ĐỘNG PHÂN LOẠI NHẸ/NẶNG)";
  Serial.printf("[LSM6DS3 MODE] Đã chuyển sang: %s\n", mName);
  broadcastNodMode();
  syncTelemetryToCloud();
}

void broadcastControlLock() {
  webSocket.broadcastTXT(isControlLocked ? "LOCK_CONTROL:ON" : "LOCK_CONTROL:OFF");
}

void setControlLock(bool lockState) {
  isControlLocked = lockState;
  unlockNodCount = 0;
  Serial.printf(">>> [CONTROL LOCK] Trạng thái điều khiển: %s <<<\n",
                isControlLocked ? "ĐÃ KHÓA (LOCK_CONTROL:ON)" : "ĐÃ MỞ KHÓA (LOCK_CONTROL:OFF)");
  broadcastControlLock();
  syncTelemetryToCloud();
  triggerBuzzer(isControlLocked ? 3 : 2);
}

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
// GIAO TIẾP UDP THỜI GIAN THỰC VỚI GATEWAY ESP32-C3 (CORE 1 - KHÔNG BLOCK)
// =========================================================================
IPAddress getStaSubnetBroadcast() {
  IPAddress ip = WiFi.localIP();
  IPAddress mask = WiFi.subnetMask();
  if (ip[0] == 0) return IPAddress(255, 255, 255, 255);
  return IPAddress(
    (ip[0] & mask[0]) | (~mask[0] & 0xFF),
    (ip[1] & mask[1]) | (~mask[1] & 0xFF),
    (ip[2] & mask[2]) | (~mask[2] & 0xFF),
    (ip[3] & mask[3]) | (~mask[3] & 0xFF)
  );
}

void sendUdpRaw(const IPAddress& ip, const char* msg) {
  if (ip[0] == 0) return;
  gwUdp.beginPacket(ip, GW_UDP_PORT);
  gwUdp.write((const uint8_t*)msg, strlen(msg));
  gwUdp.endPacket();
}

void queueGatewayRelayCmd(uint8_t channel, bool state) {
  if (channel < 1 || channel > 2) return;
  uint8_t idx = channel - 1;

  gwCmdSeq++;
  if (gwCmdSeq == 0) gwCmdSeq = 1;

  gwCmdSlots[idx].active = true;
  gwCmdSlots[idx].targetState = state;
  gwCmdSlots[idx].cmdId = gwCmdSeq;
  gwCmdSlots[idx].retries = 0;
  gwCmdSlots[idx].lastSentMs = 0;
}

void sendSyncToGateway() {
  if (gatewayIP[0] == 0) return;
  char buf[24];
  snprintf(buf, sizeof(buf), "CMD:SYNC:%u,%u", relay1State ? 1 : 0, relay2State ? 1 : 0);
  sendUdpRaw(gatewayIP, buf);
}

void sendRejoinRouterToGateway() {
  const char* msg = "CMD:REJOIN_ROUTER";
  if (gatewayIP[0] != 0) sendUdpRaw(gatewayIP, msg);
  sendUdpRaw(IPAddress(192, 168, 4, 255), msg);
  sendUdpRaw(IPAddress(255, 255, 255, 255), msg);
  Serial.println("[GATEWAY] Gửi CMD:REJOIN_ROUTER để Gateway chuyển về Router.");
}

void announceGlassesOnLan() {
  char buf[32];
  snprintf(buf, sizeof(buf), "IR_HELLO:%u,%u", relay1State ? 1 : 0, relay2State ? 1 : 0);
  if (currentNetMode == NET_ROUTER && WiFi.status() == WL_CONNECTED) {
    sendUdpRaw(getStaSubnetBroadcast(), buf);
    sendUdpRaw(IPAddress(255, 255, 255, 255), buf);
  } else if (currentNetMode == NET_LOCAL_AP) {
    sendUdpRaw(IPAddress(192, 168, 4, 255), buf);
  }
}

void processGatewayUdp() {
  unsigned long now = millis();

  // 0. Lặp gửi CMD:REJOIN_ROUTER 3 lần (cách 250ms) chống trượt gói
  if (pendingRejoinRepeats > 0 && (now - lastRejoinRepeatMs >= 250)) {
    lastRejoinRepeatMs = now;
    pendingRejoinRepeats = pendingRejoinRepeats - 1;
    sendRejoinRouterToGateway();
  }

  // 1. Gửi / gửi lại lệnh R1, R2 chưa nhận ACK (tối đa 4 lần, mỗi 120ms)
  for (uint8_t i = 0; i < 2; i++) {
    if (gwCmdSlots[i].active) {
      if (gwCmdSlots[i].lastSentMs == 0 || (now - gwCmdSlots[i].lastSentMs >= 120)) {
        if (gwCmdSlots[i].retries >= 4) {
          gwCmdSlots[i].active = false;
          Serial.printf("[GATEWAY] Cảnh báo: Lệnh R%u (seq=%u) không nhận được ACK!\n",
                        i + 1, gwCmdSlots[i].cmdId);
        } else {
          char cmdBuf[36];
          snprintf(cmdBuf, sizeof(cmdBuf), "CMD:R%u:%u,%u",
                   i + 1, gwCmdSlots[i].targetState ? 1 : 0, gwCmdSlots[i].cmdId);

          if (gatewayOnline && gatewayIP[0] != 0) {
            sendUdpRaw(gatewayIP, cmdBuf);
          } else {
            if (currentNetMode == NET_ROUTER && WiFi.status() == WL_CONNECTED) {
              sendUdpRaw(getStaSubnetBroadcast(), cmdBuf);
            }
            if (currentNetMode == NET_LOCAL_AP) {
              sendUdpRaw(IPAddress(192, 168, 4, 255), cmdBuf);
            }
            sendUdpRaw(IPAddress(255, 255, 255, 255), cmdBuf);
          }

          gwCmdSlots[i].lastSentMs = now;
          gwCmdSlots[i].retries++;
        }
      }
    }
  }

  // 2. Đọc gói tin UDP từ Gateway (Non-blocking)
  int pktSize = gwUdp.parsePacket();
  while (pktSize > 0) {
    char rxBuf[96];
    int len = gwUdp.read(rxBuf, sizeof(rxBuf) - 1);
    if (len > 0) {
      rxBuf[len] = '\0';
      IPAddress senderIP = gwUdp.remoteIP();

      if (senderIP == WiFi.localIP() || senderIP == WiFi.softAPIP()) {
        pktSize = gwUdp.parsePacket();
        continue;
      }

      // Chống Split-Brain: Kính đã lên Router mà Gateway vẫn gửi gói từ dải AP 192.168.4.x
      if (currentNetMode == NET_ROUTER && senderIP[0] == 192 && senderIP[1] == 168 && senderIP[2] == 4) {
        sendUdpRaw(senderIP, "CMD:REJOIN_ROUTER");
        pktSize = gwUdp.parsePacket();
        continue;
      }

      if (strncmp(rxBuf, "ACK:", 4) == 0) {
        unsigned int ackId = 0;
        int lVal = 0, fVal = 0;
        if (sscanf(rxBuf + 4, "%u,%d,%d", &ackId, &lVal, &fVal) == 3) {
          gatewayIP = senderIP;
          lastGatewaySeenMs = now;
          if (!gatewayOnline) {
            gatewayOnline = true;
            broadcastGatewayStatus();
            syncTelemetryToCloud();
          }

          for (uint8_t i = 0; i < 2; i++) {
            if (gwCmdSlots[i].active && gwCmdSlots[i].cmdId == (uint16_t)ackId) {
              gwCmdSlots[i].active = false;
              Serial.printf("[GATEWAY] ACK R%u OK (seq=%u) | Đèn=%d Quạt=%d\n",
                            i + 1, ackId, lVal, fVal);
            }
          }
          if (!gwCmdSlots[0].active && !gwCmdSlots[1].active) relayDirtyForGateway = false;

          bool changed = false;
          if (!gwCmdSlots[0].active && relay1State != (lVal != 0)) {
            relay1State = (lVal != 0);
            digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
            changed = true;
          }
          if (!gwCmdSlots[1].active && relay2State != (fVal != 0)) {
            relay2State = (fVal != 0);
            digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
            changed = true;
          }
          if (changed) {
            broadcastRelayState();
            syncTelemetryToCloud();
          }
        }
      }
      else if (strncmp(rxBuf, "GW_HELLO:", 9) == 0 || strncmp(rxBuf, "GW_STATE:", 9) == 0) {
        int lVal = 0, fVal = 0;
        if (sscanf(rxBuf + 9, "%d,%d", &lVal, &fVal) >= 2) {
          bool wasOffline = !gatewayOnline || (gatewayIP != senderIP);
          gatewayIP = senderIP;
          gatewayOnline = true;
          lastGatewaySeenMs = now;

          char pongBuf[32];
          snprintf(pongBuf, sizeof(pongBuf), "IR_PONG:%u,%u", relay1State ? 1 : 0, relay2State ? 1 : 0);
          sendUdpRaw(senderIP, pongBuf);

          if (wasOffline) {
            Serial.printf("[GATEWAY] Đã kết nối Gateway IoT tại %s!\n", gatewayIP.toString().c_str());
            broadcastGatewayStatus();
            if (relayDirtyForGateway) {
              sendSyncToGateway();
              relayDirtyForGateway = false;
            }
            syncTelemetryToCloud();
          }

          bool changed = false;
          if (!gwCmdSlots[0].active && !relayDirtyForGateway && relay1State != (lVal != 0)) {
            relay1State = (lVal != 0);
            digitalWrite(RELAY1_PIN, relay1State ? HIGH : LOW);
            changed = true;
          }
          if (!gwCmdSlots[1].active && !relayDirtyForGateway && relay2State != (fVal != 0)) {
            relay2State = (fVal != 0);
            digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
            changed = true;
          }
          if (changed) {
            broadcastRelayState();
            syncTelemetryToCloud();
          }
        }
      }
    }
    pktSize = gwUdp.parsePacket();
  }

  // 3. Timeout Gateway (7.5 giây không nhận Heartbeat)
  if (gatewayOnline && (now - lastGatewaySeenMs > 7500)) {
    gatewayOnline = false;
    Serial.println("[GATEWAY] Mất kết nối với Gateway IoT (Offline).");
    broadcastGatewayStatus();
    syncTelemetryToCloud();
  }
}

// =========================================================================
// GIAO THỨC I2C TRỰC TIẾP LSM6DS3 (BẬT LỌC PHẦN CỨNG SILICON)
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
      gravInitialized = false;
      signalQuality = 200;
      return;
    }
  }

  uint8_t whoAmI = readI2CRegister(LSM6DS3_ADDR, LSM6DS3_WHO_AM_I);
  if (whoAmI == 0x69 || whoAmI == 0x6A || whoAmI == 0x6C) {
    hasLSM6DS3 = true;
    gravInitialized = false;
    signalQuality = 0;
    Serial.printf("[LSM6DS3] Khởi động thành công tại 0x%02X! WHO_AM_I = 0x%02X\n", LSM6DS3_ADDR, whoAmI);

    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL1_XL, 0x5B); // 208Hz, +-4g, Anti-aliasing 50Hz
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL2_G, 0x54);  // 208Hz, 500 dps
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL3_C, 0x44);  // BDU + IF_INC
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL7_G, 0x40);  // Gyro HPF 0.016Hz
    writeI2CRegister(LSM6DS3_ADDR, LSM6DS3_CTRL8_XL, 0x88); // Accel Composite LPF ODR/9
  } else {
    Serial.printf("[LSM6DS3] Thiết bị phản hồi ID lạ (0x%02X)\n", whoAmI);
    hasLSM6DS3 = false;
    gravInitialized = false;
    signalQuality = 200;
  }
}

// =========================================================================
// XỬ LÝ SỰ KIỆN GẬT ĐẦU / NHÁY MẮT & CẬP NHẬT CHỈ SỐ MỎI CƠ CỔ CFI
// =========================================================================
void fireInstantNodEvent(bool isHeavyNod, float angleDeg, float peakDps, float peakTkeo, unsigned long latencyMs) {
  unsigned long nowMs = millis();
  lastNodTriggerMs = nowMs;

  if (isSleeping) {
    if (isHeavyNod) {
      isSleeping = false;
      lastActivityTime = nowMs;
      unlockNodCount = 0;
      WiFi.setSleep(false);
      webSocket.broadcastTXT("WAKEUP");
      syncTelemetryToCloud();
      triggerBuzzer(2);
      Serial.println(">>> [SMART SLEEP WAKEUP!] Đánh thức hệ thống thành công! <<<");
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

  if (peakDps < baselinePeakDps * 0.72f) {
    cfiScore = constrain(cfiScore + 3.2f, 0.0f, 100.0f);
  } else {
    baselinePeakDps = 0.95f * baselinePeakDps + 0.05f * peakDps;
    cfiScore = constrain(cfiScore + 1.1f, 0.0f, 100.0f);
  }

  if (cfiScore >= 75.0f && !isFatigueLocked && millis() > ignoreLockUntil) {
    triggerLocalFatigueLock(1800, "CFI_OVER_75%");
    return;
  }

  if (isControlLocked) {
    if (checkTripleNodToUnlock(nowMs)) return;
    triggerBuzzer(1);
    Serial.printf("[CONTROL LOCKED] Đã nhận %u/3 nhịp mở khóa.\n", unlockNodCount);
    return;
  }

  triggerBuzzer(isHeavyNod ? 2 : 1);
  Serial.printf(">>> [CLICK - %s] Góc: %.2f deg | Đỉnh: %.1f dps | TKEO: %.1f | CFI: %.0f%% | Trễ: %lu ms <<<\n",
                isHeavyNod ? "NẶNG" : "NHẸ",
                angleDeg, peakDps, peakTkeo, cfiScore, latencyMs);

  char wsPayload[80];
  snprintf(wsPayload, sizeof(wsPayload), "BLINK:%d,%s,%.1f,%.0f,%lu",
           (int)blinkStrength, lastNodTypeStr.c_str(), angleDeg, peakDps, latencyMs);
  webSocket.broadcastTXT(wsPayload);
}

// =========================================================================
// ĐƯỜNG ỐNG LỌC 5 TẦNG + HỆ CHUYÊN GIA Y SINH LSM6DS3 (200Hz)
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
  if (nowMicros - lastSampleMicros < 5000) return;
  float dt = (nowMicros - lastSampleMicros) * 1e-6f;
  if (dt > 0.05f || dt <= 0.0f) dt = 0.005f;
  lastSampleMicros = nowMicros;

  Wire.beginTransmission(LSM6DS3_ADDR);
  Wire.write(LSM6DS3_OUT_TEMP_L);
  if (Wire.endTransmission(false) != 0) {
    hasLSM6DS3 = false;
    gravInitialized = false;
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

  sensorTempC = 0.995f * sensorTempC + 0.005f * (25.0f + (rawTemp / 16.0f));

  accX  = rawAX * 0.122f / 1000.0f;
  accY  = rawAY * 0.122f / 1000.0f;
  accZ  = rawAZ * 0.122f / 1000.0f;
  gyroX = rawGX * 17.50f / 1000.0f;
  gyroY = rawGY * 17.50f / 1000.0f;
  gyroZ = rawGZ * 17.50f / 1000.0f;

  unsigned long now = millis();
  totalRawGyro = sqrtf(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
  accMag = sqrtf(accX * accX + accY * accY + accZ * accZ);

  // FIX P0: Neo vector trọng lực từ mẫu đầu tiên để chống tự khóa sau 1,2s khởi động
  if (!gravInitialized && accMag > 0.2f) {
    gravX = accX; gravY = accY; gravZ = accZ;
    anchorGravX = accX / accMag;
    anchorGravY = accY / accMag;
    anchorGravZ = accZ / accMag;
    baseGyroX = gyroX; baseGyroY = gyroY; baseGyroZ = gyroZ;
    postureStaticStartMs = now;
    headTiltFromRestDeg = 0.0f;
    nodHoldStartMs = 0;
    nodHoldTriggered = false;
    gravInitialized = true;
    return;
  }

  if (accMag >= 2.50f && totalRawGyro >= 200.0f) {
    spasmConfirmCount++;
    if (spasmConfirmCount >= 2 && (now - lastSpasmAlertMs > 8000)) {
      lastSpasmAlertMs = now;
      spasmConfirmCount = 0;
      triggerBuzzer(3);
      Serial.printf("[MEDICAL ALERT] PHÁT HIỆN CO GIẬT / TÉ NGÃ! ||A||=%.2fg, |w|=%.1fdps\n", accMag, totalRawGyro);
      webSocket.broadcastTXT("ALERT:SPASM_FALL," + String(accMag, 2) + "," + String(totalRawGyro, 0));
      enqueueCloudSentence("CẢNH BÁO KHẨN CẤP: PHÁT HIỆN CO GIẬT / TÉ NGÃ (||A||=" + String(accMag, 1) + "g)", true);
      syncTelemetryToCloud();
    }
  } else if (spasmConfirmCount > 0) {
    spasmConfirmCount--;
  }

  if (nodPhase == PHASE_IDLE && totalRawGyro < 10.0f && fabsf(accMag - 1.0f) < 0.12f && nodHoldStartMs == 0) {
    baseGyroX = 0.992f * baseGyroX + 0.008f * gyroX;
    baseGyroY = 0.992f * baseGyroY + 0.008f * gyroY;
    baseGyroZ = 0.992f * baseGyroZ + 0.008f * gyroZ;

    gravX = 0.992f * gravX + 0.008f * accX;
    gravY = 0.992f * gravY + 0.008f * accY;
    gravZ = 0.992f * gravZ + 0.008f * accZ;

    cfiScore = constrain(cfiScore - 0.003f, 8.0f, 100.0f);
  }

  float gNorm = sqrtf(gravX * gravX + gravY * gravY + gravZ * gravZ);
  if (gNorm < 0.1f) gNorm = 1.0f;
  float uX = gravX / gNorm;
  float uY = gravY / gNorm;
  float uZ = gravZ / gNorm;

  if (postureStaticStartMs == 0) {
    postureStaticStartMs = now;
    anchorGravX = uX; anchorGravY = uY; anchorGravZ = uZ;
    staticPostureMinutes = 0;
  } else {
    float dotPosture = constrain(uX * anchorGravX + uY * anchorGravY + uZ * anchorGravZ, -1.0f, 1.0f);
    float postureShiftDeg = acosf(dotPosture) * 57.29578f;
    if (postureShiftDeg > 8.5f) {
      postureStaticStartMs = now;
      anchorGravX = uX; anchorGravY = uY; anchorGravZ = uZ;
      staticPostureMinutes = 0;
    } else {
      staticPostureMinutes = (uint16_t)((now - postureStaticStartMs) / 60000UL);
      if ((now - postureStaticStartMs >= BEDSORE_LIMIT_MS) && (now - lastBedsoreAlertMs > 300000UL)) {
        lastBedsoreAlertMs = now;
        Serial.println("[MEDICAL ALERT] CẢNH BÁO CHỐNG LOÉT TÌ ĐÈ: Tư thế cố định quá 120 phút!");
        webSocket.broadcastTXT("ALERT:BEDSORE_120M");
        enqueueCloudSentence("NHẮC ĐIỀU DƯỠNG: Bệnh nhân nằm nguyên 1 tư thế quá 120 phút (Chống loét tì đè)", true);
        syncTelemetryToCloud();
      }
    }
  }

  float dGX = gyroX - baseGyroX;
  float dGY = gyroY - baseGyroY;
  float dGZ = gyroZ - baseGyroZ;

  float dotYaw = dGX * uX + dGY * uY + dGZ * uZ;
  float yawRate = fabsf(dotYaw);

  float pVec[3];
  pVec[0] = dGX - dotYaw * uX;
  pVec[1] = dGY - dotYaw * uY;
  pVec[2] = dGZ - dotYaw * uZ;

  if (nodPhase == PHASE_IDLE) {
    int maxIdx = 0;
    if (fabsf(pVec[1]) > fabsf(pVec[0])) maxIdx = 1;
    if (fabsf(pVec[2]) > fabsf(pVec[maxIdx])) maxIdx = 2;
    if (fabsf(pVec[maxIdx]) > 10.0f) lockedAxisIdx = maxIdx;
  }

  float rawPitchRate = pVec[lockedAxisIdx];
  int rollAxisIdx = (lockedAxisIdx == 0) ? 1 : 0;
  float rawRollRate = pVec[rollAxisIdx];
  float secondaryRollRate = 0.0f;
  for (int i = 0; i < 3; i++) {
    if (i != lockedAxisIdx) secondaryRollRate += fabsf(pVec[i]);
  }
  secondaryRollRate *= 0.7f;

  float accPitchDeg = atan2f(accY, sqrtf(accX * accX + accZ * accZ)) * 57.29578f;
  float accRollDeg  = atan2f(-accX, accZ) * 57.29578f;
  compPitchAngle = 0.96f * (compPitchAngle + rawPitchRate * dt) + 0.04f * accPitchDeg;
  compRollAngle  = 0.96f * (compRollAngle  + rawRollRate  * dt) + 0.04f * accRollDeg;

  float dotAccGrav = (accX * uX + accY * uY + accZ * uZ) / ((accMag > 0.1f) ? accMag : 1.0f);
  dotAccGrav = constrain(dotAccGrav, -1.0f, 1.0f);
  float headTiltDeg = acosf(dotAccGrav) * 57.29578f;
  headTiltFromRestDeg = 0.94f * headTiltFromRestDeg + 0.06f * headTiltDeg;

  if (headTiltFromRestDeg >= 11.5f && totalRawGyro < 22.0f && linearShockEnv < 0.30f) {
    if (nodHoldStartMs == 0) {
      nodHoldStartMs = now;
    } else if (!nodHoldTriggered && (now - nodHoldStartMs >= NOD_HOLD_DURATION_MS)) {
      nodHoldTriggered = true;
      gravX = accX; gravY = accY; gravZ = accZ;
      headTiltFromRestDeg = 0.0f;
      if (currentInputMode != INPUT_IR_ONLY) setControlLock(!isControlLocked);
      nodPhase = PHASE_RECOIL_ABSORB;
      recoilQuietStartMs = 0;
      phaseStartMs = now;
      return;
    }
  } else if (headTiltFromRestDeg < 7.5f) {
    nodHoldStartMs = 0;
    nodHoldTriggered = false;
  }

  hpPitchState = 0.953f * (hpPitchState + rawPitchRate - prevRawPitch);
  prevRawPitch = rawPitchRate;

  lpStage1 = 0.68f * lpStage1 + 0.32f * hpPitchState;
  lpStage2 = 0.68f * lpStage2 + 0.32f * lpStage1;
  float cleanPitchRate = lpStage2;
  float absPitchRate = fabsf(cleanPitchRate);

  float highFreqTremor = fabsf(hpPitchState - cleanPitchRate);
  tremorBandEnergy = 0.95f * tremorBandEnergy + 0.05f * highFreqTremor;
  if (tremorBandEnergy > 2.8f && !isSleeping) {
    cfiScore = constrain(cfiScore + 0.015f, 0.0f, 100.0f);
  }

  float rawTkeo = (cleanPitchRate * cleanPitchRate) - (omegaHist1 * omegaHist2);
  currentTkeo = (rawTkeo > 0.0f) ? rawTkeo : 0.0f;
  currentJerk = fabsf(cleanPitchRate - omegaHist1);

  omegaHist2 = omegaHist1;
  omegaHist1 = cleanPitchRate;

  if (nodPhase == PHASE_IDLE && absPitchRate < 8.0f) {
    baselineTkeo = 0.992f * baselineTkeo + 0.008f * currentTkeo;
    baselineTkeo = constrain(baselineTkeo, 1.5f, 14.0f);
  }

  float instantCrossNoise = yawRate + 0.75f * secondaryRollRate;
  yawRollNoiseEnv = 0.78f * yawRollNoiseEnv + 0.22f * instantCrossNoise;

  float dynLinearShock = fabsf(accMag - gNorm);
  linearShockEnv = 0.72f * linearShockEnv + 0.28f * dynLinearShock;

  const NodModeProfile& activeProf = (isSleeping || currentNodMode == NOD_MODE_HEAVY) ? PROFILE_HEAVY : PROFILE_LIGHT;

  float fatigueAssistScale = 1.0f;
  if (!isSleeping && currentNodMode != NOD_MODE_HEAVY && cfiScore > 35.0f) {
    fatigueAssistScale = constrain(1.0f - ((cfiScore - 35.0f) / 160.0f), 0.75f, 1.0f);
  }
  float dynTriggerDps     = activeProf.triggerDps * fatigueAssistScale;
  float dynCommitDps      = activeProf.commitDps * fatigueAssistScale;
  float dynCommitAngleDeg = activeProf.commitAngleDeg * fatigueAssistScale;
  float dynMinTkeo        = max(activeProf.minTkeo * fatigueAssistScale, baselineTkeo * 4.5f);

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
        if (currentTkeo > nodPeakTkeo) nodPeakTkeo = currentTkeo;
      }

      if (yawRollNoiseEnv > (nodPeakDps * activeProf.yawVetoRatio) ||
          linearShockEnv > activeProf.shockVetoG) {
        nodPhase = PHASE_RECOIL_ABSORB;
        recoilQuietStartMs = 0;
        phaseStartMs = now;
        break;
      }

      bool reachedCommit = (elapsedMs >= activeProf.minCommitMs) &&
                           (nodIntegratedAngle >= dynCommitAngleDeg) &&
                           (nodPeakDps >= dynCommitDps) &&
                           (nodPeakTkeo >= dynMinTkeo);

      if (reachedCommit) {
        unsigned long commitLatencyMs = elapsedMs;

        if (currentInputMode == INPUT_IR_ONLY) {
          nodPhase = PHASE_RECOIL_ABSORB;
          recoilQuietStartMs = 0;
          phaseStartMs = now;
          break;
        }

        if (isSleeping || currentNodMode == NOD_MODE_HEAVY) {
          fireInstantNodEvent(true, nodIntegratedAngle, nodPeakDps, nodPeakTkeo, commitLatencyMs);
          nodPhase = PHASE_RECOIL_ABSORB;
          recoilQuietStartMs = 0;
          phaseStartMs = now;
          break;
        } else if (currentNodMode == NOD_MODE_LIGHT) {
          fireInstantNodEvent(false, nodIntegratedAngle, nodPeakDps, nodPeakTkeo, commitLatencyMs);
          nodPhase = PHASE_RECOIL_ABSORB;
          recoilQuietStartMs = 0;
          phaseStartMs = now;
          break;
        } else {
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
        recoilQuietStartMs = 0;
        phaseStartMs = now;
      }
      break;
    }

    case PHASE_RECOIL_ABSORB: {
      if (absPitchRate < 8.5f) {
        if (recoilQuietStartMs == 0) recoilQuietStartMs = now;
        if (now - recoilQuietStartMs >= 65) {
          recoilQuietStartMs = 0;
          nodPhase = PHASE_COOLDOWN;
          phaseStartMs = now;
        }
      } else {
        recoilQuietStartMs = 0;
      }

      if (now - phaseStartMs > 600) {
        recoilQuietStartMs = 0;
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
}

// =========================================================================
// THUẬT TOÁN ĐỌC VI SAI KHỬ NHIỄU NẮNG TCRT5000 (200Hz) + AUTO-CALIBRATE 2.5s
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
  if (nowMicros - lastIrMicros < 5000) return;
  lastIrMicros = nowMicros;

  digitalWrite(IR_LED_PULSE_PIN, HIGH);
  delayMicroseconds(60);
  int adc_ambient = analogRead(IR_SENSOR_ADC_PIN);

  digitalWrite(IR_LED_PULSE_PIN, LOW);
  delayMicroseconds(60);
  int adc_on = analogRead(IR_SENSOR_ADC_PIN);

  digitalWrite(IR_LED_PULSE_PIN, HIGH);

  float ir_diff = (float)abs(adc_on - adc_ambient);

  static bool irInitDone = false;
  if (!irInitDone) {
    irEmaFast = ir_diff;
    irEmaSlow = ir_diff;
    irBaselineNoise = 10.0f;
    irInitDone = true;
    return;
  }

  irEmaFast = 0.25f * ir_diff + 0.75f * irEmaFast;
  if (!irBlinkActive) {
    irEmaSlow = 0.015f * ir_diff + 0.985f * irEmaSlow;
  }

  float irDelta = fabsf(irEmaFast - irEmaSlow);
  lastIrDelta = irDelta;
  unsigned long nowMs = millis();

  if (irBlinkActive && (nowMs - irBlinkStartMs > 2500)) {
    irBlinkActive = false;
    irEmaSlow = irEmaFast;
    irDelta = 0.0f;
    lastIrDelta = 0.0f;
    irHoldLockTriggered = false;
  }

  if (!irBlinkActive && irDelta < 25.0f) {
    irBaselineNoise = 0.995f * irBaselineNoise + 0.005f * irDelta;
    if (irBaselineNoise < 4.0f) irBaselineNoise = 4.0f;
    if (irBaselineNoise > 40.0f) irBaselineNoise = 40.0f;
  }

  float irThreshold = max(65.0f, irBaselineNoise * 4.2f);

  bool imuVeto = (totalRawGyro > 45.0f || linearShockEnv > 0.30f);
  if (imuVeto) irImuVetoOccurred = true;

  if (currentInputMode == INPUT_NOD_ONLY) {
    irBlinkActive = false;
    irHoldLockTriggered = false;
    return;
  }

  bool isOverThreshold = (irDelta >= irThreshold);

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

      if (pulseDurationMs >= 1800 && !irHoldLockTriggered) {
        if (!irImuVetoOccurred) {
          irHoldLockTriggered = true;
          Serial.println(">>> [IR EYE HOLD >= 1800ms] Đổi trạng thái Khóa/Mở điều khiển! <<<");
          setControlLock(!isControlLocked);
          irLastBlinkEndMs = nowMs;
        }
      }
    }
  } else {
    if (irBlinkActive) {
      unsigned long pulseDurationMs = nowMs - irBlinkStartMs;
      irBlinkActive = false;

      if (!irHoldLockTriggered && !irImuVetoOccurred) {
        if (pulseDurationMs >= 90 && pulseDurationMs <= 420) {
          if (nowMs - irLastBlinkEndMs >= 300) {
            Serial.printf(">>> [IR BLINK CLICK] Xung: %lu ms | Delta: %.1f | Ngưỡng: %.1f <<<\n",
                          pulseDurationMs, irPeakDelta, irThreshold);
            fireInstantNodEvent(false, 2.2f, 25.0f, irPeakDelta, pulseDurationMs);
            irLastBlinkEndMs = nowMs;
          }
        } else if (pulseDurationMs >= 800 && pulseDurationMs <= 1500) {
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

  if (enableIrDebug) {
    static unsigned long lastIrWebDebugMs = 0;
    if (nowMs - lastIrWebDebugMs >= 150) {
      lastIrWebDebugMs = nowMs;
      if (webSocket.connectedClients() > 0) {
        char irMsg[64];
        snprintf(irMsg, sizeof(irMsg), "IR_DEBUG:%d,%d,%.1f,%.1f",
                 adc_ambient, adc_on, irDelta, irThreshold);
        webSocket.broadcastTXT(irMsg);
      }
    }
  }
}

// =========================================================================
// GIÁM SÁT NGUỒN (TỰ NHẬN DIỆN NGUỒN USB 5V VS PIN LI-PO)
// =========================================================================
void initPowerMonitor() {
#if USE_INA226
  Wire.beginTransmission(INA226_I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    hasINA226 = true;
    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x00); Wire.write(0x45); Wire.write(0x27);
    Wire.endTransmission();

    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x05); Wire.write(0x0A); Wire.write(0x00);
    Wire.endTransmission();
  } else {
    hasINA226 = false;
  }
#else
  hasINA226 = false;
#endif
}

void readPowerMonitor() {
#if USE_INA226
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

    inaPower_mW = inaBusVoltage * inaCurrent_mA;
    batteryPercent = constrain((int)(((inaBusVoltage - 3.30f) / 0.90f) * 100.0f), 0, 100);
    return;
  }
#endif

#if USE_BATTERY_ADC
  int rawBat = analogRead(BATTERY_PIN);
  float adcVolt = (rawBat / 4095.0f) * 3.3f * 2.0f;
  if (adcVolt < 2.5f) adcVolt = 3.90f;
  inaBusVoltage = adcVolt;
  inaCurrent_mA = isSleeping ? 25.0f : 110.0f;
  inaPower_mW = inaBusVoltage * inaCurrent_mA;
  batteryPercent = constrain((int)(((inaBusVoltage - 3.30f) / 0.90f) * 100.0f), 0, 100);
#else
  inaBusVoltage = 5.00f;
  inaCurrent_mA = isSleeping ? 28.0f : 110.0f;
  inaPower_mW = inaBusVoltage * inaCurrent_mA;
  inaShunt_mV = 0.0f;
  batteryPercent = 100;
#endif
}

// =========================================================================
// TASK FREERTOS: LED RGB WS2812 CẦU VỒNG ONBOARD
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
    hsvToRgb(hue, 255, RGB_BRIGHTNESS_LEVEL, r, g, b);
    rgbLedWrite(RGB_LED_PIN, r, g, b);
    hue++;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// =========================================================================
// PARSE LỆNH HỢP NHẤT TỪ JSON RESPONSE CỦA POST /api/telemetry (CORE 0)
// =========================================================================
void parseTelemetryResponseForCmd(const String& resp) {
  int keyPos = resp.indexOf("\"cmd\":\"");
  if (keyPos < 0) return;
  int start = keyPos + 7;
  int end = resp.indexOf('"', start);
  if (end <= start) return;

  String cmd = resp.substring(start, end);
  cmd.trim();
  if (cmd == "NONE" || cmd == "IDLE" || cmd.length() == 0) return;

  unsigned long dur = 1800;
  int durPos = resp.indexOf("\"duration\":");
  if (durPos >= 0) {
    long parsedDur = resp.substring(durPos + 11).toInt();
    if (parsedDur > 0) dur = (unsigned long)parsedDur;
  }

  Serial.printf("[CLOUD CMD] Nhận lệnh từ Cloud (hợp nhất Telemetry): %s (dur=%lu)\n", cmd.c_str(), dur);

  if (cmd == "LOCK_REST")           enqueueCloudAction(CLOUD_CMD_LOCK_REST, dur);
  else if (cmd == "UNLOCK_REST")    enqueueCloudAction(CLOUD_CMD_UNLOCK_REST, 0);
  else if (cmd == "LOCK_CONTROL")   enqueueCloudAction(CLOUD_CMD_LOCK_CONTROL, 0);
  else if (cmd == "UNLOCK_CONTROL") enqueueCloudAction(CLOUD_CMD_UNLOCK_CONTROL, 0);
  else if (cmd == "INPUT_HYBRID")   enqueueCloudAction(CLOUD_CMD_INPUT_HYBRID, 0);
  else if (cmd == "INPUT_IR")       enqueueCloudAction(CLOUD_CMD_INPUT_IR, 0);
  else if (cmd == "INPUT_NOD")      enqueueCloudAction(CLOUD_CMD_INPUT_NOD, 0);
  else if (cmd == "R1_ON")          enqueueCloudAction(CLOUD_CMD_R1_ON, 0);
  else if (cmd == "R1_OFF")         enqueueCloudAction(CLOUD_CMD_R1_OFF, 0);
  else if (cmd == "R2_ON")          enqueueCloudAction(CLOUD_CMD_R2_ON, 0);
  else if (cmd == "R2_OFF")         enqueueCloudAction(CLOUD_CMD_R2_OFF, 0);
}

// =========================================================================
// QUẢN LÝ MẠNG ROUTER-FIRST + LOCAL AP FAILOVER & CLOUD SYNC (CORE 0)
// =========================================================================
void startGlassesMdns() {
  MDNS.end();
  if (MDNS.begin("esp32-bci")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[MDNS] Truy cập Kính IR tại: http://esp32-bci.local");
  }
}

void activateLocalApFallback() {
  WiFi.disconnect(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.softAP(AP_SSID, AP_PASS, AP_FIXED_CHANNEL);

  IPAddress apIp = WiFi.softAPIP();
  if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    snprintf(currentNetIpBuf, sizeof(currentNetIpBuf), "%s", apIp.toString().c_str());
    xSemaphoreGive(cloudMutex);
  }

  currentNetMode = NET_LOCAL_AP;
  startGlassesMdns();
  pendingUdpRebind = true;
  pendingNetBroadcast = true;
  pendingLanAnnounce = true;

  Serial.println("==========================================================");
  Serial.printf("[WIFI FAILOVER] Router hỏng -> Đã bật Local AP: %s\n", AP_SSID);
  Serial.printf("[WIFI FAILOVER] Web Local: http://%s (hoặc esp32-bci.local)\n", apIp.toString().c_str());
  Serial.println("[WIFI FAILOVER] Gateway ESP32-C3 sẽ tự động kết nối vào AP này!");
  Serial.println("==========================================================");
}

bool probeRouterRecovery() {
  Serial.println("[WIFI] Kiểm tra xem Router đã có mạng lại chưa...");
  int n = WiFi.scanNetworks(false, false, false, 120);
  bool foundKnownRouter = false;
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid == WIFI_SSID_1 || ssid == WIFI_SSID_2 || ssid == WIFI_SSID_3) {
      foundKnownRouter = true;
      break;
    }
  }
  WiFi.scanDelete();

  if (!foundKnownRouter) return false;

  Serial.println("[WIFI] Phát hiện Router đã hoạt động lại! Đang kết nối STA...");
  if (wifiMulti.run(6000) == WL_CONNECTED) {
    pendingRejoinRepeats = 3; // Core 1 gửi lặp 3 lần (cách 250ms)
    vTaskDelay(pdMS_TO_TICKS(1100));

    if (WiFi.softAPgetStationNum() == 0) {
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      Serial.println("[WIFI] Đã tắt Local AP -> Chạy thuần Router (WIFI_STA).");
    } else {
      Serial.println("[WIFI] Router đã OK! Giữ tạm AP_STA cho tới khi thiết bị đang mở 192.168.4.1 rời đi.");
    }

    if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      snprintf(currentNetIpBuf, sizeof(currentNetIpBuf), "%s", WiFi.localIP().toString().c_str());
      xSemaphoreGive(cloudMutex);
    }
    currentNetMode = NET_ROUTER;
    startGlassesMdns();
    pendingUdpRebind = true;
    pendingNetBroadcast = true;
    pendingLanAnnounce = true;
    syncTelemetryToCloud();
    Serial.printf("[WIFI] ĐÃ KHÔI PHỤC ROUTER! IP: http://%s\n", WiFi.localIP().toString().c_str());
    return true;
  }
  return false;
}

void cloudSyncTask(void * parameter) {
  unsigned long routerBootStartMs = millis();
  unsigned long routerLostStartMs = 0;
  unsigned long lastRecoveryProbeMs = 0;

  Serial.println("[WIFI] [Ưu tiên 1] Kính IR đang kết nối vào Router...");
  while (millis() - routerBootStartMs < 12000) {
    if (wifiMulti.run(4000) == WL_CONNECTED) {
      if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        snprintf(currentNetIpBuf, sizeof(currentNetIpBuf), "%s", WiFi.localIP().toString().c_str());
        xSemaphoreGive(cloudMutex);
      }
      currentNetMode = NET_ROUTER;
      startGlassesMdns();
      pendingUdpRebind = true;
      pendingNetBroadcast = true;
      pendingLanAnnounce = true;
      syncTelemetryToCloud();
      Serial.printf("[WIFI] ĐÃ VÀO ROUTER (%s)! Web: http://%s\n",
                    WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  if (currentNetMode != NET_ROUTER) {
    activateLocalApFallback();
    lastRecoveryProbeMs = millis();
  }

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(100));
    unsigned long now = millis();

    // CHẾ ĐỘ 1: ĐANG CHẠY LOCAL AP (ROUTER HỎNG)
    if (currentNetMode == NET_LOCAL_AP) {
      bool manualReq = forceRouterRetry;
      bool idleAutoProbe = (activeWsClientCount == 0) && (now - lastRecoveryProbeMs >= 90000UL);

      if (manualReq || idleAutoProbe) {
        forceRouterRetry = false;
        lastRecoveryProbeMs = now;
        probeRouterRecovery();
      }
      continue;
    }

    // CHẾ ĐỘ 2: ĐANG CHẠY ROUTER -> GIÁM SÁT RỚT MẠNG & FAILOVER
    if (WiFi.status() != WL_CONNECTED) {
      if (routerLostStartMs == 0) {
        routerLostStartMs = now;
        Serial.println("[WIFI] Mất sóng Router! Đang thử kết nối lại trong 8 giây...");
      }

      if (wifiMulti.run(3000) == WL_CONNECTED) {
        routerLostStartMs = 0;
        if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
          snprintf(currentNetIpBuf, sizeof(currentNetIpBuf), "%s", WiFi.localIP().toString().c_str());
          xSemaphoreGive(cloudMutex);
        }
        startGlassesMdns();
        pendingUdpRebind = true;
        pendingNetBroadcast = true;
        pendingLanAnnounce = true;
        syncTelemetryToCloud();
        Serial.printf("[WIFI] Đã kết nối lại Router: http://%s\n", WiFi.localIP().toString().c_str());
      } else if (now - routerLostStartMs >= 8000) {
        routerLostStartMs = 0;
        activateLocalApFallback();
        lastRecoveryProbeMs = now;
      }
      continue;
    } else {
      routerLostStartMs = 0;

      if ((WiFi.getMode() == WIFI_AP_STA) && (WiFi.softAPgetStationNum() == 0)) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        pendingUdpRebind = true;
        Serial.println("[WIFI] Không còn client trên AP -> Chuyển thuần WIFI_STA tối ưu sóng.");
      }
    }

    // ĐỒNG BỘ CLOUD (CHỈ CHẠY KHI CÓ ROUTER)
    String localCmd = "";
    char localSentence[200] = "";
    bool hasSentence = false;

    if (cloudMutex && xSemaphoreTake(cloudMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      if (pendingCloudCommand.length() > 0) {
        localCmd = pendingCloudCommand;
        pendingCloudCommand = "";
      }
      if (cloudLogCount > 0) {
        strncpy(localSentence, cloudLogQueue[cloudLogHead].sentence, sizeof(localSentence) - 1);
        localSentence[sizeof(localSentence) - 1] = '\0';
        cloudLogHead = (cloudLogHead + 1) % CLOUD_LOG_QUEUE_SIZE;
        cloudLogCount--;
        hasSentence = true;
      }
      xSemaphoreGive(cloudMutex);
    }

    // 1. Gửi lệnh xóa/ACK về Cloud nếu có (ví dụ mở khóa tại chỗ -> IDLE)
    if (localCmd.length() > 0) {
      HTTPClient cmdPost;
      cmdPost.begin(secureClient, CLOUD_COMMAND_URL);
      cmdPost.addHeader("Content-Type", "application/json");
      cmdPost.setTimeout(2500);
      char cmdJson[96];
      char safeCmd[64];
      sanitizeJsonString(localCmd, safeCmd, sizeof(safeCmd));
      snprintf(cmdJson, sizeof(cmdJson), "{\"command\":\"%s\"}", safeCmd);
      cmdPost.POST((uint8_t*)cmdJson, strlen(cmdJson));
      cmdPost.end();
    }

    // 2. Gửi câu giao tiếp / cảnh báo từ Ring Buffer
    if (hasSentence) {
      HTTPClient http;
      http.begin(secureClient, CLOUD_SERVER_URL);
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(2500);

      char safeSentence[240];
      char jsonPayload[380];
      sanitizeJsonString(String(localSentence), safeSentence, sizeof(safeSentence));
      snprintf(jsonPayload, sizeof(jsonPayload),
               "{\"sentence\":\"%s\",\"attention\":%d,\"meditation\":%d,\"signalQuality\":%d,\"cfi\":%d}",
               safeSentence, (int)attention, (int)meditation, (int)signalQuality, (int)cfiScore);
      http.POST((uint8_t*)jsonPayload, strlen(jsonPayload));
      http.end();
    }

    // 3. Đẩy Telemetry (mỗi 3s khi thức / 15s khi ngủ / tức thì khi có sự kiện)
    //    VÀ nhận luôn lệnh điều khiển từ xa trong JSON response (cắt giảm ~80% bắt tay TLS!)
    if (pendingTelemetry) {
      pendingTelemetry = false;
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

      const char* inModeStr = (currentInputMode == INPUT_IR_ONLY)  ? "IR_ONLY" :
                              (currentInputMode == INPUT_NOD_ONLY) ? "NOD_ONLY" : "HYBRID";
      const char* nodModeStr = (currentNodMode == NOD_MODE_LIGHT) ? "LIGHT" :
                               (currentNodMode == NOD_MODE_HEAVY) ? "HEAVY" : "DUAL";
      const char* netModeStr = (currentNetMode == NET_ROUTER)   ? "ROUTER" :
                               (currentNetMode == NET_LOCAL_AP) ? "LOCAL_AP" : "CONNECTING";
      const char* pwrSrcStr  = (USE_INA226 || USE_BATTERY_ADC) ? "LIPO" : "USB_5V";

      char payload[768];
      snprintf(payload, sizeof(payload),
               "{\"isSleeping\":%s,\"isControlLocked\":%s,\"signalQuality\":%d,"
               "\"isFatigueLocked\":%s,\"fatigueSecLeft\":%lu,\"cfi\":%d,"
               "\"temperature\":%.1f,\"pitch\":%.1f,\"roll\":%.1f,"
               "\"battery\":%d,\"voltage\":%.2f,\"current\":%.1f,\"power\":%.1f,\"shuntVoltage\":%.2f,"
               "\"powerSource\":\"%s\","
               "\"attention\":%d,\"meditation\":%d,\"delta\":%lu,\"theta\":%lu,\"irDelta\":%.1f,"
               "\"inputMode\":\"%s\",\"nodMode\":\"%s\","
               "\"relay1\":%s,\"relay2\":%s,"
               "\"gatewayOnline\":%s,\"netMode\":\"%s\",\"staticMinutes\":%u,"
               "\"uptime\":%lu}",
               isSleeping ? "true" : "false",
               isControlLocked ? "true" : "false",
               (int)signalQuality,
               isFatigueLocked ? "true" : "false",
               fatigueSecLeft,
               (int)cfiScore,
               sensorTempC, compPitchAngle, compRollAngle,
               batteryPercent, inaBusVoltage, inaCurrent_mA, inaPower_mW, inaShunt_mV,
               pwrSrcStr,
               (int)attention, (int)meditation, delta, theta, (float)lastIrDelta,
               inModeStr, nodModeStr,
               relay1State ? "true" : "false",
               relay2State ? "true" : "false",
               gatewayOnline ? "true" : "false",
               netModeStr,
               (unsigned int)staticPostureMinutes,
               millis() / 1000UL);

      int code = http.POST((uint8_t*)payload, strlen(payload));
      if (code == 200) {
        String resp = http.getString();
        parseTelemetryResponseForCmd(resp);
      }
      http.end();
    }
  }
}

// =========================================================================
// XỬ LÝ SỰ KIỆN WEBSOCKET (CORE 1)
// =========================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      activeWsClientCount = webSocket.connectedClients();
      Serial.printf("[WS] Client #%u Disconnected (Còn: %u)\n", num, activeWsClientCount);
      break;

    case WStype_CONNECTED: {
      activeWsClientCount = webSocket.connectedClients();
      if (isFatigueLocked) {
        unsigned long elapsed = millis() - fatigueLockStartTime;
        unsigned long secLeft = (elapsed < fatigueLockDurationMs) ? (fatigueLockDurationMs - elapsed) / 1000 : 0;
        webSocket.sendTXT(num, "FATIGUE_LOCK:" + String(secLeft) + "," + fatigueLockSource);
      }
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[WS] Client #%u Connected from %s\n", num, ip.toString().c_str());

      char buf[80];
      snprintf(buf, sizeof(buf), "RELAY:%u,%u", relay1State ? 1 : 0, relay2State ? 1 : 0);
      webSocket.sendTXT(num, buf);

      snprintf(buf, sizeof(buf), "TGAM:%d,%d,%d,%d,%.1f",
               (int)signalQuality, (int)attention, (int)meditation, (int)cfiScore, sensorTempC);
      webSocket.sendTXT(num, buf);

      snprintf(buf, sizeof(buf), "INA:%.2f,%.1f,%.1f,%d,%d",
               inaBusVoltage, inaCurrent_mA, inaPower_mW, batteryPercent, hasINA226 ? 1 : 0);
      webSocket.sendTXT(num, buf);

      const char* modeStr = (currentNodMode == NOD_MODE_LIGHT) ? "MODE:LIGHT" :
                            (currentNodMode == NOD_MODE_HEAVY) ? "MODE:HEAVY" : "MODE:DUAL";
      webSocket.sendTXT(num, modeStr);

      const char* inModeStr = (currentInputMode == INPUT_IR_ONLY)  ? "INPUT_MODE:IR_ONLY" :
                              (currentInputMode == INPUT_NOD_ONLY) ? "INPUT_MODE:NOD_ONLY" : "INPUT_MODE:HYBRID";
      webSocket.sendTXT(num, inModeStr);

      webSocket.sendTXT(num, isControlLocked ? "LOCK_CONTROL:ON" : "LOCK_CONTROL:OFF");
      if (isSleeping) webSocket.sendTXT(num, "SLEEPING");

      // Chỉ gửi riêng cho client vừa kết nối thay vì broadcast toàn mạng
      sendNetStatusTo(num);
      sendGatewayStatusTo(num);
      break;
    }

    case WStype_TEXT: {
      String msg;
      msg.reserve(length);
      for (size_t i = 0; i < length; i++) msg += (char)payload[i];
      lastActivityTime = millis();

      if (msg == "S") {
        isSleeping = true;
        if (currentNetMode == NET_ROUTER && WiFi.getMode() == WIFI_STA) WiFi.setSleep(true);
        webSocket.broadcastTXT("SLEEPING");
        syncTelemetryToCloud();
      }
      else if (msg == "W") {
        isSleeping = false;
        WiFi.setSleep(false);
        webSocket.broadcastTXT("WAKEUP");
        syncTelemetryToCloud();
      }
      else if (msg == "TOGGLE_CONTROL_LOCK") setControlLock(!isControlLocked);
      else if (msg == "LOCK_CONTROL_ON")     setControlLock(true);
      else if (msg == "LOCK_CONTROL_OFF")    setControlLock(false);
      else if (msg == "MODE_LIGHT")          setNodMode(NOD_MODE_LIGHT);
      else if (msg == "MODE_HEAVY")          setNodMode(NOD_MODE_HEAVY);
      else if (msg == "MODE_DUAL")           setNodMode(NOD_MODE_DUAL);
      else if (msg == "INPUT_HYBRID")        setInputMode(INPUT_HYBRID_BOTH);
      else if (msg == "INPUT_IR")            setInputMode(INPUT_IR_ONLY);
      else if (msg == "INPUT_NOD")           setInputMode(INPUT_NOD_ONLY);
      else if (msg == "RECONNECT_ROUTER") {
        forceRouterRetry = true;
        Serial.println("[WIFI] Nhận yêu cầu thử kết nối lại Router từ Web UI.");
      }
      else if (msg == "PING_GATEWAY") {
        announceGlassesOnLan();
        sendGatewayStatusTo(num);
      }
      else if (msg == "R1") {
        applyRelayLocalAndGateway(1, !relay1State, true);
      }
      else if (msg == "R2") {
        applyRelayLocalAndGateway(2, !relay2State, true);
      }
      else if (msg.startsWith("SENTENCE:")) {
        String sentence = msg.substring(9);
        Serial.println("[SENTENCE] Bệnh nhân giao tiếp: " + sentence);
        syncSentenceToCloud(sentence);
      }
      else if (msg == "SOS") {
        triggerBuzzer(3);
        Serial.println("[ALERT] BỆNH NHÂN GỌI CẤP CỨU (SOS)!");
        enqueueCloudSentence("SOS - CẤP CỨU KHẨN CẤP", true);
        syncTelemetryToCloud();
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
        syncCommandToCloud("IDLE");
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
// SETUP (KHỞI ĐỘNG TỨC THÌ - WI-FI & FAILOVER CHẠY TRÊN CORE 0)
// =========================================================================
void setup() {
  cloudMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(rgbRainbowTask, "RGBRainbowTask", 2048, NULL, 1, NULL, 1);

  Serial.begin(115200);
  delay(800);

  Serial.println("\n\n==========================================================");
  Serial.println("  ESP32-S3 PRO-CAD V8.2 - KÍNH IR & GATEWAY FAILOVER     ");
  Serial.println("  CHẾ ĐỘ: ROUTER ƯU TIÊN -> TỰ ĐỘNG LOCAL AP KHI HỎNG    ");
  Serial.println("==========================================================");

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(IR_LED_PULSE_PIN, OUTPUT);
  digitalWrite(IR_LED_PULSE_PIN, HIGH);
  pinMode(IR_SENSOR_ADC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(IR_SENSOR_ADC_PIN, ADC_11db);

  triggerBuzzer(2);

  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 400000);
  Wire.setTimeOut(25);
  delay(20);

  initLSM6DS3();
  initPowerMonitor();
  readPowerMonitor();

  secureClient.setInsecure();

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);
  wifiMulti.addAP(WIFI_SSID_3, WIFI_PASS_3);

  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  server.on("/", HTTP_GET, handleRoot);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(15000, 10000, 2);
  gwUdp.begin(GW_UDP_PORT);

  xTaskCreatePinnedToCore(cloudSyncTask, "CloudNetTask", 8192, NULL, 1, NULL, 0);

  lastActivityTime = millis();
  lastTelemetrySyncTime = millis();
  Serial.println("[SYSTEM] Sẵn sàng! Serial: 'd'=IR Debug, 'r'=Tìm Router, 'g'=Xem Gateway, '7'/'8'=Test Đèn/Quạt.");
}

// =========================================================================
// LOOP (CORE 1 - 100% NON-BLOCKING CHO CẢM BIẾN 200Hz, UDP GATEWAY & WS)
// =========================================================================
void loop() {
  webSocket.loop();
  server.handleClient();

  processLSM6DS3();
  processTCRT5000();
  processGatewayUdp();
  updateBuzzer();

  // Xử lý toàn bộ lệnh điều khiển từ xa Cloud -> Core 1 (Thread-safe 100%)
  CloudCmdAction cloudAct = CLOUD_CMD_NONE;
  unsigned long cloudDur = 0;
  while (dequeueCloudAction(cloudAct, cloudDur)) {
    switch (cloudAct) {
      case CLOUD_CMD_LOCK_REST:
        if (!isFatigueLocked && millis() > ignoreLockUntil) {
          triggerLocalFatigueLock(cloudDur > 0 ? cloudDur : 1800UL, "GLOBAL");
        }
        break;
      case CLOUD_CMD_UNLOCK_REST:
        if (isFatigueLocked) releaseLocalFatigueLock();
        break;
      case CLOUD_CMD_LOCK_CONTROL:
        setControlLock(true);
        break;
      case CLOUD_CMD_UNLOCK_CONTROL:
        setControlLock(false);
        break;
      case CLOUD_CMD_INPUT_HYBRID:
        setInputMode(INPUT_HYBRID_BOTH);
        break;
      case CLOUD_CMD_INPUT_IR:
        setInputMode(INPUT_IR_ONLY);
        break;
      case CLOUD_CMD_INPUT_NOD:
        setInputMode(INPUT_NOD_ONLY);
        break;
      case CLOUD_CMD_R1_ON:
        applyRelayLocalAndGateway(1, true, true);
        break;
      case CLOUD_CMD_R1_OFF:
        applyRelayLocalAndGateway(1, false, true);
        break;
      case CLOUD_CMD_R2_ON:
        applyRelayLocalAndGateway(2, true, true);
        break;
      case CLOUD_CMD_R2_OFF:
        applyRelayLocalAndGateway(2, false, true);
        break;
      default:
        break;
    }
  }

  // Rebind UDP socket an toàn trên Core 1 mỗi khi chuyển đổi Router <-> Local AP
  if (pendingUdpRebind) {
    pendingUdpRebind = false;
    gwUdp.stop();
    gwUdp.begin(GW_UDP_PORT);
    Serial.println("[UDP] Đã rebind socket UDP 4210 sau khi chuyển chế độ mạng.");
  }

  if (pendingNetBroadcast) {
    pendingNetBroadcast = false;
    gatewayOnline = false;
    gatewayIP = IPAddress(0, 0, 0, 0);
    broadcastNetStatus();
    broadcastGatewayStatus();
  }

  if (pendingLanAnnounce) {
    pendingLanAnnounce = false;
    announceGlassesOnLan();
  }

  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') setNodMode(NOD_MODE_LIGHT);
    else if (c == '2') setNodMode(NOD_MODE_HEAVY);
    else if (c == '3') setNodMode(NOD_MODE_DUAL);
    else if (c == 'l' || c == 'L') setControlLock(!isControlLocked);
    else if (c == 'i' || c == 'I') setInputMode(INPUT_IR_ONLY);
    else if (c == 'n' || c == 'N') setInputMode(INPUT_NOD_ONLY);
    else if (c == 'h' || c == 'H') setInputMode(INPUT_HYBRID_BOTH);
    else if (c == '4') triggerBuzzer(1);
    else if (c == '5') triggerBuzzer(2);
    else if (c == '6') triggerBuzzer(3);
    else if (c == '7') applyRelayLocalAndGateway(1, !relay1State, true);
    else if (c == '8') applyRelayLocalAndGateway(2, !relay2State, true);
    else if (c == 'r' || c == 'R') {
      forceRouterRetry = true;
      Serial.println("[WIFI] Đã yêu cầu dò lại Router ngay lập tức!");
    }
    else if (c == 'g' || c == 'G') {
      Serial.printf("[NET/GW] NetMode=%u | IP=%s | Gateway=%s (%s) | R1=%u R2=%u | Static=%um\n",
                    (uint8_t)currentNetMode, currentNetIpBuf,
                    gatewayOnline ? "ONLINE" : "OFFLINE",
                    gatewayIP.toString().c_str(),
                    relay1State ? 1 : 0, relay2State ? 1 : 0,
                    (unsigned int)staticPostureMinutes);
    }
    else if (c == 'd' || c == 'D') {
      enableIrDebug = !enableIrDebug;
      Serial.printf("[IR DEBUG] Khung debug hồng ngoại trên Web: %s\n", enableIrDebug ? "BẬT" : "TẮT");
      if (!enableIrDebug) webSocket.broadcastTXT("IR_DEBUG:OFF");
    }
  }

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

  static unsigned long lastSensorBroadcast = 0;
  if (millis() - lastSensorBroadcast > 1000) {
    lastSensorBroadcast = millis();
    readPowerMonitor();
    activeWsClientCount = webSocket.connectedClients();

    if (!isSleeping && activeWsClientCount > 0) {
      char sensorData[64];
      snprintf(sensorData, sizeof(sensorData), "TGAM:%d,%d,%d,%d,%.1f",
               (int)signalQuality, (int)attention, (int)meditation, (int)cfiScore, sensorTempC);
      webSocket.broadcastTXT(sensorData);

      char inaData[64];
      snprintf(inaData, sizeof(inaData), "INA:%.2f,%.1f,%.1f,%d,%d",
               inaBusVoltage, inaCurrent_mA, inaPower_mW, batteryPercent, hasINA226 ? 1 : 0);
      webSocket.broadcastTXT(inaData);
    }
  }

  if (!isSleeping && (millis() - lastActivityTime >= AUTO_SLEEP_TIMEOUT)) {
    isSleeping = true;
    if (currentNetMode == NET_ROUTER && WiFi.getMode() == WIFI_STA) WiFi.setSleep(true);
    webSocket.broadcastTXT("SLEEPING");
    syncTelemetryToCloud();
    Serial.println(">>> [SMART SLEEP] Không có hoạt động trong 15 phút -> Chuyển sang Sleep Mode! <<<");
  }

  // Nhịp Telemetry thích ứng: 3s khi thức / 15s khi ngủ
  unsigned long teleInterval = isSleeping ? TELEMETRY_INTERVAL_SLEEP : TELEMETRY_INTERVAL_ACTIVE;
  if (millis() - lastTelemetrySyncTime >= teleInterval) {
    lastTelemetrySyncTime = millis();
    syncTelemetryToCloud();
  }

  yield();
}