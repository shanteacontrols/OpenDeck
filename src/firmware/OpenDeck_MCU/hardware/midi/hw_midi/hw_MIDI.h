/*!
 *  @file       MIDI.h
 *  Project     Arduino MIDI Library
 *  @brief      MIDI Library for the Arduino
 *  @version    4.2
 *  @author     Francois Best
 *  @date       24/02/11
 *  @license    GPL v3.0 - Copyright Forty Seven Effects 2014
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//library modifed by Igor Petrovic

 #ifndef HW_MIDI_H
 #define HW_MIDI_H

#include "../../interface/settings/MIDIsettings.h"
#include "../../hardware/board/Board.h"
#include "../../hardware/uart/UART.h"
#include "../../Types.h"

#define USE_SERIAL_PORT         uart

#define MIDI_CHANNEL_OMNI       0
#define MIDI_CHANNEL_OFF        17 // and over

#define MIDI_PITCHBEND_MIN      -8192
#define MIDI_PITCHBEND_MAX      8191

class HWmidi    {

    public:
    HWmidi();
    bool init(uint8_t inChannel, bool inputEnabled, bool outputEnabled, midiInterfaceType type);

    //MIDI output

    public:
    void sendNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel);
    void sendNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel);
    void sendProgramChange(uint8_t inProgramNumber, uint8_t inChannel);
    void sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel);
    void sendPitchBend(int inPitchValue, uint8_t inChannel);
    void sendPitchBend(double inPitchValue, uint8_t inChannel);
    void sendPolyPressure(uint8_t inNoteNumber, uint8_t inPressure, uint8_t inChannel);
    void sendAfterTouch(uint8_t inPressure, uint8_t inChannel);
    void sendSysEx(uint16_t inLength, const uint8_t* inArray, bool inArrayContainsBoundaries);
    void sendTimeCodeQuarterFrame(uint8_t inTypeNibble, uint8_t inValuesNibble);
    void sendTimeCodeQuarterFrame(uint8_t inData);
    void sendSongPosition(uint16_t inBeats);
    void sendSongSelect(uint8_t inSongNumber);
    void sendTuneRequest();
    void sendRealTime(midiMessageType inType);

    void enableRunningStatus();
    void disableRunningStatus();

    private:
    void send(midiMessageType inType, uint8_t inData1, uint8_t inData2, uint8_t inChannel);

    //MIDI input

    public:
    bool read();
    bool read(uint8_t inChannel);

    midiMessageType getType() const;
    uint8_t  getChannel() const;
    uint8_t getData1() const;
    uint8_t getData2() const;
    const uint8_t* getSysExArray() const;
    uint16_t getSysExArrayLength() const;
    bool check() const;
    uint8_t getInputChannel() const;
    void setInputChannel(uint8_t inChannel);
    midiMessageType getTypeFromStatusByte(uint8_t inStatus);
    uint8_t getChannelFromStatusByte(uint8_t inStatus);
    bool isChannelMessage(midiMessageType inType);

    //MIDI soft thru

    public:
    MidiFilterMode getFilterMode() const;
    bool getThruState() const;

    void turnThruOn(MidiFilterMode inThruFilterMode = Full);
    void turnThruOff();
    void setThruFilterMode(MidiFilterMode inThruFilterMode);

    private:
    void thruFilter(uint8_t inChannel);
    bool parse();
    bool inputFilter(uint8_t inChannel);
    void resetInput();
    uint8_t getStatus(midiMessageType inType, uint8_t inChannel) const;

    //decoded data of a MIDI message
    struct Message  {

        //MIDI channel on which the message was received (1-16)
        uint8_t channel;

        //the type of the message
        midiMessageType type;

        //first data byte (0-127)
        uint8_t data1;

        //second data byte (0-127, 0 if message length is 2 bytes)
        uint8_t data2;

        //sysex array length is stocked on 16 bits, in data1 (LSB) and data2 (MSB)
        uint8_t sysexArray[MIDI_SYSEX_ARRAY_SIZE];

        //message valid/invalid (no channel consideration here, validity means the message respects the MIDI norm)
        bool valid;

    };

    bool            mThruActivated;
    MidiFilterMode  mThruFilterMode;
    bool            useRunningStatus;
    bool            use1byteParsing;

    uint8_t         mRunningStatus_RX;
    uint8_t         mRunningStatus_TX;
    uint8_t         mInputChannel;
    uint8_t         mPendingMessage[3];
    uint16_t        mPendingMessageExpectedLenght;
    uint16_t        mPendingMessageIndex;
    Message         mMessage;

};

extern HWmidi hwMIDI;

#endif