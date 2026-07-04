#include "sensor_max30102.h"
#include "display_oled.h" 

MAX30105 particleSensor;

const int LED_PIN = 27; 
const int SPO2_OFFSET = -2;

long ppg_dcFilter = 0;
int ppg_lastAcSignal = 0;

uint32_t irBuffer[100];
uint32_t redBuffer[100];
int bufferIndex = 0;
int8_t validSPO2;
int32_t maxHeartRate = 0;
int8_t validMaxHR;
long lastMaxBeat = 0;
byte rates[5];
byte rateSpot = 0;

void initMAX30102() {
    pinMode(LED_PIN, OUTPUT); 
    digitalWrite(LED_PIN, HIGH);   

    Serial.println("2. Dang khoi tao MAX30102...");
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("-> LOI: Khong tim thay MAX30102!");
        while (1);
    }
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0);    
    particleSensor.setPulseAmplitudeIR(0x1F);  
    particleSensor.setSampleRate(100);
}

void processMAX30102(int &fingerState, int &ppgFiltered, int &yPos_PPG_current, int &maxBeatAvg, int &currentSpO2, String &statusMsg, bool isLeadsOff, bool &newPpgBeat, unsigned long &lastPpgTime) {
    
    newPpgBeat = false; // Reset cờ báo nhịp mới ở mỗi chu kỳ quét

    particleSensor.check();
    if (particleSensor.available()) {
        uint32_t irValue = particleSensor.getFIFOIR();
        uint32_t redValue = particleSensor.getFIFORed();
        particleSensor.nextSample();
        
        if (irValue > 20000) {
            ppg_dcFilter = (ppg_dcFilter * 15 + irValue) / 16;
            int acSignal = irValue - ppg_dcFilter;
            int sharpSignal = (acSignal + ppg_lastAcSignal) / 2;
            ppg_lastAcSignal = acSignal; 
            ppgFiltered = sharpSignal;
            yPos_PPG_current = map(sharpSignal, -120, 120, GRAPH_PPG_BOTTOM, GRAPH_PPG_TOP);
            yPos_PPG_current = constrain(yPos_PPG_current, GRAPH_PPG_TOP, GRAPH_PPG_BOTTOM);
        } else {
            ppgFiltered = 0; 
            yPos_PPG_current = GRAPH_PPG_BOTTOM;
        }

        if (irValue > 20000) { 
            if (fingerState == 0) { 
                fingerState = 1;
                particleSensor.setPulseAmplitudeRed(0x1F); 
                bufferIndex = 0; 
            }
            statusMsg = "DANG DO...";
            
            if (checkForBeat(irValue) == true) {
                long delta = millis() - lastMaxBeat;
                lastMaxBeat = millis();
                
                // XUẤT THỜI GIAN VÀ CỜ BÁO NHỊP MỚI RA NGOÀI
                newPpgBeat = true;
                lastPpgTime = lastMaxBeat;

                float bpm = 60 / (delta / 1000.0);
                if (bpm > 40 && bpm < 180) {
                    rates[rateSpot++] = (byte)bpm;
                    if (rateSpot >= 5) rateSpot = 0;
                    int total = 0, count = 0;
                    for (byte x = 0; x < 5; x++) { 
                        if (rates[x] > 0) { total += rates[x]; count++; } 
                    }
                    if (count > 0) maxBeatAvg = total / count;
                }
            }

            irBuffer[bufferIndex] = irValue;
            redBuffer[bufferIndex] = redValue; 
            bufferIndex++;

            if (bufferIndex >= 100) {
                maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, (int32_t*)&currentSpO2, &validSPO2, &maxHeartRate, &validMaxHR);
                if (validSPO2 == 1 && currentSpO2 > 80) { 
                    currentSpO2 += SPO2_OFFSET; 
                    if (currentSpO2 > 99) currentSpO2 = 99;
                } else currentSpO2 = 0;
                
                for (byte i = 25; i < 100; i++) { 
                    irBuffer[i - 25] = irBuffer[i]; 
                    redBuffer[i - 25] = redBuffer[i];
                }
                bufferIndex = 75;
            }
        } else { 
            if (fingerState == 1) { 
                fingerState = 0;
                particleSensor.setPulseAmplitudeRed(0); 
                maxBeatAvg = 0; currentSpO2 = 0; bufferIndex = 0;
            }
            statusMsg = "CHUA CO TAY";
        }
    }
    if (isLeadsOff) statusMsg = "ECG: LEAD OFF!";
}