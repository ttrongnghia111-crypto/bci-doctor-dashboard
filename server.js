// =========================================================================
// SERVER CLOUD RENDER + MONGODB ATLAS - SMART GLASSES V4 HYBRID (V8.2)
// Yêu cầu cấu hình biến môi trường trên Render: MONGO_URI
// =========================================================================
const express = require('express');
const cors = require('cors');
const path = require('path');
const mongoose = require('mongoose');

const app = express();
app.use(cors());
app.use(express.json({ limit: '16kb' }));

// ==========================================
// 1. KẾT NỐI MONGODB ATLAS AN TOÀN (ENV VAR)
// ==========================================
const MONGO_URI = process.env.MONGO_URI;

if (!MONGO_URI) {
    console.warn("⚠️ CHƯA CẤU HÌNH BIẾN MÔI TRƯỜNG MONGO_URI! Server chạy chế độ RAM tạm thời (không lưu DB).");
} else {
    mongoose.connect(MONGO_URI, { serverSelectionTimeoutMS: 5000 })
        .then(async () => {
            console.log("✅ Đã kết nối MongoDB Atlas thành công!");
            await warmLogCache();
        })
        .catch(err => console.error("❌ Lỗi kết nối MongoDB (Kiểm tra MONGO_URI / IP Whitelist):", err.message));
}

// ==========================================
// 2. SCHEMA NHẬT KÝ & PHÂN LOẠI TỰ ĐỘNG
// ==========================================
const logSchema = new mongoose.Schema({
    timestamp: { type: Date, default: Date.now },
    sentence: { type: String, required: true, maxlength: 260 },
    attention: { type: Number, default: 0 },
    meditation: { type: Number, default: 95 },
    cfi: { type: Number, default: 12 },
    category: { type: String, default: 'feel' },
    severity: { type: String, enum: ['normal', 'warning', 'emergency'], default: 'normal' }
});
const Log = mongoose.model('Log', logSchema);

function classifySentence(sentence) {
    const s = String(sentence || '').toUpperCase();
    if (s.includes('SOS') || s.includes('CẤP CỨU') || s.includes('CO GIẬT') || s.includes('TÉ NGÃ')) {
        return { category: 'pain', severity: 'emergency' };
    }
    if (s.includes('120 PHÚT') || s.includes('CHỐNG LOÉT')) {
        return { category: 'hygiene', severity: 'warning' };
    }
    if (s.includes('ĐAU') || s.includes('NHỨC') || s.includes('KHÓ THỞ') || s.includes('VẾT MỔ') ||
        s.includes('TÊ MỎI') || s.includes('TAY CHÂN') || s.includes('CHÓNG MẶT') || s.includes('BUỒN NÔN') ||
        s.includes('NGỨA') || s.includes('PAIN')) {
        return { category: 'pain', severity: 'warning' };
    }
    if (s.includes('NƯỚC') || s.includes('UỐNG') || s.includes('ĂN') || s.includes('CƠM') ||
        s.includes('CHÁO') || s.includes('SỮA') || s.includes('THUỐC') || s.includes('TRÁI CÂY') ||
        s.includes('KHÁT') || s.includes('ĐÓI')) {
        return { category: 'food', severity: 'normal' };
    }
    if (s.includes('VỆ SINH') || s.includes('NGỒI') || s.includes('NẰM') || s.includes('NGHIÊNG') ||
        s.includes('LẬT') || s.includes('TRỞ MÌNH') || s.includes('CHĂN') || s.includes('GỐI') ||
        s.includes('GIƯỜNG') || s.includes('LAU NGƯỜI') || s.includes('THAY ÁO') || s.includes('QUẦN ÁO') ||
        s.includes('TƯ THẾ')) {
        return { category: 'hygiene', severity: 'normal' };
    }
    if (s.includes('BÁC SĨ') || s.includes('Y TÁ') || s.includes('ĐIỀU DƯỠNG') || s.includes('NGƯỜI NHÀ') ||
        s.includes('GIA ĐÌNH') || s.includes('HUYẾT ÁP') || s.includes('HÔ HẤP') || s.includes('HÚT ĐỜM') ||
        s.includes('XOA BÓP') || s.includes('GỌI')) {
        return { category: 'care', severity: 'normal' };
    }
    if (s.includes('ĐÈN') || s.includes('QUẠT') || s.includes('RƠ-LE') || s.includes('NÓNG') ||
        s.includes('LẠNH') || s.includes('ĐỌC BÁO') || s.includes('ĐỌC SÁCH') || s.includes('KHÓA') ||
        s.includes('BẬT') || s.includes('TẮT')) {
        return { category: 'iot', severity: 'normal' };
    }
    return { category: 'feel', severity: 'normal' };
}

// ==========================================
// 3. BỘ NHỚ ĐỆM RAM (CACHE LOG + TELEMETRY)
// ==========================================
const MAX_CACHED_LOGS = 200;
let cachedLogs = [];
let logsVersion = 0;

async function warmLogCache() {
    try {
        if (mongoose.connection.readyState !== 1) return;
        const rows = await Log.find().sort({ timestamp: -1 }).limit(MAX_CACHED_LOGS).lean();
        cachedLogs = rows;
        logsVersion++;
        console.log(`📦 Đã nạp ${cachedLogs.length} bản ghi lịch sử vào RAM Cache.`);
    } catch (err) {
        console.error("Lỗi warmLogCache:", err.message);
    }
}

let telemetryData = {
    isSleeping: false,
    isControlLocked: false,
    isFatigueLocked: false,
    fatigueSecLeft: 0,
    signalQuality: 200,
    attention: 0,
    meditation: 95,
    cfi: 12,
    temperature: 34.5,
    pitch: 0,
    roll: 0,
    battery: 100,
    voltage: 5.0,
    current: 110,
    power: 550,
    shuntVoltage: 0,
    powerSource: 'USB_5V',
    delta: 0,
    theta: 0,
    irDelta: 0,
    inputMode: 'HYBRID',
    nodMode: 'DUAL',
    relay1: false,
    relay2: false,
    gatewayOnline: false,
    netMode: 'CONNECTING',
    staticMinutes: 0,
    uptime: 0,
    lastUpdate: 0
};

// ==========================================
// 4. HÀNG ĐỢI LỆNH ĐIỀU KHIỂN CÓ TTL 60 GIÂY
// ==========================================
const ALLOWED_COMMANDS = new Set([
    'LOCK_REST', 'UNLOCK_REST',
    'LOCK_CONTROL', 'UNLOCK_CONTROL',
    'INPUT_HYBRID', 'INPUT_IR', 'INPUT_NOD',
    'R1_ON', 'R1_OFF', 'R2_ON', 'R2_OFF',
    'IDLE', 'NONE'
]);

const COMMAND_TTL_MS = 60 * 1000;
let commandQueue = [];
let commandSeq = 0;

function pruneExpiredCommands() {
    const now = Date.now();
    commandQueue = commandQueue.filter(item => item.expiresAt > now);
}

function popNextCommand() {
    pruneExpiredCommands();
    if (commandQueue.length === 0) return null;
    return commandQueue.shift();
}

function peekNextCommand() {
    pruneExpiredCommands();
    if (commandQueue.length === 0) return null;
    return commandQueue[0];
}

// ==========================================
// 5. CÁC ENDPOINT API ĐỒNG BỘ 5 ĐẦU
// ==========================================

// Health check (chống ngủ đông Render & theo dõi trạng thái DB)
app.get('/health', (req, res) => {
    const ageSec = telemetryData.lastUpdate ? Math.floor((Date.now() - telemetryData.lastUpdate) / 1000) : null;
    res.status(200).json({
        status: 'ok',
        uptimeSec: Math.floor(process.uptime()),
        dbConnected: mongoose.connection.readyState === 1,
        cachedLogs: cachedLogs.length,
        pendingCommands: commandQueue.length,
        lastTelemetryAgeSec: ageSec
    });
});

// Nhận Telemetry từ ESP32-S3 VÀ trả về ngay lệnh đang chờ (hợp nhất giảm ~80% phiên TLS)
app.post('/api/telemetry', (req, res) => {
    const b = req.body || {};
    const num = (v, fallback) => (typeof v === 'number' && Number.isFinite(v)) ? v : fallback;
    const bool = (v, fallback) => (typeof v === 'boolean') ? v : fallback;
    const str = (v, fallback, maxLen = 24) => (typeof v === 'string' && v.length <= maxLen) ? v : fallback;

    telemetryData = {
        ...telemetryData,
        isSleeping: bool(b.isSleeping, telemetryData.isSleeping),
        isControlLocked: bool(b.isControlLocked, telemetryData.isControlLocked),
        isFatigueLocked: bool(b.isFatigueLocked, telemetryData.isFatigueLocked),
        fatigueSecLeft: num(b.fatigueSecLeft, telemetryData.fatigueSecLeft),
        signalQuality: num(b.signalQuality, telemetryData.signalQuality),
        attention: num(b.attention, telemetryData.attention),
        meditation: num(b.meditation, telemetryData.meditation),
        cfi: num(b.cfi, telemetryData.cfi),
        temperature: num(b.temperature, telemetryData.temperature),
        pitch: num(b.pitch, telemetryData.pitch),
        roll: num(b.roll, telemetryData.roll),
        battery: num(b.battery, telemetryData.battery),
        voltage: num(b.voltage, telemetryData.voltage),
        current: num(b.current, telemetryData.current),
        power: num(b.power, telemetryData.power),
        shuntVoltage: num(b.shuntVoltage, telemetryData.shuntVoltage),
        powerSource: str(b.powerSource, telemetryData.powerSource),
        delta: num(b.delta, telemetryData.delta),
        theta: num(b.theta, telemetryData.theta),
        irDelta: num(b.irDelta, telemetryData.irDelta),
        inputMode: str(b.inputMode, telemetryData.inputMode),
        nodMode: str(b.nodMode, telemetryData.nodMode),
        relay1: bool(b.relay1, telemetryData.relay1),
        relay2: bool(b.relay2, telemetryData.relay2),
        gatewayOnline: bool(b.gatewayOnline, telemetryData.gatewayOnline),
        netMode: str(b.netMode, telemetryData.netMode),
        staticMinutes: num(b.staticMinutes, telemetryData.staticMinutes),
        uptime: num(b.uptime, telemetryData.uptime),
        lastUpdate: Date.now()
    };

    const nextCmd = popNextCommand();
    res.status(200).json({
        ok: true,
        cmd: nextCmd ? nextCmd.command : 'NONE',
        cmdId: nextCmd ? nextCmd.id : 0,
        duration: nextCmd ? nextCmd.duration : 0
    });
});

// Nhận bản tin giao tiếp / cảnh báo từ ESP32-S3 -> cập nhật RAM Cache ngay + lưu MongoDB
app.post('/api/logs', async (req, res) => {
    try {
        const rawSentence = typeof req.body?.sentence === 'string' ? req.body.sentence.trim().slice(0, 260) : '';
        if (!rawSentence) {
            return res.status(400).json({ ok: false, error: 'Thiếu nội dung sentence' });
        }

        const attention = Number.isFinite(Number(req.body.attention)) ? Number(req.body.attention) : 0;
        const meditation = Number.isFinite(Number(req.body.meditation)) ? Number(req.body.meditation) : 95;
        const cfi = Number.isFinite(Number(req.body.cfi)) ? Number(req.body.cfi) : telemetryData.cfi;
        const { category, severity } = classifySentence(rawSentence);

        let savedDoc = {
            _id: 'mem-' + Date.now() + '-' + Math.floor(Math.random() * 1000),
            timestamp: new Date(),
            sentence: rawSentence,
            attention,
            meditation,
            cfi,
            category,
            severity
        };

        if (mongoose.connection.readyState === 1) {
            const newLog = new Log({
                sentence: rawSentence,
                attention,
                meditation,
                cfi,
                category,
                severity
            });
            const doc = await newLog.save();
            savedDoc = doc.toObject();
        }

        cachedLogs.unshift(savedDoc);
        if (cachedLogs.length > MAX_CACHED_LOGS) cachedLogs.length = MAX_CACHED_LOGS;
        logsVersion++;

        res.status(200).json({ ok: true, id: savedDoc._id, category, severity });
    } catch (error) {
        console.error("Lỗi khi lưu log:", error.message);
        res.status(500).json({ ok: false, error: 'Lỗi server' });
    }
});

// Trả dữ liệu từ RAM Cache cho Web Global (không query MongoDB mỗi 1,5s!)
app.get('/api/data', (req, res) => {
    res.json({
        telemetry: telemetryData,
        logs: cachedLogs,
        logsVersion,
        dbConnected: mongoose.connection.readyState === 1
    });
});

// Nhận lệnh từ Web Global (hoặc ESP32 xóa trạng thái IDLE)
app.post('/api/command', (req, res) => {
    const cmd = typeof req.body?.command === 'string' ? req.body.command.trim().toUpperCase() : 'NONE';
    if (!ALLOWED_COMMANDS.has(cmd)) {
        return res.status(400).json({ status: 'error', message: 'Lệnh không hợp lệ' });
    }

    if (cmd === 'IDLE' || cmd === 'NONE') {
        commandQueue = [];
        return res.json({ status: 'success', command: 'NONE', queued: 0 });
    }

    const duration = Number.isFinite(Number(req.body?.duration)) ? Math.max(0, Math.min(7200, Number(req.body.duration))) : 0;
    pruneExpiredCommands();

    // Khử trùng lệnh giống hệt đang chờ trong hàng đợi
    commandQueue = commandQueue.filter(item => item.command !== cmd);
    if (commandQueue.length >= 10) commandQueue.shift();

    const entry = {
        id: ++commandSeq,
        command: cmd,
        duration,
        expiresAt: Date.now() + COMMAND_TTL_MS
    };
    commandQueue.push(entry);
    console.log(`-> Đưa lệnh vào hàng đợi (#${entry.id}, TTL 60s): ${cmd}`);

    res.json({ status: 'success', id: entry.id, command: cmd, queued: commandQueue.length });
});

// Giữ GET /api/command để tương thích ngược (trả về lệnh hợp lệ còn TTL)
app.get('/api/command', (req, res) => {
    const nextCmd = peekNextCommand();
    res.send(nextCmd ? nextCmd.command : 'NONE');
});

// ==========================================
// 6. PHỤC VỤ GIAO DIỆN WEB GLOBAL
// ==========================================
app.use(express.static(__dirname));
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

const PORT = process.env.PORT || 10000;
app.listen(PORT, () => {
    console.log(`🚀 Server Cloud Smart Glasses V4 Hybrid đang chạy tại port ${PORT}`);
});