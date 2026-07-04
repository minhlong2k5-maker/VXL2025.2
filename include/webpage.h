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
    .card { background:#1e1e1e; padding:15px; border-radius:10px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); border-top: 2px solid #001a33; border-bottom: 2px solid #ff8c00; }
    .val { font-size:2.5em; font-weight:bold; color:#00ffcc; }
    .val.spo2 { color: #ff8c00; }
    
    .chart-box { background:#000; border-radius:10px; padding:10px; height:250px; margin-bottom:15px; position:relative; border: 1px solid #001a33; }
    .no-signal { position:absolute; top:0; left:0; width:100%; height:100%; background:rgba(0,0,0,0.8); color:#ff8c00; display:flex; align-items:center; justify-content:center; font-size:3em; font-weight:bold; z-index:10; border-radius:10px; }
    
    .btn-group { margin-bottom: 15px; }
    .btn { padding: 10px 20px; color: #fff; border: none; border-radius: 5px; font-size: 1em; font-weight: bold; cursor: pointer; margin: 0 5px; transition: 0.3s; }
    .btn-download { background: #555; }
    .btn-view { background: #001a33; color: #ff8c00; border: 1px solid #ff8c00; }
    .btn-cardio { background: #ff8c00; color: #001a33; box-shadow: 0 0 10px rgba(255, 140, 0, 0.3); font-size: 1.1em;}
    .btn-cardio:hover { background: #e67e00; }
    
    .btn-record { background: #1a0000; border: 2px solid #ff3366; color: #ff3366; }
    .btn-record.active { background: #ff3366; color: #fff; box-shadow: 0 0 15px rgba(255, 51, 102, 0.6); animation: pulse 1.5s infinite; }
    @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.7; } 100% { opacity: 1; } }
    
    .btn-export { background: #001a33; color: #ff8c00; border: 2px solid #ff8c00; padding: 12px 20px; border-radius: 8px; font-weight: bold; font-size: 1.1em; cursor: pointer; margin-top: 15px; width: 100%; transition: 0.3s; }
    .btn-export:hover { background: #ff8c00; color: #001a33; }

    table { width: 100%; border-collapse: collapse; margin-top: 15px; display: none; background: #1e1e1e; border-radius: 8px; overflow: hidden; }
    th, td { border: 1px solid #444; padding: 10px; text-align: center; }
    th { background-color: #001a33; color: #ff8c00; font-size: 1.1em; }
    tr:nth-child(even) { background-color: #2a2a2a; }
    
    .modal { display: none; position: fixed; z-index: 100; left: 0; top: 0; width: 100%; height: 100%; background-color: rgba(0,0,0,0.85); overflow-y: auto; }
    .modal-content { background-color: #1e1e1e; margin: 5% auto; padding: 25px; border: 3px solid #001a33; border-radius: 10px; width: 90%; max-width: 650px; text-align: left; box-shadow: 0 4px 20px rgba(255,140,0,0.2); }
    .close-btn { color: #ff8c00; float: right; font-size: 28px; font-weight: bold; cursor: pointer; }
    .close-btn:hover { color: #fff; }
    
    .medical-section { background: #111; padding: 15px; border-radius: 8px; margin-top: 15px; border-left: 5px solid #001a33; }
    .medical-title { color: #ff8c00; font-size: 1.1em; font-weight: bold; margin-bottom: 8px; border-bottom: 1px dashed #333; padding-bottom: 5px; }
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
    <button class="btn btn-cardio" onclick="evaluateCardio()">🩺 CHẨN ĐOÁN HUYẾT ÁP & MẠCH MÁU</button>
  </div>
  <div class="btn-group">
    <a href="/download"><button class="btn btn-download">📥 FULL SD (.CSV)</button></a>
    <button class="btn btn-view" onclick="loadTable()">📊 XEM BẢNG</button>
    <button class="btn" style="background: #333; border: 1px solid #ff3366; color: #ff3366;" onclick="resetTable()">🗑️ RESET SD</button>
  </div>

  <div class="chart-box"><canvas id="ecgChart"></canvas></div>
  <div class="chart-box">
    <div id="noSignalBox" class="no-signal">CHƯA ĐẶT TAY</div>
    <canvas id="ppgChart"></canvas>
  </div>
  <table id="dataTable"></table>

  <div id="cardioModal" class="modal">
    <div class="modal-content">
      <span class="close-btn" onclick="closeModal()">&times;</span>
      <h2 style="color: #ff8c00; background: #001a33; padding: 10px; border-radius: 5px; text-align: center; margin-top:0;">BÁO CÁO Y SINH - PAT ANALYSIS</h2>
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
      data: { labels: timeLabels, datasets: [{ borderColor: '#ff8c00', data: ppgData, pointRadius: 0, borderWidth: 2, tension: 0.4 }] }, 
      options: { animation: false, maintainAspectRatio: false, scales: { x: { display: false }, y: { min: -200, max: 200, display: true, grid: { color: '#2a2a2a', lineWidth: 1 }, ticks: { display: true, color: '#ffffff', font: { size: 12, weight: 'bold' } } } }, plugins: { legend: { display: false } } } 
    });
    
    var wsProtocol = (window.location.protocol === "https:") ? "wss://" : "ws://";
    var gateway = wsProtocol + window.location.hostname + "/ws";
    var websocket = new WebSocket(gateway);
    let generatedHtmlReport = ""; 

    function toggleRecord() { if(websocket.readyState === WebSocket.OPEN) websocket.send("TOGGLE_RECORD"); }
    
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
        document.getElementById('cardioResult').innerHTML = "<div style='text-align:center; padding: 20px; color:#ff8c00;'>Đang phân tích dữ liệu PAT từ thẻ SD...</div>";
        
        fetch('/download').then(response => response.text()).then(data => {
            let lines = data.split('\n').filter(l => l.trim().length > 0);
            let lastSessionIdx = -1;
            for (let i = lines.length - 1; i >= 0; i--) { if (lines[i].includes("---NEW_SESSION---")) { lastSessionIdx = i; break; } }
            
            let sessionLines = lastSessionIdx !== -1 ? lines.slice(lastSessionIdx + 1) : lines.slice(1);
            
            let validEcg = [], validSpO2 = [], validPAT = [];
            let tableRowsHTML = "";

            sessionLines.forEach(line => {
                let cols = line.split(',');
                if(cols.length >= 5 && !line.includes("---NEW_SESSION---")) {
                    tableRowsHTML += `<tr><td>${cols[0]}</td><td>${cols[1]}</td><td>${cols[2]}</td><td>${cols[3]}</td><td>${cols[4]}</td></tr>`;
                    let ecgHR = parseInt(cols[1]); 
                    let spo2 = parseInt(cols[3]);  
                    let pat = parseInt(cols[4]);

                    if(ecgHR > 40 && ecgHR < 180) validEcg.push(ecgHR);
                    if(spo2 > 70) validSpO2.push(spo2);
                    if(pat > 50 && pat < 400) validPAT.push(pat);
                }
            });

            if(validPAT.length < 15) {
                document.getElementById('cardioResult').innerHTML = `
                    <h3 style='color:#ff8c00; text-align:center;'>Không đủ dữ liệu PAT đồng bộ!</h3>
                    <p style='text-align:center;'>Yêu cầu đo tối thiểu 15 giây liên tục có ngón tay để bắt đỉnh R và đỉnh PPG.</p>
                    <p style='text-align:center; color:#aaa;'>Vui lòng nhấn <b>🔴 BẮT ĐẦU GHI</b> và thử lại.</p>
                `; return;
            }

            let avgEcgHR = validEcg.reduce((a, b) => a + b, 0) / validEcg.length;
            let avgSpO2 = validSpO2.reduce((a, b) => a + b, 0) / validSpO2.length;
            let avgPAT = validPAT.reduce((a, b) => a + b, 0) / validPAT.length;

            // Tính biến thiên PAT (Delta PAT) giữa nửa đầu và nửa sau phiên đo
            let halfLen = Math.floor(validPAT.length / 2);
            let firstHalf = validPAT.slice(0, halfLen);
            let secondHalf = validPAT.slice(halfLen);
            let avgFirstPAT = firstHalf.reduce((a,b)=>a+b,0) / firstHalf.length;
            let avgSecondPAT = secondHalf.reduce((a,b)=>a+b,0) / secondHalf.length;
            let deltaPAT = avgSecondPAT - avgFirstPAT;

            let cardioScore = 100;
            if (avgEcgHR > 95) cardioScore -= (avgEcgHR - 95); else if (avgEcgHR < 55) cardioScore -= (55 - avgEcgHR);
            if (avgSpO2 < 95) cardioScore -= ((95 - avgSpO2) * 2);
            
            let bpTrendStr = "", bpDetail = "", stiffnessStr = "";
            
            if (avgPAT < 200) { stiffnessStr = "DẤU HIỆU CỨNG HÓA"; cardioScore -= 15; } 
            else { stiffnessStr = "ĐÀN HỒI TỐT"; }

            if (deltaPAT < -5) { bpTrendStr = "XU HƯỚNG TĂNG"; bpDetail = "Thời gian truyền sóng đang bị rút ngắn dần. Áp lực máu lên thành mạch tăng."; cardioScore -= 10; }
            else if (deltaPAT > 5) { bpTrendStr = "XU HƯỚNG GIẢM"; bpDetail = "Thời gian truyền sóng dài ra, mạch máu đang giãn và áp lực giảm."; }
            else { bpTrendStr = "ỔN ĐỊNH"; bpDetail = "Không có sự biến thiên lớn về áp suất mạch máu trong phiên đo."; }
            
            cardioScore = Math.max(0, Math.min(100, Math.round(cardioScore))); 

            let status = "", conclusion = "", color = "";
            if (avgPAT >= 200 && bpTrendStr === "ỔN ĐỊNH") { status = "HUYẾT ÁP & MẠCH MÁU KHỎE MẠNH"; conclusion = "Thành mạch có độ đàn hồi tốt, sóng truyền ổn định."; color = "#00ffcc"; }
            else if (avgPAT >= 200 && bpTrendStr === "XU HƯỚNG TĂNG") { status = "LƯU Ý: HUYẾT ÁP ĐANG TĂNG"; conclusion = "Đàn hồi tốt nhưng nhịp tim và áp lực đang đẩy nhanh sóng mạch."; color = "#ff9900"; }
            else if (avgPAT < 200 && bpTrendStr === "ỔN ĐỊNH") { status = "LƯU Ý: XƠ CỨNG MẠCH MÁU"; conclusion = "Vận tốc truyền sóng nhanh bất thường, dấu hiệu thành mạch kém đàn hồi."; color = "#ff9900"; }
            else { status = "CẢNH BÁO Y TẾ KÉP"; conclusion = "Mạch máu kém đàn hồi kèm theo chu kỳ áp lực đang tăng nhanh. Cần nghỉ ngơi ngay."; color = "#ff3366"; }

            let reportDate = new Date().toLocaleString('vi-VN');
            generatedHtmlReport = `
            <!DOCTYPE html>
            <html lang="vi">
            <head>
                <meta charset="UTF-8">
                <title>Bệnh án Tim mạch - HUST Health Monitor</title>
                <style>
                    body { font-family: 'Segoe UI', Arial, sans-serif; background-color: #f4f7f6; color: #333; margin: 0; padding: 20px; }
                    .container { max-width: 900px; margin: 0 auto; background: #fff; padding: 30px; border-radius: 8px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); border-top: 10px solid #001a33; }
                    .header { text-align: center; border-bottom: 4px solid #ff8c00; padding-bottom: 15px; margin-bottom: 25px; }
                    .header h1 { color: #001a33; margin: 0; font-size: 26px; font-weight: 900; }
                    .header h2 { color: #ff8c00; margin: 8px 0 0 0; font-size: 18px; text-transform: uppercase;}
                    .header p { color: #888; font-size: 13px; margin-top: 5px; }
                    .dashboard { display: flex; justify-content: space-between; gap: 20px; margin-bottom: 30px; }
                    .box { background: #f9f9f9; padding: 20px; border-radius: 8px; border-left: 5px solid #001a33; flex: 1; }
                    .score-panel { text-align: center; border-left-color: #ff8c00; }
                    .score-circle { width: 120px; height: 120px; border-radius: 50%; background: #001a33; color: #ff8c00; display: flex; align-items: center; justify-content: center; font-size: 42px; font-weight: bold; margin: 0 auto 10px auto; border: 5px solid #fff; outline: 3px solid #ff8c00; }
                    .status-title { color: #001a33; font-size: 18px; font-weight: bold; margin-top: 15px; }
                    h3 { color: #001a33; border-bottom: 2px solid #ff8c00; padding-bottom: 5px; margin-top: 0; display: inline-block;}
                    .info-line { margin: 8px 0; font-size: 15px; }
                    .detail-box { background: #fff; padding: 20px; border: 1px solid #ddd; border-left: 5px solid #ff8c00; border-radius: 8px; margin-bottom: 30px; }
                    table { width: 100%; border-collapse: collapse; font-size: 14px; margin-top: 10px; }
                    th, td { border: 1px solid #ddd; padding: 10px; text-align: center; }
                    th { background-color: #001a33; color: #ff8c00; position: sticky; top: 0; }
                    tr:nth-child(even) { background-color: #f4f4f4; }
                    .footer { text-align: center; margin-top: 40px; font-size: 12px; color: #888; border-top: 1px solid #eee; padding-top: 15px; }
                </style>
            </head>
            <body>
                <div class="container">
                    <div class="header">
                        <h1>ĐẠI HỌC BÁCH KHOA HÀ NỘI</h1>
                        <h2>PHÂN TÍCH HUYẾT ÁP BẰNG THUẬT TOÁN P.A.T</h2>
                        <p>Thời gian trích xuất: ${reportDate}</p>
                    </div>
                    
                    <div class="dashboard">
                        <div class="box score-panel">
                            <h3>ĐIỂM SỨC KHỎE</h3>
                            <div class="score-circle">${cardioScore}</div>
                            <div class="status-title">${status}</div>
                        </div>
                        
                        <div class="box">
                            <h3>CHỈ SỐ THU THẬP</h3>
                            <p class="info-line">♥ <b>Nhịp tim nền (ECG):</b> ${Math.round(avgEcgHR)} BPM</p>
                            <p class="info-line">🫁 <b>Oxy trong máu (SpO2):</b> ${Math.round(avgSpO2)} %</p>
                            <p class="info-line">⚡ <b>PAT Trung bình:</b> ${Math.round(avgPAT)} ms</p>
                            <p class="info-line">⏱ <b>Biến thiên Huyết áp:</b> ${bpTrendStr} (${deltaPAT > 0 ? '+':''}${deltaPAT.toFixed(1)} ms)</p>
                            <hr style="border:0; border-top:1px dashed #001a33; margin: 15px 0;">
                            <p class="info-line"><b>Kết luận:</b> ${conclusion}</p>
                        </div>
                    </div>

                    <div class="detail-box">
                        <h3 style="color: #ff8c00; border-color: #001a33;">GIẢI THÍCH LÂM SÀNG</h3>
                        <p style="line-height: 1.6;">• <b>Độ cứng Động mạch:</b> ${stiffnessStr}. ${avgPAT < 200 ? 'Sóng áp lực truyền đi quá nhanh do thành mạch cứng.' : 'Thành mạch mềm mại, co giãn tốt để hấp thụ áp lực máu.'}</p>
                        <p style="line-height: 1.6;">• <b>Động lực Huyết áp:</b> ${bpDetail}</p>
                    </div>

                    <h3 style="margin-top: 30px;">DỮ LIỆU ĐO CHI TIẾT (RAW DATA)</h3>
                    <table>
                        <thead>
                            <tr>
                                <th>Thời gian</th>
                                <th>ECG (BPM)</th>
                                <th>PPG (BPM)</th>
                                <th>SpO2 (%)</th>
                                <th>PAT (ms)</th>
                            </tr>
                        </thead>
                        <tbody>${tableRowsHTML}</tbody>
                    </table>

                    <div class="footer">
                        <p>Báo cáo trích xuất từ Hệ thống Monitor Y Sinh bằng công nghệ Edge Computing.</p>
                        <p><b>Lưu ý:</b> Chỉ số PAT có tính chất tham khảo nghiên cứu sinh lý học, không thay thế máy đo bắp tay y tế.</p>
                    </div>
                </div>
            </body>
            </html>
            `;

            document.getElementById('cardioResult').innerHTML = `
                <div style="text-align: center; margin-bottom: 20px;">
                    <h1 style="color:${color}; margin: 10px 0; font-size: 1.6em; font-weight: 900;">${status}</h1>
                    <p style="font-size: 1.2em;">Điểm Y khoa: <b style="font-size: 1.8em; color:${color};">${cardioScore}</b> <span style="font-size: 0.8em;">/100</span></p>
                    <p style="color:#ccc; font-style:italic; font-size:1em;">${conclusion}</p>
                </div>
                
                <div class="medical-section" style="border-left-color: #001a33;">
                    <div class="medical-title" style="color: #ff8c00;">1. KẾT QUẢ ĐO P.A.T (Pulse Arrival Time)</div>
                    <p class="medical-text">⏱ Thời gian truyền sóng trung bình: <b style="color: #fff;">${Math.round(avgPAT)} ms</b></p>
                    <p class="medical-text">🧬 Trạng thái đàn hồi mạch máu: <b style="color: #fff;">${stiffnessStr}</b></p>
                    <p class="medical-text">📈 Động lực xu hướng Huyết áp: <b style="color: #fff;">${bpTrendStr}</b></p>
                </div>

                <div class="medical-section" style="border-left-color: #ff8c00;">
                    <div class="medical-title" style="color: #ff8c00;">2. CHỈ SỐ NỀN</div>
                    <p class="medical-text">♥ Tần số tim cơ bản (ECG): <b>${Math.round(avgEcgHR)} BPM</b></p>
                    <p class="medical-text">🫁 Độ bão hòa Oxy (SpO2): <b>${Math.round(avgSpO2)}%</b></p>
                </div>

                <button class="btn-export" onclick="exportHTMLReport()">💾 XUẤT PHIẾU BÁO CÁO Y SINH</button>
            `;
        }).catch(err => { document.getElementById('cardioResult').innerHTML = "<p style='color:red; text-align:center;'>Lỗi truy xuất dữ liệu! Hãy kiểm tra thẻ SD.</p>"; });
    }

    function exportHTMLReport() {
        if (!generatedHtmlReport) return;
        let blob = new Blob([new Uint8Array([0xEF, 0xBB, 0xBF]), generatedHtmlReport], {type: "text/html;charset=utf-8"});
        let link = document.createElement("a");
        link.href = URL.createObjectURL(blob);
        link.download = "BaoCao_PAT_HUST_" + new Date().getTime() + ".html";
        link.click();
    }
    
    function closeModal() { document.getElementById('cardioModal').style.display = 'none'; }
    window.onclick = function(event) { let modal = document.getElementById('cardioModal'); if (event.target == modal) modal.style.display = "none"; }
  </script>
</body>
</html>
)rawliteral";

#endif