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

#include "../Types.h"
#include "../BitManipulation.h"
#include "Config.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "../core/usb/midi/Descriptors.h"

//usb
void EVENT_USB_Device_ConfigurationChanged(void);

class MIDI    {

    public:
    MIDI();
    bool init(midiInterfaceType_t type);

    public:
    void sendNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel = 0);
    void sendNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel = 0);
    void sendProgramChange(uint8_t inProgramNumber, uint8_t inChannel = 0);
    void sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel = 0);
    void sendPitchBend(int16_t inPitchValue, uint8_t inChannel);
    void sendPolyPressure(uint8_t inNoteNumber, uint8_t inPressure, uint8_t inChannel = 0);
    void sendAfterTouch(uint8_t inPressure, uint8_t inChannel = 0);
    void sendSysEx(uint16_t inLength, const uint8_t* inArray, bool inArrayContainsBoundaries);
    void sendTimeCodeQuarterFrame(uint8_t inTypeNibble, uint8_t inValuesNibble);
    void sendTimeCodeQuarterFrame(uint8_t inData);
    void sendSongPosition(uint16_t inBeats);
    void sendSongSelect(uint8_t inSongNumber);
    void sendTuneRequest();
    void sendRealTime(midiMessageType_t inType);
    void setNoteOffMode(noteOffType_t type);

    void enableRunningStatus();
    void disableRunningStatus();
    bool runningStatusEnabled();
    void setNoteChannel(uint8_t channel);
    void setCCchannel(uint8_t channel);
    void setProgramChangeChannel(uint8_t channel);

    private:
    void send(midiMessageType_t inType, uint8_t inData1, uint8_t inData2, uint8_t inChannel);

    //MIDI input

    public:
    bool read(midiInterfaceType_t type);
    bool read(uint8_t inChannel, midiInterfaceType_t type);

    midiMessageType_t getType(midiInterfaceType_t type) const;
    uint8_t  getChannel(midiInterfaceType_t type) const;
    uint8_t getData1(midiInterfaceType_t type) const;
    uint8_t getData2(midiInterfaceType_t type) const;
    uint8_t* getSysExArray(midiInterfaceType_t type);
    uint16_t getSysExArrayLength(midiInterfaceType_t type);
    bool check() const;
    uint8_t getInputChannel() const;
    void setInputChannel(uint8_t inChannel);
    midiMessageType_t getTypeFromStatusByte(uint8_t inStatus);
    uint8_t getChannelFromStatusByte(uint8_t inStatus);
    bool isChannelMessage(midiMessageType_t inType);

    private:
    bool addSysExArrayByte(midiInterfaceType_t type);

    public:
    midiFilterMode_t getFilterMode() const;
    bool getThruState() const;

    void turnThruOn(midiFilterMode_t inThruFilterMode = Full);
    void turnThruOff();
    void setThruFilterMode(midiFilterMode_t inThruFilterMode);

    private:
    void thruFilter(uint8_t inChannel);
    bool parse(midiInterfaceType_t type);
    bool inputFilter(uint8_t inChannel, midiInterfaceType_t type);
    void resetInput();
    uint8_t getStatus(midiMessageType_t inType, uint8_t inChannel) const;
    bool    usbEnabled,
            dinEnabled;

    //decoded data of a MIDI message
    struct Message  {

        //MIDI channel on which the message was received (1-16)
        uint8_t channel;

        //the type of the message
        midiMessageType_t type;

        //first data byte (0-127)
        uint8_t data1;

        //second data byte (0-127, 0 if message length is 2 bytes)
        uint8_t data2;

        //sysex array length is stocked on 16 bits, in data1 (LSB) and data2 (MSB)
        uint8_t sysexArray[MIDI_SYSEX_ARRAY_SIZE];

        //message valid/invalid (no channel consideration here, validity means the message respects the MIDI norm)
        bool valid;

    };

    bool                mThruActivated;
    midiFilterMode_t    mThruFilterMode;
    bool                useRunningStatus;
    bool                use1byteParsing;

    uint8_t             mRunningStatus_RX;
    uint8_t             mRunningStatus_TX;
    uint8_t             mInputChannel;
    uint8_t             mPendingMessage[3];
    uint16_t            dinPendingMessageExpectedLenght;
    uint16_t            dinPendingMessageIndex;
    uint16_t            sysExArrayLength;
    Message             dinMessage,
                        usbMessage;
    noteOffType_t       noteOffMode;
    uint8_t             noteChannel_,
                        ccChannel_,
                        programChangeChannel_,
                        aftertouchChannel_;

};

extern MIDI midi;

#endif