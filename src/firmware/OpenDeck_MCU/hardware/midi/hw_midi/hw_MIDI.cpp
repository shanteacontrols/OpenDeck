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

//library modifed by Igor Petrovic

#include "hw_MIDI.h"

HWmidi::HWmidi()   {

    //default constructor

}

void HWmidi::init(uint8_t inChannel, bool inputEnabled, bool outputEnabled) {

    USE_SERIAL_PORT.begin(31250, inputEnabled, outputEnabled);

    mInputChannel = inChannel;

    mRunningStatus_TX = midiMessageInvalidType;
    mRunningStatus_RX = midiMessageInvalidType;

    mPendingMessageIndex            = 0;
    mPendingMessageExpectedLenght   = 0;

    mMessage.valid      = false;
    mMessage.type       = midiMessageInvalidType;
    mMessage.channel    = 0;
    mMessage.data1      = 0;
    mMessage.data2      = 0;

    mThruFilterMode     = Off;
    mThruActivated      = false;
    useRunningStatus    = false;
    use1byteParsing     = true;

}

void HWmidi::send(midiMessageType inType, uint8_t inData1, uint8_t inData2, uint8_t inChannel)  {

    //inType:       MIDI message type
    //inData1:      The first data byte
    //inData2:      The second data byte (if the message contains only 1 data byte, set this one to 0)
    //inChannel:    The output channel on which the message will be sent (values from 1 to 16)

    //test if channel is valid
    if (inChannel >= MIDI_CHANNEL_OFF  ||
        inChannel == MIDI_CHANNEL_OMNI ||
        inType < 0x80)  {

        if (useRunningStatus)
            mRunningStatus_TX = midiMessageInvalidType;

        return; //don't send anything

    }

    if (inType <= midiMessagePitchBend) {

        //channel messages

        //protection: remove MSBs on data
        inData1 &= 0x7f;
        inData2 &= 0x7f;

        const uint8_t status = getStatus(inType, inChannel);

        if (useRunningStatus)   {

            if (mRunningStatus_TX != status)    {

                //new message, memorize and send header
                mRunningStatus_TX = status;
                USE_SERIAL_PORT.write(mRunningStatus_TX);

            }

        }   else {

            //don't care about running status, send the status byte
            USE_SERIAL_PORT.write(status);

        }

        //send data
        USE_SERIAL_PORT.write(inData1);

        if (inType != midiMessageProgramChange && inType != midiMessageAfterTouchChannel)
            USE_SERIAL_PORT.write(inData2);

    }   else if (inType >= midiMessageTuneRequest && inType <= midiMessageSystemReset)
            sendRealTime(inType); //system real-time and 1 byte

}

void HWmidi::sendNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel)    {

    //inNoteNumber:   Pitch value in the MIDI format (0 to 127)
    //inVelocity:     Note attack velocity (0 to 127)
    //inChannel:      The channel on which the message will be sent (1 to 16).

    send(midiMessageNoteOn, inNoteNumber, inVelocity, inChannel);

}

void HWmidi::sendNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel)   {

    //inNoteNumber:    Pitch value in the MIDI format (0 to 127)
    //inVelocity:      Release velocity (0 to 127)
    //inChannel:       The channel on which the message will be sent (1 to 16)

    send(midiMessageNoteOff, inNoteNumber, inVelocity, inChannel);

}

void HWmidi::sendProgramChange(uint8_t inProgramNumber, uint8_t inChannel)  {

    //inProgramNumber:    The Program to select (0 to 127)
    //inChannel:          The channel on which the message will be sent (1 to 16)

    send(midiMessageProgramChange, inProgramNumber, 0, inChannel);

}

void HWmidi::sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel)  {

    //inControlNumber:    The controller number (0 to 127)
    //inControlValue:     The value for the specified controller (0 to 127)
    //inChannel:          The channel on which the message will be sent (1 to 16)

    send(midiMessageControlChange, inControlNumber, inControlValue, inChannel);

}

void HWmidi::sendPolyPressure(uint8_t inNoteNumber, uint8_t inPressure, uint8_t inChannel)  {

     //inNoteNumber:    The note to apply AfterTouch to (0 to 127)
     //inPressure:      The amount of AfterTouch to apply (0 to 127)
     //inChannel:       The channel on which the message will be sent (1 to 16)

    send(midiMessageAfterTouchPoly, inNoteNumber, inPressure, inChannel);

}

void HWmidi::sendAfterTouch(uint8_t inPressure, uint8_t inChannel)  {

     //inPressure:  The amount of AfterTouch to apply to all notes
     //inChannel:   The channel on which the message will be sent (1 to 16)

    send(midiMessageAfterTouchChannel, inPressure, 0, inChannel);

}

void HWmidi::sendPitchBend(int inPitchValue, uint8_t inChannel) {

    //inPitchValue: The amount of bend to send (in a signed integer format),
                    //between MIDI_PITCHBEND_MIN and MIDI_PITCHBEND_MAX,
                    //center value is 0
    //inChannel:    The channel on which the message will be sent (1 to 16)

    const unsigned bend = inPitchValue - MIDI_PITCHBEND_MIN;
    send(midiMessagePitchBend, (bend & 0x7f), (bend >> 7) & 0x7f, inChannel);

}

void HWmidi::sendSysEx(uint16_t inLength, const uint8_t* inArray, bool inArrayContainsBoundaries)   {

     //inLength:                    The size of the array to send
     //inArray:                     The byte array containing the data to send
     //inArrayContainsBoundaries:   When set to 'true', 0xf0 & 0xf7 bytes
                                    //(start & stop SysEx) will NOT be sent
                                    //(and therefore must be included in the array)

    if (!inArrayContainsBoundaries)
        USE_SERIAL_PORT.write(0xf0);

    for (unsigned i = 0; i < inLength; ++i)
        USE_SERIAL_PORT.write(inArray[i]);

    if (!inArrayContainsBoundaries)
        USE_SERIAL_PORT.write(0xf7);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendTuneRequest()  {

    //when a MIDI unit receives this message,
    //it should tune its oscillators (if equipped with any)

    sendRealTime(midiMessageTuneRequest);

}

void HWmidi::sendTimeCodeQuarterFrame(uint8_t inTypeNibble, uint8_t inValuesNibble) {

     //inTypeNibble     MTC type
     //inValuesNibble   MTC data

    const uint8_t data = (((inTypeNibble & 0x07) << 4) | (inValuesNibble & 0x0f));
    sendTimeCodeQuarterFrame(data);

}

void HWmidi::sendTimeCodeQuarterFrame(uint8_t inData)   {

    //inData:   if you want to encode directly the nibbles in your program,
                //you can send the byte here.

    USE_SERIAL_PORT.write((uint8_t)midiMessageTimeCodeQuarterFrame);
    USE_SERIAL_PORT.write(inData);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendSongPosition(uint16_t inBeats) {

    //inBeats:  The number of beats since the start of the song

    USE_SERIAL_PORT.write((uint8_t)midiMessageSongPosition);
    USE_SERIAL_PORT.write(inBeats & 0x7f);
    USE_SERIAL_PORT.write((inBeats >> 7) & 0x7f);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendSongSelect(uint8_t inSongNumber)   {

    //inSongNumber: Wanted song number

    USE_SERIAL_PORT.write((uint8_t)midiMessageSongSelect);
    USE_SERIAL_PORT.write(inSongNumber & 0x7f);

    if (useRunningStatus)
        mRunningStatus_TX = midiMessageInvalidType;

}

void HWmidi::sendRealTime(midiMessageType inType)   {

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

uint8_t HWmidi::getStatus(midiMessageType inType, uint8_t inChannel) const  {

    return ((uint8_t)inType | ((inChannel - 1) & 0x0f));

}

bool HWmidi::read() {

    //returns true if a valid message has been stored in the structure, false if not
    //a valid message is a message that matches the input channel
    //if the Thru is enabled and the message matches the filter,
    //it is sent back on the MIDI output

    return read(mInputChannel);

}

bool HWmidi::read(uint8_t inChannel)    {

    if (inChannel >= MIDI_CHANNEL_OFF)
        return false; //MIDI Input disabled

    if (!parse()) return false;

    const bool channelMatch = inputFilter(inChannel);

    thruFilter(inChannel);

    return channelMatch;

}

bool HWmidi::parse()    {

    if (USE_SERIAL_PORT.available() == 0)
        //no data available
        return false;

    //parsing algorithm:
    //get a byte from the serial buffer
    //if there is no pending message to be recomposed, start a new one
    //find type and channel (if pertinent)
    //look for other bytes in buffer, call parser recursively,
    //until the message is assembled or the buffer is empty
    //else, add the extracted byte to the pending message, and check validity
    //when the message is done, store it

    const uint8_t extracted = USE_SERIAL_PORT.read();

    if (mPendingMessageIndex == 0)  {

        //start a new pending message
        mPendingMessage[0] = extracted;

        //check for running status first
        if (isChannelMessage(getTypeFromStatusByte(mRunningStatus_RX))) {

            //only channel messages allow Running Status
            //if the status byte is not received, prepend it to the pending message
            if (extracted < 0x80)   {

                mPendingMessage[0]   = mRunningStatus_RX;
                mPendingMessage[1]   = extracted;
                mPendingMessageIndex = 1;

            }

            //else: well, we received another status byte,
            //so the running status does not apply here
            //it will be updated upon completion of this message.
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
            mMessage.type    = getTypeFromStatusByte(mPendingMessage[0]);
            mMessage.channel = 0;
            mMessage.data1   = 0;
            mMessage.data2   = 0;
            mMessage.valid   = true;

            // \fix Running Status broken when receiving Clock messages.
            // Do not reset all input attributes, Running Status must remain unchanged.
            //resetInput();

            //we still need to reset these
            mPendingMessageIndex = 0;
            mPendingMessageExpectedLenght = 0;

            return true;
            break;

            //2 bytes messages
            case midiMessageProgramChange:
            case midiMessageAfterTouchChannel:
            case midiMessageTimeCodeQuarterFrame:
            case midiMessageSongSelect:
            mPendingMessageExpectedLenght = 2;
            break;

            //3 bytes messages
            case midiMessageNoteOn:
            case midiMessageNoteOff:
            case midiMessageControlChange:
            case midiMessagePitchBend:
            case midiMessageAfterTouchPoly:
            case midiMessageSongPosition:
            mPendingMessageExpectedLenght = 3;
            break;

            case midiMessageSystemExclusive:
            //the message can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE
            mPendingMessageExpectedLenght = MIDI_SYSEX_ARRAY_SIZE;
            mRunningStatus_RX = midiMessageInvalidType;
            mMessage.sysexArray[0] = midiMessageSystemExclusive;
            break;

            case midiMessageInvalidType:
            default:
            //this is obviously wrong
            //let's get the hell out'a here
            resetInput();
            return false;
            break;

        }

        if (mPendingMessageIndex >= (mPendingMessageExpectedLenght - 1))    {

            //reception complete
            mMessage.type    = getTypeFromStatusByte(mPendingMessage[0]);
            mMessage.channel = getChannelFromStatusByte(mPendingMessage[0]);
            mMessage.data1   = mPendingMessage[1];

            //save data2 only if applicable
            if (mPendingMessageExpectedLenght == 3)
                mMessage.data2 = mPendingMessage[2];
            else mMessage.data2 = 0;

            mPendingMessageIndex = 0;
            mPendingMessageExpectedLenght = 0;
            mMessage.valid = true;
            return true;

        }   else {

            //waiting for more data
            mPendingMessageIndex++;

        }

        if (use1byteParsing)    {

            //message is not complete.
            return false;

        } else {

            //call the parser recursively
            //to parse the rest of the message.
            return parse();

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
                mMessage.type    = (midiMessageType)extracted;
                mMessage.data1   = 0;
                mMessage.data2   = 0;
                mMessage.channel = 0;
                mMessage.valid   = true;
                return true;
                break;

                //end of sysex
                case 0xf7:
                if (mMessage.sysexArray[0] == midiMessageSystemExclusive)   {

                    //store the last byte (EOX)
                    mMessage.sysexArray[mPendingMessageIndex++] = 0xf7;
                    mMessage.type = midiMessageSystemExclusive;

                    //get length
                    mMessage.data1   = mPendingMessageIndex & 0xff; //LSB
                    mMessage.data2   = mPendingMessageIndex >> 8;   //MSB
                    mMessage.channel = 0;
                    mMessage.valid   = true;

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
            mMessage.sysexArray[mPendingMessageIndex] = extracted;
        else mPendingMessage[mPendingMessageIndex] = extracted;

        //now we are going to check if we have reached the end of the message
        if (mPendingMessageIndex >= (mPendingMessageExpectedLenght - 1))    {

            //"FML" case: fall down here with an overflown SysEx..
            //this means we received the last possible data byte that can fit the buffer
            //if this happens, try increasing MIDI_SYSEX_ARRAY_SIZE
            if (mPendingMessage[0] == midiMessageSystemExclusive)   {

                resetInput();
                return false;

            }

            mMessage.type = getTypeFromStatusByte(mPendingMessage[0]);

            if (isChannelMessage(mMessage.type))
                mMessage.channel = getChannelFromStatusByte(mPendingMessage[0]);
            else mMessage.channel = 0;

            mMessage.data1 = mPendingMessage[1];

            //save data2 only if applicable
            if (mPendingMessageExpectedLenght == 3)
                mMessage.data2 = mPendingMessage[2];
            else
                mMessage.data2 = 0;

            //reset local variables
            mPendingMessageIndex = 0;
            mPendingMessageExpectedLenght = 0;

            mMessage.valid = true;

            //activate running status (if enabled for the received type)
            switch (mMessage.type)  {

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
            mPendingMessageIndex++;

            if (use1byteParsing)    {

                //message is not complete.
                return false;

            }   else {

                //call the parser recursively to parse the rest of the message.
                return parse();

            }

        }

    }

}

bool HWmidi::inputFilter(uint8_t inChannel) {

    //check if the received message is on the listened channel

    if (mMessage.type == midiMessageInvalidType)
        return false;

    //first, check if the received message is Channel
    if (mMessage.type >= midiMessageNoteOff && mMessage.type <= midiMessagePitchBend)   {

        //then we need to know if we listen to it
        if ((mMessage.channel == inChannel) ||
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

}

void HWmidi::resetInput()   {

    //reset input attributes

    mPendingMessageIndex = 0;
    mPendingMessageExpectedLenght = 0;
    mRunningStatus_RX = midiMessageInvalidType;

}

midiMessageType HWmidi::getType() const {

    //get the last received message's type
    return mMessage.type;

}

uint8_t HWmidi::getChannel() const  {

    //get the channel of the message stored in the structure
    //channel range is 1 to 16
    //For non-channel messages, this will return 0

    return mMessage.channel;

}

uint8_t HWmidi::getData1() const    {

    //get the first data byte of the last received message
    return mMessage.data1;

}

uint8_t HWmidi::getData2() const    {

    //get the second data byte of the last received message
    return mMessage.data2;

}

const uint8_t* HWmidi::getSysExArray() const    {

    //get the System Exclusive byte array
    return mMessage.sysexArray;

}

uint16_t HWmidi::getSysExArrayLength() const    {

    //get the length of the System Exclusive array
    //it is coded using data1 as LSB and data2 as MSB

    const unsigned size = unsigned(mMessage.data2) << 8 | mMessage.data1;
    return size > MIDI_SYSEX_ARRAY_SIZE ? MIDI_SYSEX_ARRAY_SIZE : size;

}

bool HWmidi::check() const  {

    //check if a valid message is stored in the structure
    return mMessage.valid;

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

midiMessageType HWmidi::getTypeFromStatusByte(uint8_t inStatus) {

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
        return midiMessageType(inStatus & 0xf0);

    }

    return midiMessageType(inStatus);

}

uint8_t HWmidi::getChannelFromStatusByte(uint8_t inStatus)   {

    //returns channel in the range 1-16
    return (inStatus & 0x0f) + 1;

}

bool HWmidi::isChannelMessage(midiMessageType inType)   {

    return (inType == midiMessageNoteOff           ||
            inType == midiMessageNoteOn            ||
            inType == midiMessageControlChange     ||
            inType == midiMessageAfterTouchPoly    ||
            inType == midiMessageAfterTouchChannel ||
            inType == midiMessagePitchBend         ||
            inType == midiMessageProgramChange);

}

void HWmidi::setThruFilterMode(MidiFilterMode inThruFilterMode) {

    //set the filter for thru mirroring
    //inThruFilterMode: A filter mode

    mThruFilterMode = inThruFilterMode;

    if (mThruFilterMode != Off)
        mThruActivated = true;
    else mThruActivated = false;

}

MidiFilterMode HWmidi::getFilterMode() const    {

    return mThruFilterMode;

}

bool HWmidi::getThruState() const   {

    return mThruActivated;

}

void HWmidi::turnThruOn(MidiFilterMode inThruFilterMode)    {

    mThruActivated = true;
    mThruFilterMode = inThruFilterMode;

}

void HWmidi::turnThruOff()  {

    mThruActivated = false;
    mThruFilterMode = Off;

}

void HWmidi::thruFilter(uint8_t inChannel)  {

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
    if (mMessage.type >= midiMessageNoteOff && mMessage.type <= midiMessagePitchBend)   {

        const bool filter_condition = ((mMessage.channel == mInputChannel) ||
                                       (mInputChannel == MIDI_CHANNEL_OMNI));

        //now let's pass it to the output
        switch (mThruFilterMode)    {

            case Full:
            send(mMessage.type, mMessage.data1, mMessage.data2, mMessage.channel);
            break;

            case SameChannel:
            if (filter_condition)
                send(mMessage.type, mMessage.data1, mMessage.data2, mMessage.channel);
            break;

            case DifferentChannel:
            if (!filter_condition)
                send(mMessage.type, mMessage.data1, mMessage.data2, mMessage.channel);
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
        switch (mMessage.type)  {

            //real Time and 1 byte
            case midiMessageClock:
            case midiMessageStart:
            case midiMessageStop:
            case midiMessageContinue:
            case midiMessageActiveSensing:
            case midiMessageSystemReset:
            case midiMessageTuneRequest:
            sendRealTime(mMessage.type);
            break;

            case midiMessageSystemExclusive:
            //send SysEx (0xf0 and 0xf7 are included in the buffer)
            sendSysEx(getSysExArrayLength(), getSysExArray(), true);
            break;

            case midiMessageSongSelect:
            sendSongSelect(mMessage.data1);
            break;

            case midiMessageSongPosition:
            sendSongPosition(mMessage.data1 | ((unsigned)mMessage.data2 << 7));
            break;

            case midiMessageTimeCodeQuarterFrame:
            sendTimeCodeQuarterFrame(mMessage.data1,mMessage.data2);
            break;

            default:
            break;

        }

    }

}

HWmidi hwMIDI;