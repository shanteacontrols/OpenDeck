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
#include "../../../Types.h"
#include "../../../interface/settings/MIDIsettings.h"

class usb_midi_class    {

    public:
    void init(uint8_t channel);
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

    private:
    uint8_t inChannel;
    uint8_t msg_channel;
    uint8_t msg_type;
    uint8_t msg_data1;
    uint8_t msg_data2;
    uint8_t msg_sysex[MIDI_SYSEX_ARRAY_SIZE];
    uint8_t msg_sysex_len;

    void read_sysex_byte(uint8_t b);

};

extern usb_midi_class usbMIDI;

#endif
