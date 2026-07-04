#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <time.h>

void initSDCard(int csPin);
void markNewSession();
void logDataToSD(unsigned long currentMillis, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, int currentPAT);
bool resetSDCard();

#endif