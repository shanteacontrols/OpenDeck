/*

OpenDECK library v1.1
File: FreePins.cpp
Last revision date: 2014-09-28
Author: Igor Petrovic

*/

#include "OpenDeck.h"
#include <avr/eeprom.h>


void OpenDeck::configureFreePins()  {

    uint8_t pin,
            pinState,
            bRow = 0,
            lRow = 0;

    for (int i=0; i<SYS_EX_FREE_PIN_END; i++)   {

        pin = i;
        pinState = freePinState[i];
        
        if (pinState == SYS_EX_FREE_PIN_STATE_B_ROW)         bRow++;
        else if (pinState == SYS_EX_FREE_PIN_STATE_L_ROW)    lRow++;

        switch (pin)    {

            case SYS_EX_FREE_PIN_A:

            switch (pinState)    {

                case SYS_EX_FREE_PIN_STATE_DISABLED:
                DDRB    &=  0b11101111;
                PORTB   |=  0b00000000;
                break;

                case SYS_EX_FREE_PIN_STATE_B_ROW:
                DDRB    &=  0b11101111;
                PORTB   |=  0b00010000;
                break;

                case SYS_EX_FREE_PIN_STATE_L_ROW:
                DDRB    |=  0b00010000;
                PORTB   &=  0b11101111;
                break;
                
                default:
                break;

            }

            break;

            case SYS_EX_FREE_PIN_B:

            switch (pinState)    {

                case SYS_EX_FREE_PIN_STATE_DISABLED:
                DDRB    &=  0b11011111;
                PORTB   |=  0b00000000;
                break;

                case SYS_EX_FREE_PIN_STATE_B_ROW:
                DDRB    &=  0b11011111;
                PORTB   |=  0b00100000;
                break;

                case SYS_EX_FREE_PIN_STATE_L_ROW:
                DDRB    |=  0b00100000;
                PORTB   &=  0b11011111;
                break;

                default:
                break;

            }

            break;

            case SYS_EX_FREE_PIN_C:

            switch (pinState)    {

                case SYS_EX_FREE_PIN_STATE_DISABLED:
                DDRD    &=  0b11110111;
                PORTD   |=  0b00000000;
                break;

                case SYS_EX_FREE_PIN_STATE_B_ROW:
                DDRD    &=  0b11110111;
                PORTD   |=  0b00001000;
                break;

                case SYS_EX_FREE_PIN_STATE_L_ROW:
                DDRD    |=  0b00001000;
                PORTD   &=  0b11110111;
                break;

                default:
                break;

            }

            break;

            case SYS_EX_FREE_PIN_D:

            switch (pinState)    {

                case SYS_EX_FREE_PIN_STATE_DISABLED:
                DDRD    &=  0b11111011;
                PORTD   |=  0b00000000;
                break;

                case SYS_EX_FREE_PIN_STATE_B_ROW:
                DDRD    &=  0b11111011;
                PORTD   |=  0b00000100;
                break;

                case SYS_EX_FREE_PIN_STATE_L_ROW:
                DDRD    |=  0b00000100;
                PORTD   &=  0b11111011;
                break;

                default:
                break;

            }

            break;

            default:
            break;

        }

    }

    freePinsAsBRows = bRow;
    freePinsAsLRows = lRow;

}

bool OpenDeck::configureFreePin(uint8_t pin, uint8_t pinState)  {

    freePinState[pin] = pinState;
    configureFreePins();
    eeprom_update_byte((uint8_t*)EEPROM_FREE_PIN_START+pin, pinState);
    return (eeprom_read_byte((uint8_t*)EEPROM_FREE_PIN_START+pin) == pinState);

}

uint8_t OpenDeck::readButtonRowFreePin(uint8_t pin) {

    switch (pin)    {

        case SYS_EX_FREE_PIN_A:
        return ((PINB >> 4) & 0x01);
        break;

        case SYS_EX_FREE_PIN_B:
        return ((PINB >> 5) & 0x01);
        break;

        case SYS_EX_FREE_PIN_C:
        return ((PIND >> 3) & 0x01);
        break;

        case SYS_EX_FREE_PIN_D:
        return ((PIND >> 2) & 0x01);
        break;

        default:
        return 0;
        break;

    }   return 0;

}

void OpenDeck::ledRowOffFreePin(uint8_t pin)    {

    switch (pin)    {

        case SYS_EX_FREE_PIN_A:
        PORTB   &=  0b11101111;
        break;

        case SYS_EX_FREE_PIN_B:
        PORTB   &=  0b11011111;
        break;

        case SYS_EX_FREE_PIN_C:
        PORTD   &=  0b11110111;
        break;

        case SYS_EX_FREE_PIN_D:
        PORTD   &=  0b11111011;
        break;

        default:
        break;

    }

}

void OpenDeck::ledRowOnFreePin(uint8_t pin)    {

    switch (pin)    {

        case SYS_EX_FREE_PIN_A:
        PORTB   |=  0b00010000;
        break;

        case SYS_EX_FREE_PIN_B:
        PORTB   |=  0b00100000;
        break;

        case SYS_EX_FREE_PIN_C:
        PORTD   |=  0b00001000;
        break;

        case SYS_EX_FREE_PIN_D:
        PORTD   |=  0b00000100;
        break;

        default:
        break;

    }

}