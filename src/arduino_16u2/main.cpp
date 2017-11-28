#include "board/Board.h"
Board board;

int main(void)
{
    board.init();

    midi.setUSBMIDIstate(true);
    midi.setDINMIDIstate(true);
    midi.setOneByteParseDINstate(false);

    //if (checkNewRevision())
    //{
        //for (int i=0; i<3; i++)
        //{
            //#ifdef BOARD_OPEN_DECK
            //setHigh(LED_OUT_PORT, LED_OUT_PIN);
            //setLow(LED_IN_PORT, LED_IN_PIN);
            //_delay_ms(200);
            //setLow(LED_OUT_PORT, LED_OUT_PIN);
            //setHigh(LED_IN_PORT, LED_IN_PIN);
            //_delay_ms(200);
            //#elif defined(BOARD_A_LEO)
            //setLow(LED_OUT_PORT, LED_OUT_PIN);
            //setHigh(LED_IN_PORT, LED_IN_PIN);
            //_delay_ms(200);
            //setHigh(LED_OUT_PORT, LED_OUT_PIN);
            //setLow(LED_IN_PORT, LED_IN_PIN);
            //_delay_ms(200);
            //#endif
        //}
//
        //#ifdef BOARD_OPEN_DECK
        //setLow(LED_OUT_PORT, LED_OUT_PIN);
        //setLow(LED_IN_PORT, LED_IN_PIN);
        //#elif defined(BOARD_A_LEO)
        //setHigh(LED_OUT_PORT, LED_OUT_PIN);
        //setHigh(LED_IN_PORT, LED_IN_PIN);
        //#endif
    //}
    //else
    //{
        setLow(LED_OUT_PORT, LED_OUT_PIN);
        setLow(LED_IN_PORT, LED_IN_PIN);
        _delay_ms(200);
        setHigh(LED_OUT_PORT, LED_OUT_PIN);
        setHigh(LED_IN_PORT, LED_IN_PIN);
        _delay_ms(200);
    //}

    while (1)
    {
        //route data from uart to usb
        if (midi.read(dinInterface, THRU_FULL_USB))
        {
            MIDImessageSent = true;
        }

        //route data from usb to uart
        if (midi.read(usbInterface, THRU_FULL_DIN))
        {
            MIDImessageReceived = true;
        }
    }
}

