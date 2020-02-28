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

#include "dbms/src/LESSDB.h"
#include "blocks/Blocks.h"

///
/// \addtogroup eeprom
/// @{
///

class Database : public LESSDB
{
    public:
    Database(bool (&readCallback)(uint32_t address, sectionParameterType_t type, int32_t& value), bool (&writeCallback)(uint32_t address, int32_t value, sectionParameterType_t type), size_t maxSize)
        : LESSDB(readCallback, writeCallback, maxSize)
    {}

    bool    init();
    bool    factoryReset(LESSDB::factoryResetType_t type);
    uint8_t getSupportedPresets();
    bool    setPreset(uint8_t preset);
    uint8_t getPreset();
    bool    isSignatureValid();
    void    setPresetPreserveState(bool state);
    bool    getPresetPreserveState();
    void    setPresetChangeHandler(void (*presetChangeHandler)(uint8_t preset));

    private:
    void     writeCustomValues();
    uint16_t getDbUID();
    void     setDbUID(uint16_t uid);

    ///
    /// \brief User-specified callback called when preset is changed.
    ///
    void (*presetChangeHandler)(uint8_t preset) = nullptr;

    ///
    /// \brief Holds total memory usage for the entire database layout (system block included).
    ///
    uint32_t totalMemoryUsage;

    ///
    /// \brief Total size of system block.
    /// Used to set correct offset in database for user layout.
    ///
    uint16_t systemBlockUsage;

    ///
    /// \brief Holds currently active preset.
    ///
    uint8_t activePreset = 0;
};