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
    IO::TouchscreenBase::Common::bufferCount = 0;
    return _hwa.init();
}

bool Viewtech::deInit()
{
    return _hwa.deInit();
}

bool Viewtech::setScreen(size_t screenID)
{
    screenID &= 0xFF;

    _hwa.write(0xA5);
    _hwa.write(0x5A);
    _hwa.write(0x04);
    _hwa.write(0x80);
    _hwa.write(0x03);
    _hwa.write(0x00);
    _hwa.write(screenID);

    return true;
}

IO::TouchscreenBase::tsEvent_t Viewtech::update(IO::TouchscreenBase::tsData_t& data)
{
    pollXY();

    auto    event = IO::TouchscreenBase::tsEvent_t::none;
    uint8_t value = 0;

    while (_hwa.read(value))
    {
        IO::TouchscreenBase::Common::rxBuffer[IO::TouchscreenBase::Common::bufferCount++] = value;
    }

    // assumption - only one response is received at the time
    // if parsing fails, wipe the buffer
    if (IO::TouchscreenBase::Common::bufferCount)
    {
        // verify header first
        if (IO::TouchscreenBase::Common::rxBuffer[0] == 0xA5)
        {
            if (IO::TouchscreenBase::Common::bufferCount > 1)
            {
                if (IO::TouchscreenBase::Common::rxBuffer[1] == 0x5A)
                {
                    if (IO::TouchscreenBase::Common::bufferCount > 2)
                    {
                        // byte at index 2 holds response length, without first two bytes and without byte at index 2
                        if (IO::TouchscreenBase::Common::bufferCount >= static_cast<size_t>(3 + IO::TouchscreenBase::Common::rxBuffer[2]))
                        {
                            uint32_t response = IO::TouchscreenBase::Common::rxBuffer[2];
                            response <<= 8;
                            response |= IO::TouchscreenBase::Common::rxBuffer[3];
                            response <<= 8;
                            response |= IO::TouchscreenBase::Common::rxBuffer[4];
                            response <<= 8;
                            response |= IO::TouchscreenBase::Common::rxBuffer[5];

                            switch (response)
                            {
                            case static_cast<uint32_t>(response_t::buttonStateChange):
                            {
                                data.buttonState = IO::TouchscreenBase::Common::rxBuffer[6];
                                data.buttonID    = IO::TouchscreenBase::Common::rxBuffer[7];

                                event = IO::TouchscreenBase::tsEvent_t::button;
                            }
                            break;

                            case static_cast<uint32_t>(response_t::xyUpdate):
                            {
                                if ((IO::TouchscreenBase::Common::rxBuffer[6] == 0x01) || (IO::TouchscreenBase::Common::rxBuffer[6] == 0x03))
                                {
                                    data.xPos = IO::TouchscreenBase::Common::rxBuffer[7] << 8;
                                    data.xPos |= IO::TouchscreenBase::Common::rxBuffer[8];

                                    data.yPos = IO::TouchscreenBase::Common::rxBuffer[9] << 8;
                                    data.yPos |= IO::TouchscreenBase::Common::rxBuffer[10];
                                    data.pressType = IO::TouchscreenBase::Common::rxBuffer[6] == 0x01 ? IO::TouchscreenBase::pressType_t::initial : IO::TouchscreenBase::pressType_t::hold;

                                    event = IO::TouchscreenBase::tsEvent_t::coordinate;
                                }
                                else
                                {
                                    // screen released
                                    data.xPos      = 0;
                                    data.yPos      = 0;
                                    data.pressType = IO::TouchscreenBase::pressType_t::none;
                                    event          = IO::TouchscreenBase::tsEvent_t::coordinate;
                                }
                            }
                            break;

                            default:
                                break;
                            }

                            IO::TouchscreenBase::Common::bufferCount = 0;
                        }
                    }
                }
                else
                {
                    // header invalid - ignore the rest of the message
                    IO::TouchscreenBase::Common::bufferCount = 0;
                }
            }
        }
        else
        {
            // header invalid - ignore the rest of the message
            IO::TouchscreenBase::Common::bufferCount = 0;
        }
    }

    return event;
}

void Viewtech::setIconState(IO::TouchscreenBase::icon_t& icon, bool state)
{
    // header
    _hwa.write(0xA5);
    _hwa.write(0x5A);

    // request size
    _hwa.write(0x05);

    // write variable
    _hwa.write(0x82);

    // icon address - for viewtech displays, address is stored in xPos element
    _hwa.write(HIGH_BYTE(icon.xPos));
    _hwa.write(LOW_BYTE(icon.xPos));

    // value to set - 2 bytes are used, higher is always 0
    // inverted logic for setting state - 0 means on state, 1 is off
    _hwa.write(0x00);
    _hwa.write(state ? 0x00 : 0x01);
}

bool Viewtech::setBrightness(IO::TouchscreenBase::brightness_t brightness)
{
    // header
    _hwa.write(0xA5);
    _hwa.write(0x5A);

    // request size
    _hwa.write(0x03);

    // register write
    _hwa.write(0x80);

    // brightness settting
    _hwa.write(0x01);

    // brightness value
    _hwa.write(_brightnessMapping[static_cast<uint8_t>(brightness)]);

    return true;
}

void Viewtech::pollXY()
{
    static uint32_t lastPollTime = 0;

    if ((core::timing::currentRunTimeMs() - lastPollTime) > XY_POLL_TIME_MS)
    {
        // header
        _hwa.write(0xA5);
        _hwa.write(0x5A);

        // request size
        _hwa.write(0x03);

        // register read
        _hwa.write(0x81);

        // read register 6 but request 5 bytes
        // reg6 contains touch status (1 byte)
        // reg7 contains x/y coordinates (4 bytes)
        _hwa.write(0x06);

        // read 5 bytes
        _hwa.write(0x05);

        lastPollTime = core::timing::currentRunTimeMs();
    }
}