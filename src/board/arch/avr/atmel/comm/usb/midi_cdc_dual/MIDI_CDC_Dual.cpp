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
#include "midi/src/MIDI.h"
#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/general/Timing.h"

namespace
{
    USB_ClassInfo_CDC_Device_t             _cdcInterface;
    USB_ClassInfo_MIDI_Device_t            _midiInterface;
    volatile Board::detail::USB::txState_t _txStateCDC;
    volatile Board::detail::USB::txState_t _txStateMIDI;

}    // namespace

/// Event handler for the USB_ConfigurationChanged event.
/// This is fired when the host set the current configuration
/// of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
extern "C" void EVENT_USB_Device_ConfigurationChanged(void)
{
    CDC_Device_ConfigureEndpoints(&_cdcInterface);
    MIDI_Device_ConfigureEndpoints(&_midiInterface);
}

/** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
 *  documentation) by the application code so that the address and size of a requested descriptor can be given
 *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
 *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
 *  USB host.
 */
extern "C" uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint16_t wIndex, const void** const DescriptorAddress)
{
    const uint8_t DescriptorType   = (wValue >> 8);
    const uint8_t DescriptorNumber = (wValue & 0xFF);

    const void* Address = NULL;
    uint16_t    Size    = NO_DESCRIPTOR;

    switch (DescriptorType)
    {
    case DTYPE_Device:
    {
        Address = USBgetDeviceDescriptor(&Size);
    }
    break;

    case DTYPE_Configuration:
    {
        Address = USBgetCfgDescriptor(&Size);
    }
    break;

    case DTYPE_String:
        switch (DescriptorNumber)
        {
        case STRING_ID_Language:
        {
            Address = USBgetLanguageString(&Size);
        }
        break;

        case STRING_ID_Manufacturer:
        {
            Address = USBgetManufacturerString(&Size);
        }
        break;

        case STRING_ID_Product:
        {
            Address = USBgetProductString(&Size);
        }
        break;

        case STRING_ID_UID:
        {
            // handled internally by LUFA
        }
        break;

        default:
            break;
        }
        break;
    }

    *DescriptorAddress = Address;
    return Size;
}

extern "C" void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&_cdcInterface);
    MIDI_Device_ProcessControlRequest(&_midiInterface);
}

extern "C" void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
    Board::USB::onCDCsetLineEncoding(CDCInterfaceInfo->State.LineEncoding.BaudRateBPS);
}

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void usb()
            {
                _cdcInterface.Config.ControlInterfaceNumber = INTERFACE_ID_CDC_CCI;

                _cdcInterface.Config.DataINEndpoint.Address = CDC_IN_EPADDR;
                _cdcInterface.Config.DataINEndpoint.Size    = CDC_IN_OUT_EPSIZE;
                _cdcInterface.Config.DataINEndpoint.Banks   = 1;

                _cdcInterface.Config.DataOUTEndpoint.Address = CDC_OUT_EPADDR;
                _cdcInterface.Config.DataOUTEndpoint.Size    = CDC_IN_OUT_EPSIZE;
                _cdcInterface.Config.DataOUTEndpoint.Banks   = 1;

                _cdcInterface.Config.NotificationEndpoint.Address = CDC_NOTIFICATION_EPADDR;
                _cdcInterface.Config.NotificationEndpoint.Size    = CDC_NOTIFICATION_EPSIZE;
                _cdcInterface.Config.NotificationEndpoint.Banks   = 1;

                _midiInterface.Config.StreamingInterfaceNumber = INTERFACE_ID_AudioStream;

                _midiInterface.Config.DataINEndpoint.Address = MIDI_STREAM_IN_EPADDR;
                _midiInterface.Config.DataINEndpoint.Size    = MIDI_IN_OUT_EPSIZE;
                _midiInterface.Config.DataINEndpoint.Banks   = 1;

                _midiInterface.Config.DataOUTEndpoint.Address = MIDI_STREAM_OUT_EPADDR;
                _midiInterface.Config.DataOUTEndpoint.Size    = MIDI_IN_OUT_EPSIZE;
                _midiInterface.Config.DataOUTEndpoint.Banks   = 1;

                USB_Init();
            }
        }    // namespace setup
    }        // namespace detail

    namespace USB
    {
        bool isUSBconnected()
        {
            return USB_DeviceState == DEVICE_STATE_Configured;
        }

        bool readMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            // device must be connected and configured for the task to run
            if (!isUSBconnected())
                return false;

            // select the MIDI OUT stream
            Endpoint_SelectEndpoint(MIDI_STREAM_OUT_EPADDR);

            // check if a MIDI command has been received
            if (Endpoint_IsOUTReceived())
            {
                // read the MIDI event packet from the endpoint
                Endpoint_Read_Stream_LE(&USBMIDIpacket, sizeof(USBMIDIpacket), NULL);

                // if the endpoint is now empty, clear the bank
                if (!(Endpoint_BytesInEndpoint()))
                    Endpoint_ClearOUT();    // clear the endpoint ready for new packet

                return true;
            }
            else
            {
                return false;
            }
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            static uint32_t timeout = 0;

            if (!isUSBconnected())
                return false;

            // once the transfer fails, wait USB_TX_TIMEOUT_MS ms before trying again
            if (_txStateMIDI != Board::detail::USB::txState_t::done)
            {
                if ((core::timing::currentRunTimeMs() - timeout) < USB_TX_TIMEOUT_MS)
                    return false;

                _txStateMIDI = Board::detail::USB::txState_t::done;
                timeout      = 0;
            }

            Endpoint_SelectEndpoint(_midiInterface.Config.DataINEndpoint.Address);

            uint8_t ErrorCode;

            _txStateMIDI = Board::detail::USB::txState_t::sending;

            if ((ErrorCode = Endpoint_Write_Stream_LE(&USBMIDIpacket, sizeof(MIDI::USBMIDIpacket_t), NULL)) != ENDPOINT_RWSTREAM_NoError)
            {
                _txStateMIDI = Board::detail::USB::txState_t::waiting;
                return false;
            }

            if (!(Endpoint_IsReadWriteAllowed()))
                Endpoint_ClearIN();

            MIDI_Device_Flush(&_midiInterface);

            _txStateMIDI = Board::detail::USB::txState_t::done;
            return true;
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (!isUSBconnected())
                return false;

            size             = 0;
            int16_t readData = -1;

            do
            {
                readData = CDC_Device_ReceiveByte(&_cdcInterface);
                CDC_Device_USBTask(&_cdcInterface);

                if (readData != -1)
                {
                    buffer[size++] = static_cast<uint8_t>(readData);

                    if (size >= maxSize)
                        break;
                }
                else
                {
                    break;
                }
            } while (readData != -1);

            return size > 0;
        }

        bool readCDC(uint8_t& value)
        {
            size_t size;

            return readCDC(&value, size, 1);
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            static uint32_t timeout = 0;

            if (!isUSBconnected())
                return false;

            // once the transfer fails, wait USB_TX_TIMEOUT_MS ms before trying again
            if (_txStateCDC != Board::detail::USB::txState_t::done)
            {
                if ((core::timing::currentRunTimeMs() - timeout) < USB_TX_TIMEOUT_MS)
                    return false;

                _txStateCDC = Board::detail::USB::txState_t::done;
                timeout     = 0;
            }

            for (size_t i = 0; i < size; i++)
            {
                _txStateCDC = Board::detail::USB::txState_t::sending;

                if (CDC_Device_SendByte(&_cdcInterface, (uint8_t)buffer[i]) != ENDPOINT_READYWAIT_NoError)
                {
                    _txStateCDC = Board::detail::USB::txState_t::waiting;
                    return false;
                }
            }

            _txStateCDC = Board::detail::USB::txState_t::done;
            return true;
        }

        bool writeCDC(uint8_t value)
        {
            return writeCDC(&value, 1);
        }
    }    // namespace USB
}    // namespace Board