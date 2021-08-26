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
#include "midi/src/MIDI.h"
#include "board/Board.h"
#include "board/Internal.h"

namespace
{
    USB_ClassInfo_CDC_Device_t  CDC_Interface;
    USB_ClassInfo_MIDI_Device_t MIDI_Interface;
}    // namespace

/// Event handler for the USB_ConfigurationChanged event.
/// This is fired when the host set the current configuration
/// of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
extern "C" void EVENT_USB_Device_ConfigurationChanged(void)
{
    CDC_Device_ConfigureEndpoints(&CDC_Interface);
    MIDI_Device_ConfigureEndpoints(&MIDI_Interface);
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
        Address = USBgetDeviceDescriptor(&Size);
        break;

    case DTYPE_Configuration:
        Address = USBgetCfgDescriptor(&Size);
        break;

    case DTYPE_String:
        switch (DescriptorNumber)
        {
        case STRING_ID_Language:
            Address = USBgetLanguageString(&Size);
            break;
        case STRING_ID_Manufacturer:
            Address = USBgetManufacturerString(&Size);
            break;
        case STRING_ID_Product:
            Address = USBgetProductString(&Size);
            break;
        }
        break;
    }

    *DescriptorAddress = Address;
    return Size;
}

extern "C" void EVENT_USB_Device_ControlRequest(void)
{
    CDC_Device_ProcessControlRequest(&CDC_Interface);
    MIDI_Device_ProcessControlRequest(&MIDI_Interface);
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
                CDC_Interface.Config.ControlInterfaceNumber = INTERFACE_ID_CDC_CCI;

                CDC_Interface.Config.DataINEndpoint.Address = CDC_IN_EPADDR;
                CDC_Interface.Config.DataINEndpoint.Size    = CDC_IN_OUT_EPSIZE;
                CDC_Interface.Config.DataINEndpoint.Banks   = 1;

                CDC_Interface.Config.DataOUTEndpoint.Address = CDC_OUT_EPADDR;
                CDC_Interface.Config.DataOUTEndpoint.Size    = CDC_IN_OUT_EPSIZE;
                CDC_Interface.Config.DataOUTEndpoint.Banks   = 1;

                CDC_Interface.Config.NotificationEndpoint.Address = CDC_NOTIFICATION_EPADDR;
                CDC_Interface.Config.NotificationEndpoint.Size    = CDC_NOTIFICATION_EPSIZE;
                CDC_Interface.Config.NotificationEndpoint.Banks   = 1;

                MIDI_Interface.Config.StreamingInterfaceNumber = INTERFACE_ID_AudioStream;

                MIDI_Interface.Config.DataINEndpoint.Address = MIDI_STREAM_IN_EPADDR;
                MIDI_Interface.Config.DataINEndpoint.Size    = MIDI_IN_OUT_EPSIZE;
                MIDI_Interface.Config.DataINEndpoint.Banks   = 1;

                MIDI_Interface.Config.DataOUTEndpoint.Address = MIDI_STREAM_OUT_EPADDR;
                MIDI_Interface.Config.DataOUTEndpoint.Size    = MIDI_IN_OUT_EPSIZE;
                MIDI_Interface.Config.DataOUTEndpoint.Banks   = 1;

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
            //device must be connected and configured for the task to run
            if (USB_DeviceState != DEVICE_STATE_Configured)
                return false;

            //select the MIDI OUT stream
            Endpoint_SelectEndpoint(MIDI_STREAM_OUT_EPADDR);

            //check if a MIDI command has been received
            if (Endpoint_IsOUTReceived())
            {
                //read the MIDI event packet from the endpoint
                Endpoint_Read_Stream_LE(&USBMIDIpacket, sizeof(USBMIDIpacket), NULL);

                //if the endpoint is now empty, clear the bank
                if (!(Endpoint_BytesInEndpoint()))
                    Endpoint_ClearOUT();    //clear the endpoint ready for new packet

                return true;
            }
            else
            {
                return false;
            }
        }

        bool writeMIDI(MIDI::USBMIDIpacket_t& USBMIDIpacket)
        {
            if (USB_DeviceState != DEVICE_STATE_Configured)
                return false;

            Endpoint_SelectEndpoint(MIDI_Interface.Config.DataINEndpoint.Address);

            uint8_t ErrorCode;

            if ((ErrorCode = Endpoint_Write_Stream_LE(&USBMIDIpacket, sizeof(MIDI::USBMIDIpacket_t), NULL)) != ENDPOINT_RWSTREAM_NoError)
                return false;

            if (!(Endpoint_IsReadWriteAllowed()))
                Endpoint_ClearIN();

            MIDI_Device_Flush(&MIDI_Interface);

            return true;
        }

        bool readCDC(uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            size             = 0;
            int16_t readData = -1;

            do
            {
                readData = CDC_Device_ReceiveByte(&CDC_Interface);
                CDC_Device_USBTask(&CDC_Interface);

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
            int16_t readData = CDC_Device_ReceiveByte(&CDC_Interface);
            CDC_Device_USBTask(&CDC_Interface);

            if (readData != -1)
            {
                value = static_cast<uint8_t>(readData);
                return true;
            }

            return false;
        }

        bool writeCDC(uint8_t* buffer, size_t size)
        {
            for (size_t i = 0; i < size; i++)
            {
                if (CDC_Device_SendByte(&CDC_Interface, (uint8_t)buffer[i]) != ENDPOINT_READYWAIT_NoError)
                    return false;
            }

            return true;
        }

        bool writeCDC(uint8_t value)
        {
            if (CDC_Device_SendByte(&CDC_Interface, (uint8_t)value) != ENDPOINT_READYWAIT_NoError)
                return false;

            return true;
        }
    }    // namespace USB
}    // namespace Board