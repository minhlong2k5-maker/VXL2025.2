#include "sensor_ecg.h"
#include "display_oled.h" // Dùng để lấy các biến ranh giới màn hình

const int ECG_PIN = 34;
const int LO_PLUS_PIN = 25;  
const int LO_MINUS_PIN = 26;
const int BUZZER_PIN = 32;

int ecgRaw = 0;
unsigned long lastEcgBeatTime = 0;
bool ecgBeatDetected = false;
unsigned long beepStartTime = 0;
bool isBeeping = false;

void initECG() {
    pinMode(BUZZER_PIN, OUTPUT); 
    digitalWrite(BUZZER_PIN, LOW); 
    pinMode(LO_PLUS_PIN, INPUT); 
    pinMode(LO_MINUS_PIN, INPUT);
}

void processECG(bool &isLeadsOff, int &yPos_ECG, float &ecgFiltered, int &ecgBeatAvg) {
    isLeadsOff = (digitalRead(LO_PLUS_PIN) == 1 || digitalRead(LO_MINUS_PIN) == 1);
    yPos_ECG = 0;

    if (!isLeadsOff) {
        ecgRaw = analogRead(ECG_PIN);
        static int ecgBuffer[5];
        static int ecgIndex = 0;
        ecgBuffer[ecgIndex] = ecgRaw;
        ecgIndex = (ecgIndex + 1) % 5;
        long sum = 0;
        for(int i=0; i<5; i++) sum += ecgBuffer[i];
        int smoothedRaw = sum / 5;
        
        ecgFiltered = (ecgFiltered * 0.6) + (smoothedRaw * 0.4); 
        if (ecgFiltered > 4000) ecgFiltered = 4000;
        if (ecgFiltered < 100) ecgFiltered = 100;
        
        if (ecgFiltered > 2400 && !ecgBeatDetected && (millis() - lastEcgBeatTime > 300)) {
            int bpm = 60000 / (millis() - lastEcgBeatTime);
            lastEcgBeatTime = millis();
            if (bpm > 40 && bpm < 180) {
                ecgBeatAvg = (ecgBeatAvg == 0) ? bpm : (ecgBeatAvg * 0.7) + (bpm * 0.3);
            }
            ecgBeatDetected = true;
            if (ecgBeatAvg >= 50 && ecgBeatAvg <= 120) {
                 digitalWrite(BUZZER_PIN, HIGH);
                 beepStartTime = millis(); isBeeping = true;
            }
        }
        if (ecgFiltered < 2100) ecgBeatDetected = false;
        
        if (ecgBeatAvg > 0) {
            if (ecgBeatAvg < 50 || ecgBeatAvg > 120) {
                 digitalWrite(BUZZER_PIN, HIGH);
            } else {
                if (isBeeping && (millis() - beepStartTime > 50)) { 
                     digitalWrite(BUZZER_PIN, LOW);
                     isBeeping = false; 
                }
            }
        }
        yPos_ECG = map((int)ecgFiltered, 1000, 3000, GRAPH_ECG_BOTTOM, GRAPH_ECG_TOP);
        yPos_ECG = constrain(yPos_ECG, GRAPH_ECG_TOP, GRAPH_ECG_BOTTOM);
    } else {
        yPos_ECG = (GRAPH_ECG_TOP + GRAPH_ECG_BOTTOM) / 2;
        ecgBeatAvg = 0; 
        digitalWrite(BUZZER_PIN, LOW); 
    }
}