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

#include "interface/display/Config.h"
#include "interface/digital/output/leds/Constants.h"
#include "Layout.cpp"


///
/// \brief Helper macro for easier entry and exit from system block.
/// Important: ::init must called before trying to use this macro.
///
#define SYSTEM_BLOCK_ENTER(code)                \
{                                               \
    setStartAddress(0);                         \
    DBMS::setLayout(dbLayout, DB_BLOCKS+1);     \
    code                                        \
    DBMS::setLayout(&dbLayout[1], DB_BLOCKS);   \
    setStartAddress(systemBlockUsage + ((totalMemoryUsage - systemBlockUsage) * activePreset)); \
}

///
/// \brief Initializes database.
///
bool Database::init()
{
    setStartAddress(0);

    if (!DBMS::setLayout(dbLayout, DB_BLOCKS+1))
        return false;

    totalMemoryUsage = DBMS::getDBsize();
    systemBlockUsage = dbLayout[1].address;

    if (!isSignatureValid())
    {
        factoryReset(initFull);
    }
    else
    {
        if (getPresetPreserveState())
        {
            SYSTEM_BLOCK_ENTER
            (
                activePreset = read(0, dbSection_system_settings, systemGlobal_ActivePreset);
            )
        }
        else
        {
            activePreset = 0;
        }

        setPreset(activePreset);
    }

    return true;
}

///
/// \brief Performs factory reset of data in database.
/// @param [in] type Factory reset type. See initType_t enumeration.
///
void Database::factoryReset(initType_t type)
{
    SYSTEM_BLOCK_ENTER
    (
        if (type == initFull)
            clear();

        setDbUID(getDbUID());
    )

    for (int i=0; i<getSupportedPresets(); i++)
    {
        setPreset(i);
        initData(type);
        writeCustomValues();
    }

    setPreset(0);
}

///
/// \brief Used to set new database layout (preset).
/// @param [in] preset  New preset to set.
/// \returns False if specified preset isn't supported, true otherwise.
///
bool Database::setPreset(uint8_t preset)
{
    if (preset >= getSupportedPresets())
        return false;

    activePreset = preset;

    SYSTEM_BLOCK_ENTER
    (
        update(0, dbSection_system_settings, systemGlobal_ActivePreset, preset);
    )

    return true;
}

///
/// \brief Retrieves currently active preset.
///
uint8_t Database::getPreset()
{
    return activePreset;
}

///
/// \brief Writes custom values to specific indexes which can't be generalized within database section.
///
void Database::writeCustomValues()
{
    update(DB_BLOCK_DISPLAY, dbSection_display_features, displayFeatureMIDIeventTime, MIN_MESSAGE_RETENTION_TIME);
    update(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwController, DISPLAY_CONTROLLERS);
    update(DB_BLOCK_DISPLAY, dbSection_display_hw, displayHwResolution, DISPLAY_RESOLUTIONS);
}

///
/// \brief Retrieves number of presets possible to store in database.
/// Preset is simply another database layout copy.
///
uint8_t Database::getSupportedPresets()
{
    return (LESSDB_SIZE - systemBlockUsage) / (totalMemoryUsage - systemBlockUsage);
}

///
/// \brief Retrieves total memory usage of entire database.
/// \returns Memory usage in bytes.
///
uint16_t Database::getDBsize()
{
    uint16_t usage = getSupportedPresets() * (totalMemoryUsage - systemBlockUsage);
    return usage;
}

///
/// \brief Enables or disables preservation of preset setting.
/// If preservation is enabled, configured preset will be loaded on board power on.
/// Otherwise, first preset will be loaded instead.
///
void Database::setPresetPreserveState(bool state)
{
    SYSTEM_BLOCK_ENTER
    (
        update(0, dbSection_system_settings, systemGlobal_presetPreserve, state);
    )
}

///
/// \brief Checks if preset preservation setting is enabled or disabled.
/// \returns True if preset preservation is enabled, false otherwise.
///
bool Database::getPresetPreserveState()
{
    bool returnValue;

    SYSTEM_BLOCK_ENTER
    (
        returnValue = read(0, dbSection_system_settings, systemGlobal_presetPreserve);
    )

    return returnValue;
}

///
/// \brief Checks if database has been already initialized by checking DB_BLOCK_ID.
/// \returns True if valid, false otherwise.
///
bool Database::isSignatureValid()
{
    uint16_t signature;

    SYSTEM_BLOCK_ENTER
    (
        signature = read(0, dbSection_system_uid, 0);
    )

    return getDbUID() == signature;
}

///
/// \brief Calculates unique database ID.
/// UID is calculated by appending number of parameters and their types for all
/// sections and all blocks.
///
uint16_t Database::getDbUID()
{
    ///
    /// \brief Magic value with which calculated signature is XORed.
    ///
    #define DB_UID_BASE 0x1701

    uint16_t signature = 0;

    SYSTEM_BLOCK_ENTER
    (
        //get unique database signature based on its blocks/sections
        for (int i=0; i<DB_BLOCKS+1; i++)
        {
            for (int j=0; j<dbLayout[i].numberOfSections; j++)
            {
                signature += dbLayout[i].section[i].numberOfParameters;
                signature += dbLayout[i].section[i].parameterType;
            }
        }
    )

    return signature ^ DB_UID_BASE;
}

///
/// \brief Updates unique database UID.
/// UID is written to first two database locations.
/// @param [in] uid Database UID to set.
///
void Database::setDbUID(uint16_t uid)
{
    SYSTEM_BLOCK_ENTER
    (
        update(0, dbSection_system_uid, 0, uid);
    )
}