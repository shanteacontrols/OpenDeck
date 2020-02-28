/*

Copyright 2015-2020 Igor Petrovic

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

#include "board/common/usb/descriptors/Descriptors.h"
#include "usbd_core.h"
#include "core/src/general/RingBuffer.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/Timing.h"
#include "core/src/general/StringBuilder.h"
#include "board/Board.h"
#include "board/Internal.h"

///
/// \brief Buffer size in bytes for incoming HID messages (from device standpoint).
///
#define RX_BUFFER_SIZE 128

namespace
{
    typedef enum
    {
        HID_IDLE = 0,
        HID_BUSY,
    } HID_StateTypeDef;

    typedef struct
    {
        uint32_t         Protocol;
        uint32_t         IdleState;
        uint32_t         AltSetting;
        HID_StateTypeDef state;
    } USBD_HID_HandleTypeDef;

    USBD_HandleTypeDef hUsbDeviceFS;

    volatile bool                  TxDone;
    __ALIGN_BEGIN volatile uint8_t rxBuffer[RX_BUFFER_SIZE] __ALIGN_END;

    //rxBuffer is overriden every time RxCallback is called
    //save results in ring buffer and remove them as needed in readMIDI
    //not really the most optimized way, however, we are not in AVR land anymore
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE> rxBufferRing;

    uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
    {
        USBD_LL_OpenEP(pdev, HID_IN_EPADDR, EP_TYPE_INTERRUPT, HID_IN_EPSIZE);

        pdev->ep_in[HID_IN_EPADDR & 0xFU].is_used = 1U;

        pdev->pClassData = USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

        if (pdev->pClassData == NULL)
        {
            return USBD_FAIL;
        }

        ((USBD_HID_HandleTypeDef*)pdev->pClassData)->state = HID_IDLE;

        USBD_LL_PrepareReceive(pdev, HID_IN_EPADDR, (uint8_t*)(rxBuffer), RX_BUFFER_SIZE);

        return USBD_OK;
    }

    uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
    {
        /* Close HID EPs */
        USBD_LL_CloseEP(pdev, HID_IN_EPADDR);
        pdev->ep_in[HID_IN_EPADDR & 0xFU].is_used = 0U;

        /* Free allocated memory */
        if (pdev->pClassData != NULL)
        {
            USBD_free(pdev->pClassData);
            pdev->pClassData = NULL;
        }

        return USBD_OK;
    }

    uint8_t setupCallback(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
    {
        uint16_t len  = 0;
        uint8_t* pbuf = NULL;

        switch ((req->wValue) >> 8)
        {
        case HID_DTYPE_Report:
            pbuf = (uint8_t*)USBgetHIDreport(&len);
            break;

        case HID_DTYPE_HID:
            pbuf = (uint8_t*)USBgetHIDdescriptor(&len);
            break;

        default:
            return USBD_OK;
        }

        USBD_CtlSendData(pdev, pbuf, len);

        return USBD_OK;
    }

    uint8_t TxCompleteCallback(USBD_HandleTypeDef* pdev)
    {
        TxDone = true;
        return USBD_OK;
    }

    uint8_t RxCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
    {
        // uint32_t count = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;

        // for (uint32_t i = 0; i < count; i++)
        //     rxBufferRing.insert(rxBuffer[i]);

        // USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(rxBuffer), RX_BUFFER_SIZE);
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

    USBD_ClassTypeDef USB_HID = {
        initCallback,
        deInitCallback,
        setupCallback,
        NULL,
        NULL,
        NULL,
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
                USBD_RegisterClass(&hUsbDeviceFS, &USB_HID);
                USBD_Start(&hUsbDeviceFS);
            }
        }    // namespace setup
    }        // namespace detail

    namespace USB
    {
        bool readHID()
        {
            bool returnValue = false;
            return returnValue;
        }

        bool writeHID()
        {
            while (!TxDone)
            {
            }

            TxDone = false;
            // USBD_LL_Transmit(&hUsbDeviceFS, MIDI_STREAM_IN_EPADDR, (uint8_t*)&USBMIDIpacket, 4);

            return true;
        }
    }    // namespace USB
}    // namespace Board