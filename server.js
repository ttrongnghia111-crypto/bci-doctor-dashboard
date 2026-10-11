const express = require('express');
const cors = require('cors');
const path = require('path');
const app = express();

app.use(cors());
app.use(express.json());

// ==========================================
// 1. CƠ SỞ DỮ LIỆU TẠM THỜI (RAM)
// ==========================================
let telemetryData = {
    isSleeping: false,
    signalQuality: 200,
    attention: 0,
    meditation: 0,
    battery: 100, // Thêm Pin
    delta: 0, theta: 0, lowAlpha: 0, highAlpha: 0, lowBeta: 0, highBeta: 0, lowGamma: 0, midGamma: 0, // Thêm Sóng não
    uptime: 0,
    lastUpdate: Date.now()
};

let communicationLogs = []; 

// ==========================================
// 2. API NHẬN DỮ LIỆU TỪ ESP32
// ==========================================
app.post('/api/telemetry', (req, res) => {
    telemetryData = { ...telemetryData, ...req.body, lastUpdate: Date.now() };
    res.status(200).send("Telemetry OK");
});

app.post('/api/logs', (req, res) => {
    const { sentence, attention, meditation } = req.body;
    
    const newLog = {
        timestamp: new Date().toISOString(), // Chuẩn ISO để Frontend đọc
        sentence: sentence || "Không rõ",
        attention: attention || 0,
        meditation: meditation || 0
    };
    
    communicationLogs.unshift(newLog); 
    if (communicationLogs.length > 50) communicationLogs.pop(); 
    
    res.status(200).send("Log OK");
});

app.get('/api/data', (req, res) => {
    res.json({ telemetry: telemetryData, logs: communicationLogs });
});

// ==========================================
// 3. RENDER GIAO DIỆN WEB BÁC SĨ
// ==========================================
app.use(express.static(__dirname)); // Phục vụ file tĩnh
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// ==========================================
// KHỞI CHẠY SERVER
// ==========================================
const PORT = process.env.PORT || 10000;
app.listen(PORT, () => {
    console.log(`Server Cloud BCI dang chay tai port ${PORT}`);
});