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

#include "Updater.h"

void Updater::feed(uint8_t data)
{
    auto nextStage = [&]() {
        if (_currentStage == static_cast<uint8_t>(receiveStage_t::end))
        {
            reset();
            _writer.apply();
        }
        else
        {
            _stageBytesReceived = 0;
            _currentStage++;
        }
    };

    //continually process start command - this makes sure the fw update process can always restart

    if (_currentStage)
    {
        if (processStart(data) == processStatus_t::complete)
        {
            reset();
            nextStage();
            return;    //nothing more to do in this case
        }
    }

    processStatus_t result = (this->*processHandler[_currentStage])(data);

    if (result == processStatus_t::complete)
    {
        nextStage();
    }
    else if (result == processStatus_t::invalid)
    {
        //in this case, restart the procedure
        reset();

        //also reset again if it fails again - it cannot possibly return complete status since that requires 8 bytes
        if ((this->*processHandler[_currentStage])(data) == processStatus_t::invalid)
            reset();
    }
}

Updater::processStatus_t Updater::processStart(uint8_t data)
{
    //8 received bytes must match the startCommand (lower first, then upper)

    if (((_startCommand >> (_startBytesReceived * 8)) & static_cast<uint64_t>(0xFF)) != data)
    {
        _startBytesReceived = 0;
        return processStatus_t::invalid;
    }

    if (++_startBytesReceived == 8)
    {
        _startBytesReceived = 0;
        return processStatus_t::complete;
    }

    return processStatus_t::incomplete;
}

Updater::processStatus_t Updater::processFwMetadata(uint8_t data)
{
    //metadata consists of 4 bytes for firmware length and 4 bytes for UID

    if (_stageBytesReceived < 4)
        _fwSize |= (static_cast<uint32_t>(data) << (8 * _stageBytesReceived));
    else
        _receivedUID |= (static_cast<uint32_t>(data) << (8 * (_stageBytesReceived - 4)));

    if (++_stageBytesReceived == 8)
    {
        if (_receivedUID != _uid)
            return processStatus_t::invalid;

        return processStatus_t::complete;
    }

    return processStatus_t::incomplete;
}

Updater::processStatus_t Updater::processFwChunk(uint8_t data)
{
    _receivedWord |= (data << (8 * _stageBytesReceived));

    if (++_stageBytesReceived != 2)
        return processStatus_t::incomplete;

    if (!_fwPageBytesReceived)
        _writer.erasePage(_currentFwPage);

    _writer.fillPage(_currentFwPage, _fwPageBytesReceived, _receivedWord);

    //we are operating with words (two bytes)
    _fwPageBytesReceived += 2;
    _fwBytesReceived += 2;

    _receivedWord       = 0;
    _stageBytesReceived = 0;

    bool pageWritten = false;

    if (_fwPageBytesReceived == _writer.pageSize(_currentFwPage))
    {
        _fwPageBytesReceived = 0;
        _writer.writePage(_currentFwPage);
        pageWritten = true;
    }

    if (_fwBytesReceived == _fwSize)
    {
        //make sure page is written even if entire page range wasn't received
        if (!pageWritten)
            _writer.writePage(_currentFwPage);

        _stageBytesReceived = 0;

        return processStatus_t::complete;
    }

    if (pageWritten)
        _currentFwPage++;

    return processStatus_t::incomplete;
}

Updater::processStatus_t Updater::processEnd(uint8_t data)
{
    //4 received bytes must match the _endCommand (lower first, then upper)

    if (((_endCommand >> (_stageBytesReceived * 8)) & static_cast<uint32_t>(0xFF)) != data)
    {
        _stageBytesReceived = 0;
        return processStatus_t::invalid;
    }

    if (++_stageBytesReceived == 4)
    {
        _stageBytesReceived = 0;
        return processStatus_t::complete;
    }

    return processStatus_t::incomplete;
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