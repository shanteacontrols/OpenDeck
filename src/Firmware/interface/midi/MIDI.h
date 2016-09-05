#ifndef MIDIHELPER_H_
#define MIDIHELPER_H_

#include "../../Types.h"
#include "../../hardware/midi/MIDI.h"

class MIDI {

    public:
    MIDI();
    void init();
    void checkInput();

    void sendMIDInote(uint8_t note, bool state, uint8_t _velocity);
    void sendProgramChange(uint8_t program);
    void sendControlChange(uint8_t ccNumber, uint8_t value);
    void sendSysEx(uint8_t *sysExArray, uint8_t size, bool arrayContainsBoundaries, bool usbSend = true, bool dinSend = false);

    private:
    uint32_t            lastSysExMessageTime;
    midiInterfaceType_t source;

};

extern MIDI midi;

#endif
