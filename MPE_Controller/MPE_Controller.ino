#include <MIDI.h>
#include <stdlib.h> 
#include "Adafruit_MPR121.h"

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE(); 

//Struct for each channel
//Contains number of notes per channel, array of all pitches, and channel pressure.
struct{
    int numNotes;
    byte pitchList[3];
    int16_t pressure; //Actual midi pressure range is a 14 bit signed int
}typedef channel;

channel allChannels[15];

Adafruit_MPR121 cap = Adafruit_MPR121();

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

    if (!cap.begin(0x5A)) {
        while (1);
    }
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

int findPressure(byte pitch){
    //returns pressure for a given pitch.
    int val = cap.filteredData(pitch);
    val = 153 - (0.5079 * val);
    return val + ((127 - val) & ((127 - val) >>(sizeof(int) * 8 - 1)));
    //returns minimum of scaled values and 127. 
}

void loop() {
    for(int i = 0; i < 12; i++){
        MIDI.read();
        if(allChannels[i].numNotes != 0){
            MIDI.sendAfterTouch(findPressure(i), i + 1);
        }
    }
    /*for(uint8_t i = 0; i < 12; i++){
        Serial.print(findPressure(i)); Serial.print("\t");
    }
    Serial.println();*/
    //Pressure write to serial monitor
}
