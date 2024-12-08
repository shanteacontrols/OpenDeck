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

#include "updater.h"

using namespace updater;

Updater::Updater(Hwa& hwa, const uint32_t uid)
    : _hwa(hwa)
    , UID(uid)
{}

void Updater::feed(uint8_t data)
{
    auto nextStage = [&]()
    {
        if (_currentStage == static_cast<uint8_t>(receiveStage_t::END))
        {
            reset();
            _hwa.apply();
        }
        else
        {
            _stageBytesReceived = 0;
            _currentStage++;
        }
    };

    // continually process start command - this makes sure the fw update process can always restart

    if (_currentStage)
    {
        if (processStart(data) == processStatus_t::COMPLETE)
        {
            reset();
            nextStage();
            return;    // nothing more to do in this case
        }
    }

    processStatus_t result = (this->*_processHandler[_currentStage])(data);

    if (result == processStatus_t::COMPLETE)
    {
        nextStage();
    }
    else if (result == processStatus_t::INVALID)
    {
        // in this case, restart the procedure
        reset();

        // also reset again if it fails again - it cannot possibly return complete status since that requires 8 bytes
        if ((this->*_processHandler[_currentStage])(data) == processStatus_t::INVALID)
        {
            reset();
        }
    }
}

Updater::processStatus_t Updater::processStart(uint8_t data)
{
    // 8 received bytes must match the startCommand (lower first, then upper)

    if (((START_COMMAND >> (_startBytesReceived * 8)) & static_cast<uint64_t>(0xFF)) != data)
    {
        _startBytesReceived = 0;
        return processStatus_t::INVALID;
    }

    if (++_startBytesReceived == 8)
    {
        _startBytesReceived = 0;
        return processStatus_t::COMPLETE;
    }

    return processStatus_t::INCOMPLETE;
}

Updater::processStatus_t Updater::processFwMetadata(uint8_t data)
{
    // metadata consists of 4 bytes for firmware length and 4 bytes for UID

    if (_stageBytesReceived < 4)
    {
        _fwSize |= (static_cast<uint32_t>(data) << (8 * _stageBytesReceived));
    }
    else
    {
        _receivedUID |= (static_cast<uint32_t>(data) << (8 * (_stageBytesReceived - 4)));
    }

    if (++_stageBytesReceived == 8)
    {
        if (_receivedUID != UID)
        {
            return processStatus_t::INVALID;
        }

        // next stage is firmware update
        _hwa.onFirmwareUpdateStart();

        return processStatus_t::COMPLETE;
    }

    return processStatus_t::INCOMPLETE;
}

Updater::processStatus_t Updater::processFwChunk(uint8_t data)
{
    _receivedWord |= static_cast<uint32_t>(data) << (8 * _stageBytesReceived);

    if (++_stageBytesReceived != sizeof(_receivedWord))
    {
        return processStatus_t::INCOMPLETE;
    }

    if (!_fwPageBytesReceived)
    {
        _hwa.erasePage(_currentFwPage);
    }

    _hwa.fillPage(_currentFwPage, _fwPageBytesReceived, _receivedWord);

    _fwPageBytesReceived += sizeof(_receivedWord);
    _fwBytesReceived += sizeof(_receivedWord);

    _receivedWord       = 0;
    _stageBytesReceived = 0;

    bool pageWritten = false;

    if (_fwPageBytesReceived == _hwa.pageSize(_currentFwPage))
    {
        _fwPageBytesReceived = 0;
        _hwa.commitPage(_currentFwPage);
        pageWritten = true;
    }

    if (_fwBytesReceived == _fwSize)
    {
        // make sure page is written even if entire page range wasn't received
        if (!pageWritten)
        {
            _hwa.commitPage(_currentFwPage);
        }

        _stageBytesReceived = 0;

        return processStatus_t::COMPLETE;
    }

    if (pageWritten)
    {
        _currentFwPage++;
    }

    return processStatus_t::INCOMPLETE;
}

Updater::processStatus_t Updater::processEnd(uint8_t data)
{
    // 4 received bytes must match the _endCommand (lower first, then upper)

    if (((END_COMMAND >> (_stageBytesReceived * 8)) & static_cast<uint32_t>(0xFF)) != data)
    {
        _stageBytesReceived = 0;
        return processStatus_t::INVALID;
    }

    if (++_stageBytesReceived == 4)
    {
        _stageBytesReceived = 0;
        return processStatus_t::COMPLETE;
    }

    return processStatus_t::INCOMPLETE;
}

void Updater::reset()
{
    _currentStage        = 0;
    _currentFwPage       = 0;
    _receivedWord        = 0;
    _fwPageBytesReceived = 0;
    _stageBytesReceived  = 0;
    _fwBytesReceived     = 0;
    _fwSize              = 0;
    _startBytesReceived  = 0;
}