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
#include "board/stm32/comm/usb/USB.h"
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
#define USB_TX_TIMEOUT_MS   2000

namespace
{
    typedef struct
    {
        uint32_t          data[CDC_NOTIFICATION_EPSIZE / 4U]; /* Force 32bits alignment */
        uint8_t           CmdOpCode;
        uint8_t           CmdLength;
        uint32_t          TxLength;
        volatile uint32_t TxState;
        volatile uint32_t RxState;
    } cdcData_t;

    typedef struct
    {
        int8_t (*Control)(uint8_t cmd, uint8_t* pbuf, uint16_t length);
        int8_t (*TransmitCplt)(uint8_t* Buf, uint32_t* Len, uint8_t epnum);
    } cdcCallback_t;

    USBD_HandleTypeDef hUsbDeviceFS;
    cdcData_t          cdcData;
    volatile bool      TxDone;
    volatile uint8_t   midiRxBuffer[MIDI_IN_OUT_EPSIZE];
    volatile uint8_t   cdcRxBuffer[CDC_IN_OUT_EPSIZE];

    //these buffers are overriden every time midiRxCallback is called
    //save results in ring buffer and remove them as needed when reading
    //not really the most optimized way, however, we are not in AVR land anymore
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE_RING> cdcRxBufferRing;
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE_RING> midiRxBufferRing;

    namespace cdc
    {
        uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            UNUSED(cfgidx);

            cdcData.TxState = 0;
            cdcData.RxState = 0;

            /* Open EP IN */
            (void)USBD_LL_OpenEP(pdev, CDC_IN_EPADDR, USB_EP_TYPE_BULK, CDC_IN_OUT_EPSIZE);

            pdev->ep_in[CDC_IN_EPADDR & 0xFU].is_used = 1U;

            /* Open EP OUT */
            (void)USBD_LL_OpenEP(pdev, CDC_OUT_EPADDR, USB_EP_TYPE_BULK, CDC_IN_OUT_EPSIZE);

            pdev->ep_out[CDC_OUT_EPADDR & 0xFU].is_used           = 1U;
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].bInterval = CDC_POLLING_TIME;

            /* Open Command IN EP */
            (void)USBD_LL_OpenEP(pdev, CDC_NOTIFICATION_EPADDR, USBD_EP_TYPE_INTR, CDC_NOTIFICATION_EPSIZE);
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].is_used = 1U;

            /* Prepare Out endpoint to receive next packet */
            (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EPADDR, (uint8_t*)cdcRxBuffer, CDC_IN_OUT_EPSIZE);

            return (uint8_t)USBD_OK;
        }

        uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            UNUSED(cfgidx);
            uint8_t ret = 0U;

            /* Close EP IN */
            (void)USBD_LL_CloseEP(pdev, CDC_IN_EPADDR);
            pdev->ep_in[CDC_IN_EPADDR & 0xFU].is_used = 0U;

            /* Close EP OUT */
            (void)USBD_LL_CloseEP(pdev, CDC_OUT_EPADDR);
            pdev->ep_out[CDC_OUT_EPADDR & 0xFU].is_used = 0U;

            /* Close Command IN EP */
            (void)USBD_LL_CloseEP(pdev, CDC_NOTIFICATION_EPADDR);
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].is_used   = 0U;
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].bInterval = 0U;

            return ret;
        }

        uint8_t setupCallback(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
        {
            uint8_t            ifalt       = 0U;
            uint16_t           status_info = 0U;
            USBD_StatusTypeDef ret         = USBD_OK;

            switch (req->bmRequest & USB_REQ_TYPE_MASK)
            {
            case USB_REQ_TYPE_CLASS:
                if (req->wLength != 0U)
                {
                    if ((req->bmRequest & 0x80U) != 0U)
                    {
                        ((cdcCallback_t*)pdev->pUserData)->Control(req->bRequest, (uint8_t*)cdcData.data, req->wLength);

                        (void)USBD_CtlSendData(pdev, (uint8_t*)cdcData.data, req->wLength);
                    }
                    else
                    {
                        cdcData.CmdOpCode = req->bRequest;
                        cdcData.CmdLength = (uint8_t)req->wLength;

                        (void)USBD_CtlPrepareRx(pdev, (uint8_t*)cdcData.data, req->wLength);
                    }
                }
                else
                {
                    ((cdcCallback_t*)pdev->pUserData)->Control(req->bRequest, (uint8_t*)req, 0U);
                }
                break;

            case USB_REQ_TYPE_STANDARD:
                switch (req->bRequest)
                {
                case USB_REQ_GET_STATUS:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        (void)USBD_CtlSendData(pdev, (uint8_t*)&status_info, 2U);
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_GET_INTERFACE:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        (void)USBD_CtlSendData(pdev, &ifalt, 1U);
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_SET_INTERFACE:
                    if (pdev->dev_state != USBD_STATE_CONFIGURED)
                    {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_CLEAR_FEATURE:
                    break;

                default:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;
                }
                break;

            default:
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
                break;
            }

            return (uint8_t)ret;
        }

        uint8_t EP0_RxReadyCallback(USBD_HandleTypeDef* pdev)
        {
            if ((pdev->pUserData != NULL) && (cdcData.CmdOpCode != 0xFFU))
            {
                ((cdcCallback_t*)pdev->pUserData)->Control(cdcData.CmdOpCode, (uint8_t*)cdcData.data, (uint16_t)cdcData.CmdLength);
                cdcData.CmdOpCode = 0xFFU;
            }

            return (uint8_t)USBD_OK;
        }

        uint8_t dataInCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            PCD_HandleTypeDef* hpcd = (PCD_HandleTypeDef*)pdev->pData;

            if ((pdev->ep_in[epnum].total_length > 0U) &&
                ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
            {
                /* Update the packet total length */
                pdev->ep_in[epnum].total_length = 0U;

                /* Send ZLP */
                (void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
            }
            else
            {
                cdcData.TxState = 0U;
            }

            return (uint8_t)USBD_OK;
        }

        uint8_t dataOutCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            uint32_t length = USBD_LL_GetRxDataSize(pdev, epnum);

            /* USB data will be immediately processed, this allow next USB traffic being
            NAKed till the end of the application Xfer */

            for (uint32_t i = 0; i < length; i++)
                cdcRxBufferRing.insert(cdcRxBuffer[i]);

            (void)USBD_LL_PrepareReceive(pdev, CDC_OUT_EPADDR, (uint8_t*)cdcRxBuffer, CDC_IN_OUT_EPSIZE);

            return (uint8_t)USBD_OK;
        }
    }    // namespace cdc

    namespace midi
    {
        uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            USBD_LL_OpenEP(pdev, MIDI_STREAM_IN_EPADDR, USB_EP_TYPE_BULK, MIDI_IN_OUT_EPSIZE);
            USBD_LL_OpenEP(pdev, MIDI_STREAM_OUT_EPADDR, USB_EP_TYPE_BULK, MIDI_IN_OUT_EPSIZE);
            USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(midiRxBuffer), MIDI_IN_OUT_EPSIZE);
            TxDone = true;
            return 0;
        }

        uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            USBD_LL_CloseEP(pdev, MIDI_STREAM_IN_EPADDR);
            USBD_LL_CloseEP(pdev, MIDI_STREAM_OUT_EPADDR);
            return 0;
        }

        uint8_t dataInCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            TxDone = true;
            return USBD_OK;
        }

        uint8_t dataOutCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            uint32_t count = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;

            for (uint32_t i = 0; i < count; i++)
                midiRxBufferRing.insert(midiRxBuffer[i]);

            USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(midiRxBuffer), MIDI_IN_OUT_EPSIZE);
            return 0;
        }
    }    // namespace midi

    uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
    {
        uint8_t returnValue = cdc::initCallback(pdev, cfgidx);

        if (returnValue == USBD_OK)
            returnValue = midi::initCallback(pdev, cfgidx);

        return returnValue;
    }

    uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
    {
        uint8_t returnValue = cdc::deInitCallback(pdev, cfgidx);

        if (returnValue == USBD_OK)
            returnValue = midi::deInitCallback(pdev, cfgidx);

        return returnValue;
    }

    uint8_t setupCallback(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
    {
        if ((req->bmRequest == 33) && (req->bRequest == 32))
        {
            return cdc::setupCallback(pdev, req);
        }

        return USBD_OK;
    }

    uint8_t EP0_RxReadyCallback(USBD_HandleTypeDef* pdev)
    {
        if ((pdev->request.bmRequest == 33) && (pdev->request.bRequest == 32))
        {
            return cdc::EP0_RxReadyCallback(pdev);
        }

        return USBD_OK;
    }

    uint8_t dataInCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
    {
        if ((USB_ENDPOINT_DIR_IN | epnum) == CDC_IN_EPADDR)
            return cdc::dataInCallback(pdev, epnum);
        else
            return midi::dataInCallback(pdev, epnum);
    }

    uint8_t dataOutCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
    {
        if ((USB_ENDPOINT_DIR_OUT | epnum) == CDC_OUT_EPADDR)
            return cdc::dataOutCallback(pdev, epnum);
        else
            return midi::dataOutCallback(pdev, epnum);
    }

    int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
    {
        static uint32_t baudRate = 0;

        switch (cmd)
        {
        case CDC_REQ_SetLineEncoding:
        {
            //ignore other parameters
            baudRate = (uint32_t)(pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
            Board::USB::onCDCsetLineEncoding(baudRate);
        }
        break;

        case CDC_REQ_GetLineEncoding:
        {
            pbuf[0] = (uint8_t)(baudRate);
            pbuf[1] = (uint8_t)(baudRate >> 8);
            pbuf[2] = (uint8_t)(baudRate >> 16);
            pbuf[3] = (uint8_t)(baudRate >> 24);

            //set by default
            pbuf[4] = 0;       //1 stop bit
            pbuf[5] = 0;       //no parity
            pbuf[6] = 0x08;    //8bit word
        }
        break;

        default:
            break;
        }

        return USBD_OK;
    }

    uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef* pdev, cdcCallback_t* fops)
    {
        if (fops == NULL)
        {
            return (uint8_t)USBD_FAIL;
        }

        pdev->pUserData = fops;

        return (uint8_t)USBD_OK;
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

        const USB_Descriptor_UID_String_t* desc = USBgetSerialIDString(length, &uid[0]);
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

    USBD_ClassTypeDef USB_MIDI_CDC_Class = {
        initCallback,
        deInitCallback,
        setupCallback,
        NULL,
        EP0_RxReadyCallback,
        dataInCallback,
        dataOutCallback,
        NULL,
        NULL,
        NULL,
        NULL,
        getConfigDescriptor,
        NULL,
        NULL,
    };

    cdcCallback_t USBD_Interface_fops_FS = {
        CDC_Control_FS,
        NULL
    };

    //internal usb init functions used to specify own rx/tx fifo sizes

    USBD_StatusTypeDef _USBD_LL_Init(USBD_HandleTypeDef* pdev)
    {
        if (pdev->id == DEVICE_FS)
        {
            hpcd_USB_OTG_FS.pData = pdev;
            pdev->pData           = &hpcd_USB_OTG_FS;

            hpcd_USB_OTG_FS.Instance                 = USB_OTG_FS;
            hpcd_USB_OTG_FS.Init.dev_endpoints       = 4;
            hpcd_USB_OTG_FS.Init.speed               = PCD_SPEED_FULL;
            hpcd_USB_OTG_FS.Init.dma_enable          = DISABLE;
            hpcd_USB_OTG_FS.Init.phy_itface          = PCD_PHY_EMBEDDED;
            hpcd_USB_OTG_FS.Init.Sof_enable          = DISABLE;
            hpcd_USB_OTG_FS.Init.low_power_enable    = DISABLE;
            hpcd_USB_OTG_FS.Init.lpm_enable          = DISABLE;
            hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
            hpcd_USB_OTG_FS.Init.use_dedicated_ep1   = DISABLE;

            if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
                Board::detail::errorHandler();

            //sizes are given in words (one word = 4 bytes)
            //keep the default for rx fifo
            HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);

            //match the order of defined endpoints
            HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, CONTROL_EPSIZE / 4);
            HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, CDC_IN_OUT_EPSIZE / 4);
            HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, MIDI_IN_OUT_EPSIZE / 4);
            HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 3, CDC_NOTIFICATION_EPSIZE / 4);
        }

        return USBD_OK;
    }

    USBD_StatusTypeDef _USBD_Init(USBD_HandleTypeDef*      pdev,
                                  USBD_DescriptorsTypeDef* pdesc,
                                  uint8_t                  id)
    {
        USBD_StatusTypeDef ret;

        if (pdev == NULL)
            return USBD_FAIL;

        pdev->pClass    = NULL;
        pdev->pUserData = NULL;
        pdev->pConfDesc = NULL;

        if (pdesc != NULL)
            pdev->pDesc = pdesc;

        pdev->dev_state = USBD_STATE_DEFAULT;
        pdev->id        = id;

        ret = _USBD_LL_Init(pdev);

        return ret;
    }
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void usb()
            {
                if (_USBD_Init(&hUsbDeviceFS, &DeviceDescriptor, DEVICE_FS) != USBD_OK)
                    Board::detail::errorHandler();

                if (USBD_RegisterClass(&hUsbDeviceFS, &USB_MIDI_CDC_Class) != USBD_OK)
                    Board::detail::errorHandler();

                if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
                    Board::detail::errorHandler();

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

            if (midiRxBufferRing.count() >= 4)
            {
                midiRxBufferRing.remove(USBMIDIpacket.Event);
                midiRxBufferRing.remove(USBMIDIpacket.Data1);
                midiRxBufferRing.remove(USBMIDIpacket.Data2);
                midiRxBufferRing.remove(USBMIDIpacket.Data3);

                returnValue = true;
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

                return true;
            }
            else
            {
                return false;
            }
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (cdcRxBufferRing.isEmpty())
                return false;

            size = 0;

            size_t loopNr = cdcRxBufferRing.count() > maxSize ? maxSize : cdcRxBufferRing.count();

            for (size_t i = 0; i < loopNr; i++)
            {
                if (cdcRxBufferRing.remove(buffer[i]))
                    size++;
            }

            return true;
        }

        bool readCDC(uint8_t& value)
        {
            if (cdcRxBufferRing.isEmpty())
                return false;

            value = 0;

            return cdcRxBufferRing.remove(value);
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            if (cdcData.TxState)
                return false;

            if (!size)
                return false;

            cdcData.TxState = 1;

            hUsbDeviceFS.ep_in[CDC_IN_EPADDR & 0xFU].total_length = size;
            (void)USBD_LL_Transmit(&hUsbDeviceFS, CDC_IN_EPADDR, buffer, size);

            return true;
        }

        bool writeCDC(uint8_t value)
        {
            if (cdcData.TxState)
                return false;

            cdcData.TxState = 1;

            hUsbDeviceFS.ep_in[CDC_IN_EPADDR & 0xFU].total_length = 1;
            (void)USBD_LL_Transmit(&hUsbDeviceFS, CDC_IN_EPADDR, &value, 1);

            return true;
        }
    }    // namespace USB
}    // namespace Board