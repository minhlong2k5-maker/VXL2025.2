#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <time.h>

// Khai báo biến isRecording là extern để dùng chung giữa Web và main.cpp
extern bool isRecording;

// Khai báo các hàm sẽ dùng trong main.cpp
void initWiFiAndWeb(const char* ssid, const char* password);
void sendDataToWeb(float ecgFiltered, int ppgFiltered, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, String statusMsg);
void cleanupWebClients();

#endif