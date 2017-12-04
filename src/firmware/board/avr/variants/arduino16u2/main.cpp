#include "Board.h"

Board board;
MIDI midi;

int main(void)
{
    board.init();

    midi.setUSBMIDIstate(true);
    midi.setDINMIDIstate(true);
    midi.setOneByteParseDINstate(true);

    setLow(LED_OUT_PORT, LED_OUT_PIN);
    setLow(LED_IN_PORT, LED_IN_PIN);
    wait_ms(200);
    setHigh(LED_OUT_PORT, LED_OUT_PIN);
    setHigh(LED_IN_PORT, LED_IN_PIN);

    sei();

    while (1)
    {
        //route data from uart to usb
        if (midi.read(dinInterface, THRU_FULL_USB))
        {
            MIDIsent = true;
        }

        //route data from usb to uart
        if (midi.read(usbInterface, THRU_FULL_DIN))
        {
            MIDIreceived = true;
        }
    }
}

