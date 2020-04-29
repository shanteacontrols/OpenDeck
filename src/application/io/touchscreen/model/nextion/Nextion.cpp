#include "Nextion.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

bool Nextion::init()
{
    rxBuffer.reset();
    return hwa.init();
}

bool Nextion::setScreen(size_t screenID)
{
    return writeCommand("page %u", screenID);
}

bool Nextion::update(size_t& buttonID, bool& state)
{
    uint8_t data;

    if (hwa.read(data))
        rxBuffer.insert(data);

    if (data == 0xFF)
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

        while (rxBuffer.count())
        {
            rxBuffer.remove(data);

            if ((data == 0x65) && messageStart)
            {
                if (rxBuffer.count() == 6)
                {
                    //first data is page, don't care about that
                    rxBuffer.remove(data);

                    //next byte is component id
                    rxBuffer.remove(data);

                    buttonID = data;

                    //1 - pressed, 0 - released
                    rxBuffer.remove(data);

                    state = data ? 1 : 0;

                    rxBuffer.reset();
                    return true;
                }
            }
            else
            {
                rxBuffer.reset();
            }

            messageStart = false;
        }
    }

    return false;
}

void Nextion::setIconState(IO::Touchscreen::icon_t& icon, bool state)
{
    writeCommand("picq %u,%u,%u,%u,%u", icon.xPos, icon.yPos, icon.width, icon.height, state ? 1 : 0);
}

bool Nextion::writeCommand(const char* line, ...)
{
    va_list args;
    va_start(args, line);

    size_t retVal = vsnprintf(commandBuffer, bufferSize, line, args);

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