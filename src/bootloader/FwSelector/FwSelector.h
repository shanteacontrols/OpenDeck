/*

Copyright 2015-2021 Igor Petrovic

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

#include <inttypes.h>

class FwSelector
{
    public:
    /// List of all possible firmwares which can be loaded and their magic values.
    /// Bootloader will load a firmware based on read value.
    /// Location of this value is board-specific.
    enum class fwType_t : uint32_t
    {
        application = 0xFF,
        bootloader  = 0x47,
        cdc         = 0x74
    };

    class HWA
    {
        public:
        virtual uint8_t magicBootValue()                                 = 0;
        virtual void    setMagicBootValue(uint8_t value)                 = 0;
        virtual void    load(fwType_t fwType)                            = 0;
        virtual void    appAddrBoundary(uint32_t& first, uint32_t& last) = 0;
        virtual bool    isHWtriggerActive()                              = 0;
        virtual uint8_t readFlash(uint32_t address)                      = 0;
    };

    FwSelector(HWA& hwa)
        : _hwa(hwa)
    {}

    void init();

    private:
    /// List of all possible bootloader triggers.
    enum class btldrTrigger_t : uint8_t
    {
        software,
        hardware,
        all,
        none
    };

    /// Verifies if the programmed flash is valid.
    /// \return True if valid, false otherwise.
    bool isAppValid();

    /// Reads the state of the button responsible for hardware bootloader entry.
    /// returns: True if pressed, false otherwise. If bootloader button doesn't exist,
    ///          function will return false.
    bool isHWtriggerActive();

    HWA& _hwa;
};