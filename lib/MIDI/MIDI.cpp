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
    Last revision date: 2014-12-25

    This version of library is stripped of following features:

    *THRU mirroring
    *Polyphonic AfterTouch
    *Program Change
    *Channel (monophonic) AfterTouch
    *System Common - MIDI Time Code Quarter Frame
    *System Common - Song Position Pointer
    *System Common - Song Select
    *System Common - Tune Request
    *System Real Time - Active Sensing
    *System Real Time - System Reset

    In addition to that, MIDI.begin is changed to accept two parameters: input channel (uint8_t
    and boolean parameter used to determine whether running status is enabled or disabled. begin
    function does not automatically open serial port anymore, do that in main program. Code
    formatting is changed to match coding style of OpenDeck library.

*/


#include "MIDI.h"
#include <stdlib.h>
#include "Ownduino.h"

/*! \brief Main instance (the class comes pre-instantiated). */
MIDI_Class MIDI;


/*! \brief Default constructor for MIDI_Class. */
MIDI_Class::MIDI_Class()    { 

#if USE_CALLBACKS

    //Initialize callbacks to NULL pointer
    mNoteOffCallback            = NULL;
    mNoteOnCallback             = NULL;
    mControlChangeCallback      = NULL;
    mProgramChangeCallback      = NULL;
    mPitchBendCallback          = NULL;
    mSystemExclusiveCallback    = NULL;
    mClockCallback              = NULL;
    mStartCallback              = NULL;
    mContinueCallback           = NULL;
    mStopCallback               = NULL;

#endif

}


/*! \brief Call the begin method in the setup() function of the Arduino.
 */
void MIDI_Class::begin(uint8_t inChannel, bool _runningStatusEn)    {

#if COMPILE_MIDI_OUT

    runningStatusEn = _runningStatusEn;

    if (runningStatusEn)
        mRunningStatus_TX = InvalidType;

#endif

#if COMPILE_MIDI_IN

    mInputChannel = inChannel;
    mRunningStatus_RX = InvalidType;
    mPendingMessageIndex = 0;
    mPendingMessageExpectedLenght = 0;

    mMessage.valid = false;
    mMessage.type = InvalidType;
    mMessage.channel = 0;
    mMessage.data1 = 0;
    mMessage.data2 = 0;

#endif

}


#if COMPILE_MIDI_OUT

// Private method for generating a status byte from channel and type
const byte MIDI_Class::genstatus(const kMIDIType inType,
                                 const byte inChannel) const    {

    return ((byte)inType | ((inChannel-1) & 0x0F));

}


/*! \brief Generate and send a MIDI message from the values given.
 \param type    The message type (see type defines for reference)
 \param data1   The first data byte.
 \param data2   The second data byte (if the message contains only 1 data byte, set this one to 0).
 \param channel The output channel on which the message will be sent (values from 1 to 16). Note: you cannot send to OMNI.
 
 This is an internal method, use it only if you need to send raw data from your code, at your own risks.
 */
void MIDI_Class::send(kMIDIType type,
                      byte data1,
                      byte data2,
                      byte channel) {

    //Then test if channel is valid
    if (channel >= MIDI_CHANNEL_OFF || channel == MIDI_CHANNEL_OMNI || type < NoteOff) {

    if (runningStatusEn)
        mRunningStatus_TX = InvalidType;

        return; //Don't send anything
    }

    if (type <= PitchBend)  {

        //Channel messages
        //Protection: remove MSBs on data
        data1 &= 0x7F;
        data2 &= 0x7F;

        byte statusbyte = genstatus(type,channel);

        if (runningStatusEn)    {

            //Check Running Status
            if (mRunningStatus_TX != statusbyte) {

                //New message, memorize and send header
                mRunningStatus_TX = statusbyte;
                USE_SERIAL_PORT.write(mRunningStatus_TX);

            }

        }   else
        //Don't care about running status, send the Control byte.
        USE_SERIAL_PORT.write(statusbyte);

        // Then send data
        USE_SERIAL_PORT.write(data1);
        
        if (type != ProgramChange && type != 0xD0)
            USE_SERIAL_PORT.write(data2);

        return;
    }

    if (type >= 0xF6 && type <= 0xFF) 
        //System Real-time and 1 byte.
        sendRealTime(type);

}


/*! \brief Send a Note On message 
 \param NoteNumber  Pitch value in the MIDI format (0 to 127). Take a look at the values, names and frequencies of notes here: http://www.phys.unsw.edu.au/jw/notes.html\n
 \param Velocity    Note attack velocity (0 to 127). A NoteOn with 0 velocity is considered as a NoteOff.
 \param Channel     The channel on which the message will be sent (1 to 16). 
 */
void MIDI_Class::sendNoteOn(byte NoteNumber,
                            byte Velocity,
                            byte Channel)   {

    send(NoteOn, NoteNumber, Velocity, Channel);

}


/*! \brief Send a Note Off message (a real Note Off, not a Note On with null velocity)
 \param NoteNumber  Pitch value in the MIDI format (0 to 127). Take a look at the values, names and frequencies of notes here: http://www.phys.unsw.edu.au/jw/notes.html\n
 \param Velocity    Release velocity (0 to 127).
 \param Channel     The channel on which the message will be sent (1 to 16).
 */
void MIDI_Class::sendNoteOff(byte NoteNumber,
                             byte Velocity,
                             byte Channel)  {

    send(NoteOff, NoteNumber, Velocity, Channel);

}



/*! \brief Send a Control Change message 
 \param ControlNumber   The controller number (0 to 127). See the detailed description here: http://www.somascape.org/midi/tech/spec.html#ctrlnums
 \param ControlValue    The value for the specified controller (0 to 127).
 \param Channel         The channel on which the message will be sent (1 to 16). 
 */
void MIDI_Class::sendControlChange(byte ControlNumber,
                                   byte ControlValue,
                                   byte Channel)    {

    send(ControlChange, ControlNumber, ControlValue, Channel);

}

/*! \brief Send a Program Change message 
 \param ProgramNumber	The Program to select (0 to 127).
 \param Channel			The channel on which the message will be sent (1 to 16).
 */
void MIDI_Class::sendProgramChange(byte ProgramNumber, byte Channel)    {

    send(ProgramChange,ProgramNumber,0,Channel);

}


/*! \brief Send a Pitch Bend message using a signed integer value.
 \param PitchValue  The amount of bend to send (in a signed integer format), between -8192 (maximum downwards bend) and 8191 (max upwards bend), center value is 0.
 \param Channel     The channel on which the message will be sent (1 to 16).
 */
void MIDI_Class::sendPitchBend(int PitchValue,
                               byte Channel)    {

    unsigned int bend = PitchValue + 8192;
    sendPitchBend(bend,Channel);

}


/*! \brief Send a Pitch Bend message using an unsigned integer value.
 \param PitchValue  The amount of bend to send (in a signed integer format), between 0 (maximum downwards bend) and 16383 (max upwards bend), center value is 8192.
 \param Channel     The channel on which the message will be sent (1 to 16).
 */
void MIDI_Class::sendPitchBend(unsigned int PitchValue,
                               byte Channel)    {

    send(PitchBend,(PitchValue & 0x7F),(PitchValue >> 7) & 0x7F,Channel);

}


/*! \brief Send a Pitch Bend message using a floating point value.
 \param PitchValue  The amount of bend to send (in a floating point format), between -1.0f (maximum downwards bend) and +1.0f (max upwards bend), center value is 0.0f.
 \param Channel     The channel on which the message will be sent (1 to 16).
 */
void MIDI_Class::sendPitchBend(double PitchValue,
                               byte Channel)    {

    unsigned int pitchval = (PitchValue+1.f)*8192;

    if (pitchval > 16383) pitchval = 16383;     //overflow protection
        sendPitchBend(pitchval,Channel);

}


/*! \brief Generate and send a System Exclusive frame.
 \param length  The size of the array to send
 \param array   The byte array containing the data to send
 \param ArrayContainsBoundaries  When set to 'true', 0xF0 & 0xF7 bytes (start & stop SysEx) will NOT be sent (and therefore must be included in the array).
 default value is set to 'false' for compatibility with previous versions of the library.
 */
void MIDI_Class::sendSysEx(int length,
                           const byte *const array,
                           bool ArrayContainsBoundaries)    {

    if (ArrayContainsBoundaries == false) {

        USE_SERIAL_PORT.write(0xF0);

        for (int i=0; i<length; ++i)    USE_SERIAL_PORT.write(array[i]);

        USE_SERIAL_PORT.write(0xF7);

    }   else    for (int i=0;i<length;++i)  USE_SERIAL_PORT.write(array[i]);


    if (runningStatusEn)
        mRunningStatus_TX = InvalidType;

}


/*! \brief Send a Real Time (one byte) message. 
 
 \param Type The available Real Time types are: Start, Stop, Continue, Clock, ActiveSensing and SystemReset.
 You can also send a Tune Request with this method.
 @see kMIDIType
 */
void MIDI_Class::sendRealTime(kMIDIType Type)   {

    switch (Type) {

        case Clock:
        case Start:
        case Stop:  
        case Continue:
        USE_SERIAL_PORT.write((byte)Type);
        break;

        default:
        //Invalid Real Time marker
        break;

    }

}

#endif



#if COMPILE_MIDI_IN

/*! \brief Read a MIDI message from the serial port using the main input channel (see setInputChannel() for reference).
 
 Returned value: true if any valid message has been stored in the structure, false if not.
 A valid message is a message that matches the input channel. \n\n
 If the Thru is enabled and the messages matches the filter, it is sent back on the MIDI output.
 */
bool MIDI_Class::read() {

    int16_t bytes_available = USE_SERIAL_PORT.available();

    if (bytes_available <= 0) return false;

    return read(mInputChannel);

}


/*! \brief Reading/thru-ing method, the same as read() with a given input channel to read on. */
bool MIDI_Class::read(const byte inChannel) {

    if (inChannel >= MIDI_CHANNEL_OFF) return false; //MIDI Input disabled.

    if (parse(inChannel)) {

        if (input_filter(inChannel)) {

            #if USE_CALLBACKS
                launchCallback();
            #endif

                return true;

        }

    }

    return false;

}


// Private method: MIDI parser
bool MIDI_Class::parse(byte inChannel)  {

        
        /* Parsing algorithm:
         Get a byte from the serial buffer.
         * If there is no pending message to be recomposed, start a new one.
         - Find type and channel (if pertinent)
         - Look for other bytes in buffer, call parser recursively, until the message is assembled or the buffer is empty.
         * Else, add the extracted byte to the pending message, and check validity. When the message is done, store it.
         */

        const byte extracted = USE_SERIAL_PORT.read();

        if (mPendingMessageIndex == 0)  { //Start a new pending message

            mPendingMessage[0] = extracted;

            //Check for running status first
            switch (getTypeFromStatusByte(mRunningStatus_RX))   {

                //Only these types allow Running Status:
                case NoteOff:
                case NoteOn:
                case ControlChange:
                case PitchBend:
                case ProgramChange:

                //If the status byte is not received, prepend it to the pending message
                if (extracted < 0x80) {

                    mPendingMessage[0] = mRunningStatus_RX;
                    mPendingMessage[1] = extracted;
                    mPendingMessageIndex = 1;

                }

                //Else: well, we received another status byte, so the running status does not apply here.
                //It will be updated upon completion of this message.
                break;

                default:
                //No running status
                break;

            }

            switch (getTypeFromStatusByte(mPendingMessage[0])) {

                //1 byte messages
                case Start:
                case Continue:
                case Stop:
                case Clock:
                //Handle the message type directly here.
                mMessage.type = getTypeFromStatusByte(mPendingMessage[0]);
                mMessage.channel = 0;
                mMessage.data1 = 0;
                mMessage.data2 = 0;
                mMessage.valid = true;

                //\fix Running Status broken when receiving Clock messages.
                //Do not reset all input attributes, Running Status must remain unchanged.
                //reset_input_attributes(); 

                //We still need to reset these
                mPendingMessageIndex = 0;
                mPendingMessageExpectedLenght = 0;

                return true;
                break;

                //2 bytes messages
                case ProgramChange:
                mPendingMessageExpectedLenght = 2;
                break;

                //3 bytes messages
                case NoteOn:
                case NoteOff:
                case ControlChange:
                case PitchBend:
                mPendingMessageExpectedLenght = 3;
                break;

                case SystemExclusive:
                mPendingMessageExpectedLenght = MIDI_SYSEX_ARRAY_SIZE; // As the message can be any length between 3 and MIDI_SYSEX_ARRAY_SIZE bytes
                mRunningStatus_RX = InvalidType;
                break;

                case InvalidType:
                default:
                //This is obviously wrong. Let's get the hell out'a here.
                reset_input_attributes();
                return false;
                break;

            }

            //Then update the index of the pending message.
            mPendingMessageIndex++;

            #if USE_1BYTE_PARSING
                //Message is not complete.
                return false;
            #else
                //Call the parser recursively
                //to parse the rest of the message.
                return parse(inChannel);
            #endif

        }

         { 

            //First, test if this is a status byte
            if (extracted >= 0x80) {

                //Reception of status bytes in the middle of an uncompleted message
                //are allowed only for interleaved Real Time message or EOX
                switch (extracted) {

                    case Clock:
                    case Start:
                    case Continue:
                    case Stop:

                    /*
                    This is tricky. Here we will have to extract the one-byte message,
                    pass it to the structure for being read outside the MIDI class,
                    and recompose the message it was interleaved into.

                    Oh, and without killing the running status.. 

                    This is done by leaving the pending message as is, it will be completed on next calls.
                    */

                    mMessage.type = (kMIDIType)extracted;
                    mMessage.data1 = 0;
                    mMessage.data2 = 0;
                    mMessage.channel = 0;
                    mMessage.valid = true;
                    return true;
                    break;

                    //End of Exclusive
                    case 0xF7:
                    if (getTypeFromStatusByte(mPendingMessage[0]) == SystemExclusive)   {

                        //Store System Exclusive array in midimsg structure
                        for (byte i=0;i<MIDI_SYSEX_ARRAY_SIZE;i++)  mMessage.sysex_array[i] = mPendingMessage[i];

                        mMessage.type = SystemExclusive;

                        //Get length
                        mMessage.data1 = (mPendingMessageIndex+1) & 0xFF;
                        mMessage.data2 = (mPendingMessageIndex+1) >> 8;
                        mMessage.channel = 0;
                        mMessage.valid = true;
                        reset_input_attributes();

                        return true;

                    }   else {

                            //Well well well.. error.
                            reset_input_attributes();
                            return false;

                        }

                    break;

                    default:
                    break;

                }

            }

            //Add extracted data byte to pending message
            mPendingMessage[mPendingMessageIndex] = extracted;

            //Now we are going to check if we have reached the end of the message
            if (mPendingMessageIndex >= (mPendingMessageExpectedLenght-1)) {

                // "FML" case: fall down here with an overflown SysEx..
                // This means we received the last possible data byte that can fit the buffer.
                // If this happens, try increasing MIDI_SYSEX_ARRAY_SIZE.
                if (getTypeFromStatusByte(mPendingMessage[0]) == SystemExclusive) {

                    reset_input_attributes();
                    return false;

                }

                mMessage.type = getTypeFromStatusByte(mPendingMessage[0]);
                mMessage.channel = (mPendingMessage[0] & 0x0F)+1; //Don't check if it is a Channel Message
                mMessage.data1 = mPendingMessage[1];

                //Save data2 only if applicable
                if (mPendingMessageExpectedLenght == 3) mMessage.data2 = mPendingMessage[2];
                else mMessage.data2 = 0;

                //Reset local variables
                mPendingMessageIndex = 0;
                mPendingMessageExpectedLenght = 0;
                mMessage.valid = true;

                //Activate running status (if enabled for the received type)
                switch (mMessage.type) {

                    case NoteOff:
                    case NoteOn:
                    case ControlChange:
                    case PitchBend:
                    case ProgramChange:
                    //Running status enabled: store it from received message
                    mRunningStatus_RX = mPendingMessage[0];
                    break;

                    default:
                    //No running status
                    mRunningStatus_RX = InvalidType;
                    break;

                }

                return true;

            }   else {

                //Then update the index of the pending message.
                mPendingMessageIndex++;

                #if USE_1BYTE_PARSING
                    //Message is not complete.
                    return false;
                #else
                    //Call the parser recursively
                    //to parse the rest of the message.
                    return parse(inChannel);
                #endif

            }

        }

    // What are our chances to fall here?
    return false;

}


// Private method: check if the received message is on the listened channel
bool MIDI_Class::input_filter(byte inChannel)   {

    //This method handles recognition of channel (to know if the message is destinated to the Arduino)

    if (mMessage.type == InvalidType) return false;

    //First, check if the received message is Channel
    if (mMessage.type >= NoteOff && mMessage.type <= PitchBend) {

        // Then we need to know if we listen to it
        if ((mMessage.channel == mInputChannel) || (mInputChannel == MIDI_CHANNEL_OMNI))    return true;
        else    return false; //We don't listen to this channel

    }   else {

            //System messages are always received
            return true;

    }

}


// Private method: reset input attributes
void MIDI_Class::reset_input_attributes()   {

    mPendingMessageIndex = 0;
    mPendingMessageExpectedLenght = 0;
    mRunningStatus_RX = InvalidType;

}


// Getters
/*! \brief Get the last received message's type
 
 Returns an enumerated type. @see kMIDIType
 */
kMIDIType MIDI_Class::getType() const   {

    return mMessage.type;

}


/*! \brief Get the channel of the message stored in the structure.
 
 Channel range is 1 to 16. For non-channel messages, this will return 0.
 */
byte MIDI_Class::getChannel() const {

    return mMessage.channel;

}


/*! \brief Get the first data byte of the last received message. */
byte MIDI_Class::getData1() const   {

    return mMessage.data1;

}


/*! \brief Get the second data byte of the last received message. */
byte MIDI_Class::getData2() const   { 

    return mMessage.data2;

}


/*! \brief Get the System Exclusive byte array. 
 
 @see getSysExArrayLength to get the array's length in bytes.
 */
const byte * MIDI_Class::getSysExArray() const  { 

    return mMessage.sysex_array;

}

/*! \brief Get the length of the System Exclusive array.
 
 It is coded using data1 as LSB and data2 as MSB.
 \return The array's length, in bytes.
 */
unsigned int MIDI_Class::getSysExArrayLength() const    {

    unsigned int coded_size = ((unsigned int)(mMessage.data2) << 8) | mMessage.data1;

    return (coded_size > MIDI_SYSEX_ARRAY_SIZE) ? MIDI_SYSEX_ARRAY_SIZE : coded_size;

}


/*! \brief Check if a valid message is stored in the structure. */
bool MIDI_Class::check() const  { 

    return mMessage.valid;

}


// Setters
/*! \brief Set the value for the input MIDI channel 
 \param Channel the channel value. Valid values are 1 to 16, 
 MIDI_CHANNEL_OMNI if you want to listen to all channels, and MIDI_CHANNEL_OFF to disable MIDI input.
 */
void MIDI_Class::setInputChannel(const byte Channel)    { 

    mInputChannel = Channel;

}


#if USE_CALLBACKS

void MIDI_Class::setHandleNoteOff(void (*fptr)(byte channel, byte note, byte velocity))         { mNoteOffCallback          = fptr; }
void MIDI_Class::setHandleNoteOn(void (*fptr)(byte channel, byte note, byte velocity))          { mNoteOnCallback           = fptr; }
void MIDI_Class::setHandleControlChange(void (*fptr)(byte channel, byte number, byte value))    { mControlChangeCallback    = fptr; }
void MIDI_Class::setHandleProgramChange(void (*fptr)(byte channel, byte number))                { mProgramChangeCallback    = fptr; }
void MIDI_Class::setHandlePitchBend(void (*fptr)(byte channel, int bend))                       { mPitchBendCallback        = fptr; }
void MIDI_Class::setHandleSystemExclusive(void (*fptr)(byte * array, byte size))                { mSystemExclusiveCallback  = fptr; }
void MIDI_Class::setHandleClock(void (*fptr)(void))                                             { mClockCallback            = fptr; }
void MIDI_Class::setHandleStart(void (*fptr)(void))                                             { mStartCallback            = fptr; }
void MIDI_Class::setHandleContinue(void (*fptr)(void))                                          { mContinueCallback         = fptr; }
void MIDI_Class::setHandleStop(void (*fptr)(void))                                              { mStopCallback             = fptr; }


// Private - launch callback function based on received type.
void MIDI_Class::launchCallback()   {

    // The order is mixed to allow frequent messages to trigger their callback faster.

    switch (mMessage.type) {

        //Notes
        case NoteOff:
        if (mNoteOffCallback != NULL)
        mNoteOffCallback(mMessage.channel,mMessage.data1,mMessage.data2);
        break;

        case NoteOn:
        if (mNoteOnCallback != NULL)
        mNoteOnCallback(mMessage.channel,mMessage.data1,mMessage.data2);
        break;

        //Continuous controllers
        case ControlChange:
        if (mControlChangeCallback != NULL)
        mControlChangeCallback(mMessage.channel,mMessage.data1,mMessage.data2);
        break;

        case ProgramChange:
        if (mProgramChangeCallback != NULL)
        mProgramChangeCallback(mMessage.channel,mMessage.data1);
        break;

        case PitchBend:
        if (mPitchBendCallback != NULL)
        mPitchBendCallback(mMessage.channel,(int)((mMessage.data1 & 0x7F) | ((mMessage.data2 & 0x7F)<< 7)) - 8192);
        break; // TODO: check this

        case SystemExclusive:
        if (mSystemExclusiveCallback != NULL)
        mSystemExclusiveCallback(mMessage.sysex_array,mMessage.data1);
        break;

        //Real-time messages
        case Clock:
        if (mClockCallback != NULL)
        mClockCallback();
        break;

        case Start:
        if (mStartCallback != NULL)
        mStartCallback();
        break;

        case Continue:
        if (mContinueCallback != NULL)
        mContinueCallback();
        break;

        case Stop:
        if (mStopCallback != NULL)
        mStopCallback();
        break;

        case InvalidType:
        default:
        break;

    }

}


#endif // USE_CALLBACKS

#endif // COMPILE_MIDI_IN