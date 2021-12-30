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

#include "board/common/comm/usb/USB.h"
#include "usbd_core.h"
#include "midi/src/MIDI.h"
#include "core/src/general/RingBuffer.h"
#include "core/src/general/Atomic.h"
#include "core/src/general/Timing.h"
#include "core/src/general/StringBuilder.h"
#include "board/Board.h"
#include "board/Internal.h"
#include <comm/usb/USB.h>

/// Buffer size in bytes for incoming and outgoing MIDI messages (from device standpoint).

#define RX_BUFFER_SIZE_RING 4096

namespace
{
    typedef struct
    {
        uint32_t data[CDC_NOTIFICATION_EPSIZE / 4U]; /* Force 32bits alignment */
        uint8_t  cmdOpCode;
        uint8_t  cmdLength;
    } cdcData_t;

    USBD_HandleTypeDef                             _usbHandler;
    cdcData_t                                      _cdcData;
    volatile uint8_t                               _midiRxBuffer[MIDI_IN_OUT_EPSIZE];
    volatile uint8_t                               _cdcRxBuffer[CDC_IN_OUT_EPSIZE];
    volatile Board::detail::USB::txState_t         _txStateMIDI;
    volatile Board::detail::USB::txState_t         _txStateCDC;
    volatile bool                                  _cdcBufferWait;
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE_RING> _cdcRxBufferRing;
    core::RingBuffer<uint8_t, RX_BUFFER_SIZE_RING> _midiRxBufferRing;

    namespace cdc
    {
        uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            _txStateCDC = Board::detail::USB::txState_t::done;

            USBD_LL_OpenEP(pdev, CDC_IN_EPADDR, USB_EP_TYPE_BULK, CDC_IN_OUT_EPSIZE);
            pdev->ep_in[CDC_IN_EPADDR & 0xFU].is_used = 1U;

            USBD_LL_OpenEP(pdev, CDC_OUT_EPADDR, USB_EP_TYPE_BULK, CDC_IN_OUT_EPSIZE);
            pdev->ep_out[CDC_OUT_EPADDR & 0xFU].is_used           = 1U;
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].bInterval = CDC_POLLING_TIME;

            USBD_LL_OpenEP(pdev, CDC_NOTIFICATION_EPADDR, USBD_EP_TYPE_INTR, CDC_NOTIFICATION_EPSIZE);
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].is_used = 1U;

            // prepare out endpoint to receive next packet
            return USBD_LL_PrepareReceive(pdev, CDC_OUT_EPADDR, (uint8_t*)_cdcRxBuffer, CDC_IN_OUT_EPSIZE);
        }

        uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            USBD_LL_CloseEP(pdev, CDC_IN_EPADDR);
            pdev->ep_in[CDC_IN_EPADDR & 0xFU].is_used = 0U;

            USBD_LL_CloseEP(pdev, CDC_OUT_EPADDR);
            pdev->ep_out[CDC_OUT_EPADDR & 0xFU].is_used = 0U;

            USBD_LL_CloseEP(pdev, CDC_NOTIFICATION_EPADDR);
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].is_used   = 0U;
            pdev->ep_in[CDC_NOTIFICATION_EPADDR & 0xFU].bInterval = 0U;

            return USBD_OK;
        }

        uint8_t cdcControlCallback(uint8_t cmd, uint8_t* pbuf, uint16_t length)
        {
            static uint32_t baudRate = 0;

            switch (cmd)
            {
            case CDC_REQ_SetLineEncoding:
            {
                // ignore other parameters
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

                // set by default
                pbuf[4] = 0;       // 1 stop bit
                pbuf[5] = 0;       // no parity
                pbuf[6] = 0x08;    // 8bit word
            }
            break;

            default:
                break;
            }

            return USBD_OK;
        }

        uint8_t setupCallback(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
        {
            uint8_t            ifalt      = 0U;
            uint16_t           statusInfo = 0U;
            USBD_StatusTypeDef status     = USBD_OK;

            switch (req->bmRequest & USB_REQ_TYPE_MASK)
            {
            case USB_REQ_TYPE_CLASS:
            {
                if (req->wLength != 0U)
                {
                    if ((req->bmRequest & 0x80U) != 0U)
                    {
                        cdcControlCallback(req->bRequest, (uint8_t*)_cdcData.data, req->wLength);
                        USBD_CtlSendData(pdev, (uint8_t*)_cdcData.data, req->wLength);
                    }
                    else
                    {
                        _cdcData.cmdOpCode = req->bRequest;
                        _cdcData.cmdLength = (uint8_t)req->wLength;

                        USBD_CtlPrepareRx(pdev, (uint8_t*)_cdcData.data, req->wLength);
                    }
                }
                else
                {
                    cdcControlCallback(req->bRequest, (uint8_t*)req, 0U);
                }
            }
            break;

            case USB_REQ_TYPE_STANDARD:
            {
                switch (req->bRequest)
                {
                case USB_REQ_GET_STATUS:
                {
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        USBD_CtlSendData(pdev, (uint8_t*)&statusInfo, 2U);
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        status = USBD_FAIL;
                    }
                }
                break;

                case USB_REQ_GET_INTERFACE:
                {
                    if (pdev->dev_state == USBD_STATE_CONFIGURED)
                    {
                        USBD_CtlSendData(pdev, &ifalt, 1U);
                    }
                    else
                    {
                        USBD_CtlError(pdev, req);
                        status = USBD_FAIL;
                    }
                }
                break;

                case USB_REQ_SET_INTERFACE:
                {
                    if (pdev->dev_state != USBD_STATE_CONFIGURED)
                    {
                        USBD_CtlError(pdev, req);
                        status = USBD_FAIL;
                    }
                }
                break;

                case USB_REQ_CLEAR_FEATURE:
                    break;

                default:
                {
                    USBD_CtlError(pdev, req);
                    status = USBD_FAIL;
                }
                break;
                }
            }
            break;

            default:
            {
                USBD_CtlError(pdev, req);
                status = USBD_FAIL;
            }
            break;
            }

            return status;
        }

        uint8_t EP0_RxReadyCallback(USBD_HandleTypeDef* pdev)
        {
            if (_cdcData.cmdOpCode != 0xFFU)
            {
                cdcControlCallback(_cdcData.cmdOpCode, (uint8_t*)_cdcData.data, (uint16_t)_cdcData.cmdLength);
                _cdcData.cmdOpCode = 0xFFU;
            }

            return USBD_OK;
        }

        uint8_t dataInCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            PCD_HandleTypeDef* hpcd = (PCD_HandleTypeDef*)pdev->pData;

            if ((pdev->ep_in[epnum].total_length > 0U) &&
                ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
            {
                pdev->ep_in[epnum].total_length = 0U;

                // send ZLP (zero length packet)
                return USBD_LL_Transmit(pdev, epnum, NULL, 0U);
            }
            else
            {
                _txStateCDC = Board::detail::USB::txState_t::done;
                return USBD_OK;
            }
        }

        uint8_t dataOutCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            uint32_t length = USBD_LL_GetRxDataSize(pdev, epnum);

            for (uint32_t i = 0; i < length; i++)
                _cdcRxBufferRing.insert(_cdcRxBuffer[i]);

            // make sure the data is removed from buffer by application before declaring endpoint ready

            if ((RX_BUFFER_SIZE_RING - _cdcRxBufferRing.count()) > CDC_IN_OUT_EPSIZE)
            {
                _cdcBufferWait = false;
                return USBD_LL_PrepareReceive(pdev, CDC_OUT_EPADDR, (uint8_t*)_cdcRxBuffer, CDC_IN_OUT_EPSIZE);
            }
            else
            {
                _cdcBufferWait = true;
            }

            return USBD_OK;
        }
    }    // namespace cdc

    namespace midi
    {
        uint8_t initCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            USBD_LL_OpenEP(pdev, MIDI_STREAM_IN_EPADDR, USB_EP_TYPE_BULK, MIDI_IN_OUT_EPSIZE);
            USBD_LL_OpenEP(pdev, MIDI_STREAM_OUT_EPADDR, USB_EP_TYPE_BULK, MIDI_IN_OUT_EPSIZE);
            USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(_midiRxBuffer), MIDI_IN_OUT_EPSIZE);

            _txStateMIDI = Board::detail::USB::txState_t::done;

            return USBD_OK;
        }

        uint8_t deInitCallback(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
        {
            USBD_LL_CloseEP(pdev, MIDI_STREAM_IN_EPADDR);
            USBD_LL_CloseEP(pdev, MIDI_STREAM_OUT_EPADDR);

            return USBD_OK;
        }

        uint8_t dataInCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            _txStateMIDI = Board::detail::USB::txState_t::done;

            return USBD_OK;
        }

        uint8_t dataOutCallback(USBD_HandleTypeDef* pdev, uint8_t epnum)
        {
            uint32_t count = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;

            for (uint32_t i = 0; i < count; i++)
                _midiRxBufferRing.insert(_midiRxBuffer[i]);

            return USBD_LL_PrepareReceive(pdev, MIDI_STREAM_OUT_EPADDR, (uint8_t*)(_midiRxBuffer), MIDI_IN_OUT_EPSIZE);
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
        return cdc::setupCallback(pdev, req);

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

    uint8_t* getDeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        const USB_Descriptor_Device_t* desc = USBgetDeviceDescriptor(length);
        return (uint8_t*)desc;
    }

    uint8_t* getLangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        const USB_Descriptor_String_t* desc = USBgetLanguageString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        const USB_Descriptor_String_t* desc = USBgetManufacturerString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        const USB_Descriptor_String_t* desc = USBgetProductString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getSerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        Board::uniqueID_t uid;
        Board::uniqueID(uid);

        const USB_Descriptor_UID_String_t* desc = USBgetSerialIDString(length, &uid[0]);
        return (uint8_t*)desc;
    }

    uint8_t* getConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
        const USB_Descriptor_String_t* desc = USBgetManufacturerString(length);
        return (uint8_t*)desc;
    }

    uint8_t* getInterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
    {
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

    // internal usb init functions used to specify own rx/tx fifo sizes

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

            // sizes are given in words (one word = 4 bytes)
            // keep the default for rx fifo
            HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);

            // match the order of defined endpoints
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
        if (pdev == NULL)
            return USBD_FAIL;

        pdev->pClass    = NULL;
        pdev->pUserData = NULL;
        pdev->pConfDesc = NULL;

        if (pdesc != NULL)
            pdev->pDesc = pdesc;

        pdev->dev_state = USBD_STATE_DEFAULT;
        pdev->id        = id;

        return _USBD_LL_Init(pdev);
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
                if (_USBD_Init(&_usbHandler, &DeviceDescriptor, DEVICE_FS) != USBD_OK)
                    Board::detail::errorHandler();

                if (USBD_RegisterClass(&_usbHandler, &USB_MIDI_CDC_Class) != USBD_OK)
                    Board::detail::errorHandler();

                USBD_Start(&_usbHandler);
            }
        }    // namespace setup
    }        // namespace detail

    namespace USB
    {
        bool isUSBconnected()
        {
            return (_usbHandler.dev_state == USBD_STATE_CONFIGURED);
        }

        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            bool returnValue = false;

            if (_midiRxBufferRing.count() >= 4)
            {
                _midiRxBufferRing.remove(USBMIDIpacket.Event);
                _midiRxBufferRing.remove(USBMIDIpacket.Data1);
                _midiRxBufferRing.remove(USBMIDIpacket.Data2);
                _midiRxBufferRing.remove(USBMIDIpacket.Data3);

                returnValue = true;
            }

            return returnValue;
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            if (!isUSBconnected())
                return false;

            if (_txStateMIDI != Board::detail::USB::txState_t::done)
            {
                if (_txStateMIDI == Board::detail::USB::txState_t::waiting)
                {
                    return false;
                }
                else
                {
                    uint32_t currentTime = core::timing::currentRunTimeMs();

                    while ((core::timing::currentRunTimeMs() - currentTime) < USB_TX_TIMEOUT_MS)
                    {
                        if (_txStateMIDI == Board::detail::USB::txState_t::done)
                            break;
                    }

                    if (_txStateMIDI != Board::detail::USB::txState_t::done)
                        _txStateMIDI = Board::detail::USB::txState_t::waiting;
                }
            }

            if (_txStateMIDI == Board::detail::USB::txState_t::done)
            {
                _txStateMIDI = Board::detail::USB::txState_t::sending;
                return USBD_LL_Transmit(&_usbHandler, MIDI_STREAM_IN_EPADDR, (uint8_t*)&USBMIDIpacket, 4) == USBD_OK;
            }

            return false;
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (_cdcRxBufferRing.isEmpty())
                return false;

            size = 0;

            size_t loopNr = _cdcRxBufferRing.count() > maxSize ? maxSize : _cdcRxBufferRing.count();

            for (size_t i = 0; i < loopNr; i++)
            {
                if (_cdcRxBufferRing.remove(buffer[i]))
                {
                    size++;
                }
                else
                {
                    break;
                }
            }

            if (_cdcBufferWait)
            {
                if ((RX_BUFFER_SIZE_RING - _cdcRxBufferRing.count()) > CDC_IN_OUT_EPSIZE)
                {
                    _cdcBufferWait = false;
                    USBD_LL_PrepareReceive(&_usbHandler, CDC_OUT_EPADDR, (uint8_t*)_cdcRxBuffer, CDC_IN_OUT_EPSIZE);
                }
            }

            return true;
        }

        bool readCDC(uint8_t& value)
        {
            size_t size;

            return readCDC(&value, size, 1);
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            if (!isUSBconnected())
                return false;

            if (!size)
                return false;

            if (_txStateCDC != Board::detail::USB::txState_t::done)
            {
                if (_txStateCDC == Board::detail::USB::txState_t::waiting)
                {
                    return false;
                }
                else
                {
                    uint32_t currentTime = core::timing::currentRunTimeMs();

                    while ((core::timing::currentRunTimeMs() - currentTime) < USB_TX_TIMEOUT_MS)
                    {
                        if (_txStateCDC == Board::detail::USB::txState_t::done)
                            break;
                    }

                    if (_txStateCDC != Board::detail::USB::txState_t::done)
                        _txStateCDC = Board::detail::USB::txState_t::waiting;
                }
            }

            if (_txStateCDC == Board::detail::USB::txState_t::done)
            {
                _txStateCDC                                          = Board::detail::USB::txState_t::sending;
                _usbHandler.ep_in[CDC_IN_EPADDR & 0xFU].total_length = size;
                return USBD_LL_Transmit(&_usbHandler, CDC_IN_EPADDR, buffer, size) == USBD_OK;
            }

            return false;
        }

        bool writeCDC(uint8_t value)
        {
            return writeCDC(&value, 1);
        }
    }    // namespace USB
}    // namespace Board