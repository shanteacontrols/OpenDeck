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

#include "Updater.h"

using namespace Bootloader;

void Updater::feed(uint8_t data)
{
    //lower byte first, higher byte second

    switch (currentStage)
    {
    case receiveStage_t::start:
    {
        if (processStart(data))
            currentStage = receiveStage_t::fwMetadata;
    }
    break;

    case receiveStage_t::fwMetadata:
    {
        if (processFwMetadata(data))
            currentStage = receiveStage_t::fwChunk;
    }
    break;

    case receiveStage_t::fwChunk:
    {
        if (processFwChunk(data))
            currentStage = receiveStage_t::end;
    }
    break;

    case receiveStage_t::end:
    {
        if (processEnd(data))
        {
            reset();
            writer.apply();
        }
    }
    break;

    default:
        break;
    }
}

bool Updater::processStart(uint8_t data)
{
    //first 4 received bytes must match the startValue

    if (((startValue >> (byteCountReceived * 8)) & static_cast<uint32_t>(0xFF)) != data)
    {
        byteCountReceived = 0;
        return false;
    }

    if (++byteCountReceived == 4)
    {
        byteCountReceived = 0;
        nextPageSize      = writer.pageSize(currentPage);
        return true;
    }

    return false;
}

bool Updater::processFwMetadata(uint8_t data)
{
    //metadata consists of 4 bytes of total fw length

    fwSize |= (static_cast<uint32_t>(data) << (8 * byteCountReceived));

    if (++byteCountReceived == 4)
    {
        byteCountReceived = 0;
        return true;
    }

    return false;
}

bool Updater::processFwChunk(uint8_t data)
{
    receivedWord |= (data << (8 * byteCountReceived));
    byteCountReceived++;

    if (byteCountReceived != 2)
        return false;

    if (!pageBytesReceived)
        writer.erasePage(currentPage);

    writer.fillPage(currentPage, pageBytesReceived, receivedWord);

    //we are operating with words (two bytes)
    pageBytesReceived += 2;
    fwBytesReceived += 2;

    receivedWord      = 0;
    byteCountReceived = 0;

    bool pageWritten = false;

    if (pageBytesReceived == writer.pageSize(currentPage))
    {
        pageBytesReceived = 0;
        writer.writePage(currentPage);
        pageWritten = true;
        currentPage++;
        nextPageSize = writer.pageSize(currentPage);
    }

    if (fwBytesReceived == fwSize)
    {
        //make sure page is written even if entire page range wasn't received
        if (!pageWritten)
            writer.writePage(currentPage);

        byteCountReceived = 0;

        return true;
    }

    return false;
}

bool Updater::processEnd(uint8_t data)
{
    //last 4 received bytes must match the endValue

    if (((endValue >> (byteCountReceived * 8)) & static_cast<uint32_t>(0xFF)) != data)
    {
        byteCountReceived = 0;
        return false;
    }

    if (++byteCountReceived == 4)
        return true;

    return false;
}

void Updater::reset()
{
    currentStage      = receiveStage_t::start;
    nextPageSize      = 0;
    currentPage       = 0;
    receivedWord      = 0;
    pageBytesReceived = 0;
    fwBytesReceived   = 0;
    fwSize            = 0;
    byteCountReceived = 0;
}