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
    class AnalogFilterMedian : public IO::Analog::Filter
    {
        public:
        AnalogFilterMedian() {}

        bool isFiltered(size_t index, uint16_t value, uint16_t& filteredValue) override
        {
            auto compare = [](const void* a, const void* b) {
                if (*(uint16_t*)a < *(uint16_t*)b)
                    return -1;
                else if (*(uint16_t*)a > *(uint16_t*)b)
                    return 1;

                return 0;
            };

            analogSampleMedian[index][sampleCounter[index]++] = value;

            //take the median value to avoid using outliers
            if (sampleCounter[index] == 3)
            {
                qsort(analogSampleMedian[index], 3, sizeof(uint16_t), compare);
                sampleCounter[index] = 0;

                //finally, pass the value through exponential moving average filter for increased stability
                filteredValue = emaFilter[index].value(analogSampleMedian[index][1]);
                return true;
            }

            return false;
        }

        void reset(size_t index) override
        {
            sampleCounter[index] = 0;
            emaFilter[index].reset();
        }

        private:
        class EMA
        {
            //exponential moving average filter
            public:
            EMA() {}

            uint16_t value(uint16_t rawData)
            {
                currentValue = (percentage * static_cast<uint32_t>(rawData) + (100 - percentage) * static_cast<uint32_t>(currentValue)) / 100;
                return currentValue;
            }

            void reset()
            {
                currentValue = 0;
            }

            private:
            uint16_t                  currentValue = 0;
            static constexpr uint32_t percentage   = 80;
        };

        EMA      emaFilter[MAX_NUMBER_OF_ANALOG];
        size_t   sampleCounter[MAX_NUMBER_OF_ANALOG]            = {};
        uint16_t analogSampleMedian[MAX_NUMBER_OF_ANALOG][3]    = {};
        uint16_t analogSampleMovingAvg[MAX_NUMBER_OF_ANALOG][3] = {};
    };    // namespace IO

    template<size_t nrOfSamples>
    class AnalogFilterSMA : public IO::Analog::Filter
    {
        public:
        AnalogFilterSMA() {}

        bool isFiltered(size_t index, uint16_t value, uint16_t& filteredValue) override
        {
            //simple moving average filter
            analogSample[index][sampleCounter[index]++] = value;

            if (sampleCounter[index] == _nrOfSamples)
            {
                filteredValue = 0;

                for (size_t i = 0; i < _nrOfSamples; i++)
                    filteredValue += analogSample[index][i];

                filteredValue /= _nrOfSamples;

                for (size_t i = 0; i < _nrOfSamples - 2; i++)
                    analogSample[index][i] = analogSample[index][i + 1];

                analogSample[index][nrOfSamples - 2] = filteredValue;
                sampleCounter[index]                 = nrOfSamples - 1;

                return true;
            }

            return false;
        }

        void reset(size_t index) override
        {
            sampleCounter[index] = 0;
        }

        private:
        const size_t _nrOfSamples                                    = nrOfSamples;
        uint16_t     analogSample[MAX_NUMBER_OF_ANALOG][nrOfSamples] = {};
        size_t       sampleCounter[MAX_NUMBER_OF_ANALOG]             = {};
    };
}    // namespace IO