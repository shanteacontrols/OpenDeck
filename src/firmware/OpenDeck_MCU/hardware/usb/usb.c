/*
    Notice:

    Portions of this software were developed at http://www.pjrc.com/
    Those portions licensed under MIT License Agreement, (the "License"); You may not use these files except in compliance with the License.
    You may obtain a copy of the License at: http://opensource.org/licenses/MIT
    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
    and limitations under the License.
*/

#include <util/delay.h>
#include "usb_common.h"
#include "usb.h"
#include "Descriptors.h"

//zero when we are not configured, non-zero when enumerated
volatile uint8_t usb_configuration USBSTATE;
volatile uint8_t usb_suspended USBSTATE;

//initialize USB
void usb_init(void) {

    cli();
    uint8_t u;

    u = USBCON;
    if ((u & (1<<USBE)) && !(u & (1<<FRZCLK))) return;
    HW_CONFIG();
    USB_FREEZE();                   //enable USB
    PLL_CONFIG();                   //config PLL
    while (!(PLLCSR & (1<<PLOCK))); //wait for PLL lock
    USB_CONFIG();                   //start USB clock
    UDCON = 0;                      //enable attach resistor
    usb_configuration = 0;
    usb_suspended = 0;
    UDINT = 0;
    UDIEN = (1<<EORSTE)|(1<<SOFE);
    sei();

}

void usb_shutdown(void) {

    UDIEN = 0;      //disable interrupts
    UDCON = 1;      //disconnect attach resistor
    USBCON = 0;     //shut off USB peripheral
    PLLCSR = 0;     //shut off PLL
    usb_configuration = 0;
    usb_suspended = 1;

}

static inline void send_now(void)   {

    uint8_t intr_state;

    if (!usb_configuration) return;
    intr_state = SREG;
    cli();
    UENUM = MIDI_TX_ENDPOINT;

    if (UEBCLX != MIDI_TX_SIZE)
    UEINTX = 0x3A;

    SREG = intr_state;

}

void usbSend(uint8_t usbByte0, uint8_t usbByte1, uint8_t usbByte2, uint8_t usbByte3)   {

    uint8_t intr_state, timeout;

    if (!usb_configuration) return;
    intr_state = SREG;
    cli();
    UENUM = MIDI_TX_ENDPOINT;
    timeout = UDFNUML + 2;

    while (1) {

        //are we ready to transmit?
        if (UEINTX & (1<<RWAL)) break;
        SREG = intr_state;
        if (UDFNUML == timeout) return;
        if (!usb_configuration) return;
        intr_state = SREG;
        cli();
        UENUM = MIDI_TX_ENDPOINT;

    }

    UEDATX = usbByte0;
    UEDATX = usbByte1;
    UEDATX = usbByte2;
    UEDATX = usbByte3;

    if (!(UEINTX & (1<<RWAL))) UEINTX = 0x3A;
    SREG = intr_state;

    send_now();

}

//USB Device Interrupt - handle all device-level events
//the transmit buffer flushing is triggered by the start of frame
ISR(USB_GEN_vect)   {

    uint8_t intbits;
    intbits = UDINT;
    UDINT = 0;

    if (intbits & (1<<EORSTI)) {

        UENUM = 0;
        UECONX = 1;
        UECFG0X = EP_TYPE_CONTROL;
        UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
        UEIENX = (1<<RXSTPE);
        usb_configuration = 0;

    }

    if ((intbits & (1<<SOFI)) && usb_configuration) {

        UENUM = MIDI_TX_ENDPOINT;
        if (UEBCLX) UEINTX = 0x3A;

    }

    if (intbits & (1<<SUSPI)) {

        //USB Suspend (inactivity for 3ms)
        UDIEN = (1<<WAKEUPE);
        usb_configuration = 0;
        usb_suspended = 1;

        #if (F_CPU >= 8000000L)
        //WAKEUPI does not work with USB clock freeze 
        //when CPU is running less than 8 MHz.
        //Is this a hardware bug?
        USB_FREEZE();   //shut off USB
        PLLCSR = 0;     //shut off PLL
        #endif

        //to properly meet the USB spec, current must
        //reduce to less than 2.5 mA, which means using
        //powerdown mode, but that breaks the Arduino
        //user's paradigm....

    }

    if (usb_suspended && (intbits & (1<<WAKEUPI))) {

        //USB Resume (pretty much any activity)
        #if (F_CPU >= 8000000L)
        PLL_CONFIG();
        while (!(PLLCSR & (1<<PLOCK))) ;
        USB_CONFIG();
        #endif

        UDIEN = (1<<EORSTE)|(1<<SOFE)|(1<<SUSPE);
        usb_suspended = 0;

        return;

    }

}

// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready(void)  {

    while (!(UEINTX & (1<<TXINI))) ;

}

static inline void usb_send_in(void)    {

    UEINTX = ~(1<<TXINI);

}

//USB Endpoint Interrupt - endpoint 0 is handled here.  The
//other endpoints are manipulated by the user-callable
//functions, and the start-of-frame interrupt.
ISR(USB_COM_vect)   {

    uint8_t intbits;
    const uint8_t *list;
    const uint8_t *cfg;
    uint8_t i, n, len, en;
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
    uint16_t desc_val;
    const uint8_t *desc_addr;
    uint8_t desc_length;

    UENUM = 0;
    intbits = UEINTX;

    if (intbits & (1<<RXSTPI)) {

        bmRequestType = UEDATX;
        bRequest = UEDATX;

        read_word_lsbfirst(wValue, UEDATX);
        read_word_lsbfirst(wIndex, UEDATX);
        read_word_lsbfirst(wLength, UEDATX);

        UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));

        if (bRequest == GET_DESCRIPTOR) {

            list = (const uint8_t *)descriptor_list;

            for (i=0; ; i++) {

                if (i >= DESCRIPTOR_LIST_SIZE) {

                    UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
                    return;

                }

                pgm_read_word_postinc(desc_val, list);

                if (desc_val != wValue) {

                    list += sizeof(struct descriptor_list_struct)-2;
                    continue;

                }

                pgm_read_word_postinc(desc_val, list);

                if (desc_val != wIndex) {

                    list += sizeof(struct descriptor_list_struct)-4;
                    continue;

                }

                pgm_read_word_postinc(desc_addr, list);

                desc_length = pgm_read_byte(list);
                break;

            }

            len = (wLength < 256) ? wLength : 255;
            if (len > desc_length) len = desc_length;
            list = desc_addr;

            do {

                //wait for host ready for IN packet
                do {

                    i = UEINTX;

                } while (!(i & ((1<<TXINI)|(1<<RXOUTI))));

                if (i & (1<<RXOUTI)) return;    //abort

                //send IN packet
                n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;

                for (i = n; i; i--) {

                    pgm_read_byte_postinc(UEDATX, list);

                }

                len -= n;
                usb_send_in();

            } while (len || n == ENDPOINT0_SIZE);

            return;

        }

        if (bRequest == SET_ADDRESS) {

            usb_send_in();
            usb_wait_in_ready();
            UDADDR = wValue | (1<<ADDEN);
            return;

        }

        if (bRequest == SET_CONFIGURATION && bmRequestType == 0)    {

            usb_configuration = wValue;
            usb_send_in();
            cfg = endpoint_config_table;

            for (i=1; i<NUM_ENDPOINTS; i++) {

                UENUM = i;
                pgm_read_byte_postinc(en, cfg);
                UECONX = en;

                if (en) {

                    pgm_read_byte_postinc(UECFG0X, cfg);
                    pgm_read_byte_postinc(UECFG1X, cfg);

                }

            }

            UERST = 0x1E;
            UERST = 0;

            return;

        }

        if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80) {

            usb_wait_in_ready();
            UEDATX = usb_configuration;
            usb_send_in();
            return;

        }

        if (bRequest == GET_STATUS) {

            usb_wait_in_ready();
            i = 0;

            if (bmRequestType == 0x82) {

                UENUM = wIndex;
                if (UECONX & (1<<STALLRQ)) i = 1;
                UENUM = 0;

            }

            UEDATX = i;
            UEDATX = 0;
            usb_send_in();
            return;

        }

        if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE)
          && bmRequestType == 0x02 && wValue == 0) {

            i = wIndex & 0x7F;

            if (i >= 1 && i <= MAX_ENDPOINT) {

                usb_send_in();
                UENUM = i;

                if (bRequest == SET_FEATURE)
                    UECONX = (1<<STALLRQ)|(1<<EPEN);
                else {

                    UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);
                    UERST = (1 << i);
                    UERST = 0;

                }

                return;

            }

        }

        UECONX = (1<<STALLRQ) | (1<<EPEN);

    }

}