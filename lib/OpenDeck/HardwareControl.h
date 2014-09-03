/*

OpenDECK library v1.96
File: HardwareControl.h
Last revision date: 2014-09-03
Author: Igor Petrovic

*/


#ifndef HARDWARECONTROL_H_
#define HARDWARECONTROL_H_

#define BOARD_OPEN_DECK_1   0x01
#define BOARD_TANNIN        0x02

class HCTannin {

    public:

    //initialize pins
    static void initPins() {

        DDRB |= 0x10;
        DDRC = 0x1C;
        DDRD = 0x7E;

        //enable internal pull-up resistors for button rows
        PORTB |= 0x0F;

        //set all columns to HIGH
        PORTD |= 0x7C;

    }

    //switch to next matrix column
    static void activateColumn(uint8_t column)  {

        //there can only be one column active at the time, the rest is set to HIGH
        switch (column)  {

            case 0:
            PORTD = 0x7A;
            break;

            case 1:
            PORTD = 0x76;
            break;

            case 2:
            PORTD = 0x6E;
            break;

            case 3:
            PORTD = 0x5E;
            break;

            case 4:
            PORTD = 0x3E;
            break;

            default:
            break;

        }

    }

    //turn led row on
    static void ledRowOn(uint8_t rowNumber)  {

        switch (rowNumber)  {

            case 0:
            PORTB |= 0x10;
            break;

            default:
            break;

        }

    }

    //turn all LED rows off
    static void ledRowsOff()   {

        //turn all LED rows off
        PORTB &= 0xEF;

    }

    //control select pins on mux
    static void setMuxOutput(uint8_t muxInput) {

        PORTC = muxInput << 2;

    }

    //get button readings from all rows in matrix
    static void readButtons(uint8_t &buttonColumnState)    {

        //get the readings from all the rows
        buttonColumnState = (PINB & 0x0F);

    }

};

class HCOpenDeck1 {

    public:

    static void initPins() {

        //set in/out pins
        DDRD = 0x02;
        DDRB = 0x0F;
        DDRC = 0x3F;

        //enable internal pull-up resistors for button rows
        PORTD = 0xFC;

        //select first column
        PORTC = 0x00;

    }

    //switch to next matrix column
    static void activateColumn(uint8_t column) {

        //column switching is controlled by 74HC238 decoder
        PORTC &= 0xC7;
        PORTC |= (0xC7 | (column << 3));

    }

    //turn led row on
    static void ledRowOn(uint8_t rowNumber)    {

        switch (rowNumber)  {

            case 0:
            //turn on first LED row
            PORTB |= 0x01;
            break;

            case 1:
            //turn on second LED row
            PORTB |= 0x02;
            break;

            case 2:
            //turn on third LED row
            PORTB |= 0x04;
            break;

            case 3:
            //turn on fourth LED row
            PORTB |= 0x08;
            break;

            default:
            break;

        }

    }

    //turn all LED rows off
    static void ledRowsOff()   {

        PORTB &= 0xF0;

    }

    //control select pins on mux
    static void setMuxOutput(uint8_t muxInput) {

        PORTC &= 0xF8;
        PORTC |= muxInput;

    }

    //get button readings from all rows in matrix
    static void readButtons(uint8_t &buttonColumnState)    {

        buttonColumnState = ((PIND >> 4) & 0x0F);

    }

};

#endif /* HARDWARECONTROL_H_ */