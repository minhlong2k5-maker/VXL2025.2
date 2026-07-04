#ifndef SENSOR_ECG_H
#define SENSOR_ECG_H

#include <Arduino.h>

void initECG();
void processECG(bool &isLeadsOff, int &yPos_ECG, float &ecgFiltered, int &ecgBeatAvg, unsigned long &lastEcgTime);

#endif