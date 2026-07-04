#include "web_manager.h"
#include "sd_logger.h" // Import SD logger để web gọi lệnh reset và mark session
#include "webpage.h"   // Import giao diện HTML

// Định nghĩa biến toàn cục điều khiển ghi thẻ
bool isRecording = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

// Xử lý sự kiện từ WebSocket (nút bấm trên web)
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            data[len] = 0;
            String msg = (char*)data;
            if (msg == "TOGGLE_RECORD") {
                isRecording = !isRecording;
                Serial.print("-> Nhan lenh tu Web. Trang thai ghi the nho: ");
                Serial.println(isRecording ? "ON" : "OFF");
                if (isRecording) {
                    markNewSession(); // Gọi hàm từ sd_logger
                }
            }
        }
    }
}

// Khởi tạo WiFi, Thời gian thực và Web Server
void initWiFiAndWeb(const char* ssid, const char* password) {
    Serial.println("3. Dang ket noi WiFi...");
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500); 
        Serial.print("."); 
    }
    Serial.println("\n-> WiFi OK!");
    Serial.print("-> IP DE VAO WEB LA: "); Serial.println(WiFi.localIP());

    // Cấu hình thời gian
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com");

    if (!MDNS.begin("hust-health")) Serial.println("Loi tao ten mDNS!");
    else Serial.println("-> Ban co the vao Web bang link: http://hust-health.local");

    Serial.println("4. Dang bat Web Server...");
    
    // Cấu hình các đường dẫn Web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
        request->send_P(200, "text/html", index_html); 
    });
    
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        if (SD.exists("/dulieu_benhnhan.csv")) request->send(SD, "/dulieu_benhnhan.csv", "text/csv", true);
        else request->send(404, "text/plain", "Chua co du lieu hoac the nho chua duoc cam dung cach!");
    });
    
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
        if (resetSDCard()) request->send(200, "text/plain", "Da xoa sach va reset file tren the SD thanh cong!");
        else request->send(500, "text/plain", "Loi: Khong the khoi tao lai file moi tren the SD!");
    });

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
}

void sendDataToWeb(float ecgFiltered, int ppgFiltered, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, int currentPAT, String statusMsg) {
    if (ws.count() > 0) {
        static unsigned long lastTimeUpdate = 0;
        static char timeStrCached[32] = "--/--/---- --:--:--"; 

        unsigned long currentMillis = millis();
        if (currentMillis - lastTimeUpdate >= 1000) {
            lastTimeUpdate = currentMillis;
            struct tm timeinfo; 
            if (getLocalTime(&timeinfo, 0)) { 
                strftime(timeStrCached, sizeof(timeStrCached), "%d/%m/%Y %H:%M:%S", &timeinfo);
            }
        }

        // CẬP NHẬT: Thêm biến "pat" vào chuỗi JSON
        String js;
        js.reserve(160); 
        js = "{\"v\":"; js += (int)ecgFiltered;
        js += ",\"p\":"; js += ppgFiltered;
        js += ",\"bE\":"; js += ecgBeatAvg;
        js += ",\"bM\":"; js += maxBeatAvg;
        js += ",\"o2\":"; js += currentSpO2;
        js += ",\"pat\":"; js += currentPAT; // Biến PAT mới
        js += ",\"st\":\""; js += statusMsg;
        js += "\",\"t\":\""; js += timeStrCached;
        js += "\",\"rec\":"; js += (isRecording ? 1 : 0);
        js += "}";

       if (ws.availableForWriteAll()) {
            ws.textAll(js);
       }
    }
}

// Xóa các client bị ngắt kết nối
void cleanupWebClients() {
    ws.cleanupClients();
}