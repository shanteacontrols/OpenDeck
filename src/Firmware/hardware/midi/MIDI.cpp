/*!
 *  @file       MIDI.hpp
 *  Project     Arduino MIDI Library
 *  @brief      MIDI Library for the Arduino - Inline implementations
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

//library modified by Igor Petrovic

#include "MIDI.h"

#ifdef HW_MIDI_H

USB_ClassInfo_MIDI_Device_t MIDI_Interface;

HWmidi::HWmidi()   {

    //default constructor

    mRunningStatus_TX               = midiMessageInvalidType;
    mRunningStatus_RX               = midiMessageInvalidType;

    dinPendingMessageIndex          = 0;
    dinPendingMessageExpectedLenght = 0;

    dinMessage.valid                = false;
    dinMessage.type                 = midiMessageInvalidType;
    dinMessage.channel              = 0;
    dinMessage.data1                = 0;
    dinMessage.data2                = 0;

    usbMessage.valid                = false;
    usbMessage.type                 = midiMessageInvalidType;
    usbMessage.channel              = 0;
    usbMessage.data1                = 0;
    usbMessage.data2                = 0;

    mThruFilterMode                 = Off;
    mThruActivated                  = false;
    useRunningStatus                = false;
    use1byteParsing                 = true;

    MIDI_Interface.Config.StreamingInterfaceNumber = INTERFACE_ID_AudioStream;

    MIDI_Interface.Config.DataINEndpoint.Address = MIDI_STREAM_IN_EPADDR;
    MIDI_Interface.Config.DataINEndpoint.Size = MIDI_STREAM_EPSIZE;
    MIDI_Interface.Config.DataINEndpoint.Banks = 1;

    MIDI_Interface.Config.DataOUTEndpoint.Address = MIDI_STREAM_OUT_EPADDR;
    MIDI_Interface.Config.DataOUTEndpoint.Size = MIDI_STREAM_EPSIZE;
    MIDI_Interface.Config.DataOUTEndpoint.Banks = 1;

}

bool HWmidi::init(bool inputEnabled, bool outputEnabled, midiInterfaceType_t type) {

    switch(type)    {

        case dinInterface:
        USE_SERIAL_PORT.begin(31250, inputEnabled, outputEnabled);
        return true;
        break;

        case usbInterface:
        USB_Init();
        return true;
        break;

        default:
        return false;
        break;

    }

}

void HWmidi::send(midiMessageType_t inType, uint8_t inData1, uint8_t inData2, uint8_t inChannel, midiInterfaceType_t type)  {

    //inType:       MIDI message type
    //inData1:      The first data byte
    //inData2:      The second data byte (if the message contains only 1 data byte, set this one to 0)
    //inChannel:    The output channel on which the message will be sent (values from 1 to 16)

    //test if channel is valid
    if (inChannel >= MIDI_CHANNEL_OFF  ||
    inChannel == MIDI_CHANNEL_OMNI ||
    inType < 0x80)  {

        if (useRunningStatus && (type == dinInterface))
            mRunningStatus_TX = midiMessageInvalidType;

        return; //don't send anything

    }

    if (inType <= midiMessagePitchBend) {

        //channel messages

        //protection: remove MSBs on data
        inData1 &= 0x7f;
        inData2 &= 0x7f;

        const uint8_t status = getStatus(inType, inChannel);

        if (useRunningStatus && (type == dinInterface))   {

            if (mRunningStatus_TX != status)    {

                //new message, memorize and send header
                mRunningStatus_TX = status;
                USE_SERIAL_PORT.write(mRunningStatus_TX);

            }

        }   else {

            switch(type)    {

                case dinInterface:
                //don't care about running status, send the status byte
                USE_SERIAL_PORT.write(status);
                break;

                default:
                //nothing
                break;

            }

        }

        //send data
        switch(type)    {

            case dinInterface:
            USE_SERIAL_PORT.write(inData1);
            break;

            default:
            break;

        }

        if ((inType != midiMessageProgramChange) && (inType != midiMessageAfterTouchChannel))   {

            switch(type)    {

                case dinInterface:
                USE_SERIAL_PORT.write(inData2);
                break;

                default:
                break;

            }

        }

        if (type == usbInterface)   {

            uint8_t midiEvent = (uint8_t)inType >> 4;
            uint8_t data1 = getStatus(inType, inChannel);

            MIDI_EventPacket_t MIDIEvent = (MIDI_EventPacket_t)
            {
                .Event       = midiEvent,

                .Data1       = data1,
                .Data2       = inData1,
                .Data3       = inData2,
            };

            //write the MIDI event packet to the endpoint
            //Endpoint_Write_Stream_LE(&MIDIEvent, sizeof(MIDIEvent), NULL);

            //send the data in the endpoint to the host
            //Endpoint_ClearIN();

            MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
            MIDI_Device_Flush(&MIDI_Interface);

        }

    }   else if (inType >= midiMessageTuneRequest && inType <= midiMessageSystemReset)
            sendRealTime(inType, type); //system real-time and 1 byte

}

void HWmidi::sendNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel, midiInterfaceType_t type)    {

    //inNoteNumber:   Pitch value in the MIDI format (0 to 127)
    //inVelocity:     Note attack velocity (0 to 127)
    //inChannel:      The channel on which the message will be sent (1 to 16).

    send(midiMessageNoteOn, inNoteNumber, inVelocity, inChannel, type);

}

void HWmidi::sendNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel, midiInterfaceType_t type)   {

    //inNoteNumber:    Pitch value in the MIDI format (0 to 127)
    //inVelocity:      Release velocity (0 to 127)
    //inChannel:       The channel on which the message will be sent (1 to 16)

    send(midiMessageNoteOff, inNoteNumber, inVelocity, inChannel, type);

}

void HWmidi::sendProgramChange(uint8_t inProgramNumber, uint8_t inChannel, midiInterfaceType_t type)  {

    //inProgramNumber:    The Program to select (0 to 127)
    //inChannel:          The channel on which the message will be sent (1 to 16)
    send(midiMessageProgramChange, inProgramNumber, 0, inChannel, type);

}

void HWmidi::sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel, midiInterfaceType_t type)  {

    //inControlNumber:    The controller number (0 to 127)
    //inControlValue:     The value for the specified controller (0 to 127)
    //inChannel:          The channel on which the message will be sent (1 to 16)

    send(midiMessageControlChange, inControlNumber, inControlValue, inChannel, type);

}

void HWmidi::sendPolyPressure(uint8_t inNoteNumber, uint8_t inPressure, uint8_t inChannel, midiInterfaceType_t type)  {

     //inNoteNumber:    The note to apply AfterTouch to (0 to 127)
     //inPressure:      The amount of AfterTouch to apply (0 to 127)
     //inChannel:       The channel on which the message will be sent (1 to 16)

    send(midiMessageAfterTouchPoly, inNoteNumber, inPressure, inChannel, type);

}

void HWmidi::sendAfterTouch(uint8_t inPressure, uint8_t inChannel, midiInterfaceType_t type)  {

     //inPressure:  The amount of AfterTouch to apply to all notes
     //inChannel:   The channel on which the message will be sent (1 to 16)

    send(midiMessageAfterTouchChannel, inPressure, 0, inChannel, type);

}

void HWmidi::sendPitchBend(int16_t inPitchValue, uint8_t inChannel, midiInterfaceType_t type) {

    //inPitchValue: The amount of bend to send (in a signed integer format),
                    //between MIDI_PITCHBEND_MIN and MIDI_PITCHBEND_MAX,
                    //center value is 0
    //inChannel:    The channel on which the message will be sent (1 to 16)

    const unsigned bend = inPitchValue - MIDI_PITCHBEND_MIN;
    send(midiMessagePitchBend, lowByte_7bit(bend), highByte_7bit(bend), inChannel, type);

}

void HWmidi::sendSysEx(uint16_t inLength, const uint8_t* inArray, bool inArrayContainsBoundaries, midiInterfaceType_t type)   {

     //inLength:                    The size of the array to send
     //inArray:                     The byte array containing the data to send
     //inArrayContainsBoundaries:   When set to 'true', 0xf0 & 0xf7 bytes
                                    //(start & stop SysEx) will NOT be sent
                                    //(and therefore must be included in the array)

    MIDI_EventPacket_t MIDIEvent;

    switch(type)    {

        case dinInterface:
        if (!inArrayContainsBoundaries)
            USE_SERIAL_PORT.write(0xf0);

        for (uint16_t i=0; i<inLength; ++i)
            USE_SERIAL_PORT.write(inArray[i]);

        if (!inArrayContainsBoundaries)
            USE_SERIAL_PORT.write(0xf7);

        if (useRunningStatus)
            mRunningStatus_TX = midiMessageInvalidType;
        break;

        case usbInterface:
        if (!inArrayContainsBoundaries)   {

            //append sysex start (0xF0) and stop (0xF7) bytes to array

            bool firstByte = true;
            bool startSent = false;

            while (inLength > 3) {

                if (firstByte)  {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStartCin),

                        .Data1       = midiMessageSystemExclusive,
                        .Data2       = inArray[0],
                        .Data3       = inArray[1],
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                    firstByte = false;
                    startSent = true;
                    inArray += 2;
                    inLength -= 2;

                }   else {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStartCin),

                        .Data1       = inArray[0],
                        .Data2       = inArray[1],
                        .Data3       = inArray[2],
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                    inArray += 3;
                    inLength -= 3;

                }

            }

            if (inLength == 3)    {

                if (startSent)  {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStartCin),

                        .Data1       = inArray[0],
                        .Data2       = inArray[1],
                        .Data3       = inArray[2],
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStop1byteCin),

                        .Data1       = 0xF7,
                        .Data2       = 0,
                        .Data3       = 0,
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                }   else {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStartCin),

                        .Data1       = midiMessageSystemExclusive,
                        .Data2       = inArray[0],
                        .Data3       = inArray[1],
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStop2byteCin),

                        .Data1       = inArray[2],
                        .Data2       = 0xF7,
                        .Data3       = 0,
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                }

            }

            else if (inLength == 2) {

                if (startSent)  {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStop3byteCin),

                        .Data1       = inArray[0],
                        .Data2       = inArray[1],
                        .Data3       = 0xF7,
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                }

                else {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStartCin),

                        .Data1       = midiMessageSystemExclusive,
                        .Data2       = inArray[0],
                        .Data3       = inArray[1],
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStop1byteCin),

                        .Data1       = 0xF7,
                        .Data2       = 0,
                        .Data3       = 0,
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                }

            }

            else if (inLength == 1) {

                if (startSent)  {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStop2byteCin),

                        .Data1       = inArray[0],
                        .Data2       = 0xF7,
                        .Data3       = 0,
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                }   else {

                    MIDIEvent = (MIDI_EventPacket_t)
                    {
                        .Event       = MIDI_EVENT(0, sysExStop3byteCin),

                        .Data1       = 0xF0,
                        .Data2       = inArray[0],
                        .Data3       = 0xF7,
                    };

                    MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                    MIDI_Device_Flush(&MIDI_Interface);

                }

            }

        }   else {

            while (inLength > 3) {

                MIDIEvent = (MIDI_EventPacket_t)
                {
                    .Event       = MIDI_EVENT(0, sysExStartCin),

                    .Data1       = inArray[0],
                    .Data2       = inArray[1],
                    .Data3       = inArray[2],
                };

                MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                MIDI_Device_Flush(&MIDI_Interface);

                inArray += 3;
                inLength -= 3;

            }

            if (inLength == 3)  {

                MIDIEvent = (MIDI_EventPacket_t)
                {
                    .Event       = MIDI_EVENT(0, sysExStop3byteCin),

                    .Data1       = inArray[0],
                    .Data2       = inArray[1],
                    .Data3       = inArray[2],
                };

                MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                MIDI_Device_Flush(&MIDI_Interface);

            }   else if (inLength == 2) {

                MIDIEvent = (MIDI_EventPacket_t)
                {
                    .Event       = MIDI_EVENT(0, sysExStop2byteCin),

                    .Data1       = inArray[0],
                    .Data2       = inArray[1],
                    .Data3       = 0,
                };

                MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                MIDI_Device_Flush(&MIDI_Interface);

            }   else if (inLength == 1) {

                MIDIEvent = (MIDI_EventPacket_t)
                {
                    .Event       = MIDI_EVENT(0, sysExStop1byteCin),

                    .Data1       = inArray[0],
                    .Data2       = 0,
                    .Data3       = 0,
                };

                MIDI_Device_SendEventPacket(&MIDI_Interface, &MIDIEvent);
                MIDI_Device_Flush(&MIDI_Interface);

            }

        }
        break;

    }

}

void HWmidi::sendTuneRequest(midiInterfaceType_t type)  {

    //when a MIDI unit receives this message,
    //it should tune its oscillators (if equipped with any)

    sendRealTime(midiMessageTuneRequest, type);

}

void HWmidi::sendTimeCodeQuarterFrame(uint8_t inTypeNibble, uint8_t inValuesNibble, midiInterfaceType_t type) {

     //inTypeNibble     MTC type
     //inValuesNibble   MTC data

    uint8_t data = (((inTypeNibble & 0x07) << 4) | (inValuesNibble & 0x0f));
    sendTimeCodeQuarterFrame(data, type);

}

void HWmidi::sendTimeCodeQuarterFrame(uint8_t inData, midiInterfaceType_t type)   {

    //inData:   if you want to encode directly the nibbles in your program,
                //you can send the byte here.

    USE_SERIAL_PORT.write((uint8_t)midiMessageTimeCodeQuarterFrame);
    USE_SERIAL_PORT.write(inData);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendSongPosition(uint16_t inBeats, midiInterfaceType_t type) {

    //inBeats:  The number of beats since the start of the song

    USE_SERIAL_PORT.write((uint8_t)midiMessageSongPosition);
    USE_SERIAL_PORT.write(inBeats & 0x7f);
    USE_SERIAL_PORT.write((inBeats >> 7) & 0x7f);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendSongSelect(uint8_t inSongNumber, midiInterfaceType_t type)   {

    //inSongNumber: Wanted song number

    USE_SERIAL_PORT.write((uint8_t)midiMessageSongSelect);
    USE_SERIAL_PORT.write(inSongNumber & 0x7f);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendRealTime(midiMessageType_t inType, midiInterfaceType_t type)   {

     //inType:  The available Real Time types are:
                //Start, Stop, Continue, Clock, ActiveSensing and SystemReset
                //You can also send a Tune Request with this method

    switch (inType) {

        case midiMessageTuneRequest: //not really real-time, but one byte anyway
        case midiMessageClock:
        case midiMessageStart:
        case midiMessageStop:
        case midiMessageContinue:
        case midiMessageActiveSensing:
        case midiMessageSystemReset:
        USE_SERIAL_PORT.write((uint8_t)inType);
        break;

        default:
        //invalid Real Time marker
        break;

    }

    //do not cancel Running Status for real-time messages as they can be
    //interleaved within any message
    //tuneRequest can be sent here, and as it is a System Common message,
    //it must reset Running Status
    if (useRunningStatus && inType == midiMessageTuneRequest)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::enableRunningStatus()  {

    useRunningStatus = true;

}

void HWmidi::disableRunningStatus() {

    useRunningStatus = false;

}

bool HWmidi::runningStatusEnabled() {

    return useRunningStatus;

}

uint8_t HWmidi::getStatus(midiMessageType_t inType, uint8_t inChannel) const  {

    return ((uint8_t)inType | ((inChannel - 1) & 0x0f));

}

bool HWmidi::read(midiInterfaceType_t type) {

    //returns true if a valid message has been stored in the structure, false if not
    //a valid message is a message that matches the input channel
    //if the Thru is enabled and the message matches the filter,
    //it is sent back on the MIDI output

    return read(mInputChannel, type);

}

bool HWmidi::read(uint8_t inChannel, midiInterfaceType_t type)    {

    if (inChannel >= MIDI_CHANNEL_OFF)
        return false; //MIDI Input disabled

    if (!parse(type)) return false;

    const bool channelMatch = inputFilter(inChannel, type);

    //thruFilter(inChannel, type);

    return channelMatch;

}

bool HWmidi::parse(midiInterfaceType_t type)    {

    if (type == dinInterface)   {

        //parsing algorithm:
        //get a byte from the serial buffer
        //if there is no pending message to be recomposed, start a new one
        //find type and channel (if pertinent)
        //look for other bytes in buffer, call parser recursively,
        //until the message is assembled or the buffer is empty
        //else, add the extracted byte to the pending message, and check validity
        //when the message is done, store it

        if (USE_SERIAL_PORT.available() == 0)
            //no data available
            return false;

        const uint8_t extracted = USE_SERIAL_PORT.read();

        if (dinPendingMessageIndex == 0)  {

            //start a new pending message
            mPendingMessage[0] = extracted;

            //check for running status first (din only)
            if (isChannelMessage(getTypeFromStatusByte(mRunningStatus_RX))) {

                //only channel messages allow Running Status
                //if the status byte is not received, prepend it to the pending message
                if (extracted < 0x80)   {

                    mPendingMessage[0]   = mRunningStatus_RX;
                    mPendingMessage[1]   = extracted;
                    dinPendingMessageIndex = 1;

                }

                //else: well, we received another status byte,
                //so the running status does not apply here
                //it will be updated upon completion of this message

            }

            switch (getTypeFromStatusByte(mPendingMessage[0]))  {

                //1 byte messages
                case midiMessageStart:
                case midiMessageContinue:
                case midiMessageStop:
                case midiMessageClock:
                case midiMessageActiveSensing:
                case midiMessageSystemReset:
                case midiMessageTuneRequest:
                //handle the message type directly here.
                dinMessage.type    = getTypeFromStatusByte(mPendingMessage[0]);
                dinMessage.channel = 0;
                dinMessage.data1   = 0;
                dinMessage.data2   = 0;
                dinMessage.valid   = true;

                // \fix Running Status broken when receiving Clock messages.
                // Do not reset all input attributes, Running Status must remain unchanged.
                //resetInput();

                //we still need to reset these
                dinPendingMessageIndex = 0;
                dinPendingMessageExpectedLenght = 0;

                return true;
                break;

                //2 bytes messages
                case midiMessageProgramChange:
                case midiMessageAfterTouchChannel:
                case midiMessageTimeCodeQuarterFrame:
                case midiMessageSongSelect:
                dinPendingMessageExpectedLenght = 2;
                break;

                //3 bytes messages
                case midiMessageNoteOn:
                case midiMessageNoteOff:
                case midiMessageControlChange:
                case midiMessagePitchBend:
                case midiMessageAfterTouchPoly:
                case midiMessageSongPosition:
                dinPendingMessageExpectedLenght = 3;
                break;

                case midiMessageSystemExclusive:
                //the message can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE
                dinPendingMessageExpectedLenght = MIDI_SYSEX_ARRAY_SIZE;
                mRunningStatus_RX = midiMessageInvalidType;
                dinMessage.sysexArray[0] = midiMessageSystemExclusive;
                break;

                case midiMessageInvalidType:
                default:
                //this is obviously wrong
                //let's get the hell out'a here
                resetInput();
                return false;
                break;

            }

            if (dinPendingMessageIndex >= (dinPendingMessageExpectedLenght - 1))    {

                //reception complete
                dinMessage.type    = getTypeFromStatusByte(mPendingMessage[0]);
                dinMessage.channel = getChannelFromStatusByte(mPendingMessage[0]);
                dinMessage.data1   = mPendingMessage[1];

                //save data2 only if applicable
                if (dinPendingMessageExpectedLenght == 3)
                dinMessage.data2 = mPendingMessage[2];
                else dinMessage.data2 = 0;

                dinPendingMessageIndex = 0;
                dinPendingMessageExpectedLenght = 0;
                dinMessage.valid = true;
                return true;

            }   else {

                //waiting for more data
                dinPendingMessageIndex++;

            }

            if (use1byteParsing)    {

                //message is not complete.
                return false;

            } else {

                //call the parser recursively
                //to parse the rest of the message.
                return parse(dinInterface);

            }

        }   else {

            //first, test if this is a status byte
            if (extracted >= 0x80)  {

                //reception of status bytes in the middle of an uncompleted message
                //are allowed only for interleaved Real Time message or EOX
                switch (extracted)  {

                    case midiMessageClock:
                    case midiMessageStart:
                    case midiMessageContinue:
                    case midiMessageStop:
                    case midiMessageActiveSensing:
                    case midiMessageSystemReset:
                    //here we will have to extract the one-byte message,
                    //pass it to the structure for being read outside
                    //the MIDI class, and recompose the message it was
                    //interleaved into without killing the running status..
                    //this is done by leaving the pending message as is,
                    //it will be completed on next calls
                    dinMessage.type    = (midiMessageType_t)extracted;
                    dinMessage.data1   = 0;
                    dinMessage.data2   = 0;
                    dinMessage.channel = 0;
                    dinMessage.valid   = true;
                    return true;
                    break;

                    //end of sysex
                    case 0xf7:
                    if (dinMessage.sysexArray[0] == midiMessageSystemExclusive)   {

                        //store the last byte (EOX)
                        dinMessage.sysexArray[dinPendingMessageIndex++] = 0xf7;
                        dinMessage.type = midiMessageSystemExclusive;

                        //get length
                        dinMessage.data1   = dinPendingMessageIndex & 0xff; //LSB
                        dinMessage.data2   = dinPendingMessageIndex >> 8;   //MSB
                        dinMessage.channel = 0;
                        dinMessage.valid   = true;

                        resetInput();
                        return true;

                    }   else {

                        //error
                        resetInput();
                        return false;

                    }
                    break;

                    default:
                    break;

                }

            }

            //add extracted data byte to pending message
            if (mPendingMessage[0] == midiMessageSystemExclusive)
                dinMessage.sysexArray[dinPendingMessageIndex] = extracted;
            else mPendingMessage[dinPendingMessageIndex] = extracted;

            //now we are going to check if we have reached the end of the message
            if (dinPendingMessageIndex >= (dinPendingMessageExpectedLenght - 1))    {

                //"FML" case: fall down here with an overflown SysEx..
                //this means we received the last possible data byte that can fit the buffer
                //if this happens, try increasing MIDI_SYSEX_ARRAY_SIZE
                if (mPendingMessage[0] == midiMessageSystemExclusive)   {

                    resetInput();
                    return false;

                }

                dinMessage.type = getTypeFromStatusByte(mPendingMessage[0]);

                if (isChannelMessage(dinMessage.type))
                    dinMessage.channel = getChannelFromStatusByte(mPendingMessage[0]);
                else dinMessage.channel = 0;

                dinMessage.data1 = mPendingMessage[1];

                //save data2 only if applicable
                if (dinPendingMessageExpectedLenght == 3)
                    dinMessage.data2 = mPendingMessage[2];
                else dinMessage.data2 = 0;

                //reset local variables
                dinPendingMessageIndex = 0;
                dinPendingMessageExpectedLenght = 0;

                dinMessage.valid = true;

                //activate running status (if enabled for the received type)
                switch (dinMessage.type)  {

                    case midiMessageNoteOff:
                    case midiMessageNoteOn:
                    case midiMessageAfterTouchPoly:
                    case midiMessageControlChange:
                    case midiMessageProgramChange:
                    case midiMessageAfterTouchChannel:
                    case midiMessagePitchBend:
                    //running status enabled: store it from received message
                    mRunningStatus_RX = mPendingMessage[0];
                    break;

                    default:
                    //no running status
                    mRunningStatus_RX = midiMessageInvalidType;
                    break;

                }

                return true;

            }   else {

                //update the index of the pending message
                dinPendingMessageIndex++;

                if (use1byteParsing)    {

                    //message is not complete.
                    return false;

                }   else {

                    //call the parser recursively to parse the rest of the message.
                    return parse(dinInterface);

                }

            }

        }

    }   else if (type == usbInterface)  {

            MIDI_EventPacket_t MIDIEvent;

            //device must be connected and configured for the task to run
            if (USB_DeviceState != DEVICE_STATE_Configured) return false;

            //select the MIDI OUT stream
            Endpoint_SelectEndpoint(MIDI_STREAM_OUT_EPADDR);

            //check if a MIDI command has been received
            if (Endpoint_IsOUTReceived())   {

                //read the MIDI event packet from the endpoint
                Endpoint_Read_Stream_LE(&MIDIEvent, sizeof(MIDIEvent), NULL);

                //if the endpoint is now empty, clear the bank
                if (!(Endpoint_BytesInEndpoint()))  {

                    //clear the endpoint ready for new packet
                    Endpoint_ClearOUT();

                }

            }   else return false;

            //we already have entire message here
            //MIDIEvent.Event is CIN, see midi10.pdf
            //shift cin four bytes left to get midiMessageType_t
            uint8_t midiMessage = MIDIEvent.Event << 4;

            switch(midiMessage) {

                //1 byte messages
                case sysCommon1byteCin:
                if (MIDIEvent.Data1 != 0xF7)   {

                    //this isn't end of sysex, it's 1byte system common message

                    //case midiMessageClock:
                    //case midiMessageStart:
                    //case midiMessageContinue:
                    //case midiMessageStop:
                    //case midiMessageActiveSensing:
                    //case midiMessageSystemReset:
                    usbMessage.type    = (midiMessageType_t)MIDIEvent.Data1;
                    usbMessage.channel = 0;
                    usbMessage.data1   = 0;
                    usbMessage.data2   = 0;
                    usbMessage.valid   = true;
                    return true;

                }   else {

                    //end of sysex
                    usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data1;
                    sysExArrayLength++;
                    usbMessage.type    = (midiMessageType_t)midiMessageSystemExclusive;
                    usbMessage.channel = 0;
                    usbMessage.valid   = true;
                    return true;

                }
                break;

                //2 byte messages
                case sysCommon2byteCin:
                //case midiMessageProgramChange:
                //case midiMessageAfterTouchChannel:
                //case midiMessageTimeCodeQuarterFrame:
                //case midiMessageSongSelect:
                usbMessage.type    = (midiMessageType_t)MIDIEvent.Data1;
                usbMessage.channel = (MIDIEvent.Data1 & 0x0F) + 1;
                usbMessage.data1   = MIDIEvent.Data2;
                usbMessage.data2   = 0;
                usbMessage.valid   = true;
                return true;
                break;

                //3 byte messages
                case midiMessageNoteOn:
                case midiMessageNoteOff:
                case midiMessageControlChange:
                case midiMessagePitchBend:
                case midiMessageAfterTouchPoly:
                case midiMessageSongPosition:
                usbMessage.type    = (midiMessageType_t)midiMessage;
                usbMessage.channel = (MIDIEvent.Data1 & 0x0F) + 1;
                usbMessage.data1   = MIDIEvent.Data2;
                usbMessage.data2   = MIDIEvent.Data3;
                usbMessage.valid   = true;
                return true;
                break;

                //sysex
                case sysExStartCin:
                //the message can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE
                if (MIDIEvent.Data1 == 0xF0) {

                    //this is a new sysex message, reset length
                    sysExArrayLength = 0;

                }
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data1;
                sysExArrayLength++;
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data2;
                sysExArrayLength++;
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data3;
                sysExArrayLength++;
                return false;
                break;

                case sysExStop2byteCin:
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data1;
                sysExArrayLength++;
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data2;
                sysExArrayLength++;
                usbMessage.type    = midiMessageSystemExclusive;
                usbMessage.channel = 0;
                usbMessage.valid   = true;
                return true;
                break;

                case sysExStop3byteCin:
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data1;
                sysExArrayLength++;
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data2;
                sysExArrayLength++;
                usbMessage.sysexArray[sysExArrayLength] = MIDIEvent.Data3;
                sysExArrayLength++;
                usbMessage.type    = midiMessageSystemExclusive;
                usbMessage.channel = 0;
                usbMessage.valid   = true;
                return true;
                break;

                default:
                return false;
                break;

            }

    }   else return false;

}

bool HWmidi::inputFilter(uint8_t inChannel, midiInterfaceType_t type) {

    //check if the received message is on the listened channel

    switch(type)    {

        case dinInterface:
        if (dinMessage.type == midiMessageInvalidType)
        return false;

        //first, check if the received message is Channel
        if (dinMessage.type >= midiMessageNoteOff && dinMessage.type <= midiMessagePitchBend)   {

            //then we need to know if we listen to it
            if ((dinMessage.channel == inChannel) ||
            (inChannel == MIDI_CHANNEL_OMNI))
            return true;

            else {

                //we don't listen to this channel
                return false;

            }

            }   else {

            //system messages are always received
            return true;

        }
        break;

        case usbInterface:
        if (usbMessage.type == midiMessageInvalidType)
            return false;

        //first, check if the received message is Channel
        if (usbMessage.type >= midiMessageNoteOff && usbMessage.type <= midiMessagePitchBend)   {

            //then we need to know if we listen to it
            if ((usbMessage.channel == inChannel) ||
                (inChannel == MIDI_CHANNEL_OMNI))
            return true;

            else {

                //we don't listen to this channel
                return false;

            }

            }   else {

            //system messages are always received
            return true;

        }
        break;

        default:
        return false;
        break;

    }

}

void HWmidi::resetInput()   {

    //reset input attributes

    dinPendingMessageIndex = 0;
    dinPendingMessageExpectedLenght = 0;
    mRunningStatus_RX = midiMessageInvalidType;

}

midiMessageType_t HWmidi::getType(midiInterfaceType_t type) const {

    //get the last received message's type

    switch(type)    {

        case dinInterface:
        return dinMessage.type;
        break;

        case usbInterface:
        return usbMessage.type;
        break;

    }   return midiMessageInvalidType;

}

uint8_t HWmidi::getChannel(midiInterfaceType_t type) const  {

    //get the channel of the message stored in the structure
    //channel range is 1 to 16
    //For non-channel messages, this will return 0

    switch(type)    {

        case dinInterface:
        return dinMessage.channel;
        break;

        case usbInterface:
        return usbMessage.channel;
        break;

    }   return 0;

}

uint8_t HWmidi::getData1(midiInterfaceType_t type) const    {

    //get the first data byte of the last received message
    switch(type)    {

        case dinInterface:
        return dinMessage.data1;
        break;

        case usbInterface:
        return usbMessage.data1;
        break;

    }   return 0;

}

uint8_t HWmidi::getData2(midiInterfaceType_t type) const    {

    //get the second data byte of the last received message
    //get the first data byte of the last received message
    switch(type)    {

        case dinInterface:
        return dinMessage.data2;
        break;

        case usbInterface:
        return usbMessage.data2;
        break;

    }   return 0;

}

uint8_t* HWmidi::getSysExArray(midiInterfaceType_t type)    {

    //get the System Exclusive byte array
    switch(type)    {

        case dinInterface:
        return dinMessage.sysexArray;
        break;

        case usbInterface:
        return usbMessage.sysexArray;
        break;

    }   return 0;

}

uint16_t HWmidi::getSysExArrayLength(midiInterfaceType_t type)  {

    //get the length of the System Exclusive array
    //it is coded using data1 as LSB and data2 as MSB

    uint16_t size = 0;

    switch(type)    {

        case dinInterface:
        size = unsigned(dinMessage.data2) << 8 | dinMessage.data1;
        break;

        case usbInterface:
        return sysExArrayLength;
        break;

    }

    return size > MIDI_SYSEX_ARRAY_SIZE ? MIDI_SYSEX_ARRAY_SIZE : size;

}

bool HWmidi::check() const  {

    //check if a valid message is stored in the structure
    return dinMessage.valid;

}

uint8_t HWmidi::getInputChannel() const {

    return mInputChannel;

}

void HWmidi::setInputChannel(uint8_t inChannel) {

    //set the value for the input MIDI channel
    //inChannel:    The channel value
                    //Valid values are 1 to 16, MIDI_CHANNEL_OMNI

    mInputChannel = inChannel;

}

midiMessageType_t HWmidi::getTypeFromStatusByte(uint8_t inStatus) {

    //extract an enumerated MIDI type from a status byte

    if ((inStatus  < 0x80) ||
        (inStatus == 0xf4) ||
        (inStatus == 0xf5) ||
        (inStatus == 0xf9) ||
        (inStatus == 0xfD)) {

        //data bytes and undefined
        return midiMessageInvalidType;

    }

    if (inStatus < 0xf0)    {

        //channel message, remove channel nibble
        return midiMessageType_t(inStatus & 0xf0);

    }

    return midiMessageType_t(inStatus);

}

uint8_t HWmidi::getChannelFromStatusByte(uint8_t inStatus)   {

    //returns channel in the range 1-16
    return (inStatus & 0x0f) + 1;

}

bool HWmidi::isChannelMessage(midiMessageType_t inType)   {

    return (inType == midiMessageNoteOff           ||
            inType == midiMessageNoteOn            ||
            inType == midiMessageControlChange     ||
            inType == midiMessageAfterTouchPoly    ||
            inType == midiMessageAfterTouchChannel ||
            inType == midiMessagePitchBend         ||
            inType == midiMessageProgramChange);

}

void HWmidi::setThruFilterMode(midiFilterMode_t inThruFilterMode) {

    //set the filter for thru mirroring
    //inThruFilterMode: A filter mode

    mThruFilterMode = inThruFilterMode;

    if (mThruFilterMode != Off)
        mThruActivated = true;
    else mThruActivated = false;

}

midiFilterMode_t HWmidi::getFilterMode() const    {

    return mThruFilterMode;

}

bool HWmidi::getThruState() const   {

    return mThruActivated;

}

void HWmidi::turnThruOn(midiFilterMode_t inThruFilterMode)    {

    mThruActivated = true;
    mThruFilterMode = inThruFilterMode;

}

void HWmidi::turnThruOff()  {

    mThruActivated = false;
    mThruFilterMode = Off;

}

void HWmidi::thruFilter(uint8_t inChannel, midiInterfaceType_t type)  {

    //this method is called upon reception of a message
    //and takes care of Thru filtering and sending

    //all system messages (System Exclusive, Common and Real Time) are passed
    //to output unless filter is set to Off

    //channel messages are passed to the output if their channel
    //is matching the input channel and the filter setting

    //if the feature is disabled, don't do anything.
    if (!mThruActivated || (mThruFilterMode == Off))
        return;

    //first, check if the received message is Channel
    if (dinMessage.type >= midiMessageNoteOff && dinMessage.type <= midiMessagePitchBend)   {

        const bool filter_condition = ((dinMessage.channel == mInputChannel) ||
                                       (mInputChannel == MIDI_CHANNEL_OMNI));

        //now let's pass it to the output
        switch (mThruFilterMode)    {

            case Full:
            send(dinMessage.type, dinMessage.data1, dinMessage.data2, dinMessage.channel, type);
            break;

            case SameChannel:
            if (filter_condition)
                send(dinMessage.type, dinMessage.data1, dinMessage.data2, dinMessage.channel, type);
            break;

            case DifferentChannel:
            if (!filter_condition)
                send(dinMessage.type, dinMessage.data1, dinMessage.data2, dinMessage.channel, type);
            break;

            case Off:
            //do nothing
            //technically it's impossible to get there because
            //the case was already tested earlier.
            break;

            default:
            break;

        }

    }   else {

        //send the message to the output
        switch (dinMessage.type)  {

            //real Time and 1 byte
            case midiMessageClock:
            case midiMessageStart:
            case midiMessageStop:
            case midiMessageContinue:
            case midiMessageActiveSensing:
            case midiMessageSystemReset:
            case midiMessageTuneRequest:
            sendRealTime(dinMessage.type, type);
            break;

            case midiMessageSystemExclusive:
            //send SysEx (0xf0 and 0xf7 are included in the buffer)
            sendSysEx(getSysExArrayLength(dinInterface), getSysExArray(dinInterface), true, type);
            break;

            case midiMessageSongSelect:
            sendSongSelect(dinMessage.data1, type);
            break;

            case midiMessageSongPosition:
            sendSongPosition(dinMessage.data1 | ((unsigned)dinMessage.data2 << 7), type);
            break;

            case midiMessageTimeCodeQuarterFrame:
            sendTimeCodeQuarterFrame(dinMessage.data1,dinMessage.data2, type);
            break;

            default:
            break;

        }

    }

}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
 */
void EVENT_USB_Device_ConfigurationChanged(void) {

    bool ConfigSuccess = true;

    /* Setup MIDI Data Endpoints */
    ConfigSuccess &= Endpoint_ConfigureEndpoint(MIDI_STREAM_IN_EPADDR, EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(MIDI_STREAM_OUT_EPADDR, EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);

}

HWmidi hwMIDI;
#endif