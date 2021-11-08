#include <MIDI.h>
#include <stdlib.h> 
#include <CapacitiveSensor.h>
//CapacitiveSensor Sensor = CapacitiveSensor(4, 6);

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

void setup()
{
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleCC);
    MIDI.begin(MIDI_CHANNEL_OMNI);
    MIDI.turnThruOff();
    Serial.begin(256000);

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
    int val = Sensor.capacitiveSensor(1);
    /*char str[43];
    sprintf(str, "%d", val);
    Serial.write(str);
    Serial.write(0x20); 
    //This prints capacitive touch values to serial monitor*/
    return val;
}

void loop() {
    MIDI.read();
    /*for(int i = 0; i < 16; i++){
        if(allChannels[i].numNotes != 0){
            MIDI.sendAfterTouch(findPressure(allChannels[i].pitchList[allChannels[i].numNotes - 1]), i);
        }
    }*/
    //Pressure read
}
