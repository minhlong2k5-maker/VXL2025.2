#ifndef WEBPAGE_H
#define WEBPAGE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8"><title>HUST Health Monitor</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { background:#121212; color:#fff; font-family:sans-serif; text-align:center; padding:10px; }
    .status-bar { background:#001a33; padding:10px; margin-bottom:10px; border-radius:8px; border-left:5px solid #ff8c00; font-size: 1.2em; box-shadow: 0 2px 10px rgba(0, 26, 51, 0.5); }
    .grid { display:grid; grid-template-columns: 1fr 1fr 1fr; gap:10px; margin-bottom:10px; }
    .card { background:#1e1e1e; padding:15px; border-radius:10px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); border-top: 2px solid #333; }
    .val { font-size:2.5em; font-weight:bold; color:#00ffcc; }
    .val.spo2 { color: #ff3366; }
    
    .chart-box { background:#000; border-radius:10px; padding:10px; height:250px; margin-bottom:15px; position:relative; border: 1px solid #222; }
    .no-signal { position:absolute; top:0; left:0; width:100%; height:100%; background:rgba(0,0,0,0.8); color:#ff3366; display:flex; align-items:center; justify-content:center; font-size:3em; font-weight:bold; z-index:10; border-radius:10px; }
    
    .btn-group { margin-bottom: 15px; }
    .btn { padding: 10px 20px; color: #fff; border: none; border-radius: 5px; font-size: 1em; font-weight: bold; cursor: pointer; margin: 0 5px; transition: 0.3s; }
    .btn-download { background: #555; }
    .btn-view { background: #00ffcc; color: #000; }
    .btn-cardio { background: #ff8c00; color: #001a33; box-shadow: 0 0 10px rgba(255, 140, 0, 0.3); }
    .btn-cardio:hover { background: #e67e00; }
    
    .btn-record { background: #1a0000; border: 2px solid #ff3366; color: #ff3366; }
    .btn-record.active { background: #ff3366; color: #fff; box-shadow: 0 0 15px rgba(255, 51, 102, 0.6); animation: pulse 1.5s infinite; }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.7; } 100% { opacity: 1; } }
    
    .btn-export { background: #00aaff; color: #001a33; border: none; padding: 12px 20px; border-radius: 8px; font-weight: bold; font-size: 1.1em; cursor: pointer; margin-top: 15px; width: 100%; transition: 0.3s; }
    .btn-export:hover { background: #0088cc; color: #fff; }

    table { width: 100%; border-collapse: collapse; margin-top: 15px; display: none; background: #1e1e1e; border-radius: 8px; overflow: hidden; }
    th, td { border: 1px solid #444; padding: 10px; text-align: center; }
    th { background-color: #001a33; color: #ff8c00; font-size: 1.1em; }
    tr:nth-child(even) { background-color: #2a2a2a; }
    
    .modal { display: none; position: fixed; z-index: 100; left: 0; top: 0; width: 100%; height: 100%; background-color: rgba(0,0,0,0.85); overflow-y: auto; }
    .modal-content { background-color: #1e1e1e; margin: 5% auto; padding: 25px; border: 2px solid #ff8c00; border-radius: 10px; width: 90%; max-width: 600px; text-align: left; box-shadow: 0 4px 20px rgba(255,140,0,0.3); }
    .close-btn { color: #aaa; float: right; font-size: 28px; font-weight: bold; cursor: pointer; }
    .close-btn:hover { color: #fff; }
    
    .medical-section { background: #111; padding: 15px; border-radius: 8px; margin-top: 15px; border-left: 4px solid #00ffcc; }
    .medical-title { color: #00ffcc; font-size: 1.1em; font-weight: bold; margin-bottom: 8px; border-bottom: 1px dashed #333; padding-bottom: 5px; }
    .medical-text { font-size: 0.95em; color: #ccc; line-height: 1.5; margin: 5px 0;}
  </style>
</head>
<body>
  <div class="status-bar">
    TRẠNG THÁI: <span id="msg" style="color:#ff8c00;">CONNECTING...</span> | THỜI GIAN: <span id="timeDisplay">--:--:--</span>
  </div>
  
  <div class="grid">
    <div class="card"><div>BPM (ECG)</div><div id="bE" class="val">--</div></div>
    <div class="card"><div>BPM (MAX)</div><div id="bM" class="val">--</div></div>
    <div class="card"><div>SpO2 (%)</div><div id="o2" class="val spo2">--</div></div>
  </div>

  <div class="btn-group">
    <button id="btnRecord" class="btn btn-record" onclick="toggleRecord()">🔴 BẮT ĐẦU GHI (OFF)</button>
    <button class="btn btn-cardio" onclick="evaluateCardio()">❤️ ĐÁNH GIÁ CHẤT LƯỢNG TIM MẠCH</button>
  </div>
  <div class="btn-group">
    <a href="/download"><button class="btn btn-download">📥 FULL SD (.CSV)</button></a>
    <button class="btn btn-view" onclick="loadTable()">📊 XEM BẢNG</button>
    <button class="btn" style="background: #333; border: 1px solid #ff3366; color: #ff3366;" onclick="resetTable()">🗑️ RESET SD</button>
  </div>

  <div class="chart-box"><canvas id="ecgChart"></canvas></div>
  <div class="chart-box">
    <div id="noSignalBox" class="no-signal">NO SIGNAL</div>
    <canvas id="ppgChart"></canvas>
  </div>
  <table id="dataTable"></table>

  <div id="cardioModal" class="modal">
    <div class="modal-content">
      <span class="close-btn" onclick="closeModal()">&times;</span>
      <h2 style="color: #ff8c00; border-bottom: 1px solid #444; padding-bottom: 10px; margin-top:0; text-align: center;">BÁO CÁO TỪ LẦN ĐO GẦN NHẤT</h2>
      <div id="cardioResult">Đang tải và xử lý dữ liệu...</div>
    </div>
  </div>

  <script>
    var ecgData = new Array(150).fill(2048);
    var timeLabels = new Array(150).fill("");
    var ctx = document.getElementById('ecgChart').getContext('2d');
    var chart = new Chart(ctx, { 
      type: 'line', 
      data: { labels: timeLabels, datasets: [{ borderColor: '#00ffcc', data: ecgData, pointRadius: 0, borderWidth: 2, tension: 0.4 }] }, 
      options: { animation: false, maintainAspectRatio: false, scales: { x: { display: false }, y: { min: 500, max: 3500, display: true, grid: { color: '#2a2a2a', lineWidth: 1 }, ticks: { display: true, color: '#888' } } }, plugins: { legend: { display: false } } } 
    });

    var ppgData = new Array(150).fill(0);
    var ctx2 = document.getElementById('ppgChart').getContext('2d');
    var chart2 = new Chart(ctx2, { 
      type: 'line', 
      data: { labels: timeLabels, datasets: [{ borderColor: '#ff3366', data: ppgData, pointRadius: 0, borderWidth: 2, tension: 0.4 }] }, 
      options: { animation: false, maintainAspectRatio: false, scales: { x: { display: false }, y: { min: -200, max: 200, display: true, grid: { color: '#2a2a2a', lineWidth: 1 }, ticks: { display: true, color: '#ffffff', font: { size: 12, weight: 'bold' } } } }, plugins: { legend: { display: false } } } 
    });
    
    var wsProtocol = (window.location.protocol === "https:") ? "wss://" : "ws://";
    var gateway = wsProtocol + window.location.hostname + "/ws";
    var websocket = new WebSocket(gateway);
    
    let generatedHtmlReport = ""; 
    let renderCounter = 0; // Biến đếm để giảm khung hình vẽ

    function toggleRecord() {
      if(websocket.readyState === WebSocket.OPEN) {
        websocket.send("TOGGLE_RECORD");
      }
    }
    
    websocket.onmessage = function(e) {
      var d = JSON.parse(e.data);
      document.getElementById('bE').innerText = d.bE > 0 ? d.bE : "--";
      document.getElementById('bM').innerText = d.bM > 0 ? d.bM : "--";
      document.getElementById('o2').innerText = d.o2 > 0 ? d.o2 : "--";
      document.getElementById('msg').innerText = d.st;
      document.getElementById('timeDisplay').innerText = d.t;

      let btn = document.getElementById('btnRecord');
      if (d.rec === 1) {
          if (!btn.classList.contains('active')) {
              btn.classList.add('active');
              btn.innerText = "⏹ ĐANG GHI DỮ LIỆU LẦN NÀY...";
          }
      } else {
          if (btn.classList.contains('active')) {
              btn.classList.remove('active');
              btn.innerText = "🔴 BẮT ĐẦU GHI (OFF)";
          }
      }

      if (d.st === "CHUA CO TAY") document.getElementById('noSignalBox').style.display = 'flex'; 
      else document.getElementById('noSignalBox').style.display = 'none';

      ecgData.push(d.v); ecgData.shift(); 
      ppgData.push(d.p); ppgData.shift();
      
      var parts = d.t.split(" ");
      if (parts.length === 2) { timeLabels.push([parts[0], parts[1]]); } else { timeLabels.push(d.t); }
      timeLabels.shift();
      
      chart.update(); chart2.update();
    };

    function loadTable() { fetch('/download').then(response => response.text()).then(data => { let rows = data.split('\n'); let tableHTML = ''; rows.forEach((row, index) => { if(row.trim() === '') return; let cols = row.split(','); tableHTML += '<tr>'; cols.forEach(col => { if(index === 0) tableHTML += '<th>' + col + '</th>'; else tableHTML += '<td>' + col + '</td>'; }); tableHTML += '</tr>'; }); document.getElementById('dataTable').innerHTML = tableHTML; document.getElementById('dataTable').style.display = 'table'; }).catch(err => alert("Chưa có dữ liệu!")); }
    function resetTable() { if (confirm("Xóa toàn bộ dữ liệu trên thẻ nhớ?")) { fetch('/reset').then(response => response.text()).then(msg => { alert(msg); document.getElementById('dataTable').innerHTML = ''; document.getElementById('dataTable').style.display = 'none'; }).catch(err => alert("Lỗi!")); } }

    function evaluateCardio() {
        document.getElementById('cardioModal').style.display = 'block';
        document.getElementById('cardioResult').innerHTML = "<div style='text-align:center; padding: 20px;'>Đang kiểm tra kết nối cảm biến và phân tích dữ liệu...</div>";
        
        fetch('/download').then(response => response.text()).then(data => {
            let lines = data.split('\n').filter(l => l.trim().length > 0);
            
            let lastSessionIdx = -1;
            for (let i = lines.length - 1; i >= 0; i--) {
                if (lines[i].includes("---NEW_SESSION---")) {
                    lastSessionIdx = i;
                    break;
                }
            }
            
            let sessionLines = [];
            if (lastSessionIdx !== -1) {
                sessionLines = lines.slice(lastSessionIdx + 1); 
            } else {
                lines.shift(); 
                sessionLines = lines;
            }
            
            let validEcg = []; 
            let validPpg = []; 
            let validSpO2 = [];
            let validPairsCount = 0;
            let pulseDeficitCount = 0; 
            
            let tableRowsHTML = "";

            sessionLines.forEach(line => {
                let cols = line.split(',');
                if(cols.length >= 4 && !line.includes("---NEW_SESSION---")) {
                    tableRowsHTML += `<tr><td>${cols[0]}</td><td>${cols[1]}</td><td>${cols[2]}</td><td>${cols[3]}</td></tr>`;
                    let ecgHR = parseInt(cols[1]); 
                    let ppgHR = parseInt(cols[2]); 
                    let spo2 = parseInt(cols[3]);  
                    
                    let hasEcg = (ecgHR > 40 && ecgHR < 180);
                    let hasPpg = (ppgHR > 40 && ppgHR < 180 && spo2 > 70);

                    if(hasEcg) validEcg.push(ecgHR);
                    if(hasPpg) {
                        validPpg.push(ppgHR);
                        validSpO2.push(spo2);
                    }

                    if(hasEcg && hasPpg) {
                        validPairsCount++;
                        if (Math.abs(ecgHR - ppgHR) > 12) pulseDeficitCount++;
                    }
                }
            });

            if(validEcg.length < 15 || validPpg.length < 15) {
                let errorHtml = "<h3 style='color:#ff9900; text-align:center;'>Không đủ dữ liệu đồng bộ!</h3>";
                errorHtml += "<p style='text-align:center;'>Hệ thống yêu cầu cả 2 cảm biến phải ghi nhận tín hiệu đồng thời để có thể đối chiếu.</p>";
                errorHtml += "<div style='background: #222; padding: 15px; border-radius: 8px; text-align: left; width: 90%; margin: 0 auto;'>";
                
                if (validEcg.length < 15) errorHtml += "<p style='color:#ff3366; margin: 5px 0;'>❌ <b>Điện tim (ECG):</b> Mất tín hiệu (Chỉ đo được " + validEcg.length + " giây). Hãy kiểm tra lại miếng dán ngực và dây dẫn.</p>";
                else errorHtml += "<p style='color:#00ffcc; margin: 5px 0;'>✅ <b>Điện tim (ECG):</b> Tín hiệu ổn định (" + validEcg.length + " giây).</p>";

                if (validPpg.length < 15) errorHtml += "<p style='color:#ff3366; margin: 5px 0;'>❌ <b>Mạch ngón tay (MAX30102):</b> Mất tín hiệu (Chỉ đo được " + validPpg.length + " giây). Hãy đảm bảo ngón tay đặt chắc chắn lên cảm biến.</p>";
                else errorHtml += "<p style='color:#00ffcc; margin: 5px 0;'>✅ <b>Mạch ngón tay (MAX30102):</b> Tín hiệu ổn định (" + validPpg.length + " giây).</p>";

                errorHtml += "</div>";
                errorHtml += "<p style='text-align:center; margin-top: 15px; color:#aaa;'>Vui lòng chỉnh lại cảm biến, nhấn <b>🔴 BẮT ĐẦU GHI</b> và đo lại tối thiểu 1 phút.</p>";
                
                document.getElementById('cardioResult').innerHTML = errorHtml;
                return;
            }

            let avgEcgHR = validEcg.reduce((a, b) => a + b, 0) / validEcg.length;
            let avgSpO2 = validSpO2.reduce((a, b) => a + b, 0) / validSpO2.length;
            let deficitRate = (pulseDeficitCount / validPairsCount) * 100; 
            
            let cardioScore = 100;
            if (avgEcgHR > 95) cardioScore -= (avgEcgHR - 95); else if (avgEcgHR < 55) cardioScore -= (55 - avgEcgHR);
            if (avgSpO2 < 95) cardioScore -= ((95 - avgSpO2) * 2);
            cardioScore -= (deficitRate * 1.5); 
            cardioScore = Math.max(0, Math.min(100, Math.round(cardioScore))); 

            let status = "", color = "", conclusion = "", medicalDetail = "", themeColor = "";
            
            if(cardioScore >= 85) { 
                status = "HỆ TIM MẠCH ỔN ĐỊNH"; color = "#00ffcc"; themeColor = "#28a745"; 
                conclusion = "Tuần hoàn ngoại vi tốt, cơ tim co bóp hiệu quả.";
                medicalDetail = "• <b>Chức năng bơm máu:</b> Hoạt động đồng bộ giữa lệnh thần kinh (ECG) và mạch thể tích thực tế (PPG) hoàn hảo.<br>• <b>Tưới máu:</b> SpO2 duy trì tốt phản ánh khả năng cung cấp oxy ngoại vi không bị cản trở.";
            }
            else if (cardioScore >= 65) { 
                status = "CẦN LƯU Ý THEO DÕI"; color = "#ff9900"; themeColor = "#ffc107";
                conclusion = "Hiệu suất bơm máu ngoại vi giảm nhẹ hoặc nhịp tim nền cao.";
                medicalDetail = "• <b>Yếu tố:</b> Có sự xuất hiện của độ lệch điện - cơ nhẹ. Hệ thần kinh phát lệnh nhưng áp lực chưa đủ mạnh truyền đến tay.<br>• <b>Khuyến nghị:</b> Thường gặp khi mệt mỏi, stress hoặc sai tư thế. Cần nghỉ ngơi và đo lại phiên mới.";
            }
            else { 
                status = "CẢNH BÁO Y TẾ"; color = "#ff3366"; themeColor = "#dc3545"; 
                conclusion = "Dấu hiệu rõ rệt của Mạch khuyết (Pulse Deficit) hoặc thiếu oxy.";
                medicalDetail = "• <b>Phân tích Lâm sàng:</b> Sự sai lệch lớn giữa nhịp ECG và PPG là dấu chỉ điểm của co bóp tâm thất rỗng.<br>• <b>Hành động:</b> Cần thăm khám chuyên khoa Tim mạch ngay lập tức nếu kèm theo khó thở.";
            }

            let reportDate = new Date().toLocaleString('vi-VN');
            generatedHtmlReport = `
            <!DOCTYPE html>
            <html lang="vi">
            <head>
                <meta charset="UTF-8">
                <title>Bệnh án Tim mạch - HUST Health Monitor</title>
                <style>
                    body { font-family: 'Segoe UI', Arial, sans-serif; background-color: #f4f7f6; color: #333; margin: 0; padding: 20px; }
                    .container { max-width: 900px; margin: 0 auto; background: #fff; padding: 30px; border-radius: 8px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
                    .header { text-align: center; border-bottom: 4px solid #cc0000; padding-bottom: 15px; margin-bottom: 25px; }
                    .header h1 { color: #cc0000; margin: 0; font-size: 26px; text-transform: uppercase; letter-spacing: 1px; }
                    .header h2 { color: #555; margin: 8px 0 0 0; font-size: 18px; }
                    .header p { color: #888; font-size: 13px; margin-top: 5px; }
                    
                    .dashboard { display: flex; justify-content: space-between; gap: 20px; margin-bottom: 30px; }
                    .box { background: #f9f9f9; padding: 20px; border-radius: 8px; border-left: 5px solid #cc0000; flex: 1; }
                    
                    .score-panel { text-align: center; border-left-color: ${themeColor}; }
                    .score-circle { width: 120px; height: 120px; border-radius: 50%; background: ${themeColor}; color: white; display: flex; align-items: center; justify-content: center; font-size: 42px; font-weight: bold; margin: 0 auto 10px auto; box-shadow: 0 4px 10px rgba(0,0,0,0.2); border: 5px solid #fff; outline: 3px solid ${themeColor}; }
                    .status-title { color: ${themeColor}; font-size: 20px; font-weight: bold; margin-top: 15px; text-transform: uppercase;}
                    
                    h3 { color: #cc0000; border-bottom: 1px solid #eee; padding-bottom: 10px; margin-top: 0; }
                    .info-line { margin: 8px 0; font-size: 15px; }
                    
                    .detail-box { background: #fff; padding: 20px; border: 1px solid #ddd; border-left: 5px solid #001a33; border-radius: 8px; margin-bottom: 30px; }
                    
                    table { width: 100%; border-collapse: collapse; font-size: 14px; margin-top: 10px; }
                    th, td { border: 1px solid #ddd; padding: 10px; text-align: center; }
                    th { background-color: #cc0000; color: white; position: sticky; top: 0; }
                    tr:nth-child(even) { background-color: #f4f4f4; }
                    tr:hover { background-color: #ffe6e6; }
                    
                    .footer { text-align: center; margin-top: 40px; font-size: 12px; color: #888; border-top: 1px solid #eee; padding-top: 15px; }
                </style>
            </head>
            <body>
                <div class="container">
                    <div class="header">
                        <h1>ĐẠI HỌC BÁCH KHOA HÀ NỘI</h1>
                        <h2>PHIẾU KẾT QUẢ ĐÁNH GIÁ CHẤT LƯỢNG TIM MẠCH</h2>
                        <p>Thời gian trích xuất: ${reportDate}</p>
                    </div>
                    
                    <div class="dashboard">
                        <div class="box score-panel">
                            <h3>ĐIỂM ĐÁNH GIÁ TỔNG HỢP</h3>
                            <div class="score-circle">${cardioScore}</div>
                            <div class="status-title">${status}</div>
                        </div>
                        
                        <div class="box">
                            <h3>THÔNG SỐ LÂM SÀNG THU THẬP</h3>
                            <p class="info-line">♥ <b>Nhịp tim nền (ECG):</b> ${Math.round(avgEcgHR)} BPM</p>
                            <p class="info-line">🫁 <b>Độ bão hòa Oxy (SpO2):</b> ${Math.round(avgSpO2)} %</p>
                            <p class="info-line">⚡ <b>Tỷ lệ Mạch khuyết:</b> ${deficitRate.toFixed(1)} %</p>
                            <p class="info-line">⏱ <b>Kích thước mẫu đồng bộ:</b> ${validPairsCount} giây</p>
                            <hr style="border:0; border-top:1px solid #ddd; margin: 15px 0;">
                            <p class="info-line"><b>Kết luận tóm tắt:</b> ${conclusion}</p>
                        </div>
                    </div>

                    <div class="detail-box">
                        <h3 style="color: #001a33;">CHI TIẾT CHẨN ĐOÁN Y KHOA</h3>
                        <p style="line-height: 1.6;">${medicalDetail}</p>
                    </div>

                    <h3 style="margin-top: 30px;">BẢNG DỮ LIỆU ĐO CHI TIẾT (RAW DATA)</h3>
                    <table>
                        <thead>
                            <tr>
                                <th>Thời gian (ms/Ngày)</th>
                                <th>Nhịp Điện (ECG)</th>
                                <th>Nhịp Thể Tích (PPG)</th>
                                <th>SpO2 (%)</th>
                            </tr>
                        </thead>
                        <tbody>
                            ${tableRowsHTML}
                        </tbody>
                    </table>

                    <div class="footer">
                        <p>Báo cáo được trích xuất tự động từ Hệ thống Monitor Y Sinh bằng công nghệ Edge Computing.</p>
                        <p><b>Lưu ý:</b> Chỉ số mang tính chất tham khảo kỹ thuật cho nghiên cứu, không thay thế chẩn đoán y khoa chuyên nghiệp.</p>
                    </div>
                </div>
            </body>
            </html>
            `;

            document.getElementById('cardioResult').innerHTML = `
                <div style="text-align: center; margin-bottom: 20px;">
                    <h1 style="color:${color}; margin: 10px 0; font-size: 1.8em;">${status}</h1>
                    <p style="font-size: 1.2em;">Điểm Y khoa: <b style="font-size: 1.8em; color:${color};">${cardioScore}</b> <span style="font-size: 0.8em;">/100</span></p>
                    <p style="color:#ccc; font-style:italic; font-size:1em;">${conclusion}</p>
                </div>
                
                <div class="medical-section" style="border-left-color: #00aaff;">
                    <div class="medical-title" style="color: #00aaff;">1. THỐNG KÊ TỪ PHIÊN ĐO NÀY (Đồng bộ: ${validPairsCount}s)</div>
                    <p class="medical-text">♥ Tần số tim cơ bản (ECG): <b>${Math.round(avgEcgHR)} BPM</b></p>
                    <p class="medical-text">🫁 Độ bão hòa Oxy (SpO2): <b>${Math.round(avgSpO2)}%</b></p>
                    <p class="medical-text">⚡ Tỷ lệ Mạch khuyết (Pulse Deficit): <b>${deficitRate.toFixed(1)}%</b></p>
                </div>

                <div class="medical-section" style="border-left-color: ${color};">
                    <div class="medical-title" style="color: ${color};">2. CHẨN ĐOÁN CỦA HỆ THỐNG</div>
                    <p class="medical-text">${medicalDetail}</p>
                </div>

                <button class="btn-export" onclick="exportHTMLReport()">💾 TẢI PHIẾU BÁO CÁO CHI TIẾT</button>
            `;
        }).catch(err => { document.getElementById('cardioResult').innerHTML = "<p style='color:red; text-align:center;'>Lỗi truy xuất dữ liệu! Hãy kiểm tra thẻ SD.</p>"; });
    }

    function exportHTMLReport() {
        if (!generatedHtmlReport) return;
        let blob = new Blob([new Uint8Array([0xEF, 0xBB, 0xBF]), generatedHtmlReport], {type: "text/html;charset=utf-8"});
        let link = document.createElement("a");
        link.href = URL.createObjectURL(blob);
        link.download = "PhieuKetQua_TimMach_HUST_" + new Date().getTime() + ".html";
        link.click();
    }
    
    function closeModal() { document.getElementById('cardioModal').style.display = 'none'; }
    window.onclick = function(event) { let modal = document.getElementById('cardioModal'); if (event.target == modal) modal.style.display = "none"; }
  </script>
</body>
</html>
)rawliteral";

#endif