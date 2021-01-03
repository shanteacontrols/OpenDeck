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

using namespace Bootloader;

void Updater::feed(uint8_t data)
{
    auto nextStage = [&]() {
        if (currentStage == static_cast<uint8_t>(receiveStage_t::end))
        {
            reset();
            writer.apply();
        }
        else
        {
            stageBytesReceived = 0;
            currentStage++;
        }
    };

    //continually process start command - this makes sure the fw update process can always restart

    if (currentStage)
    {
        if (processStart(data) == processStatus_t::complete)
        {
            reset();
            nextStage();
            return;    //nothing more to do in this case
        }
    }

    processStatus_t result = (this->*processHandler[currentStage])(data);

    if (result == processStatus_t::complete)
    {
        nextStage();
    }
    else if (result == processStatus_t::invalid)
    {
        //in this case, restart the procedure
        reset();

        //also reset again if it fails again - it cannot possibly return complete status since that requires 8 bytes
        if ((this->*processHandler[currentStage])(data) == processStatus_t::invalid)
            reset();
    }
}

Updater::processStatus_t Updater::processStart(uint8_t data)
{
    //8 received bytes must match the startCommand (lower first, then upper)

    if (((startCommand >> (startBytesReceived * 8)) & static_cast<uint64_t>(0xFF)) != data)
    {
        startBytesReceived = 0;
        return processStatus_t::invalid;
    }

    if (++startBytesReceived == 8)
    {
        startBytesReceived = 0;
        return processStatus_t::complete;
    }

    return processStatus_t::incomplete;
}

Updater::processStatus_t Updater::processFwMetadata(uint8_t data)
{
    //metadata consists of 4 bytes for firmware length and 4 bytes for UID

    if (stageBytesReceived < 4)
        fwSize |= (static_cast<uint32_t>(data) << (8 * stageBytesReceived));
    else
        receivedUID |= (static_cast<uint32_t>(data) << (8 * (stageBytesReceived - 4)));

    if (++stageBytesReceived == 8)
    {
        if (receivedUID != uid)
            return processStatus_t::invalid;

        return processStatus_t::complete;
    }

    return processStatus_t::incomplete;
}

Updater::processStatus_t Updater::processFwChunk(uint8_t data)
{
    receivedWord |= (data << (8 * stageBytesReceived));

    if (++stageBytesReceived != 2)
        return processStatus_t::incomplete;

    if (!fwPageBytesReceived)
        writer.erasePage(currentFwPage);

    writer.fillPage(currentFwPage, fwPageBytesReceived, receivedWord);

    //we are operating with words (two bytes)
    fwPageBytesReceived += 2;
    fwBytesReceived += 2;

    receivedWord       = 0;
    stageBytesReceived = 0;

    bool pageWritten = false;

    if (fwPageBytesReceived == writer.pageSize(currentFwPage))
    {
        fwPageBytesReceived = 0;
        writer.writePage(currentFwPage);
        pageWritten = true;
    }

    if (fwBytesReceived == fwSize)
    {
        //make sure page is written even if entire page range wasn't received
        if (!pageWritten)
            writer.writePage(currentFwPage);

        stageBytesReceived = 0;

        return processStatus_t::complete;
    }

    if (pageWritten)
        currentFwPage++;

    return processStatus_t::incomplete;
}

Updater::processStatus_t Updater::processEnd(uint8_t data)
{
    //4 received bytes must match the endCommand (lower first, then upper)

    if (((endCommand >> (stageBytesReceived * 8)) & static_cast<uint32_t>(0xFF)) != data)
    {
        stageBytesReceived = 0;
        return processStatus_t::invalid;
    }

    if (++stageBytesReceived == 4)
    {
        stageBytesReceived = 0;
        return processStatus_t::complete;
    }

    return processStatus_t::incomplete;
}

void Updater::reset()
{
    currentStage           = 0;
    currentFwPage          = 0;
    receivedWord           = 0;
    fwPageBytesReceived    = 0;
    stageBytesReceived     = 0;
    commandRepeatsReceived = 0;
    fwBytesReceived        = 0;
    fwSize                 = 0;
    startBytesReceived     = 0;
}