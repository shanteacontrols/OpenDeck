#ifndef LIB_MIDI_H_
#define LIB_MIDI_H_

#include <inttypes.h>
#include "..\hardware/uart/UART.h"
#include "..\Types.h"
#include "..\midi\MIDIsettings.h"

#define COMPILE_MIDI_IN         1
#define COMPILE_MIDI_OUT        1

#define USE_SERIAL_PORT         uart

//each call to read() will only parse one uint8_t (might be faster)
#define USE_1BYTE_PARSING       1

#define MIDI_CHANNEL_OMNI       0
#define MIDI_CHANNEL_OFF        17  //and over

//decoded data of a MIDI message
struct midimsg  {

    //MIDI channel on which the message was received (1-16)
    uint8_t channel; 

    midiMessageType type;

    //first data byte (0-127)
    uint8_t data1;

    //second data byte (0-127, 0 if message length is 2 bytes)
    uint8_t data2;

    //sysex array length is stocked on 16 bits, in data1 (LSB) and data2 (MSB)
    uint8_t sysex_array[MIDI_SYSEX_ARRAY_SIZE];

    //message valid/invalid (no channel consideration here, validity means the message respects the MIDI norm)
    bool valid;

};

class MIDI_Class {

    public:
    MIDI_Class();

    void begin(uint8_t inChannel);

    #if COMPILE_MIDI_OUT

    public:
    void sendNoteOn(uint8_t NoteNumber, uint8_t Velocity, uint8_t Channel);
    void sendNoteOff(uint8_t NoteNumber, uint8_t Velocity, uint8_t Channel);
    void sendControlChange(uint8_t ControlNumber, uint8_t ControlValue, uint8_t Channel);
    void sendProgramChange(uint8_t ProgramNumber, uint8_t Channel);
    void sendPitchBend(uint16_t PitchValue, uint8_t Channel);
    void sendSysEx(int length, const uint8_t *const array, bool ArrayContainsBoundaries = false);
    void enableRunningStatus();
    void disableRunningStatus();

    void send(midiMessageType type, uint8_t param1, uint8_t param2, uint8_t channel);

    private:
    const uint8_t genstatus(const midiMessageType inType, const uint8_t inChannel) const;
    void setRunningStatusState(bool state);
    uint8_t mRunningStatus_TX;
    bool runningStatusEnabled;

    #endif

    #if COMPILE_MIDI_IN 

    public:
    bool read();
    bool read(const uint8_t Channel);

    //getters
    midiMessageType getType() const;
    uint8_t getChannel() const;
    uint8_t getData1() const;
    uint8_t getData2() const;
    uint8_t* getSysExArray();
    int16_t getSysExArrayLength();
    bool check() const;

    uint8_t getInputChannel() const    { return mInputChannel; }

    //setters
    void setInputChannel(const uint8_t Channel);

    //extract an enumerated MIDI type from a status byte
    static inline const midiMessageType getTypeFromStatusByte(const uint8_t inStatus)    {

        if (
            (inStatus < 0x80)   ||
            (inStatus == 0xF4)  ||
            (inStatus == 0xF5)  ||
            (inStatus == 0xF9)  ||
            (inStatus == 0xFD)
           )    return midiMessageInvalidType;  //data bytes and undefined

        if (inStatus < 0xF0)    return (midiMessageType)(inStatus & 0xF0);  //channel message, remove channel nibble
        else                    return (midiMessageType)(inStatus);

    }

    private:
    bool input_filter(uint8_t inChannel);
    bool parse(uint8_t inChannel);
    void reset_input_attributes();

    //attributes
    uint8_t     mRunningStatus_RX;
    uint8_t     mInputChannel;

    uint8_t     mPendingMessage[MIDI_SYSEX_ARRAY_SIZE];
    uint16_t    mPendingMessageExpectedLenght;
    uint16_t    mPendingMessageIndex;   //extended to unsigned int for larger sysex payloads.

    midimsg     mMessage;

    #endif

};

extern MIDI_Class hwMIDI;

#endif