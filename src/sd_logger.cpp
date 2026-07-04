#include "sd_logger.h"

void initSDCard(int csPin) {
    Serial.println("1.5. Dang khoi tao The Nho SD...");
    if (!SD.begin(csPin)) {
        Serial.println("-> LOI: Khong tim thay the nho SD hoac loi chan CS!");
    } else {
        Serial.println("-> The Nho SD OK!");
        File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_APPEND);
        if (dataFile) {
            // THÊM CỘT PAT VÀO HEADER FILE CSV
            dataFile.println("Thoi_Gian,NhipTim_ECG,NhipTim_MAX30102,SpO2(%),PAT(ms)");
            dataFile.close();
        }
    }
}

void markNewSession() {
    File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_APPEND);
    if (dataFile) {
        dataFile.println("---NEW_SESSION---,0,0,0,0");
        dataFile.close();
        Serial.println("-> Da danh dau Phien do moi!");
    }
}

void logDataToSD(unsigned long currentMillis, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, int currentPAT) {
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
        dataFile.print(","); dataFile.print(currentSpO2);   
        // GHI THÊM PAT XUỐNG DÒNG CUỐI CÙNG 
        dataFile.print(","); dataFile.println(currentPAT);
        dataFile.close();
    }
}

bool resetSDCard() {
    if (SD.exists("/dulieu_benhnhan.csv")) {
        SD.remove("/dulieu_benhnhan.csv");
    }
    File dataFile = SD.open("/dulieu_benhnhan.csv", FILE_WRITE);
    if (dataFile) {
        // CẬP NHẬT HEADER KHI RESET THẺ MỚI
        dataFile.println("Thoi_Gian,NhipTim_ECG,NhipTim_MAX30102,SpO2(%),PAT(ms)"); 
        dataFile.close();
        return true;
    }
    return false;
}