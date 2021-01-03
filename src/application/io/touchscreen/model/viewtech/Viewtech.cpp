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

#include "Viewtech.h"
#include "core/src/general/Timing.h"

#define LOW_BYTE(value)  ((value) & (0xFF))
#define HIGH_BYTE(value) (((value) >> 8) & 0xFF)

bool Viewtech::init()
{
    IO::Touchscreen::Model::Common::rxBuffer.reset();
    return hwa.init();
}

bool Viewtech::deInit()
{
    return hwa.deInit();
}

bool Viewtech::setScreen(size_t screenID)
{
    screenID &= 0xFF;

    hwa.write(0xA5);
    hwa.write(0x5A);
    hwa.write(0x04);
    hwa.write(0x80);
    hwa.write(0x03);
    hwa.write(0x00);
    hwa.write(screenID);

    return true;
}

///
/// \brief Checks for incoming data from display.
/// \returns True if there is incoming data, false otherwise.
///
bool Viewtech::update(size_t& buttonID, bool& state)
{
    uint8_t data = 0;

    if (hwa.read(data))
        IO::Touchscreen::Model::Common::rxBuffer.insert(data);
    else
        return false;

    if (data == 0xFF)
    {
        endCounter++;
    }
    else
    {
        if (endCounter)
            endCounter = 0;
    }

    if (endCounter == endBytes)
    {
        //new message received
        endCounter = 0;

        //handle only button messages for now

        if (IO::Touchscreen::Model::Common::rxBuffer.count() >= 7)
        {
            uint8_t startHeader[2];

            IO::Touchscreen::Model::Common::rxBuffer.remove(startHeader[0]);
            IO::Touchscreen::Model::Common::rxBuffer.remove(startHeader[1]);

            if ((startHeader[0] == 0xA5) && (startHeader[1] == 0x5A))
            {
                IO::Touchscreen::Model::Common::rxBuffer.remove(data);

                //button press event - 0/release, 1/press
                if ((data == 0x00) || (data == 0x01))
                {
                    state = data;

                    //button id is the next byte
                    IO::Touchscreen::Model::Common::rxBuffer.remove(data);
                    buttonID = data;

                    IO::Touchscreen::Model::Common::rxBuffer.reset();
                    return true;
                }
            }
        }

        IO::Touchscreen::Model::Common::rxBuffer.reset();
        return false;
    }

    return false;
}

void Viewtech::setIconState(IO::Touchscreen::icon_t& icon, bool state)
{
    //header
    hwa.write(0xA5);
    hwa.write(0x5A);

    //request size
    hwa.write(0x05);

    //write variable
    hwa.write(0x82);

    //icon address - for viewtech displays, address is stored in xPos element
    hwa.write(HIGH_BYTE(icon.xPos));
    hwa.write(LOW_BYTE(icon.xPos));

    //value to set - 2 bytes are used, higher is always 0
    //inverted logic for setting state - 0 means on state, 1 is off
    hwa.write(0x00);
    hwa.write(state ? 0x00 : 0x01);
}