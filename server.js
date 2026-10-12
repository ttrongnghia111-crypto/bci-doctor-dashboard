const express = require('express');
const cors = require('cors');
const path = require('path');
const mongoose = require('mongoose'); // Thêm thư viện MongoDB

const app = express();
app.use(cors());
app.use(express.json());

// ==========================================
// 1. KẾT NỐI MONGODB ATLAS
// ==========================================
//
const MONGO_URI = const MONGO_URI = "mongodb+srv://ttrongnghia111_db_user:ijf4PcErVJSQwNCB@cluster0.2fyogcj.mongodb.net/bci_database?retryWrites=true&w=majority&appName=Cluster0";;

mongoose.connect(MONGO_URI)
    .then(() => console.log("✅ Đã kết nối MongoDB Atlas thành công! Lịch sử sẽ được lưu vĩnh viễn."))
    .catch(err => console.error("❌ Lỗi kết nối MongoDB:", err));

// ==========================================
// 2. KHAI BÁO CẤU TRÚC LƯU LỊCH SỬ (SCHEMA)
// ==========================================
const logSchema = new mongoose.Schema({
    timestamp: { type: Date, default: Date.now },
    sentence: String,
    attention: Number,
    meditation: Number
});
const Log = mongoose.model('Log', logSchema); // Tạo bảng 'logs' trong Database

// ==========================================
// 3. CƠ SỞ DỮ LIỆU TẠM THỜI CHO SÓNG NÃO (RAM)
// (Chỉ dùng cho Telemetry realtime để tránh quá tải DB)
// ==========================================
let telemetryData = {
    isSleeping: false,
    signalQuality: 200,
    attention: 0,
    meditation: 0,
    battery: 100,
    delta: 0, theta: 0, lowAlpha: 0, highAlpha: 0, lowBeta: 0, highBeta: 0, lowGamma: 0, midGamma: 0,
    uptime: 0,
    lastUpdate: Date.now()
};

// ==========================================
// 4. CÁC API NHẬN/TRẢ DỮ LIỆU
// ==========================================

// Nhận Heartbeat & sóng não từ ESP32
app.post('/api/telemetry', (req, res) => {
    telemetryData = { ...telemetryData, ...req.body, lastUpdate: Date.now() };
    res.status(200).send("Telemetry OK");
});

// Nhận Lịch sử phát ngôn từ ESP32 và LƯU VÀO MONGODB
app.post('/api/logs', async (req, res) => {
    try {
        const { sentence, attention, meditation } = req.body;
        
        // Tạo log mới và lưu vào Database
        const newLog = new Log({
            sentence: sentence || "Không rõ",
            attention: attention || 0,
            meditation: meditation || 0
        });
        
        await newLog.save(); // Lưu vĩnh viễn
        res.status(200).send("Log đã được lưu vào MongoDB");
    } catch (error) {
        console.error("Lỗi khi lưu log:", error);
        res.status(500).send("Lỗi server");
    }
});

// Gửi dữ liệu cho Web Frontend (Cloud)
app.get('/api/data', async (req, res) => {
    try {
        // Lấy lịch sử từ MongoDB (sắp xếp thời gian mới nhất lên đầu, giới hạn lấy 1000 câu để web không bị giật)
        const communicationLogs = await Log.find().sort({ timestamp: -1 }).limit(1000);
        
        res.json({ telemetry: telemetryData, logs: communicationLogs });
    } catch (error) {
        console.error("Lỗi lấy dữ liệu:", error);
        res.status(500).json({ error: "Lỗi server" });
    }
});

// ==========================================
// 5. RENDER GIAO DIỆN WEB BÁC SĨ
// ==========================================
app.use(express.static(__dirname)); 
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// ==========================================
// KHỞI CHẠY SERVER
// ==========================================
const PORT = process.env.PORT || 10000;
app.listen(PORT, () => {
    console.log(`🚀 Server Cloud BCI đang chạy tại port ${PORT}`);
});