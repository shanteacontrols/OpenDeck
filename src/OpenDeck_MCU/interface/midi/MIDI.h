#ifndef MIDIHELPER_H_
#define MIDIHELPER_H_

#include "..\Types.h"
#include "..\midi\hw_midi\HWmidi.h"

class MIDI {

    public:
    MIDI();
    void init();
    void checkInput();
    uint8_t getParameter(uint8_t messageType, uint8_t parameterID);
    bool setFeature(uint8_t featureID, uint8_t newValue);
    bool setParameter(uint8_t messageType, uint8_t parameterID, uint8_t newParameterID);

    void sendMIDInote(uint8_t, bool, uint8_t);
    void sendProgramChange(uint8_t program);
    void sendControlChange(uint8_t ccNumber, uint8_t value);
    void sendSysEx(uint8_t *sysExArray, uint8_t size);

    private:
    uint32_t            lastSysExMessageTime;
    midiMessageSource   source;

    //functions
    uint8_t getMIDIchannel(uint8_t);
    bool setMIDIchannel(uint8_t channelID, uint8_t channelNumber);
    bool getFeature(uint8_t featureID);

};

extern MIDI midi;

#endif