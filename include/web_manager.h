#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <time.h>

extern bool isRecording;

void initWiFiAndWeb(const char* ssid, const char* password);
void sendDataToWeb(float ecgFiltered, int ppgFiltered, int ecgBeatAvg, int maxBeatAvg, int currentSpO2, int currentPAT, String statusMsg);
void cleanupWebClients();

#endif