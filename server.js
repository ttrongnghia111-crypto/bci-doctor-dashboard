const express = require('express');
const cors = require('cors');
const path = require('path');
const mongoose = require('mongoose');

const app = express();
app.use(cors());
app.use(express.json());

// ==========================================
// 1. KẾT NỐI MONGODB ATLAS (CÓ CHỐNG TREO)
// ==========================================
const MONGO_URI = "mongodb+srv://ttrongnghia111_db_user:Nghia123456789@cluster0.2fyogcj.mongodb.net/bci_database?retryWrites=true&w=majority&appName=Cluster0";

// Thêm serverSelectionTimeoutMS để nếu DB lỗi mạng, nó sẽ báo ngay chứ không treo Server
mongoose.connect(MONGO_URI, { serverSelectionTimeoutMS: 5000 })
    .then(() => console.log("✅ Đã kết nối MongoDB Atlas thành công!"))
    .catch(err => console.error("❌ Lỗi kết nối MongoDB (Kiểm tra lại IP/Mạng):", err.message));

// ==========================================
// 2. KHAI BÁO CẤU TRÚC LƯU LỊCH SỬ (SCHEMA)
// ==========================================
const logSchema = new mongoose.Schema({
    timestamp: { type: Date, default: Date.now },
    sentence: String,
    attention: Number,
    meditation: Number
});
const Log = mongoose.model('Log', logSchema);

// ==========================================
// 3. CƠ SỞ DỮ LIỆU TẠM THỜI CHO SÓNG NÃO (RAM)
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
        // Nếu DB chưa kết nối được, bỏ qua việc lưu để không bị treo ESP32
        if (mongoose.connection.readyState !== 1) {
            return res.status(503).send("Database chưa sẵn sàng");
        }

        const { sentence, attention, meditation } = req.body;
        const newLog = new Log({
            sentence: sentence || "Không rõ",
            attention: attention || 0,
            meditation: meditation || 0
        });
        
        await newLog.save();
        res.status(200).send("Log đã được lưu vào MongoDB");
    } catch (error) {
        console.error("Lỗi khi lưu log:", error.message);
        res.status(500).send("Lỗi server");
    }
});

// Gửi dữ liệu cho Web Frontend (Cloud)
app.get('/api/data', async (req, res) => {
    try {
        let communicationLogs = [];
        
        // CHỈ LẤY LỊCH SỬ NẾU MONGODB ĐANG HOẠT ĐỘNG
        if (mongoose.connection.readyState === 1) {
            communicationLogs = await Log.find().sort({ timestamp: -1 }).limit(1000);
        }
        
        // LUÔN LUÔN TRẢ VỀ SÓNG NÃO (TELEMETRY) DÙ DATABASE CÓ LỖI HAY KHÔNG
        res.json({ telemetry: telemetryData, logs: communicationLogs });
    } catch (error) {
        console.error("Lỗi lấy dữ liệu DB:", error.message);
        // Nếu lỗi DB, vẫn trả về sóng não để Web không bị báo "Chưa kết nối"
        res.json({ telemetry: telemetryData, logs: [] });
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