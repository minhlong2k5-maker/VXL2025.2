#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Wire.h> // Khởi tạo I2C chung

// Các module thư viện tự viết
#include "sd_logger.h"
#include "display_oled.h"
#include "web_manager.h"
#include "sensor_ecg.h"
#include "sensor_max30102.h"

// ==========================================
// BIẾN TRUNG TÂM (Dùng chung cho các module)
// ==========================================
bool isLeadsOff = false;
int fingerState = 0;
String statusMsg = "READY";

float ecgFiltered = 2048;
int ecgBeatAvg = 0;
int yPos_ECG = 0;

int ppgFiltered = 0;
int yPos_PPG_current = 53;
int maxBeatAvg = 0;
int currentSpO2 = 0;

// Cấu hình vòng lặp
unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL = 4;
unsigned long lastLogTime = 0;
const int LOG_INTERVAL = 1000;

// Cấu hình Mạng
const char* ssid = "E318-1C7"; // Đổi lại theo wifi của bạn nhé
const char* password = "bme318c7";

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ESP32 KHOI DONG ---");

    // Bật I2C dùng chung cho OLED và MAX30102
    Wire.begin(); 
    Wire.setClock(400000);

    // Khởi tạo toàn bộ phần cứng
    initOLED();
    initSDCard(4); 
    initECG();
    initMAX30102();
    initWiFiAndWeb(ssid, password);

    // Kích hoạt Watchdog Timer (Chống treo 5 giây)
    esp_task_wdt_init(5, true); 
    esp_task_wdt_add(NULL); 
}

void loop() {
    esp_task_wdt_reset(); // "Cho chó ăn" để mạch không bị reset

    unsigned long currentMillis = millis();

    // ----------------------------------------------------
    // 1. CHU KỲ 4ms: ĐỌC CẢM BIẾN VÀ CẬP NHẬT MÀN HÌNH
    // ----------------------------------------------------
    if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
        lastSampleTime = currentMillis;

        // Xử lý tín hiệu
        processECG(isLeadsOff, yPos_ECG, ecgFiltered, ecgBeatAvg);
        processMAX30102(fingerState, ppgFiltered, yPos_PPG_current, maxBeatAvg, currentSpO2, statusMsg, isLeadsOff);
        
        // Vẽ đồ thị
        drawWaveforms(yPos_ECG, yPos_PPG_current, isLeadsOff, ecgBeatAvg, maxBeatAvg, currentSpO2, fingerState);

        // Đẩy lên Web (Dùng biến đếm để giảm tải băng thông mạng, chỉ gửi mỗi 40ms)
        static int wsCounter = 0;
        wsCounter++;
        if (wsCounter >= 10) {
            wsCounter = 0;
            sendDataToWeb(ecgFiltered, ppgFiltered, ecgBeatAvg, maxBeatAvg, currentSpO2, statusMsg);
        }
    }

    // ----------------------------------------------------
    // 2. CHU KỲ 1 GIÂY: LƯU THẺ NHỚ
    // ----------------------------------------------------
    if (currentMillis - lastLogTime >= LOG_INTERVAL) {
        lastLogTime = currentMillis;
        if (isRecording && (fingerState == 1 || ecgBeatAvg > 0)) {
            logDataToSD(currentMillis, ecgBeatAvg, maxBeatAvg, currentSpO2);
        }
    }

    // Dọn dẹp bộ nhớ mạng
    cleanupWebClients();
}