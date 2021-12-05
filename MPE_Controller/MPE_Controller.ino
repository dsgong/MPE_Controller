#include <MIDI.h>
#include <stdlib.h> 
#include "Adafruit_MPR121.h"
#include <EEPROM.h>


// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE(); 

//Struct for each channel
//Contains number of notes per channel, array of all pitches, and channel pressure.
struct{
    int numNotes;
    byte pitchList[3];
    int16_t pressure;
}typedef channel;

channel allChannels[15];

Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
Adafruit_MPR121 cap3 = Adafruit_MPR121();
Adafruit_MPR121 cap4 = Adafruit_MPR121();
int keyMinMax[48][2];

void setup()
{
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleCC);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();
    Serial.begin(115200);

    Serial.write(0xBF);
    Serial.write(0x79);
    Serial.write(0x00);
    Serial.write(0x64);
    Serial.write(0x06);
    Serial.write(0x65);
    Serial.write(0x00);
    Serial.write(0x06);
    Serial.write(0x0F);
    //Write MCM
    //Midi configuration message for one zone with 15 channels (1-15). Channel 16 is global channel. See pg 5 of MPE spec.

    for(int i = 0; i < 15; i++){
        allChannels[i].numNotes = 0;
        allChannels[i].pressure = 0;
    }

    readMinMax(0);
    /*if (!cap1.begin(0x5C)) {
        while (1);
    }*/
    cap1.begin(0x5C);
    cap2.begin(0x5A);
    cap3.begin(0x5D);
    cap4.begin(0x5B);
    //cap1 corresponds to octave 1, cap2 to octave 2 etc.
}

void handleNoteOn(byte channel, byte pitch, byte velocity){
    byte i = 0;
    while(i < 16){
        if(allChannels[i].numNotes == 0){
            break;
        }
        i++;
    }
    allChannels[i].numNotes++;
    allChannels[i].pitchList[allChannels[i].numNotes - 1] = pitch;
    MIDI.sendNoteOn(pitch, velocity, i + 1);
}

void handleNoteOff(byte channel, byte pitch, byte velocity){
    byte i = 0;
    while(i < 16){
        if(allChannels[i].pitchList[allChannels[i].numNotes - 1] == pitch){
            break;
        }
        i++;
    }
    allChannels[i].numNotes--;
    allChannels[i].pitchList[allChannels[i].numNotes - 1] = 0;
    MIDI.sendNoteOff(pitch, velocity, i + 1);
}

void handlePitchBend(byte channel, int ammount){
    MIDI.sendPitchBend(16, ammount);
    //Send pitch bend wheel to universal channel
}

void handleCC(byte channel, byte MSB, byte LSB){
    MIDI.sendControlChange(MSB, LSB, 16);
    //Sends CC to universal channel
    //Mod wheel is a CC message
}

void readMinMax(int address){
    int temp;
    for(int i = 0; i < 48; i++){
        EEPROM.get(address, temp);
        address += sizeof(int);
        keyMinMax[i][0] = temp;
        EEPROM.get(address, temp);
        address += sizeof(int);
        keyMinMax[i][1] = temp;
    }
}

int findPressure(byte pitch){
    //returns pressure for a given pitch.
    int val;
    double keyMin, keyMax;
        if(pitch < 36 || pitch > 83){
        return 0;
    }
    keyMin = keyMinMax[pitch - 36][0];
    keyMax = keyMinMax[pitch - 36][1];
    if(pitch < 48) {
        val = cap1.filteredData(pitch - 36);
    }else if(pitch < 60){
        val = cap2.filteredData(pitch - 48);
    }else if(pitch < 72){
        val = cap3.filteredData(pitch - 60);
    }else{
        val = cap4.filteredData(pitch - 72);
    }
    val = 127.0 * (val - keyMax) / (keyMin - keyMax);
    return val + ((127 - val) & ((127 - val) >>(sizeof(int) * 8 - 1)));
    //Use point slope to scale value on the line containing (keyMax, 0) and (keyMin, 127)
    //returns minimum of scaled values and 127. 
}

void loop() {
    for(int i = 0; i < 12; i++){
        MIDI.read();
        if(allChannels[i].numNotes != 0){
            MIDI.sendAfterTouch(findPressure(allChannels[i].pitchList[0]), i + 1);
        }
    }
    /*Serial.println();
    Serial.println("Scaled Data:");
    for(uint8_t i = 0; i < 12; i++){
        Serial.print(findPressure(i)); Serial.print("\t");
    }
    Serial.println();
    Serial.println("Unscaled Data:");
    for(uint8_t i = 0; i < 12; i++){
        Serial.print(cap1.filteredData(i)); Serial.print("\t");
    }
    delay(1000);*/
    //Pressure write to serial monitor
}
