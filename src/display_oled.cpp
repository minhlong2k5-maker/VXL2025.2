#include "display_oled.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

// BIẾN ĐỒ THỊ OLED CHỈ DÙNG NỘI BỘ TRONG FILE NÀY
int xPos = 0;
int lastY_ECG = 32;
int lastY_PPG = 53;

// 1. Khởi tạo màn hình
void initOLED() {
    Serial.println("1. Dang khoi tao OLED SPI...");
    if(!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("-> LOI: Khong khoi tao duoc OLED!");
        while (1);
    }
    display.clearDisplay(); 
    display.setTextColor(WHITE); 
    display.setTextSize(1);
    display.setCursor(15, 25); 
    display.print("Init Sensors..."); 
    display.display();
}

// 2. Vẽ đồ thị đè lên nhau (chạy theo xCompress)
void drawWaveforms(int yPos_ECG, int yPos_PPG_current, bool isLeadsOff, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, int fingerState) {
    static int xCompress = 0;
    static int peakY_ECG = GRAPH_ECG_BOTTOM; 
    
    // Lưu điểm nhọn nhất của ECG
    if (yPos_ECG < peakY_ECG) peakY_ECG = yPos_ECG;
    xCompress++;
    
    if (xCompress >= 10) { 
        xCompress = 0;
        // Quét dọn 4 pixel phía trước
        int clearWidth = 4;
        for (int i = 0; i < clearWidth; i++) {
            int clearX = (xPos + i) % 128;
            display.drawLine(clearX, GRAPH_ECG_TOP, clearX, GRAPH_ECG_BOTTOM, BLACK);
            display.drawLine(clearX, GRAPH_PPG_TOP, clearX, GRAPH_PPG_BOTTOM, BLACK);
        }

        // Vạch phân cách đứt nét
        if (xPos % 3 == 0) display.drawPixel(xPos, GRAPH_PPG_TOP - 2, WHITE);

        if (xPos == 0) {
            display.fillRect(0, 0, 128, 22, BLACK);
            display.setTextSize(1);
            display.setCursor(0, 5);
            if (isLeadsOff) {
                display.print("ECG: LEAD OFF!");
            } else {
                display.print("ECG HR: ");
                if (ecgBeatAvg > 0) display.print(ecgBeatAvg);
                else display.print("--");
                display.print(" BPM");
            }

            display.setCursor(0, 14);
            if (fingerState == 1) {
                display.print("MAX:"); display.print(maxBeatAvg); 
                display.print(" O2:"); display.print(currentSpO2); display.print("%");
            } else {
                display.print("MAX: No Finger");
            }
            
            lastY_ECG = peakY_ECG;
            lastY_PPG = yPos_PPG_current;
        } else {
            // Nối điểm
            display.drawLine(xPos - 1, lastY_ECG, xPos, peakY_ECG, WHITE);
            display.drawLine(xPos - 1, lastY_PPG, xPos, yPos_PPG_current, WHITE);
            lastY_ECG = peakY_ECG;
            lastY_PPG = yPos_PPG_current;
        }

        peakY_ECG = GRAPH_ECG_BOTTOM;
        xPos++;
        if (xPos >= 128) xPos = 0;
        
        if (xPos % 2 == 0) display.display();
    }
}