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
    feel_sleepy: `<svg viewBox="0 0 120 120"><path d="M74 22 C44 22 28 56 48 84 C64 104 94 96 98 76 C72 82 54 56 74 22 Z" fill="#fde047" stroke="#fff" stroke-width="4"/></svg>`,
    feel_thanks: `<svg viewBox="0 0 120 120"><path d="M60 98 C60 98 18 70 18 42 C18 24 36 18 50 30 L60 40 L70 30 C84 18 102 24 102 42 C102 70 60 98 60 98 Z" fill="#fde047" stroke="#fff" stroke-width="5"/></svg>`,
    feel_yes: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="44" fill="#fff"/><path d="M34 62 L52 80 L88 42" stroke="#059669" stroke-width="12" fill="none" stroke-linecap="round" stroke-linejoin="round"/></svg>`,
    feel_no: `<svg viewBox="0 0 120 120"><circle cx="60" cy="60" r="44" fill="#fff"/><path d="M38 38 L82 82 M82 38 L38 82" stroke="#e11d48" stroke-width="12" stroke-linecap="round"/></svg>`,
    feel_tv: `<svg viewBox="0 0 120 120"><rect x="18" y="28" width="84" height="58" rx="10" fill="#fff"/><rect x="26" y="36" width="68" height="42" rx="6" fill="#0284c7"/><rect x="42" y="92" width="36" height="8" rx="4" fill="#fde047"/></svg>`
};

// ĐỒNG BỘ 100% TÊN 6 NHÓM AAC VỚI WEB GLOBAL & BỘ HỒ SƠ
const quickData = {
    main: [
        { id: 'cat_pain', title: 'Báo Đau & Cấp Cứu',     voice: 'Nhóm báo đau và cấp cứu',     target: 'pain', bg: 'linear-gradient(135deg, #e11d48, #be123c)' },
        { id: 'cat_food', title: 'Ăn Uống & Thuốc',       voice: 'Nhóm ăn uống và thuốc',       target: 'food', bg: 'linear-gradient(135deg, #0284c7, #0369a1)' },
        { id: 'cat_life', title: 'Vệ Sinh & Tư Thế',      voice: 'Nhóm vệ sinh và tư thế',      target: 'life', bg: 'linear-gradient(135deg, #0d9488, #0f766e)' },
        { id: 'cat_help', title: 'Gọi Người & Y Tế',      voice: 'Nhóm gọi người và y tế',      target: 'help', bg: 'linear-gradient(135deg, #d97706, #b45309)' },
        { id: 'cat_room', title: 'Đèn Quạt & Phòng',      voice: 'Nhóm đèn quạt và phòng',      target: 'room', bg: 'linear-gradient(135deg, #4f46e5, #3730a3)' },
        { id: 'cat_feel', title: 'Cảm Xúc & Trò Chuyện',  voice: 'Nhóm cảm xúc và trò chuyện',  target: 'feel', bg: 'linear-gradient(135deg, #059669, #047857)' }
    ],
    pain: [
        { id: 'pain_head',  title: 'Đau Đầu / Chóng Mặt', voice: 'Đau đầu chóng mặt', speakOut: 'Tôi bị đau đầu và chóng mặt', bg: 'linear-gradient(135deg, #e11d48, #be123c)' },
        { id: 'pain_chest', title: 'Đau Ngực / Khó Thở',  voice: 'Đau ngực khó thở',  speakOut: 'Tôi bị đau ngực và khó thở, cần kiểm tra ngay', bg: 'linear-gradient(135deg, #dc2626, #991b1b)' },
        { id: 'pain_belly', title: 'Đau Bụng / Buồn Nôn', voice: 'Đau bụng buồn nôn', speakOut: 'Tôi bị đau bụng và buồn nôn', bg: 'linear-gradient(135deg, #ea580c, #c2410c)' },
        { id: 'pain_back',  title: 'Đau Lưng / Mỏi Cổ',   voice: 'Đau lưng mỏi cổ',   speakOut: 'Tôi bị đau lưng và mỏi cổ', bg: 'linear-gradient(135deg, #d97706, #b45309)' },
        { id: 'pain_limbs', title: 'Tê Mỏi Tay Chân',     voice: 'Tê mỏi tay chân',   speakOut: 'Tay chân tôi đang bị tê mỏi', bg: 'linear-gradient(135deg, #c026d3, #86198f)' },
        { id: 'pain_wound', title: 'Đau Vết Mổ / Ngứa',   voice: 'Đau vết mổ hoặc ngứa', speakOut: 'Tôi bị đau hoặc ngứa ở vết mổ', bg: 'linear-gradient(135deg, #be185d, #831843)' }
    ],
    food: [
        { id: 'food_water',    title: 'Uống Nước',   voice: 'Uống nước',   speakOut: 'Tôi khát, cho tôi uống nước', bg: 'linear-gradient(135deg, #0284c7, #0369a1)' },
        { id: 'food_milk',     title: 'Uống Sữa',    voice: 'Uống sữa',    speakOut: 'Cho tôi uống sữa', bg: 'linear-gradient(135deg, #0891b2, #155e75)' },
        { id: 'food_porridge', title: 'Ăn Cháo',     voice: 'Ăn cháo',     speakOut: 'Tôi đói, cho tôi ăn cháo', bg: 'linear-gradient(135deg, #d97706, #b45309)' },
        { id: 'food_rice',     title: 'Ăn Cơm',      voice: 'Ăn cơm',      speakOut: 'Tôi muốn ăn cơm', bg: 'linear-gradient(135deg, #ca8a04, #854d0e)' },
        { id: 'food_fruit',    title: 'Ăn Trái Cây', voice: 'Ăn trái cây', speakOut: 'Cho tôi ăn một ít trái cây', bg: 'linear-gradient(135deg, #16a34a, #166534)' },
        { id: 'food_meds',     title: 'Uống Thuốc',  voice: 'Uống thuốc',  speakOut: 'Đến giờ uống thuốc rồi, cho tôi uống thuốc', bg: 'linear-gradient(135deg, #e11d48, #9f1239)' }
    ],
    life: [
        { id: 'life_toilet',  title: 'Đi Vệ Sinh',      voice: 'Đi vệ sinh',      speakOut: 'Tôi cần đi vệ sinh', bg: 'linear-gradient(135deg, #0284c7, #075985)' },
        { id: 'life_liedown', title: 'Hạ Giường Nằm',   voice: 'Hạ giường nằm',   speakOut: 'Hỗ trợ hạ giường cho tôi nằm xuống', bg: 'linear-gradient(135deg, #4f46e5, #3730a3)' },
        { id: 'life_situp',   title: 'Nâng Đầu Ngồi',   voice: 'Nâng đầu ngồi',   speakOut: 'Hỗ trợ nâng đầu giường cho tôi ngồi dậy', bg: 'linear-gradient(135deg, #0d9488, #115e59)' },
        { id: 'life_turn',    title: 'Hỗ Trợ Trở Mình', voice: 'Hỗ trợ trở mình', speakOut: 'Giúp tôi nghiêng người trở mình cho đỡ mỏi', bg: 'linear-gradient(135deg, #059669, #065f46)' },
        { id: 'life_clothes', title: 'Thay Áo / Lau Người', voice: 'Thay áo lau người', speakOut: 'Giúp tôi lau người và thay quần áo', bg: 'linear-gradient(135deg, #7c3aed, #5b21b6)' },
        { id: 'life_blanket', title: 'Đắp Chăn / Gối',  voice: 'Đắp chăn chỉnh gối', speakOut: 'Giúp tôi đắp chăn và chỉnh lại gối', bg: 'linear-gradient(135deg, #2563eb, #1e40af)' }
    ],
    help: [
        { id: 'help_doctor',  title: 'Gọi Bác Sĩ (SOS)', voice: 'Gọi bác sĩ cấp cứu', speakOut: 'Cấp cứu! Cần bác sĩ giúp đỡ gấp!', isSOS: true, bg: 'linear-gradient(135deg, #dc2626, #991b1b)' },
        { id: 'help_nurse',   title: 'Gọi Y Tá',         voice: 'Gọi y tá',           speakOut: 'Nhờ gọi y tá điều dưỡng giúp tôi', bg: 'linear-gradient(135deg, #e11d48, #be123c)' },
        { id: 'help_family',  title: 'Gọi Người Nhà',    voice: 'Gọi người nhà',      speakOut: 'Nhờ gọi người nhà vào với tôi', bg: 'linear-gradient(135deg, #0284c7, #0369a1)' },
        { id: 'help_bp',      title: 'Đo Huyết Áp',      voice: 'Đo huyết áp',        speakOut: 'Nhờ đo huyết áp và kiểm tra sức khỏe cho tôi', bg: 'linear-gradient(135deg, #d97706, #b45309)' },
        { id: 'help_breath',  title: 'Hỗ Trợ Hô Hấp',    voice: 'Hỗ trợ hô hấp hút đờm', speakOut: 'Tôi khó thở, cần hỗ trợ hô hấp hoặc hút đờm', bg: 'linear-gradient(135deg, #0891b2, #155e75)' },
        { id: 'help_massage', title: 'Xoa Bóp Giảm Mỏi', voice: 'Xoa bóp giảm mỏi',   speakOut: 'Nhờ xoa bóp tay chân giúp tôi đỡ mỏi', bg: 'linear-gradient(135deg, #059669, #047857)' }
    ],
    room: [
        { id: 'room_light', title: 'Bật / Tắt Đèn',  voice: 'Bật tắt đèn phòng', speakOut: 'Đã chuyển trạng thái đèn phòng', sysCmd: 'R1', bg: 'linear-gradient(135deg, #d97706, #b45309)' },
        { id: 'room_fan',   title: 'Bật / Tắt Quạt', voice: 'Bật tắt quạt máy',  speakOut: 'Đã chuyển trạng thái quạt máy',  sysCmd: 'R2', bg: 'linear-gradient(135deg, #0284c7, #0369a1)' },
        { id: 'room_cold',  title: 'Phòng Hơi Lạnh', voice: 'Phòng hơi lạnh',    speakOut: 'Tôi thấy hơi lạnh, nhờ tăng nhiệt độ hoặc tắt gió', bg: 'linear-gradient(135deg, #2563eb, #1e3a8a)' },
        { id: 'room_hot',   title: 'Phòng Hơi Nóng', voice: 'Phòng hơi nóng',    speakOut: 'Tôi thấy hơi nóng, nhờ bật mát hơn', bg: 'linear-gradient(135deg, #ea580c, #9a3412)' },
        { id: 'room_book',  title: 'Đọc Sách Báo',   voice: 'Mở đọc sách báo',   speakOut: 'Mở trình đọc sách báo', sysCmd: 'OPEN_READER', bg: 'linear-gradient(135deg, #4f46e5, #3730a3)' },
        { id: 'room_lock',  title: 'Khóa Tạm Nghỉ',  voice: 'Khóa tạm nghỉ',     speakOut: 'Khóa tạm nghỉ điều khiển', sysCmd: 'LOCK_NOW', bg: 'linear-gradient(135deg, #e11d48, #9f1239)' }
    ],
    feel: [
        { id: 'feel_good',   title: 'Tôi Thấy Ổn',      voice: 'Tôi thấy ổn và khỏe', speakOut: 'Tôi cảm thấy ổn và dễ chịu rồi', bg: 'linear-gradient(135deg, #059669, #047857)' },
        { id: 'feel_sleepy', title: 'Tôi Buồn Ngủ',     voice: 'Tôi buồn ngủ',        speakOut: 'Tôi buồn ngủ, muốn nghỉ ngơi một lát', bg: 'linear-gradient(135deg, #4f46e5, #312e81)' },
        { id: 'feel_thanks', title: 'Cảm Ơn Nhiều',     voice: 'Cảm ơn nhiều',        speakOut: 'Cảm ơn bạn rất nhiều', bg: 'linear-gradient(135deg, #db2777, #9d174d)' },
        { id: 'feel_yes',    title: 'Đúng / Đồng Ý',    voice: 'Đúng đồng ý',         speakOut: 'Vâng, đúng rồi, tôi đồng ý', bg: 'linear-gradient(135deg, #16a34a, #166534)' },
        { id: 'feel_no',     title: 'Không / Chưa Cần', voice: 'Không chưa cần',      speakOut: 'Không phải, tôi chưa cần', bg: 'linear-gradient(135deg, #e11d48, #9f1239)' },
        { id: 'feel_tv',     title: 'Mở Tivi / Nhạc',   voice: 'Mở ti vi hoặc nhạc',  speakOut: 'Nhờ mở ti vi hoặc bật nhạc nhẹ giúp tôi', bg: 'linear-gradient(135deg, #0284c7, #075985)' }
    ]
};

const storyLibrary = [
    {
        title: "Hạt Giống Tâm Hồn: Tia Nắng Buổi Sáng",
        content: `<p>Mỗi sớm mai thức dậy là một món quà tuyệt vời của cuộc sống. Ánh nắng ban mai len lỏi qua ô cửa sổ, mang theo sự ấm áp và niềm hy vọng mới mẻ cho ngày hôm nay.</p><p>Dù phía trước có những khó khăn hay thử thách, chỉ cần trái tim chúng ta luôn hướng về phía ánh sáng, sự bình an và sức mạnh nội tại sẽ luôn đồng hành cùng bạn.</p><p>Hãy hít thở thật sâu, cảm nhận từng nhịp đập dịu êm của cơ thể và mỉm cười đón nhận ngày mới đầy bình yên bạn nhé.</p>`
    },
    {
        title: "Góc Bình Yên: Thanh Âm Cuộc Sống",
        content: `<p>Sự tĩnh lặng của tâm hồn giúp cơ thể phục hồi năng lượng nhanh hơn bất kỳ phương thuốc nào. Khi đôi mắt được thả lỏng và hơi thở trở nên đều đặn, mọi căng thẳng sẽ dần tan biến.</p><p>Bên ngoài khung cửa, bầu trời vẫn xanh trong và những đám mây trắng vẫn nhẹ nhàng trôi, gửi đến bạn lời chúc bình an và khỏe mạnh mỗi ngày.</p>`
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
let useRealPhotoMode = localStorage.getItem('aac_real_mode') === '1';
let pendingPhotoSlotKey = null;
let currentQuickSubView = 'main';
let scanStrategy = localStorage.getItem('aac_scan_strategy') || 'SMART';
let toastTimer = null;
let gatewayOnlineState = false;
let gatewayIpStr = "";
let cfiBannerDismissedUntil = 0;

function isDirectItemScanActive() {
    if (scanStrategy === 'ITEM') return true;
    if (scanStrategy === 'ROW_COL') return false;
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
                (scanStrategy === 'ITEM')  ? "Kiểu quét Từng ô: Chớp mắt 1 lần chọn ngay" : "Kiểu quét Hàng và Cột";
    showToastAlert(msg);
}

function updateRealModeButtonUI() {
    const btn = document.getElementById('top-realmode-pill');
    if (!btn) return;
    if (useRealPhotoMode) {
        btn.innerText = "Ảnh: Riêng";
        btn.classList.add('photo-mode-active');
    } else {
        btn.innerText = "Ảnh: SVG Gốc";
        btn.classList.remove('photo-mode-active');
    }
}
updateRealModeButtonUI();

function toggleRealPhotoMode() {
    useRealPhotoMode = !useRealPhotoMode;
    localStorage.setItem('aac_real_mode', useRealPhotoMode ? '1' : '0');
    updateRealModeButtonUI();
    if (currentView === 'QUICK') renderQuickGrid(currentQuickSubView);
    showToastAlert(useRealPhotoMode ? "Đang ưu tiên hiển thị Ảnh Riêng đã tải" : "Đang hiển thị Hình Minh Họa SVG Gốc");
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
    Object.keys(localStorage).forEach(k => { if (k.startsWith('aac_photo_')) localStorage.removeItem(k); });
    useRealPhotoMode = false;
    localStorage.setItem('aac_real_mode', '0');
    updateRealModeButtonUI();
    if (currentView === 'QUICK') renderQuickGrid(currentQuickSubView);
    showToastAlert("Đã khôi phục hình minh họa SVG gốc");
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
                useRealPhotoMode = true;
                localStorage.setItem('aac_real_mode', '1');
                updateRealModeButtonUI();
                if (currentView === 'QUICK') renderQuickGrid(currentQuickSubView);
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
    if (savedPhoto && useRealPhotoMode) return `<img class="real-photo-img" src="${savedPhoto}" alt="${item.title}">`;
    return `<div class="svg-fallback">${svgCode}</div>`;
}

// ================= BANNER CẢNH BÁO LÂM SÀNG LOCAL =================
function showLocalClinicalBanner(level, title, sub, showIrBtn = false) {
    const banner = document.getElementById('clinical-local-banner');
    if (!banner) return;
    banner.classList.toggle('warn', level === 'WARN');
    document.getElementById('cb-title').innerText = title;
    document.getElementById('cb-sub').innerText = sub;
    document.getElementById('cb-action-ir').style.display = showIrBtn ? 'inline-block' : 'none';
    banner.style.display = 'flex';
}

function dismissLocalClinicalBanner() {
    const banner = document.getElementById('clinical-local-banner');
    if (banner) banner.style.display = 'none';
    cfiBannerDismissedUntil = Date.now() + 60000;
}

function switchToIrFromBanner() {
    sendCommand('INPUT_IR');
    dismissLocalClinicalBanner();
    showToastAlert("Đã chuyển sang Chỉ Kính IR để giảm mỏi cổ!");
}

function switchToIrAndUnlock() {
    sendCommand('INPUT_IR');
    sendCommand('UNLOCK_FATIGUE');
    showToastAlert("Đã chuyển sang Chỉ Kính IR và mở khóa!");
}

function showGatewayInfo() {
    sendCommand('PING_GATEWAY');
    if (gatewayOnlineState && gatewayIpStr) {
        showToastAlert(`Gateway IoT Online (${gatewayIpStr})`);
    } else {
        showToastAlert("Gateway IoT đang Offline — Đang quét lại trên LAN/AP...");
    }
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
let wsReconnecting = false;
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
    ws.onclose = () => {
        if (!wsReconnecting) {
            wsReconnecting = true;
            setTimeout(() => { wsReconnecting = false; connectWebSocket(); }, 2500);
        }
    };
    ws.onerror = () => { try { ws.close(); } catch(e) {} };

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
                showLocalClinicalBanner('DANGER',
                    '🚨 CẢNH BÁO NGUY HIỂM: PHÁT HIỆN CO GIẬT / TÉ NGÃ (≥ 2,5g)',
                    'Cảm biến LSM6DS3 ghi nhận rung chấn bất thường. Đề nghị người thân kiểm tra ngay!',
                    false);
                speakActionFeedback("Cảnh báo khẩn cấp! Phát hiện dấu hiệu co giật hoặc té ngã!");
            } else if (payload.startsWith("BEDSORE_120M")) {
                playClickBeep('HEAVY');
                showLocalClinicalBanner('WARN',
                    '⏰ NHẮC CHĂM SÓC: CHỐNG LOÉT TÌ ĐÈ (120 PHÚT)',
                    'Bệnh nhân nằm nguyên 1 tư thế quá 120 phút. Cần hỗ trợ lật trở mình.',
                    false);
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
            if (elMini) { elMini.innerText = `${strength}%`; elMini.style.color = colorHex; }
            triggerInstantNodClick(nodType);
        }
        else if (data.startsWith("MODE:")) { updateNodModeUI(data.substring(5).trim(), false); }
        else if (data.startsWith("INPUT_MODE:")) { updateInputSourceUI(data.substring(11).trim(), false); }
        else if (data.startsWith("TGAM:")) {
            let parts = data.substring(5).split(',');
            if (parts.length >= 1) updateSensorDisplay(parseInt(parts[0]));
            if (parts.length >= 4) {
                let cfi = parseInt(parts[3]) || 0;
                const cfiEl = document.getElementById('cfi-val');
                const cfiMini = document.getElementById('cfi-mini');
                if (cfiEl) {
                    cfiEl.innerText = `${cfi}%`;
                    cfiEl.style.color = (cfi >= 65) ? "var(--danger)" : "var(--warning)";
                }
                if (cfiMini) cfiMini.innerText = `${cfi}%`;

                // Cảnh báo sớm mỏi cơ cổ (>= 65%) -> gợi ý chuyển sang Chỉ Kính IR (Bảng 8 Phụ lục IIIa)
                if (cfi >= 65 && currentInputSource !== 'IR_ONLY' && !isFatigueLocked && Date.now() > cfiBannerDismissedUntil) {
                    showLocalClinicalBanner('WARN',
                        `⚠️ CỔ ĐANG MỎI (CFI = ${cfi}%) — GỢI Ý CHUYỂN CHỈ KÍNH IR`,
                        'Chuyển sang chế độ Chỉ dùng Mắt (TCRT5000) để giảm tải cho cơ cổ.',
                        true);
                }
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
        else if (data.startsWith("NET:")) {
            let parts = data.substring(4).split(',');
            const netBtn = document.getElementById('top-net-pill');
            if (netBtn) {
                if (parts[0] === "ROUTER") {
                    netBtn.innerText = "WiFi: Router";
                    netBtn.style.borderColor = "var(--success)";
                    netBtn.style.color = "var(--success)";
                } else if (parts[0] === "LOCAL_AP") {
                    netBtn.innerText = "WiFi: Local AP";
                    netBtn.style.borderColor = "var(--warning)";
                    netBtn.style.color = "var(--warning)";
                } else {
                    netBtn.innerText = "WiFi: Đang dò...";
                }
            }
        }
        else if (data.startsWith("GW:")) {
            let parts = data.substring(3).split(',');
            const gwBtn = document.getElementById('top-gw-pill');
            gatewayOnlineState = (parts[0] === "ONLINE");
            gatewayIpStr = parts[1] || "";
            if (gwBtn) {
                if (gatewayOnlineState) {
                    gwBtn.innerText = "GW: Online";
                    gwBtn.style.borderColor = "var(--success)";
                    gwBtn.style.color = "var(--success)";
                } else {
                    gwBtn.innerText = "GW: Offline";
                    gwBtn.style.borderColor = "var(--danger)";
                    gwBtn.style.color = "var(--danger)";
                }
            }
        }
        else if (data === "IR_DEBUG:OFF") {
            const dbgBanner = document.getElementById('ir-debug-banner');
            if (dbgBanner) dbgBanner.style.display = 'none';
        }
        else if (data.startsWith("IR_DEBUG:")) {
            let parts = data.substring(9).split(',');
            if (parts.length >= 4) {
                const dbgBanner = document.getElementById('ir-debug-banner');
                if (dbgBanner) {
                    dbgBanner.style.display = 'block';
                    document.getElementById('ir-amb').innerText = parts[0];
                    document.getElementById('ir-on').innerText = parts[1];
                    document.getElementById('ir-delta').innerText = parts[2];
                    document.getElementById('ir-thresh').innerText = parts[3];
                }
            }
        }
    };
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
    if (!alreadyLocked) speakActionFeedback("Cảnh báo mỏi cơ cổ. Gợi ý chuyển sang chế độ chỉ dùng mắt hoặc nghỉ 30 phút.");
}

function exitFatigueLockMode() {
    isFatigueLocked = false;
    document.getElementById('fatigue-lock-overlay').style.display = 'none';
    speakActionFeedback("Hệ thống đã mở lại.", () => { if (!isControlLocked) startScanning(); });
}

function updateClock() {
    const now = new Date();
    document.getElementById('clock-time').innerText = now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', hour12: false });
    document.getElementById('clock-date').innerText = `${now.getDate().toString().padStart(2,'0')}/${(now.getMonth()+1).toString().padStart(2,'0')}/${now.getFullYear()}`;
}
setInterval(updateClock, 1000); updateClock();

function updateSensorDisplay(sig) {
    let elSig = document.getElementById('sig-val'), elSigMini = document.getElementById('sig-mini');
    if (isControlLocked) {
        elSig.innerText = "Tạm Khóa"; elSig.className = "dash-value bad";
        if (elSigMini) { elSigMini.innerText = "Tạm Khóa"; elSigMini.style.color = "var(--warning)"; }
        return;
    }
    if (sig === 0) {
        elSig.innerText = "Sẵn Sàng"; elSig.className = "dash-value good";
        if (elSigMini) { elSigMini.innerText = "Sẵn Sàng"; elSigMini.style.color = "var(--success)"; }
    } else {
        elSig.innerText = "Chờ Cảm Biến"; elSig.className = "dash-value bad";
        if (elSigMini) { elSigMini.innerText = "Chờ Cảm Biến"; elSigMini.style.color = "var(--warning)"; }
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

        html += `<div class="aac-row-2 scan-group">
<button class="${cls1}" data-slot="${item1.id}" data-target="${item1.target || ''}" data-voice="${item1.voice}" data-speakout="${item1.speakOut || item1.voice}" data-phrase="${item1.title}" data-syscmd="${item1.sysCmd || ''}" data-sos="${item1.isSOS ? '1' : '0'}" style="background: ${item1.bg};"><span class="photo-edit-badge">Chạm để đổi ảnh thật</span><div class="aac-visual-box">${buildVisualBoxHTML(item1)}</div><div class="aac-title">${item1.title}</div></button>
<button class="${cls2}" data-slot="${item2.id}" data-target="${item2.target || ''}" data-voice="${item2.voice}" data-speakout="${item2.speakOut || item2.voice}" data-phrase="${item2.title}" data-syscmd="${item2.sysCmd || ''}" data-sos="${item2.isSOS ? '1' : '0'}" style="background: ${item2.bg};"><span class="photo-edit-badge">Chạm để đổi ảnh thật</span><div class="aac-visual-box">${buildVisualBoxHTML(item2)}</div><div class="aac-title">${item2.title}</div></button>
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
        msg.rate = Math.min(1.35, Math.max(0.95, ttsRate * 1.05));
        msg.volume = ttsVolume;
        selectVietnameseVoice(msg);

        let finished = false;
        const finish = () => {
            if (finished) return;
            finished = true;
            if (activeSpeechSafetyTimer) { clearTimeout(activeSpeechSafetyTimer); activeSpeechSafetyTimer = null; }
            if (myToken === activeSpeechToken && !isActionSpeechPlaying) onPromptFinished(true);
        };

        msg.onend = finish;
        msg.onerror = finish;
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

const silentStepMs      = [1900, 1350, 950];
const holdAfterSpeechMs = [1350, 950, 650];
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

function stopScannerTimer() { if (scanTimer) { clearTimeout(scanTimer); scanTimer = null; } }

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
        const gridGroups = Array.from(document.querySelectorAll('#quick-grid .scan-group'));
        const navGroup = document.getElementById('quick-nav-group');
        scanGroups = navGroup ? [...gridGroups, navGroup] : gridGroups;
    } else {
        scanGroups = Array.from(activeView.querySelectorAll('.scan-group'));
    }

    flatScanItems = [];
    scanGroups.forEach(grp => {
        const items = Array.from(grp.querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null && el.innerText.trim() !== '');
        flatScanItems.push(...items);
    });

    currentGroupIdx = 0; currentItemIdx = 0; currentFlatIdx = 0; scanState = 'GROUP'; loopCount = 0;
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
        if (cards.length === 2) return `${cards[0].getAttribute('data-voice') || ""}, hoặc ${cards[1].getAttribute('data-voice') || ""}`;
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
                if (loopCount >= 1) {
                    scanState = 'GROUP'; currentItemIdx = 0; loopCount = 0;
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
        scanTimer = setTimeout(() => { advanceScanIndices(); runCurrentScanStep(); }, waitMs);
    });
}

function startScanning() {
    stopScannerTimer();
    if (isControlLocked || isSleeping || isSettingsOpen || isFatigueLocked) return;
    runCurrentScanStep();
}

function executeNormalSelection() {
    if (isControlLocked || isSleeping || isSettingsOpen || isFatigueLocked) return;

    if (isDirectItemScanActive()) {
        if (!flatScanItems || flatScanItems.length === 0) return;
        const targetBtn = flatScanItems[currentFlatIdx];
        if (targetBtn) { stopScannerTimer(); targetBtn.click(); }
        return;
    }

    if (!scanGroups || scanGroups.length === 0) return;
    if (scanState === 'GROUP') {
        let items = Array.from(scanGroups[currentGroupIdx].querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
        if (items && items.length > 0) {
            if (items.length === 1) { stopScannerTimer(); items[0].click(); }
            else {
                scanState = 'ITEM'; currentItemIdx = 0; loopCount = 0;
                lastRowEnterTimestamp = Date.now();
                runCurrentScanStep();
            }
        }
    } else if (scanState === 'ITEM') {
        if (Date.now() - lastRowEnterTimestamp < 420) return;
        let items = Array.from(scanGroups[currentGroupIdx].querySelectorAll('.scan-item')).filter(el => el.offsetParent !== null);
        if (items && items[currentItemIdx]) {
            stopScannerTimer();
            let btn = items[currentItemIdx];
            scanState = 'GROUP'; currentGroupIdx = 0; loopCount = 0;
            btn.click();
        }
    }
}

function sendQuickAlert(msg, onSpeechDone = null) {
    if (msg === "SOS") {
        sendCommand("SOS"); showToastAlert("SOS");
        speakActionFeedback("CẤP CỨU! CẦN BÁC SĨ GIÚP ĐỠ GẤP!", onSpeechDone);
    } else {
        sendCommand("SENTENCE:" + msg); showToastAlert(msg);
        speakActionFeedback(msg, onSpeechDone);
    }
}

function showToastAlert(msg) {
    const toast = document.getElementById('toast-alert');
    if (toastTimer) { clearTimeout(toastTimer); toastTimer = null; }
    toast.innerText = msg === "SOS" ? "CẤP CỨU KHẨN CẤP" : msg;
    toast.style.background = msg === "SOS" ? "var(--danger)" : "var(--primary)";
    toast.style.display = 'block';
    toastTimer = setTimeout(() => { toast.style.display = 'none'; toastTimer = null; }, 3500);
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
    scanState = 'GROUP'; currentGroupIdx = 0; currentItemIdx = 0; loopCount = 0;
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
        applySuggestion(text); speakActionFeedback(text, resumeScanningAfterAction); return;
    }
    else if (element.classList.contains('quick-cat')) {
        renderQuickGrid(element.getAttribute('data-target')); return;
    }
    else if (element.classList.contains('quick-phrase')) {
        let phrase = element.getAttribute('data-phrase') || text;
        let speakOutText = element.getAttribute('data-speakout') || element.getAttribute('data-voice') || phrase;
        let isSOS = element.getAttribute('data-sos') === '1';
        let sysCmd = element.getAttribute('data-syscmd') || '';

        if (sysCmd === 'R1') {
            toggleRelay(1); showToastAlert("Đã bật/tắt Đèn phòng");
            speakActionFeedback(speakOutText, () => { renderQuickGrid('main'); });
        } else if (sysCmd === 'R2') {
            toggleRelay(2); showToastAlert("Đã bật/tắt Quạt máy");
            speakActionFeedback(speakOutText, () => { renderQuickGrid('main'); });
        } else if (sysCmd === 'OPEN_READER') {
            showReader();
        } else if (sysCmd === 'LOCK_NOW') {
            sendCommand('TOGGLE_CONTROL_LOCK');
        } else if (isSOS) {
            sendQuickAlert("SOS", () => { renderQuickGrid('main'); });
        } else {
            sendCommand("SENTENCE:" + phrase); showToastAlert(phrase);
            speakActionFeedback(speakOutText, () => { renderQuickGrid('main'); });
        }
        return;
    }
    else if (element.classList.contains('char')) {
        outBox.textContent += text; outBox.textContent = processTelex(outBox.textContent);
        updatePredictiveText(); speakActionFeedback(text, resumeScanningAfterAction); return;
    }
    else if (text === "CÁCH") {
        if (!outBox.textContent.endsWith(' ') && outBox.textContent !== '') { outBox.textContent += " "; updatePredictiveText(); }
        speakActionFeedback("Dấu cách", resumeScanningAfterAction); return;
    }
    else if (text === "XÓA") {
        outBox.textContent = outBox.textContent.slice(0, -1); updatePredictiveText();
        speakActionFeedback("Xóa", resumeScanningAfterAction); return;
    }
    else if (text === "XÓA HẾT") {
        outBox.textContent = ""; updatePredictiveText();
        speakActionFeedback("Xóa hết", resumeScanningAfterAction); return;
    }
    else if (text === "ĐỌC CÂU") {
        let phrase = outBox.textContent.trim();
        if (phrase !== "") { sendCommand("SENTENCE:" + phrase); showToastAlert(phrase); speakActionFeedback(phrase, resumeScanningAfterAction); }
        else speakActionFeedback("Chưa có nội dung", resumeScanningAfterAction);
        return;
    }
    else if (text === "ĐỌC BÁO") { showReader(); return; }
    else if (text.startsWith("TỐC ĐỘ")) { toggleSpeed(); return; }
    else if (text.includes("ĐÈN")) { toggleRelay(1); speakActionFeedback("Đèn phòng", resumeScanningAfterAction); return; }
    else if (text.includes("QUẠT")) { toggleRelay(2); speakActionFeedback("Quạt máy", resumeScanningAfterAction); return; }
    else if (text === "CẤP CỨU") { sendQuickAlert("SOS", resumeScanningAfterAction); return; }

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
    isSleeping = true; stopScannerTimer();
    document.getElementById('sleep-overlay').style.display = 'flex';
    if (isAutoScrolling) toggleAutoScroll();
    speakActionFeedback("Chế độ nghỉ thư giãn.");
}
function exitSleepMode() {
    isSleeping = false;
    document.getElementById('sleep-overlay').style.display = 'none';
    speakActionFeedback("Đã thức dậy", () => { if (!isControlLocked) startScanning(); });
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
    speakActionFeedback("Hệ thống đọc rõ từng mục và chờ bạn chọn!", () => { ttsRate = curR; ttsVolume = curV; });
}

document.getElementById('rate-slider').oninput = function() { document.getElementById('rate-val').innerText = parseFloat(this.value).toFixed(2); };
document.getElementById('vol-slider').oninput = function() { document.getElementById('vol-val').innerText = Math.round(this.value * 100) + '%'; };

window.addEventListener('keydown', (e) => {
    if (e.code === 'Space') {
        e.preventDefault(); e.stopPropagation();
        if (document.activeElement) document.activeElement.blur();
        sendCommand('SIMULATE_BLINK');
    }
    else if (e.code === 'KeyI') { e.preventDefault(); toggleHybridTrack(); }
    else if (e.code === 'KeyL') { e.preventDefault(); sendCommand('TOGGLE_CONTROL_LOCK'); }
    else if (e.code === 'KeyM') { e.preventDefault(); cycleNodMode(); }
    else if (e.code === 'KeyF') { e.preventDefault(); sendCommand('SIMULATE_FATIGUE'); }
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
```

---

### GIAI ĐOẠN 5 — Web Global Giám Sát & Điều Khiển Từ Xa (`index.html` trên Render)

**Các sửa lỗi & nâng cấp đã tích hợp:**
- Chuyển `CLOUD_COMMAND_URL` sang `${API_BASE}/api/command` (không hardcode URL Render).
- Sửa cơ chế phát hiện log mới & cảnh báo khẩn cấp theo **`logs[0]._id` + `logsVersion`** (không bao giờ bị đứng hình ở mốc 200 hoặc 1.000 bản ghi).
- Khớp phân loại **100% cả 36 câu giao tiếp thật** từ `web_ui.h` (bổ sung `TÊ MỎI`, `TAY CHÂN`, `CHÓNG MẶT`, `BUỒN NÔN`, `NGỨA`, `TRỞ MÌNH`, `LAU NGƯỜI`, `THAY ÁO`, `GIƯỜNG`, `GỐI`, `HÔ HẤP`, `TIVI`, `NHẠC`...) và đồng bộ song ngữ VI/EN cho toàn bộ `<option>` và Modal hồ sơ.
- Hiển thị kiến trúc **2-Node thật (Kính S3 + Gateway C3)**: huy hiệu `Mạng Kính (Router/Local AP)`, `Gateway IoT (Online/Offline)`, đồng hồ `Cập nhật: Xs trước`, tiến trình chống loét thật `staticMinutes / 120 phút`, tự nhận diện `NGUỒN USB 5V` vs `PIN LI-PO`, đồ thị 2 kênh chỉ vẽ khi có `lastUpdate` mới với `irDelta` thật.
- Bổ sung **Thanh Điều Khiển Từ Xa Cho Người Thân / Điều Dưỡng** ngay dưới Hàng 2 (Chuyển nhanh `Kính IR + IMU` / `Chỉ Mắt IR` / `Chỉ Đầu IMU`, Bật/Tắt từ xa `Đèn R1` & `Quạt R2` qua Cloud → Kính S3 → UDP <5ms → Gateway C3, và `Khóa Tạm Nghỉ`) kèm **Web Notification API** báo đẩy ra màn hình điện thoại khi có `SOS` / `Co giật`.

```html
<!DOCTYPE html>
<html lang="vi" data-theme="dark">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kính Thông Minh V4 Hybrid — Giám Sát & Điều Khiển Từ Xa</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        :root {
            --bg-base: #0b132b;
            --bg-grad-bottom: #131f3a;
            --bg-panel: rgba(20, 30, 52, 0.92);
            --bg-card: #16223b;
            --bg-hover: #1d2d4e;
            --bg-chart: #070d1d;
            --bg-overlay: rgba(8, 14, 28, 0.92);
            --border: #243554;
            --border-light: #364d75;
            --text-primary: #f8fafc;
            --text-secondary: #94a3b8;
            --text-dim: #64748b;
            --cyan: #38bdf8;
            --emerald: #34d399;
            --amber: #fbbf24;
            --rose: #fb7185;
            --danger-solid: #e11d48;
            --indigo: #818cf8;
            --bar-empty: #1e293b;
            --shadow-card: 0 8px 24px rgba(0, 0, 0, 0.28);
            --alert-bg: rgba(159, 18, 57, 0.22);
            --alert-border: #fb7185;
            --alert-text: #ffffff;
        }

        [data-theme="light"] {
            --bg-base: #e0f2fe;
            --bg-grad-bottom: #f8fafc;
            --bg-panel: rgba(255, 255, 255, 0.94);
            --bg-card: #f8fafc;
            --bg-hover: #f1f5f9;
            --bg-chart: #f8fafc;
            --bg-overlay: rgba(248, 250, 252, 0.94);
            --border: #cbd5e1;
            --border-light: #94a3b8;
            --text-primary: #0f172a;
            --text-secondary: #334155;
            --text-dim: #64748b;
            --cyan: #0284c7;
            --emerald: #059669;
            --amber: #d97706;
            --rose: #e11d48;
            --danger-solid: #dc2626;
            --indigo: #4f46e5;
            --bar-empty: #e2e8f0;
            --shadow-card: 0 6px 20px rgba(15, 23, 42, 0.08);
            --alert-bg: #fef2f2;
            --alert-border: #f87171;
            --alert-text: #991b1b;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, Helvetica, Arial, sans-serif;
            transition: background-color 0.2s ease, border-color 0.2s ease, color 0.15s ease;
        }

        body {
            background: linear-gradient(180deg, var(--bg-base) 0%, var(--bg-grad-bottom) 100%);
            background-attachment: fixed;
            color: var(--text-primary);
            padding: 16px 22px;
            font-size: 14px;
            line-height: 1.5;
            min-height: 100vh;
            overflow-x: hidden;
        }

        ::-webkit-scrollbar { width: 6px; height: 6px; }
        ::-webkit-scrollbar-track { background: transparent; }
        ::-webkit-scrollbar-thumb { background: var(--border-light); border-radius: 4px; }

        /* ================= TOP NAVIGATION ================= */
        .top-nav {
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 12px;
            margin-bottom: 16px;
            padding: 12px 18px;
            background: var(--bg-panel);
            backdrop-filter: blur(12px);
            border: 1.5px solid var(--border);
            border-radius: 18px;
            box-shadow: var(--shadow-card);
        }

        .dept-brand { display: flex; align-items: center; gap: 14px; }

        .clock-box {
            padding-right: 14px;
            border-right: 1.5px solid var(--border);
            display: flex;
            flex-direction: column;
            align-items: flex-start;
        }

        #clock-time {
            font-size: 20px;
            font-weight: 900;
            color: var(--cyan);
            line-height: 1.1;
            font-variant-numeric: tabular-nums;
        }

        #clock-date {
            font-size: 11px;
            font-weight: 700;
            color: var(--text-dim);
            font-variant-numeric: tabular-nums;
        }

        .brand-text h1 {
            font-size: 16px;
            font-weight: 900;
            letter-spacing: 0.4px;
            text-transform: uppercase;
            color: var(--text-primary);
            margin-bottom: 2px;
        }

        .brand-text p {
            font-size: 11px;
            font-family: ui-monospace, SFMono-Regular, Menlo, monospace;
            color: var(--text-dim);
            letter-spacing: 0.3px;
        }

        .nav-actions {
            display: flex;
            gap: 7px;
            align-items: center;
            flex-wrap: wrap;
        }

        .btn {
            padding: 7px 11px;
            border-radius: 12px;
            border: 1.5px solid var(--border);
            font-size: 12px;
            font-weight: 800;
            cursor: pointer;
            display: inline-flex;
            align-items: center;
            gap: 6px;
            background: var(--bg-card);
            color: var(--text-secondary);
        }

        .btn:hover { background: var(--bg-hover); color: var(--text-primary); border-color: var(--border-light); }
        .btn-theme { border-color: var(--border-light); color: var(--amber); background: rgba(251, 191, 36, 0.1); }
        .btn-lang { border-color: var(--border-light); color: var(--cyan); background: rgba(56, 189, 248, 0.1); }
        .btn-notif { border-color: var(--indigo); color: var(--indigo); background: rgba(129, 140, 248, 0.12); }
        .btn-notif.granted { border-color: var(--emerald); color: var(--emerald); background: rgba(52, 211, 153, 0.12); }
        .btn-demo { border-color: #b45309; color: var(--amber); background: rgba(251, 191, 36, 0.1); }
        .btn-demo.active { background: var(--amber); border-color: var(--amber); color: #0f172a; }
        .btn-trigger-alert { border-color: var(--rose); color: var(--rose); background: rgba(225, 29, 72, 0.12); }
        .btn-trigger-alert:hover { background: var(--danger-solid); border-color: var(--danger-solid); color: #fff; }

        .alert-test-wrap { display: inline-flex; align-items: center; gap: 6px; }

        .select-mini {
            background: var(--bg-card);
            border: 1.5px solid var(--border);
            color: var(--text-primary);
            padding: 6px 9px;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 700;
            outline: none;
        }

        .status-pill {
            display: flex;
            align-items: center;
            gap: 6px;
            font-family: ui-monospace, SFMono-Regular, Menlo, monospace;
            font-size: 11px;
            font-weight: 700;
            padding: 6px 10px;
            background: var(--bg-card);
            border-radius: 12px;
            border: 1.5px solid var(--border);
        }

        .dot { width: 8px; height: 8px; border-radius: 50%; flex-shrink: 0; }
        .dot-green { background: var(--emerald); box-shadow: 0 0 8px var(--emerald); }
        .dot-yellow { background: var(--amber); box-shadow: 0 0 8px var(--amber); }
        .dot-red { background: var(--rose); box-shadow: 0 0 8px var(--rose); }

        /* ================= BÁO ĐỘNG LÂM SÀNG 4-TRONG-1 ================= */
        .clinical-alert-banner {
            display: none;
            background: var(--alert-bg);
            border: 1.5px solid var(--alert-border);
            border-left: 7px solid var(--danger-solid);
            border-radius: 16px;
            padding: 14px 20px;
            margin-bottom: 16px;
            align-items: center;
            justify-content: space-between;
            gap: 14px;
            flex-wrap: wrap;
            box-shadow: 0 6px 24px rgba(225, 29, 72, 0.25);
        }

        .clinical-alert-banner.warn-mode {
            border-color: var(--amber);
            border-left-color: var(--amber);
            background: rgba(245, 158, 11, 0.14);
            box-shadow: 0 6px 24px rgba(245, 158, 11, 0.2);
        }

        .alert-left { display: flex; flex-direction: column; gap: 3px; flex: 1; min-width: 260px; }
        .alert-tag { font-size: 11.5px; font-weight: 900; color: var(--rose); text-transform: uppercase; font-family: ui-monospace, monospace; letter-spacing: 0.5px; }
        .clinical-alert-banner.warn-mode .alert-tag { color: var(--amber); }
        .alert-msg { font-size: 15px; font-weight: 800; color: var(--alert-text); }
        .alert-sub { font-size: 12px; color: var(--text-secondary); }
        .alert-actions { display: flex; gap: 10px; align-items: center; flex-wrap: wrap; }

        .btn-rest-timer {
            background: var(--danger-solid);
            color: #fff;
            border: none;
            font-weight: 800;
            padding: 8px 16px;
            border-radius: 12px;
            font-size: 12px;
            cursor: pointer;
        }

        .btn-ir-quick {
            background: var(--emerald);
            color: #0f172a;
            border: none;
            font-weight: 900;
            padding: 8px 14px;
            border-radius: 12px;
            font-size: 12px;
            cursor: pointer;
        }

        .btn-dismiss-alert {
            background: var(--bg-panel);
            border: 1.5px solid var(--border-light);
            color: var(--text-primary);
            padding: 8px 14px;
            border-radius: 12px;
            font-weight: 800;
            font-size: 12px;
            cursor: pointer;
        }

        .main-wrapper { display: flex; flex-direction: column; gap: 16px; max-width: 1480px; margin: 0 auto; }

        /* ================= HÀNG 1: HỒ SƠ & 3 THẺ ĐỒNG BỘ ================= */
        .row-vitals { display: grid; grid-template-columns: 1.5fr repeat(3, 1fr); gap: 14px; }

        .panel {
            background: var(--bg-panel);
            backdrop-filter: blur(12px);
            border-radius: 18px;
            padding: 16px 18px;
            border: 1.5px solid var(--border);
            position: relative;
            box-shadow: var(--shadow-card);
        }

        .patient-card { display: flex; align-items: center; justify-content: space-between; gap: 12px; }
        .patient-profile { display: flex; align-items: center; gap: 14px; }

        .avatar-initials {
            width: 52px; height: 52px;
            background: rgba(56, 189, 248, 0.14);
            border: 2px solid var(--cyan);
            border-radius: 14px;
            display: flex; align-items: center; justify-content: center;
            font-size: 18px; font-weight: 900; color: var(--cyan); flex-shrink: 0;
        }

        .patient-info h2 { font-size: 16px; font-weight: 800; display: flex; align-items: center; gap: 8px; flex-wrap: wrap; margin-bottom: 4px; }

        .badge-id {
            background: var(--bg-card); padding: 3px 9px; border-radius: 8px;
            font-size: 11px; font-weight: 800; color: var(--cyan); border: 1.5px solid var(--border); cursor: pointer;
        }
        .badge-id:hover { border-color: var(--cyan); }

        .state-chip {
            padding: 3px 9px; border-radius: 8px; font-size: 10.5px; font-weight: 900;
            background: rgba(52, 211, 153, 0.16); color: var(--emerald); border: 1px solid var(--emerald);
        }

        .patient-details { color: var(--text-secondary); font-size: 12.5px; line-height: 1.45; }
        .metric-card { display: flex; flex-direction: column; justify-content: center; }

        .metric-header {
            display: flex; justify-content: space-between; align-items: center;
            font-size: 11.5px; font-weight: 800; color: var(--text-secondary);
            text-transform: uppercase; letter-spacing: 0.4px; margin-bottom: 6px;
        }

        .metric-val-row { display: flex; align-items: baseline; gap: 6px; margin: 2px 0 8px 0; }
        .metric-value { font-size: 32px; font-weight: 900; line-height: 1; font-variant-numeric: tabular-nums; }
        .metric-unit { font-size: 12px; color: var(--text-dim); font-weight: 700; }
        .metric-sub-hint { font-size: 11px; color: var(--text-dim); margin-top: 6px; }
        .progress-bar-bg { width: 100%; height: 6px; background: var(--bar-empty); border-radius: 4px; overflow: hidden; }
        .progress-bar-fill { height: 100%; border-radius: 4px; transition: width 0.35s ease; }

        /* ================= HÀNG 2: HỆ CHUYÊN GIA Y SINH V4 HYBRID ================= */
        .biomarkers-container {
            background: var(--bg-panel);
            backdrop-filter: blur(12px);
            border: 1.5px solid var(--border);
            border-radius: 18px;
            padding: 16px 18px;
            box-shadow: var(--shadow-card);
        }

        .biomarkers-header {
            display: flex; justify-content: space-between; align-items: center;
            flex-wrap: wrap; gap: 8px; margin-bottom: 12px; padding-bottom: 8px;
            border-bottom: 1.5px solid var(--border);
        }

        .biomarkers-title {
            font-size: 12.5px; font-weight: 900; color: var(--text-primary);
            letter-spacing: 0.5px; text-transform: uppercase; display: flex; align-items: center; gap: 8px;
        }

        .biomarkers-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 12px; }

        .bio-box {
            background: var(--bg-card); border: 1.5px solid var(--border);
            border-radius: 14px; padding: 13px 15px; display: flex; flex-direction: column; justify-content: space-between;
        }
        .bio-box:hover { border-color: var(--border-light); }
        .bio-top { display: flex; justify-content: space-between; align-items: center; gap: 6px; font-size: 11.5px; margin-bottom: 6px; }
        .bio-status-badge { font-size: 10px; font-weight: 800; padding: 2px 7px; border-radius: 6px; background: rgba(148, 163, 184, 0.14); white-space: nowrap; }
        .bio-main-val { font-size: 22px; font-weight: 900; color: var(--text-primary); margin: 2px 0 8px 0; font-variant-numeric: tabular-nums; display: flex; align-items: baseline; flex-wrap: wrap; gap: 6px; }
        .metric-pct { font-size: 12px; font-weight: 700; color: var(--text-dim); }
        .bio-desc { font-size: 11px; color: var(--text-dim); margin-top: 7px; }

        /* ================= THANH ĐIỀU KHIỂN NHANH TỪ XA (CLOUD -> KÍNH S3 -> GATEWAY C3) ================= */
        .remote-control-bar {
            background: var(--bg-panel);
            backdrop-filter: blur(12px);
            border: 1.5px solid var(--border);
            border-radius: 18px;
            padding: 14px 18px;
            box-shadow: var(--shadow-card);
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 14px;
        }

        .remote-group {
            display: flex;
            align-items: center;
            gap: 8px;
            flex-wrap: wrap;
        }

        .remote-label {
            font-size: 11.5px;
            font-weight: 900;
            text-transform: uppercase;
            color: var(--text-secondary);
            margin-right: 4px;
        }

        .rc-btn {
            padding: 8px 13px;
            border-radius: 12px;
            border: 1.5px solid var(--border);
            background: var(--bg-card);
            color: var(--text-secondary);
            font-size: 12px;
            font-weight: 800;
            cursor: pointer;
        }

        .rc-btn:hover { border-color: var(--cyan); color: var(--text-primary); }
        .rc-btn.active { background: var(--cyan); color: #0f172a; border-color: transparent; }
        .rc-btn.relay-on { background: rgba(52, 211, 153, 0.22); color: var(--emerald); border-color: var(--emerald); }
        .rc-btn.lock-on { background: var(--danger-solid); color: #fff; border-color: transparent; }
        .rc-btn.suggest-ir { border-color: var(--amber); color: var(--amber); box-shadow: 0 0 12px rgba(251, 191, 36, 0.35); }

        #rc-feedback {
            font-size: 11.5px;
            font-family: ui-monospace, monospace;
            font-weight: 700;
            color: var(--cyan);
            margin-left: auto;
        }

        /* ================= HÀNG 3: ĐỒ THỊ & RADAR ================= */
        .row-charts { display: grid; grid-template-columns: 2.8fr 1.2fr; gap: 14px; }

        .chart-panel {
            background: var(--bg-panel); backdrop-filter: blur(12px);
            border-radius: 18px; padding: 16px 18px; border: 1.5px solid var(--border);
            display: flex; flex-direction: column; box-shadow: var(--shadow-card);
        }

        .section-title {
            font-size: 12.5px; font-weight: 900; text-transform: uppercase; color: var(--text-secondary);
            letter-spacing: 0.5px; margin-bottom: 12px; display: flex; justify-content: space-between;
            align-items: center; flex-wrap: wrap; gap: 8px;
        }

        .chart-container-wave {
            width: 100%; height: 295px; position: relative; background: var(--bg-chart);
            border-radius: 14px; border: 1.5px solid var(--border); overflow: hidden; padding: 10px;
        }

        #no-signal-overlay {
            position: absolute; inset: 0; background: var(--bg-overlay);
            display: flex; flex-direction: column; justify-content: center; align-items: center;
            text-align: center; padding: 20px; z-index: 10; gap: 8px;
        }

        .chart-container-radar {
            height: 195px; width: 100%; background: var(--bg-chart); border-radius: 14px;
            border: 1.5px solid var(--border); padding: 8px; margin-bottom: 12px;
            display: flex; align-items: center; justify-content: center;
        }

        .mini-metric-list { display: grid; grid-template-columns: 1fr; gap: 6px; }

        .mini-metric-row {
            display: flex; justify-content: space-between; align-items: center;
            background: var(--bg-card); padding: 7px 11px; border-radius: 10px;
            font-size: 12px; border: 1px solid var(--border);
        }

        .metric-lbl { color: var(--text-secondary); display: flex; align-items: center; gap: 8px; font-weight: 700; }
        .m-dot { width: 7px; height: 7px; border-radius: 2px; display: inline-block; }
        .m-val { font-weight: 800; color: var(--text-primary); font-variant-numeric: tabular-nums; font-family: ui-monospace, monospace; }

        /* ================= HÀNG 4: THỐNG KÊ 6 NHÓM AAC & NHẬT KÝ ================= */
        .row-bottom { display: grid; grid-template-columns: 1.05fr 2.35fr; gap: 14px; }
        .stats-col { display: flex; flex-direction: column; gap: 14px; }

        .stat-box {
            background: var(--bg-panel); backdrop-filter: blur(12px);
            border-radius: 18px; padding: 16px 18px; border: 1.5px solid var(--border); box-shadow: var(--shadow-card);
        }

        .dist-item { margin-bottom: 9px; }
        .dist-item:last-child { margin-bottom: 0; }
        .dist-info { display: flex; justify-content: space-between; font-size: 12px; font-weight: 700; margin-bottom: 4px; color: var(--text-secondary); }
        .dist-count { color: var(--text-primary); font-weight: 800; font-variant-numeric: tabular-nums; }

        .chart-wrapper {
            height: 92px; background: var(--bg-chart); border-radius: 12px; border: 1.5px solid var(--border);
            display: flex; align-items: flex-end; justify-content: space-around; padding: 10px 8px 0 8px; margin-top: 8px;
        }

        .bar-col { width: 12%; background: var(--bar-empty); border-radius: 4px 4px 0 0; transition: height 0.35s ease; min-height: 4px; }
        .bar-col.active { background: var(--cyan); }
        .x-labels { display: flex; justify-content: space-around; font-size: 11px; font-weight: 700; color: var(--text-dim); margin-top: 6px; }

        .history-card {
            background: var(--bg-panel); backdrop-filter: blur(12px); border-radius: 18px;
            border: 1.5px solid var(--border); display: flex; flex-direction: column;
            overflow: hidden; min-height: 460px; box-shadow: var(--shadow-card);
        }

        .history-header {
            padding: 14px 18px; border-bottom: 1.5px solid var(--border);
            display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 10px;
        }

        .history-title {
            font-size: 12.5px; font-weight: 900; text-transform: uppercase; color: var(--text-secondary);
            letter-spacing: 0.5px; display: flex; align-items: center; gap: 8px;
        }

        .filters-group { display: flex; gap: 8px; flex-wrap: wrap; align-items: center; }

        .select-filter {
            background: var(--bg-card); border: 1.5px solid var(--border); color: var(--text-primary);
            padding: 6px 10px; border-radius: 10px; font-size: 12px; font-weight: 700; outline: none;
        }

        .export-group { display: flex; gap: 6px; }

        .btn-export {
            padding: 6px 11px; background: var(--bg-card); border: 1.5px solid var(--border);
            color: var(--text-secondary); font-size: 11.5px; border-radius: 10px; cursor: pointer; font-weight: 800;
        }
        .btn-export:hover { border-color: var(--cyan); color: var(--cyan); }

        .table-container { flex: 1; max-height: 500px; overflow-y: auto; overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; font-size: 13px; min-width: 640px; }
        thead { position: sticky; top: 0; background: var(--bg-card); z-index: 2; border-bottom: 1.5px solid var(--border); }
        th { text-align: left; padding: 11px 14px; color: var(--text-dim); font-size: 11px; text-transform: uppercase; font-weight: 800; letter-spacing: 0.4px; }
        td { padding: 11px 14px; border-bottom: 1px solid var(--border); vertical-align: middle; }
        tr:hover td { background-color: var(--bg-hover); }
        tr.row-emergency td { background-color: rgba(225, 29, 72, 0.14); }
        tr.row-warning td { background-color: rgba(245, 158, 11, 0.12); }

        .time-hm { font-weight: 800; color: var(--text-primary); font-size: 13px; font-variant-numeric: tabular-nums; }
        .time-date { color: var(--text-dim); font-size: 11px; font-variant-numeric: tabular-nums; }
        .col-sentence { font-weight: 700; color: var(--text-primary); font-size: 14px; }

        .tag {
            padding: 3px 9px; border-radius: 8px; font-size: 11px; font-weight: 800;
            display: inline-block; border: 1px solid var(--border); background: var(--bg-card); color: var(--text-secondary);
        }
        .tag-sos { background: rgba(225, 29, 72, 0.18); color: var(--rose); border-color: var(--rose); }
        .tag-warn { background: rgba(245, 158, 11, 0.18); color: var(--amber); border-color: var(--amber); }
        .tag-cyan { background: rgba(56, 189, 248, 0.15); color: var(--cyan); border-color: var(--cyan); }
        .tag-green { background: rgba(52, 211, 153, 0.15); color: var(--emerald); border-color: var(--emerald); }

        .btn-speak {
            background: var(--bg-card); border: 1.5px solid var(--border); color: var(--text-secondary);
            padding: 5px 11px; border-radius: 10px; cursor: pointer; font-size: 11.5px; font-weight: 800;
        }
        .btn-speak:hover { border-color: var(--cyan); color: var(--cyan); }

        /* ================= MODAL HỒ SƠ BỆNH NHÂN ================= */
        .modal-overlay {
            position: fixed; inset: 0; background: rgba(4, 7, 13, 0.72); backdrop-filter: blur(4px);
            display: none; align-items: center; justify-content: center; z-index: 1000; padding: 16px;
        }

        .modal-card {
            background: var(--bg-panel); border: 1.5px solid var(--border-light);
            border-radius: 18px; width: 100%; max-width: 480px; padding: 22px;
            box-shadow: 0 16px 40px rgba(0, 0, 0, 0.45);
        }

        .modal-header {
            display: flex; justify-content: space-between; align-items: center;
            margin-bottom: 14px; border-bottom: 1.5px solid var(--border); padding-bottom: 10px;
        }

        .modal-title { font-size: 14px; font-weight: 900; text-transform: uppercase; color: var(--text-primary); }
        .user-list { max-height: 220px; overflow-y: auto; margin: 12px 0; display: flex; flex-direction: column; gap: 8px; }

        .user-row {
            padding: 11px 14px; background: var(--bg-card); border: 1.5px solid var(--border);
            border-radius: 12px; cursor: pointer; display: flex; justify-content: space-between; align-items: center;
        }
        .user-row:hover { border-color: var(--cyan); }

        .form-input {
            width: 100%; background: var(--bg-card); border: 1.5px solid var(--border);
            color: var(--text-primary); padding: 9px 12px; border-radius: 10px; font-size: 13px;
            margin-top: 4px; margin-bottom: 12px; outline: none;
        }
        .form-input:focus { border-color: var(--cyan); }

        .btn-primary {
            background: var(--cyan); border: none; color: #fff; font-weight: 800;
            padding: 8px 16px; border-radius: 10px; cursor: pointer; font-size: 12.5px;
        }

        .footer-note { text-align: center; color: var(--text-dim); font-size: 11.5px; font-weight: 700; padding: 8px 0 18px 0; }

        @media (max-width: 1180px) {
            .row-vitals { grid-template-columns: repeat(2, 1fr); }
            .biomarkers-grid { grid-template-columns: repeat(2, 1fr); }
            .row-charts { grid-template-columns: 1fr; }
            .row-bottom { grid-template-columns: 1fr; }
        }

        @media (max-width: 680px) {
            body { padding: 10px 12px; }
            .row-vitals { grid-template-columns: 1fr; }
            .biomarkers-grid { grid-template-columns: 1fr; }
            .clock-box { border-right: none; padding-right: 0; }
            .brand-text h1 { font-size: 14px; }
        }
    </style>
</head>

<body>

    <!-- THANH ĐIỀU HƯỚNG TRÊN CÙNG (HIỂN THỊ KIẾN TRÚC 2-NODE KÍNH S3 + GATEWAY C3) -->
    <header class="top-nav">
        <div class="dept-brand">
            <div class="clock-box">
                <span id="clock-time">--:--</span>
                <span id="clock-date">--/--/----</span>
            </div>
            <div class="brand-text">
                <h1 id="txt-brand-title">Kính Thông Minh V4 Hybrid — Giám Sát Từ Xa</h1>
                <p id="txt-brand-sub">TCRT5000 (IR 200Hz • 60µs) + LSM6DS3 (IMU 6-DOF) • ESP32-S3 + GATEWAY C3 • CLOUD RENDER</p>
            </div>
        </div>

        <div class="nav-actions">
            <button class="btn btn-theme" id="btn-theme" onclick="toggleTheme()">☀️ Nền: Sáng</button>
            <button class="btn btn-lang" id="btn-lang" onclick="toggleLanguage()">Ngôn ngữ: VI</button>
            <button class="btn btn-notif" id="btn-notif" onclick="requestNotifPermission()">🔔 Thông Báo</button>
            <div class="status-pill">
                <span class="dot dot-red" id="conn-dot"></span>
                <span id="conn-text">KÍNH OFFLINE</span>
                <span id="sync-age-text" style="color:var(--text-dim); margin-left:4px;">(--)</span>
            </div>
            <div class="status-pill">
                <span class="dot dot-yellow" id="net-dot"></span>
                <span id="net-mode-text">MẠNG: --</span>
            </div>
            <div class="status-pill">
                <span class="dot dot-red" id="gw-dot"></span>
                <span id="gw-status-text">GW: OFFLINE</span>
            </div>
            <div class="status-pill">
                <span class="dot dot-green" id="cloud-dot"></span>
                <span id="cloud-text">CLOUD SẴN SÀNG</span>
            </div>
            <button class="btn btn-demo" id="btn-demo" onclick="toggleDemo()">Mô Phỏng Dữ Liệu</button>
            <div class="alert-test-wrap">
                <select id="test-alert-select" class="select-mini" aria-label="Chọn kịch bản thử cảnh báo">
                    <option value="CFI" id="opt-test-cfi">Mỏi cổ (CFI ≥ 75%)</option>
                    <option value="SOS" id="opt-test-sos">Gọi cấp cứu (SOS)</option>
                    <option value="SPASM" id="opt-test-spasm">Co giật / Té ngã (≥ 2,5g)</option>
                    <option value="BEDSORE" id="opt-test-bedsore">Chống loét (120 phút)</option>
                </select>
                <button class="btn btn-trigger-alert" id="btn-test-alert" onclick="triggerTestScenario()">Thử Cảnh Báo</button>
            </div>
        </div>
    </header>

    <!-- BANNER BÁO ĐỘNG Y SINH 4-TRONG-1 -->
    <div id="clinical-alert-banner" class="clinical-alert-banner">
        <div class="alert-left">
            <div class="alert-tag" id="txt-alarm-tag">CẢNH BÁO AN TOÀN Y SINH 24/7</div>
            <div class="alert-msg" id="clinical-banner-msg">Hệ thống ghi nhận dấu hiệu bất thường cần hỗ trợ.</div>
            <div class="alert-sub" id="txt-alarm-nurse">Tín hiệu đã đồng bộ từ Kính V4 Hybrid qua Cloud Render đến điện thoại người thân & điều dưỡng.</div>
        </div>
        <div class="alert-actions">
            <button class="btn-ir-quick" id="btn-banner-ir" onclick="sendRemoteCommand('INPUT_IR')" style="display:none">CHUYỂN CHỈ MẮT (IR)</button>
            <button class="btn-rest-timer" id="btn-rest-toggle" onclick="toggleRestBreak()">NGHỈ 30 PHÚT</button>
            <button class="btn-dismiss-alert" id="btn-dismiss-alert" onclick="dismissClinicalAlert()">XÁC NHẬN ĐÃ XỬ LÝ</button>
        </div>
    </div>

    <div class="main-wrapper">
        <!-- HÀNG 1: HỒ SƠ BỆNH NHÂN + 3 THẺ ĐỒNG BỘ 1-1 VỚI WEB LOCAL -->
        <section class="row-vitals">
            <div class="panel patient-card">
                <div class="patient-profile">
                    <div class="avatar-initials" id="patient-initials">NA</div>
                    <div class="patient-info">
                        <h2>
                            <span id="patient-display-name">Nguyễn Văn An</span>
                            <span class="badge-id" onclick="openProfileModal()" id="patient-code" title="Nhấn để đổi hồ sơ">ĐỔI HỒ SƠ</span>
                            <span class="state-chip" id="patient-state-badge">ĐANG MỞ</span>
                        </h2>
                        <div class="patient-details">
                            <span id="patient-location" style="color:var(--text-primary); font-weight:700;">Phòng Chăm Sóc 01</span><br>
                            <span style="color: var(--text-dim)" id="lbl-patient-diag">Đối tượng:</span>
                            <span id="patient-diag">Suy giảm vận động / Hội chứng khóa trong (LIS)</span>
                        </div>
                    </div>
                </div>
            </div>

            <!-- THẺ 2: LỰC PHẢN HỒI THAO TÁC -->
            <div class="panel metric-card">
                <div class="metric-header">
                    <span id="lbl-force-title">Lực Phản Hồi Thao Tác</span>
                    <span id="force-status" style="color:var(--cyan)">SẴN SÀNG</span>
                </div>
                <div class="metric-val-row">
                    <span class="metric-value" style="color:var(--cyan)" id="val-force">0%</span>
                    <span class="metric-unit" id="unit-force">IR / IMU</span>
                </div>
                <div class="progress-bar-bg">
                    <div class="progress-bar-fill" id="bar-force" style="width: 0%; background: var(--cyan);"></div>
                </div>
                <div class="metric-sub-hint" id="hint-force">Nháy mắt (90–420ms) hoặc Gật đầu (&gt; 12°)</div>
            </div>

            <!-- THẺ 3: ĐỘ MỎI CƠ CỔ CFI -->
            <div class="panel metric-card">
                <div class="metric-header">
                    <span id="lbl-cfi-title">Độ Mỏi Cơ Cổ (CFI)</span>
                    <span id="cfi-status" style="color:var(--emerald)">TỐI ƯU</span>
                </div>
                <div class="metric-val-row">
                    <span class="metric-value" style="color:var(--amber)" id="val-cfi">12%</span>
                    <span class="metric-unit" id="unit-cfi">/ 75% ngưỡng</span>
                </div>
                <div class="progress-bar-bg">
                    <div class="progress-bar-fill" id="bar-cfi" style="width: 12%; background: var(--emerald);"></div>
                </div>
                <div class="metric-sub-hint" id="cfi-hint">Tự động cảnh báo & gợi ý chuyển Chỉ Mắt khi ≥ 75%</div>
            </div>

            <!-- THẺ 4: NGUỒN HỆ THỐNG (TỰ NHẬN DIỆN USB 5V VS PIN LI-PO) -->
            <div class="panel metric-card">
                <div class="metric-header">
                    <span id="lbl-pwr-title">Nguồn Hệ Thống</span>
                    <span id="pwr-status" style="color:var(--emerald)">ỔN ĐỊNH</span>
                </div>
                <div class="metric-val-row">
                    <span class="metric-value" style="color:var(--emerald)" id="val-bat">100%</span>
                    <span class="metric-unit" id="val-volt" style="font-size:14px; color:var(--text-secondary); margin-left:6px;">5.00V</span>
                    <span class="metric-unit" id="val-pwr" style="margin-left:auto; font-size:12.5px;">0 mW</span>
                </div>
                <div class="progress-bar-bg">
                    <div class="progress-bar-fill" id="bar-bat" style="width: 100%; background: var(--emerald);"></div>
                </div>
                <div class="metric-sub-hint" id="hint-pwr">Tự động nhận diện Nguồn USB 5V / Pin Li-Po</div>
            </div>
        </section>

        <!-- HÀNG 2: HỆ CHUYÊN GIA Y SINH V4 HYBRID (4 KHỐI CẢM BIẾN THỰC) -->
        <section class="biomarkers-container">
            <div class="biomarkers-header">
                <div class="biomarkers-title">
                    <span class="dot dot-green"></span>
                    <span id="lbl-bio-section-title">HỆ CHUYÊN GIA Y SINH & AN TOÀN 24/7 • V4 HYBRID</span>
                </div>
                <div style="font-size:11.5px; font-family:ui-monospace, monospace; color:var(--text-dim);" id="lbl-bio-section-sub">
                    HỢP NHẤT TCRT5000 (VI SAI 60µs • EMA KÉP) + LSM6DS3 (6-DOF • IMU VETO)
                </div>
            </div>

            <div class="biomarkers-grid">
                <!-- Ô 1: CỤM CẢM BIẾN KÉP & CỔNG IMU VETO -->
                <div class="bio-box">
                    <div>
                        <div class="bio-top">
                            <span style="font-weight:800; color:var(--text-primary);" id="lbl-bio-sensor">CẢM BIẾN KÉP IR + IMU</span>
                            <span class="bio-status-badge" id="bio-sensor-badge" style="color:var(--emerald)">SẴN SÀNG</span>
                        </div>
                        <div class="bio-main-val">
                            <span id="bio-sensor-val">Kính IR + IMU</span>
                            <span class="metric-pct" id="bio-sensor-sub">Độ nhạy: Kép</span>
                        </div>
                        <div class="progress-bar-bg">
                            <div class="progress-bar-fill" id="bar-bio-sensor" style="width:100%; background:var(--emerald)"></div>
                        </div>
                    </div>
                    <div class="bio-desc" id="desc-bio-sensor">Lọc EMA kép khử nắng & cổng IMU Veto (&gt;45°/s)</div>
                </div>

                <!-- Ô 2: TƯ THẾ ĐẦU 3D (PITCH / ROLL 6-DOF) -->
                <div class="bio-box">
                    <div>
                        <div class="bio-top">
                            <span style="font-weight:800; color:var(--text-primary);" id="lbl-bio-pose">TƯ THẾ ĐẦU 6-DOF</span>
                            <span class="bio-status-badge" id="bio-pose-badge" style="color:var(--cyan)">CÂN BẰNG</span>
                        </div>
                        <div class="bio-main-val">
                            <span id="bio-pose-val">0.0° / 0.0°</span>
                            <span class="metric-pct" id="bio-pose-sub">Pitch / Roll</span>
                        </div>
                        <div class="progress-bar-bg">
                            <div class="progress-bar-fill" id="bar-bio-pose" style="width:15%; background:var(--cyan)"></div>
                        </div>
                    </div>
                    <div class="bio-desc" id="desc-bio-pose">Lọc bù 6-DOF LSM6DS3 • Gật &gt;12° chọn ô, giữ 1,2s khóa</div>
                </div>

                <!-- Ô 3: GIÁM SÁT AN TOÀN & CHỐNG LOÉT 120 PHÚT THẬT -->
                <div class="bio-box">
                    <div>
                        <div class="bio-top">
                            <span style="font-weight:800; color:var(--text-primary);" id="lbl-bio-safety">AN TOÀN & CHỐNG LOÉT</span>
                            <span class="bio-status-badge" id="bio-safety-badge" style="color:var(--emerald)">AN TOÀN</span>
                        </div>
                        <div class="bio-main-val">
                            <span id="bio-safety-val">0 / 120p</span>
                            <span class="metric-pct" id="bio-safety-sub">Đang Thức</span>
                        </div>
                        <div class="progress-bar-bg">
                            <div class="progress-bar-fill" id="bar-bio-safety" style="width:4%; background:var(--emerald)"></div>
                        </div>
                    </div>
                    <div class="bio-desc" id="desc-bio-safety">Tiến trình bất động thật từ LSM6DS3 (tự đặt lại khi xoay &gt;8,5°)</div>
                </div>

                <!-- Ô 4: NHIỆT ĐỘ CẢM BIẾN & GATEWAY NHÀ THÔNG MINH -->
                <div class="bio-box">
                    <div>
                        <div class="bio-top">
                            <span style="font-weight:800; color:var(--text-primary);" id="lbl-bio-env">NHIỆT ĐỘ & GATEWAY C3</span>
                            <span class="bio-status-badge" id="bio-env-badge" style="color:var(--cyan)">BÌNH THƯỜNG</span>
                        </div>
                        <div class="bio-main-val">
                            <span id="bio-env-val">34.5°C</span>
                            <span class="metric-pct" id="bio-env-sub">Đèn: Tắt • Quạt: Tắt</span>
                        </div>
                        <div class="progress-bar-bg">
                            <div class="progress-bar-fill" id="bar-bio-env" style="width:65%; background:var(--cyan)"></div>
                        </div>
                    </div>
                    <div class="bio-desc" id="desc-bio-env">Nhiệt độ LSM6DS3 & trạng thái 2 kênh Rơ-le không dây UDP</div>
                </div>
            </div>
        </section>

        <!-- THANH ĐIỀU KHIỂN NHANH TỪ XA CHO NGƯỜI THÂN / ĐIỀU DƯỠNG -->
        <section class="remote-control-bar">
            <div class="remote-group">
                <span class="remote-label" id="lbl-rc-mode">Chế độ cảm biến Kính:</span>
                <button class="rc-btn active" id="rc-mode-hybrid" onclick="sendRemoteCommand('INPUT_HYBRID')">Kính IR + IMU</button>
                <button class="rc-btn" id="rc-mode-ir" onclick="sendRemoteCommand('INPUT_IR')">Chỉ Mắt (IR)</button>
                <button class="rc-btn" id="rc-mode-nod" onclick="sendRemoteCommand('INPUT_NOD')">Chỉ Đầu (IMU)</button>
            </div>

            <div class="remote-group">
                <span class="remote-label" id="lbl-rc-iot">Gateway Nhà Thông Minh:</span>
                <button class="rc-btn" id="rc-relay-1" onclick="toggleRemoteRelay(1)">💡 Đèn (R1): Tắt</button>
                <button class="rc-btn" id="rc-relay-2" onclick="toggleRemoteRelay(2)">🌀 Quạt (R2): Tắt</button>
                <button class="rc-btn" id="rc-ctrl-lock" onclick="toggleRemoteControlLock()">🔓 Khóa Tạm Nghỉ</button>
            </div>

            <span id="rc-feedback">Sẵn sàng điều khiển từ xa (Cloud → Kính S3 → Gateway C3)</span>
        </section>

        <!-- HÀNG 3: ĐỒ THỊ TÍN HIỆU 2 KÊNH (IR DELTA THẬT & PITCH) + RADAR V4 HYBRID -->
        <section class="row-charts">
            <div class="chart-panel">
                <div class="section-title">
                    <span id="lbl-chart-title">ĐỒ THỊ TÍN HIỆU VI SAI HỒNG NGOẠI 60µs (irDelta) & GÓC ĐẦU 6-DOF</span>
                    <span id="signal-text" style="color: var(--amber); font-size: 11.5px;">ĐANG CHỜ KÍNH KẾT NỐI</span>
                </div>
                <div class="chart-container-wave">
                    <div id="no-signal-overlay">
                        <div style="color: var(--amber); font-weight: 900; font-size: 16px;" id="lbl-no-sig-title">CHỜ TÍN HIỆU TỪ KÍNH THÔNG MINH V4 HYBRID</div>
                        <div style="color: var(--text-secondary); font-size: 12.5px; max-width: 560px;" id="lbl-no-sig-desc">
                            Kiểm tra nguồn trên càng kính, kết nối Wi-Fi đồng bộ Cloud hoặc nhấn “Mô Phỏng Dữ Liệu” để xem tín hiệu mẫu.
                        </div>
                    </div>
                    <canvas id="sensorChart"></canvas>
                </div>
            </div>

            <div class="chart-panel">
                <div class="section-title"><span id="lbl-radar-title">HỒ SƠ VẬN ĐỘNG & CẢM BIẾN (V4 HYBRID)</span></div>
                <div class="chart-container-radar">
                    <canvas id="radarChart"></canvas>
                </div>
                <div class="mini-metric-list">
                    <div class="mini-metric-row">
                        <div class="metric-lbl"><span class="m-dot" style="background:var(--cyan)"></span><span id="rl-force">Lực phản hồi thao tác</span></div>
                        <span class="m-val" id="m-force">0%</span>
                    </div>
                    <div class="mini-metric-row">
                        <div class="metric-lbl"><span class="m-dot" style="background:var(--emerald)"></span><span id="rl-stability">Độ ổn định nền cảm biến</span></div>
                        <span class="m-val" id="m-stability">95%</span>
                    </div>
                    <div class="mini-metric-row">
                        <div class="metric-lbl"><span class="m-dot" style="background:var(--indigo)"></span><span id="rl-neck">Thể lực cơ cổ (100 - CFI)</span></div>
                        <span class="m-val" id="m-neck">88%</span>
                    </div>
                    <div class="mini-metric-row">
                        <div class="metric-lbl"><span class="m-dot" style="background:var(--amber)"></span><span id="rl-angle">Biên độ gật đầu (Pitch)</span></div>
                        <span class="m-val" id="m-angle">0.0°</span>
                    </div>
                    <div class="mini-metric-row">
                        <div class="metric-lbl"><span class="m-dot" style="background:var(--rose)"></span><span id="rl-tkeo">Xung vi sai IR (irDelta)</span></div>
                        <span class="m-val" id="m-tkeo">0.0</span>
                    </div>
                </div>
            </div>
        </section>

        <!-- HÀNG 4: THỐNG KÊ 6 NHÓM NHU CẦU (KHỚP 100% WEB LOCAL) & NHẬT KÝ GIAO TIẾP -->
        <section class="row-bottom">
            <div class="stats-col">
                <div class="stat-box">
                    <div class="section-title" style="margin-bottom: 12px;" id="lbl-dist-title">PHÂN BỔ 6 NHÓM NHU CẦU (ĐỒNG BỘ LOCAL)</div>
                    <div class="dist-item">
                        <div class="dist-info"><span id="lbl-dist-pain">Báo Đau & Cấp Cứu</span><span id="txt-pain" class="dist-count">0 (0%)</span></div>
                        <div class="progress-bar-bg"><div class="progress-bar-fill" id="bar-pain" style="width:0%; background:var(--rose)"></div></div>
                    </div>
                    <div class="dist-item">
                        <div class="dist-info"><span id="lbl-dist-food">Ăn Uống & Thuốc</span><span id="txt-food" class="dist-count">0 (0%)</span></div>
                        <div class="progress-bar-bg"><div class="progress-bar-fill" id="bar-food" style="width:0%; background:var(--cyan)"></div></div>
                    </div>
                    <div class="dist-item">
                        <div class="dist-info"><span id="lbl-dist-hygiene">Vệ Sinh & Tư Thế</span><span id="txt-hygiene" class="dist-count">0 (0%)</span></div>
                        <div class="progress-bar-bg"><div class="progress-bar-fill" id="bar-hygiene" style="width:0%; background:var(--emerald)"></div></div>
                    </div>
                    <div class="dist-item">
                        <div class="dist-info"><span id="lbl-dist-care">Gọi Người & Y Tế</span><span id="txt-care" class="dist-count">0 (0%)</span></div>
                        <div class="progress-bar-bg"><div class="progress-bar-fill" id="bar-care" style="width:0%; background:var(--amber)"></div></div>
                    </div>
                    <div class="dist-item">
                        <div class="dist-info"><span id="lbl-dist-iot">Đèn Quạt & Phòng</span><span id="txt-iot" class="dist-count">0 (0%)</span></div>
                        <div class="progress-bar-bg"><div class="progress-bar-fill" id="bar-iot" style="width:0%; background:var(--indigo)"></div></div>
                    </div>
                    <div class="dist-item">
                        <div class="dist-info"><span id="lbl-dist-feel">Cảm Xúc & Trò Chuyện</span><span id="txt-feel" class="dist-count">0 (0%)</span></div>
                        <div class="progress-bar-bg"><div class="progress-bar-fill" id="bar-feel" style="width:0%; background:var(--text-secondary)"></div></div>
                    </div>
                </div>

                <div class="stat-box">
                    <div class="section-title" id="lbl-hourly-title">MẬT ĐỘ GIAO TIẾP THEO KHUNG GIỜ</div>
                    <div class="chart-wrapper">
                        <div class="bar-col" id="h-0"></div>
                        <div class="bar-col" id="h-4"></div>
                        <div class="bar-col" id="h-8"></div>
                        <div class="bar-col" id="h-12"></div>
                        <div class="bar-col" id="h-16"></div>
                        <div class="bar-col" id="h-20"></div>
                    </div>
                    <div class="x-labels">
                        <span>0-4h</span><span>4-8h</span><span>8-12h</span><span>12-16h</span><span>16-20h</span><span>20-24h</span>
                    </div>
                    <div style="font-size: 11.5px; font-weight: 700; margin-top: 10px; color: var(--text-dim);" id="peak-text">Khung giờ cao điểm: Chưa ghi nhận</div>
                </div>
            </div>

            <div class="history-card">
                <div class="history-header">
                    <div class="history-title">
                        <span id="lbl-history-title">NHẬT KÝ GIAO TIẾP & CẢNH BÁO</span>
                        <span style="background:var(--bg-card); border:1px solid var(--border); color:var(--cyan); padding:2px 9px; border-radius:8px; font-size:11px;" id="count-badge">0 MỤC</span>
                    </div>

                    <div class="filters-group">
                        <input type="text" id="search-input" class="select-filter" placeholder="Tìm nội dung..." onkeyup="applyFilters()" style="width:145px;">
                        <select id="category-filter" class="select-filter" onchange="applyFilters()">
                            <option value="all" id="opt-cat-all">Tất cả nhóm</option>
                            <option value="pain" id="opt-cat-pain">Báo Đau & Cấp Cứu</option>
                            <option value="food" id="opt-cat-food">Ăn Uống & Thuốc</option>
                            <option value="hygiene" id="opt-cat-hygiene">Vệ Sinh & Tư Thế</option>
                            <option value="care" id="opt-cat-care">Gọi Người & Y Tế</option>
                            <option value="iot" id="opt-cat-iot">Đèn Quạt & Phòng</option>
                            <option value="feel" id="opt-cat-feel">Cảm Xúc & Trò Chuyện</option>
                        </select>
                        <select id="time-filter" class="select-filter" onchange="applyFilters()">
                            <option value="all" id="opt-time-all">Tất cả thời gian</option>
                            <option value="day" id="opt-time-day">Hôm nay</option>
                            <option value="week" id="opt-time-week">7 ngày qua</option>
                            <option value="month" id="opt-time-month">Tháng này</option>
                        </select>
                        <div class="export-group">
                            <button class="btn-export" id="btn-export-csv" onclick="exportCSV()">XUẤT CSV</button>
                            <button class="btn-export" id="btn-export-json" onclick="exportJSON()">XUẤT JSON</button>
                        </div>
                    </div>
                </div>

                <div class="table-container">
                    <table>
                        <thead>
                            <tr>
                                <th width="15%" id="th-time">Thời Gian</th>
                                <th width="42%" id="th-sentence">Nội Dung Phát Ngôn / Cảnh Báo</th>
                                <th width="17%" id="th-cat">Nhóm Nhu Cầu</th>
                                <th width="8%" style="text-align:center" id="th-force">Lực (%)</th>
                                <th width="8%" style="text-align:center" id="th-status">Ổn Định</th>
                                <th width="10%" style="text-align:center" id="th-action">Giọng Đọc</th>
                            </tr>
                        </thead>
                        <tbody id="logs-body">
                            <tr>
                                <td colspan="6" style="text-align:center; padding: 50px; color: var(--text-dim); font-size: 13px;" id="lbl-waiting-logs">
                                    Hệ thống đang chờ bản tin giao tiếp từ Kính thông minh V4 Hybrid...
                                </td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </section>

        <div class="footer-note" id="lbl-footer">
            HỆ THỐNG HỖ TRỢ GIAO TIẾP DÀNH RIÊNG CHO NGƯỜI SUY GIẢM Ý THỨC – VẬN ĐỘNG • KÍNH THÔNG MINH V4 HYBRID (PRO-CAD V8.2) • THIẾT BỊ HỖ TRỢ, KHÔNG THAY THẾ CHẨN ĐOÁN Y KHOA
        </div>
    </div>

    <!-- MODAL HỒ SƠ BỆNH NHÂN (ĐÃ DỊCH ĐẦY ĐỦ VI / EN) -->
    <div id="user-modal" class="modal-overlay">
        <div class="modal-card">
            <div class="modal-header">
                <div class="modal-title" id="modal-title">HỒ SƠ NGƯỜI SỬ DỤNG</div>
                <button onclick="closeModal()" id="btn-close-modal" style="background:transparent; border:none; color:var(--text-dim); font-size:13px; font-weight:800; cursor:pointer; display:none;">ĐÓNG</button>
            </div>
            <div id="view-select-user">
                <div class="user-list" id="saved-user-list"></div>
                <div style="display: flex; gap: 8px; justify-content: flex-end; margin-top: 14px;">
                    <button class="btn" id="btn-modal-create" onclick="showCreateUserView()">THÊM MỚI</button>
                    <button class="btn-primary" id="btn-modal-confirm" onclick="confirmSelectUser()">XÁC NHẬN CHỌN</button>
                </div>
            </div>
            <div id="view-create-user" style="display: none;">
                <label id="lbl-m-name" style="font-size:11px; color:var(--text-dim); font-weight:800;">HỌ TÊN NGƯỜI BỆNH</label>
                <input type="text" id="input-new-name" class="form-input" placeholder="VD: Nguyễn Văn An">

                <div style="display: flex; gap: 10px;">
                    <div style="flex:1">
                        <label id="lbl-m-age" style="font-size:11px; color:var(--text-dim); font-weight:800;">TUỔI</label>
                        <input type="number" id="input-new-age" class="form-input" placeholder="Tuổi">
                    </div>
                    <div style="flex:1">
                        <label id="lbl-m-code" style="font-size:11px; color:var(--text-dim); font-weight:800;">MÃ HỒ SƠ</label>
                        <input type="text" id="input-new-code" class="form-input" placeholder="VD: SG-01">
                    </div>
                </div>

                <label id="lbl-m-room" style="font-size:11px; color:var(--text-dim); font-weight:800;">PHÒNG / ĐỊA ĐIỂM CHĂM SÓC</label>
                <input type="text" id="input-new-room" class="form-input" placeholder="VD: Phòng Chăm Sóc 01 / Tại nhà">

                <label id="lbl-m-diag" style="font-size:11px; color:var(--text-dim); font-weight:800;">TÌNH TRẠNG VẬN ĐỘNG</label>
                <input type="text" id="input-new-diag" class="form-input" placeholder="VD: Hội chứng khóa trong (LIS) / Sau tai biến">

                <div style="display: flex; gap: 8px; justify-content: flex-end; margin-top: 14px;">
                    <button class="btn" id="btn-cancel-create" onclick="cancelCreateUser()">QUAY LẠI</button>
                    <button class="btn-primary" id="btn-save-patient" onclick="saveNewPatient()">LƯU HỒ SƠ</button>
                </div>
            </div>
        </div>
    </div>

    <script>
        function setText(id, text) {
            const el = document.getElementById(id);
            if (el) el.innerText = text;
        }
        function escapeHtml(str) {
            return String(str || '')
                .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
                .replace(/"/g, '&quot;').replace(/'/g, '&#39;');
        }

        // Luôn dùng API_BASE tương đối khi deploy trên Render (không hardcode URL)
        const API_BASE = window.location.protocol.startsWith('http') ? '' : 'https://bci-api-server.onrender.com';
        const CLOUD_COMMAND_URL = `${API_BASE}/api/command`;

        // ================= ĐỒNG HỒ & TUỔI BẢN TIN ĐỒNG BỘ =================
        let lastServerTelemetryAt = 0;
        function updateClock() {
            const now = new Date();
            setText('clock-time', now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', hour12: false }));
            setText('clock-date', `${String(now.getDate()).padStart(2, '0')}/${String(now.getMonth() + 1).padStart(2, '0')}/${now.getFullYear()}`);

            if (isDemoMode) {
                setText('sync-age-text', currentLang === 'vi' ? '(mô phỏng)' : '(demo)');
            } else if (lastServerTelemetryAt > 0) {
                const ageSec = Math.max(0, Math.floor((Date.now() - lastServerTelemetryAt) / 1000));
                setText('sync-age-text', currentLang === 'vi' ? `(${ageSec}s trước)` : `(${ageSec}s ago)`);
            } else {
                setText('sync-age-text', '(--)');
            }
        }
        setInterval(updateClock, 1000);
        updateClock();

        // ================= WEB NOTIFICATION API (CẢNH BÁO RA MÀN HÌNH ĐIỆN THOẠI) =================
        function updateNotifButtonUI() {
            const btn = document.getElementById('btn-notif');
            if (!btn) return;
            if (!('Notification' in window)) {
                btn.style.display = 'none';
                return;
            }
            const t = i18n[currentLang];
            if (Notification.permission === 'granted') {
                btn.classList.add('granted');
                btn.innerText = t.notif_on;
            } else {
                btn.classList.remove('granted');
                btn.innerText = t.notif_ask;
            }
        }

        function requestNotifPermission() {
            if (!('Notification' in window)) return;
            Notification.requestPermission().then(() => {
                updateNotifButtonUI();
                if (Notification.permission === 'granted') {
                    pushBrowserNotification(
                        currentLang === 'vi' ? 'Kính Thông Minh V4 Hybrid' : 'Smart Glasses V4 Hybrid',
                        currentLang === 'vi' ? 'Đã bật thông báo khẩn cấp SOS & Co giật cho người thân.' : 'Emergency SOS & Seizure notifications enabled.'
                    );
                }
            });
        }

        function pushBrowserNotification(title, body) {
            try {
                if ('Notification' in window && Notification.permission === 'granted') {
                    new Notification(title, { body, requireInteraction: true });
                }
            } catch (e) { }
        }

        // ================= ÂM THANH CÒI CẢNH BÁO WEB AUDIO =================
        let audioCtx = null;
        function playAlarmSiren(level = 'HIGH') {
            try {
                if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
                if (audioCtx.state === 'suspended') audioCtx.resume();
                const osc = audioCtx.createOscillator();
                const gain = audioCtx.createGain();
                osc.connect(gain);
                gain.connect(audioCtx.destination);
                const now = audioCtx.currentTime;
                if (level === 'HIGH') {
                    osc.type = 'sawtooth';
                    osc.frequency.setValueAtTime(780, now);
                    osc.frequency.linearRampToValueAtTime(1380, now + 0.22);
                    osc.frequency.linearRampToValueAtTime(780, now + 0.45);
                    gain.gain.setValueAtTime(0.22, now);
                    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.50);
                    osc.start(now); osc.stop(now + 0.50);
                } else {
                    osc.type = 'sine';
                    osc.frequency.setValueAtTime(880, now);
                    gain.gain.setValueAtTime(0.14, now);
                    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.25);
                    osc.start(now); osc.stop(now + 0.25);
                }
            } catch (e) { }
        }

        // ================= TỪ ĐIỂN SONG NGỮ HOÀN CHỈNH (VI / EN) =================
        let currentLang = localStorage.getItem('smartglass_lang') || 'vi';
        let currentTheme = localStorage.getItem('smartglass_theme') || 'dark';

        const i18n = {
            vi: {
                brand_title: 'Kính Thông Minh V4 Hybrid — Giám Sát Từ Xa',
                brand_sub: 'TCRT5000 (IR 200Hz • 60µs) + LSM6DS3 (IMU 6-DOF) • ESP32-S3 + GATEWAY C3 • CLOUD RENDER',
                btn_lang: 'Ngôn ngữ: VI',
                btn_theme_light: '☀️ Nền: Sáng',
                btn_theme_dark: '🌙 Nền: Tối',
                notif_ask: '🔔 Bật Thông Báo',
                notif_on: '🔔 Thông Báo: Bật',
                conn_online: 'KÍNH ONLINE',
                conn_offline: 'KÍNH OFFLINE',
                conn_demo: 'ĐANG MÔ PHỎNG',
                cloud_ok: 'CLOUD SẴN SÀNG',
                demo_off: 'Mô Phỏng Dữ Liệu',
                demo_on: 'Tắt Mô Phỏng',
                btn_test: 'Thử Cảnh Báo',
                opt_test_cfi: 'Mỏi cổ (CFI ≥ 75%)',
                opt_test_sos: 'Gọi cấp cứu (SOS)',
                opt_test_spasm: 'Co giật / Té ngã (≥ 2,5g)',
                opt_test_bedsore: 'Chống loét (120 phút)',
                btn_banner_ir: 'CHUYỂN CHỈ MẮT (IR)',
                btn_rest: 'NGHỈ 30 PHÚT',
                btn_dismiss: 'XÁC NHẬN ĐÃ XỬ LÝ',
                btn_profile: 'ĐỔI HỒ SƠ',
                lbl_diag: 'Đối tượng:',
                lbl_force: 'Lực Phản Hồi Thao Tác',
                hint_force: 'Nháy mắt (90–420ms) hoặc Gật đầu (> 12°)',
                lbl_cfi: 'Độ Mỏi Cơ Cổ (CFI)',
                unit_cfi: '/ 75% ngưỡng',
                hint_cfi: 'Tự động cảnh báo & gợi ý chuyển Chỉ Mắt khi ≥ 75%',
                lbl_pwr_usb: 'Nguồn USB 5V / Sạc',
                lbl_pwr_lipo: 'Dung Lượng Pin Li-Po',
                hint_pwr_usb: 'Đang cấp nguồn trực tiếp USB 5V (Chế độ không ADC pin)',
                hint_pwr_lipo: '2×300mAh đối xứng 2 càng kính • Bảo vệ nguồn',
                bio_title: 'HỆ CHUYÊN GIA Y SINH & AN TOÀN 24/7 • V4 HYBRID',
                bio_sub: 'HỢP NHẤT TCRT5000 (VI SAI 60µs • EMA KÉP) + LSM6DS3 (6-DOF • IMU VETO)',
                bio_sensor: 'CẢM BIẾN KÉP IR + IMU',
                desc_sensor: 'Lọc EMA kép khử nắng & cổng IMU Veto (>45°/s)',
                bio_pose: 'TƯ THẾ ĐẦU 6-DOF',
                desc_pose: 'Lọc bù 6-DOF LSM6DS3 • Gật >12° chọn ô, giữ 1,2s khóa',
                bio_safety: 'AN TOÀN & CHỐNG LOÉT',
                desc_safety: 'Tiến trình bất động thật từ LSM6DS3 (tự đặt lại khi xoay >8,5°)',
                bio_env: 'NHIỆT ĐỘ & GATEWAY C3',
                desc_env: 'Nhiệt độ LSM6DS3 & trạng thái 2 kênh Rơ-le không dây UDP',
                lbl_rc_mode: 'Chế độ cảm biến Kính:',
                lbl_rc_iot: 'Gateway Nhà Thông Minh:',
                rc_hybrid: 'Kính IR + IMU',
                rc_ir: 'Chỉ Mắt (IR)',
                rc_nod: 'Chỉ Đầu (IMU)',
                chart_title: 'ĐỒ THỊ TÍN HIỆU VI SAI HỒNG NGOẠI 60µs (irDelta) & GÓC ĐẦU 6-DOF',
                sig_good: 'TÍN HIỆU ỔN ĐỊNH (IR + IMU)',
                sig_lost: 'CHỜ TÍN HIỆU CẢM BIẾN',
                no_sig_title: 'CHỜ TÍN HIỆU TỪ KÍNH THÔNG MINH V4 HYBRID',
                no_sig_desc: 'Kiểm tra nguồn trên càng kính, kết nối Wi-Fi đồng bộ Cloud hoặc nhấn “Mô Phỏng Dữ Liệu” để xem tín hiệu mẫu.',
                radar_title: 'HỒ SƠ VẬN ĐỘNG & CẢM BIẾN (V4 HYBRID)',
                rl_force: 'Lực phản hồi thao tác',
                rl_stability: 'Độ ổn định nền cảm biến',
                rl_neck: 'Thể lực cơ cổ (100 - CFI)',
                rl_angle: 'Biên độ gật đầu (Pitch)',
                rl_tkeo: 'Xung vi sai IR (irDelta)',
                dist_title: 'PHÂN BỔ 6 NHÓM NHU CẦU (ĐỒNG BỘ LOCAL)',
                dist_pain: 'Báo Đau & Cấp Cứu',
                dist_food: 'Ăn Uống & Thuốc',
                dist_hygiene: 'Vệ Sinh & Tư Thế',
                dist_care: 'Gọi Người & Y Tế',
                dist_iot: 'Đèn Quạt & Phòng',
                dist_feel: 'Cảm Xúc & Trò Chuyện',
                hourly_title: 'MẬT ĐỘ GIAO TIẾP THEO KHUNG GIỜ',
                peak_prefix: 'Khung giờ cao điểm: ',
                peak_none: 'Chưa ghi nhận',
                history_title: 'NHẬT KÝ GIAO TIẾP & CẢNH BÁO',
                search_ph: 'Tìm nội dung...',
                opt_cat_all: 'Tất cả nhóm',
                opt_time_all: 'Tất cả thời gian',
                opt_time_day: 'Hôm nay',
                opt_time_week: '7 ngày qua',
                opt_time_month: 'Tháng này',
                btn_csv: 'XUẤT CSV',
                btn_json: 'XUẤT JSON',
                th_time: 'Thời Gian',
                th_sentence: 'Nội Dung Phát Ngôn / Cảnh Báo',
                th_cat: 'Nhóm Nhu Cầu',
                th_force: 'Lực (%)',
                th_status: 'Ổn Định',
                th_action: 'Giọng Đọc',
                btn_speak: 'ĐỌC LẠI',
                waiting_logs: 'Hệ thống đang chờ bản tin giao tiếp từ Kính thông minh V4 Hybrid...',
                footer: 'HỆ THỐNG HỖ TRỢ GIAO TIẾP DÀNH RIÊNG CHO NGƯỜI SUY GIẢM Ý THỨC – VẬN ĐỘNG • KÍNH THÔNG MINH V4 HYBRID (PRO-CAD V8.2) • THIẾT BỊ HỖ TRỢ, KHÔNG THAY THẾ CHẨN ĐOÁN Y KHOA',
                modal_title: 'HỒ SƠ NGƯỜI SỬ DỤNG',
                btn_close: 'ĐÓNG',
                btn_create: 'THÊM MỚI',
                btn_confirm: 'XÁC NHẬN CHỌN',
                btn_back: 'QUAY LẠI',
                btn_save: 'LƯU HỒ SƠ',
                m_name: 'HỌ TÊN NGƯỜI BỆNH',
                m_age: 'TUỔI',
                m_code: 'MÃ HỒ SƠ',
                m_room: 'PHÒNG / ĐỊA ĐIỂM CHĂM SÓC',
                m_diag: 'TÌNH TRẠNG VẬN ĐỘNG'
            },
            en: {
                brand_title: 'Smart Glasses V4 Hybrid — Remote Care Monitor',
                brand_sub: 'TCRT5000 (IR 200Hz • 60µs) + LSM6DS3 (6-DOF IMU) • ESP32-S3 + GATEWAY C3 • RENDER',
                btn_lang: 'Language: EN',
                btn_theme_light: '☀️ Light Mode',
                btn_theme_dark: '🌙 Dark Mode',
                notif_ask: '🔔 Enable Alerts',
                notif_on: '🔔 Alerts: ON',
                conn_online: 'GLASSES ONLINE',
                conn_offline: 'GLASSES OFFLINE',
                conn_demo: 'SIMULATION',
                cloud_ok: 'CLOUD READY',
                demo_off: 'Simulate Data',
                demo_on: 'Stop Simulation',
                btn_test: 'Test Alert',
                opt_test_cfi: 'Neck Fatigue (CFI ≥ 75%)',
                opt_test_sos: 'Emergency Call (SOS)',
                opt_test_spasm: 'Seizure / Fall (≥ 2.5g)',
                opt_test_bedsore: 'Repositioning (120 min)',
                btn_banner_ir: 'SWITCH TO IR-ONLY',
                btn_rest: 'REST 30 MIN',
                btn_dismiss: 'ACKNOWLEDGE',
                btn_profile: 'SWITCH USER',
                lbl_diag: 'Profile:',
                lbl_force: 'Gesture Response Force',
                hint_force: 'Intentional Blink (90–420ms) or Head Nod (> 12°)',
                lbl_cfi: 'Cervical Fatigue Index (CFI)',
                unit_cfi: '/ 75% limit',
                hint_cfi: 'Auto alert & suggest IR-Only mode at ≥ 75%',
                lbl_pwr_usb: 'USB 5V Power / Charging',
                lbl_pwr_lipo: 'Li-Po Battery Pack',
                hint_pwr_usb: 'Direct USB 5V power detected (No Battery ADC)',
                hint_pwr_lipo: 'Dual symmetric 2×300mAh • Power Protection',
                bio_title: 'BIOMEDICAL EXPERT SYSTEM & 24/7 SAFETY • V4 HYBRID',
                bio_sub: 'TCRT5000 (60µs DIFFERENTIAL • DUAL EMA) + LSM6DS3 (6-DOF • IMU VETO)',
                bio_sensor: 'DUAL SENSOR IR + IMU',
                desc_sensor: 'Sunlight rejection (60µs) & IMU Veto Gate (>45°/s)',
                bio_pose: '6-DOF HEAD POSTURE',
                desc_pose: 'Complementary 6-DOF LSM6DS3 • Nod >12° select, hold 1.2s lock',
                bio_safety: 'SAFETY & BEDSORE (120M)',
                desc_safety: 'Real static timer from LSM6DS3 (resets on >8.5° tilt shift)',
                bio_env: 'TEMPERATURE & GATEWAY C3',
                desc_env: 'LSM6DS3 thermal monitor & 2-ch Wireless UDP Relay',
                lbl_rc_mode: 'Glasses Input Mode:',
                lbl_rc_iot: 'Smart Home Gateway:',
                rc_hybrid: 'Hybrid IR + IMU',
                rc_ir: 'Eye Only (IR)',
                rc_nod: 'Head Only (IMU)',
                chart_title: 'REAL-TIME 60µs IR DIFFERENTIAL (irDelta) & 6-DOF HEAD MOTION',
                sig_good: 'SIGNAL STABLE (IR + IMU)',
                sig_lost: 'AWAITING SENSOR SIGNAL',
                no_sig_title: 'AWAITING V4 HYBRID SMART GLASSES SIGNAL',
                no_sig_desc: 'Check glasses power, Wi-Fi Cloud sync or click “Simulate Data” to preview live waveforms.',
                radar_title: 'MULTIMODAL SENSOR PROFILE (V4 HYBRID)',
                rl_force: 'Gesture Response Force',
                rl_stability: 'Sensor Baseline Stability',
                rl_neck: 'Neck Endurance (100 - CFI)',
                rl_angle: 'Head Nod Angle (Pitch)',
                rl_tkeo: 'IR Differential Pulse (irDelta)',
                dist_title: '6 AAC NEED CATEGORIES (LOCAL SYNC)',
                dist_pain: 'Pain & Emergency',
                dist_food: 'Food, Drink & Meds',
                dist_hygiene: 'Hygiene & Posture',
                dist_care: 'Call Family & Medical',
                dist_iot: 'Light, Fan & Room',
                dist_feel: 'Emotions & Chat',
                hourly_title: 'HOURLY COMMUNICATION DENSITY',
                peak_prefix: 'Peak Window: ',
                peak_none: 'None',
                history_title: 'COMMUNICATION & ALERT LOGS',
                search_ph: 'Search logs...',
                opt_cat_all: 'All Categories',
                opt_time_all: 'All Time',
                opt_time_day: 'Today',
                opt_time_week: 'Last 7 Days',
                opt_time_month: 'This Month',
                btn_csv: 'EXPORT CSV',
                btn_json: 'EXPORT JSON',
                th_time: 'Timestamp',
                th_sentence: 'Spoken Phrase / Alert',
                th_cat: 'Category',
                th_force: 'Force (%)',
                th_status: 'Stability',
                th_action: 'Voice',
                btn_speak: 'SPEAK',
                waiting_logs: 'Awaiting telemetry & messages from Smart Glasses V4 Hybrid...',
                footer: 'ASSISTIVE COMMUNICATION SYSTEM FOR MOTOR-IMPAIRED PATIENTS • SMART GLASSES V4 HYBRID (PRO-CAD V8.2) • ASSISTIVE DEVICE, NOT A MEDICAL DIAGNOSTIC INSTRUMENT',
                modal_title: 'PATIENT PROFILE',
                btn_close: 'CLOSE',
                btn_create: 'ADD NEW',
                btn_confirm: 'CONFIRM',
                btn_back: 'BACK',
                btn_save: 'SAVE PROFILE',
                m_name: 'FULL NAME',
                m_age: 'AGE',
                m_code: 'PROFILE ID',
                m_room: 'CARE ROOM / LOCATION',
                m_diag: 'MOTOR CONDITION'
            }
        };

        // ================= BIỂU ĐỒ 2 KÊNH (PITCH + IR DELTA THẬT) & RADAR =================
        Chart.defaults.font.family = 'Segoe UI, monospace';

        const sensorChart = new Chart(document.getElementById('sensorChart').getContext('2d'), {
            type: 'line',
            data: {
                labels: Array(50).fill(''),
                datasets: [
                    {
                        label: 'Góc gật đầu Pitch (°)',
                        data: Array(50).fill(0),
                        borderColor: '#38bdf8',
                        backgroundColor: 'rgba(56, 189, 248, 0.12)',
                        borderWidth: 2,
                        pointRadius: 0,
                        tension: 0.25,
                        fill: true
                    },
                    {
                        label: 'Xung vi sai IR (irDelta)',
                        data: Array(50).fill(0),
                        borderColor: '#fbbf24',
                        borderWidth: 1.8,
                        pointRadius: 0,
                        tension: 0.2,
                        fill: false
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    x: { display: false },
                    y: {
                        display: true,
                        suggestedMin: -25,
                        suggestedMax: 45,
                        grid: { color: '#1a2744' },
                        ticks: { color: '#94a3b8', font: { size: 10.5 } }
                    }
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top',
                        align: 'end',
                        labels: { color: '#94a3b8', boxWidth: 12, font: { size: 11, weight: '700' } }
                    }
                }
            }
        });

        const radarChart = new Chart(document.getElementById('radarChart').getContext('2d'), {
            type: 'radar',
            data: {
                labels: ['Lực chọn', 'Ổn định', 'Thể lực cổ', 'Góc gật', 'Xung IR'],
                datasets: [{
                    data: [0, 95, 88, 0, 0],
                    backgroundColor: 'rgba(56, 189, 248, 0.16)',
                    borderColor: '#38bdf8',
                    borderWidth: 2,
                    pointRadius: 3,
                    pointBackgroundColor: '#38bdf8'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    r: {
                        suggestedMin: 0,
                        suggestedMax: 100,
                        angleLines: { color: 'rgba(148, 163, 184, 0.16)' },
                        grid: { color: 'rgba(148, 163, 184, 0.16)' },
                        pointLabels: { color: '#94a3b8', font: { size: 11, weight: '700' } },
                        ticks: { display: false }
                    }
                },
                plugins: { legend: { display: false } }
            }
        });

        function applyTheme(theme) {
            currentTheme = theme;
            document.documentElement.setAttribute('data-theme', theme);
            localStorage.setItem('smartglass_theme', theme);
            const t = i18n[currentLang];
            setText('btn-theme', theme === 'light' ? t.btn_theme_dark : t.btn_theme_light);

            const isLight = theme === 'light';
            sensorChart.options.scales.y.grid.color = isLight ? '#cbd5e1' : '#1a2744';
            sensorChart.options.scales.y.ticks.color = isLight ? '#334155' : '#94a3b8';
            sensorChart.options.plugins.legend.labels.color = isLight ? '#334155' : '#94a3b8';
            sensorChart.data.datasets[0].borderColor = isLight ? '#0284c7' : '#38bdf8';
            sensorChart.update('none');

            radarChart.options.scales.r.pointLabels.color = isLight ? '#0f172a' : '#94a3b8';
            radarChart.options.scales.r.angleLines.color = isLight ? 'rgba(15,23,42,0.12)' : 'rgba(148,163,184,0.16)';
            radarChart.options.scales.r.grid.color = isLight ? 'rgba(15,23,42,0.12)' : 'rgba(148,163,184,0.16)';
            radarChart.data.datasets[0].borderColor = isLight ? '#0284c7' : '#38bdf8';
            radarChart.data.datasets[0].pointBackgroundColor = isLight ? '#0284c7' : '#38bdf8';
            radarChart.update('none');
        }

        function toggleTheme() { applyTheme(currentTheme === 'dark' ? 'light' : 'dark'); }
        function toggleLanguage() {
            currentLang = currentLang === 'vi' ? 'en' : 'vi';
            localStorage.setItem('smartglass_lang', currentLang);
            applyLanguage();
        }

        function applyLanguage() {
            const t = i18n[currentLang];
            setText('btn-lang', t.btn_lang);
            setText('btn-theme', currentTheme === 'light' ? t.btn_theme_dark : t.btn_theme_light);
            updateNotifButtonUI();
            setText('txt-brand-title', t.brand_title);
            setText('txt-brand-sub', t.brand_sub);
            setText('cloud-text', t.cloud_ok);
            setText('btn-demo', isDemoMode ? t.demo_on : t.demo_off);
            setText('btn-test-alert', t.btn_test);
            setText('opt-test-cfi', t.opt_test_cfi);
            setText('opt-test-sos', t.opt_test_sos);
            setText('opt-test-spasm', t.opt_test_spasm);
            setText('opt-test-bedsore', t.opt_test_bedsore);
            setText('btn-banner-ir', t.btn_banner_ir);
            setText('btn-rest-toggle', isRestBreakActive ? document.getElementById('btn-rest-toggle').innerText : t.btn_rest);
            setText('btn-dismiss-alert', t.btn_dismiss);
            setText('patient-code', t.btn_profile);
            setText('lbl-patient-diag', t.lbl_diag);

            setText('lbl-force-title', t.lbl_force);
            setText('hint-force', t.hint_force);
            setText('lbl-cfi-title', t.lbl_cfi);
            setText('unit-cfi', t.unit_cfi);
            setText('cfi-hint', t.hint_cfi);

            setText('lbl-bio-section-title', t.bio_title);
            setText('lbl-bio-section-sub', t.bio_sub);
            setText('lbl-bio-sensor', t.bio_sensor);
            setText('desc-bio-sensor', t.desc_sensor);
            setText('lbl-bio-pose', t.bio_pose);
            setText('desc-bio-pose', t.desc_pose);
            setText('lbl-bio-safety', t.bio_safety);
            setText('desc-bio-safety', t.desc_safety);
            setText('lbl-bio-env', t.bio_env);
            setText('desc-bio-env', t.desc_env);

            setText('lbl-rc-mode', t.lbl_rc_mode);
            setText('lbl-rc-iot', t.lbl_rc_iot);
            setText('rc-mode-hybrid', t.rc_hybrid);
            setText('rc-mode-ir', t.rc_ir);
            setText('rc-mode-nod', t.rc_nod);

            setText('lbl-chart-title', t.chart_title);
            setText('lbl-no-sig-title', t.no_sig_title);
            setText('lbl-no-sig-desc', t.no_sig_desc);
            setText('lbl-radar-title', t.radar_title);
            setText('rl-force', t.rl_force);
            setText('rl-stability', t.rl_stability);
            setText('rl-neck', t.rl_neck);
            setText('rl-angle', t.rl_angle);
            setText('rl-tkeo', t.rl_tkeo);

            setText('lbl-dist-title', t.dist_title);
            setText('lbl-dist-pain', t.dist_pain);
            setText('lbl-dist-food', t.dist_food);
            setText('lbl-dist-hygiene', t.dist_hygiene);
            setText('lbl-dist-care', t.dist_care);
            setText('lbl-dist-iot', t.dist_iot);
            setText('lbl-dist-feel', t.dist_feel);

            setText('opt-cat-all', t.opt_cat_all);
            setText('opt-cat-pain', t.dist_pain);
            setText('opt-cat-food', t.dist_food);
            setText('opt-cat-hygiene', t.dist_hygiene);
            setText('opt-cat-care', t.dist_care);
            setText('opt-cat-iot', t.dist_iot);
            setText('opt-cat-feel', t.dist_feel);

            setText('opt-time-all', t.opt_time_all);
            setText('opt-time-day', t.opt_time_day);
            setText('opt-time-week', t.opt_time_week);
            setText('opt-time-month', t.opt_time_month);

            setText('lbl-hourly-title', t.hourly_title);
            setText('lbl-history-title', t.history_title);
            const searchEl = document.getElementById('search-input');
            if (searchEl) searchEl.placeholder = t.search_ph;

            setText('btn-export-csv', t.btn_csv);
            setText('btn-export-json', t.btn_json);
            setText('th-time', t.th_time);
            setText('th-sentence', t.th_sentence);
            setText('th-cat', t.th_cat);
            setText('th-force', t.th_force);
            setText('th-status', t.th_status);
            setText('th-action', t.th_action);
            setText('lbl-footer', t.footer);

            // Dịch Modal hồ sơ
            setText('modal-title', t.modal_title);
            setText('btn-close-modal', t.btn_close);
            setText('btn-modal-create', t.btn_create);
            setText('btn-modal-confirm', t.btn_confirm);
            setText('btn-cancel-create', t.btn_back);
            setText('btn-save-patient', t.btn_save);
            setText('lbl-m-name', t.m_name);
            setText('lbl-m-age', t.m_age);
            setText('lbl-m-code', t.m_code);
            setText('lbl-m-room', t.m_room);
            setText('lbl-m-diag', t.m_diag);

            radarChart.data.labels = currentLang === 'vi'
                ? ['Lực chọn', 'Ổn định', 'Thể lực cổ', 'Góc gật', 'Xung IR']
                : ['Force', 'Stability', 'Neck', 'Pitch', 'IR Pulse'];
            radarChart.update('none');

            sensorChart.data.datasets[0].label = currentLang === 'vi' ? 'Góc gật đầu Pitch (°)' : 'Head Pitch Angle (°)';
            sensorChart.data.datasets[1].label = currentLang === 'vi' ? 'Xung vi sai IR (irDelta)' : 'IR Differential Pulse (irDelta)';
            sensorChart.update('none');

            applyFilters();
        }

        // ================= QUẢN LÝ HỒ SƠ NGƯỜI SỬ DỤNG =================
        let savedProfiles = JSON.parse(localStorage.getItem('smartglass_profiles') || '[]');
        if (savedProfiles.length === 0) {
            savedProfiles = [{
                id: 'pat-default',
                name: 'Nguyễn Văn An',
                code: 'SG-01',
                age: '62',
                room: 'Phòng Chăm Sóc 01',
                diag: 'Suy giảm vận động / Hội chứng khóa trong (LIS)'
            }];
            localStorage.setItem('smartglass_profiles', JSON.stringify(savedProfiles));
        }
        let currentPatientId = localStorage.getItem('smartglass_active_id') || savedProfiles[0].id;
        let currentPatient = null;
        let selectedPatientIdInModal = '';

        function initUserSystem() {
            currentPatient = savedProfiles.find(p => p.id === currentPatientId) || savedProfiles[0];
            if (currentPatient) applyCurrentPatientToUI(currentPatient);
        }

        function openProfileModal() {
            const listEl = document.getElementById('saved-user-list');
            listEl.innerHTML = '';
            savedProfiles.forEach(p => {
                const row = document.createElement('div');
                row.className = 'user-row';
                if (p.id === currentPatient?.id) {
                    selectedPatientIdInModal = p.id;
                    row.style.borderColor = 'var(--cyan)';
                }
                row.innerHTML = `<div>
                    <div style="font-weight:800; color:var(--text-primary); font-size:14px;">${escapeHtml(p.name)} <span style="font-size:11px; color:var(--cyan)">[${escapeHtml(p.code)}]</span></div>
                    <div style="color:var(--text-dim); font-size:11.5px; margin-top:2px;">${escapeHtml(p.room)} • ${escapeHtml(p.diag)}</div>
                </div>`;
                row.onclick = () => {
                    selectedPatientIdInModal = p.id;
                    document.querySelectorAll('.user-row').forEach(el => el.style.borderColor = 'var(--border)');
                    row.style.borderColor = 'var(--cyan)';
                };
                listEl.appendChild(row);
            });
            document.getElementById('view-select-user').style.display = 'block';
            document.getElementById('view-create-user').style.display = 'none';
            document.getElementById('btn-close-modal').style.display = currentPatient ? 'block' : 'none';
            document.getElementById('user-modal').style.display = 'flex';
        }

        function showCreateUserView() {
            document.getElementById('view-select-user').style.display = 'none';
            document.getElementById('view-create-user').style.display = 'block';
        }
        function cancelCreateUser() { if (savedProfiles.length > 0) openProfileModal(); }

        function saveNewPatient() {
            const name = document.getElementById('input-new-name').value.trim();
            if (!name) return alert(currentLang === 'vi' ? 'Vui lòng nhập họ tên người sử dụng' : 'Please enter patient name');
            const newPatient = {
                id: 'pat-' + Date.now(),
                name: name,
                code: document.getElementById('input-new-code').value.trim() || 'SG-02',
                age: document.getElementById('input-new-age').value.trim() || '',
                room: document.getElementById('input-new-room').value.trim() || 'Tại nhà',
                diag: document.getElementById('input-new-diag').value.trim() || 'Hỗ trợ giao tiếp vận động'
            };
            savedProfiles.push(newPatient);
            localStorage.setItem('smartglass_profiles', JSON.stringify(savedProfiles));
            activatePatient(newPatient);
        }

        function confirmSelectUser() {
            const p = savedProfiles.find(item => item.id === selectedPatientIdInModal) || savedProfiles[0];
            if (p) activatePatient(p);
        }

        function activatePatient(p) {
            currentPatient = p;
            localStorage.setItem('smartglass_active_id', p.id);
            applyCurrentPatientToUI(p);
            closeModal();
            applyFilters();
        }

        function closeModal() { if (currentPatient) document.getElementById('user-modal').style.display = 'none'; }

        function applyCurrentPatientToUI(p) {
            setText('patient-display-name', p.name);
            setText('patient-location', p.room || 'Phòng Chăm Sóc');
            setText('patient-diag', p.diag || 'Hỗ trợ giao tiếp');
            const initials = p.name.trim().split(/\s+/).map(w => w[0]).slice(-2).join('').toUpperCase() || 'SG';
            setText('patient-initials', initials);
        }

        // ================= PHÂN LOẠI 6 NHÓM AAC KHỚP 100% CẢ 36 CÂU TỪ web_ui.h =================
        let allLogs = [];
        let lastLatestLogKey = '';
        let lastLogsVersion = -1;
        let lastChartTimestamp = 0;
        let isDemoMode = false;
        let activeAlertType = null;
        let isRestBreakActive = false;
        let restBreakSecondsLeft = 30 * 60;
        let restTimerInterval = null;
        let userDismissedLockUntil = 0;
        let latestTelemetryState = { relay1: false, relay2: false, isControlLocked: false, inputMode: 'HYBRID' };

        function identifyCategory(sentence, serverCategory) {
            if (serverCategory && ['pain', 'food', 'hygiene', 'care', 'iot', 'feel'].includes(serverCategory)) {
                return serverCategory;
            }
            const s = String(sentence || '').toUpperCase();
            if (s.includes('SOS') || s.includes('CẤP CỨU') || s.includes('CO GIẬT') || s.includes('TÉ NGÃ') ||
                s.includes('ĐAU') || s.includes('NHỨC') || s.includes('KHÓ THỞ') || s.includes('VẾT MỔ') ||
                s.includes('TÊ MỎI') || s.includes('TAY CHÂN') || s.includes('CHÓNG MẶT') ||
                s.includes('BUỒN NÔN') || s.includes('NGỨA') || s.includes('PAIN')) {
                return 'pain';
            }
            if (s.includes('NƯỚC') || s.includes('UỐNG') || s.includes('ĂN') || s.includes('CƠM') ||
                s.includes('CHÁO') || s.includes('SỮA') || s.includes('THUỐC') || s.includes('TRÁI CÂY') ||
                s.includes('KHÁT') || s.includes('ĐÓI')) {
                return 'food';
            }
            if (s.includes('CHỐNG LOÉT') || s.includes('120 PHÚT') || s.includes('VỆ SINH') ||
                s.includes('NGỒI') || s.includes('NẰM') || s.includes('NGHIÊNG') || s.includes('LẬT') ||
                s.includes('TRỞ MÌNH') || s.includes('CHĂN') || s.includes('GỐI') || s.includes('GIƯỜNG') ||
                s.includes('THAY ÁO') || s.includes('LAU NGƯỜI') || s.includes('QUẦN ÁO') || s.includes('TƯ THẾ')) {
                return 'hygiene';
            }
            if (s.includes('BÁC SĨ') || s.includes('Y TÁ') || s.includes('ĐIỀU DƯỠNG') || s.includes('NGƯỜI NHÀ') ||
                s.includes('GIA ĐÌNH') || s.includes('HUYẾT ÁP') || s.includes('HÔ HẤP') || s.includes('HÚT ĐỜM') ||
                s.includes('XOA BÓP') || s.includes('GỌI')) {
                return 'care';
            }
            if (s.includes('ĐÈN') || s.includes('QUẠT') || s.includes('RƠ-LE') || s.includes('NÓNG') ||
                s.includes('LẠNH') || s.includes('ĐỌC BÁO') || s.includes('ĐỌC SÁCH') || s.includes('KHÓA') ||
                s.includes('BẬT') || s.includes('TẮT')) {
                return 'iot';
            }
            return 'feel';
        }

        function getTagHtml(cat, sentence) {
            const s = String(sentence || '').toUpperCase();
            if (s.includes('SOS') || s.includes('CO GIẬT') || s.includes('TÉ NGÃ')) {
                return `<span class="tag tag-sos">${currentLang === 'vi' ? 'KHẨN CẤP' : 'EMERGENCY'}</span>`;
            }
            if (s.includes('120 PHÚT') || s.includes('CHỐNG LOÉT')) {
                return `<span class="tag tag-warn">${currentLang === 'vi' ? 'CHỐNG LOÉT 120P' : 'BEDSORE 120M'}</span>`;
            }
            const t = i18n[currentLang];
            const map = {
                pain: { label: t.dist_pain, cls: 'tag-sos' },
                food: { label: t.dist_food, cls: 'tag-cyan' },
                hygiene: { label: t.dist_hygiene, cls: 'tag-green' },
                care: { label: t.dist_care, cls: 'tag-warn' },
                iot: { label: t.dist_iot, cls: '' },
                feel: { label: t.dist_feel, cls: '' }
            };
            const item = map[cat] || map.feel;
            return `<span class="tag ${item.cls}">${item.label}</span>`;
        }

        function speakText(text) {
            if ('speechSynthesis' in window && text) {
                window.speechSynthesis.cancel();
                const msg = new SpeechSynthesisUtterance(text);
                msg.lang = currentLang === 'vi' ? 'vi-VN' : 'en-US';
                msg.rate = 0.96;
                window.speechSynthesis.speak(msg);
            }
        }

        function formatTime(isoString) {
            const d = new Date(isoString);
            if (isNaN(d.getTime())) return { hm: '--:--:--', date: '--/--/----' };
            return {
                hm: d.toLocaleTimeString(currentLang === 'vi' ? 'vi-VN' : 'en-US', { hour: '2-digit', minute: '2-digit', second: '2-digit' }),
                date: d.toLocaleDateString(currentLang === 'vi' ? 'vi-VN' : 'en-US')
            };
        }

        function updateCharts(logs) {
            const total = logs.length;
            const counts = { pain: 0, food: 0, hygiene: 0, care: 0, iot: 0, feel: 0 };
            const hours = [0, 0, 0, 0, 0, 0];

            logs.forEach(l => {
                const cat = identifyCategory(l.sentence, l.category);
                if (counts[cat] !== undefined) counts[cat]++;
                const d = new Date(l.timestamp);
                if (!isNaN(d.getTime())) hours[Math.floor(d.getHours() / 4)]++;
            });

            ['pain', 'food', 'hygiene', 'care', 'iot', 'feel'].forEach(c => {
                const pct = total ? Math.round((counts[c] / total) * 100) : 0;
                setText(`txt-${c}`, `${counts[c]} (${pct}%)`);
                const bar = document.getElementById(`bar-${c}`);
                if (bar) bar.style.width = pct + '%';
            });

            const maxH = Math.max(...hours) || 1;
            const peakIdx = hours.indexOf(maxH);
            hours.forEach((v, i) => {
                const el = document.getElementById(`h-${i * 4}`);
                if (el) {
                    el.style.height = Math.max(5, (v / maxH) * 100) + '%';
                    v > 0 && i === peakIdx ? el.classList.add('active') : el.classList.remove('active');
                }
            });
            const t = i18n[currentLang];
            setText('peak-text', hours[peakIdx] > 0
                ? `${t.peak_prefix}${peakIdx * 4}h - ${(peakIdx + 1) * 4}h`
                : `${t.peak_prefix}${t.peak_none}`);
        }

        function applyFilters() {
            const catFilter = document.getElementById('category-filter')?.value || 'all';
            const timeFilter = document.getElementById('time-filter')?.value || 'all';
            const search = (document.getElementById('search-input')?.value || '').toLowerCase();
            const now = new Date();

            const filtered = allLogs.filter(l => {
                const sentence = String(l.sentence || '');
                const cat = identifyCategory(sentence, l.category);
                const matchCat = catFilter === 'all' || cat === catFilter;
                const matchSearch = !search || sentence.toLowerCase().includes(search);
                const logTime = new Date(l.timestamp);
                let matchTime = true;
                if (!isNaN(logTime.getTime())) {
                    if (timeFilter === 'day') matchTime = logTime.toDateString() === now.toDateString();
                    else if (timeFilter === 'week') matchTime = Math.abs(now - logTime) <= (7 * 24 * 60 * 60 * 1000);
                    else if (timeFilter === 'month') matchTime = logTime.getMonth() === now.getMonth() && logTime.getFullYear() === now.getFullYear();
                }
                return matchCat && matchSearch && matchTime;
            });

            setText('count-badge', filtered.length + (currentLang === 'vi' ? ' MỤC' : ' ITEMS'));
            updateCharts(filtered);

            const t = i18n[currentLang];
            const tbody = document.getElementById('logs-body');
            if (!tbody) return;

            if (!filtered.length) {
                tbody.innerHTML = `<tr><td colspan="6" style="text-align:center; padding: 50px; color: var(--text-dim); font-size:13px;">${escapeHtml(t.waiting_logs)}</td></tr>`;
                return;
            }

            tbody.innerHTML = filtered.map((l, idx) => {
                const timeObj = formatTime(l.timestamp);
                const upper = String(l.sentence || '').toUpperCase();
                let rowCls = '';
                if (l.severity === 'emergency' || upper.includes('SOS') || upper.includes('CO GIẬT') || upper.includes('TÉ NGÃ')) rowCls = 'row-emergency';
                else if (l.severity === 'warning' || upper.includes('120 PHÚT') || upper.includes('CHỐNG LOÉT')) rowCls = 'row-warning';

                return `<tr class="${rowCls}">
                    <td><div class="time-hm">${timeObj.hm}</div><div class="time-date">${timeObj.date}</div></td>
                    <td class="col-sentence">${escapeHtml(l.sentence)}</td>
                    <td>${getTagHtml(identifyCategory(l.sentence, l.category), l.sentence)}</td>
                    <td align="center" style="color:var(--cyan); font-weight:800;">${l.attention || 0}%</td>
                    <td align="center" style="color:var(--emerald); font-weight:800;">${l.meditation || 95}%</td>
                    <td align="center"><button class="btn-speak" data-idx="${idx}" onclick="speakFilteredLog(${idx})">${t.btn_speak}</button></td>
                </tr>`;
            }).join('');

            window._lastFilteredLogs = filtered;
        }

        function speakFilteredLog(idx) {
            const item = window._lastFilteredLogs && window._lastFilteredLogs[idx];
            if (item && item.sentence) speakText(item.sentence);
        }

        // ================= GỬI LỆNH ĐIỀU KHIỂN TỪ XA XUỐNG CLOUD -> KÍNH S3 -> GATEWAY C3 =================
        async function syncRestCommandToCloud(action, durationSec = 1800) {
            try {
                const res = await fetch(CLOUD_COMMAND_URL, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ command: action, duration: durationSec, timestamp: Date.now() })
                });
                return res.ok;
            } catch (err) {
                console.warn('[CLOUD CMD] Lỗi gửi lệnh:', err);
                return false;
            }
        }

        async function sendRemoteCommand(cmd) {
            setText('rc-feedback', currentLang === 'vi' ? `Đang gửi lệnh ${cmd} xuống kính...` : `Sending ${cmd} to glasses...`);
            const ok = await syncRestCommandToCloud(cmd, 0);
            if (ok) {
                setText('rc-feedback', currentLang === 'vi'
                    ? `✅ Đã xếp lệnh ${cmd} (đồng bộ xuống kính trong ~3s)`
                    : `✅ Queued ${cmd} (syncing to glasses in ~3s)`);
            } else {
                setText('rc-feedback', currentLang === 'vi' ? '❌ Lỗi gửi lệnh lên Cloud' : '❌ Failed to send command');
            }
        }

        function toggleRemoteRelay(ch) {
            const cur = (ch === 1) ? latestTelemetryState.relay1 : latestTelemetryState.relay2;
            const cmd = (ch === 1) ? (cur ? 'R1_OFF' : 'R1_ON') : (cur ? 'R2_OFF' : 'R2_ON');
            sendRemoteCommand(cmd);
        }

        function toggleRemoteControlLock() {
            sendRemoteCommand(latestTelemetryState.isControlLocked ? 'UNLOCK_CONTROL' : 'LOCK_CONTROL');
        }

        // ================= HỆ THỐNG BÁO ĐỘNG 4 KỊCH BẢN =================
        function startRestCountdown(secondsLeft) {
            if (restTimerInterval) clearInterval(restTimerInterval);
            isRestBreakActive = true;
            restBreakSecondsLeft = Math.max(1, secondsLeft);
            const btn = document.getElementById('btn-rest-toggle');
            if (btn) {
                btn.style.display = 'inline-block';
                btn.style.background = 'var(--amber)';
                btn.style.color = '#0f172a';
            }

            const renderTick = () => {
                const m = Math.floor(restBreakSecondsLeft / 60);
                const s = restBreakSecondsLeft % 60;
                setText('btn-rest-toggle', (currentLang === 'vi' ? 'ĐANG NGHỈ: ' : 'RESTING: ') + `${m}:${s < 10 ? '0' : ''}${s}`);
            };
            renderTick();

            restTimerInterval = setInterval(() => {
                restBreakSecondsLeft--;
                if (restBreakSecondsLeft <= 0) {
                    clearInterval(restTimerInterval);
                    restTimerInterval = null;
                    isRestBreakActive = false;
                    dismissClinicalAlert();
                    return;
                }
                renderTick();
            }, 1000);
        }

        function showClinicalAlert(type, customMsg = '', sendLockToESP = false, durationSec = 1800) {
            const banner = document.getElementById('clinical-alert-banner');
            const restBtn = document.getElementById('btn-rest-toggle');
            const irBtn = document.getElementById('btn-banner-ir');
            if (!banner) return;

            activeAlertType = type;
            banner.style.display = 'flex';
            banner.classList.remove('warn-mode');
            if (irBtn) irBtn.style.display = 'none';

            if (type === 'SOS') {
                const msg = customMsg || (currentLang === 'vi'
                    ? 'Người bệnh vừa kích hoạt tín hiệu cấp cứu khẩn cấp SOS từ Kính thông minh V4 Hybrid!'
                    : 'Emergency SOS triggered from Smart Glasses V4 Hybrid!');
                setText('txt-alarm-tag', currentLang === 'vi' ? '🚨 BÁO ĐỘNG ĐỎ: NGƯỜI BỆNH GỌI CẤP CỨU (SOS)' : '🚨 RED ALERT: PATIENT SOS CALL');
                setText('clinical-banner-msg', msg);
                setText('txt-alarm-nurse', currentLang === 'vi'
                    ? 'Phản hồi tại chỗ: Còi Buzzer SMD 5020 đã hú 3 hồi dài • Đề nghị người thân kiểm tra ngay lập tức.'
                    : 'Local buzzer activated (3 long beeps) • Immediate caregiver check required.');
                if (restBtn) restBtn.style.display = 'none';
                playAlarmSiren('HIGH');
                pushBrowserNotification('🚨 SOS KHẨN CẤP', msg);
                speakText(currentLang === 'vi' ? 'Cảnh báo khẩn cấp! Người bệnh đang gọi cấp cứu S O S!' : 'Emergency! Patient triggered SOS alert!');
            } else if (type === 'SPASM') {
                const msg = customMsg || (currentLang === 'vi'
                    ? 'Cảm biến LSM6DS3 ghi nhận gia tốc bất thường vượt ngưỡng 2,5g và vận tốc góc ≥ 200°/s!'
                    : 'LSM6DS3 detected abnormal acceleration ≥ 2.5g and angular velocity ≥ 200°/s!');
                setText('txt-alarm-tag', currentLang === 'vi' ? '🚨 CẢNH BÁO NGUY HIỂM: RUNG CHẤN CO GIẬT / TÉ NGÃ (≥ 2,5g)' : '🚨 DANGER: SEIZURE / FALL MOTION (≥ 2.5g)');
                setText('clinical-banner-msg', msg);
                setText('txt-alarm-nurse', currentLang === 'vi'
                    ? 'Lưu ý an toàn: Đây là tín hiệu cảnh báo sớm cần kiểm tra ngay, không thay thế chẩn đoán y khoa.'
                    : 'Safety note: Early warning requiring immediate physical check, not a medical diagnosis.');
                if (restBtn) restBtn.style.display = 'none';
                playAlarmSiren('HIGH');
                pushBrowserNotification('🚨 CẢNH BÁO CO GIẬT / TÉ NGÃ', msg);
                speakText(currentLang === 'vi' ? 'Cảnh báo nguy hiểm! Phát hiện rung chấn bất thường nghi co giật hoặc té ngã!' : 'Warning! Abnormal motion detected, possible seizure or fall!');
            } else if (type === 'BEDSORE') {
                banner.classList.add('warn-mode');
                const msg = customMsg || (currentLang === 'vi'
                    ? 'Người bệnh đã nằm nguyên một tư thế quá 120 phút. Cần hỗ trợ lật trở mình chống loét tì đè.'
                    : 'Patient remained static for over 120 minutes. Please assist with repositioning.');
                setText('txt-alarm-tag', currentLang === 'vi' ? '⏰ NHẮC CHĂM SÓC: CHỐNG LOÉT TÌ ĐÈ (120 PHÚT)' : '⏰ CARE REMINDER: REPOSITIONING (120 MIN)');
                setText('clinical-banner-msg', msg);
                setText('txt-alarm-nurse', currentLang === 'vi'
                    ? 'Bộ đếm tư thế LSM6DS3 sẽ tự đặt lại khi góc đầu/thân thay đổi trên 8,5°.'
                    : 'LSM6DS3 posture timer resets automatically once head orientation shifts over 8.5°.');
                if (restBtn) restBtn.style.display = 'none';
                playAlarmSiren('LOW');
                pushBrowserNotification('⏰ NHẮC CHỐNG LOÉT 120 PHÚT', msg);
                speakText(currentLang === 'vi' ? 'Nhắc nhở chăm sóc: Bệnh nhân đã nằm bất động 120 phút, cần hỗ trợ lật trở mình.' : 'Care reminder: 120 minutes static posture, assist repositioning.');
            } else {
                banner.classList.add('warn-mode');
                const msg = customMsg || (currentLang === 'vi'
                    ? 'Chỉ số mỏi cơ cổ CFI vượt ngưỡng 75%. Gợi ý chuyển sang chế độ “Chỉ dùng Mắt (IR)” hoặc tạm nghỉ 30 phút.'
                    : 'Cervical Fatigue Index reached 75%. Suggest switching to IR-Only mode or taking a 30-min rest.');
                setText('txt-alarm-tag', currentLang === 'vi' ? '⚠️ CẢNH BÁO MỎI CƠ CỔ (CFI ≥ 75%)' : '⚠️ NECK MUSCLE FATIGUE ALERT (CFI ≥ 75%)');
                setText('clinical-banner-msg', msg);
                setText('txt-alarm-nurse', currentLang === 'vi'
                    ? 'Có thể bấm nút “CHUYỂN CHỈ MẮT (IR)” ngay bên phải để tắt cảm biến gật đầu từ xa trên kính ESP32-S3.'
                    : 'Click “SWITCH TO IR-ONLY” to disable head-nod input remotely on ESP32-S3.');
                if (restBtn) restBtn.style.display = 'inline-block';
                if (irBtn) irBtn.style.display = 'inline-block';
                playAlarmSiren('LOW');
                pushBrowserNotification('⚠️ CẢNH BÁO MỎI CƠ CỔ (CFI ≥ 75%)', msg);
                speakText(currentLang === 'vi' ? 'Cảnh báo mỏi cơ cổ. Gợi ý chuyển sang chế độ chỉ dùng mắt hoặc nghỉ ngơi.' : 'Neck fatigue alert. Suggest switching to eye-only mode.');
                if (sendLockToESP) {
                    syncRestCommandToCloud('LOCK_REST', durationSec);
                    startRestCountdown(durationSec);
                }
            }
        }

        function triggerTestScenario() {
            const sel = document.getElementById('test-alert-select')?.value || 'CFI';
            if (sel === 'CFI') showClinicalAlert('CFI', '', true, 1800);
            else showClinicalAlert(sel, '', false);
        }

        function dismissClinicalAlert() {
            const banner = document.getElementById('clinical-alert-banner');
            if (banner) banner.style.display = 'none';
            userDismissedLockUntil = Date.now() + 30000;
            activeAlertType = null;
            isRestBreakActive = false;
            if (restTimerInterval) { clearInterval(restTimerInterval); restTimerInterval = null; }
            const restBtn = document.getElementById('btn-rest-toggle');
            if (restBtn) {
                restBtn.style.background = 'var(--danger-solid)';
                restBtn.style.color = '#fff';
                restBtn.innerText = i18n[currentLang].btn_rest;
            }
            syncRestCommandToCloud('UNLOCK_REST', 0);
        }

        function toggleRestBreak() {
            if (isRestBreakActive) dismissClinicalAlert();
            else {
                syncRestCommandToCloud('LOCK_REST', 1800);
                startRestCountdown(1800);
            }
        }

        function toggleDemo() {
            isDemoMode = !isDemoMode;
            const btn = document.getElementById('btn-demo');
            if (btn) btn.classList.toggle('active', isDemoMode);
            setText('btn-demo', isDemoMode ? i18n[currentLang].demo_on : i18n[currentLang].demo_off);

            if (isDemoMode) {
                runDemoTick();
            } else {
                sensorChart.data.datasets[0].data = Array(50).fill(0);
                sensorChart.data.datasets[1].data = Array(50).fill(0);
                sensorChart.update('none');
                radarChart.data.datasets[0].data = [0, 0, 0, 0, 0];
                radarChart.update('none');
                lastChartTimestamp = 0;
                updateUI({ signalQuality: 200, attention: 0, meditation: 0, cfi: 12, pitch: 0, roll: 0, temperature: 34.5, battery: 0, voltage: 0, current: 0, staticMinutes: 0 });
            }
        }

        // ================= CẬP NHẬT GIAO DIỆN TỪ TELEMETRY THẬT CỦA ESP32-S3 =================
        function updateUI(tData) {
            const t = i18n[currentLang];
            if (!isDemoMode && tData.lastUpdate) lastServerTelemetryAt = Number(tData.lastUpdate);

            const sig = isDemoMode ? 0 : (tData.signalQuality !== undefined ? Number(tData.signalQuality) : 200);
            const isRecent = isDemoMode || (tData.lastUpdate ? (Date.now() - Number(tData.lastUpdate) < 20000) : false);
            const hasSignal = isDemoMode || (isRecent && sig === 0);

            const force = hasSignal ? Math.min(100, Math.max(0, Number(tData.attention || 0))) : 0;
            const stability = hasSignal ? Math.min(100, Math.max(0, Number(tData.meditation !== undefined ? tData.meditation : 95))) : 0;
            const cfi = tData.cfi !== undefined ? Math.min(100, Math.max(0, Math.round(Number(tData.cfi)))) : 12;
            const pitch = tData.pitch !== undefined ? Number(tData.pitch) : 0;
            const roll = tData.roll !== undefined ? Number(tData.roll) : 0;
            const tempC = tData.temperature !== undefined ? Number(tData.temperature) : 34.5;
            const angleDeg = tData.delta ? (Number(tData.delta) / 100.0) : Math.abs(pitch);
            const irDeltaVal = tData.irDelta !== undefined ? Number(tData.irDelta) : 0;
            const staticMin = tData.staticMinutes !== undefined ? Math.max(0, Math.round(Number(tData.staticMinutes))) : 0;

            const bat = tData.battery !== undefined ? Math.min(100, Math.max(0, Math.round(Number(tData.battery)))) : (isDemoMode ? 88 : 0);
            const voltNum = tData.voltage !== undefined ? Number(tData.voltage) : (isDemoMode ? 3.94 : 0);
            const volt = voltNum.toFixed(2);
            const pwr = tData.power !== undefined ? Math.round(Number(tData.power)) : Math.round(voltNum * Number(tData.current || 0));
            const isUsb5v = tData.powerSource === 'USB_5V' || voltNum >= 4.6;

            latestTelemetryState = {
                relay1: Boolean(tData.relay1),
                relay2: Boolean(tData.relay2),
                isControlLocked: Boolean(tData.isControlLocked),
                inputMode: tData.inputMode || 'HYBRID'
            };

            // 1. Huy hiệu Top Nav: Kính Online, Mạng Kính (Router/Local AP), Gateway C3
            const connDot = document.getElementById('conn-dot');
            if (connDot) connDot.className = isRecent ? 'dot dot-green' : 'dot dot-red';
            setText('conn-text', isDemoMode ? t.conn_demo : (isRecent ? t.conn_online : t.conn_offline));

            const netMode = tData.netMode || (isRecent ? 'ROUTER' : '--');
            const netDot = document.getElementById('net-dot');
            if (netDot) netDot.className = (netMode === 'ROUTER') ? 'dot dot-green' : ((netMode === 'LOCAL_AP') ? 'dot dot-yellow' : 'dot dot-red');
            setText('net-mode-text', (currentLang === 'vi' ? 'MẠNG: ' : 'NET: ') + netMode);

            const gwOnline = Boolean(tData.gatewayOnline) && isRecent;
            const gwDot = document.getElementById('gw-dot');
            if (gwDot) gwDot.className = gwOnline ? 'dot dot-green' : 'dot dot-red';
            setText('gw-status-text', gwOnline ? 'GW: ONLINE' : 'GW: OFFLINE');

            // Chip trạng thái bệnh nhân
            const stateChip = document.getElementById('patient-state-badge');
            if (stateChip) {
                if (tData.isSleeping) {
                    stateChip.innerText = currentLang === 'vi' ? 'CHẾ ĐỘ NGỦ' : 'SLEEP MODE';
                    stateChip.style.color = 'var(--indigo)';
                    stateChip.style.borderColor = 'var(--indigo)';
                    stateChip.style.background = 'rgba(129, 140, 248, 0.16)';
                } else if (tData.isControlLocked || tData.isFatigueLocked) {
                    stateChip.innerText = currentLang === 'vi' ? 'ĐANG TẠM KHÓA' : 'LOCKED';
                    stateChip.style.color = 'var(--rose)';
                    stateChip.style.borderColor = 'var(--rose)';
                    stateChip.style.background = 'rgba(251, 113, 133, 0.16)';
                } else {
                    stateChip.innerText = currentLang === 'vi' ? 'ĐANG MỞ' : 'ACTIVE';
                    stateChip.style.color = 'var(--emerald)';
                    stateChip.style.borderColor = 'var(--emerald)';
                    stateChip.style.background = 'rgba(52, 211, 153, 0.16)';
                }
            }

            // 2. Hàng 1: Lực phản hồi, Độ mỏi cổ CFI, Nguồn USB 5V / Pin Li-Po
            setText('val-force', `${force}%`);
            const barForce = document.getElementById('bar-force');
            if (barForce) barForce.style.width = `${force}%`;
            setText('force-status', force >= 75 ? (currentLang === 'vi' ? 'GẬT / NHÁY MẠNH' : 'STRONG') : (force > 0 ? (currentLang === 'vi' ? 'ĐÃ NHẬN LỆNH' : 'TRIGGERED') : (currentLang === 'vi' ? 'SẴN SÀNG' : 'READY')));

            setText('val-cfi', `${cfi}%`);
            const valCfiEl = document.getElementById('val-cfi');
            const barCfiEl = document.getElementById('bar-cfi');
            const stCfiEl = document.getElementById('cfi-status');
            if (barCfiEl) barCfiEl.style.width = `${cfi}%`;

            if (cfi >= 75) {
                if (valCfiEl) valCfiEl.style.color = 'var(--rose)';
                if (barCfiEl) barCfiEl.style.background = 'var(--rose)';
                if (stCfiEl) { stCfiEl.innerText = currentLang === 'vi' ? 'MỎI CAO • DÙNG MẮT' : 'HIGH • USE IR'; stCfiEl.style.color = 'var(--rose)'; }
            } else if (cfi >= 50) {
                if (valCfiEl) valCfiEl.style.color = 'var(--amber)';
                if (barCfiEl) barCfiEl.style.background = 'var(--amber)';
                if (stCfiEl) { stCfiEl.innerText = currentLang === 'vi' ? 'CẬN NGƯỠNG' : 'MODERATE'; stCfiEl.style.color = 'var(--amber)'; }
            } else {
                if (valCfiEl) valCfiEl.style.color = 'var(--emerald)';
                if (barCfiEl) barCfiEl.style.background = 'var(--emerald)';
                if (stCfiEl) { stCfiEl.innerText = currentLang === 'vi' ? 'TỐI ƯU' : 'OPTIMAL'; stCfiEl.style.color = 'var(--emerald)'; }
            }

            // Tự động phân biệt Nguồn USB 5V vs Pin Li-Po
            setText('lbl-pwr-title', isUsb5v ? t.lbl_pwr_usb : t.lbl_pwr_lipo);
            setText('hint-pwr', isUsb5v ? t.hint_pwr_usb : t.hint_pwr_lipo);
            setText('val-bat', isUsb5v ? 'USB 5V' : `${bat}%`);
            setText('val-volt', `${volt}V`);
            setText('val-pwr', `${pwr} mW`);
            const barBat = document.getElementById('bar-bat');
            if (barBat) {
                barBat.style.width = isUsb5v ? '100%' : `${bat}%`;
                barBat.style.background = isUsb5v ? 'var(--cyan)' : (bat < 20 ? 'var(--rose)' : (bat < 45 ? 'var(--amber)' : 'var(--emerald)'));
            }
            setText('pwr-status', isUsb5v ? (currentLang === 'vi' ? 'NGUỒN TRỰC TIẾP' : 'USB DIRECT') : (bat < 20 ? (currentLang === 'vi' ? 'PIN YẾU' : 'LOW') : (currentLang === 'vi' ? 'ỔN ĐỊNH' : 'GOOD')));

            // 3. Hàng 2: 4 khối Hệ Chuyên Gia Y Sinh
            const inputModeMap = {
                HYBRID: currentLang === 'vi' ? 'Kính IR + IMU' : 'Hybrid IR + IMU',
                IR_ONLY: currentLang === 'vi' ? 'Chỉ dùng Mắt (IR)' : 'IR Eye Only',
                NOD_ONLY: currentLang === 'vi' ? 'Chỉ Gật Đầu (IMU)' : 'Head IMU Only'
            };
            const nodModeMap = {
                LIGHT: currentLang === 'vi' ? 'Độ nhạy: Nhẹ' : 'Sens: Light',
                HEAVY: currentLang === 'vi' ? 'Độ nhạy: Mạnh' : 'Sens: Heavy',
                DUAL: currentLang === 'vi' ? 'Độ nhạy: Kép' : 'Sens: Dual'
            };
            setText('bio-sensor-val', inputModeMap[tData.inputMode] || inputModeMap.HYBRID);
            setText('bio-sensor-sub', nodModeMap[tData.nodMode] || nodModeMap.DUAL);
            setText('bio-sensor-badge', hasSignal ? (currentLang === 'vi' ? 'SẴN SÀNG' : 'READY') : (currentLang === 'vi' ? 'CHỜ TÍN HIỆU' : 'WAITING'));
            const bioSensorBadge = document.getElementById('bio-sensor-badge');
            if (bioSensorBadge) bioSensorBadge.style.color = hasSignal ? 'var(--emerald)' : 'var(--amber)';
            const barBioSensor = document.getElementById('bar-bio-sensor');
            if (barBioSensor) {
                barBioSensor.style.width = hasSignal ? '100%' : '20%';
                barBioSensor.style.background = hasSignal ? 'var(--emerald)' : 'var(--amber)';
            }

            // Tư thế đầu 6-DOF
            setText('bio-pose-val', `${pitch.toFixed(1)}° / ${roll.toFixed(1)}°`);
            const tiltMag = Math.min(100, Math.round((Math.max(Math.abs(pitch), Math.abs(roll)) / 30) * 100));
            const barPose = document.getElementById('bar-bio-pose');
            if (barPose) barPose.style.width = `${Math.max(8, tiltMag)}%`;
            const poseBadge = document.getElementById('bio-pose-badge');
            if (poseBadge) {
                if (Math.abs(pitch) > 12) {
                    poseBadge.innerText = currentLang === 'vi' ? 'GẬT CHỌN (>12°)' : 'NOD (>12°)';
                    poseBadge.style.color = 'var(--amber)';
                } else if (Math.abs(roll) > 10) {
                    poseBadge.innerText = currentLang === 'vi' ? 'NGHIÊNG ĐẦU' : 'TILTED';
                    poseBadge.style.color = 'var(--cyan)';
                } else {
                    poseBadge.innerText = currentLang === 'vi' ? 'CÂN BẰNG' : 'NEUTRAL';
                    poseBadge.style.color = 'var(--emerald)';
                }
            }

            // Tiến trình chống loét thật từ LSM6DS3 (staticMinutes / 120 phút)
            const bedsorePct = Math.min(100, Math.round((staticMin / 120) * 100));
            setText('bio-safety-val', `${staticMin} / 120p`);
            setText('bio-safety-sub', tData.isSleeping
                ? (currentLang === 'vi' ? 'Đang Ngủ' : 'Sleeping')
                : (currentLang === 'vi' ? 'Đang Thức' : 'Awake'));
            const barSafety = document.getElementById('bar-bio-safety');
            if (barSafety) {
                barSafety.style.width = `${Math.max(4, bedsorePct)}%`;
                barSafety.style.background = staticMin >= 110 ? 'var(--rose)' : (staticMin >= 80 ? 'var(--amber)' : 'var(--emerald)');
            }
            const safetyBadge = document.getElementById('bio-safety-badge');
            if (safetyBadge) {
                if (staticMin >= 120) {
                    safetyBadge.innerText = currentLang === 'vi' ? 'CẦN LẬT NGƯỜI' : 'TURN NEEDED';
                    safetyBadge.style.color = 'var(--rose)';
                } else if (staticMin >= 90) {
                    safetyBadge.innerText = currentLang === 'vi' ? 'SẮP ĐẾN HẠN' : 'SOON';
                    safetyBadge.style.color = 'var(--amber)';
                } else {
                    safetyBadge.innerText = currentLang === 'vi' ? 'AN TOÀN' : 'SAFE';
                    safetyBadge.style.color = 'var(--emerald)';
                }
            }

            // Nhiệt độ LSM6DS3 & Trạng thái Gateway R1/R2
            setText('bio-env-val', `${tempC.toFixed(1)}°C`);
            const r1Txt = latestTelemetryState.relay1 ? (currentLang === 'vi' ? 'Bật' : 'ON') : (currentLang === 'vi' ? 'Tắt' : 'OFF');
            const r2Txt = latestTelemetryState.relay2 ? (currentLang === 'vi' ? 'Bật' : 'ON') : (currentLang === 'vi' ? 'Tắt' : 'OFF');
            setText('bio-env-sub', currentLang === 'vi' ? `Đèn: ${r1Txt} • Quạt: ${r2Txt}` : `Light: ${r1Txt} • Fan: ${r2Txt}`);
            const envBadge = document.getElementById('bio-env-badge');
            if (envBadge) {
                envBadge.innerText = gwOnline ? 'GW ONLINE' : 'GW OFFLINE';
                envBadge.style.color = gwOnline ? 'var(--emerald)' : 'var(--rose)';
            }

            // 4. Đồng bộ Thanh Điều Khiển Từ Xa
            document.getElementById('rc-mode-hybrid')?.classList.toggle('active', latestTelemetryState.inputMode === 'HYBRID');
            const irBtn = document.getElementById('rc-mode-ir');
            if (irBtn) {
                irBtn.classList.toggle('active', latestTelemetryState.inputMode === 'IR_ONLY');
                irBtn.classList.toggle('suggest-ir', cfi >= 75 && latestTelemetryState.inputMode !== 'IR_ONLY');
            }
            document.getElementById('rc-mode-nod')?.classList.toggle('active', latestTelemetryState.inputMode === 'NOD_ONLY');

            const r1Btn = document.getElementById('rc-relay-1');
            if (r1Btn) {
                r1Btn.classList.toggle('relay-on', latestTelemetryState.relay1);
                r1Btn.innerText = (currentLang === 'vi' ? '💡 Đèn (R1): ' : '💡 Light (R1): ') + r1Txt;
            }
            const r2Btn = document.getElementById('rc-relay-2');
            if (r2Btn) {
                r2Btn.classList.toggle('relay-on', latestTelemetryState.relay2);
                r2Btn.innerText = (currentLang === 'vi' ? '🌀 Quạt (R2): ' : '🌀 Fan (R2): ') + r2Txt;
            }
            const lockBtn = document.getElementById('rc-ctrl-lock');
            if (lockBtn) {
                lockBtn.classList.toggle('lock-on', latestTelemetryState.isControlLocked);
                lockBtn.innerText = latestTelemetryState.isControlLocked
                    ? (currentLang === 'vi' ? '🔒 Đang Khóa (Mở lại)' : '🔒 Locked (Unlock)')
                    : (currentLang === 'vi' ? '🔓 Khóa Tạm Nghỉ' : '🔓 Lock Control');
            }

            // 5. Banner khóa mỏi cổ tự động từ ESP32-S3
            if (tData.isFatigueLocked && Date.now() > userDismissedLockUntil) {
                const secLeft = parseInt(tData.fatigueSecLeft, 10) || 1800;
                if (activeAlertType !== 'CFI') showClinicalAlert('CFI', '', false, secLeft);
                if (!isRestBreakActive) startRestCountdown(secLeft);
            } else if (activeAlertType === 'CFI' && !isRestBreakActive && !tData.isFatigueLocked) {
                const banner = document.getElementById('clinical-alert-banner');
                if (banner) banner.style.display = 'none';
                activeAlertType = null;
            }

            // 6. Đồ thị 2 kênh — chỉ đẩy điểm mới khi nhận gói Telemetry mới (lastUpdate thay đổi)
            const overlay = document.getElementById('no-signal-overlay');
            if (overlay) overlay.style.display = hasSignal ? 'none' : 'flex';
            setText('signal-text', hasSignal ? t.sig_good : t.sig_lost);
            const sigTextEl = document.getElementById('signal-text');
            if (sigTextEl) sigTextEl.style.color = hasSignal ? 'var(--emerald)' : 'var(--amber)';

            const curTs = isDemoMode ? Date.now() : Number(tData.lastUpdate || 0);
            if (hasSignal && curTs !== lastChartTimestamp) {
                lastChartTimestamp = curTs;
                const dPitch = sensorChart.data.datasets[0].data;
                const dIr = sensorChart.data.datasets[1].data;
                dPitch.push(Number(pitch.toFixed(2)));
                dPitch.shift();
                dIr.push(Number(irDeltaVal.toFixed(1)));
                dIr.shift();
                sensorChart.update('none');
            }

            // 7. Radar 5 trục
            const neckHealth = Math.max(0, 100 - cfi);
            const angleScore = Math.min(100, Math.round((angleDeg / 18) * 100));
            const irScore = Math.min(100, Math.round((irDeltaVal / 100) * 100));

            setText('m-force', `${force}%`);
            setText('m-stability', `${stability}%`);
            setText('m-neck', `${neckHealth}%`);
            setText('m-angle', `${angleDeg.toFixed(1)}°`);
            setText('m-tkeo', `${irDeltaVal.toFixed(1)}`);

            radarChart.data.datasets[0].data = hasSignal
                ? [force, stability, neckHealth, angleScore, irScore]
                : [0, 0, 0, 0, 0];
            radarChart.update('none');
        }

        // ================= PHÁT HIỆN CẢNH BÁO MỚI THEO ID BẢN TIN (KHÔNG ĐỨNG HÌNH Ở 200/1000) =================
        function getLogKey(log) {
            if (!log) return '';
            return String(log._id || `${log.timestamp}|${log.sentence}`);
        }

        function checkIncomingEmergencyLogs(newLogs) {
            if (!Array.isArray(newLogs) || newLogs.length === 0) return;
            const newestKey = getLogKey(newLogs[0]);
            if (lastLatestLogKey && newestKey !== lastLatestLogKey) {
                const latest = newLogs[0];
                const text = String(latest?.sentence || '').toUpperCase();
                if (text.includes('CO GIẬT') || text.includes('TÉ NGÃ')) {
                    showClinicalAlert('SPASM', latest.sentence, false);
                } else if (text.includes('SOS') || text.includes('CẤP CỨU')) {
                    showClinicalAlert('SOS', latest.sentence, false);
                } else if (text.includes('120 PHÚT') || text.includes('CHỐNG LOÉT')) {
                    showClinicalAlert('BEDSORE', latest.sentence, false);
                }
            }
            lastLatestLogKey = newestKey;
        }

        // ================= MÔ PHỎNG TÍN HIỆU V4 HYBRID =================
        let demoPhase = 0;
        function runDemoTick() {
            if (!isDemoMode) return;
            demoPhase += 0.35;
            const blinkPulse = Math.random() > 0.80 ? (28 + Math.random() * 25) : (3 + Math.random() * 4);
            const nodWave = Math.sin(demoPhase) * 6 + (Math.random() > 0.86 ? 13 : 0);
            const simCfi = activeAlertType === 'CFI' ? 82 : Math.round(14 + Math.abs(Math.sin(demoPhase * 0.15)) * 18);

            updateUI({
                signalQuality: 0,
                attention: blinkPulse > 20 ? Math.round(62 + Math.random() * 28) : Math.round(12 + Math.random() * 15),
                meditation: 96,
                cfi: simCfi,
                pitch: nodWave,
                roll: Math.cos(demoPhase * 0.6) * 4.2,
                temperature: 34.4 + Math.sin(demoPhase * 0.1) * 0.3,
                battery: 88,
                voltage: 3.94,
                current: 112,
                power: 441,
                powerSource: 'LIPO',
                delta: Math.round(Math.abs(nodWave) * 100),
                theta: Math.round(blinkPulse * 3.2),
                irDelta: blinkPulse,
                inputMode: 'HYBRID',
                nodMode: 'DUAL',
                relay1: latestTelemetryState.relay1,
                relay2: latestTelemetryState.relay2,
                gatewayOnline: true,
                netMode: 'ROUTER',
                staticMinutes: Math.min(120, Math.floor(demoPhase * 2) % 120),
                uptime: Math.floor(performance.now() / 1000) + 900,
                isSleeping: false,
                isControlLocked: false
            });
        }

        // ================= XUẤT FILE CSV / JSON =================
        function exportCSV() {
            if (!allLogs.length) return alert(currentLang === 'vi' ? 'Chưa có dữ liệu để xuất' : 'No logs to export');
            const csv = 'Time,Sentence,Category,Severity,ForcePct,StabilityPct\n' + allLogs.map(l =>
                `"${l.timestamp}","${String(l.sentence || '').replace(/"/g, '""')}","${identifyCategory(l.sentence, l.category)}","${l.severity || 'normal'}",${l.attention || 0},${l.meditation || 95}`
            ).join('\n');
            const a = document.createElement('a');
            a.href = URL.createObjectURL(new Blob(['\uFEFF' + csv], { type: 'text/csv;charset=utf-8;' }));
            a.download = 'SmartGlass_V4_Logs.csv';
            a.click();
        }

        function exportJSON() {
            if (!allLogs.length) return alert(currentLang === 'vi' ? 'Chưa có dữ liệu để xuất' : 'No logs to export');
            const a = document.createElement('a');
            a.href = URL.createObjectURL(new Blob([JSON.stringify({ device: 'SmartGlass_V4_Hybrid_PRO-CAD_V8.2', patient: currentPatient, logs: allLogs }, null, 2)], { type: 'application/json' }));
            a.download = 'SmartGlass_V4_Logs.json';
            a.click();
        }

        // ================= POLLING DỮ LIỆU TỪ RENDER (RAM CACHE) =================
        setInterval(async () => {
            if (isDemoMode) {
                runDemoTick();
                return;
            }
            try {
                const res = await fetch(`${API_BASE}/api/data`);
                if (res.ok) {
                    const data = await res.json();
                    const cloudDot = document.getElementById('cloud-dot');
                    if (cloudDot) cloudDot.className = 'dot dot-green';
                    if (data.telemetry) updateUI(data.telemetry);
                    if (data.logs && Array.isArray(data.logs)) {
                        checkIncomingEmergencyLogs(data.logs);
                        const newestKey = data.logs.length > 0 ? getLogKey(data.logs[0]) : '';
                        const ver = data.logsVersion !== undefined ? data.logsVersion : -1;
                        if (ver !== lastLogsVersion || newestKey !== (allLogs[0] ? getLogKey(allLogs[0]) : '') || data.logs.length !== allLogs.length) {
                            lastLogsVersion = ver;
                            allLogs = data.logs;
                            applyFilters();
                        }
                    }
                }
            } catch (e) {
                const cloudDot = document.getElementById('cloud-dot');
                if (cloudDot) cloudDot.className = 'dot dot-yellow';
            }
        }, 1500);

        // ================= KHỞI CHẠY =================
        applyTheme(currentTheme);
        applyLanguage();
        initUserSystem();
    </script>
</body>

</html>