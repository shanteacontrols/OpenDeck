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

#ifdef BOARD_USE_UPDATE_HOOKS

#include "internal.h"

#include <vector>

namespace
{
    std::vector<board::detail::updateHook_t> updateHooks;
}    // namespace

namespace board
{
    void update()
    {
        core::mcu::idle();

        for (size_t i = 0; i < updateHooks.size(); i++)
        {
            updateHooks[i]();
        }
    }

    namespace detail
    {
        void registerUpdateHook(updateHook_t hook)
        {
            updateHooks.push_back(hook);
        }
    }    // namespace detail
}    // namespace board

#endif
