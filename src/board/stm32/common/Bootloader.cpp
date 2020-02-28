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

#include "board/Board.h"

namespace
{
    ///
    /// \brief Variable used to specify whether to enter bootloader or application.
    ///
    uint32_t fwEntryType __attribute__((section(".noinit"))) __attribute__((used));
}    // namespace

namespace Board
{
    namespace detail
    {
        namespace bootloader
        {
            bool isSWtriggerActive()
            {
                return fwEntryType == BTLDR_REBOOT_VALUE;
            }

            void enableSWtrigger()
            {
                fwEntryType = BTLDR_REBOOT_VALUE;
            }

            void clearSWtrigger()
            {
                fwEntryType = APP_REBOOT_VALUE;
            }
        }    // namespace bootloader
    }        // namespace detail
}    // namespace Board