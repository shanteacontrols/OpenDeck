#ifndef USB_STRINGS_H_
#define USB_STRINGS_H_

#define STR_PRODUCT         L"OpenDeck"
#define STR_MANUFACTURER    L"Shantea Controls"
#define STR_SERIAL_NUMBER   L"R1.0.1"

#define I_MANUFACTURER_ID   0x01
#define I_PRODUCT_ID        0x02
#define I_SERIAL_NUMBER_ID  0x03

//Descriptors are the data that your computer reads when it auto-detects
//this USB device (called "enumeration" in USB lingo).  The most commonly
//changed items are editable at the top of this file.  Changing things
//in here should only be done by those who've read chapter 9 of the USB
//spec and relevant portions of any USB class specifications!

//more info here: http://docs.lpcware.com/usbromlib/v1.0/_page__usb_descriptors.html

static const uint8_t PROGMEM device_descriptor[] = {

    18,                                 //bLength
    0x01,                               //bDescriptorType
    0x00, 0x02,                         //bcdUSB
    0x00,                               //bDeviceClass
    0x00,                               //bDeviceSubClass
    0x00,                               //bDeviceProtocol
    ENDPOINT0_SIZE,                     //bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),     //idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID),   //idProduct
    0x00, 0x01,                         //bcdDevice
    I_MANUFACTURER_ID,                  //iManufacturer
    I_PRODUCT_ID,                       //iProduct
    I_SERIAL_NUMBER_ID,                 //iSerialNumber
    0x01                                //bNumConfigurations

};

#define CONFIG_DESCRIPTOR_SIZE          83

static const uint8_t PROGMEM config_descriptor[CONFIG_DESCRIPTOR_SIZE] = {

    //configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    0x09,                               //bLength;
    0x02,                               //bDescriptorType;
    LSB(CONFIG_DESCRIPTOR_SIZE),        //wTotalLength LSB
    MSB(CONFIG_DESCRIPTOR_SIZE),        //wTotalLength MSB
    0x01,                               //bNumInterfaces
    0x01,                               //bConfigurationValue
    0x00,                               //iConfiguration
    0xC0,                               //bmAttributes
    250,                                //bMaxPower

    // This MIDI stuff is a copy of the example from the Audio Class
    // MIDI spec 1.0 (Nov 1, 1999), Appendix B, pages 37 to 43.

    //Standard MS Interface Descriptor
    0x09,                               //bLength
    0x04,                               //bDescriptorType
    MIDI_INTERFACE,                     //bInterfaceNumber
    0x00,                               //AlternateSetting
    0x02,                               //bNumEndpoints
    0x01,                               //bInterfaceClass (0x01 = Audio)
    0x03,                               //bInterfaceSubClass (0x03 = MIDI)
    0x00,                               //bInterfaceProtocol (unused for MIDI)
    0x00,                               //iInterface

    //MIDI MS Interface Header, USB MIDI 6.1.2.1, page 21, Table 6-2
    0x07,                               //bLength
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x01,                               //bDescriptorSubtype = MS_HEADER
    0x00, 0x01,                         //bcdMSC = revision 01.00
    0x41, 0x00,                         //wTotalLength

    //MIDI IN Jack Descriptor, B.4.3, Table B-7 (embedded), page 40
    6,                                  //bLength
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x02,                               //bDescriptorSubtype = MIDI_IN_JACK
    0x01,                               //bJackType = EMBEDDED
    0x01,                               //bJackID, ID = 1
    0x00,                               //iJack

    //MIDI IN Jack Descriptor, B.4.3, Table B-8 (external), page 40
    0x06,                               //bLength
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x02,                               //bDescriptorSubtype = MIDI_IN_JACK
    0x02,                               //bJackType = EXTERNAL
    0x02,                               //bJackID, ID = 2
    0x00,                               //iJack

    //MIDI OUT Jack Descriptor, B.4.4, Table B-9, page 41
    0x09,
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x03,                               //bDescriptorSubtype = MIDI_OUT_JACK
    0x01,                               //bJackType = EMBEDDED
    0x03,                               //bJackID, ID = 3
    0x01,                               //bNrInputPins = 1 pin
    0x02,                               //BaSourceID(1) = 2
    0x01,                               //BaSourcePin(1) = first pin
    0x00,                               //iJack

    //MIDI OUT Jack Descriptor, B.4.4, Table B-10, page 41
    0x09,
    0x24,                               //bDescriptorType = CS_INTERFACE
    0x03,                               //bDescriptorSubtype = MIDI_OUT_JACK
    0x02,                               //bJackType = EXTERNAL
    0x04,                               //bJackID, ID = 4
    0x01,                               //bNrInputPins = 1 pin
    0x01,                               //BaSourceID(1) = 1
    0x01,                               //BaSourcePin(1) = first pin
    0x00,                               //iJack

    //Standard Bulk OUT Endpoint Descriptor, B.5.1, Table B-11, page 42
    0x09,                               //bLength
    0x05,                               //bDescriptorType = ENDPOINT
    MIDI_RX_ENDPOINT,                   //bEndpointAddress
    0x02,                               //bmAttributes (0x02=bulk)
    MIDI_RX_SIZE, 0x00,                 //wMaxPacketSize
    0x00,                               //bInterval
    0x00,                               //bRefresh
    0x00,                               //bSynchAddress

    //Class-specific MS Bulk OUT Endpoint Descriptor, B.5.2, Table B-12, page 42
    0x05,                               //bLength
    0x25,                               //bDescriptorSubtype = CS_ENDPOINT
    0x01,                               //bJackType = MS_GENERAL
    0x01,                               //bNumEmbMIDIJack = 1 jack
    0x01,                               //BaAssocJackID(1) = jack ID #1

    //Standard Bulk IN Endpoint Descriptor, B.5.1, Table B-11, page 42
    0x09,                               //bLength
    0x05,                               //bDescriptorType = ENDPOINT
    MIDI_TX_ENDPOINT | 0x80,            //bEndpointAddress
    0x02,                               //bmAttributes (0x02=bulk)
    MIDI_TX_SIZE, 0,                    //wMaxPacketSize
    0x00,                               //bInterval
    0x00,                               //bRefresh
    0x00,                               //bSynchAddress

    //Class-specific MS Bulk IN Endpoint Descriptor, B.5.2, Table B-12, page 42
    0x05,                               //bLength
    0x25,                               //bDescriptorSubtype = CS_ENDPOINT
    0x01,                               //bJackType = MS_GENERAL
    0x01,                               //bNumEmbMIDIJack = 1 jack
    0x03                                //BaAssocJackID(1) = jack ID #3

};

struct usb_string_descriptor_struct {

    uint8_t bLength;            //size of this descriptor in bytes
    uint8_t bDescriptorType;    //string descriptor type (always 0x03)
    int16_t bString[];          //UNICODE encoded string

};

static const struct usb_string_descriptor_struct PROGMEM usbStringLANG = {

    //index 0x00: LANGID Codes
    0x04,                       //bLenght
    USB_STRING_DESCRIPTOR_TYPE, //bDescriptorType
    { USB_LANGUAGE_ID }         //bString - wLANGID

};

static const struct usb_string_descriptor_struct PROGMEM usbStringManufacturer = {

    //index 0x01: Manufacturer
    sizeof(STR_MANUFACTURER),   //bLenght
    USB_STRING_DESCRIPTOR_TYPE, //bDescriptorType
    STR_MANUFACTURER,           //bString - vendor

};

static const struct usb_string_descriptor_struct PROGMEM usbStringProduct = {

    //index 0x02: Product
    sizeof(STR_PRODUCT),        //bLenght
    USB_STRING_DESCRIPTOR_TYPE, //bDescriptorType
    STR_PRODUCT                 //bString - product

};

static const struct usb_string_descriptor_struct PROGMEM usbStringSerialNumber = {

    //index 0x03: Serial Number
    sizeof(STR_SERIAL_NUMBER),  //bLenght
    USB_STRING_DESCRIPTOR_TYPE, //bDescriptorType
    STR_SERIAL_NUMBER           //bString - serial number

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

    { DESCRIPTOR_DEVICE_WVALUE, 0x0000, device_descriptor, sizeof(device_descriptor) },
    { DESCRIPTOR_CONFIGURATION_WVALUE, 0x0000, config_descriptor, sizeof(config_descriptor) },
    { DESCRIPTOR_STRING_WVALUE, 0x0000, (const uint8_t *)&usbStringLANG, 0x04 },
    { DESCRIPTOR_STRING_WVALUE | I_MANUFACTURER_ID, USB_LANGUAGE_ID, (const uint8_t *)&usbStringManufacturer, sizeof(STR_MANUFACTURER) },
    { DESCRIPTOR_STRING_WVALUE | I_PRODUCT_ID, USB_LANGUAGE_ID, (const uint8_t *)&usbStringProduct, sizeof(STR_PRODUCT) },
    { DESCRIPTOR_STRING_WVALUE | I_SERIAL_NUMBER_ID, USB_LANGUAGE_ID, (const uint8_t *)&usbStringSerialNumber, sizeof(STR_SERIAL_NUMBER) },

};

#define DESCRIPTOR_LIST_SIZE (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))

#endif