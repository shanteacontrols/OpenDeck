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

#include "core/util/util.h"

namespace io::encoders
{
    class FilterHw : public Filter
    {
        public:
        FilterHw() = default;

        bool isFiltered(size_t      index,
                        position_t  position,
                        position_t& filteredPosition,
                        uint32_t    sampleTakenTime) override
        {
            filteredPosition = position;

            // disable debouncing mode if encoder isn't moving for more than
            // ENCODERS_DEBOUNCE_RESET_TIME milliseconds
            if ((sampleTakenTime - _lastMovementTime[index]) > ENCODERS_DEBOUNCE_RESET_TIME)
            {
                reset(index);
            }

            if (position != position_t::STOPPED)
            {
                if (_debounceCounter[index] != ENCODERS_DEBOUNCE_COUNT)
                {
                    if (position != _lastDirection[index])
                    {
                        _debounceCounter[index] = 0;
                    }

                    _debounceCounter[index]++;

                    if (_debounceCounter[index] == ENCODERS_DEBOUNCE_COUNT)
                    {
                        _debounceCounter[index]   = 0;
                        _debounceDirection[index] = position;
                    }
                }

                _lastDirection[index]    = position;
                _lastMovementTime[index] = sampleTakenTime;

                if (_debounceDirection[index] != position_t::STOPPED)
                {
                    filteredPosition = _debounceDirection[index];
                }

                return true;
            }

            return false;
        }

        void reset(size_t index) override
        {
            _debounceCounter[index]   = 0;
            _debounceDirection[index] = position_t::STOPPED;
        }

        uint32_t lastMovementTime(size_t index) override
        {
            return _lastMovementTime[index];
        }

        private:
        /// Time in milliseconds after which debounce mode is reset if encoder isn't moving.
        static constexpr uint32_t ENCODERS_DEBOUNCE_RESET_TIME = 50;

        /// Number of times movement in the same direction must be registered in order
        /// for debouncer to become active. Once the debouncer is active, all further changes
        /// in the movement will be ignored, that is, all movements will be registered in the
        /// direction which was repeated. This state is reset until the encoder is either stopped
        /// or if same amount of movements are registered in the opposite direction.
        static constexpr uint8_t ENCODERS_DEBOUNCE_COUNT = 4;

        /// Array holding previous encoder direction for all encoders.
        position_t _lastDirection[Collection::SIZE()] = {};

        /// Array holding current debounced direction for all encoders.
        position_t _debounceDirection[Collection::SIZE()] = {};

        /// Used to detect constant rotation in single direction.
        /// Once n consecutive movements in same direction are detected,
        /// all further movements are assumed to have same direction until
        /// encoder stops moving for ENCODERS_DEBOUNCE_RESET_TIME milliseconds *or*
        /// n new consecutive movements are made in the opposite direction.
        /// n = ENCODERS_DEBOUNCE_COUNT (defined in Constants.h)
        uint8_t _debounceCounter[Collection::SIZE()] = {};

        /// Array holding last movement time for all encoders.
        uint32_t _lastMovementTime[Collection::SIZE()] = {};
    };
}    // namespace io::encoders