#ifndef SENSOR_MAX30102_H
#define SENSOR_MAX30102_H

#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

void initMAX30102();
void processMAX30102(int &fingerState, int &ppgFiltered, int &yPos_PPG_current, int &maxBeatAvg, int &currentSpO2, String &statusMsg, bool isLeadsOff);

#endif