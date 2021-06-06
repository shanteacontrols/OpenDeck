/*

Copyright 2015-2021 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board/common/comm/usb/descriptors/Descriptors.h"
#include "usbd_core.h"
#include "midi/src/MIDI.h"
#include "core/src/general/RingBuffer.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/Timing.h"
#include "core/src/general/StringBuilder.h"
#include "board/Board.h"
#include "board/Internal.h"

/// Buffer size in bytes for incoming and outgoing MIDI messages (from device standpoint).

#define RX_BUFFER_SIZE_RING 4096
#define RX_BUFFER_SIZE_USB  128
#define TX_BUFFER_SIZE_USB  128
#define USB_TX_TIMEOUT_MS   2000

namespace
{
    USBD_HandleTypeDef hUsbDeviceFS;
    volatile bool      TxDone;
    volatile uint8_t   rxBuffer[RX_BUFFER_SIZE_USB];

    //rxBuffer is overriden every time RxCallback is called
    //save results in ring buffer and remove them as needed in readMIDI
    //not really the most optimized way, however, we are not in AVR land anymore
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE_RING> rxBufferRing;

    uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
    {
        USBD_LL_OpenEP(pdev, MIDI_STREAM_IN_EPADDR, USB_EP_TYPE_BULK, TX_BUFFER_SIZE_USB);
        USBD_LL_OpenEP(pdev, MIDI_STREAM_OUT_EPADDR, USB_EP_TYPE_BULK, RX_BUFFER_SIZE_USB);
        USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(rxBuffer), RX_BUFFER_SIZE_USB);
        TxDone = true;
        return 0;
    }

    uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
    {
        USBD_LL_CloseEP(pdev, MIDI_STREAM_IN_EPADDR);
        USBD_LL_CloseEP(pdev, MIDI_STREAM_OUT_EPADDR);
        return 0;
    }

    uint8_t TxCompleteCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
    {
        TxDone = true;
        return USBD_OK;
    }

    uint8_t RxCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
    {
        uint32_t count = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;

        for (uint32_t i = 0; i < count; i++)
            rxBufferRing.insert(rxBuffer[i]);

        USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(rxBuffer), RX_BUFFER_SIZE_USB);
        return 0;
    }

    uint8_t* getDeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);
        const USB_Descriptor_Device_t* desc = USBgetDeviceDescriptor(length);
        return (uint8_t*)desc;
    }

    uint8_t* getLangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);
        const USB_Descriptor_String_t* desc = USBgetLanguageString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);
        const USB_Descriptor_String_t* desc = USBgetManufacturerString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);
        const USB_Descriptor_String_t* desc = USBgetProductString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getSerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);

        Board::uniqueID_t uid;
        Board::uniqueID(uid);

        const USB_Descriptor_UID_String_t* desc = USBgetSerialIDString(length, uid.uid);
        return (uint8_t*)desc;
    }

    uint8_t* getConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);
        const USB_Descriptor_String_t* desc = USBgetManufacturerString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getInterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        UNUSED(speed);
        const USB_Descriptor_String_t* desc = USBgetManufacturerString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getConfigDescriptor(uint16_t* length)
    {
        const USB_Descriptor_Configuration_t* cfg = USBgetCfgDescriptor(length);
        return (uint8_t*)cfg;
    }

    USBD_DescriptorsTypeDef DeviceDescriptor = {
        getDeviceDescriptor,
        getLangIDStrDescriptor,
        getManufacturerStrDescriptor,
        getProductStrDescriptor,
        getSerialStrDescriptor,
        getConfigStrDescriptor,
        getInterfaceStrDescriptor
    };

    USBD_ClassTypeDef USB_MIDI = {
        initCallback,
        deInitCallback,
        NULL,
        NULL,
        NULL,
        TxCompleteCallback,
        RxCallback,
        NULL,
        NULL,
        NULL,
        NULL,
        getConfigDescriptor,
        NULL,
        NULL,
    };
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void usb()
            {
                USBD_Init(&hUsbDeviceFS, &DeviceDescriptor, DEVICE_FS);
                USBD_RegisterClass(&hUsbDeviceFS, &USB_MIDI);
                USBD_Start(&hUsbDeviceFS);
            }
        }    // namespace setup
    }        // namespace detail

    namespace USB
    {
        bool isUSBconnected()
        {
            return (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);
        }

        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            bool returnValue = false;

            if (rxBufferRing.count() >= 4)
            {
                rxBufferRing.remove(USBMIDIpacket.Event);
                rxBufferRing.remove(USBMIDIpacket.Data1);
                rxBufferRing.remove(USBMIDIpacket.Data2);
                rxBufferRing.remove(USBMIDIpacket.Data3);

                returnValue = true;
            }

            if (returnValue)
            {
#ifdef FW_APP
#ifdef LED_INDICATORS
                Board::detail::io::indicateMIDItraffic(MIDI::interface_t::usb, Board::detail::midiTrafficDirection_t::incoming);
#endif
#endif
            }

            return returnValue;
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            if (!isUSBconnected())
                return false;

            static bool timeoutActive = false;

            if (!TxDone)
            {
                if (timeoutActive)
                {
                    return false;
                }
                else
                {
                    uint32_t currentTime = core::timing::currentRunTimeMs();
                    timeoutActive        = true;

                    while ((core::timing::currentRunTimeMs() - currentTime) < USB_TX_TIMEOUT_MS)
                    {
                        if (TxDone)
                        {
                            timeoutActive = false;
                            break;
                        }
                    }
                }
            }

            if (TxDone)
            {
                timeoutActive = false;
                TxDone        = false;
                USBD_LL_Transmit(&hUsbDeviceFS, MIDI_STREAM_IN_EPADDR, (uint8_t*)&USBMIDIpacket, 4);

#ifdef LED_INDICATORS
                Board::detail::io::indicateMIDItraffic(MIDI::interface_t::usb, Board::detail::midiTrafficDirection_t::outgoing);
#endif
                return true;
            }
            else
            {
                return false;
            }
        }
    }    // namespace USB
}    // namespace Board