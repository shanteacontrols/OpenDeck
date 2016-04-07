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

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "../../usb/usb_common.h"
#include "../../usb/usb.h"
#include "USBmidi.h"
#include "CIN.h"
#include "../hw_midi/hw_MIDI.h"

void usb_midi_class::init(uint8_t channel) {

    inChannel = channel;

}

bool usb_midi_class::read() {

    uint8_t decodedChannel, intr_state;
    uint8_t b0, b1, b2, b3, cin, messageType;

    intr_state = SREG;
    cli();

    if (!usb_configuration) {

        SREG = intr_state;
        return false;

    }

    UENUM = MIDI_RX_ENDPOINT;

    retry:
    decodedChannel = UEINTX;

    if (!(decodedChannel & (1<<RWAL))) {

        if (decodedChannel & (1<<RXOUTI)) {

            UEINTX = 0x6B;
            goto retry;

        }

        SREG = intr_state;
        return false;

    }

    b0 = UEDATX;
    b1 = UEDATX;
    b2 = UEDATX;
    b3 = UEDATX;

    if (!(UEINTX & (1<<RWAL))) UEINTX = 0x6B;
    SREG = intr_state;

    cin = b0 & 0x0F;
    messageType = b1 & 0xF0;
    decodedChannel = (b1 & 0x0F) + 1;

    if (cin >= CIN_NOTE_OFF && cin <= CIN_PITCH_BEND) {

        if (inChannel && inChannel != decodedChannel) {

            //ignore other channels when user wants single channel read
            return false;

        }

        if (cin == CIN_NOTE_OFF && messageType == midiMessageNoteOff) {

            //note off
            msg_type = midiMessageNoteOff;
            goto return_message;

        }

        if (cin == CIN_NOTE_ON && messageType == midiMessageNoteOn) {

            //note on
            msg_type = midiMessageNoteOn;
            goto return_message;

        }

        if (cin == CIN_CONTROL_CHANGE && messageType == midiMessageControlChange) {

            //control change
            msg_type = midiMessageControlChange;
            goto return_message;

        }

        if (cin == CIN_PROGRAM_CHANGE && messageType == midiMessageProgramChange) {

            //program change
            msg_type = midiMessageProgramChange;
            goto return_message;

        }

        if (cin == CIN_CHANNEL_AFTERTOUCH && messageType == midiMessageAfterTouchChannel) {

            //aftertouch
            msg_type = midiMessageAfterTouchChannel;
            goto return_message;

        }

        if (cin == CIN_PITCH_BEND && messageType == midiMessagePitchBend) {

            //pitch bend
            msg_type = midiMessagePitchBend;
            goto return_message;

        }

        return false;

        return_message:
        // only update these when returning true for a parsed message
        // all other return cases will preserve these user-visible values
        msg_channel = decodedChannel;
        msg_data1 = b2;
        msg_data2 = b3;
        return true;

    }

    if (cin == CIN_SYSEX_START) {

        read_sysex_byte(b1);
        read_sysex_byte(b2);
        read_sysex_byte(b3);
        return false;

    }

    if (cin >= CIN_SYSEX_STOP_1BYTE && cin <= CIN_SYSEX_STOP_3BYTE) {

        read_sysex_byte(b1);
        if (cin >= CIN_SYSEX_STOP_2BYTE) read_sysex_byte(b2);
        if (cin == CIN_SYSEX_STOP_3BYTE) read_sysex_byte(b3);
        msg_data1 = msg_sysex_len;
        msg_sysex_len = 0;
        msg_type = midiMessageSystemExclusive;
        return true;

    }

    if (cin == midiMessageSystemExclusive) {

        //TODO: does this need to be a full MIDI parser?
        //What software actually uses this message type in practice?
        //opendeck does!

        if (msg_sysex_len > 0) {

            //From David Sorlien, dsorlien at gmail.com, http://axe4live.wordpress.com
            //OSX sometimes uses Single Byte Unparsed to
            //send bytes in the middle of a SYSEX message.
            read_sysex_byte(b1);

        }

    }

    return false;

}

void usb_midi_class::read_sysex_byte(uint8_t b) {

    if (msg_sysex_len < MIDI_SYSEX_ARRAY_SIZE)
        msg_sysex[msg_sysex_len++] = b;

}

usb_midi_class usbMIDI;