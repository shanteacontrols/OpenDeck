/*

Copyright 2015-2019 Igor Petrovic

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

#include "Descriptors.h"
#include "midi/src/MIDI.h"
#include "board/Board.h"

namespace
{
    ///
    /// \brief MIDI Class Device Mode Configuration and State Structure.
    ///
    USB_ClassInfo_MIDI_Device_t MIDI_Interface;
}    // namespace

///
/// \brief Event handler for the USB_ConfigurationChanged event.
/// This is fired when the host set the current configuration
/// of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
///
void EVENT_USB_Device_ConfigurationChanged(void)
{
    /* Setup MIDI Data Endpoints */
    Endpoint_ConfigureEndpoint(MIDI_STREAM_IN_EPADDR, EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
    Endpoint_ConfigureEndpoint(MIDI_STREAM_OUT_EPADDR, EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
}

namespace Board
{
    namespace setup
    {
        void usb()
        {
            MIDI_Interface.Config.StreamingInterfaceNumber = INTERFACE_ID_AudioStream;

            MIDI_Interface.Config.DataINEndpoint.Address = MIDI_STREAM_IN_EPADDR;
            MIDI_Interface.Config.DataINEndpoint.Size = MIDI_STREAM_EPSIZE;
            MIDI_Interface.Config.DataINEndpoint.Banks = 1;

            MIDI_Interface.Config.DataOUTEndpoint.Address = MIDI_STREAM_OUT_EPADDR;
            MIDI_Interface.Config.DataOUTEndpoint.Size = MIDI_STREAM_EPSIZE;
            MIDI_Interface.Config.DataOUTEndpoint.Banks = 1;

            USB_Init();
        }
    }    // namespace setup

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

#ifdef LED_INDICATORS
                Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::usb, Board::midiTrafficDirection_t::incoming);
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

#ifdef LED_INDICATORS
            Board::interface::digital::output::indicateMIDItraffic(MIDI::interface_t::usb, Board::midiTrafficDirection_t::outgoing);
#endif

            return true;
        }

        bool isUSBconnected()
        {
            return (USB_DeviceState == DEVICE_STATE_Configured);
        }
    }    // namespace USB
}    // namespace Board