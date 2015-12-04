/* USB API for Teensy USB Development Board
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
#include "usb_common.h"
#include "usb_private.h"
#include "USBmidi.h"

void usb_midi_class::begin(uint8_t channel)
{
    usb_init();
    inChannel = channel;
}

void usb_midi_class::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel)
{
    send_raw(0x08, 0x80 | ((channel - 1) & 0x0F), note & 0x7F, velocity & 0x7F);
}
void usb_midi_class::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel)
{
    send_raw(0x09, 0x90 | ((channel - 1) & 0x0F), note & 0x7F, velocity & 0x7F);
}
void usb_midi_class::sendPolyPressure(uint8_t note, uint8_t pressure, uint8_t channel)
{
    send_raw(0x0A, 0xA0 | ((channel - 1) & 0x0F), note & 0x7F, pressure & 0x7F);
}
void usb_midi_class::sendControlChange(uint8_t control, uint8_t value, uint8_t channel)
{
    send_raw(0x0B, 0xB0 | ((channel - 1) & 0x0F), control & 0x7F, value & 0x7F);
}
void usb_midi_class::sendProgramChange(uint8_t program, uint8_t channel)
{
    send_raw(0x0C, 0xC0 | ((channel - 1) & 0x0F), program & 0x7F, 0);
}
void usb_midi_class::sendAfterTouch(uint8_t pressure, uint8_t channel)
{
    send_raw(0x0D, 0xD0 | ((channel - 1) & 0x0F), pressure & 0x7F, 0);
}
void usb_midi_class::sendHwMIDIpitchBend(uint16_t value, uint8_t channel)
{
    send_raw(0x0E, 0xE0 | ((channel - 1) & 0x0F), value & 0x7F, (value >> 7) & 0x7F);
}

void usb_midi_class::sendSysEx(uint8_t length, const uint8_t *data)
{
    // TODO: MIDI 2.5 lib automatically adds start and stop bytes
    while (length > 3) {
        send_raw(0x04, data[0], data[1], data[2]);
        data += 3;
        length -= 3;
    }
    if (length == 3) {
        send_raw(0x07, data[0], data[1], data[2]);
    } else if (length == 2) {
        send_raw(0x06, data[0], data[1], 0);
    } else if (length == 1) {
        send_raw(0x05, data[0], 0, 0);
    }
}

void usb_midi_class::send_raw(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    uint8_t intr_state, timeout;

    if (!usb_configuration) return;
    intr_state = SREG;
    cli();
    UENUM = MIDI_TX_ENDPOINT;
    timeout = UDFNUML + 2;
    while (1) {
        // are we ready to transmit?
        if (UEINTX & (1<<RWAL)) break;
        SREG = intr_state;
        if (UDFNUML == timeout) return;
        if (!usb_configuration) return;
        intr_state = SREG;
        cli();
        UENUM = MIDI_TX_ENDPOINT;
    }
    UEDATX = b0;
    UEDATX = b1;
    UEDATX = b2;
    UEDATX = b3;
    if (!(UEINTX & (1<<RWAL))) UEINTX = 0x3A;
    SREG = intr_state;
}

void usb_midi_class::send_now(void)
{
    uint8_t intr_state;

    if (!usb_configuration) return;
    intr_state = SREG;
    cli();
    UENUM = MIDI_TX_ENDPOINT;
    if (UEBCLX != MIDI_TX_SIZE) {
        UEINTX = 0x3A;
    }
    SREG = intr_state;
}




bool usb_midi_class::read()
{
    uint8_t c, intr_state;
    uint8_t b0, b1, b2, b3, type1, type2;

    intr_state = SREG;
    cli();
    if (!usb_configuration) {
        SREG = intr_state;
        return false;
    }
    UENUM = MIDI_RX_ENDPOINT;
    retry:
    c = UEINTX;
    if (!(c & (1<<RWAL))) {
        if (c & (1<<RXOUTI)) {
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

    type1 = b0 & 0x0F;
    type2 = b1 & 0xF0;
    c = (b1 & 0x0F) + 1;
    if (type1 >= 0x08 && type1 <= 0x0E) {
        if (inChannel && inChannel != c) {
            // ignore other channels when user wants single channel read
            return false;
        }
        if (type1 == 0x08 && type2 == 0x80) {
            msg_type = 0;               // Note off
            if (handleNoteOff) (*handleNoteOff)(c, b2, b3);
            goto return_message;
        }
        if (type1 == 0x09 && type2 == 0x90) {
            if (b3) {
                msg_type = 1;           // Note on
                if (handleNoteOn) (*handleNoteOn)(c, b2, b3);
            } else {
                msg_type = 0;           // Note off
                if (handleNoteOff) (*handleNoteOff)(c, b2, b3);
            }
            goto return_message;
        }
        if (type1 == 0x0A && type2 == 0xA0) {
            msg_type = 2;               // Poly Pressure
            if (handleVelocityChange) (*handleVelocityChange)(c, b2, b3);
            goto return_message;
        }
        if (type1 == 0x0B && type2 == 0xB0) {
            msg_type = 3;               // Control Change
            if (handleControlChange) (*handleControlChange)(c, b2, b3);
            goto return_message;
        }
        if (type1 == 0x0C && type2 == 0xC0) {
            msg_type = 4;               // Program Change
            if (handleProgramChange) (*handleProgramChange)(c, b2);
            goto return_message;
        }
        if (type1 == 0x0D && type2 == 0xD0) {
            msg_type = 5;               // After Touch
            if (handleAfterTouch) (*handleAfterTouch)(c, b2);
            goto return_message;
        }
        if (type1 == 0x0E && type2 == 0xE0) {
            msg_type = 6;               // Pitch Bend
            if (handlePitchChange) (*handlePitchChange)(c,
                (b2 & 0x7F) | ((b3 & 0x7F) << 7));
            goto return_message;
        }
        return false;
        return_message:
        // only update these when returning true for a parsed message
        // all other return cases will preserve these user-visible values
        msg_channel = c;
        msg_data1 = b2;
        msg_data2 = b3;
        return true;
    }
    if (type1 == 0x04) {
        read_sysex_byte(b1);
        read_sysex_byte(b2);
        read_sysex_byte(b3);
        return false;
    }
    if (type1 >= 0x05 && type1 <= 0x07) {
        read_sysex_byte(b1);
        if (type1 >= 0x06) read_sysex_byte(b2);
        if (type1 == 0x07) read_sysex_byte(b3);
        msg_data1 = msg_sysex_len;
        msg_sysex_len = 0;
        msg_type = 7;
        return true;
    }
    if (type1 == 0x0F) {
        // TODO: does this need to be a full MIDI parser?
        // What software actually uses this message type in practice?
        if (msg_sysex_len > 0) {
            // From David Sorlien, dsorlien at gmail.com, http://axe4live.wordpress.com
            // OSX sometimes uses Single Byte Unparsed to
            // send bytes in the middle of a SYSEX message.
            read_sysex_byte(b1);
        } else {
            // From Sebastian Tomczak, seb.tomczak at gmail.com
            // http://little-scale.blogspot.com/2011/08/usb-midi-game-boy-sync-for-16.html
            msg_type = 8;
            if (handleRealTimeSystem) (*handleRealTimeSystem)(b1);
            goto return_message;
        }
    }
    return false;
}

void usb_midi_class::read_sysex_byte(uint8_t b)
{
    if (msg_sysex_len < USB_MIDI_SYSEX_MAX) {
        msg_sysex[msg_sysex_len++] = b;
    }
}





static volatile uint8_t prev_byte=0;

usb_midi_class      usbMIDI = usb_midi_class();