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

#include "board/common/usb/descriptors/Descriptors.h"
#include "midi/src/MIDI.h"
#include "board/Board.h"
#include "board/Internal.h"

namespace
{
    /// MIDI Class Device Mode Configuration and State Structure.
    USB_ClassInfo_MIDI_Device_t MIDI_Interface;
}    // namespace

/// Event handler for the USB_ConfigurationChanged event.
/// This is fired when the host set the current configuration
/// of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
extern "C" void EVENT_USB_Device_ConfigurationChanged(void)
{
    /* Setup MIDI Data Endpoints */
    Endpoint_ConfigureEndpoint(MIDI_STREAM_IN_EPADDR, USB_EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
    Endpoint_ConfigureEndpoint(MIDI_STREAM_OUT_EPADDR, USB_EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
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

namespace Board
{
    namespace detail
    {
        namespace setup
        {
            void usb()
            {
                MIDI_Interface.Config.StreamingInterfaceNumber = INTERFACE_ID_AudioStream;

                MIDI_Interface.Config.DataINEndpoint.Address = MIDI_STREAM_IN_EPADDR;
                MIDI_Interface.Config.DataINEndpoint.Size    = MIDI_STREAM_EPSIZE;
                MIDI_Interface.Config.DataINEndpoint.Banks   = 1;

                MIDI_Interface.Config.DataOUTEndpoint.Address = MIDI_STREAM_OUT_EPADDR;
                MIDI_Interface.Config.DataOUTEndpoint.Size    = MIDI_STREAM_EPSIZE;
                MIDI_Interface.Config.DataOUTEndpoint.Banks   = 1;

                USB_Init();
            }
        }    // namespace setup
    }        // namespace detail

    namespace USB
    {
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

#ifdef FW_APP
#ifdef LED_INDICATORS
                Board::detail::io::indicateMIDItraffic(MIDI::interface_t::usb, Board::detail::midiTrafficDirection_t::incoming);
#endif
#endif

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

#ifdef FW_APP
#ifdef LED_INDICATORS
            Board::detail::io::indicateMIDItraffic(MIDI::interface_t::usb, Board::detail::midiTrafficDirection_t::outgoing);
#endif
#endif

            return true;
        }
    }    // namespace USB
}    // namespace Board