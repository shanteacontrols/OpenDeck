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

///
/// \brief Time in milliseconds after which debounce mode is reset if encoder isn't moving.
///
#define ENCODERS_DEBOUNCE_RESET_TIME 50

///
/// Number of times movement in the same direction must be registered in order
/// for debouncer to become active. Once the debouncer is active, all further changes
/// in the movement will be ignored, that is, all movements will be registered in the
/// direction which was repeated. This state is reset until the encoder is either stopped
/// or if same amount of movements are registered in the opposite direction.
///
#define ENCODERS_DEBOUNCE_COUNT 4

///
/// \brief Time threshold in milliseconds between two encoder steps used to detect fast movement.
///
#define ENCODERS_SPEED_TIMEOUT 140