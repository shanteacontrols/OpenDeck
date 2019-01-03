/*

Copyright 2015-2019 Igor Petrovic

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

typedef enum
{
    ccLimitLow,
    ccLimitHigh
} ccLimitType_t;

typedef enum 
{
    aType_potentiometer_cc,
    aType_potentiometer_note,
    aType_fsr,
    aType_button,
    aType_NRPN_7,
    aType_NRPN_14,
    aType_PitchBend,
    ANALOG_TYPES
} analogType_t;

typedef enum
{
    velocity,
    aftertouch
} pressureType_t;
