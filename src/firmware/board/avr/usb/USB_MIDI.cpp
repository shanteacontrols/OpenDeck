#include "../../Board.h"
#include "Descriptors.h"

///
/// \brief MIDI Class Device Mode Configuration and State Structure.
///
USB_ClassInfo_MIDI_Device_t MIDI_Interface;

///
/// \brief Event handler for the USB_ConfigurationChanged event.
/// This is fired when the host set the current configuration
/// of the USB device after enumeration - the device endpoints are configured and the MIDI management task started.
///
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    /* Setup MIDI Data Endpoints */
    ConfigSuccess &= Endpoint_ConfigureEndpoint(MIDI_STREAM_IN_EPADDR, EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
    ConfigSuccess &= Endpoint_ConfigureEndpoint(MIDI_STREAM_OUT_EPADDR, EP_TYPE_BULK, MIDI_STREAM_EPSIZE, 1);
}

bool usbRead(USBMIDIpacket_t& USBMIDIpacket)
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

        MIDIreceived = true;
        return true;
    }
    else
    {
        return false;
    }
}

bool usbWrite(USBMIDIpacket_t& USBMIDIpacket)
{
    if (USB_DeviceState != DEVICE_STATE_Configured)
        return false;

    Endpoint_SelectEndpoint(MIDI_Interface.Config.DataINEndpoint.Address);

    uint8_t ErrorCode;

    if ((ErrorCode = Endpoint_Write_Stream_LE(&USBMIDIpacket, sizeof(USBMIDIpacket_t), NULL)) != ENDPOINT_RWSTREAM_NoError)
        return false;

    if (!(Endpoint_IsReadWriteAllowed()))
        Endpoint_ClearIN();

    MIDI_Device_Flush(&MIDI_Interface);

    MIDIsent = true;

    return true;
}

void Board::initUSB_MIDI()
{
    MIDI_Interface.Config.StreamingInterfaceNumber  = INTERFACE_ID_AudioStream;

    MIDI_Interface.Config.DataINEndpoint.Address    = MIDI_STREAM_IN_EPADDR;
    MIDI_Interface.Config.DataINEndpoint.Size       = MIDI_STREAM_EPSIZE;
    MIDI_Interface.Config.DataINEndpoint.Banks      = 1;

    MIDI_Interface.Config.DataOUTEndpoint.Address   = MIDI_STREAM_OUT_EPADDR;
    MIDI_Interface.Config.DataOUTEndpoint.Size      = MIDI_STREAM_EPSIZE;
    MIDI_Interface.Config.DataOUTEndpoint.Banks     = 1;

    USB_Init();
    midi.handleUSBread(usbRead);
    midi.handleUSBwrite(usbWrite);
}
