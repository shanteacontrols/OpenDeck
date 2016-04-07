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


/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/


static const uint8_t PROGMEM endpoint_config_table[] = {

    1, EP_TYPE_BULK_IN,       EP_SIZE(MIDI_TX_SIZE) | MIDI_TX_BUFFER,
    1, EP_TYPE_BULK_OUT,      EP_SIZE(MIDI_RX_SIZE) | MIDI_RX_BUFFER

};


/**************************************************************************
 *
 *  Descriptor Data
 *
 **************************************************************************/

//Descriptors are the data that your computer reads when it auto-detects
//this USB device (called "enumeration" in USB lingo).  The most commonly
//changed items are editable at the top of this file.  Changing things
//in here should only be done by those who've read chapter 9 of the USB
//spec and relevant portions of any USB class specifications!

//more info here: http://docs.lpcware.com/usbromlib/v1.0/_page__usb_descriptors.html

static const uint8_t PROGMEM device_descriptor[] = {

    18,                                 //bLength               //0
    1,                                  //bDescriptorType       //1
    0x00, 0x02,                         //bcdUSB                //2
    0,                                  //bDeviceClass          //3
    0,                                  //bDeviceSubClass       //4
    0,                                  //bDeviceProtocol       //5
    ENDPOINT0_SIZE,                     //bMaxPacketSize0       //6
    LSB(VENDOR_ID), MSB(VENDOR_ID),     //idVendor              //7
    LSB(PRODUCT_ID), MSB(PRODUCT_ID),   //idProduct             //8
    0x00, 0x01,                         //bcdDevice             //9
    0,                                  //iManufacturer         //10
    1,                                  //iProduct              //11
    0,                                  //iSerialNumber         //12
    1                                   //bNumConfigurations    //13

};

#define CONFIG1_DESC_SIZE               (9 + 74 + 32)

static const uint8_t PROGMEM config_descriptor[CONFIG1_DESC_SIZE] = {

    //configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9,                                  //bLength;
    2,                                  //bDescriptorType;
    LSB(CONFIG1_DESC_SIZE),             //wTotalLength LSB
    MSB(CONFIG1_DESC_SIZE),             //wTotalLength MSB
    1,                                  //bNumInterfaces
    1,                                  //bConfigurationValue
    0,                                  //iConfiguration
    0xC0,                               //bmAttributes
    250,                                //bMaxPower

    // This MIDI stuff is a copy of the example from the Audio Class
    // MIDI spec 1.0 (Nov 1, 1999), Appendix B, pages 37 to 43.

    //Standard MS Interface Descriptor
    9,                                  //bLength
    4,                                  //bDescriptorType
    MIDI_INTERFACE,                     //bInterfaceNumber
    0,                                  //AlternateSetting
    2,                                  //bNumEndpoints
    0x01,                               //bInterfaceClass (0x01 = Audio)
    0x03,                               //bInterfaceSubClass (0x03 = MIDI)
    0x00,                               //bInterfaceProtocol (unused for MIDI)
    0,                                  //iInterface

    //MIDI MS Interface Header, USB MIDI 6.1.2.1, page 21, Table 6-2
    7,                                  //bLength
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x01,                               //bDescriptorSubtype = MS_HEADER 
    0x00, 0x01,                         //bcdMSC = revision 01.00
    0x41, 0x00,                         //wTotalLength

    //MIDI IN Jack Descriptor, B.4.3, Table B-7 (embedded), page 40
    6,                                  //bLength
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x02,                               //bDescriptorSubtype = MIDI_IN_JACK
    0x01,                               //bJackType = EMBEDDED
    1,                                  //bJackID, ID = 1
    0,                                  //iJack

    //MIDI IN Jack Descriptor, B.4.3, Table B-8 (external), page 40
    6,                                  //bLength
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x02,                               //bDescriptorSubtype = MIDI_IN_JACK
    0x02,                               //bJackType = EXTERNAL
    2,                                  //bJackID, ID = 2
    0,                                  //iJack

    //MIDI OUT Jack Descriptor, B.4.4, Table B-9, page 41
    9,
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x03,                               //bDescriptorSubtype = MIDI_OUT_JACK
    0x01,                               //bJackType = EMBEDDED
    3,                                  //bJackID, ID = 3
    1,                                  //bNrInputPins = 1 pin
    2,                                  //BaSourceID(1) = 2
    1,                                  //BaSourcePin(1) = first pin
    0,                                  //iJack

    //MIDI OUT Jack Descriptor, B.4.4, Table B-10, page 41
    9,
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x03,                               //bDescriptorSubtype = MIDI_OUT_JACK
    0x02,                               //bJackType = EXTERNAL
    4,                                  //bJackID, ID = 4
    1,                                  //bNrInputPins = 1 pin
    1,                                  //BaSourceID(1) = 1
    1,                                  //BaSourcePin(1) = first pin
    0,                                  //iJack

    //Standard Bulk OUT Endpoint Descriptor, B.5.1, Table B-11, page 42
    9,                                  //bLength
    5,                                  //bDescriptorType = ENDPOINT 
    MIDI_RX_ENDPOINT,                   //bEndpointAddress
    0x02,                               //bmAttributes (0x02=bulk)
    MIDI_RX_SIZE, 0,                    //wMaxPacketSize
    0,                                  //bInterval
    0,                                  //bRefresh
    0,                                  //bSynchAddress

    //Class-specific MS Bulk OUT Endpoint Descriptor, B.5.2, Table B-12, page 42
    5,                                  //bLength
    0x25,                               //bDescriptorSubtype = CS_ENDPOINT
    0x01,                               //bJackType = MS_GENERAL
    1,                                  //bNumEmbMIDIJack = 1 jack
    1,                                  //BaAssocJackID(1) = jack ID #1

    //Standard Bulk IN Endpoint Descriptor, B.5.1, Table B-11, page 42
    9,                                  //bLength
    5,                                  //bDescriptorType = ENDPOINT 
    MIDI_TX_ENDPOINT | 0x80,            //bEndpointAddress
    0x02,                               //bmAttributes (0x02=bulk)
    MIDI_TX_SIZE, 0,                    //wMaxPacketSize
    0,                                  //bInterval
    0,                                  //bRefresh
    0,                                  //bSynchAddress

    //Class-specific MS Bulk IN Endpoint Descriptor, B.5.2, Table B-12, page 42
    5,                                  //bLength
    0x25,                               //bDescriptorSubtype = CS_ENDPOINT
    0x01,                               //bJackType = MS_GENERAL
    1,                                  //bNumEmbMIDIJack = 1 jack
    3,                                  //BaAssocJackID(1) = jack ID #3

};

//If you're desperate for a little extra code memory, these strings
//can be completely removed if iManufacturer, iProduct, iSerialNumber
//in the device descriptor are changed to zeros.

struct usb_string_descriptor_struct {

    uint8_t bLength;
    uint8_t bDescriptorType;
    int16_t wString[];

};

static const struct usb_string_descriptor_struct PROGMEM string0 = {

    4,
    3,
    {0x0409}

};

static const struct usb_string_descriptor_struct PROGMEM string1 = {

    sizeof(STR_PRODUCT),
    3,
    STR_PRODUCT

};

//This table defines which descriptor data is sent for each specific
//request from the host (in wValue and wIndex).
static const struct descriptor_list_struct {

    uint16_t        wValue;
    uint16_t        wIndex;
    const uint8_t   *addr;
    uint8_t         length;

}

PROGMEM descriptor_list[] = {

    {0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
    {0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
    {0x0300, 0x0000, (const uint8_t *)&string0, 4},
    {0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_PRODUCT)},

};

#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))


/**************************************************************************
 *
 *  Variables - these are the only non-stack RAM usage
 *
 **************************************************************************/

//zero when we are not configured, non-zero when enumerated
volatile uint8_t usb_configuration USBSTATE;
volatile uint8_t usb_suspended USBSTATE;


/**************************************************************************
 *
 *  Public Functions - these are the API intended for the user
 *
 **************************************************************************/

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

/**************************************************************************
 *
 *  Private Functions - not intended for general user consumption....
 *
 **************************************************************************/


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

                if (i >= NUM_DESC_LIST) {

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