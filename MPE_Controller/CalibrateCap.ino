#include <EEPROM.h>
#include "Adafruit_MPR121.h"
#include <stdlib.h> 

int keyMinMax[48][2];
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
Adafruit_MPR121 cap3 = Adafruit_MPR121();
Adafruit_MPR121 cap4 = Adafruit_MPR121();


void setup() {
    for(int i = 0; i < 48; i++){
        keyMinMax[i][0] = 32767;
        keyMinMax[i][1] = 0;
    }

    cap1.begin(0x5C);
    cap2.begin(0x5A);
    cap3.begin(0x5D);
    cap4.begin(0x5B);
    Serial.begin(115200);
}

void loop() {
    int temp;
    for(int j = 0; j < 100; j++){
        for(int i = 0; i < 12; i++){
            temp = cap1.filteredData(i);
            if(temp < keyMinMax[i][0]){
                keyMinMax[i][0] = temp;
            }
            if(temp > keyMinMax[i][1]){
                keyMinMax[i][1] = temp;
            }

            temp = cap2.filteredData(i);
            if(temp < keyMinMax[i + 12][0]){
                keyMinMax[i + 12][0] = temp;
            }
            if(temp > keyMinMax[i + 12][1]){
                keyMinMax[i + 12][1] = temp;
            }

            temp = cap3.filteredData(i);
            if(temp < keyMinMax[i + 24][0]){
                keyMinMax[i + 24][0] = temp;
            }
            if(temp > keyMinMax[i + 24][1]){
                keyMinMax[i + 24][1] = temp;
            }
            temp = cap4.filteredData(i);
            if(temp < keyMinMax[i + 36][0]){
                keyMinMax[i + 36][0] = temp;
            }
            if(temp > keyMinMax[i + 36][1]){
                keyMinMax[i + 36][1] = temp;
            }
        }
    }
    int address = 0;
    for(int i = 0; i < 48; i++){
        EEPROM.put(address, keyMinMax[i][0]);
        address += sizeof(int);
        EEPROM.put(address, keyMinMax[i][1]);
        address += sizeof(int);
        Serial.print("Min and Max of key");
        Serial.print(i);
        Serial.print(" ");
        EEPROM.get(address - 4, temp);
        Serial.print(temp);
        Serial.print(" ");
        EEPROM.get(address - 2, temp);
        Serial.print(temp);
        Serial.print("\n");
        
    }
    delay(1000);
        
}
