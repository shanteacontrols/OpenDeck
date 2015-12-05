/*
 * USB API for Teensy USB Development Board
 * http://www.pjrc.com/teensy/teensyduino.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef USBserial_h_
#define USBserial_h_

#include <inttypes.h>

#define USB_MIDI_SYSEX_MAX          80  //maximum sysex length we can receive

#define USBmidiNoteOff              0
#define USBmidiNoteOn               1
#define USBmidiAfterTouchPoly       2
#define USBmidiControlChange        3
#define USBmidiProgramChange        4
#define USBmidiAfterTouchChannel    5
#define USBmidiHwMIDIpitchBend      6
#define USBmidiSystemExclusive      7


class usb_midi_class    {

public:
    void begin(uint8_t channel);
    void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);
    void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
    void sendPolyPressure(uint8_t note, uint8_t pressure, uint8_t channel);
    void sendControlChange(uint8_t control, uint8_t value, uint8_t channel);
    void sendProgramChange(uint8_t program, uint8_t channel);
    void sendAfterTouch(uint8_t pressure, uint8_t channel);
    void sendHwMIDIpitchBend(uint16_t value, uint8_t channel);
    void sendSysEx(uint8_t length, const uint8_t *data);
    void send_now(void);
    bool read();
    inline uint8_t getType(void) {
        return msg_type;
    };
    inline uint8_t getChannel(void) {
        return msg_channel;
    };
    inline uint8_t getData1(void) {
        return msg_data1;
    };
    inline uint8_t getData2(void) {
        return msg_data2;
    };
    inline uint8_t * getSysExArray(void) {
        return msg_sysex;
    };
    inline void setHandleNoteOff(void (*fptr)(uint8_t channel, uint8_t note, uint8_t velocity)) {
        handleNoteOff = fptr;
    };
    inline void setHandleNoteOn(void (*fptr)(uint8_t channel, uint8_t note, uint8_t velocity)) {
        handleNoteOn = fptr;
    };
    inline void setHandleVelocityChange(void (*fptr)(uint8_t channel, uint8_t note, uint8_t velocity)) {
        handleVelocityChange = fptr;
    };
    inline void setHandleControlChange(void (*fptr)(uint8_t channel, uint8_t control, uint8_t value)) {
        handleControlChange = fptr;
    };
    inline void setHandleProgramChange(void (*fptr)(uint8_t channel, uint8_t program)) {
        handleProgramChange = fptr;
    };
    inline void setHandleAfterTouch(void (*fptr)(uint8_t channel, uint8_t pressure)) {
        handleAfterTouch = fptr;
    };
    inline void setHandlePitchChange(void (*fptr)(uint8_t channel, int pitch)) {
        handlePitchChange = fptr;
    };
    inline void setHandleRealTimeSystem(void (*fptr)(uint8_t realtimebyte)) {
        handleRealTimeSystem = fptr;
    };
private:
    void send_raw(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
    void read_sysex_byte(uint8_t b);
    uint8_t inChannel;
    uint8_t msg_channel;
    uint8_t msg_type;
    uint8_t msg_data1;
    uint8_t msg_data2;
    uint8_t msg_sysex[USB_MIDI_SYSEX_MAX];
    uint8_t msg_sysex_len;
    void (*handleNoteOff)(uint8_t ch, uint8_t note, uint8_t vel);
    void (*handleNoteOn)(uint8_t ch, uint8_t note, uint8_t vel);
    void (*handleVelocityChange)(uint8_t ch, uint8_t note, uint8_t vel);
    void (*handleControlChange)(uint8_t ch, uint8_t, uint8_t);
    void (*handleProgramChange)(uint8_t ch, uint8_t);
    void (*handleAfterTouch)(uint8_t ch, uint8_t);
    void (*handlePitchChange)(uint8_t ch, int pitch);
    void (*handleRealTimeSystem)(uint8_t rtb);
};

extern usb_midi_class usbMIDI;

#endif