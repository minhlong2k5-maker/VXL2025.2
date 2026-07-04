#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Wire.h> 

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

// CÁC BIẾN MỚI CHO TÍNH TOÁN PAT
unsigned long lastEcgTime = 0;
unsigned long lastPpgTime = 0;
bool newPpgBeat = false;
int currentPAT = 0;

// Cấu hình vòng lặp
unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL = 4;
unsigned long lastLogTime = 0;
const int LOG_INTERVAL = 1000;

// Cấu hình Mạng
const char* ssid = "Trâm Anh";
const char* password = "12345678";

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- ESP32 KHOI DONG ---");

    Wire.begin(); 
    Wire.setClock(400000);

    initOLED();
    initSDCard(4); 
    initECG();
    initMAX30102();
    initWiFiAndWeb(ssid, password);

    esp_task_wdt_init(5, true); 
    esp_task_wdt_add(NULL); 
}

void loop() {
    esp_task_wdt_reset(); 

    unsigned long currentMillis = millis();

    // ----------------------------------------------------
    // 1. CHU KỲ 4ms: ĐỌC CẢM BIẾN VÀ CẬP NHẬT MÀN HÌNH
    // ----------------------------------------------------
    if (currentMillis - lastSampleTime >= SAMPLE_INTERVAL) {
        lastSampleTime = currentMillis;

        // Cập nhật hàm gọi với đầy đủ các tham số timestamp
        processECG(isLeadsOff, yPos_ECG, ecgFiltered, ecgBeatAvg, lastEcgTime);
        processMAX30102(fingerState, ppgFiltered, yPos_PPG_current, maxBeatAvg, currentSpO2, statusMsg, isLeadsOff, newPpgBeat, lastPpgTime);
        
        // TÍNH TOÁN PAT: Tính độ trễ giữa đỉnh ECG và đỉnh PPG
        if (newPpgBeat) {
            if (lastPpgTime > lastEcgTime) {
                int calcPAT = lastPpgTime - lastEcgTime;
                // Lọc nhiễu: Giữ lại các giá trị PAT hợp lý trong khoảng sinh lý học (50ms - 400ms)
                if (calcPAT > 50 && calcPAT < 400) {
                    currentPAT = calcPAT;
                }
            }
        }

        drawWaveforms(yPos_ECG, yPos_PPG_current, isLeadsOff, ecgBeatAvg, maxBeatAvg, currentSpO2, fingerState);

        static int wsCounter = 0;
        wsCounter++;
        if (wsCounter >= 10) {
            wsCounter = 0;
            // Cập nhật tham số currentPAT để đẩy lên Web
            sendDataToWeb(ecgFiltered, ppgFiltered, ecgBeatAvg, maxBeatAvg, currentSpO2, currentPAT, statusMsg);
        }
    }

    // ----------------------------------------------------
    // 2. CHU KỲ 1 GIÂY: LƯU THẺ NHỚ
    // ----------------------------------------------------
    if (currentMillis - lastLogTime >= LOG_INTERVAL) {
        lastLogTime = currentMillis;
        if (isRecording && (fingerState == 1 || ecgBeatAvg > 0)) {
            // Cập nhật tham số currentPAT để ghi vào thẻ SD
            logDataToSD(currentMillis, ecgBeatAvg, maxBeatAvg, currentSpO2, currentPAT);
        }
    }

    cleanupWebClients();
}