/*

Copyright Igor Petrovic

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

#ifdef PROJECT_TARGET_SUPPORT_TOUCHSCREEN

#include "viewtech.h"

#include "application/io/touchscreen/touchscreen.h"

#include "core/mcu.h"
#include "core/util/util.h"

using namespace io::touchscreen;

Viewtech::Viewtech(Hwa& hwa)
    : _hwa(hwa)
{
    Touchscreen::registerModel(model_t::VIEWTECH, this);
}

bool Viewtech::init()
{
    Model::_bufferCount = 0;

    if (_hwa.init())
    {
        // add slight delay to ensure display can receive commands after power on
        core::mcu::timing::waitMs(3000);

        return true;
    }

    return false;
}

bool Viewtech::deInit()
{
    return _hwa.deInit();
}

bool Viewtech::setScreen(size_t index)
{
    index &= 0xFF;

    _hwa.write(0xA5);
    _hwa.write(0x5A);
    _hwa.write(0x04);
    _hwa.write(0x80);
    _hwa.write(0x03);
    _hwa.write(0x00);
    _hwa.write(index);

    return true;
}

tsEvent_t Viewtech::update(Data& data)
{
    auto    event = tsEvent_t::NONE;
    uint8_t value = 0;

    while (_hwa.read(value))
    {
        Model::_rxBuffer[Model::_bufferCount++] = value;
    }

    // assumption - only one response is received at the time
    // if parsing fails, wipe the buffer
    if (Model::_bufferCount)
    {
        // verify header first
        if (Model::_rxBuffer[0] == 0xA5)
        {
            if (Model::_bufferCount > 1)
            {
                if (Model::_rxBuffer[1] == 0x5A)
                {
                    if (Model::_bufferCount > 2)
                    {
                        // byte at index 2 holds response length, without first two bytes and without byte at index 2
                        if (Model::_bufferCount >= static_cast<size_t>(3 + Model::_rxBuffer[2]))
                        {
                            uint32_t response = Model::_rxBuffer[2];
                            response <<= 8;
                            response |= Model::_rxBuffer[3];
                            response <<= 8;
                            response |= Model::_rxBuffer[4];
                            response <<= 8;
                            response |= Model::_rxBuffer[5];

                            switch (response)
                            {
                            case static_cast<uint32_t>(response_t::BUTTON_STATE_CHANGE):
                            {
                                data.buttonState = Model::_rxBuffer[6];
                                data.buttonIndex = Model::_rxBuffer[7];

                                event = tsEvent_t::BUTTON;
                            }
                            break;

                            default:
                                break;
                            }

                            Model::_bufferCount = 0;
                        }
                    }
                }
                else
                {
                    // header invalid - ignore the rest of the message
                    Model::_bufferCount = 0;
                }
            }
        }
        else
        {
            // header invalid - ignore the rest of the message
            Model::_bufferCount = 0;
        }
    }

    return event;
}

void Viewtech::setIconState(Icon& icon, bool state)
{
    // header
    _hwa.write(0xA5);
    _hwa.write(0x5A);

    // request size
    _hwa.write(0x05);

    // write variable
    _hwa.write(0x82);

    // icon address - for viewtech displays, address is stored in xPos element
    _hwa.write(core::util::MSB_U16(icon.xPos));
    _hwa.write(core::util::LSB_U16(icon.xPos));

    // value to set - 2 bytes are used, higher is always 0
    // inverted logic for setting state - 0 means on state, 1 is off
    _hwa.write(0x00);
    _hwa.write(state ? 0x00 : 0x01);
}

bool Viewtech::setBrightness(brightness_t brightness)
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
    _hwa.write(BRIGHTNESS_MAPPING[static_cast<uint8_t>(brightness)]);

    return true;
}

#endif