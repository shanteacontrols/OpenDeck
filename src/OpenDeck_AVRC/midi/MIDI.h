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
    bool setParameter(uint8_t messageType, uint8_t parameterID, uint8_t newParameterID);
    void setHandleSysEx(void(*fptr)(uint8_t sysExArray[], uint8_t arraySize));
    void setHandleNote(void(*fptr)(uint8_t note, uint8_t noteVelocity));

    void sendMIDInote(uint8_t, bool, uint8_t);
    void sendProgramChange(uint8_t program);
    void sendControlChange(uint8_t ccNumber, uint8_t value);
    void sendSysEx(uint8_t *sysExArray, uint8_t size);

    private:
    uint32_t            lastSysExMessageTime;
    midiMessageSource   source;

    //functions
    uint8_t getMIDIchannel(uint8_t);
    bool setMIDIchannel(uint8_t, uint8_t);
    bool getFeature(uint8_t featureID);
    void (*sendSysExCallback)(uint8_t sysExArray[], uint8_t arraySize);
    void (*sendNoteCallback)(uint8_t note, uint8_t noteVelocity);

};

extern MIDI midi;


#endif /* MIDIHELPER_H_ */