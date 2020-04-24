/*

Copyright 2015-2020 Igor Petrovic

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
#include "Commands.h"

#define LOW_BYTE(value)  ((value) & (0xFF))
#define HIGH_BYTE(value) (((value) >> 8) & 0xFF)

bool SDW::init()
{
    bufferIndex_rx = 0;
    hwa.init();
    return true;
}

///
/// \brief Switches to requested page on display
/// @param [in] pageID  Index of page to display.
///
void SDW::setScreen(uint8_t screenID)
{
    sendMessage(PICTURE_DISPLAY, messageByteType_t::start);
    sendMessage(HIGH_BYTE(screenID), messageByteType_t::content);
    sendMessage(LOW_BYTE(screenID), messageByteType_t::end);
}

///
/// \brief Sends data to display.
/// @param [in] value           Byte value.
/// @param [in] messageByteType Type of data to send. Enumerated type. See messageByteType_t.
///
void SDW::sendMessage(uint8_t value, messageByteType_t messageByteType)
{
    uint8_t data[10];
    size_t  txSize = 0;

    auto append = [&](uint8_t value) {
        data[txSize] = value;
        txSize++;
    };

    switch (messageByteType)
    {
    case messageByteType_t::start:
        //write start byte before value
        append(START_BYTE);
        append(value);
        break;

    case messageByteType_t::content:
        //just write value
        append(value);
        return;

    case messageByteType_t::singleByte:
        //start byte, value, end bytes
        append(START_BYTE);
        append(value);

        for (int i = 0; i < END_CODES; i++)
            append(endCode[i]);
        break;

    case messageByteType_t::end:
        //value first
        append(value);

        //send message end bytes
        for (int i = 0; i < END_CODES; i++)
            append(endCode[i]);
        break;

    default:
        break;
    }

    hwa.write(data, txSize);
}

///
/// \brief Checks for incoming data from display.
/// \returns True if there is incoming data, false otherwise.
///
bool SDW::update(uint8_t& buttonID, bool& state)
{
    uint8_t data   = 0;
    size_t  rxSize = 0;

    if (!hwa.read(&data, rxSize))
        return false;

    bool parse = false;

    if (data == START_BYTE)
    {
        //reset buffer index, this is a new message
        bufferIndex_rx = 0;
    }
    else if (data == endCode[END_CODES - 1])
    {
        //this is last byte, start parsing
        parse = true;
    }

    displayRxBuffer[bufferIndex_rx] = data;
    bufferIndex_rx++;

    if (parse)
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
            state = false;

            if (displayRxBuffer[COMMAND_ID_INDEX] == BUTTON_ON_ID)
                state = true;

            buttonID = displayRxBuffer[BUTTON_INDEX_2];
            return true;
        }

        return false;
    }

    return false;
}