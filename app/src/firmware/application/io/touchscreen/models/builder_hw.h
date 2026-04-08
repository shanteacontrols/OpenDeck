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

#include "application/io/touchscreen/deps.h"

#include "nextion/nextion.h"
#include "viewtech/viewtech.h"

namespace io::touchscreen
{
    class ModelsBuilder
    {
        public:
        ModelsBuilder(Hwa& hwa)
            : _nextion(hwa)
            , _viewtech(hwa)
        {}

        private:
        Nextion  _nextion;
        Viewtech _viewtech;
    };
}    // namespace io::touchscreen