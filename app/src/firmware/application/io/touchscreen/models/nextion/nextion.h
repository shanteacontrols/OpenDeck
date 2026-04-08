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

#include "core/util/ring_buffer.h"

namespace io::touchscreen
{
    class Nextion : public Model
    {
        public:
        Nextion(Hwa& hwa);

        bool      init() override;
        bool      deInit() override;
        bool      setScreen(size_t index) override;
        tsEvent_t update(Data& data) override;
        void      setIconState(Icon& icon, bool state) override;
        bool      setBrightness(brightness_t brightness) override;

        private:
        enum class responseId_t : uint8_t
        {
            BUTTON,
            AMOUNT
        };

        struct ResponseDescriptor
        {
            uint8_t size       = 0;
            uint8_t responseId = 0;
        };

        static constexpr ResponseDescriptor RESPONSES[static_cast<size_t>(responseId_t::AMOUNT)] = {
            // button
            {
                .size       = 6,
                .responseId = 0x65,
            },
        };

        // there are 7 levels of brighness - scale them to available range (0-100)
        static constexpr uint8_t BRIGHTNESS_MAPPING[7] = {
            10,
            25,
            50,
            75,
            80,
            90,
            100
        };

        Hwa&   _hwa;
        char   _commandBuffer[Model::BUFFER_SIZE];
        size_t _endCounter = 0;

        bool      writeCommand(const char* line, ...);
        bool      endCommand();
        tsEvent_t response(Data& data);
    };
}    // namespace io::touchscreen
