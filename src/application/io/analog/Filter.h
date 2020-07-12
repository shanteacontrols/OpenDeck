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

#pragma once

#include "io/analog/Analog.h"

namespace IO
{
    template<size_t nrOfSamples>
    class AnalogFilter : public IO::Analog::Filter
    {
        public:
        AnalogFilter() {}

        bool isFiltered(size_t index, uint16_t value, uint16_t& filteredValue) override
        {
            auto compare = [](const void* a, const void* b) {
                if (*(uint16_t*)a < *(uint16_t*)b)
                    return -1;
                else if (*(uint16_t*)a > *(uint16_t*)b)
                    return 1;

                return 0;
            };

            analogSample[index][sampleCounter[index]++] = value;

            if (sampleCounter[index] == _nrOfSamples)
            {
                sampleCounter[index] = 0;
                filteredValue        = 0;

                if (_nrOfSamples > 2)
                {
                    qsort(analogSample[index], _nrOfSamples, sizeof(uint16_t), compare);

                    //now delete half of samples (one quarter lower, one quarter upper)

                    for (size_t i = _nrOfSamples / 4; i < (_nrOfSamples - (_nrOfSamples / 4)); i++)
                        filteredValue += analogSample[index][i];

                    filteredValue /= (_nrOfSamples / 2);
                }
                else
                {
                    for (size_t i = 0; i < _nrOfSamples; i++)
                        filteredValue += analogSample[index][i];

                    filteredValue /= _nrOfSamples;
                }

                //finally, pass the value through exponential moving average filter for increased stability
                filteredValue = emaFilter[index].value(filteredValue);

                return true;
            }

            return false;
        }

        void reset(size_t index) override
        {
            sampleCounter[index] = 0;
        }

        private:
        class EMA
        {
            //exponential moving average filter
            public:
            EMA() {}

            // use factor 0.5 for easier bitwise math
            uint16_t value(uint16_t rawData)
            {
                currentValue = (rawData >> 1) + (currentValue >> 1);
                return currentValue;
            }

            void reset()
            {
                currentValue = 0;
            }

            private:
            uint16_t currentValue = 0;
        };

        const size_t _nrOfSamples = nrOfSamples;
        EMA          emaFilter[MAX_NUMBER_OF_ANALOG];
        uint16_t     analogSample[MAX_NUMBER_OF_ANALOG][nrOfSamples] = {};
        size_t       sampleCounter[MAX_NUMBER_OF_ANALOG]             = {};
    };
}    // namespace IO