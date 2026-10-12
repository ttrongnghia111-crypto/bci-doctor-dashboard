#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h> // Thư viện bắt buộc để gọi HTTPS
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESPmDNS.h>

// =========================================================================
// HỆ THỐNG BCI ESP32-S3 - BÀN PHÍM TRỢ NĂNG GIAO TIẾP & ĐIỀU KHIỂN THIẾT BỊ
// =========================================================================

WiFiMulti wifiMulti;
WiFiClientSecure secureClient; // Khai báo Toàn cục để không bị tràn RAM (Lỗi Crash)

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

// 3. Cấu hình chân phần cứng ESP32-S3
#define TGAM_RX_PIN 16
#define TGAM_TX_PIN 17
#define TGAM_BAUDRATE 57600

#define RELAY1_PIN 18 
#define RELAY2_PIN 19 
#define BATTERY_PIN 34 // Chân đọc điện áp Pin (ADC)

WebServer server(80);
WebSocketsServer webSocket(81);
HardwareSerial TGAMSerial(2);

// Trạng thái cảm biến NeuroSky TGAM
volatile int signalQuality = 200; 
volatile int attention = 0;       
volatile int meditation = 0;      
volatile int blinkStrength = 0;   

// Các dải sóng não
unsigned long delta = 0, theta = 0, lowAlpha = 0, highAlpha = 0, lowBeta = 0, highBeta = 0, lowGamma = 0, midGamma = 0;
int batteryPercent = 100;

volatile bool relay1State = false;
volatile bool relay2State = false;

volatile bool isSleeping = false;
volatile int blinkCountForWake = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastActivityTime = 0;
const unsigned long AUTO_SLEEP_TIMEOUT = 900000; // 15 phút (900000 ms) để tự động chuyển sang chế độ ngủ
unsigned long lastTelemetrySyncTime = 0;
const unsigned long TELEMETRY_INTERVAL = 30000;  

uint8_t payloadData[256];

// =========================================================================
// GIAO DIỆN WEB CỤC BỘ DÀNH CHO BỆNH NHÂN (LƯU TRONG BỘ NHỚ FLASH)
// =========================================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>ESP32 BCI - Giao Tiếp & Điều Khiển</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #0a0d14;
            color: #c9d1d9;
            overflow-x: hidden;
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
        }

        #clock {
            position: fixed;
            top: 15px;
            left: 20px;
            text-align: left;
            z-index: 100;
        }
        #clock-time {
            font-size: 32px;
            font-weight: 900;
            color: #58a6ff;
            font-family: monospace;
            line-height: 1;
        }
        #clock-date {
            font-size: 13px;
            color: #8b949e;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-top: 4px;
        }

        /* VIEW 1: MENU ĐIỀU KHIỂN CƠ BẢN */
        #view-menu {
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 100%;
            padding: 70px 20px 40px 20px;
        }
        .page-title {
            font-size: 30px;
            font-weight: 900;
            color: #c9d1d9;
            text-transform: uppercase;
            letter-spacing: 2px;
            margin-bottom: 24px;
            text-align: center;
        }

        .dash-row {
            display: flex;
            gap: 16px;
            width: 90%;
            max-width: 800px;
            margin-bottom: 25px;
        }
        .dash-card {
            background: #161b22;
            border: 2px solid #30363d;
            border-radius: 14px;
            padding: 18px 20px;
            flex: 1;
            box-shadow: 0 8px 16px rgba(0,0,0,0.4);
            text-align: center;
        }
        .dash-title {
            font-size: 14px;
            color: #8b949e;
            font-weight: bold;
            margin-bottom: 6px;
            text-transform: uppercase;
        }
        .dash-value {
            font-size: 44px;
            font-weight: 900;
            color: #58a6ff;
            font-family: monospace;
        }
        .dash-value.good { color: #3fb950; }
        .dash-value.bad { color: #f85149; }
        .dash-value.warn { color: #d29922; }

        .menu-actions {
            display: flex;
            gap: 14px;
            flex-wrap: wrap;
            justify-content: center;
            width: 90%;
            max-width: 800px;
        }
        .btn-main {
            background: #238636;
            color: #fff;
            border: none;
            padding: 18px 36px;
            font-size: 22px;
            font-weight: bold;
            border-radius: 14px;
            cursor: pointer;
            box-shadow: 0 6px 14px rgba(0,0,0,0.4);
            transition: 0.2s;
            width: 100%;
            max-width: 500px;
            margin-bottom: 12px;
        }
        .btn-main:active { transform: scale(0.97); background: #2ea043; }

        .btn-sub {
            background: #21262d;
            color: #c9d1d9;
            border: 2px solid #30363d;
            padding: 14px 22px;
            font-size: 17px;
            font-weight: bold;
            border-radius: 10px;
            cursor: pointer;
            transition: 0.2s;
            flex: 1;
            min-width: 140px;
        }
        .btn-sub:active { transform: scale(0.97); background: #30363d; }

        /* VIEW 2: BÀN PHÍM QUÉT TRỢ NĂNG */
        #view-keyboard {
            display: none;
            flex-direction: column;
            width: 100%;
            height: 100vh;
        }
        #tgam-status-mini {
            position: fixed;
            top: 15px;
            right: 20px;
            text-align: right;
            font-size: 14px;
            color: #8b949e;
            line-height: 1.4;
            z-index: 50;
        }
        .tgam-val {
            font-weight: 900;
            color: #3fb950;
            font-size: 17px;
            font-family: monospace;
        }

        #top-controls {
            width: 94%;
            margin: 50px auto 0 auto;
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
        }
        .top-btn {
            background: #2d3139;
            color: #a5d6ff;
            padding: 10px 18px;
            border-radius: 8px;
            font-size: 18px;
            font-weight: 800;
            cursor: pointer;
            border: 2px solid #30363d;
            transition: 0.2s;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        .top-btn:active { background: #58a6ff; color: #0d1117; transform: scale(0.95); }
        .btn-back { background: #da3633; color: #fff; border-color: #b62324; }
        .helper-text { color: #8b949e; font-size: 14px; font-weight: bold; }

        #output-box {
            width: 94%;
            height: 65px;
            font-size: 32px;
            margin: 12px auto;
            padding: 10px 20px;
            background: #161b22;
            border-radius: 12px;
            border: 2px solid #30363d;
            color: #58a6ff;
            text-transform: uppercase;
            font-weight: 800;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 4px 15px rgba(0,0,0,0.5);
            white-space: pre-wrap;
            word-break: break-word;
            font-family: monospace;
        }

        .grid { display: flex; flex-direction: column; gap: 8px; width: 96%; margin: 0 auto; }
        .row { display: flex; gap: 8px; justify-content: center; padding: 4px; border-radius: 12px; border: 3px solid transparent; }
        .cell {
            flex: 1;
            padding: 18px 4px;
            background: #21262d;
            border-radius: 8px;
            font-size: 24px;
            font-weight: 800;
            transition: 0.1s;
            border: 2px solid #30363d;
            color: #c9d1d9;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
            display: flex;
            justify-content: center;
            align-items: center;
            cursor: pointer;
        }

        .cell.suggest { background-color: #1f3a28; border-color: #2ea043; color: #56d364; font-size: 22px; }
        .cell.sys { background-color: #2d3139; color: #a5d6ff; font-size: 17px; }
        .cell.iot { background-color: #3d2222; border-color: #8b2828; font-size: 17px; }

        .scanning-row { border-color: #58a6ff; background-color: rgba(88, 166, 255, 0.05); }
        .scanning-cell {
            background-color: #58a6ff !important;
            color: #0d1117 !important;
            transform: scale(1.05);
            border-color: #fff;
            box-shadow: 0 0 20px rgba(88, 166, 255, 0.8);
            z-index: 10;
        }

        .overlay {
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background: rgba(0,0,0,0.95);
            z-index: 1000;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            display: none;
        }
        .sleep-text { font-size: 38px; font-weight: bold; color: #8b949e; margin-bottom: 20px; text-align: center; }
        .wake-hint { font-size: 22px; color: #56d364; font-weight: bold; animation: pulse 1.5s infinite; }
        @keyframes pulse { 0% { opacity: 0.5; } 50% { opacity: 1; } 100% { opacity: 0.5; } }

        #settings-panel {
            background: #161b22;
            border: 2px solid #30363d;
            border-radius: 15px;
            padding: 28px;
            width: 85%;
            max-width: 600px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.8);
            text-align: left;
        }
        .set-title {
            font-size: 24px;
            color: #58a6ff;
            font-weight: bold;
            margin-bottom: 16px;
            text-align: center;
            border-bottom: 1px solid #30363d;
            padding-bottom: 10px;
        }
        .set-group { margin-bottom: 18px; }
        .set-label { font-size: 16px; color: #c9d1d9; font-weight: bold; margin-bottom: 8px; display: block; }
        .set-select, .set-slider {
            width: 100%;
            padding: 10px;
            font-size: 16px;
            background: #0d1117;
            color: #c9d1d9;
            border: 1px solid #30363d;
            border-radius: 8px;
        }
        .set-val { float: right; color: #56d364; }
        .set-btns { display: flex; justify-content: space-between; margin-top: 24px; gap: 10px; }
        .btn-test { background: #2ea043; color: #fff; padding: 12px 20px; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; }
        .btn-close { background: #da3633; color: #fff; padding: 12px 20px; border: none; border-radius: 8px; font-size: 16px; font-weight: bold; cursor: pointer; }

        #status-bar {
            position: fixed;
            bottom: 0;
            left: 0;
            width: 100%;
            padding: 6px 14px;
            background: #07090e;
            border-top: 1px solid #18202d;
            font-size: 12px;
            color: #8b949e;
            text-align: center;
            z-index: 90;
        }
    </style>
</head>
<body>
    <div id="clock">
        <div id="clock-time">--:--:--</div>
        <div id="clock-date"></div>
    </div>

    <!-- MÀN HÌNH CHẾ ĐỘ NGỦ -->
    <div id="sleep-overlay" class="overlay">
        <div class="sleep-text" id="sleep-msg">Zzz... Hệ thống đang ngủ.</div>
        <div class="wake-hint">Nháy mắt 4 lần liên tiếp để đánh thức</div>
    </div>

    <!-- CÀI ĐẶT GIỌNG ĐỌC -->
    <div id="settings-overlay" class="overlay">
        <div id="settings-panel">
            <div class="set-title">CÀI ĐẶT GIỌNG ĐỌC (TTS)</div>
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
                <button class="btn-test" onclick="testVoice()">Thử giọng</button>
                <button class="btn-close" onclick="closeSettings()">Lưu & Đóng</button>
            </div>
        </div>
    </div>

    <!-- VIEW 1: MAIN MENU -->
    <div id="view-menu">
        <h1 class="page-title">BẢNG THEO DÕI TÌNH TRẠNG BỆNH NHÂN</h1>

        <div class="dash-row">
            <div class="dash-card">
                <div class="dash-title">Tín Hiệu (Signal)</div>
                <div class="dash-value bad" id="sig-val">CHƯA GẮN</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Tập Trung (Attention)</div>
                <div class="dash-value" id="att-val">0</div>
            </div>
            <div class="dash-card">
                <div class="dash-title">Thư Giãn (Meditation)</div>
                <div class="dash-value" id="med-val">0</div>
            </div>
        </div>

        <div class="menu-actions">
            <button class="btn-main" onclick="showKeyboard()">⌨️ MỞ BÀN PHÍM GIAO TIẾP</button>
            <div style="display: flex; gap: 12px; width: 100%; max-width: 500px;">
                <button class="btn-sub" onclick="openSettings()">⚙️ CÀI ĐẶT GIỌNG</button>
                <button class="btn-sub" onclick="toggleRelay(1)">💡 ĐÈN: <span id="menu-r1">TẮT</span></button>
                <button class="btn-sub" onclick="toggleRelay(2)">🌀 QUẠT: <span id="menu-r2">TẮT</span></button>
            </div>
        </div>
    </div>

    <!-- VIEW 2: KEYBOARD -->
    <div id="view-keyboard">
        <div id="tgam-status-mini">
            Tín hiệu: <span id="sig-mini" class="tgam-val" style="color:#f85149">200</span> | 
            Tập trung: <span id="att-mini" class="tgam-val">0</span> | 
            Thư giãn: <span id="med-mini" class="tgam-val">0</span>
        </div>

        <div id="top-controls">
            <button class="top-btn btn-back" onclick="showMenu()">⬅ QUAY LẠI MENU</button>
            <div class="helper-text">Nháy mắt mạnh (hoặc bấm Space) để CHỌN</div>
            <button class="top-btn" id="top-btn-speed" onclick="toggleSpeed()">TỐC ĐỘ: VỪA</button>
        </div>

        <div id="output-box"></div>

        <div class="grid" id="main-grid">
            <div class="row">
                <div class="cell suggest word-suggest" id="sug1">TÔI</div>
                <div class="cell suggest word-suggest" id="sug2">CẦN</div>
                <div class="cell suggest word-suggest" id="sug3">MỆT</div>
                <div class="cell suggest word-suggest" id="sug4">ĐAU</div>
            </div>

            <div class="row">
                <div class="cell char">A</div><div class="cell char">Ă</div><div class="cell char">Â</div>
                <div class="cell char">B</div><div class="cell char">C</div><div class="cell char">D</div>
                <div class="cell char">Đ</div><div class="cell char">E</div><div class="cell char">Ê</div>
            </div>

            <div class="row">
                <div class="cell char">G</div><div class="cell char">H</div><div class="cell char">I</div>
                <div class="cell char">K</div><div class="cell char">L</div><div class="cell char">M</div>
                <div class="cell char">N</div><div class="cell char">O</div><div class="cell char">Ô</div>
            </div>

            <div class="row">
                <div class="cell char">Ơ</div><div class="cell char">P</div><div class="cell char">Q</div>
                <div class="cell char">R</div><div class="cell char">S</div><div class="cell char">T</div>
                <div class="cell char">U</div><div class="cell char">Ư</div><div class="cell char">V</div>
            </div>

            <div class="row">
                <div class="cell char">X</div><div class="cell char">Y</div>
                <div class="cell sys">DẤU CÁCH</div>
                <div class="cell sys">XÓA KÝ TỰ</div>
                <div class="cell sys">XÓA HẾT</div>
                <div class="cell sys" style="background-color:#1b4728; border-color:#2ea043; color:#7ee787;">ĐỌC CÂU</div>
            </div>

            <div class="row">
                <div class="cell sys" id="grid-btn-speed">TỐC ĐỘ: VỪA</div>
                <div class="cell iot" id="grid-btn-r1">BẬT ĐÈN</div>
                <div class="cell iot" id="grid-btn-r2">BẬT QUẠT</div>
                <div class="cell sys" style="background-color:#48261e; color:#ffa198;">NGỦ</div>
                <div class="cell sys" style="background-color:#da3633; color:#fff; font-size:24px;">SOS</div>
            </div>
        </div>
    </div>

    <div id="status-bar">Đang kết nối WebSocket...</div>

    <script>
        const hostIP = window.location.hostname || "esp32-bci.local";
        let ws;
        let currentView = 'MENU';

        function connectWebSocket() {
            ws = new WebSocket(`ws://${hostIP}:81`);

            ws.onopen = () => { 
                document.getElementById('status-bar').innerHTML = "Trạng thái: <span style='color:#3fb950'>ĐÃ KẾT NỐI WEBSOCKET ESP32</span>"; 
            };
            
            ws.onclose = () => { 
                document.getElementById('status-bar').innerHTML = "Trạng thái: <span style='color:#f85149'>MẤT KẾT NỐI ESP32 (ĐANG THỬ LẠI...)</span>"; 
                // Gọi lại chính hàm này để tạo kết nối mới VÀ gán lại toàn bộ sự kiện
                setTimeout(connectWebSocket, 2500); 
            };

            ws.onmessage = (event) => {
                const data = event.data;
                if (data.startsWith("BLINK:")) {
                    handleBlink();
                } else if (data.startsWith("TGAM:")) {
                    let parts = data.substring(5).split(',');
                    if (parts.length >= 3) {
                        let sig = parseInt(parts[0]);
                        let att = parseInt(parts[1]);
                        let med = parseInt(parts[2]);
                        updateTGAMDisplay(sig, att, med);
                    }
                } else if (data.startsWith("RELAY:")) {
                    let parts = data.substring(6).split(',');
                    updateRelayUI(1, parts[0] === '1');
                    updateRelayUI(2, parts[1] === '1');
                } else if (data === "SLEEPING") {
                    enterSleepMode();
                } else if (data === "WAKEUP") {
                    exitSleepMode();
                }
            };
        }

        // Khởi chạy kết nối lần đầu tiên
        connectWebSocket();

        function updateClock() {
            const now = new Date();
            document.getElementById('clock-time').innerText = 
                now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false });
            document.getElementById('clock-date').innerText = 
                `${now.getDate().toString().padStart(2,'0')}/${(now.getMonth()+1).toString().padStart(2,'0')}/${now.getFullYear()}`;
        }
        setInterval(updateClock, 1000);
        updateClock();

        function sendCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(cmd);
            }
        }

        ws.onmessage = (event) => {
            const data = event.data;
            if (data.startsWith("BLINK:")) {
                handleBlink();
            } else if (data.startsWith("TGAM:")) {
                let parts = data.substring(5).split(',');
                if (parts.length >= 3) {
                    let sig = parseInt(parts[0]);
                    let att = parseInt(parts[1]);
                    let med = parseInt(parts[2]);
                    updateTGAMDisplay(sig, att, med);
                }
            } else if (data.startsWith("RELAY:")) {
                let parts = data.substring(6).split(',');
                updateRelayUI(1, parts[0] === '1');
                updateRelayUI(2, parts[1] === '1');
            } else if (data === "SLEEPING") {
                enterSleepMode();
            } else if (data === "WAKEUP") {
                exitSleepMode();
            }
        };

        function updateTGAMDisplay(sig, att, med) {
            let elSig = document.getElementById('sig-val');
            let elSigMini = document.getElementById('sig-mini');

            if (sig === 0) {
                elSig.innerText = "TỐT (0)"; elSig.className = "dash-value good";
                elSigMini.innerText = "TỐT (0)"; elSigMini.style.color = "#3fb950";
            } else if (sig === 200) {
                elSig.innerText = "RỚT ĐIỆN CỰC"; elSig.className = "dash-value bad";
                elSigMini.innerText = "RỚT (200)"; elSigMini.style.color = "#f85149";
            } else {
                elSig.innerText = `KÉM (${sig})`; elSig.className = "dash-value warn";
                elSigMini.innerText = `KÉM (${sig})`; elSigMini.style.color = "#d29922";
            }

            document.getElementById('att-val').innerText = att;
            document.getElementById('med-val').innerText = med;
            document.getElementById('att-mini').innerText = att;
            document.getElementById('med-mini').innerText = med;
        }

        function showMenu() {
            currentView = 'MENU';
            document.getElementById('view-keyboard').style.display = 'none';
            document.getElementById('view-menu').style.display = 'flex';
            clearInterval(scanInterval); 
            speakVoice("Quay lại menu chính");
        }

        function showKeyboard() {
            currentView = 'KEYBOARD';
            document.getElementById('view-menu').style.display = 'none';
            document.getElementById('view-keyboard').style.display = 'flex';
            scanState = 'ROW'; 
            currentRow = 0; 
            startScanning(); 
            speakVoice("Mở bàn phím giao tiếp");
        }

        // ==========================================
        // QUẢN LÝ GIỌNG ĐỌC FIFO CHỐNG MẤT TỪ
        // ==========================================
        let voices = [];
        let ttsRate = parseFloat(localStorage.getItem('ttsRate')) || 0.85;
        let ttsVolume = parseFloat(localStorage.getItem('ttsVolume')) || 1.0;
        let savedVoiceURI = localStorage.getItem('ttsVoiceURI');
        let isSettingsOpen = false;

        let speechQueue = [];
        let isSpeechPlaying = false;
        let activeUtterances = []; 
        let ttsWatchdog = null;

        document.getElementById('rate-slider').value = ttsRate;
        document.getElementById('rate-val').innerText = ttsRate;
        document.getElementById('vol-slider').value = ttsVolume;
        document.getElementById('vol-val').innerText = Math.round(ttsVolume * 100) + '%';

        function populateVoices() {
            if (!('speechSynthesis' in window)) return;
            voices = window.speechSynthesis.getVoices();
            const select = document.getElementById('voice-select');
            select.innerHTML = '';

            let viVoices = voices.filter(v => v.lang.includes('vi') || v.lang.includes('VN'));
            let defaultVoices = viVoices.length > 0 ? viVoices : voices;

            defaultVoices.forEach((v) => {
                const opt = document.createElement('option');
                opt.value = v.voiceURI;
                opt.textContent = `${v.name} (${v.lang})`;
                if (savedVoiceURI && v.voiceURI === savedVoiceURI) opt.selected = true;
                select.appendChild(opt);
            });
        }
        if ('speechSynthesis' in window) {
            window.speechSynthesis.onvoiceschanged = populateVoices;
            populateVoices();
        }

        function processNextSpeech() {
            if (speechQueue.length === 0) {
                isSpeechPlaying = false;
                activeUtterances = [];
                if (ttsWatchdog) {
                    clearTimeout(ttsWatchdog);
                    ttsWatchdog = null;
                }
                return;
            }

            if (!('speechSynthesis' in window)) {
                speechQueue = [];
                isSpeechPlaying = false;
                return;
            }

            isSpeechPlaying = true;
            const item = speechQueue.shift();
            const msg = new SpeechSynthesisUtterance(item.text);

            msg.rate = ttsRate;
            msg.volume = ttsVolume;

            if (voices.length > 0) {
                const curURI = document.getElementById('voice-select').value;
                const v = voices.find(vox => vox.voiceURI === curURI);
                if (v) msg.voice = v;
                else {
                    const viVoice = voices.find(vox => vox.lang.includes('vi') || vox.lang.includes('VN'));
                    if (viVoice) msg.voice = viVoice;
                }
            }

            activeUtterances.push(msg);

            let finished = false;
            const completeSpeech = () => {
                if (finished) return;
                finished = true;
                if (ttsWatchdog) {
                    clearTimeout(ttsWatchdog);
                    ttsWatchdog = null;
                }
                setTimeout(() => {
                    processNextSpeech();
                }, 50);
            };

            msg.onend = completeSpeech;
            msg.onerror = completeSpeech;

            let estimatedDuration = Math.max(1200, item.text.length * 150 + 500);
            ttsWatchdog = setTimeout(() => {
                if (!finished) {
                    completeSpeech();
                }
            }, estimatedDuration);

            try {
                window.speechSynthesis.speak(msg);
            } catch(e) {
                completeSpeech();
            }
        }

        function speakVoice(text, immediate = false) {
            if (!('speechSynthesis' in window)) return;
            if (!text || text.trim() === '') return;

            if (immediate) {
                speechQueue = [];
                activeUtterances = [];
                if (ttsWatchdog) {
                    clearTimeout(ttsWatchdog);
                    ttsWatchdog = null;
                }
                try {
                    window.speechSynthesis.cancel();
                } catch(e) {}
                isSpeechPlaying = false;
            }

            if (speechQueue.length >= 8) {
                speechQueue.shift();
            }
            speechQueue.push({ text: text.trim() });

            if (!isSpeechPlaying) {
                processNextSpeech();
            }
        }

        const speeds = [1500, 1000, 600];  
        const speedLabels = ["CHẬM", "VỪA", "NHANH"];
        let speedIdx = 1;  
        let scanInterval;

        function toggleSpeed() {
            speedIdx = (speedIdx + 1) % 3;
            const newLabel = "TỐC ĐỘ: " + speedLabels[speedIdx];
            document.getElementById('top-btn-speed').innerText = newLabel;
            document.getElementById('grid-btn-speed').innerText = newLabel;
            speakVoice(newLabel);
            if (!isSleeping && !isSettingsOpen && currentView === 'KEYBOARD') {
                clearInterval(scanInterval);
                scanInterval = setInterval(tick, speeds[speedIdx]);
            }
        }

        // ==========================================
        // DUAL-LAYER CONTEXT: 1-GRAM & 2-GRAM
        // ==========================================
        const dictionary = [
            "TÔI", "BẠN", "MẸ", "CHA", "BÁC SĨ", "Y TÁ", "CON", "CHỒNG", "VỢ", "NHÀ", "BỆNH VIỆN", 
            "CẦN", "MUỐN", "THÍCH", "LÀM", "ĂN", "UỐNG", "ĐI", "NGỦ", "TẮM", "KHÁM", "MỞ", "BẬT", "TẮT", "LẤY", "GIÚP", "NGỒI", "NẰM",
            "MỆT", "ĐAU", "NHỨC", "MỎI", "NGỨA", "HO", "THỞ", "KHÁT", "ĐÓI", "NÓNG", "LẠNH", "KHỎE", "YẾU", "CHÓNG MẶT", "BUỒN NÔN",
            "NƯỚC", "CƠM", "MÌ", "PHỞ", "CHÁO", "SỮA", "THUỐC", "ÁO", "QUẦN", "GIƯỜNG", "ĐÈN", "QUẠT", "TV", "CHĂN", "GỐI",
            "ĐẦU", "BỤNG", "TAY", "CHÂN", "LƯNG", "NGỰC", "MẮT", "CỔ", "MIỆNG",
            "ĐÚNG", "SAI", "RẤT", "QUÁ", "KHÔNG", "CÓ", "CHƯA", "RỒI", "ĐANG", "SẼ", "XIN LỖI", "CẢM ƠN", "VÂNG", "DẠ", "NHIỀU", "ÍT", "NHANH", "CHẬM"
        ];

        const bigrams = {
            "TÔI": ["CẦN", "MỆT", "ĐAU", "KHÁT", "ĐÓI", "MUỐN", "LẠNH", "NÓNG"],
            "CẦN": ["GIÚP", "UỐNG", "ĂN", "NGỦ", "ĐI", "THUỐC", "LẤY", "BÁC SĨ"],
            "MUỐN": ["UỐNG", "ĂN", "VỆ SINH", "NGỦ", "TẮM", "VỀ", "NGỒI", "NẰM"],
            "ĐAU": ["ĐẦU", "BỤNG", "LƯNG", "CHÂN", "TAY", "NGỰC", "MẮT", "CỔ"],
            "UỐNG": ["NƯỚC", "SỮA", "THUỐC"],
            "ĂN": ["CƠM", "CHÁO", "PHỞ", "MÌ"],
            "ĐI": ["VỆ SINH", "NGỦ", "VIỆN", "VỀ", "TẮM"],
            "BẬT": ["ĐÈN", "QUẠT", "TV", "MÁY"],
            "TẮT": ["ĐÈN", "QUẠT", "TV", "MÁY"],
            "BÁC SĨ": ["ƠI", "KHÁM", "CHỮA", "GIÚP"],
            "LẤY": ["NƯỚC", "THUỐC", "CHĂN", "GỐI", "ÁO"],
            "CẢM": ["ƠN", "LẠNH", "THẤY", "GIÁC"],
            "XIN": ["LỖI", "NƯỚC", "THÊM", "GIÚP"],
            "RẤT": ["MỆT", "ĐAU", "ĐÓI", "KHÁT", "NÓNG", "LẠNH"],
            "KHÔNG": ["ĂN", "UỐNG", "CẦN", "THỞ", "ĐƯỢC", "SAO", "KỊP"],
            "CÓ": ["ĐAU", "MỆT", "AI", "THỂ"],
            "MỎI": ["LƯNG", "CHÂN", "TAY", "CỔ", "MẮT"],
            "KHÓ": ["THỞ", "CHỊU", "NGỦ", "ĂN", "UỐNG"]
        };

        function removeAccents(str) { 
            let clean = str.normalize("NFD").replace(/[\u0300-\u036f]/g, ""); 
            return clean.replace(/Đ/g, "D").replace(/đ/g, "d");
        }

        // ==========================================
        // XỬ LÝ GÕ TELEX TIẾNG VIỆT THỜI GIAN THỰC
        // ==========================================
        function processTelex(text) {
            let words = text.split(' ');
            let lastWord = words[words.length - 1];
            if (!lastWord) return text;
            lastWord = lastWord.replace(/A[WA]/gi, m => (m[1]==='W'||m[1]==='w')?'Ă':'Â')
                               .replace(/E[E]/gi, 'Ê').replace(/O[O]/gi, 'Ô')
                               .replace(/O[W]/gi, 'Ơ').replace(/U[W]/gi, 'Ư').replace(/D[D]/gi, 'Đ');

            const tones = {'S':'\u0301', 'F':'\u0300', 'R':'\u0309', 'X':'\u0303', 'J':'\u0323'};
            let lastChar = lastWord.slice(-1).toUpperCase();
            if (tones[lastChar]) {
                let baseWord = lastWord.slice(0, -1);
                baseWord = baseWord.normalize('NFD').replace(/[\u0300-\u036f]/g, '');
                let vowels = 'AĂÂEÊIOÔƠUƯY';
                let targetIdx = -1;
                for (let i = 0; i < baseWord.length; i++) { if (vowels.includes(baseWord[i])) targetIdx = i; }
                if (targetIdx !== -1) {
                    baseWord = baseWord.slice(0, targetIdx+1) + tones[lastChar] + baseWord.slice(targetIdx+1);
                    lastWord = baseWord.normalize('NFC');
                } else { lastWord = baseWord + lastWord.slice(-1); }
            }
            words[words.length - 1] = lastWord;
            return words.join(' ');
        }

        function updatePredictiveText() {
            const outText = document.getElementById('output-box').textContent;
            const words = outText.trim().split(/\s+/);
            const currentPrefix = outText.endsWith(' ') ? '' : (words[words.length - 1] || '').toUpperCase();
            
            let lastCompleteWord = '';
            if (outText.endsWith(' ')) {
                lastCompleteWord = (words[words.length - 1] || '').toUpperCase();
            } else if (words.length >= 2) {
                lastCompleteWord = words[words.length - 2].toUpperCase();
            }

            let matches = [];

            if (currentPrefix === '') {
                if (lastCompleteWord && bigrams[lastCompleteWord]) {
                    matches = [...bigrams[lastCompleteWord]];
                } else { matches = ["TÔI", "CẦN", "MỆT", "ĐAU"]; }
            } else {
                const cleanPrefix = removeAccents(currentPrefix);
                if (lastCompleteWord && bigrams[lastCompleteWord]) {
                    const contextMatches = bigrams[lastCompleteWord].filter(w => removeAccents(w).startsWith(cleanPrefix) && w !== currentPrefix);
                    matches.push(...contextMatches);
                }
                const generalMatches = dictionary.filter(w => removeAccents(w).startsWith(cleanPrefix) && w !== currentPrefix);
                generalMatches.forEach(w => { if (!matches.includes(w)) matches.push(w); });
            }

            if (matches.length < 4) {
                dictionary.forEach(w => { if (!matches.includes(w) && w !== currentPrefix && matches.length < 4) matches.push(w); });
            }

            document.getElementById('sug1').innerText = matches[0] || '';
            document.getElementById('sug2').innerText = matches[1] || '';
            document.getElementById('sug3').innerText = matches[2] || '';
            document.getElementById('sug4').innerText = matches[3] || '';
        }

        function applySuggestion(word) {
            const outBox = document.getElementById('output-box');
            let text = outBox.textContent;
            
            if (text === '' || text.endsWith(' ')) {
                outBox.textContent = text + word + ' ';
            } else {
                let words = text.trim().split(/\s+/);
                words.pop();
                words.push(word);
                outBox.textContent = words.join(' ') + ' ';
            }
            updatePredictiveText();
        }

        const rows = document.querySelectorAll('.row');
        let currentRow = 0, currentCol = 0;
        let scanState = 'ROW'; 
        let loopCount = 0;     
        let isSleeping = false;

        function renderScan() { 
            rows.forEach(r => { 
                r.classList.remove('scanning-row'); 
                Array.from(r.children).forEach(c => c.classList.remove('scanning-cell')); 
            }); 
            
            if (scanState === 'ROW') { 
                rows[currentRow].classList.add('scanning-row'); 
            } else { 
                rows[currentRow].classList.add('scanning-row'); 
                rows[currentRow].children[currentCol].classList.add('scanning-cell'); 
            } 
        }

        function tick() { 
            if (isSleeping || isSettingsOpen || currentView !== 'KEYBOARD') return; 
            
            if (scanState === 'ROW') { 
                currentRow = (currentRow + 1) % rows.length; 
            } else if (scanState === 'CELL') {
                currentCol++;
                if (currentCol >= rows[currentRow].children.length) {
                    loopCount++;
                    if (loopCount >= 2) {
                        scanState = 'ROW'; 
                        currentCol = 0; 
                        loopCount = 0;
                    } else {
                        currentCol = 0;
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

        function executeAction(element) {
            const text = element.innerText.trim();
            if (text === '') return; 

            const outBox = document.getElementById('output-box');

            if (element.classList.contains('word-suggest')) { 
                applySuggestion(text); 
                speakVoice(text); 
            }
            else if (element.classList.contains('char')) { 
                outBox.textContent += text; 
                outBox.textContent = processTelex(outBox.textContent); 
                updatePredictiveText(); 
                speakVoice(text); 
            }
            else if (text === "DẤU CÁCH") { 
                if (!outBox.textContent.endsWith(' ') && outBox.textContent !== '') {
                    outBox.textContent += " "; 
                    updatePredictiveText(); 
                }
                speakVoice("Dấu cách"); 
            }
            else if (text === "XÓA KÝ TỰ") { 
                outBox.textContent = outBox.textContent.slice(0, -1); 
                updatePredictiveText(); 
                speakVoice("Xóa"); 
            }
            else if (text === "XÓA HẾT") { 
                outBox.textContent = ""; 
                updatePredictiveText(); 
                speakVoice("Xóa hết"); 
            }
            else if (text === "ĐỌC CÂU") { 
                let phrase = outBox.textContent.trim();
                if (phrase !== "") {
                    speakVoice(phrase, true);
                    sendCommand("SENTENCE:" + phrase);
                } else {
                    speakVoice("Chưa có nội dung");
                }
            }
            else if (text.startsWith("TỐC ĐỘ")) { toggleSpeed(); }
            else if (text.includes("ĐÈN")) { toggleRelay(1); }
            else if (text.includes("QUẠT")) { toggleRelay(2); }
            else if (text === "NGỦ") { enterSleepMode(); sendCommand('S'); }
            else if (text === "SOS") { 
                speakVoice("CẤP CỨU! CẦN BÁC SĨ GIÚP ĐỠ GẤP!", true); 
                sendCommand("SOS"); 
            }

            scanState = 'ROW';
            currentCol = 0;
            loopCount = 0;
            renderScan();
        }

        function handleBlink() {
            if (isSleeping || isSettingsOpen) return;
            if (currentView === 'MENU') {
                showKeyboard();
                return;
            }
            if (scanState === 'ROW') {
                scanState = 'CELL';
                currentCol = 0;
                loopCount = 0;
                renderScan();
            } else if (scanState === 'CELL') {
                const targetCell = rows[currentRow].children[currentCol];
                executeAction(targetCell);
            }
        }

        document.querySelectorAll('.cell').forEach(cell => {
            cell.addEventListener('click', (e) => {
                e.stopPropagation();
                executeAction(cell);
            });
        });

        function toggleRelay(num) {
            sendCommand('R' + num);
        }
        function updateRelayUI(num, state) {
            let label = state ? "BẬT" : "TẮT";
            let actionLabel = state ? "TẮT" : "BẬT";
            let color = state ? "#3fb950" : "#8b949e";

            let mEl = document.getElementById('menu-r' + num);
            if (mEl) { mEl.innerText = label; mEl.style.color = color; }

            let kEl = document.getElementById('grid-btn-r' + num);
            if (kEl) {
                let name = num === 1 ? "ĐÈN" : "QUẠT";
                kEl.innerText = `${actionLabel} ${name}`;
                kEl.style.backgroundColor = state ? "#1f3a28" : "#3d2222";
                kEl.style.borderColor = state ? "#2ea043" : "#8b2828";
            }
        }

        function enterSleepMode() {
            isSleeping = true;
            document.getElementById('sleep-overlay').style.display = 'flex';
            clearInterval(scanInterval);
            speakVoice("Hệ thống chuyển sang chế độ ngủ");
        }
        function exitSleepMode() {
            isSleeping = false;
            document.getElementById('sleep-overlay').style.display = 'none';
            if (currentView === 'KEYBOARD') startScanning();
            speakVoice("Hệ thống đã thức dậy");
        }

        function openSettings() {
            isSettingsOpen = true;
            document.getElementById('settings-overlay').style.display = 'flex';
            clearInterval(scanInterval);
        }
        function closeSettings() {
            isSettingsOpen = false;
            ttsRate = parseFloat(document.getElementById('rate-slider').value);
            ttsVolume = parseFloat(document.getElementById('vol-slider').value);
            savedVoiceURI = document.getElementById('voice-select').value;
            localStorage.setItem('ttsRate', ttsRate);
            localStorage.setItem('ttsVolume', ttsVolume);
            localStorage.setItem('ttsVoiceURI', savedVoiceURI);
            document.getElementById('settings-overlay').style.display = 'none';
            if (currentView === 'KEYBOARD' && !isSleeping) startScanning();
        }
        function testVoice() {
            let curRate = parseFloat(document.getElementById('rate-slider').value);
            let curVol = parseFloat(document.getElementById('vol-slider').value);
            let oldR = ttsRate, oldV = ttsVolume;
            ttsRate = curRate; ttsVolume = curVol;
            speakVoice("Kiểm tra thử nghiệm âm thanh giọng nói tiếng Việt", true);
            ttsRate = oldR; ttsVolume = oldV;
        }

        document.getElementById('rate-slider').oninput = function() {
            document.getElementById('rate-val').innerText = this.value;
        };
        document.getElementById('vol-slider').oninput = function() {
            document.getElementById('vol-val').innerText = Math.round(this.value * 100) + '%';
        };

      window.addEventListener('keydown', (e) => {
            if (e.code === 'Space') {
                e.preventDefault();
                if (isSleeping) {
                    sendCommand('W');
                } else {
                    // THÊM DÒNG NÀY: Gửi lệnh báo cho ESP32 biết là đang test
                    sendCommand('SIMULATE_BLINK'); 
                    handleBlink();
                }
            }
        });

        updatePredictiveText();
    </script>
</body>
</html>
)rawliteral";

// =========================================================================
// BIẾN GIAO TIẾP ĐA LUỒNG (FREERTOS) - THÊM MỚI
// =========================================================================
String pendingCloudSentence = "";
bool pendingTelemetry = false;

// =========================================================================
// TASK FREERTOS: XỬ LÝ GỬI DỮ LIỆU LÊN CLOUD (CHẠY NGẦM Ở CORE 0)
// =========================================================================
void cloudSyncTask(void * parameter) {
  for(;;) {
    // 1. Gửi câu giao tiếp lên Cloud
    if (pendingCloudSentence != "") {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(secureClient, CLOUD_SERVER_URL);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(5000); // Timeout 5 giây

        String jsonPayload = "{\"sentence\":\"" + pendingCloudSentence + "\",\"attention\":" + String(attention) + ",\"meditation\":" + String(meditation) + ",\"signalQuality\":" + String(signalQuality) + "}";
        int httpResponseCode = http.POST(jsonPayload);
        
        if (httpResponseCode > 0) {
          Serial.printf("[CLOUD SYNC] Đã đồng bộ lên Render (HTTP %d): %s\n", httpResponseCode, pendingCloudSentence.c_str());
        } else {
          Serial.printf("[CLOUD SYNC] Lỗi gửi Render: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
      }
      pendingCloudSentence = ""; // Xóa sau khi gửi xong
    }

    // 2. Gửi Telemetry (Heartbeat, Pin, Sóng não) lên Cloud
    if (pendingTelemetry) {
      if (WiFi.status() == WL_CONNECTED) {
        int rawBat = analogRead(BATTERY_PIN);
        batteryPercent = map(rawBat, 0, 4095, 0, 100); 
        if(batteryPercent > 100) batteryPercent = 100;
        if(batteryPercent < 0) batteryPercent = 0;

        HTTPClient http;
        http.begin(secureClient, CLOUD_TELEMETRY_URL);
        http.addHeader("Content-Type", "application/json");
        http.setTimeout(5000); // Timeout 5 giây

        String payload = "{\"isSleeping\":" + String(isSleeping ? "true" : "false") + "," +
                         "\"signalQuality\":" + String(signalQuality) + "," +
                         "\"battery\":" + String(batteryPercent) + "," +
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
          Serial.printf("[TELEMETRY] Đã gửi Heartbeat lên Render (HTTP %d)\n", httpCode);
        }
        http.end();
      }
      pendingTelemetry = false; // Xóa cờ sau khi gửi xong
    }

    // Nhường CPU cho các tác vụ khác của Core 0 (Rất quan trọng để không bị crash)
    vTaskDelay(100 / portTICK_PERIOD_MS); 
  }
}

// Hàm gọi từ Core 1 để kích hoạt Task ở Core 0
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
        WiFi.setSleep(true); // Modem sleep tiết kiệm pin
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
      // ---- BẠN COPY VÀ DÁN ĐOẠN NÀY VÀO ĐÂY ----
      else if (msg == "SIMULATE_BLINK") {
        // Giả lập có sóng não tốt và các chỉ số ngẫu nhiên
        signalQuality = 0;
        attention = random(60, 100);
        meditation = random(60, 100);
        delta = random(100000, 500000);
        theta = random(50000, 100000);
        lowAlpha = random(10000, 50000);
        lowBeta = random(10000, 30000);
        lowGamma = random(1000, 10000);
        blinkStrength = random(50, 100);
        
        // Báo cho Web Local chớp mắt
        webSocket.broadcastTXT("BLINK:" + String(blinkStrength));
        // Bắn dữ liệu giả lập lên Web Global (Render) ngay lập tức
        syncTelemetryToCloud(); 
      }
      // ------------------------------------------
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
        // Lấy 8 dải sóng não (24 byte tiếp theo)
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
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  const char* ptr = INDEX_HTML;
  size_t totalLen = strlen_P(INDEX_HTML);
  size_t chunkSize = 2048;
  
  while (totalLen > 0) {
    size_t len = (totalLen > chunkSize) ? chunkSize : totalLen;
    server.sendContent_P(ptr, len);
    ptr += len;
    totalLen -= len;
  }
  server.sendContent(""); 
}

// =========================================================================
// KHỞI ĐỘNG HỆ THỐNG
// =========================================================================
void setup() {
  Serial.begin(115200);
  TGAMSerial.begin(TGAM_BAUDRATE, SERIAL_8N1, TGAM_RX_PIN, TGAM_TX_PIN);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  // Bỏ qua kiểm tra chứng chỉ SSL để tránh lỗi kết nối HTTPS
  secureClient.setInsecure(); 

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.softAP(AP_SSID, AP_PASS);

  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASS_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASS_2);

  Serial.println("\n[SYSTEM] Đang kết nối Router Wi-Fi...");
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[WIFI] Kết nối Router thành công!");
    Serial.print("[IP] http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WIFI] Chưa kết nối được Router, chạy trên SoftAP:");
    Serial.print("[IP] http://");
    Serial.println(WiFi.softAPIP());
  }

  if (MDNS.begin("esp32-bci")) {
    Serial.println("[MDNS] http://esp32-bci.local");
  }

  server.on("/", HTTP_GET, handleRoot);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  webSocket.enableHeartbeat(15000, 10000, 2);


  // KHỞI TẠO TASK FREERTOS CHẠY Ở CORE 0 (Chuyên lo gửi Cloud)
  xTaskCreatePinnedToCore(
    cloudSyncTask,   /* Hàm thực thi Task */
    "CloudTask",     /* Tên Task */
    8192,            /* Kích thước Stack (RAM) */
    NULL,            /* Tham số truyền vào */
    1,               /* Độ ưu tiên */
    NULL,            /* Handle */
    0                /* Chạy ở Core 0 (Core 1 để dành cho Web Local) */
  );

  lastActivityTime = millis();
  lastTelemetrySyncTime = millis();
}

// =========================================================================
// VÒNG LẶP CHÍNH (MAIN LOOP) - CHẠY Ở CORE 1
// =========================================================================
void loop() {
  webSocket.loop();
  server.handleClient();
  processTGAMSerial();

  // Tự động duy trì kết nối Wi-Fi Router
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) { 
      wifiMulti.run();
    }
  }

  // Tự động chuyển sang chế độ Modem Sleep nếu không có thao tác sau AUTO_SLEEP_TIMEOUT
  if (!isSleeping && (millis() - lastActivityTime > AUTO_SLEEP_TIMEOUT)) {
    isSleeping = true;
    WiFi.setSleep(true);
    webSocket.broadcastTXT("SLEEPING");
    syncTelemetryToCloud();
  }

  // Cập nhật thông số cơ bản cho client mỗi 1 giây
  static unsigned long lastBroadcastTime = 0;
  if (millis() - lastBroadcastTime > 1000) {
    lastBroadcastTime = millis();

    if (!isSleeping && webSocket.connectedClients() > 0) {
      String tgamData = "TGAM:" + String(signalQuality) + "," + 
                                  String(attention) + "," + 
                                  String(meditation);
      webSocket.broadcastTXT(tgamData);
    }
  }

  // Định kỳ gửi Telemetry Heartbeat lên máy chủ Cloud Render
  if (millis() - lastTelemetrySyncTime > TELEMETRY_INTERVAL) {
    lastTelemetrySyncTime = millis();
    syncTelemetryToCloud();
  }
  
  delay(10); 
}