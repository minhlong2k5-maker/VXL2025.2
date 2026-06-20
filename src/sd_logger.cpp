#include "sd_logger.h"

// 1. Khởi tạo thẻ SD
void initSDCard(int csPin) {
    Serial.println("1.5. Dang khoi tao The Nho SD...");
    if (!SD.begin(csPin)) {
        Serial.println("-> LOI: Khong tim thay the nho SD hoac loi chan CS!");
    } else {
        Serial.println("-> The Nho SD OK!");
        // Tạo file và ghi tiêu đề cột nếu file chưa tồn tại
        File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_APPEND);
        if (dataFile) {
            dataFile.println("Thoi_Gian,NhipTim_ECG,NhipTim_MAX30102,SpO2(%)");
            dataFile.close();
        }
    }
}

// 2. Đánh dấu phiên đo mới (khi bấm nút Record trên web)
void markNewSession() {
    File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_APPEND);
    if (dataFile) {
        dataFile.println("---NEW_SESSION---,0,0,0");
        dataFile.close();
        Serial.println("-> Da danh dau Phien do moi!");
    }
}

// 3. Ghi dữ liệu cảm biến theo thời gian thực
void logDataToSD(unsigned long currentMillis, int ecgBeatAvg, int maxBeatAvg, int currentSpO2) {
    File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_APPEND);
    if (dataFile) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 0)) { 
            char timeStringBuff[64];
            strftime(timeStringBuff, sizeof(timeStringBuff), "%d/%m/%Y %H:%M:%S", &timeinfo);
            dataFile.print(timeStringBuff); 
        } else {
            dataFile.print(currentMillis);
        }
        dataFile.print(","); dataFile.print(ecgBeatAvg);       
        dataFile.print(","); dataFile.print(maxBeatAvg);       
        dataFile.print(","); dataFile.println(currentSpO2);    
        dataFile.close();
    }
}

// 4. Xóa sạch thẻ nhớ
bool resetSDCard() {
    if (SD.exists("/dulieu_benhnhan.csv")) {
        SD.remove("/dulieu_benhnhan.csv");
    }
    File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_WRITE);
    if (dataFile) {
        dataFile.println("Thoi_Gian,NhipTim_ECG,NhipTim_MAX30102,SpO2(%)"); 
        dataFile.close();
        return true;
    }
    return false;
}