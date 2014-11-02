/*

OpenDECK library v1.1
File: HardwareControl.cpp
Last revision date: 2014-10-02
Author: Igor Petrovic

*/ 

#include "OpenDeck.h"
#include "Ownduino.h"


void OpenDeck::enableAnalogueInput(uint8_t muxNumber, uint8_t adcChannel)  {

    analogueEnabledArray[muxNumber] = adcChannel;

}

void OpenDeck::initBoard()  {

    initPins();

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        _numberOfMux = 2;
        _numberOfColumns = 5;
        _numberOfButtonRows = 4;
        _numberOfLEDrows = 1;
        enableAnalogueInput(0, 0);
        enableAnalogueInput(1, 1);
        freePinConfEn = false;
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        _numberOfMux = 2;
        _numberOfColumns = 8;
        _numberOfButtonRows = 4;
        _numberOfLEDrows = 4;
        enableAnalogueInput(0, 6);
        enableAnalogueInput(1, 7);
        freePinConfEn = true;
        break;

        default:
        break;

    }

}

void OpenDeck::initPins() {

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        DDRB |= 0x10;
        DDRC = 0x1C;
        DDRD = 0x7E;

        //enable internal pull-up resistors for button rows
        PORTB |= 0x0F;

        //set all columns to HIGH
        PORTD |= 0x7C;
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        DDRD = 0x02;
        DDRB = 0x0F;
        DDRC = 0x3F;

        //enable internal pull-up resistors for button rows
        PORTD = 0xFC;

        //select first column
        PORTC = 0x00;
        break;

        default:
        break;

    }

}

void OpenDeck::activateColumn(uint8_t column)  {

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
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

        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        //column switching is controlled by 74HC238 decoder
        PORTC &= 0xC7;
        PORTC |= (0xC7 | (column << 3));
        break;

        default:
        break;

    }

}

void OpenDeck::ledRowOn(uint8_t rowNumber)  {

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        switch (rowNumber)  {

            case 0:
            PORTB |= 0x10;
            break;

            default:
            break;

        }

        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
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
        
        for (int i=0; i<SYS_EX_FREE_PIN_END; i++)   {
            
            if (freePinState[i] == SYS_EX_FREE_PIN_STATE_L_ROW)
                ledRowOnFreePin(i);

        }

        break;
        
        default:
        break;

    }

}

void OpenDeck::ledRowsOff()   {

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        //turn all LED rows off
        PORTB &= 0xEF;
        break;
        
        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        PORTB &= 0xF0;

        for (int i=0; i<SYS_EX_FREE_PIN_END; i++)   {

            if (freePinState[i] == SYS_EX_FREE_PIN_STATE_L_ROW)
                ledRowOffFreePin(i);

        }
        
        break;

        default:
        break;

    }

}

void OpenDeck::setMuxOutput(uint8_t muxInput) {

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        PORTC = muxInput << 2;
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        PORTC &= 0xF8;
        PORTC |= muxInput;
        break;

        default:
        break;

    }

}

void OpenDeck::readButtonColumn(uint8_t &buttonColumnState)    {

    uint8_t freePinRowCounter = 0;

    switch (_board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        buttonColumnState = (PINB & 0x0F);
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        buttonColumnState = ((PIND >> 4) & 0x0F);

        for (int i=0; i<SYS_EX_FREE_PIN_END; i++)
            if (freePinState[i] == SYS_EX_FREE_PIN_STATE_B_ROW) {

                buttonColumnState |= (readButtonRowFreePin(i) << (_numberOfButtonRows+freePinRowCounter));
                freePinRowCounter++;

            }

        break;

        default:
        break;

    }

}