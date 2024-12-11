/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_USB
#ifdef FW_APP

#include "board/board.h"
#include "internal.h"
#include "common/communication/usb/descriptors/common/common.h"
#include "common/communication/usb/descriptors/midi_cdc_dual/descriptors.h"

#include "core/mcu.h"
#include "LUFA/Drivers/USB/USB.h"

namespace
{
    constexpr uint32_t                     USB_TX_TIMEOUT_MS = 2000;
    USB_ClassInfo_CDC_Device_t             cdcInterface;
    USB_ClassInfo_MIDI_Device_t            midiInterface;
    volatile board::detail::usb::txState_t txStateCdc;
    volatile board::detail::usb::txState_t txStateMidi;
}    // namespace

/// Event handler for the USB_ConfigurationChanged event.
/// This is fired when the host set the current configuration
/// of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
extern "C" void EVENT_USB_Device_ConfigurationChanged(void)
{
    CDC_Device_ConfigureEndpoints(&cdcInterface);
    MIDI_Device_ConfigureEndpoints(&midiInterface);
}

/// This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
/// documentation) by the application code so that the address and size of a requested descriptor can be given
/// to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
/// is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
/// USB host.
extern "C" uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint16_t wIndex, const void** const DescriptorAddress)    // NOLINT
{
    const uint8_t DESCRIPTOR_TYPE   = (wValue >> 8);
    const uint8_t DESCRIPTOR_NUMBER = (wValue & 0xFF);

    const void* address = NULL;
    uint16_t    size    = 0;

    switch (DESCRIPTOR_TYPE)
    {
    case core::mcu::usb::DESC_TYPE_DEVICE:
    {
        address = board::detail::usb::deviceDescriptor(&size);
    }
    break;

    case core::mcu::usb::DESC_TYPE_CONFIGURATION:
    {
        address = board::detail::usb::cfgDescriptor(&size);
    }
    break;

    case core::mcu::usb::DESC_TYPE_STRING:
    {
        switch (DESCRIPTOR_NUMBER)
        {
        case USB_STRING_ID_LANGUAGE:
        {
            address = board::detail::usb::languageString(&size);
        }
        break;

        case USB_STRING_ID_MANUFACTURER:
        {
            address = board::detail::usb::manufacturerString(&size);
        }
        break;

        case USB_STRING_ID_PRODUCT:
        {
            address = board::detail::usb::productString(&size);
        }
        break;

        case USB_STRING_ID_UID:
        {
            // handled internally by LUFA
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    *DescriptorAddress = address;
    return size;
}

extern "C" void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&cdcInterface);
    MIDI_Device_ProcessControlRequest(&midiInterface);
}

extern "C" void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)    // NOLINT
{
    board::usb::onCdcSetLineEncoding(CDCInterfaceInfo->State.LineEncoding.BaudRateBPS);
}

namespace board
{
    namespace detail::usb
    {
        void init()
        {
            cdcInterface.Config.ControlInterfaceNumber       = USB_INTERFACE_ID_CDC_CCI;
            cdcInterface.Config.DataINEndpoint.Address       = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_IN;
            cdcInterface.Config.DataINEndpoint.Size          = PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_IN_OUT;
            cdcInterface.Config.DataINEndpoint.Banks         = 1;
            cdcInterface.Config.DataOUTEndpoint.Address      = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_OUT;
            cdcInterface.Config.DataOUTEndpoint.Size         = PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_IN_OUT;
            cdcInterface.Config.DataOUTEndpoint.Banks        = 1;
            cdcInterface.Config.NotificationEndpoint.Address = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_CDC_NOTIFICATION;
            cdcInterface.Config.NotificationEndpoint.Size    = PROJECT_MCU_USB_ENDPOINT_SIZE_CDC_NOTIFICATION;
            cdcInterface.Config.NotificationEndpoint.Banks   = 1;
            midiInterface.Config.StreamingInterfaceNumber    = USB_INTERFACE_ID_AUDIO_STREAM;
            midiInterface.Config.DataINEndpoint.Address      = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_IN;
            midiInterface.Config.DataINEndpoint.Size         = PROJECT_MCU_USB_ENDPOINT_SIZE_MIDI_IN_OUT;
            midiInterface.Config.DataINEndpoint.Banks        = 1;
            midiInterface.Config.DataOUTEndpoint.Address     = PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_OUT;
            midiInterface.Config.DataOUTEndpoint.Size        = PROJECT_MCU_USB_ENDPOINT_SIZE_MIDI_IN_OUT;
            midiInterface.Config.DataOUTEndpoint.Banks       = 1;

            USB_Init();
        }

        void deInit()
        {
            USB_Disable();
        }
    }    // namespace detail::usb

    namespace usb
    {
        bool isUsbConnected()
        {
            return USB_DeviceState == DEVICE_STATE_Configured;
        }

        bool readMidi(lib::midi::usb::Packet& packet)
        {
            // device must be connected and configured for the task to run
            if (!isUsbConnected())
            {
                return false;
            }

            // select the MIDI OUT stream
            Endpoint_SelectEndpoint(PROJECT_MCU_USB_ENDPOINT_MIDI_CDC_DUAL_ADDR_MIDI_OUT);

            // check if a MIDI command has been received
            if (Endpoint_IsOUTReceived())
            {
                // read the MIDI event packet from the endpoint
                Endpoint_Read_Stream_LE(&packet.data[0], sizeof(packet.data), NULL);

                // if the endpoint is now empty, clear the bank
                if (!(Endpoint_BytesInEndpoint()))
                {
                    Endpoint_ClearOUT();    // clear the endpoint ready for new packet
                }

                return true;
            }

            return false;
        }

        bool writeMidi(lib::midi::usb::Packet& packet)
        {
            static uint32_t timeout = 0;

            if (!isUsbConnected())
            {
                return false;
            }

            // once the transfer fails, wait USB_TX_TIMEOUT_MS ms before trying again
            if (txStateMidi != board::detail::usb::txState_t::DONE)
            {
                if ((core::mcu::timing::ms() - timeout) < USB_TX_TIMEOUT_MS)
                {
                    return false;
                }

                txStateMidi = board::detail::usb::txState_t::DONE;
                timeout     = 0;
            }

            Endpoint_SelectEndpoint(midiInterface.Config.DataINEndpoint.Address);

            txStateMidi = board::detail::usb::txState_t::SENDING;

            if (Endpoint_Write_Stream_LE(&packet.data[0], sizeof(packet.data), NULL) != ENDPOINT_RWSTREAM_NoError)
            {
                txStateMidi = board::detail::usb::txState_t::WAITING;
                return false;
            }

            if (!(Endpoint_IsReadWriteAllowed()))
            {
                Endpoint_ClearIN();
            }

            MIDI_Device_Flush(&midiInterface);

            txStateMidi = board::detail::usb::txState_t::DONE;
            return true;
        }

        bool readCdc(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            if (!isUsbConnected())
            {
                return false;
            }

            size             = 0;
            int16_t readData = -1;

            do
            {
                readData = CDC_Device_ReceiveByte(&cdcInterface);
                CDC_Device_USBTask(&cdcInterface);

                if (readData != -1)
                {
                    buffer[size++] = static_cast<uint8_t>(readData);

                    if (size >= maxSize)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            } while (readData != -1);

            return size > 0;
        }

        bool readCdc(uint8_t& value)
        {
            size_t size;

            return readCdc(&value, size, 1);
        }

        bool writeCdc(uint8_t* buffer, size_t size)
        {
            static uint32_t timeout = 0;

            if (!isUsbConnected())
            {
                return false;
            }

            // once the transfer fails, wait USB_TX_TIMEOUT_MS ms before trying again
            if (txStateCdc != board::detail::usb::txState_t::DONE)
            {
                if ((core::mcu::timing::ms() - timeout) < USB_TX_TIMEOUT_MS)
                {
                    return false;
                }

                txStateCdc = board::detail::usb::txState_t::DONE;
                timeout    = 0;
            }

            for (size_t i = 0; i < size; i++)
            {
                txStateCdc = board::detail::usb::txState_t::SENDING;

                if (CDC_Device_SendByte(&cdcInterface, (uint8_t)buffer[i]) != ENDPOINT_READYWAIT_NoError)
                {
                    txStateCdc = board::detail::usb::txState_t::WAITING;
                    return false;
                }
            }

            txStateCdc = board::detail::usb::txState_t::DONE;
            return true;
        }

        bool writeCdc(uint8_t value)
        {
            return writeCdc(&value, 1);
        }
    }    // namespace usb
}    // namespace board

#endif
#endif