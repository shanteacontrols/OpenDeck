/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SDW.h"
#include "../../Variables.h"
#include "Commands.h"
#include "board/Board.h"

#define LOW_BYTE(value)  ((value) & 0xFF)
#define HIGH_BYTE(value) (((value) >> 8) & 0xFF)

bool uartReadSDW(uint8_t &data)
{
    return board.uartRead(UART_TOUCHSCREEN_CHANNEL, data);
}

bool uartWriteSDW(uint8_t data)
{
    return board.uartWrite(UART_TOUCHSCREEN_CHANNEL, data);
}

void sdw_init()
{
    bufferIndex_rx = 0;
    displayUpdatePtr = sdw_update;
    setPagePtr = sdw_setPage;
    board.initUART(38400, UART_TOUCHSCREEN_CHANNEL);
}

///
/// \brief Sends data to display.
/// @param [in] value           Byte value.
/// @param [in] messageByteType Type of data to send. Enumerated type. See messageByteType_t.
///
void sdw_sendByte(uint8_t value, messageByteType_t messageByteType)
{
    switch(messageByteType)
    {
        case messageStart:
        //write start byte before value
        uartWriteSDW(START_BYTE);
        uartWriteSDW(value);
        break;

        case messageContent:
        //just write value
        uartWriteSDW(value);
        return;

        case messageSingleByte:
        //start byte, value, end bytes
        uartWriteSDW(START_BYTE);
        uartWriteSDW(value);

        for (int i=0; i<END_CODES; i++)
            uartWriteSDW(endCode[i]);
        break;

        case messageEnd:
        //value first
        uartWriteSDW(value);

        //send message end bytes
        for (int i=0; i<END_CODES; i++)
            uartWriteSDW(endCode[i]);
        break;

        default:
        break;
    }
}

///
/// \brief Parses incoming data from display.
/// \returns True if received message is correct, false otherwise.
///
bool sdw_parse()
{
    //by now, we have complete message
    if (displayRxBuffer[0] != START_BYTE)
    {
        //message is invalid, reset buffer counter and return
        bufferIndex_rx = 0;
        return false;
    }

    if ((displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_ON_ID) || (displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_OFF_ID))
    {
        //button press event
        activeButtonState = false;

        if (displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_ON_ID)
            activeButtonState = true;

        activeButtonID = displayRxBuffer[BUTTON_INDEX_2];
        return true;
    }

    return false;
}

///
/// \brief Checks for incoming data from display.
/// \returns True if there is incoming data, false otherwise.
///
bool sdw_update()
{
    uint8_t data = 0;

    if (!uartReadSDW(data))
        return false;

    bool parse = false;

    if (data == START_BYTE)
    {
        //reset buffer index, this is a new message
        bufferIndex_rx = 0;
    }
    else if (data == endCode[END_CODES-1])
    {
        //this is last byte, start parsing
        parse = true;
    }

    displayRxBuffer[bufferIndex_rx] = data;
    bufferIndex_rx++;

    if (parse)
        return sdw_parse();

    return false;
}

///
/// \brief Switches to requested page on display
/// @param [in] pageID  Index of page to display.
///
void sdw_setPage(uint8_t pageID)
{
    sdw_sendByte(PICTURE_DISPLAY, messageStart);
    sdw_sendByte(HIGH_BYTE(pageID), messageContent);
    sdw_sendByte(LOW_BYTE(pageID), messageEnd);
}

///
/// \brief Sets display brightness.
/// @param [in] type    Backlight type.
/// @param [in] value   Brightness value. Ignore for backlightOff and backlightMax backlight type.
///
void sdw_setBrightness(backlightType_t type, int8_t value)
{
    switch(type)
    {
        case backlightOff:
        sdw_sendByte(BACKLIGHT_OFF, messageSingleByte);
        break;

        case backlightMax:
        sdw_sendByte(BACKLIGHT_ON_MAX, messageSingleByte);
        break;

        case backlightPwm:
        sdw_sendByte(BACKLIGHT_ON_MAX, messageStart);

        //check if value is valid
        if (value > PWM_BACKLIGHT_MAX)
        {
            value = PWM_BACKLIGHT_MAX;
        }
        else if (value < 0)
        {
            value = 0;
        }

        sdw_sendByte(value, messageEnd);
        break;

        default:
        return;
    }
}
