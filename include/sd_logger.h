#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <time.h>

// Khai báo các hàm sẽ dùng để gọi từ main.cpp
void initSDCard(int csPin);
void markNewSession();
void logDataToSD(unsigned long currentMillis, int ecgBeatAvg, int maxBeatAvg, int currentSpO2);
bool resetSDCard();

#endif