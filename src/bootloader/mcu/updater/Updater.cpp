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

void Updater::feed(uint16_t data)
{
    if (pageBytesReceived < addressBytes)
    {
        if (!pageBytesReceived)
            currentPage = data;
        else
            currentPage |= ((uint32_t)data << 16);

        pageBytesReceived += 2;

        if (pageBytesReceived == addressBytes)
        {
            pageWord = 0;

            if (currentPage == updateCompletePage)
            {
                pageBytesReceived = 0;
                currentPage       = 0;
                writer.apply();
                return;
            }

            writer.erasePage(currentPage);
        }
    }
    else
    {
        writer.fillPage(currentPage + (pageWord << 1), data);

        if (++pageWord == (pageSize / 2))
        {
            pageBytesReceived = 0;
            pageWord          = 0;
            writer.writePage(currentPage);
        }
    }
}

void Updater::reset()
{
    pageBytesReceived = 0;
    currentPage       = 0;
    pageWord          = 0;
}