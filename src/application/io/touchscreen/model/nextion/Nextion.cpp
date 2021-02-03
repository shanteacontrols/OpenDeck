/*

Copyright 2015-2021 Igor Petrovic

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

#include "Nextion.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

bool Nextion::init()
{
    IO::Touchscreen::Model::Common::bufferCount = 0;

    if (hwa.init())
    {
        writeCommand("dims=100");
        return true;
    }

    return false;
}

bool Nextion::deInit()
{
    return hwa.deInit();
}

bool Nextion::setScreen(size_t screenID)
{
    return writeCommand("page %u", screenID);
}

IO::Touchscreen::tsEvent_t Nextion::update(IO::Touchscreen::tsData_t& data)
{
    uint8_t byte;

    if (hwa.read(byte))
        IO::Touchscreen::Model::Common::rxBuffer[IO::Touchscreen::Model::Common::bufferCount++] = byte;
    else
        return IO::Touchscreen::tsEvent_t::none;

    if (byte == 0xFF)
    {
        endCounter++;
    }
    else
    {
        if (endCounter)
            endCounter = 0;
    }

    if (endCounter == 3)
    {
        //new message received
        endCounter = 0;

        //handle only button messages for now
        bool messageStart = true;

        while (IO::Touchscreen::Model::Common::bufferCount)
        {
            byte = IO::Touchscreen::Model::Common::rxBuffer[0];

            if ((byte == 0x65) && messageStart)
            {
                if (IO::Touchscreen::Model::Common::bufferCount >= 5)
                {
                    //state
                    //1 - pressed, 0 - released
                    byte = IO::Touchscreen::Model::Common::rxBuffer[1];

                    data.buttonState = byte ? 1 : 0;

                    //button id
                    byte          = IO::Touchscreen::Model::Common::rxBuffer[2];
                    data.buttonID = byte;

                    IO::Touchscreen::Model::Common::bufferCount = 0;
                    return IO::Touchscreen::tsEvent_t::button;
                }
            }
            else
            {
                IO::Touchscreen::Model::Common::bufferCount = 0;
            }

            messageStart = false;
            IO::Touchscreen::Model::Common::bufferCount--;
        }
    }

    return IO::Touchscreen::tsEvent_t::none;
}

void Nextion::setIconState(IO::Touchscreen::icon_t& icon, bool state)
{
    //ignore width/height zero - set either intentionally to avoid display or incorrectly
    if (!icon.width)
        return;

    if (!icon.height)
        return;

    writeCommand("picq %u,%u,%u,%u,%u", icon.xPos, icon.yPos, icon.width, icon.height, state ? icon.onScreen : icon.offScreen);
}

bool Nextion::writeCommand(const char* line, ...)
{
    va_list args;
    va_start(args, line);

    size_t retVal = vsnprintf(commandBuffer, IO::Touchscreen::Model::Common::bufferSize, line, args);

    va_end(args);

    if (retVal < 0)
        return false;

    for (size_t i = 0; i < strlen(commandBuffer); i++)
    {
        if (!hwa.write(commandBuffer[i]))
            return false;
    }

    return endCommand();
}

bool Nextion::endCommand()
{
    for (int i = 0; i < 3; i++)
    {
        if (!hwa.write(0xFF))
            return false;
    }

    return true;
}

bool Nextion::setBrightness(IO::Touchscreen::brightness_t brightness)
{
    return writeCommand("dims=%d", brightnessMapping[static_cast<uint8_t>(brightness)]);
}