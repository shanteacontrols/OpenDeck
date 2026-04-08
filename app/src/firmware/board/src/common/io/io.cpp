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

#include "board/board.h"
#include "internal.h"

#include "core/util/util.h"

namespace board::detail::io
{
    void init()
    {
        bootloader::init();
        indicators::init();

        // initialize the rest of IO only for app

#ifdef OPENDECK_FW_APP
        analog::init();
        digital_in::init();
        digital_out::init();
        unused::init();
#endif
    }
}    // namespace board::detail::io