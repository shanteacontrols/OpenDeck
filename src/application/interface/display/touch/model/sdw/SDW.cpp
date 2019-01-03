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

#include "SDW.h"
#include "DataTypes.h"
#include "Commands.h"
#include "board/Board.h"

#define LOW_BYTE(value)  ((value) & 0xFF)
#define HIGH_BYTE(value) (((value) >> 8) & 0xFF)

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
        Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, START_BYTE);
        Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, value);
        break;

        case messageContent:
        //just write value
        Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, value);
        return;

        case messageSingleByte:
        //start byte, value, end bytes
        Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, START_BYTE);
        Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, value);

        for (int i=0; i<END_CODES; i++)
            Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, endCode[i]);
        break;

        case messageEnd:
        //value first
        Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, value);

        //send message end bytes
        for (int i=0; i<END_CODES; i++)
            Board::uartWrite(UART_TOUCHSCREEN_CHANNEL, endCode[i]);
        break;

        default:
        break;
    }
}

///
/// \brief Checks for incoming data from display.
/// \returns True if there is incoming data, false otherwise.
///
bool sdw_update(Touchscreen &base)
{
    uint8_t data = 0;

    if (!Board::uartRead(UART_TOUCHSCREEN_CHANNEL, data))
        return false;

    bool parse = false;

    if (data == START_BYTE)
    {
        //reset buffer index, this is a new message
        base.bufferIndex_rx = 0;
    }
    else if (data == endCode[END_CODES-1])
    {
        //this is last byte, start parsing
        parse = true;
    }

    base.displayRxBuffer[base.bufferIndex_rx] = data;
    base.bufferIndex_rx++;

    if (parse)
    {
        //by now, we have complete message
        if (base.displayRxBuffer[0] != START_BYTE)
        {
            //message is invalid, reset buffer counter and return
            base.bufferIndex_rx = 0;
            return false;
        }

        if ((base.displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_ON_ID) || (base.displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_OFF_ID))
        {
            //button press event
            base.activeButtonState = false;

            if (base.displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_ON_ID)
                base.activeButtonState = true;

            base.activeButtonID = base.displayRxBuffer[BUTTON_INDEX_2];
            return true;
        }

        return false;
    }

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

void sdw_init(Touchscreen &base)
{
    base.bufferIndex_rx = 0;
    base.displayUpdatePtr = sdw_update;
    base.setPagePtr = sdw_setPage;
    Board::initUART(38400, UART_TOUCHSCREEN_CHANNEL);
}