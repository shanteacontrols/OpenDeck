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

#include "DataTypes.h"

///
/// \brief Touchscreen control.
/// \defgroup interfaceLCDTouch Touchscreen
/// \ingroup interfaceLCD
/// @{

class Touchscreen
{
    public:
    Touchscreen() {}
    bool init(ts_t touchscreenType);
    void update();
    void setPage(uint8_t pageID);
    uint8_t getPage();
    void setButtonHandler(void(*fptr)(uint8_t index, bool state));

    friend void sdw_init(Touchscreen &base);
    friend bool sdw_update(Touchscreen &base);

    protected:
    void        (*buttonHandler)(uint8_t index, bool state) = nullptr;
    bool        (*displayUpdatePtr)(Touchscreen &instance) = nullptr;
    void        (*setPagePtr)(uint8_t pageID) = nullptr;

    uint8_t     displayRxBuffer[TOUCHSCREEN_RX_BUFFER_SIZE] = {};
    uint8_t     bufferIndex_rx = 0;
    uint8_t     activeButtonID = 0;
    bool        activeButtonState = false;
    uint8_t     activePage = 0;
};

/// @}
