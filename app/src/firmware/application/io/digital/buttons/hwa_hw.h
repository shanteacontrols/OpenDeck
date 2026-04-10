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

#include "deps.h"
#include "application/io/digital/frame_store.h"

namespace io::buttons
{
    class HwaHw : public Hwa
    {
        public:
        explicit HwaHw(io::digital::FrameStore& frameStore)
            : _frameStore(frameStore)
        {}

        bool init() override
        {
            return true;
        }

        std::optional<bool> state(size_t index) override
        {
            return _frameStore.state(index);
        }

        std::optional<uint8_t> encoderState(size_t index) override
        {
            return _frameStore.encoderState(index);
        }

        size_t buttonToEncoderIndex(size_t index) override
        {
            return _frameStore.buttonToEncoderIndex(index);
        }

        private:
        io::digital::FrameStore& _frameStore;
    };
}    // namespace io::buttons
