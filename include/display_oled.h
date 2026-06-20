#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Định nghĩa ranh giới đồ thị để main.cpp cũng dùng được
const int GRAPH_ECG_TOP = 23;
const int GRAPH_ECG_BOTTOM = 42;
const int GRAPH_PPG_TOP = 44;
const int GRAPH_PPG_BOTTOM = 63;

// Các hàm sẽ được main.cpp gọi
void initOLED();
void drawWaveforms(int yPos_ECG, int yPos_PPG_current, bool isLeadsOff, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, int fingerState);

#endif