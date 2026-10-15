#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>

// =========================================================================
// HỆ THỐNG BCI ESP32-S3 - BÀN PHÍM TRỢ NĂNG GIAO TIẾP & ĐIỀU KHIỂN THIẾT BỊ
// KIẾN TRÚC DUAL-LAYER CONTEXT, BỘ GÕ TELEX, MODEM SLEEP & INA226 SENSOR
// =========================================================================

WiFiMulti wifiMulti;
WiFiClientSecure secureClient; 

// 1. Cấu hình Wi-Fi Router
const char* WIFI_SSID_1 = "Trong Nghia";     
const char* WIFI_PASS_1 = "109876543"; 
const char* WIFI_SSID_2 = "Tuyet Nghia";     
const char* WIFI_PASS_2 = "@tuyet1003@";      

const char* AP_SSID = "ESP32_BCI_SYSTEM";
const char* AP_PASS = "12345678";

// 2. ĐỊA CHỈ WEB SERVER CLOUD TRÊN RENDER
const char* CLOUD_SERVER_URL = "https://bci-api-server.onrender.com/api/logs"; 
const char* CLOUD_TELEMETRY_URL = "https://bci-api-server.onrender.com/api/telemetry";

// 3. CẤU HÌNH CHÂN PHẦN CỨNG TƯƠNG THÍCH HOÀN TOÀN ESP32-S3
// UART1: RX/TX cho module cảm biến sóng não NeuroSky TGAM (57600 baud)
#define TGAM_RX_PIN 18
#define TGAM_TX_PIN 17
#define TGAM_BAUDRATE 57600

// I2C: Dành cho cảm biến năng lượng INA226 (ESP32-S3 Default Pins)
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define INA226_I2C_ADDR 0x40

// GPIO Output Relay (Tránh tuyệt đối GPIO 19 & 20 là chân USB D-/D+ của ESP32-S3)
#define RELAY1_PIN 4 
#define RELAY2_PIN 5 

// ADC1 Chân đo điện áp pin (GPIO 1 là ADC1_CH0, không bị Wi-Fi khóa như ADC2)
#define BATTERY_PIN 1 

WebServer server(80);
WebSocketsServer webSocket(81);
HardwareSerial TGAMSerial(1); // Sử dụng UART1 của ESP32-S3

// Trạng thái cảm biến NeuroSky TGAM
volatile int signalQuality = 200; 
volatile int attention = 0;       
volatile int meditation = 0;      
volatile int blinkStrength = 0;   

// Các dải sóng não
unsigned long delta = 0, theta = 0, lowAlpha = 0, highAlpha = 0, lowBeta = 0, highBeta = 0, lowGamma = 0, midGamma = 0;

// Cảm biến năng lượng INA226
bool hasINA226 = false;
float inaBusVoltage = 3.90f;
float inaCurrent_mA = 110.0f;
float inaPower_mW = 429.0f;
float inaShunt_mV = 11.0f;
int batteryPercent = 100;

volatile bool relay1State = false;
volatile bool relay2State = false;

volatile bool isSleeping = false;
volatile int blinkCountForWake = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastActivityTime = 0;
const unsigned long AUTO_SLEEP_TIMEOUT = 900000; 
unsigned long lastTelemetrySyncTime = 0;
const unsigned long TELEMETRY_INTERVAL = 25000;  

uint8_t payloadData[256];

// =========================================================================
// KHỞI TẠO & ĐỌC CẢM BIẾN NĂNG LƯỢNG INA226 (CHỐNG TREO BUS I2C)
// =========================================================================
void initINA226() {
  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
  Wire.setTimeOut(25); 
  delay(10);
  
  Wire.beginTransmission(INA226_I2C_ADDR);
  if (Wire.endTransmission() == 0) {
    hasINA226 = true;
    Serial.println("[INA226] Nhận diện thành công cảm biến INA226 tại 0x40 trên ESP32-S3!");
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
    Serial.println("[INA226] Không có cảm biến INA226, tự động đọc ADC dự phòng qua GPIO 1.");
  }
}

void readINA226() {
  if (hasINA226) {
    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x02);
    if (Wire.endTransmission() == 0) {
      Wire.requestFrom((uint8_t)INA226_I2C_ADDR, (uint8_t)2);
      if (Wire.available() >= 2) {
        int16_t rawBus = (Wire.read() << 8) | Wire.read();
        inaBusVoltage = rawBus * 0.00125f;
      }
    }

    Wire.beginTransmission(INA226_I2C_ADDR);
    Wire.write(0x01);
    if (Wire.endTransmission() == 0) {
      Wire.requestFrom((uint8_t)INA226_I2C_ADDR, (uint8_t)2);
      if (Wire.available() >= 2) {
        int16_t rawShunt = (Wire.read() << 8) | Wire.read();
        inaShunt_mV = rawShunt * 0.0025f;
        inaCurrent_mA = inaShunt_mV * 10.0f;
        if (inaCurrent_mA < 0) inaCurrent_mA = 0;
      }
    }

    inaPower_mW = inaBusVoltage * inaCurrent_mA;

    if (inaBusVoltage >= 4.20f) batteryPercent = 100;
    else if (inaBusVoltage <= 3.30f) batteryPercent = 0;
    else batteryPercent = (int)(((inaBusVoltage - 3.30f) / 0.90f) * 100.0f);
  } else {
    int rawBat = analogRead(BATTERY_PIN);
    batteryPercent = map(rawBat, 0, 4095, 0, 100); 
    if(batteryPercent > 100) batteryPercent = 100;
    if(batteryPercent < 0) batteryPercent = 0;
    inaBusVoltage = (rawBat / 4095.0f) * 3.3f * 2.0f;
    inaCurrent_mA = isSleeping ? 25.0f : 110.0f;
    inaPower_mW = inaBusVoltage * inaCurrent_mA;
  }
}

// =========================================================================
// GIAO DIỆN WEB CỤC BỘ DÀNH CHO BỆNH NHÂN
// =========================================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi" data-theme="light">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Hệ Thống Giao Tiếp BCI ESP32-S3</title>
    <style>
        :root {
            --bg-main: #e0f2fe; 
            --bg-card: rgba(255, 255, 255, 0.85);
            --text-main: #0f172a;
            --text-muted: #475569;
            --primary: #0ea5e9;
            --primary-glow: rgba(14, 165, 233, 0.4);
            --key-bg: #ffffff;
            --key-border: #cbd5e1;
            --key-shadow: rgba(0,0,0,0.08);
            --key-sys-bg: #f1f5f9;
            --grass-top: #4ade80;
            --grass-bottom: #16a34a;
            --cloud-opacity: 0.8;
            --scenery-brightness: 1;
            --success: #10b981;
            --danger: #ef4444;
            --warning: #f59e0b;
            --scan-color: #f59e0b;
        }

        [data-theme="dark"] {
            --bg-main: #0f172a; 
            --bg-card: rgba(30, 41, 59, 0.85);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --primary: #38bdf8;
            --primary-glow: rgba(56, 189, 248, 0.3);
            --key-bg: #1e293b;
            --key-border: #334155;
            --key-shadow: rgba(0,0,0,0.3);
            --key-sys-bg: #0f172a;
            --grass-top: #166534;
            --grass-bottom: #14532d;
            --cloud-opacity: 0.2;
            --scenery-brightness: 0.6;
            --success: #34d399;
            --danger: #f87171;
            --warning: #fbbf24;
            --scan-color: #fbbf24;
        }

        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        body {
            font-family: 'Segoe UI', Quicksand, Roboto, Helvetica, Arial, sans-serif;
            background-color: var(--bg-main);
            color: var(--text-main);
            overflow-x: hidden;
            display: flex; flex-direction: column; align-items: center;
            min-height: 100vh; user-select: none; -webkit-user-select: none;
            transition: background-color 0.5s ease, color 0.5s ease;
        }

        .scenery-layer {
            position: fixed; bottom: 0; left: 0; width: 100%; height: 100vh;
            pointer-events: none; z-index: -1; overflow: hidden;
            filter: brightness(var(--scenery-brightness));
            transition: filter 0.5s ease;
        }
        .grass {
            position: absolute; bottom: 0; left: 0; width: 100%; height: 20vh;
            background: linear-gradient(to bottom, var(--grass-top), var(--grass-bottom));
            border-top-left-radius: 50% 10vh; border-top-right-radius: 50% 10vh;
            transition: background 0.5s ease;
        }
        .cloud {
            position: absolute; background: white; border-radius: 50px;
            opacity: var(--cloud-opacity); animation: float 40s linear infinite;
            transition: opacity 0.5s ease;
        }
        .cloud::before, .cloud::after { content: ''; position: absolute; background: white; border-radius: 50%; }
        .c1 { top: 10%; left: -20%; width: 120px; height: 40px; }
        .c1::before { width: 50px; height: 50px; top: -25px; left: 15px; }
        .c1::after { width: 40px; height: 40px; top: -15px; right: 15px; }
        .c2 { top: 25%; left: -30%; width: 160px; height: 50px; animation-duration: 60s; animation-delay: -10s; transform: scale(0.8);}
        .c2::before { width: 70px; height: 70px; top: -35px; left: 20px; }
        .c2::after { width: 60px; height: 60px; top: -25px; right: 25px; }
        
        @keyframes float { 0% { transform: translateX(-10vw); } 100% { transform: translateX(110vw); } }

        .mascot { position: absolute; font-size: 65px; z-index: 0; }
        .m-dog { 
            left: 15%; bottom: 12vh; display: inline-block; transform-origin: center bottom;
            animation: dog-3d-action 3s infinite ease-in-out; filter: drop-shadow(0px 15px 8px rgba(0,0,0,0.3));
        }
        @keyframes dog-3d-action {
            0%, 100% { transform: perspective(500px) rotateY(0deg) rotateX(0deg) translateY(0) scale(1); }
            25% { transform: perspective(500px) rotateY(35deg) rotateX(10deg) translateY(-20px) scale(1.1); }
            50% { transform: perspective(500px) rotateY(0deg) rotateX(0deg) translateY(0) scale(1); }
            75% { transform: perspective(500px) rotateY(-35deg) rotateX(10deg) translateY(-20px) scale(1.1); }
        }
        .m-cat { right: 15%; bottom: 14vh; animation: cat-breathe 4s infinite ease-in-out; transform-origin: bottom center; }
        @keyframes cat-breathe { 0%, 100% { transform: scale(1) rotate(0deg); } 50% { transform: scale(1.05) rotate(2deg); } }

        button, .cell { touch-action: manipulation; cursor: pointer; border: none; outline: none; font-family: inherit; }

        .top-bar {
            position: fixed; top: 20px; left: 0; width: 100%; padding: 0 30px;
            display: flex; justify-content: space-between; align-items: flex-start; z-index: 100; pointer-events: none;
        }
        .widget-pill {
            background: var(--bg-card); backdrop-filter: blur(10px);
            border: 2px solid var(--key-border); padding: 10px 20px; border-radius: 30px;
            box-shadow: 0 8px 25px var(--key-shadow); pointer-events: auto; display: flex; align-items: center; gap: 15px;
        }
        #clock-time { font-size: 22px; font-weight: 900; color: var(--primary); }
        #clock-date { font-size: 12px; color: var(--text-muted); font-weight: 700; }
        .theme-btn { background: transparent; font-size: 24px; cursor: pointer; transition: transform 0.2s; padding: 5px; }
        .theme-btn:active { transform: scale(0.8); }

        #view-menu {
            display: flex; flex-direction: column; align-items: center; justify-content: center;
            width: 100%; min-height: 100vh; padding: 20px; max-width: 1000px; z-index: 10;
        }
        .page-title { font-size: 36px; font-weight: 900; margin-bottom: 40px; text-align: center; }
        .dash-row { display: flex; gap: 24px; width: 100%; margin-bottom: 30px; }
        .dash-card {
            background: var(--bg-card); backdrop-filter: blur(10px); border: 2px solid var(--key-border); border-radius: 32px;
            padding: 30px 20px; flex: 1; text-align: center; box-shadow: 0 15px 35px var(--key-shadow); transition: all 0.3s;
        }
        .dash-title { font-size: 15px; color: var(--text-muted); font-weight: 800; margin-bottom: 12px; text-transform: uppercase; }
        .dash-value { font-size: 52px; font-weight: 900; color: var(--primary); line-height: 1; }
        .dash-value.good { color: var(--success); }
        .dash-value.bad { color: var(--danger); font-size: 36px; margin-top: 8px;}

        .menu-actions { display: flex; flex-direction: column; gap: 16px; width: 100%; max-width: 700px; }
        .btn-main {
            background: var(--primary); color: #fff; padding: 22px; font-size: 22px; font-weight: 800;
            border-radius: 28px; box-shadow: 0 10px 25px var(--primary-glow); transition: all 0.2s;
        }
        .btn-main:active { transform: scale(0.97); }
        .btn-reader-menu {
            background: linear-gradient(135deg, #059669, #10b981); color: #fff; padding: 22px; font-size: 22px; font-weight: 800;
            border-radius: 28px; box-shadow: 0 10px 25px rgba(16, 185, 129, 0.4); transition: all 0.2s;
        }
        .btn-reader-menu:active { transform: scale(0.97); }
        .sub-actions-row { display: flex; gap: 16px; width: 100%; }
        .btn-sub {
            background: var(--bg-card); border: 2px solid var(--key-border); color: var(--text-main); padding: 18px; font-size: 16px; font-weight: 800;
            border-radius: 20px; flex: 1; transition: all 0.2s; display: flex; flex-direction: column; align-items: center; gap: 8px;
            box-shadow: 0 8px 20px var(--key-shadow);
        }
        .btn-sub:active { transform: scale(0.96); background: var(--key-sys-bg); }
        .btn-sub span { color: var(--primary); font-size: 14px; font-weight: 700; background: var(--key-sys-bg); padding: 4px 12px; border-radius: 12px;}

        #view-keyboard {
            display: none; flex-direction: column; width: 100%; height: 100vh;
            max-width: 1300px; margin: 0 auto; padding: 15px; z-index: 10;
        }
        #top-controls { width: 100%; margin: 90px auto 10px auto; display: flex; justify-content: space-between; align-items: center; padding: 0 10px; }
        .top-btn {
            background: var(--bg-card); color: var(--text-main); padding: 14px 24px; border-radius: 30px; font-size: 16px; font-weight: 800;
            border: 2px solid var(--key-border); box-shadow: 0 6px 15px var(--key-shadow);
        }
        .helper-text { color: var(--text-main); font-size: 15px; font-weight: 800; background: var(--bg-card); padding: 10px 20px; border-radius: 20px; border: 2px solid var(--key-border);}
        #output-box {
            width: calc(100% - 20px); min-height: 85px; margin: 10px auto 15px auto; padding: 15px 30px;
            background: var(--bg-card); border-radius: 32px; border: 4px solid var(--primary); color: var(--text-main);
            font-size: 40px; font-weight: 900; display: flex; align-items: center; justify-content: center;
            box-shadow: 0 10px 25px var(--primary-glow); text-align: center; word-break: break-word;
        }
        .grid { display: flex; flex-direction: column; gap: 12px; width: 100%; padding: 0 10px; }
        .row { display: flex; gap: 12px; justify-content: center; }
        .cell {
            flex: 1; padding: 22px 5px; background: var(--key-bg); border: 2px solid var(--key-border);
            border-radius: 20px; font-size: 32px; font-weight: 900; color: var(--text-main);
            display: flex; justify-content: center; align-items: center; box-shadow: 0 6px 12px var(--key-shadow); transition: all 0.15s ease;
        }
        .cell.suggest { background-color: rgba(16, 185, 129, 0.15); border-color: var(--success); color: var(--success); font-size: 26px; }
        .cell.sys { background-color: var(--key-sys-bg); font-size: 20px; color: var(--text-muted); }
        .cell.iot { background-color: rgba(245, 158, 11, 0.15); border-color: var(--warning); color: var(--warning); font-size: 20px; }
        .cell.danger { background-color: rgba(239, 68, 68, 0.15); border-color: var(--danger); color: var(--danger); font-size: 26px; }
        .cell.action { background-color: var(--primary); border-color: var(--primary); color: #fff; font-size: 22px; box-shadow: 0 6px 20px var(--primary-glow);}

        #view-reader {
            display: none; flex-direction: column; width: 100%; min-height: 100vh;
            max-width: 1000px; margin: 0 auto; padding: 95px 20px 40px 20px; z-index: 10;
        }
        .reader-nav {
            display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap;
            gap: 12px; margin-bottom: 24px; background: var(--bg-card); padding: 16px 24px;
            border-radius: 24px; border: 2px solid var(--key-border); box-shadow: 0 8px 25px var(--key-shadow);
        }
        .reader-nav h2 { font-size: 22px; font-weight: 900; color: var(--text-main); }
        .reader-nav p { font-size: 13px; color: var(--primary); font-weight: 700; margin-top: 4px; }
        .reader-controls { display: flex; gap: 10px; align-items: center; }
        .btn-reader-action {
            padding: 10px 18px; background: var(--key-sys-bg); border: 2px solid var(--key-border);
            color: var(--text-main); border-radius: 16px; font-size: 14px; font-weight: 800; transition: 0.2s;
        }
        .btn-reader-action.active { background: #059669; border-color: var(--success); color: #fff; }
        .reader-viewport {
            background: var(--bg-card); border: 2px solid var(--key-border); border-radius: 28px;
            padding: 40px 45px; line-height: 2.2; font-size: 28px; font-weight: 600;
            box-shadow: 0 15px 35px var(--key-shadow); position: relative; margin-bottom: 25px;
        }
        .reader-viewport p { margin-bottom: 26px; text-align: justify; }
        .reader-guide-box {
            background: var(--bg-card); border: 2px solid var(--primary); border-radius: 24px; padding: 18px 25px; font-size: 15px; font-weight: 800;
            text-align: center; color: var(--text-main); line-height: 1.8; box-shadow: 0 8px 25px var(--primary-glow);
        }

        .scan-group {
            border: 4px solid transparent;
            border-radius: 24px;
            transition: all 0.2s ease;
            padding: 6px;
        }
        .scanning-row { 
            border-color: var(--scan-color) !important; 
            background-color: rgba(245, 158, 11, 0.1) !important; 
            box-shadow: 0 0 20px rgba(245, 158, 11, 0.2) !important;
        }
        .scanning-cell {
            background-color: var(--scan-color) !important;
            color: #fff !important;
            transform: scale(1.05) translateY(-3px) !important;
            border-color: var(--scan-color) !important;
            box-shadow: 0 15px 30px rgba(245, 158, 11, 0.6) !important;
            z-index: 10;
        }

        .overlay {
            position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: var(--bg-main); z-index: 1000; opacity: 0.95;
            display: flex; flex-direction: column; align-items: center; justify-content: center; display: none; backdrop-filter: blur(10px);
        }
        .sleep-icon { font-size: 100px; margin-bottom: 20px; animation: cat-breathe 3s infinite;}
        .sleep-text { font-size: 45px; font-weight: 900; color: var(--text-main); margin-bottom: 20px; text-align: center;}
        .wake-hint { font-size: 24px; color: var(--success); font-weight: 800; animation: pulse 2s infinite; background: rgba(16,185,129,0.1); padding: 20px 40px; border-radius: 40px; border: 3px solid var(--success);}
        @keyframes pulse { 0% { opacity: 0.8; transform: scale(0.98);} 50% { opacity: 1; transform: scale(1.02);} 100% { opacity: 0.8; transform: scale(0.98);} }

        #settings-panel {
            background: var(--bg-card); border: 4px solid var(--primary); border-radius: 32px;
            padding: 40px; width: 90%; max-width: 550px; box-shadow: 0 25px 50px rgba(0,0,0,0.3);
        }
        .set-title { font-size: 28px; color: var(--text-main); font-weight: 900; margin-bottom: 30px; text-align: center; }
        .set-group { margin-bottom: 24px; }
        .set-label { font-size: 18px; color: var(--text-main); font-weight: 800; margin-bottom: 12px; display: block; }
        .set-select, .set-slider { width: 100%; padding: 16px; font-size: 18px; font-weight: 700; background: var(--key-sys-bg); color: var(--text-main); border: 2px solid var(--key-border); border-radius: 16px; outline: none; }
        .set-val { float: right; color: var(--primary); }
        .set-btns { display: flex; gap: 16px; margin-top: 40px; }
        .set-btns button { flex: 1; padding: 20px; border-radius: 20px; font-size: 18px; font-weight: 800; color: #fff; transition: 0.2s;}
        .btn-test { background: var(--key-sys-bg); color: var(--text-main) !important; border: 2px solid var(--key-border);}
        .btn-close { background: var(--primary); box-shadow: 0 6px 20px var(--primary-glow);}

        #status-bar {
            position: fixed; bottom: 0; left: 0; width: 100%; padding: 12px; background: var(--bg-card); font-size: 15px; color: var(--text-main); 
            text-align: center; z-index: 90; font-weight: 800; box-shadow: 0 -5px 15px var(--key-shadow);
        }
        #blink-counter {
            position: fixed; top: 90px; right: 30px; background: var(--bg-card); color: var(--primary); border: 3px solid var(--primary);
            padding: 16px 32px; border-radius: 40px; font-size: 26px; font-weight: 900; z-index: 100; display: none; box-shadow: 0 10px 25px var(--primary-glow);
        }
        .toast {
            position: fixed; top: 40px; left: 50%; transform: translateX(-50%); background: var(--danger); color: white; padding: 20px 45px; border-radius: 40px;
            font-size: 24px; font-weight: 900; z-index: 9999; display: none; box-shadow: 0 15px 40px rgba(239, 68, 68, 0.4); border: 4px solid white;
        }
    </style>
</head>
<body>
    <div class="scenery-layer">
        <div class="cloud c1"></div>
        <div class="cloud c2"></div>
        <div class="grass"></div>
        <div class="mascot m-cat">🐈</div>
        <div class="mascot m-dog">🐕</div>
    </div>

    <div class="top-bar">
        <div class="widget-pill" id="clock">
            <div>
                <div id="clock-time">--:--</div>
                <div id="clock-date">--/--/----</div>
            </div>
            <button class="theme-btn" onclick="toggleTheme()" id="theme-icon">🌓</button>
        </div>
        <div class="widget-pill" id="tgam-status-mini" style="display: none;">
            <span style="font-size: 14px; font-weight: 700; color: var(--text-muted)">
                Sóng não: <span id="sig-mini" style="color:var(--danger); font-weight: 900">200</span> &nbsp;|&nbsp; 
                Tập trung: <span id="att-mini" style="color:var(--text-main); font-weight: 900">0</span> &nbsp;|&nbsp; 
                Pin: <span id="bat-mini" style="color:var(--success); font-weight: 900">100%</span>
            </span>
        </div>
    </div>

    <div id="blink-counter"> Nháy mắt: 1</div>
    <div id="toast-alert" class="toast"></div>

    <div id="sleep-overlay" class="overlay">
        <div class="sleep-icon"></div>
        <div class="sleep-text" id="sleep-msg">Hệ thống ESP32-S3 đang nghỉ ngơi</div>
        <div class="wake-hint"> Nháy mắt 4 lần liên tiếp để thức dậy</div>
    </div>

    <div id="settings-overlay" class="overlay">
        <div id="settings-panel">
            <div class="set-title">Cài Đặt Giọng Đọc</div>
            <div class="set-group">
                <label class="set-label">Chọn giọng đọc:</label>
                <select id="voice-select" class="set-select"></select>
            </div>
            <div class="set-group">
                <label class="set-label">Tốc độ đọc: <span id="rate-val" class="set-val">0.85</span></label>
                <input type="range" id="rate-slider" class="set-slider" min="0.1" max="1.5" step="0.05" value="0.85">
            </div>
            <div class="set-group">
                <label class="set-label">Âm lượng: <span id="vol-val" class="set-val">100%</span></label>
                <input type="range" id="vol-slider" class="set-slider" min="0" max="1" step="0.1" value="1.0">
            </div>
            <div class="set-btns">
                <button class="btn-test" onclick="testVoice()">Nghe Thử</button>
                <button class="btn-close" onclick="closeSettings()">Lưu & Đóng</button>
            </div>
        </div>
    </div>

    <!-- VIEW 1: MAIN MENU -->
    <div id="view-menu">
        <h1 class="page-title">Bảng Theo Dõi Sức Khỏe </h1>

        <div class="dash-row">
            <div class="dash-card">
                <div class="dash-title">Sóng Não (Kết Nối)</div>
                <div class="dash-value bad" id="sig-val">CHƯA GẮN</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Độ Tập Trung</div>
                <div class="dash-value" id="att-val">0</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Năng Lượng Pin (INA226)</div>
                <div class="dash-value" id="bat-val" style="color: var(--success)">100%</div>
            </div>
        </div>

        <div class="menu-actions">
            <div class="sub-actions-row scan-group">
                <button class="btn-main scan-item" style="flex: 1; padding: 22px 10px; font-size: 20px;" onclick="showQuick()"> Giao Tiếp Nhanh</button>
                <button class="btn-main scan-item" style="flex: 1; padding: 22px 10px; font-size: 20px; background: var(--warning);" onclick="showKeyboard()"> Gõ Bàn Phím</button>
            </div>
            <div class="scan-group" style="width: 100%; display: flex;">
                <button class="btn-reader-menu scan-item" style="width: 100%" onclick="showReader()">Đọc Sách & Báo Giải Trí </button>
            </div>
            <div class="sub-actions-row scan-group">
                <button class="btn-sub scan-item" onclick="openSettings()"> Cài Đặt Giọng</button>
                <button class="btn-sub scan-item" onclick="toggleRelay(1)"> Đèn Phòng <span id="menu-r1">Đang Tắt</span></button>
                <button class="btn-sub scan-item" onclick="toggleRelay(2)"> Quạt Máy <span id="menu-r2">Đang Tắt</span></button>
            </div>
        </div>
    </div>

    <!-- VIEW 2: KEYBOARD -->
    <div id="view-keyboard">
        <div id="top-controls" class="scan-group">
            <button class="top-btn scan-item" onclick="showMenu()"> Trở Về Menu</button>
            <div class="helper-text"> Nháy 2 lần: Chọn | Nháy 3-5 lần: Gọi Nhanh | Nháy 6 lần: SOS</div>
            <button class="top-btn scan-item" id="top-btn-speed" onclick="toggleSpeed()">Tốc Độ: Vừa </button>
        </div>

        <div id="output-box"></div>

        <div class="grid" id="main-grid">
            <div class="row scan-group">
                <button class="cell suggest word-suggest scan-item" id="sug1">TÔI</button>
                <button class="cell suggest word-suggest scan-item" id="sug2">CẦN</button>
                <button class="cell suggest word-suggest scan-item" id="sug3">MỆT</button>
                <button class="cell suggest word-suggest scan-item" id="sug4">ĐAU</button>
            </div>
            <!-- HÀNG 1: Các chữ dùng cực nhiều + NÚT XÓA và DẤU CÁCH đưa lên ngay đầu -->
            <div class="row scan-group">
                <button class="cell char scan-item">A</button><button class="cell char scan-item">N</button><button class="cell char scan-item">H</button>
                <button class="cell char scan-item">T</button><button class="cell char scan-item">C</button><button class="cell char scan-item">M</button>
                <button class="cell char scan-item">G</button><button class="cell sys scan-item">CÁCH</button><button class="cell sys scan-item">XÓA</button>
            </div>
            <!-- HÀNG 2: Nguyên âm & Phụ âm thông dụng -->
            <div class="row scan-group">
                <button class="cell char scan-item">E</button><button class="cell char scan-item">I</button><button class="cell char scan-item">O</button>
                <button class="cell char scan-item">U</button><button class="cell char scan-item">Y</button><button class="cell char scan-item">B</button>
                <button class="cell char scan-item">D</button><button class="cell char scan-item">Đ</button><button class="cell char scan-item">K</button>
            </div>
            <!-- HÀNG 3: Nguyên âm có dấu & Phụ âm ít dùng hơn -->
            <div class="row scan-group">
                <button class="cell char scan-item">Â</button><button class="cell char scan-item">Ă</button><button class="cell char scan-item">Ê</button>
                <button class="cell char scan-item">Ô</button><button class="cell char scan-item">Ơ</button><button class="cell char scan-item">Ư</button>
                <button class="cell char scan-item">L</button><button class="cell char scan-item">P</button><button class="cell char scan-item">Q</button>
            </div>
            <!-- HÀNG 4: Ít dùng nhất & Ngoại lai -->
            <div class="row scan-group">
                <button class="cell char scan-item">R</button><button class="cell char scan-item">S</button><button class="cell char scan-item">V</button>
                <button class="cell char scan-item">X</button><button class="cell char scan-item">F</button><button class="cell char scan-item">J</button>
                <button class="cell char scan-item">W</button><button class="cell char scan-item">Z</button><button class="cell sys scan-item">XÓA HẾT</button>
            </div>
            <!-- HÀNG 5: Chức năng -->
            <div class="row scan-group">
                <button class="cell sys scan-item" id="grid-btn-speed">TỐC ĐỘ</button>
                <button class="cell iot scan-item" id="grid-btn-r1"> BẬT ĐÈN</button>
                <button class="cell iot scan-item" id="grid-btn-r2"> BẬT QUẠT</button>
                <button class="cell sys scan-item" onclick="showReader()"> ĐỌC BÁO</button>
                <button class="cell danger scan-item">CẤP CỨU </button>
                <button class="cell action scan-item">ĐỌC CÂU </button>
            </div>
        </div>
    </div> 

<!-- VIEW 4: GIAO TIẾP NHANH BẰNG NHÁY MẮT (CHỈ 6 Ô) -->
    
    <div id="view-quick" style="display: none; flex-direction: column; width: 100%; height: 100vh; max-width: 1300px; margin: 0 auto; padding: 15px; z-index: 10;">
        <div id="top-controls" class="scan-group" style="margin-top: 90px; margin-bottom: 20px;">
            <button class="top-btn scan-item" onclick="showMenu()"> Trở Về Menu</button>
            <button class="top-btn scan-item" id="btn-quick-back" onclick="renderQuickGrid('main')" style="display: none; background: var(--warning); color: white;"> Chọn Lại Chủ Đề</button>
            <div class="helper-text" style="font-size: 22px;"> Nháy 2 lần để chọn</div>
        </div>
        <div class="grid" id="quick-grid" style="gap: 20px;">
            
        </div>
    </div>

    <!-- VIEW 4: TRÌNH ĐỌC SÁCH & BÁO BẰNG NHÁY MẮT -->
    <div id="view-reader">
        <div class="reader-nav">
            <div>
                <h2 id="story-title">Hạt Giống Tâm Hồn</h2>
                <p>CHẾ ĐỘ GIẢI TRÍ • QUÉT CHỌN HOÀN TOÀN BẰNG NHÁY MẮT</p>
            </div>
            <div style="display: flex; flex-direction: column; gap: 10px; align-items: flex-end;">
                <div class="reader-controls scan-group">
                    <button class="btn-reader-action scan-item" onclick="scrollReader(-1)">CUỘN LÊN </button>
                    <button class="btn-reader-action scan-item" onclick="scrollReader(1)">CUỘN XUỐNG </button>
                    <button class="btn-reader-action scan-item" id="btn-auto-scroll" onclick="toggleAutoScroll()">TỰ CUỘN ⏸</button>
                </div>
                <div class="reader-controls scan-group">
                    <button class="btn-reader-action scan-item" onclick="cycleStory()">ĐỔI BÀI </button>
                    <button class="btn-reader-action scan-item" onclick="readArticleTTS()">ĐỌC TO </button>
                    <button class="btn-reader-action scan-item" onclick="showMenu()">MENU </button>
                </div>
            </div>
        </div>

        <div class="reader-viewport" id="reader-content"></div>

        <div class="reader-guide-box">
             <strong>NHÁY 2 LẦN (HOẶC SPACE)</strong>: Chọn mục đang quét &nbsp;|&nbsp; 
             <strong>NHÁY >=6 LẦN</strong>: Cấp Cứu SOS
        </div>
    </div>

    <div id="status-bar">Đang kết nối hệ thống ESP32-S3...</div>

    <script>
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
                title: "Bản Tin Y Học: Sức Mạnh Não Bộ & Sóng Alpha",
                content: `
                <p>Các nghiên cứu thần kinh học hiện đại đã chứng minh: Khi tâm trí bệnh nhân thư giãn và tiếp nhận những thông điệp tích cực, biên độ sóng Alpha và Theta sẽ tăng mạnh mẽ.</p>
                <p>Sự gia tăng sóng Alpha giúp làm dịu hệ thống thần kinh giao cảm, kích hoạt cơ chế tự phục hồi của các tế bào não và tăng cường năng lượng nhận thức.</p>
                <p>Công nghệ trợ năng giao tiếp không lời chính là chiếc cầu nối kỳ diệu giúp người bệnh phục hồi sự tự chủ và niềm vui trong cuộc sống mỗi ngày.</p>
                `
            },
            {
                title: "Mẩu Chuyện Cười Lạc Quan Buổi Sáng",
                content: `
                <p>Bác sĩ ân cần hỏi bệnh nhân: "Dạo này anh thấy tinh thần của mình thế nào rồi?". Bệnh nhân cười tươi đáp: "Thưa bác sĩ, tôi thấy mình lạc quan tới mức mỗi lần nghe tiếng chuông báo thức reo, tôi cứ ngỡ cả thế giới đang vỗ tay hoan hô mình bắt đầu ngày mới!".</p>
                <p>Tiếng cười chính là liều thuốc bổ tự nhiên quý giá nhất mà tạo hóa ban tặng cho con người. Hãy luôn giữ nụ cười và tinh thần lạc quan bạn nhé!</p>
                `
            }
        ];
        let currentStoryIndex = 0;
        let isAutoScrolling = false;
        let autoScrollTimer = null;

        function loadStory(idx) {
            currentStoryIndex = idx % storyLibrary.length;
            const item = storyLibrary[currentStoryIndex];
            document.getElementById('story-title').innerText = item.title;
            document.getElementById('reader-content').innerHTML = item.content;
            window.scrollTo({ top: 0, behavior: 'smooth' });
        }

        function cycleStory() {
            loadStory(currentStoryIndex + 1);
            speakVoice("Đã chuyển sang bài đọc mới", true);
        }

        function scrollReader(dir) {
            window.scrollBy({ top: dir * window.innerHeight * 0.6, behavior: 'smooth' });
            speakVoice(dir > 0 ? "Cuộn xuống" : "Cuộn lên", true);
        }

        function toggleAutoScroll() {
            isAutoScrolling = !isAutoScrolling;
            const btn = document.getElementById('btn-auto-scroll');
            if (isAutoScrolling) {
                btn.classList.add('active');
                btn.innerText = "TỰ CUỘN ";
                speakVoice("Bắt đầu tự động cuộn trang", true);
                if (autoScrollTimer) clearInterval(autoScrollTimer);
                autoScrollTimer = setInterval(() => {
                    window.scrollBy({ top: 3, behavior: 'smooth' });
                }, 45);
            } else {
                btn.classList.remove('active');
                btn.innerText = "TỰ CUỘN ";
                speakVoice("Đã dừng tự động cuộn trang", true);
                clearInterval(autoScrollTimer);
                autoScrollTimer = null;
            }
        }

        function readArticleTTS() {
            const rawText = document.getElementById('reader-content').innerText;
            speakVoice(rawText, true);
        }

        function toggleTheme() {
            const html = document.documentElement;
            const currentTheme = html.getAttribute('data-theme');
            const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
            html.setAttribute('data-theme', newTheme);
            localStorage.setItem('bci-theme', newTheme);
            document.getElementById('theme-icon').innerText = newTheme === 'dark' ? '☀️' : '🌓';
        }
        const savedTheme = localStorage.getItem('bci-theme') || 'light';
        document.documentElement.setAttribute('data-theme', savedTheme);
        document.getElementById('theme-icon').innerText = savedTheme === 'dark' ? '☀️' : '🌓';

        const hostIP = window.location.hostname || "esp32-bci.local";
        let ws;
        let currentView = 'MENU';
        let blinkCount = 0;
        let blinkTimeout = null;
        const BLINK_WINDOW = 1400; 
        let isSleeping = false;

        function connectWebSocket() {
            ws = new WebSocket(`ws://${hostIP}:81`);
            ws.onopen = () => { document.getElementById('status-bar').innerHTML = "Trạng thái ESP32-S3: <span style='color:var(--success)'>Đã kết nối thiết bị 🟢</span>"; };
            ws.onclose = () => { document.getElementById('status-bar').innerHTML = "Trạng thái ESP32-S3: <span style='color:var(--danger)'>Mất kết nối (Đang thử lại...) 🔴</span>"; setTimeout(connectWebSocket, 2500); };
            ws.onmessage = (event) => {
                const data = event.data;
                if (data.startsWith("BLINK:")) registerBlink(); 
                else if (data.startsWith("TGAM:")) {
                    let parts = data.substring(5).split(',');
                    if (parts.length >= 3) updateTGAMDisplay(parseInt(parts[0]), parseInt(parts[1]), parseInt(parts[2]));
                } 
                else if (data.startsWith("INA:")) {
                    let parts = data.substring(4).split(',');
                    if (parts.length >= 4) {
                        document.getElementById('bat-val').innerText = parts[3] + '%';
                        const elMini = document.getElementById('bat-mini');
                        if (elMini) elMini.innerText = parts[3] + '%';
                    }
                }
                else if (data.startsWith("RELAY:")) {
                    let parts = data.substring(6).split(',');
                    updateRelayUI(1, parts[0] === '1'); updateRelayUI(2, parts[1] === '1');
                } 
                else if (data === "SLEEPING") enterSleepMode();
                else if (data === "WAKEUP") exitSleepMode();
            };
        }
        connectWebSocket();

        function updateClock() {
            const now = new Date();
            document.getElementById('clock-time').innerText = now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', hour12: false });
            document.getElementById('clock-date').innerText = `${now.getDate().toString().padStart(2,'0')}/${(now.getMonth()+1).toString().padStart(2,'0')}/${now.getFullYear()}`;
        }
        setInterval(updateClock, 1000); updateClock();

        function sendCommand(cmd) { if (ws && ws.readyState === WebSocket.OPEN) ws.send(cmd); }

        function updateTGAMDisplay(sig, att, med) {
            let elSig = document.getElementById('sig-val'); let elSigMini = document.getElementById('sig-mini');
            if (sig === 0) { elSig.innerText = "TỐT"; elSig.className = "dash-value good"; if(elSigMini) { elSigMini.innerText = "TỐT"; elSigMini.style.color = "var(--success)"; } } 
            else if (sig === 200) { elSig.innerText = "RỚT ĐIỆN CỰC"; elSig.className = "dash-value bad"; if(elSigMini) { elSigMini.innerText = "RỚT"; elSigMini.style.color = "var(--danger)"; } } 
            else { elSig.innerText = `KÉM (${sig})`; elSig.className = "dash-value warn"; if(elSigMini) { elSigMini.innerText = `KÉM`; elSigMini.style.color = "var(--warning)"; } }
            document.getElementById('att-val').innerText = att;
            if(document.getElementById('att-mini')) document.getElementById('att-mini').innerText = att;
        }

        function showMenu() {
            currentView = 'MENU';
            document.getElementById('view-keyboard').style.display = 'none'; 
            document.getElementById('view-reader').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'none';
            document.getElementById('view-menu').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'none';
            if (isAutoScrolling) toggleAutoScroll();
            buildScanTree();
            startScanning();
            speakVoice("Quay lại màn hình chính");
        }

        function showKeyboard() {
            currentView = 'KEYBOARD';
            document.getElementById('view-menu').style.display = 'none'; 
            document.getElementById('view-reader').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'none';
            document.getElementById('view-keyboard').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'flex';
            if (isAutoScrolling) toggleAutoScroll();
            buildScanTree();
            startScanning();
            speakVoice("Mở bàn phím chữ");
        }

        function showReader() {
            currentView = 'READER';
            document.getElementById('view-menu').style.display = 'none'; 
            document.getElementById('view-keyboard').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'none';
            document.getElementById('view-reader').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'flex';
            loadStory(currentStoryIndex);
            buildScanTree();
            startScanning();
            speakVoice("Mở trình đọc sách báo", true);
        }
            const quickData = {
            main: [
                { label: "🍽️ ĂN UỐNG", target: "food", bg: "#3b82f6" },
                { label: "🤒 BÁO ĐAU", target: "pain", bg: "#ef4444" },
                { label: "🛏️ SINH HOẠT", target: "life", bg: "#10b981" },
                { label: "🆘 GỌI NGƯỜI", target: "help", bg: "#f59e0b" }
            ],
            food: [
                { label: "💧 TÔI KHÁT NƯỚC", bg: "#0ea5e9" }, { label: "🥛 TÔI MUỐN UỐNG SỮA", bg: "#0ea5e9" },
                { label: "🍚 TÔI MUỐN ĂN CƠM", bg: "#059669" }, { label: "🥣 TÔI MUỐN ĂN CHÁO", bg: "#059669" }
            ],
            pain: [
                { label: "🤕 ĐAU ĐẦU / CHÓNG MẶT", bg: "#b91c1c" }, { label: "🫁 ĐAU NGỰC / KHÓ THỞ", bg: "#b91c1c" },
                { label: "🤢 ĐAU BỤNG / BUỒN NÔN", bg: "#dc2626" }, { label: "🦵 ĐAU NHỨC TAY CHÂN", bg: "#dc2626" }
            ],
            life: [
                { label: "🚽 CẦN ĐI VỆ SINH", bg: "#059669" }, { label: "👕 MUỐN THAY QUẦN ÁO", bg: "#059669" },
                { label: "🛏️ ĐỠ TÔI NẰM XUỐNG", bg: "#16a34a" }, { label: "🪑 ĐỠ TÔI NGỒI DẬY", bg: "#16a34a" }
            ],
            help: [
                { label: "👨‍⚕️ GỌI BÁC SĨ KHÁM", bg: "#d97706" }, { label: "👩‍⚕️ GỌI Y TÁ", bg: "#d97706" },
                { label: "👨‍👩‍👧 GỌI NGƯỜI NHÀ", bg: "#ea580c" }, { label: "🥵 TÔI THẤY NÓNG / LẠNH", bg: "#ea580c" }
            ]
        };

        function renderQuickGrid(type) {
            const grid = document.getElementById('quick-grid');
            const backBtn = document.getElementById('btn-quick-back');
            grid.innerHTML = '';
            
            if (type === 'main') {
                backBtn.style.display = 'none';
                let html = '';
                for(let i=0; i<quickData.main.length; i+=2) {
                    html += `<div class="row scan-group" style="gap: 20px;">
                        <button class="cell action quick-cat scan-item" data-target="${quickData.main[i].target}" style="background: ${quickData.main[i].bg}; font-size: 40px; padding: 60px 10px; font-weight: 900;">${quickData.main[i].label}</button>
                        <button class="cell action quick-cat scan-item" data-target="${quickData.main[i+1].target}" style="background: ${quickData.main[i+1].bg}; font-size: 40px; padding: 60px 10px; font-weight: 900;">${quickData.main[i+1].label}</button>
                    </div>`;
                }
                grid.innerHTML = html;
            } else {
                backBtn.style.display = 'block'; // Hiện nút quay lại
                let items = quickData[type];
                let html = '';
                for(let i=0; i<items.length; i+=2) {
                    html += `<div class="row scan-group" style="gap: 20px;">
                        <button class="cell action quick-phrase scan-item" style="background: ${items[i].bg}; font-size: 32px; padding: 50px 10px; font-weight: 900;">${items[i].label}</button>
                        <button class="cell action quick-phrase scan-item" style="background: ${items[i+1].bg}; font-size: 32px; padding: 50px 10px; font-weight: 900;">${items[i+1].label}</button>
                    </div>`;
                }
                grid.innerHTML = html;
            }
            
            // Gắn sự kiện click cho các nút mới tạo bằng JS
            grid.querySelectorAll('.scan-item').forEach(btn => {
                btn.addEventListener('click', (e) => { 
                    e.preventDefault(); e.stopPropagation(); 
                    executeAction(btn); 
                    if (document.activeElement) document.activeElement.blur(); 
                });
            });

            // Cập nhật lại khung quét
            buildScanTree();
            startScanning();
        }

        function showQuick() {
            currentView = 'QUICK';
            document.getElementById('view-menu').style.display = 'none'; 
            document.getElementById('view-keyboard').style.display = 'none'; 
            document.getElementById('view-reader').style.display = 'none'; 
            document.getElementById('view-quick').style.display = 'flex';
            document.getElementById('tgam-status-mini').style.display = 'flex';
            if (isAutoScrolling) toggleAutoScroll();
            
            // Luôn mở trang danh mục gốc của Giao tiếp nhanh
            renderQuickGrid('main');
            speakVoice("Mở bảng giao tiếp nhanh");
        }

        let voices = []; 
        let ttsRate = parseFloat(localStorage.getItem('ttsRate')) || 0.85; 
        let ttsVolume = parseFloat(localStorage.getItem('ttsVolume')) || 1.0;
        let savedVoiceURI = localStorage.getItem('ttsVoiceURI'); 
        let isSettingsOpen = false;
        
        let speechQueue = []; 
        let isSpeechPlaying = false; 
        let ttsWatchdog = null;
        let currentSpeechId = 0; // Thêm biến quản lý ID để chống kẹt giọng đọc

        document.getElementById('rate-slider').value = ttsRate; document.getElementById('rate-val').innerText = ttsRate;
        document.getElementById('vol-slider').value = ttsVolume; document.getElementById('vol-val').innerText = Math.round(ttsVolume * 100) + '%';

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

        function processNextSpeech() {
            if (speechQueue.length === 0) { isSpeechPlaying = false; if (ttsWatchdog) { clearTimeout(ttsWatchdog); ttsWatchdog = null; } return; }
            if (!('speechSynthesis' in window)) { speechQueue = []; isSpeechPlaying = false; return; }
            
            isSpeechPlaying = true;
            const item = speechQueue.shift();
            const msg = new SpeechSynthesisUtterance(item.text);
            msg.rate = ttsRate; msg.volume = ttsVolume;
            
            if (voices.length > 0) {
                const curURI = document.getElementById('voice-select').value;
                const v = voices.find(vox => vox.voiceURI === curURI) || voices.find(vox => vox.lang.includes('vi'));
                if (v) msg.voice = v;
            }
            
            const mySpeechId = ++currentSpeechId;
            let finished = false;
            
            const completeSpeech = () => { 
                if (finished) return; 
                finished = true; 
                if (ttsWatchdog) { clearTimeout(ttsWatchdog); ttsWatchdog = null; } 
                // Chỉ gọi câu tiếp theo nếu ID không bị thay đổi (tức là không bị cancel ngang do quét nhanh)
                if (currentSpeechId === mySpeechId) {
                    setTimeout(() => { processNextSpeech(); }, 20); 
                }
            };
            
            msg.onend = completeSpeech; msg.onerror = completeSpeech;
            ttsWatchdog = setTimeout(() => { if (!finished) completeSpeech(); }, Math.max(1000, item.text.length * 150 + 500));
            
            try { window.speechSynthesis.speak(msg); } catch(e) { completeSpeech(); }
        }

        function speakVoice(text, immediate = false) {
            if (!('speechSynthesis' in window) || !text || text.trim() === '') return;
            
            if (immediate) { 
                currentSpeechId++; // Vô hiệu hóa callback của câu đang đọc dở
                speechQueue = []; 
                if (ttsWatchdog) { clearTimeout(ttsWatchdog); ttsWatchdog = null; } 
                try { window.speechSynthesis.cancel(); } catch(e) {} 
                isSpeechPlaying = false; 
            }
            
            if (speechQueue.length >= 8) speechQueue.shift();
            speechQueue.push({ text: text.trim() });
            
            if (!isSpeechPlaying) {
                if (immediate) {
                    // Đợi 30ms để trình duyệt kịp hủy câu cũ trước khi đọc câu mới (Chống kẹt delay)
                    setTimeout(() => { processNextSpeech(); }, 30);
                } else {
                    processNextSpeech();
                }
            }
        }

        const speeds = [1500, 1000, 600]; const speedLabels = ["CHẬM", "VỪA", "NHANH"]; let speedIdx = 1; let scanInterval;
        let scanGroups = []; let currentGroupIdx = 0; let currentItemIdx = 0; let scanState = 'GROUP'; let loopCount = 0;

        function toggleSpeed() {
            speedIdx = (speedIdx + 1) % 3; const newLabel = "TỐC ĐỘ: " + speedLabels[speedIdx];
            document.getElementById('top-btn-speed').innerText = newLabel + " "; document.getElementById('grid-btn-speed').innerText = newLabel;
            speakVoice(newLabel, true);
            if (!isSleeping && !isSettingsOpen) startScanning();
        }

        function buildScanTree() {
            let activeView = document.getElementById('view-menu');
            if (currentView === 'KEYBOARD') activeView = document.getElementById('view-keyboard');
            else if (currentView === 'READER') activeView = document.getElementById('view-reader');
            else if (currentView === 'QUICK') activeView = document.getElementById('view-quick');
            
            scanGroups = Array.from(activeView.querySelectorAll('.scan-group'));
            currentGroupIdx = 0;
            currentItemIdx = 0;
            scanState = 'GROUP';
            loopCount = 0;
            renderScan();
        }

        function renderScan() { 
            document.querySelectorAll('.scanning-row').forEach(el => el.classList.remove('scanning-row'));
            document.querySelectorAll('.scanning-cell').forEach(el => el.classList.remove('scanning-cell'));
            
            if (!scanGroups || scanGroups.length === 0) return;
            
            if (scanState === 'GROUP') {
                if(scanGroups[currentGroupIdx]) {
                    scanGroups[currentGroupIdx].classList.add('scanning-row');
                }
            } else if (scanState === 'ITEM') {
                if(scanGroups[currentGroupIdx]) {
                    scanGroups[currentGroupIdx].classList.add('scanning-row');
                    let items = scanGroups[currentGroupIdx].querySelectorAll('.scan-item');
                    if (items && items[currentItemIdx]) {
                        items[currentItemIdx].classList.add('scanning-cell');
                    }
                }
            }
        }

        function tick() { 
            if (isSleeping || isSettingsOpen) return; 
            if (!scanGroups || scanGroups.length === 0) return;

            if (scanState === 'GROUP') {
                currentGroupIdx = (currentGroupIdx + 1) % scanGroups.length;
            } else if (scanState === 'ITEM') {
                let items = scanGroups[currentGroupIdx].querySelectorAll('.scan-item');
                if (!items || items.length === 0) {
                    scanState = 'GROUP';
                } else {
                    currentItemIdx++;
                    if (currentItemIdx >= items.length) {
                        loopCount++;
                        if (loopCount >= 2) { 
                            scanState = 'GROUP'; 
                            currentItemIdx = 0; 
                            loopCount = 0; 
                        } else {
                            currentItemIdx = 0;
                        }
                    }
                    
                    if (scanState === 'ITEM' && items[currentItemIdx]) {
                        let text = items[currentItemIdx].innerText.trim().replace(/[^a-zA-ZÀ-ỹ0-9 ]/g, "").trim();
                        if (text === "CÁCH") text = "Cách"; 
                        else if (text === "XÓA") text = "Xóa"; 
                        else if (text === "XÓA HẾT") text = "Xóa hết"; 
                        else if (text === "ĐỌC CÂU") text = "Đọc câu"; 
                        else if (text === "CẤP CỨU") text = "Cấp cứu"; 
                        else if (text.startsWith("TỐC ĐỘ")) text = "Tốc độ"; 
                        else if (text.includes("ĐỌC BÁO") || text.includes("ĐỌC SÁCH")) text = "Đọc báo";
                        else if (text.includes("ĐỔI BÀI")) text = "Đổi bài";
                        else if (text.includes("TỰ CUỘN")) text = "Tự cuộn";
                        else if (text.includes("ĐỌC TO")) text = "Đọc to";
                        else if (text.includes("MENU")) text = "Về Menu";
                        else if (text.includes("CUỘN LÊN")) text = "Cuộn lên";
                        else if (text.includes("CUỘN XUỐNG")) text = "Cuộn xuống";
                        else if (text.includes("ĐÈN")) text = "Đèn phòng";
                        else if (text.includes("QUẠT")) text = "Quạt máy";
                        else if (text.includes("CÀI ĐẶT")) text = "Cài đặt";
                        else if (text.includes("BÀN PHÍM")) text = "Bàn phím";
else if (text.includes("GIAO TIẾP NHANH")) text = "Mở giao tiếp nhanh";
                    else if (text.includes("GÕ BÀN PHÍM")) text = "Mở gõ bàn phím chữ";
            

                        if (text) speakVoice(text, true); 
                    }
                }
            }
            renderScan(); 
        }

        function startScanning() { 
            clearInterval(scanInterval); 
            renderScan(); 
            scanInterval = setInterval(tick, speeds[speedIdx]); 
        }

        function registerBlink() {
            if (isSleeping || isSettingsOpen) return;
            blinkCount++;
            if (blinkCount === 1) clearInterval(scanInterval);
            const counterEl = document.getElementById('blink-counter');
            counterEl.innerText = " Nháy mắt: " + blinkCount; counterEl.style.display = 'block';
            if (blinkTimeout) clearTimeout(blinkTimeout);
            blinkTimeout = setTimeout(() => { counterEl.style.display = 'none'; processBlinks(blinkCount); blinkCount = 0; }, BLINK_WINDOW);
        }

        function processBlinks(count) {
            if (count >= 6) {
                sendQuickAlert("SOS");
                return;
            }

            if (count === 1) {
                console.log("Bỏ qua 1 lần nháy mắt (sinh học)");
            } else if (count === 2) {
                executeNormalSelection();
            } else if (count === 3) {
                sendQuickAlert("TÔI CẦN UỐNG NƯỚC");
            } else if (count === 4) {
                sendQuickAlert("TÔI CẦN ĐI VỆ SINH");
            } else if (count === 5) {
                sendQuickAlert("TÔI ĐANG RẤT ĐAU");
            }

            if (!isSleeping && !isSettingsOpen) startScanning();
        }

        function executeNormalSelection() {
            if (!scanGroups || scanGroups.length === 0) return;

            if (scanState === 'GROUP') {
                let items = scanGroups[currentGroupIdx].querySelectorAll('.scan-item');
                if (items && items.length > 0) {
                    // CHỖ SỬA LỖI NẰM Ở ĐÂY:
                    if (items.length === 1) {
                        // Nếu hàng này CHỈ CÓ 1 NÚT, lập tức click luôn không cần vào trong
                        items[0].click();
                        scanState = 'GROUP'; 
                        loopCount = 0;
                        renderScan();
                    } else {
                        // Nếu hàng có từ 2 nút trở lên (như bàn phím, cài đặt), mới quét tiếp vào trong
                        scanState = 'ITEM'; 
                        currentItemIdx = 0; 
                        loopCount = 0; 
                        renderScan();
                        
                        let text = items[0].innerText.trim().replace(/[^a-zA-ZÀ-ỹ0-9 ]/g, "").trim();
                        if (text) speakVoice(text, true);
                    }
                }
            } 
            else if (scanState === 'ITEM') {
                let items = scanGroups[currentGroupIdx].querySelectorAll('.scan-item');
                if (items && items[currentItemIdx]) {
                    let btn = items[currentItemIdx];
                    btn.click();
                    
                    if (scanState === 'ITEM') {
                        scanState = 'GROUP';
                        loopCount = 0;
                        renderScan();
                    }
                }
            }
        }

        function sendQuickAlert(msg) {
            if (msg === "SOS") { speakVoice("CẤP CỨU! CẦN BÁC SĨ GIÚP ĐỠ GẤP!", true); sendCommand("SOS"); } 
            else { speakVoice(msg, true); sendCommand("SENTENCE:" + msg); }
            showToastAlert(msg);
        }

        function showToastAlert(msg) {
            const toast = document.getElementById('toast-alert');
            toast.innerText = msg === "SOS" ? " CẤP CỨU KHẨN CẤP " : " " + msg;
            toast.style.background = msg === "SOS" ? "var(--danger)" : "var(--warning)";
            toast.style.display = 'block'; setTimeout(() => { toast.style.display = 'none'; }, 4000);
        }

        const dictionary = ["TÔI", "BẠN", "MẸ", "CHA", "BÁC SĨ", "Y TÁ", "CON", "CHỒNG", "VỢ", "NHÀ", "BỆNH VIỆN", "CẦN", "MUỐN", "THÍCH", "LÀM", "ĂN", "UỐNG", "ĐI", "NGỦ", "TẮM", "KHÁM", "MỞ", "BẬT", "TẮT", "LẤY", "GIÚP", "NGỒI", "NẰM", "MỆT", "ĐAU", "NHỨC", "MỎI", "NGỨA", "HO", "THỞ", "KHÁT", "ĐÓI", "NÓNG", "LẠNH", "KHỎE", "YẾU", "CHÓNG MẶT", "BUỒN NÔN", "NƯỚC", "CƠM", "MÌ", "PHỞ", "CHÁO", "SỮA", "THUỐC", "ÁO", "QUẦN", "GIƯỜNG", "ĐÈN", "QUẠT", "TV", "CHĂN", "GỐI", "ĐẦU", "BỤNG", "TAY", "CHÂN", "LƯNG", "NGỰC", "MẮT", "CỔ", "MIỆNG", "ĐÚNG", "SAI", "RẤT", "QUÁ", "KHÔNG", "CÓ", "CHƯA", "RỒI", "ĐANG", "SẼ", "XIN LỖI", "CẢM ƠN", "VÂNG", "DẠ", "NHIỀU", "ÍT", "NHANH", "CHẬM", "VỆ SINH", "QUẦN ÁO", "LAU NGƯỜI"];
        const bigrams = { "TÔI": ["CẦN", "MỆT", "ĐAU", "KHÁT", "ĐÓI", "MUỐN", "LẠNH", "NÓNG"], "CẦN": ["GIÚP", "UỐNG", "ĂN", "NGỦ", "ĐI", "THUỐC", "LẤY", "BÁC SĨ", "VỆ SINH"], "MUỐN": ["UỐNG", "ĂN", "VỆ SINH", "NGỦ", "TẮM", "VỀ", "NGỒI", "NẰM"], "ĐAU": ["ĐẦU", "BỤNG", "LƯNG", "CHÂN", "TAY", "NGỰC", "MẮT", "CỔ"], "UỐNG": ["NƯỚC", "SỮA", "THUỐC"], "ĂN": ["CƠM", "CHÁO", "PHỞ", "MÌ"], "ĐI": ["VỆ SINH", "NGỦ", "VIỆN", "VỀ", "TẮM"], "BẬT": ["ĐÈN", "QUẠT", "TV", "MÁY"], "TẮT": ["ĐÈN", "QUẠT", "TV", "MÁY"], "BÁC SĨ": ["ƠI", "KHÁM", "CHỮA", "GIÚP"], "LẤY": ["NƯỚC", "THUỐC", "CHĂN", "GỐI", "ÁO"], "CẢM": ["ƠN", "LẠNH", "THẤY", "GIÁC"], "XIN": ["LỖI", "NƯỚC", "THÊM", "GIÚP"], "RẤT": ["MỆT", "ĐAU", "ĐÓI", "KHÁT", "NÓNG", "LẠNH"], "KHÔNG": ["ĂN", "UỐNG", "CẦN", "THỞ", "ĐƯỢC", "SAO", "KỊP"], "CÓ": ["ĐAU", "MỆT", "AI", "THỂ"], "MỎI": ["LƯNG", "CHÂN", "TAY", "CỔ", "MẮT"], "KHÓ": ["THỞ", "CHỊU", "NGỦ", "ĂN", "UỐNG"] };

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
            if (currentPrefix === '') { matches = (lastCompleteWord && bigrams[lastCompleteWord]) ? [...bigrams[lastCompleteWord]] : ["TÔI", "CẦN", "MỆT", "ĐAU"]; } 
            else {
                const cleanPrefix = removeAccents(currentPrefix);
                if (lastCompleteWord && bigrams[lastCompleteWord]) { matches.push(...bigrams[lastCompleteWord].filter(w => removeAccents(w).startsWith(cleanPrefix) && w !== currentPrefix)); }
                dictionary.filter(w => removeAccents(w).startsWith(cleanPrefix) && w !== currentPrefix).forEach(w => { if (!matches.includes(w)) matches.push(w); });
            }
            if (matches.length < 4) dictionary.forEach(w => { if (!matches.includes(w) && w !== currentPrefix && matches.length < 4) matches.push(w); });
            for(let i=1; i<=4; i++) document.getElementById('sug'+i).innerText = matches[i-1] || '';
        }

        function applySuggestion(word) {
            const outBox = document.getElementById('output-box'); let text = outBox.textContent;
            if (text === '' || text.endsWith(' ')) outBox.textContent = text + word + ' ';
            else { let words = text.trim().split(/\s+/); words.pop(); words.push(word); outBox.textContent = words.join(' ') + ' '; }
            updatePredictiveText();
        }

        function executeAction(element) {
            const textRaw = element.innerText.trim();
            const text = textRaw.replace(/[^a-zA-ZÀ-ỹ0-9 ]/g, "").trim(); 
            if (textRaw === '') return; 
            const outBox = document.getElementById('output-box');

            if (element.classList.contains('word-suggest')) { applySuggestion(text); speakVoice(text, true); }

            // LEVEL 1: Nếu bệnh nhân nháy vào Chủ đề (Ăn uống, Báo đau...)
            else if (element.classList.contains('quick-cat')) { 
                let target = element.getAttribute('data-target');
                speakVoice(text, true); // Máy đọc tên Chủ đề
                renderQuickGrid(target); // Vẽ ra giao diện con tương ứng
            }
            // LEVEL 2: Nếu bệnh nhân nháy vào Câu nói chi tiết
            else if (element.classList.contains('quick-phrase')) { 
                speakVoice(text, true); // Máy phát âm câu chốt
                sendCommand("SENTENCE:" + text); // Đẩy lên Cloud / Y tá
                showToastAlert(text);
                setTimeout(showMenu, 2500); // Giao tiếp xong tự động dọn dẹp và quay về Menu
            }
            else if (element.classList.contains('char')) { outBox.textContent += text; outBox.textContent = processTelex(outBox.textContent); updatePredictiveText(); speakVoice(text, true); }
            else if (text === "CÁCH") { if (!outBox.textContent.endsWith(' ') && outBox.textContent !== '') { outBox.textContent += " "; updatePredictiveText(); } speakVoice("Dấu cách", true); }
            else if (text === "XÓA") { outBox.textContent = outBox.textContent.slice(0, -1); updatePredictiveText(); speakVoice("Xóa", true); }
            else if (text === "XÓA HẾT") { outBox.textContent = ""; updatePredictiveText(); speakVoice("Xóa hết", true); }
            else if (text === "ĐỌC CÂU") { let phrase = outBox.textContent.trim(); if (phrase !== "") { speakVoice(phrase, true); sendCommand("SENTENCE:" + phrase); } else speakVoice("Chưa có nội dung", true); }
            else if (text.startsWith("TỐC ĐỘ")) toggleSpeed();
            else if (text.includes("ĐÈN")) toggleRelay(1);
            else if (text.includes("QUẠT")) toggleRelay(2);
            else if (text === "NGỦ") { enterSleepMode(); sendCommand('S'); }
            else if (text === "CẤP CỨU") sendQuickAlert("SOS");
            
            scanState = 'GROUP'; loopCount = 0; renderScan();
        }

        document.querySelectorAll('button, .cell, .btn-main, .btn-sub, .top-btn').forEach(btn => {
            btn.addEventListener('click', (e) => { 
                e.preventDefault(); 
                e.stopPropagation(); 
                if(btn.classList.contains('cell')) executeAction(btn); 
                if (document.activeElement) document.activeElement.blur(); 
            });
        });

        function toggleRelay(num) { sendCommand('R' + num); }
        function updateRelayUI(num, state) {
            let label = state ? "Đang Bật" : "Đang Tắt"; let actionLabel = state ? "TẮT" : "BẬT"; let color = state ? "var(--success)" : "var(--text-muted)";
            let mEl = document.getElementById('menu-r' + num); if (mEl) { mEl.innerText = label; mEl.style.color = color; }
            let kEl = document.getElementById('grid-btn-r' + num);
            if (kEl) {
                let name = num === 1 ? "ĐÈN" : "QUẠT"; let icon = num === 1 ? "" : "";
                kEl.innerText = `${icon} ${actionLabel} ${name}`;
                kEl.style.backgroundColor = state ? "rgba(245, 158, 11, 0.15)" : "var(--key-sys-bg)"; 
                kEl.style.color = state ? "var(--warning)" : "var(--text-muted)";
                kEl.style.borderColor = state ? "var(--warning)" : "var(--key-border)";
            }
        }

        function enterSleepMode() { 
            isSleeping = true; 
            document.getElementById('sleep-overlay').style.display = 'flex'; 
            clearInterval(scanInterval); 
            if (isAutoScrolling) toggleAutoScroll();
            speakVoice("Hệ thống chuyển sang chế độ nghỉ ngơi", true); 
        }
        function exitSleepMode() { 
            isSleeping = false; 
            document.getElementById('sleep-overlay').style.display = 'none'; 
            startScanning(); 
            speakVoice("Hệ thống đã thức dậy", true); 
        }

        function openSettings() { isSettingsOpen = true; document.getElementById('settings-overlay').style.display = 'flex'; clearInterval(scanInterval); }
        function closeSettings() {
            isSettingsOpen = false; ttsRate = parseFloat(document.getElementById('rate-slider').value); ttsVolume = parseFloat(document.getElementById('vol-slider').value);
            savedVoiceURI = document.getElementById('voice-select').value;
            localStorage.setItem('ttsRate', ttsRate); localStorage.setItem('ttsVolume', ttsVolume); localStorage.setItem('ttsVoiceURI', savedVoiceURI);
            document.getElementById('settings-overlay').style.display = 'none'; if (!isSleeping) startScanning();
        }
        function testVoice() { let curR = ttsRate, curV = ttsVolume; ttsRate = parseFloat(document.getElementById('rate-slider').value); ttsVolume = parseFloat(document.getElementById('vol-slider').value); speakVoice("Chào bạn, chúc bạn một ngày tốt lành!", true); ttsRate = curR; ttsVolume = curV; }

        document.getElementById('rate-slider').oninput = function() { document.getElementById('rate-val').innerText = this.value; };
        document.getElementById('vol-slider').oninput = function() { document.getElementById('vol-val').innerText = Math.round(this.value * 100) + '%'; };

        window.addEventListener('keydown', (e) => { 
    if (e.code === 'Space') { 
        e.preventDefault(); 
        e.stopPropagation();
        if (document.activeElement) document.activeElement.blur(); 
        
        if (isSleeping) {
            sendCommand('W'); 
            exitSleepMode();
        } else {
            // Chỉ gửi lệnh xuống ESP32, để ESP32 báo ngược lại thì mới đếm
            sendCommand('SIMULATE_BLINK'); 
        }
    } 
}, true);
        // Khởi động giao diện & Cây quét
        updatePredictiveText();
        
        // CHỐT CHẶN: Chỉ khởi chạy quét khi web đã load được 100% để chống lỗi null
        document.addEventListener('DOMContentLoaded', () => {
            buildScanTree();
            startScanning();
        });
    </script>
</body>
</html>
)rawliteral";

// =========================================================================
// BIẾN GIAO TIẾP ĐA LUỒNG (FREERTOS)
// =========================================================================
String pendingCloudSentence = "";
bool pendingTelemetry = false;

// =========================================================================
// TASK FREERTOS: XỬ LÝ GỬI DỮ LIỆU LÊN CLOUD RENDER (CHẠY NGẦM Ở CORE 0)
// =========================================================================
void cloudSyncTask(void * parameter) {
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(100));

    if (pendingCloudSentence != "") {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(secureClient, CLOUD_SERVER_URL);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(2500); 

        String jsonPayload = "{\"sentence\":\"" + pendingCloudSentence + "\",\"attention\":" + String(attention) + ",\"meditation\":" + String(meditation) + ",\"signalQuality\":" + String(signalQuality) + "}";
        int httpResponseCode = http.POST(jsonPayload);
        
        if (httpResponseCode > 0) {
          Serial.printf("[CLOUD SYNC] Đã đồng bộ lên Render (HTTP %d): %s\n", httpResponseCode, pendingCloudSentence.c_str());
        } else {
          Serial.printf("[CLOUD SYNC] Lỗi gửi Render: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
      }
      pendingCloudSentence = ""; 
    }

    if (pendingTelemetry) {
      if (WiFi.status() == WL_CONNECTED) {
        readINA226();

        HTTPClient http;
        http.begin(secureClient, CLOUD_TELEMETRY_URL);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(2500);

        String payload = "{\"isSleeping\":" + String(isSleeping ? "true" : "false") + "," +
                         "\"signalQuality\":" + String(signalQuality) + "," +
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

        int httpCode = http.POST(payload);
        if (httpCode > 0) {
          Serial.printf("[TELEMETRY] Đã gửi Heartbeat & INA226 lên Render (HTTP %d)\n", httpCode);
        }
        http.end();
      }
      pendingTelemetry = false;
    }
  }
}

void syncSentenceToCloud(String sentence) {
  pendingCloudSentence = sentence; 
}

void syncTelemetryToCloud() {
  pendingTelemetry = true; 
}

// =========================================================================
// XỬ LÝ SỰ KIỆN WEBSOCKET TẠI MÁY CỤC BỘ
// =========================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Client #%u Disconnected\n", num);
      break;

    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[WS] Client #%u Connected from %s\n", num, ip.toString().c_str());
      
      String rState = "RELAY:" + String(relay1State ? "1" : "0") + "," + String(relay2State ? "1" : "0");
      webSocket.sendTXT(num, rState);

      String inaMsg = "INA:" + String(inaBusVoltage, 2) + "," + String(inaCurrent_mA, 1) + "," + String(inaPower_mW, 1) + "," + String(batteryPercent);
      webSocket.sendTXT(num, inaMsg);
      
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
        blinkCountForWake = 0;
        webSocket.broadcastTXT("WAKEUP");
        syncTelemetryToCloud();
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
        Serial.println("[ALERT] BỆNH NHÂN GỌI CẤP CỨU (SOS)!");
        syncSentenceToCloud("SOS - CẤP CỨU KHẨN CẤP");
      }
      else if (msg == "SIMULATE_BLINK") {
        signalQuality = 0;
        attention = random(60, 100);
        meditation = random(60, 100);
        delta = random(100000, 500000);
        theta = random(50000, 100000);
        lowAlpha = random(10000, 50000);
        lowBeta = random(10000, 30000);
        lowGamma = random(1000, 10000);
        blinkStrength = random(50, 100);
        
        webSocket.broadcastTXT("BLINK:" + String(blinkStrength));
        syncTelemetryToCloud(); 
      }
      break;
    }
    default:
      break;
  }
}

// =========================================================================
// GIẢI MÃ BẢN TIN CẢM BIẾN NEUROSKY TGAM (57600 BAUD)
// =========================================================================
void parseTGAMPacket(uint8_t* data, int length) {
  int i = 0;
  while (i < length) {
    uint8_t code = data[i++];
    if (code == 0x02) {
      signalQuality = data[i++];
    } 
    else if (code == 0x04) {
      attention = data[i++];
    } 
    else if (code == 0x05) {
      meditation = data[i++];
    } 
    else if (code == 0x16) {
      blinkStrength = data[i++];
      lastActivityTime = millis();

      if (isSleeping) {
        if (millis() - lastBlinkTime < 2500) {
          blinkCountForWake++;
        } else {
          blinkCountForWake = 1;
        }
        lastBlinkTime = millis();

        if (blinkCountForWake >= 4) {
          isSleeping = false;
          WiFi.setSleep(false);
          blinkCountForWake = 0;
          webSocket.broadcastTXT("WAKEUP");
          syncTelemetryToCloud();
        }
      } else {
        webSocket.broadcastTXT("BLINK:" + String(blinkStrength));
      }
    } 
    else if (code == 0x83) { 
        int vLength = data[i++];
        if (vLength == 24) {
            delta = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            theta = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            lowAlpha = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            highAlpha = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            lowBeta = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            highBeta = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            lowGamma = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
            midGamma = (data[i]<<16) | (data[i+1]<<8) | data[i+2]; i+=3;
        }
    }
    else {
      if (code >= 0x80) {
        i += data[i] + 1;
      }
    }
  }
}

void processTGAMSerial() {
  while (TGAMSerial.available() > 2) {
    yield(); 
    if (TGAMSerial.read() == 0xAA) {
      if (TGAMSerial.read() == 0xAA) {
        uint8_t pLength = TGAMSerial.read();
        if (pLength > 169) continue;

        uint8_t checkSum = 0;
        int bytesRead = TGAMSerial.readBytes((char*)payloadData, pLength);
        if (bytesRead == pLength) {
          for (int i = 0; i < pLength; i++) {
            checkSum += payloadData[i];
          }
          checkSum = (~checkSum) & 0xFF;

          uint8_t expectedCheckSum = TGAMSerial.read();
          if (checkSum == expectedCheckSum) {
            parseTGAMPacket(payloadData, pLength);
          }
        }
      }
    }
  }
}

void handleRoot() {
  // BẢN FIX DÀNH CHO ESP32-S3: 
  // ESP32-S3 cấp phát động file rất tốt, không cần chia nhỏ file chunk thủ công.
  // Gửi trực tiếp bằng server.send_P sẽ ngăn việc file HTML bị cắt đứt giữa chừng gây lỗi Web quét.
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", INDEX_HTML);
}

// =========================================================================
// KHỞI ĐỘNG HỆ THỐNG ESP32-S3
// =========================================================================
void setup() {
  // 1. Mở Serial và đợi 1.2 giây để USB CDC kết nối ổn định với Terminal
  Serial.begin(115200);
  delay(1200); 

  Serial.println("\n\n==========================================");
  Serial.println("  ESP32-S3 BCI SYSTEM - KHỞI ĐỘNG HỆ THỐNG ");
  Serial.println("==========================================");

  // 2. Cấu hình chân Relay
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  // 3. Khởi tạo UART1 cho TGAM với RX=18, TX=17 trên ESP32-S3
  Serial.println("[UART] Khởi tạo HardwareSerial 1 cho TGAM (RX=18, TX=17)...");
  TGAMSerial.begin(TGAM_BAUDRATE, SERIAL_8N1, TGAM_RX_PIN, TGAM_TX_PIN);

  // 4. Khởi tạo INA226 (an toàn không treo)
  Serial.println("[I2C] Đang kiểm tra cảm biến INA226 (SDA=8, SCL=9)...");
  initINA226();

  secureClient.setInsecure(); 

  // 5. Cấu hình Wi-Fi AP + STA
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.softAP(AP_SSID, AP_PASS);

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

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
    Serial.println("[WIFI] Chưa kết nối được Router, chạy trên SoftAP cục bộ:");
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

  // Tạo Task chạy ngầm trên Core 0 của ESP32-S3
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
  Serial.println("[SYSTEM] Toàn bộ dịch vụ WebServer & WebSocket đã sẵn sàng!");
}

// =========================================================================
// VÒNG LẶP CHÍNH (MAIN LOOP) - CHẠY Ở CORE 1
// =========================================================================
void loop() {
  webSocket.loop();
  server.handleClient();
  processTGAMSerial();

  static unsigned long lastSensorBroadcast = 0;
  if (millis() - lastSensorBroadcast > 1000) {
    lastSensorBroadcast = millis();
    readINA226();

    if (!isSleeping && webSocket.connectedClients() > 0) {
      String tgamData = "TGAM:" + String(signalQuality) + "," + 
                                  String(attention) + "," + 
                                  String(meditation);
      webSocket.broadcastTXT(tgamData);

      String inaData = "INA:" + String(inaBusVoltage, 2) + "," + 
                                String(inaCurrent_mA, 1) + "," + 
                                String(inaPower_mW, 1) + "," + 
                                String(batteryPercent);
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

  if (!isSleeping && (millis() - lastActivityTime > AUTO_SLEEP_TIMEOUT)) {
    isSleeping = true;
    WiFi.setSleep(true);
    webSocket.broadcastTXT("SLEEPING");
    syncTelemetryToCloud();
  }

  if (millis() - lastTelemetrySyncTime > TELEMETRY_INTERVAL) {
    lastTelemetrySyncTime = millis();
    syncTelemetryToCloud();
  }
  
  delay(10); 
}