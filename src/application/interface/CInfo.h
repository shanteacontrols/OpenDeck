/*

Copyright 2015-2018 Igor Petrovic

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

#include "database/blocks/Blocks.h"
#include "sysex/src/DataTypes.h"

typedef bool (*cinfoHandler_t)(dbBlockID_t dbBlock, sysExParameter_t componentID);

///
/// \brief Common handler used to identify currently active component during SysEx configuration.
/// Must be implemented externally.
///
extern cinfoHandler_t cinfoHandler;