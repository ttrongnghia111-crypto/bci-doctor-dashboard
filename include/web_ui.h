#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi" data-theme="dark">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>Kính IR Hỗ Trợ Giao Tiếp</title>
<style>:root{--bg-sky-top:#e0f2fe;--bg-sky-bottom:#f8fafc;--bg-card:rgba(255,255,255,0.90);--text-main:#0f172a;--text-muted:#475569;--primary:#0284c7;--primary-glow:rgba(2,132,199,0.25);--key-bg:rgba(255,255,255,0.95);--key-border:#cbd5e1;--key-shadow:rgba(15,23,42,0.08);--key-sys-bg:#f1f5f9;--grass-top:#4ade80;--grass-bottom:#16a34a;--cloud-opacity:0.75;--scenery-brightness:1;--success:#059669;--danger:#e11d48;--warning:#d97706;--scan-border:#f59e0b}[data-theme="dark"]{--bg-sky-top:#0b132b;--bg-sky-bottom:#1e293b;--bg-card:rgba(30,41,59,0.88);--text-main:#f8fafc;--text-muted:#94a3b8;--primary:#38bdf8;--primary-glow:rgba(56,189,248,0.25);--key-bg:rgba(30,41,59,0.94);--key-border:#334155;--key-shadow:rgba(0,0,0,0.35);--key-sys-bg:#0f172a;--grass-top:#14532d;--grass-bottom:#052e16;--cloud-opacity:0.10;--scenery-brightness:0.65;--success:#34d399;--danger:#fb7185;--warning:#fbbf24;--scan-border:#fde047}*{box-sizing:border-box;margin:0;padding:0}body{font-family:'Segoe UI',Quicksand,Roboto,sans-serif;background:linear-gradient(180deg,var(--bg-sky-top) 0%,var(--bg-sky-bottom) 100%);color:var(--text-main);overflow:hidden;display:flex;flex-direction:column;align-items:center;height:100vh;width:100vw;user-select:none;-webkit-user-select:none;transition:background 0.4s ease,color 0.4s ease}.scenery-layer{position:fixed;bottom:0;left:0;width:100%;height:100vh;pointer-events:none;z-index:0;overflow:hidden;filter:brightness(var(--scenery-brightness))}.grass{position:absolute;bottom:-2vh;left:-5%;width:110%;height:15vh;background:linear-gradient(to bottom,var(--grass-top),var(--grass-bottom));border-top-left-radius:50% 10vh;border-top-right-radius:50% 10vh;opacity:0.85}.cloud{position:absolute;background:#ffffff;border-radius:50px;opacity:var(--cloud-opacity);animation:floatCloud 55s linear infinite}.cloud::before,.cloud::after{content:'';position:absolute;background:#ffffff;border-radius:50%}.c1{top:10%;left:-20%;width:130px;height:42px}.c1::before{width:54px;height:54px;top:-26px;left:18px}.c1::after{width:44px;height:44px;top:-18px;right:18px}.c2{top:24%;left:-30%;width:170px;height:52px;animation-duration:75s;animation-delay:-15s;transform:scale(0.85)}.c2::before{width:72px;height:72px;top:-36px;left:24px}.c2::after{width:62px;height:62px;top:-26px;right:26px}@keyframes floatCloud{0%{transform:translateX(-15vw)}100%{transform:translateX(115vw)}}button,.cell{touch-action:manipulation;cursor:pointer;border:none;outline:none;font-family:inherit}.top-bar{width:100%;max-width:1380px;padding:10px 18px 0 18px;display:flex;justify-content:space-between;align-items:center;gap:8px;z-index:100}.widget-pill{background:var(--bg-card);backdrop-filter:blur(14px);border:1.5px solid var(--key-border);padding:6px 14px;border-radius:24px;box-shadow:0 8px 24px var(--key-shadow);display:flex;align-items:center;gap:6px;flex-wrap:wrap}.clock-group{display:flex;align-items:baseline;gap:8px;padding-right:10px;border-right:1.5px solid var(--key-border)}#clock-time{font-size:18px;font-weight:900;color:var(--primary);font-variant-numeric:tabular-nums}#clock-date{font-size:11px;color:var(--text-muted);font-weight:700;font-variant-numeric:tabular-nums}.pill-btn{background:var(--key-sys-bg);color:var(--text-main);border:1.5px solid var(--key-border);font-weight:800;font-size:12px;padding:6px 11px;border-radius:16px;transition:all 0.2s ease}.pill-btn:active{transform:scale(0.96)}.pill-btn.accent{background:var(--primary);color:#ffffff;border-color:transparent}.pill-btn.scan-mode-pill{background:#4f46e5;color:#ffffff;border-color:transparent}.pill-btn.photo-mode-active{background:#059669;color:#ffffff;border-color:transparent}.pill-btn.lock-btn{background:rgba(16,185,129,0.16);color:var(--success);border-color:var(--success)}.pill-btn.lock-btn.locked{background:var(--danger);color:#ffffff;border-color:transparent}#control-lock-banner{position:fixed;top:64px;left:50%;transform:translateX(-50%);background:var(--danger);color:#ffffff;padding:10px 26px;border-radius:30px;font-size:16px;font-weight:800;z-index:950;display:none;align-items:center;gap:16px;box-shadow:0 12px 30px rgba(225,29,72,0.45)}#control-lock-banner button{background:#ffffff;color:var(--danger);font-weight:900;font-size:13px;padding:6px 14px;border-radius:14px}#clinical-local-banner{position:fixed;top:70px;left:50%;transform:translateX(-50%);width:min(94vw,860px);background:#be123c;color:#ffffff;padding:14px 22px;border-radius:20px;border:3px solid #fde047;z-index:960;display:none;align-items:center;justify-content:space-between;gap:14px;flex-wrap:wrap;box-shadow:0 16px 38px rgba(0,0,0,0.55)}#clinical-local-banner.warn{background:#b45309;border-color:#fde047}#clinical-local-banner .cb-title{font-size:17px;font-weight:900;text-transform:uppercase}#clinical-local-banner .cb-sub{font-size:13px;font-weight:700;opacity:0.95}#clinical-local-banner .cb-actions{display:flex;gap:8px;flex-wrap:wrap}#clinical-local-banner .cb-btn{background:#fde047;color:#0f172a;font-weight:900;font-size:13px;padding:8px 14px;border-radius:12px}#clinical-local-banner .cb-btn.secondary{background:#ffffff;color:#0f172a}#view-menu{display:flex;flex-direction:column;justify-content:space-between;width:100%;flex:1;max-width:1280px;padding:12px 20px 22px 20px;z-index:10}.page-title{font-size:26px;font-weight:900;text-align:center;letter-spacing:0.3px;margin:2px 0 8px 0}.dash-row{display:grid;grid-template-columns:repeat(4,1fr);gap:14px;width:100%}.dash-card{background:var(--bg-card);backdrop-filter:blur(14px);border:1.5px solid var(--key-border);border-radius:22px;padding:16px 14px;text-align:center;box-shadow:0 10px 28px var(--key-shadow);display:flex;flex-direction:column;justify-content:center;gap:6px}.dash-title{font-size:12px;color:var(--text-muted);font-weight:800;text-transform:uppercase;letter-spacing:0.5px}.dash-value{font-size:32px;font-weight:900;color:var(--primary);line-height:1.1;font-variant-numeric:tabular-nums}.dash-value.good{color:var(--success);font-size:26px}.dash-value.bad{color:var(--warning);font-size:22px}.menu-actions{display:flex;flex-direction:column;gap:14px;width:100%;flex:1;margin-top:14px}.dual-track-container{display:grid;grid-template-columns:1fr 1fr;gap:18px;width:100%;flex:1}.track-card-btn{border-radius:28px;color:#ffffff;display:flex;align-items:center;justify-content:center;gap:22px;padding:20px 26px;box-shadow:0 14px 32px rgba(0,0,0,0.18);border:3px solid rgba(255,255,255,0.45);transition:transform 0.2s ease,box-shadow 0.2s ease}.track-visual{width:104px;height:104px;min-width:104px;background:rgba(255,255,255,0.22);border:3px solid #ffffff;border-radius:24px;display:flex;align-items:center;justify-content:center;overflow:hidden;box-shadow:0 8px 18px rgba(0,0,0,0.2)}.track-visual svg{width:82%;height:82%}.track-card-btn .track-title{font-size:29px;font-weight:900;letter-spacing:0.5px;text-transform:uppercase;text-align:left;line-height:1.2}.track-aac{background:linear-gradient(135deg,#0284c7,#0369a1)}.track-kb{background:linear-gradient(135deg,#0d9488,#0f766e)}.btn-reader-menu{background:linear-gradient(135deg,#4f46e5,#4338ca);color:#ffffff;padding:17px;font-size:20px;font-weight:900;letter-spacing:0.5px;text-transform:uppercase;border-radius:22px;border:3px solid rgba(255,255,255,0.35);box-shadow:0 10px 25px rgba(79,70,229,0.28);width:100%}.sub-actions-row{display:grid;grid-template-columns:repeat(4,1fr);gap:14px;width:100%}.btn-sub{background:var(--bg-card);backdrop-filter:blur(14px);border:1.5px solid var(--key-border);color:var(--text-main);padding:14px 12px;font-size:16px;font-weight:800;border-radius:20px;display:flex;align-items:center;justify-content:center;gap:6px;box-shadow:0 6px 18px var(--key-shadow)}.btn-sub span{color:var(--primary);font-weight:900}#view-quick{display:none;flex-direction:column;width:100%;flex:1;max-width:1340px;padding:10px 20px 20px 20px;gap:10px;z-index:10}.aac-header-bar{display:flex;justify-content:space-between;align-items:center;gap:10px;width:100%}.aac-nav-btn{flex:1;background:var(--bg-card);backdrop-filter:blur(14px);color:var(--text-main);padding:11px 14px;border-radius:18px;font-size:15px;font-weight:900;border:1.5px solid var(--key-border);box-shadow:0 6px 16px var(--key-shadow)}.aac-grid-2x2{display:flex;flex-direction:column;gap:10px;flex:1;width:100%}.aac-row-2{display:grid;grid-template-columns:1fr 1fr;gap:14px;flex:1;width:100%}.aac-card{border-radius:24px;color:#ffffff;display:flex;flex-direction:row;align-items:center;justify-content:flex-start;padding:12px 22px;gap:20px;text-align:left;position:relative;border:3.5px solid rgba(255,255,255,0.55);box-shadow:0 10px 24px rgba(0,0,0,0.22);transition:transform 0.16s ease,box-shadow 0.16s ease;overflow:hidden}.aac-visual-box{width:122px;height:122px;min-width:122px;background:rgba(255,255,255,0.24);border:3.5px solid #ffffff;border-radius:22px;display:flex;align-items:center;justify-content:center;overflow:hidden;box-shadow:0 6px 16px rgba(0,0,0,0.22);position:relative}.aac-visual-box img.real-photo-img{width:100%;height:100%;object-fit:cover;display:block}.aac-visual-box .svg-fallback{width:84%;height:84%;display:flex;align-items:center;justify-content:center}.aac-visual-box .svg-fallback svg{width:100%;height:100%;filter:drop-shadow(0 3px 5px rgba(0,0,0,0.25))}.aac-title{font-size:30px;font-weight:900;letter-spacing:0.3px;line-height:1.2;text-transform:uppercase;text-shadow:0 2px 8px rgba(0,0,0,0.32);flex:1}.photo-edit-badge{position:absolute;top:8px;right:12px;background:rgba(15,23,42,0.90);color:#fde047;padding:4px 10px;border-radius:10px;font-size:11px;font-weight:900;border:1.5px solid #fde047;display:none;z-index:5}body.photo-edit-mode .photo-edit-badge{display:block}#view-keyboard{display:none;flex-direction:column;width:100%;flex:1;max-width:1320px;padding:10px 20px 22px 20px;gap:8px;z-index:10}#top-controls{width:100%;display:flex;justify-content:space-between;align-items:center;gap:10px}.top-btn{flex:1;background:var(--bg-card);backdrop-filter:blur(14px);color:var(--text-main);padding:11px 14px;border-radius:18px;font-size:15px;font-weight:900;border:1.5px solid var(--key-border);box-shadow:0 6px 15px var(--key-shadow)}#output-box{width:100%;min-height:68px;padding:10px 24px;background:var(--bg-card);backdrop-filter:blur(14px);border-radius:22px;border:2.5px solid var(--primary);color:var(--text-main);font-size:34px;font-weight:900;display:flex;align-items:center;justify-content:center;box-shadow:0 8px 24px var(--primary-glow);text-align:center;word-break:break-word}.grid{display:flex;flex-direction:column;gap:7px;width:100%;flex:1}.row{display:flex;gap:7px;justify-content:center;flex:1}.cell{flex:1;background:var(--key-bg);border:1.5px solid var(--key-border);border-radius:16px;font-size:26px;font-weight:900;color:var(--text-main);display:flex;justify-content:center;align-items:center;box-shadow:0 4px 12px var(--key-shadow);transition:all 0.15s ease}.cell.suggest{background-color:rgba(16,185,129,0.16);border-color:var(--success);color:var(--success);font-size:23px}.cell.sys{background-color:var(--key-sys-bg);font-size:17px;color:var(--text-main)}.cell.iot{background-color:rgba(217,119,6,0.15);border-color:var(--warning);color:var(--warning);font-size:17px}.cell.danger{background-color:rgba(225,29,72,0.16);border-color:var(--danger);color:var(--danger);font-size:19px}.cell.action{background:var(--primary);border-color:var(--primary);color:#ffffff;font-size:19px}#view-reader{display:none;flex-direction:column;width:100%;flex:1;max-width:1080px;padding:12px 20px 24px 20px;gap:12px;z-index:10;overflow:hidden}.reader-nav{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:12px;background:var(--bg-card);backdrop-filter:blur(14px);padding:14px 22px;border-radius:22px;border:1.5px solid var(--key-border);box-shadow:0 8px 24px var(--key-shadow)}.reader-nav h2{font-size:21px;font-weight:900;color:var(--text-main)}.reader-controls{display:flex;gap:10px;align-items:center}.btn-reader-action{padding:10px 16px;background:var(--key-sys-bg);border:1.5px solid var(--key-border);color:var(--text-main);border-radius:16px;font-size:15px;font-weight:800}.btn-reader-action.active{background:var(--success);border-color:var(--success);color:#ffffff}.reader-viewport{background:var(--bg-card);backdrop-filter:blur(14px);border:1.5px solid var(--key-border);border-radius:26px;padding:28px 36px;line-height:2.1;font-size:27px;font-weight:600;box-shadow:0 12px 30px var(--key-shadow);flex:1;overflow-y:auto}.reader-viewport p{margin-bottom:20px;text-align:justify}.scan-group{border:3.5px solid transparent;border-radius:24px;padding:3px;transition:all 0.15s ease}.scanning-row{border-color:var(--scan-border)!important;background-color:rgba(250,204,21,0.16)!important;box-shadow:0 0 22px rgba(245,158,11,0.35)!important}.scanning-cell{outline:6px solid #fde047!important;box-shadow:0 0 0 10px rgba(255,255,255,0.95),0 16px 36px rgba(0,0,0,0.5)!important;transform:translateY(-3px) scale(1.015)!important;filter:brightness(1.1)!important;z-index:25}.cell.scanning-cell,.btn-sub.scanning-cell,.top-btn.scanning-cell,.aac-nav-btn.scanning-cell,.btn-reader-action.scanning-cell{background:#facc15!important;color:#0f172a!important;border-color:#ffffff!important}.btn-sub.scanning-cell span{color:#0f172a!important}.overlay{position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(15,23,42,0.92);z-index:1000;display:none;flex-direction:column;align-items:center;justify-content:center;backdrop-filter:blur(14px)}.sleep-text{font-size:36px;font-weight:900;color:#f8fafc;margin-bottom:24px;text-align:center}.wake-hint{font-size:20px;color:var(--success);font-weight:800;background:rgba(16,185,129,0.15);padding:16px 34px;border-radius:30px;border:2px solid var(--success)}#settings-panel{background:var(--bg-card);border:2px solid var(--primary);border-radius:28px;padding:30px;width:90%;max-width:520px;box-shadow:0 20px 50px rgba(0,0,0,0.4)}.set-title{font-size:23px;color:var(--text-main);font-weight:900;margin-bottom:20px;text-align:center}.set-group{margin-bottom:16px}.set-label{font-size:15px;color:var(--text-main);font-weight:800;margin-bottom:8px;display:block}.set-select,.set-slider{width:100%;padding:11px;font-size:15px;font-weight:700;background:var(--key-sys-bg);color:var(--text-main);border:1.5px solid var(--key-border);border-radius:14px}.set-val{float:right;color:var(--primary)}.set-btns{display:flex;gap:10px;margin-top:24px}.set-btns button{flex:1;padding:13px 10px;border-radius:16px;font-size:14px;font-weight:900;color:#ffffff}.btn-test{background:var(--key-sys-bg);color:var(--text-main)!important;border:1.5px solid var(--key-border)}.btn-close{background:var(--primary)}#blink-counter{position:fixed;top:68px;right:24px;background:var(--success);color:#ffffff;border:2px solid #ffffff;padding:9px 20px;border-radius:30px;font-size:15px;font-weight:900;z-index:200;display:none;box-shadow:0 8px 20px rgba(16,185,129,0.4)}.toast{position:fixed;top:28px;left:50%;transform:translateX(-50%);background:var(--primary);color:#ffffff;padding:15px 34px;border-radius:32px;font-size:21px;font-weight:900;z-index:9999;display:none;box-shadow:0 14px 36px rgba(0,0,0,0.35);border:3px solid #ffffff}@media (max-width:900px){.aac-visual-box{width:92px;height:92px;min-width:92px}.aac-title{font-size:21px}.track-card-btn .track-title{font-size:21px}}</style>
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
<button class="pill-btn" onclick="toggleRealPhotoMode()" id="top-realmode-pill">Ảnh: SVG Gốc</button>
<button class="pill-btn" onclick="togglePhotoEditMode()" id="top-photo-pill">Tải Ảnh Riêng: Tắt</button>
<button class="pill-btn" onclick="toggleAudioScan()" id="top-audio-pill">Âm Quét: Bật</button>
<button class="pill-btn" onclick="cycleNodMode()" id="top-mode-pill">Độ Nhạy: Kép</button>
<button class="pill-btn" onclick="cycleInputSource()" id="top-input-pill">Kính IR + IMU</button>
<button class="pill-btn" onclick="openSettings()">Giọng Đọc</button>
<button class="pill-btn" onclick="sendCommand('RECONNECT_ROUTER')" id="top-net-pill" title="Bấm để thử kết nối lại Router">WiFi: --</button>
<button class="pill-btn" onclick="showGatewayInfo()" id="top-gw-pill" title="Bấm để kiểm tra kết nối Gateway IoT">GW: Offline</button>
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

<!-- BANNER CẢNH BÁO LÂM SÀNG ĐỒNG BỘ (SPASM / CHỐNG LOÉT 120P / GỢI Ý CHỈ KÍNH IR KHI CFI CAO) -->
<div id="clinical-local-banner">
<div>
<div class="cb-title" id="cb-title">CẢNH BÁO AN TOÀN Y SINH</div>
<div class="cb-sub" id="cb-sub">Hệ thống phát hiện dấu hiệu cần hỗ trợ.</div>
</div>
<div class="cb-actions">
<button class="cb-btn" id="cb-action-ir" onclick="switchToIrFromBanner()" style="display:none">Chuyển Sang Chỉ Kính IR</button>
<button class="cb-btn secondary" id="cb-dismiss-btn" onclick="dismissLocalClinicalBanner()">Đã Kiểm Tra</button>
</div>
</div>

<div id="blink-counter">Đã chọn</div>
<div id="toast-alert" class="toast"></div>

<div id="sleep-overlay" class="overlay">
<div class="sleep-text" id="sleep-msg">Chế Độ Nghỉ Thư Giãn</div>
<div class="wake-hint">Nhắm mắt giữ 1 giây hoặc Gật mạnh để thức dậy</div>
</div>

<div id="fatigue-lock-overlay" class="overlay">
<div class="sleep-text" style="color:var(--warning)">Tạm Nghỉ Thư Giãn Cơ (CFI &ge; 75%)</div>
<div id="fatigue-timer-display" style="font-size: 56px; font-weight: 900; color: #ffffff; margin-bottom: 18px; font-variant-numeric: tabular-nums;">30:00</div>
<div style="display:flex; gap:12px; flex-wrap:wrap; justify-content:center;">
<button type="button" onclick="switchToIrAndUnlock()" style="padding:14px 26px; font-size:16px; font-weight:900; border-radius:20px; background:var(--success); color:#ffffff;">Chuyển Chỉ Kính IR &amp; Mở Khóa</button>
<button type="button" onclick="sendCommand('UNLOCK_FATIGUE')" style="padding:14px 26px; font-size:16px; font-weight:900; border-radius:20px; background:var(--primary); color:#ffffff;">Tiếp Tục Sử Dụng</button>
</div>
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

<!-- VIEW 1: TRANG CHỦ (100% SVG NỘI BỘ - TẢI TỨC THÌ OFFLINE) -->
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
<div class="dash-title">Độ Mỏi Cơ (CFI)</div>
<div class="dash-value" id="cfi-val" style="color: var(--warning)">12%</div>
</div>
<div class="dash-card">
<div class="dash-title">Nguồn Hệ Thống</div>
<div class="dash-value" id="bat-val" style="color: var(--success)">100%</div>
</div>
</div>

<div class="menu-actions">
<div class="dual-track-container scan-group">
<button class="track-card-btn track-aac scan-item" data-voice="Chọn bằng hình ảnh" onclick="showQuick()">
<div class="track-visual">
<svg viewBox="0 0 100 100" fill="none">
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
<svg viewBox="0 0 100 100" fill="none">
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

<!-- VIEW 2: BẢNG HÌNH ẢNH 6 NHÓM NGỮ CẢNH (THỐNG NHẤT TÊN VỚI GLOBAL & HỒ SƠ) -->
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
<button class="cell sys scan-item" data-voice="Đọc báo">ĐỌC BÁO</button>
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
    feel_sleepy: `<svg viewBox="0 0 120 120"><path d="M74 22 C44 22 28 56 48 84 C64 104 94 96 98 76 C72 82 54 56 74 22 Z" fill="#fde04