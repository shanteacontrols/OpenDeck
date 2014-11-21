/*

OpenDECK library v1.2
File: HardwareControl.cpp
Last revision date: 2014-11-22
Author: Igor Petrovic

*/ 

#include "OpenDeck.h"
#include "Ownduino.h"
#include <avr/interrupt.h>

volatile uint8_t column = 0;
volatile uint8_t activeMux = 0;
volatile bool changeSwitch = true;

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
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        _numberOfMux = 2;
        _numberOfColumns = 8;
        _numberOfButtonRows = 4;
        _numberOfLEDrows = 4;
        enableAnalogueInput(1, 6);
        enableAnalogueInput(0, 7);
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

inline void activateColumnInline(uint8_t column, uint8_t board)  {

    switch (board) {

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
        switch (column) {

            case 0:
            PORTC &= 0xC7;
            break;

            case 1:
            PORTC |= (0xC7 | 0x20);
            break;

            case 2:
            PORTC |= (0xC7 | 0x10);
            break;

            case 3:
            PORTC |= (0xC7 | 0x30);
            break;

            case 4:
            PORTC |= (0xC7 | 0x08);
            break;

            case 5:
            PORTC |= (0xC7 | 0x28);
            break;

            case 6:
            PORTC |= (0xC7 | 0x18);
            break;

            case 7:
            PORTC |= (0xC7 | 0x38);
            break;

            default:
            break;

        }
        break;

        default:
        break;

    }

}

void OpenDeck::activateColumn(uint8_t column)   {

    activateColumnInline(column, _board);

}

inline void ledRowOnInline(uint8_t rowNumber, uint8_t board)  {

    switch (board) {

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

        break;
        
        default:
        break;

    }

}

void OpenDeck::ledRowOn(uint8_t rowNumber)  {

    ledRowOnInline(rowNumber, _board);

}

inline void ledRowsOffInline(uint8_t board)   {

    switch (board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        //turn all LED rows off
        PORTB &= 0xEF;
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        PORTB &= 0xF0;

        break;

        default:
        break;

    }

}

void OpenDeck::ledRowsOff()   {

    ledRowsOffInline(_board);

}

inline void setMuxInputInline(uint8_t muxInput, uint8_t board) {

    switch (board) {

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

void OpenDeck::setMuxInput(uint8_t muxInput)    {

    setMuxInputInline(muxInput, _board);

}

inline void readButtonColumnInline(uint8_t &buttonColumnState, uint8_t board)    {

    switch (board) {

        case SYS_EX_BOARD_TYPE_TANNIN:
        buttonColumnState = (PINB & 0x0F);
        break;

        case SYS_EX_BOARD_TYPE_OPEN_DECK_1:
        buttonColumnState = ((PIND >> 4) & 0x0F);

        break;

        default:
        break;

    }

}

void OpenDeck::readButtonColumn(uint8_t &buttonColumnState) {

    readButtonColumnInline(buttonColumnState, _board);

}

ISR(TIMER2_COMPA_vect)  {

    uint8_t _column = column;
    uint8_t _activeMux = activeMux;
    bool _changeSwitch = changeSwitch;
    uint8_t board = openDeck.getBoard();

    switch(_changeSwitch)    {

        case true:
        //switch column
        if (_column == openDeck.getNumberOfColumns()) _column = 0;
        //turn off all LED rows before switching to next column
        ledRowsOffInline(board);
        activateColumnInline(_column, board);
        _column++;
        column = _column;
        break;

        case false:
        //switch analogue input
        if (_activeMux == openDeck.getNumberOfMux())    _activeMux = 0;
        setADCchannel(openDeck.getMuxPin(_activeMux));
        _activeMux++;
        activeMux = _activeMux;
        break;

        default:
        break;

    }

    _changeSwitch = !_changeSwitch;
    changeSwitch = _changeSwitch;

}

uint8_t OpenDeck::getActiveColumn() {

    uint8_t _column = column;

    //return currently active column
    return (_column - 1);

}

uint8_t OpenDeck::getActiveMux() {

    uint8_t _activeMux = activeMux;

    //return currently active column
    return (_activeMux - 1);

}