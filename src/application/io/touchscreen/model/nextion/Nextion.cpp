/*

Copyright 2015-2022 Igor Petrovic

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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "Nextion.h"
#include "core/src/general/Timing.h"
#include "io/touchscreen/Touchscreen.h"

using namespace IO;

Nextion::Nextion(IO::Touchscreen::HWA& hwa)
    : _hwa(hwa)
{
    IO::Touchscreen::registerModel(IO::Touchscreen::model_t::nextion, this);
}

bool Nextion::init()
{
    Touchscreen::Model::_bufferCount = 0;

    if (_hwa.init())
    {
        endCommand();
        writeCommand("sendxy=1");

        return true;
    }

    return false;
}

bool Nextion::deInit()
{
    return _hwa.deInit();
}

bool Nextion::setScreen(size_t screenID)
{
    return writeCommand("page %u", screenID);
}

Touchscreen::tsEvent_t Nextion::update(Touchscreen::tsData_t& data)
{
    pollXY();

    uint8_t value     = 0;
    bool    process   = false;
    auto    returnVal = Touchscreen::tsEvent_t::none;

    while (_hwa.read(value))
    {
        Touchscreen::Model::_rxBuffer[Touchscreen::Model::_bufferCount++] = value;

        if (value == 0xFF)
        {
            _endCounter++;
        }
        else
        {
            if (_endCounter)
                _endCounter = 0;
        }

        if (_endCounter == 3)
        {
            // new message arrived
            _endCounter = 0;
            process     = true;
            break;
        }
    }

    if (process)
    {
        returnVal                        = response(data);
        Touchscreen::Model::_bufferCount = 0;
    }

    return returnVal;
}

void Nextion::setIconState(Touchscreen::icon_t& icon, bool state)
{
    // ignore width/height zero - set either intentionally to avoid display or incorrectly
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

    int retVal = vsnprintf(_commandBuffer, Touchscreen::Model::BUFFER_SIZE, line, args);

    va_end(args);

    if (retVal < 0)
        return false;

    for (size_t i = 0; i < strlen(_commandBuffer); i++)
    {
        if (!_hwa.write(_commandBuffer[i]))
            return false;
    }

    return endCommand();
}

bool Nextion::endCommand()
{
    for (int i = 0; i < 3; i++)
    {
        if (!_hwa.write(0xFF))
            return false;
    }

    return true;
}

bool Nextion::setBrightness(Touchscreen::brightness_t brightness)
{
    return writeCommand("dims=%d", _brightnessMapping[static_cast<uint8_t>(brightness)]);
}

Touchscreen::tsEvent_t Nextion::response(Touchscreen::tsData_t& data)
{
    bool responseFound = false;
    auto response      = responseID_t::button;    // assumption for now

    for (size_t i = 0; i < static_cast<size_t>(responseID_t::AMOUNT); i++)
    {
        if (Touchscreen::Model::_bufferCount == _responses[i].size)
        {
            if (Touchscreen::Model::_rxBuffer[0] == static_cast<uint8_t>(_responses[i].responseID))
            {
                response      = static_cast<responseID_t>(i);
                responseFound = true;
                break;
            }
        }
    }

    if (responseFound)
    {
        switch (response)
        {
        case responseID_t::button:
        {
            data.buttonState = Touchscreen::Model::_rxBuffer[1];
            data.buttonID    = Touchscreen::Model::_rxBuffer[2];

            return Touchscreen::tsEvent_t::button;
        }
        break;

        case responseID_t::initialFinalCoord:
        {
            data.xPos = Touchscreen::Model::_rxBuffer[1] << 8;
            data.xPos |= Touchscreen::Model::_rxBuffer[2];

            data.yPos = Touchscreen::Model::_rxBuffer[3] << 8;
            data.yPos |= Touchscreen::Model::_rxBuffer[4];

            if (Touchscreen::Model::_rxBuffer[5])
            {
                _screenPressed = true;
                data.pressType = Touchscreen::pressType_t::initial;

                // on each press, restart the xy coordinate retrieval process
                _xyRequestState = xyRequestState_t::xRequest;
            }
            else
            {
                _screenPressed = false;
                data.pressType = Touchscreen::pressType_t::none;
            }

            return Touchscreen::tsEvent_t::coordinate;
        }
        break;

        case responseID_t::coordUpdate:
        {
            bool xyUpdated = false;

            switch (_xyRequestState)
            {
            case xyRequestState_t::xRequested:
            {
                _xPos = Touchscreen::Model::_rxBuffer[2] << 8;
                _xPos |= Touchscreen::Model::_rxBuffer[1];

                _xyRequestState = xyRequestState_t::yRequest;
            }
            break;

            case xyRequestState_t::yRequested:
            {
                data.xPos = _xPos;

                data.yPos = Touchscreen::Model::_rxBuffer[2] << 8;
                data.yPos |= Touchscreen::Model::_rxBuffer[1];

                _xyRequestState = xyRequestState_t::xRequest;
                xyUpdated       = true;
            }
            break;

            default:
                break;
            }

            if (xyUpdated && _screenPressed)
            {
                data.pressType = Touchscreen::pressType_t::hold;
                return Touchscreen::tsEvent_t::coordinate;
            }
            else
            {
                return Touchscreen::tsEvent_t::none;
            }
        }
        break;

        default:
            return Touchscreen::tsEvent_t::none;
        }
    }

    return Touchscreen::tsEvent_t::none;
}

void Nextion::pollXY()
{
    static uint32_t lastPollTime = 0;

    if (!_screenPressed)
        return;

    if ((core::timing::currentRunTimeMs() - lastPollTime) > XY_POLL_TIME_MS)
    {
        switch (_xyRequestState)
        {
        case xyRequestState_t::xRequest:
        {
            writeCommand("get tch0");
            _xyRequestState = xyRequestState_t::xRequested;
        }
        break;

        case xyRequestState_t::yRequest:
        {
            writeCommand("get tch1");
            _xyRequestState = xyRequestState_t::yRequested;
        }
        break;

        default:
            break;
        }

        lastPollTime = core::timing::currentRunTimeMs();
    }
}