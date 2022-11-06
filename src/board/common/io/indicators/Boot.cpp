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

#ifdef HW_SUPPORT_LED_INDICATORS
#ifdef FW_BOOT

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/Util.h"
#include "core/src/Timing.h"
#include <Target.h>

#include "Indicators.h"

namespace
{
    bool              _fwUpdateInProgress;
    volatile uint16_t _fwUpdateIndicatorTimeout;
    bool              _fwUpdateIndicatorState;
}    // namespace

namespace board::detail::io::indicators
{
    void init()
    {
        INIT_ALL_INDICATORS();
    }

    void indicateBootloaderLoad()
    {
        // to indicate bootloader, just leave all the indicators on
        ALL_INDICATORS_ON();
    }

    void update()
    {
        using namespace board::io::indicators;

        if (_fwUpdateInProgress)
        {
            if (++_fwUpdateIndicatorTimeout == LED_DFU_INDICATOR_TIMEOUT)
            {
                _fwUpdateIndicatorTimeout = 0;
                _fwUpdateIndicatorState   = !_fwUpdateIndicatorState;

                if (_fwUpdateIndicatorState)
                {
                    ALL_INDICATORS_ON();
                }
                else
                {
                    ALL_INDICATORS_OFF();
                }
            }
        }
    }
}    // namespace board::detail::io::indicators

namespace board::io::indicators
{
    void indicateFirmwareUpdateStart()
    {
        _fwUpdateInProgress = true;
    }
}    // namespace board::io::indicators

#endif
#endif