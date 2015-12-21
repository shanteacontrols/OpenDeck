/*!
 *  @file       MIDI.cpp
 *  Project     MIDI Library
 *  @brief      MIDI Library for the Arduino
 *  @version    3.2
 *  @author     Francois Best 
 *  @date       24/02/11
 *  license     GPL Forty Seven Effects - 2011
 */

/*
    Modified by Igor Petrovic
*/

#include "HWmidi.h"
#include <stdlib.h>

MIDI_Class::MIDI_Class()    { 

    //default constructor

}

void MIDI_Class::begin(uint8_t inChannel)    {

    #if COMPILE_MIDI_IN

    mInputChannel = inChannel;
    mRunningStatus_RX = midiMessageInvalidType;
    mPendingMessageIndex = 0;
    mPendingMessageExpectedLenght = 0;

    mMessage.valid = false;
    mMessage.type = midiMessageInvalidType;
    mMessage.channel = 0;
    mMessage.data1 = 0;
    mMessage.data2 = 0;

    #endif

    #if COMPILE_MIDI_OUT

    disableRunningStatus();

    #endif

    USE_SERIAL_PORT.begin(31250, true, true);

}

#if COMPILE_MIDI_OUT

void MIDI_Class::enableRunningStatus()  {

    setRunningStatusState(true);

}

void MIDI_Class::disableRunningStatus() {

    setRunningStatusState(false);

}

void MIDI_Class::setRunningStatusState(bool state)  {

    mRunningStatus_TX = midiMessageInvalidType;
    runningStatusEnabled = state;

}

//private method for generating a status byte from channel and type
const uint8_t MIDI_Class::genstatus(const midiMessageType inType, const uint8_t inChannel) const    {

    return ((uint8_t)inType | ((inChannel-1) & MAX_MIDI_CHANNEL_MASK));

}


//internal method, use it only if you need to send raw data from your code, at your own risks
void MIDI_Class::send(midiMessageType type, uint8_t data1, uint8_t data2, uint8_t channel) {

    //type                      MIDI message type
    //data1                     The first data byte
    //data2                     The second data byte (if the message contains only 1 data byte, set this one to 0)
    //channel                   The output channel on which the message will be sent (values from 1 to 16)

    if (runningStatusEnabled)   {

        //reset running status
        mRunningStatus_TX = midiMessageInvalidType;

    }

    //test if channel is valid
    if (channel >= MIDI_CHANNEL_OFF || channel == MIDI_CHANNEL_OMNI || type < midiMessageNoteOff)
        return; //Don't send anything

    if (type <= midiMessagePitchBend)  {

        //channel messages
        //protection: remove MSBs on data
        data1 &= MAX_MIDI_VALUE_MASK;
        data2 &= MAX_MIDI_VALUE_MASK;

        uint8_t statusByte = genstatus(type, channel);

        if (runningStatusEnabled)   {

            if (mRunningStatus_TX != statusByte)    {

                //new message, memorize and send header
                mRunningStatus_TX = statusByte;
                USE_SERIAL_PORT.write(mRunningStatus_TX);

            }

        } else

        //don't care about running status, send the control byte
        USE_SERIAL_PORT.write(statusByte);

        //send data
        USE_SERIAL_PORT.write(data1);

        if (type != midiMessageProgramChange && type != midiMessageAfterTouchChannel)
            USE_SERIAL_PORT.write(data2);

        return;

    }

}

void MIDI_Class::sendNoteOn(uint8_t NoteNumber, uint8_t Velocity, uint8_t Channel)   {

    //NoteNumber                Pitch value in the MIDI format (0 to 127)
    //Velocity                  Note attack velocity (0 to 127)
    //Channel                   The channel on which the message will be sent (1 to 16).

    send(midiMessageNoteOn, NoteNumber, Velocity, Channel);

}

void MIDI_Class::sendNoteOff(uint8_t NoteNumber, uint8_t Velocity, uint8_t Channel)  {

    //NoteNumber                Pitch value in the MIDI format (0 to 127)
    //Velocity                  Release velocity (0 to 127)
    //Channel                   The channel on which the message will be sent (1 to 16)

    send(midiMessageNoteOff, NoteNumber, Velocity, Channel);

}

void MIDI_Class::sendControlChange(uint8_t ControlNumber, uint8_t ControlValue, uint8_t Channel)    {

    //ControlNumber             The controller number (0 to 127)
    //ControlValue              The value for the specified controller (0 to 127)
    //Channel                   The channel on which the message will be sent (1 to 16)
    send(midiMessageControlChange, ControlNumber, ControlValue, Channel);

}

void MIDI_Class::sendProgramChange(uint8_t ProgramNumber, uint8_t Channel)    {

    //ProgramNumber             The Program to select (0 to 127)
    //Channel                   The channel on which the message will be sent (1 to 16)

    send(midiMessageProgramChange, ProgramNumber, 0, Channel);

}

void MIDI_Class::sendPitchBend(uint16_t PitchValue, uint8_t Channel)    {

    //PitchValue                The amount of bend to send, between 0 and 16383 (center value is 8192)
    //Channel                   The channel on which the message will be sent (1 to 16)

    send(midiMessagePitchBend, (PitchValue & 0x7F), (PitchValue >> 7) & 0x7F, Channel);

}

void MIDI_Class::sendSysEx(int length, const uint8_t *const array, bool ArrayContainsBoundaries)    {

    //length                    The size of the array to send
    //array                     The byte array containing the data to send
    //ArrayContainsBoundaries   When set to 'true', 0xF0 & 0xF7 bytes (start & stop SysEx) will NOT be sent (and therefore must be included in the array)

    if (ArrayContainsBoundaries == false) {

        USE_SERIAL_PORT.write(0xF0);

        for (int i=0; i<length; ++i)    USE_SERIAL_PORT.write(array[i]);

        USE_SERIAL_PORT.write(0xF7);

    }   else    for (int i=0; i<length; ++i)  USE_SERIAL_PORT.write(array[i]);

    if (runningStatusEnabled)
        mRunningStatus_TX = midiMessageInvalidType;

}

#endif

#if COMPILE_MIDI_IN

bool MIDI_Class::read() {

    //return true if any valid message has been stored in the structure
    //a valid message is a message that matches the input channel

    int16_t bytes_available = USE_SERIAL_PORT.available();

    if (bytes_available <= 0) return false;

    return read(mInputChannel);

}

bool MIDI_Class::read(const uint8_t inChannel) {

    //the same as read() with a given input channel to read on

    if (inChannel >= MIDI_CHANNEL_OFF) return false; //MIDI Input disabled

    if (parse(inChannel)) {

        if (input_filter(inChannel)) {

                return true;

        }

    }

    return false;

}

bool MIDI_Class::parse(uint8_t inChannel)  {

    /* Parsing algorithm:
        Get a byte from the serial buffer.
        If there is no pending message to be recomposed, start a new one.
        Find type and channel (if pertinent)
        Look for other bytes in buffer, call parser recursively, until the message is assembled or the buffer is empty.
        Else, add the extracted byte to the pending message, and check validity. When the message is done, store it.
    */

    const uint8_t extracted = USE_SERIAL_PORT.read();

    if (mPendingMessageIndex == 0)  {

        //start a new pending message

        mPendingMessage[0] = extracted;

        //check for running status first
        switch (getTypeFromStatusByte(mRunningStatus_RX))   {

            //only these types allow Running Status:
            case midiMessageNoteOff:
            case midiMessageNoteOn:
            case midiMessageControlChange:
            case midiMessagePitchBend:
            case midiMessageProgramChange:

            //if the status byte is not received, prepend it to the pending message
            if (extracted < 0x80) {

                mPendingMessage[0] = mRunningStatus_RX;
                mPendingMessage[1] = extracted;
                mPendingMessageIndex = 1;

            }

            //we received another status byte, so the running status does not apply here
            //it will be updated upon completion of this message
            break;

            default:
            //no running status
            break;

        }

        switch (getTypeFromStatusByte(mPendingMessage[0])) {

            //2 bytes messages
            case midiMessageProgramChange:
            mPendingMessageExpectedLenght = 2;
            break;

            //3 bytes messages
            case midiMessageNoteOn:
            case midiMessageNoteOff:
            case midiMessageControlChange:
            case midiMessagePitchBend:
            mPendingMessageExpectedLenght = 3;
            break;

            case midiMessageSystemExclusive:
            //sysex can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE bytes
            mPendingMessageExpectedLenght = MIDI_SYSEX_ARRAY_SIZE;
            mRunningStatus_RX = midiMessageInvalidType;
            break;

            case midiMessageInvalidType:
            default:
            //This is obviously wrong. Let's get the hell out'a here.
            reset_input_attributes();
            return false;
            break;

        }

        //update the index of the pending message.
        mPendingMessageIndex++;

        #if USE_1BYTE_PARSING
        //message is not complete
        return false;
        #else
        //call the parser recursively to parse the rest of the message
        return parse(inChannel);
        #endif

    }

    //test if this is a status byte
    if (extracted >= 0x80) {

        //reception of status bytes in the middle of an uncompleted message
        //are allowed only for interleaved Real Time message or EOX
        switch (extracted) {

            //end of sysex
            case 0xF7:
            if (getTypeFromStatusByte(mPendingMessage[0]) == midiMessageSystemExclusive)   {

                //store sysex array in midimsg structure
                for (uint8_t i=0; i<MIDI_SYSEX_ARRAY_SIZE; i++)  mMessage.sysex_array[i] = mPendingMessage[i];

                mMessage.type = midiMessageSystemExclusive;

                //get length
                mMessage.data1 = (mPendingMessageIndex+1) & 0xFF;
                mMessage.data2 = (mPendingMessageIndex+1) >> 8;
                mMessage.channel = 0;
                mMessage.valid = true;
                reset_input_attributes();

                return true;

            }   else {

                //error
                reset_input_attributes();
                return false;

            }

            break;

            default:
            break;

        }

    }

    //add extracted data byte to pending message
    mPendingMessage[mPendingMessageIndex] = extracted;

    //now we are going to check if we have reached the end of the message
    if (mPendingMessageIndex >= (mPendingMessageExpectedLenght-1)) {

        //"FML" case: fall down here with an overflown SysEx..
        //This means we received the last possible data byte that can fit the buffer.
        //If this happens, try increasing MIDI_SYSEX_ARRAY_SIZE.
        if (getTypeFromStatusByte(mPendingMessage[0]) == midiMessageSystemExclusive) {

            reset_input_attributes();
            return false;

        }

        mMessage.type = getTypeFromStatusByte(mPendingMessage[0]);
        //don't check if it is a channel message
        mMessage.channel = (mPendingMessage[0] & MAX_MIDI_CHANNEL_MASK)+1;
        mMessage.data1 = mPendingMessage[1];

        //save data2 only if applicable
        if (mPendingMessageExpectedLenght == 3) mMessage.data2 = mPendingMessage[2];
        else mMessage.data2 = 0;

        //reset local variables
        mPendingMessageIndex = 0;
        mPendingMessageExpectedLenght = 0;
        mMessage.valid = true;

        //activate running status (if enabled for the received type)
        switch (mMessage.type) {

            case midiMessageNoteOff:
            case midiMessageNoteOn:
            case midiMessageControlChange:
            case midiMessagePitchBend:
            case midiMessageProgramChange:
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

        #if USE_1BYTE_PARSING
        //message is not complete
        return false;
        #else
        //call the parser recursively to parse the rest of the message
        return parse(inChannel);
        #endif

    }

    //what are our chances to fall here?
    return false;

}

bool MIDI_Class::input_filter(uint8_t inChannel)   {

    //check if message channel matches the one specified in begin() method

    if (mMessage.type == midiMessageInvalidType) return false;

    //check if the received message is Channel
    if (mMessage.type >= midiMessageNoteOff && mMessage.type <= midiMessagePitchBend) {

        //then we need to know if we listen to it
        if ((mMessage.channel == mInputChannel) || (mInputChannel == MIDI_CHANNEL_OMNI))    return true;
        else    return false; //we don't listen to this channel

    }   else {

            //System messages are always received
            return true;

    }

}

void MIDI_Class::reset_input_attributes()   {

    mPendingMessageIndex = 0;
    mPendingMessageExpectedLenght = 0;
    mRunningStatus_RX = midiMessageInvalidType;

}

midiMessageType MIDI_Class::getType() const   {

    //get the last received message's type
    return mMessage.type;

}

uint8_t MIDI_Class::getChannel() const {

    //get the channel of the message stored in the structure
    //channel range is 1 to 16
    //for non-channel messages, this will return 0
    return mMessage.channel;

}

uint8_t MIDI_Class::getData1() const   {

    //get the first data byte of the last received message
    return mMessage.data1;

}

uint8_t MIDI_Class::getData2() const   { 

    //get the second data byte of the last received message
    return mMessage.data2;

}

uint8_t* MIDI_Class::getSysExArray()  { 

    //get the System Exclusive byte array
    return mMessage.sysex_array;

}

int16_t MIDI_Class::getSysExArrayLength()  {

    //get the length of the System Exclusive array in bytes
    //length is coded using data1 as LSB and data2 as MSB

    uint16_t coded_size = ((unsigned int)(mMessage.data2) << 8) | mMessage.data1;

    return (coded_size > MIDI_SYSEX_ARRAY_SIZE) ? MIDI_SYSEX_ARRAY_SIZE : coded_size;

}

bool MIDI_Class::check() const  { 

    //check if a valid message is stored in the structure
    return mMessage.valid;

}

void MIDI_Class::setInputChannel(const uint8_t Channel)    { 

    //set the value for the input MIDI channel (1-16)
    //set to MIDI_CHANNEL_OMNI if you want to listen to all channels, and MIDI_CHANNEL_OFF to disable MIDI input
    mInputChannel = Channel;

}

MIDI_Class hwMIDI;

#endif