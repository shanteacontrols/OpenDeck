/*
    Notice:

    Portions of this software were developed at http://www.pjrc.com/
    Those portions licensed under MIT License Agreement, (the "License"); You may not use these files except in compliance with the License.
    You may obtain a copy of the License at: http://opensource.org/licenses/MIT
    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
    and limitations under the License.
*/


#ifndef usb_serial_h__
#define usb_serial_h__

#include <stdint.h>
#include "VID_PID.h"

#ifdef __cplusplus
extern "C"{
#endif

/**************************************************************************
 *
 *  Configurable Options
 *
 **************************************************************************/



#define TRANSMIT_FLUSH_TIMEOUT  4   /* in milliseconds */
#define TRANSMIT_TIMEOUT        25  /* in milliseconds */


/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

// These buffer sizes are best for most applications, but perhaps if you
// want more buffering on some endpoint at the expense of others, this
// is where you can make such changes.  The AT90USB162 has only 176 bytes
// of DPRAM (USB buffers) and only endpoints 3 & 4 can double buffer.


// 0: control
// 1: debug IN
// 2: debug OUT
// 3: midi IN
// 4: midi OUT

#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)

// Some operating systems, especially Windows, may cache USB device
// info.  Changes to the device name may not update on the same
// computer unless the vendor or product ID numbers change, or the
// "bcdDevice" revision code is increased.

#define STR_PRODUCT             L"OpenDeck"
#define ENDPOINT0_SIZE          64

#define DEBUG_INTERFACE     1
#define DEBUG_TX_ENDPOINT   1
#define DEBUG_TX_SIZE       64
#define DEBUG_TX_BUFFER     EP_DOUBLE_BUFFER
#define DEBUG_TX_INTERVAL   1
#define DEBUG_RX_ENDPOINT   2
#define DEBUG_RX_SIZE       32
#define DEBUG_RX_BUFFER     EP_DOUBLE_BUFFER
#define DEBUG_RX_INTERVAL   2

#define MIDI_INTERFACE      0
#define MIDI_TX_ENDPOINT    3
#define MIDI_TX_SIZE        64
#define MIDI_TX_BUFFER      EP_DOUBLE_BUFFER
#define MIDI_RX_ENDPOINT    4
#define MIDI_RX_SIZE        64
#define MIDI_RX_BUFFER      EP_DOUBLE_BUFFER

#define NUM_ENDPOINTS       5
#define NUM_INTERFACE       2

#endif

// setup
void usb_init(void);            //initialize everything
void usb_shutdown(void);        //shut off USB

// variables
extern volatile uint8_t usb_configuration;
extern volatile uint8_t usb_suspended;
extern volatile uint8_t debug_flush_timer;

#ifdef __cplusplus
} // extern "C"
#endif


#endif