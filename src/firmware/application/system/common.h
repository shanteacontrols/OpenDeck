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

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace sys
{
    // Time in milliseconds after which preset change will be reported to message dispatcher.
    // It's possible that the presets could be changed rapidly in which case the system will be
    // flooded with many forced component refresh messages, normally occuring after preset change.
    // Once the preset change is notified, MIDI data for all components will be forcefully resent.
    constexpr inline uint32_t PRESET_CHANGE_NOTIFY_DELAY = 500;

    // Time in milliseconds after which all internal MIDI values will be forcefully resent
    // once USB connection is detected. This delay makes sure that the data is actually sent
    // to USB software using the firmware on USB plugging it. Otherwise, the data might get
    // lost due to the data being sent out too fast (immediately on plugging it), giving the
    // MIDI software no time to react.
    constexpr inline uint32_t USB_CHANGE_FORCED_REFRESH_DELAY = 1000;

    // Maximum amount of component indexes which will be checked per single run() call. All indexes aren't
    // processed in order to reduce the amount of time spent in a single run() call.
    constexpr inline size_t MAX_UPDATES_PER_RUN = 16;
}    // namespace sys