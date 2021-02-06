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

#include "FwSelector.h"
#include "core/src/general/CRC.h"

void FwSelector::init()
{
    auto fwType = fwType_t::application;

    if (_hwa.isHWtriggerActive())
    {
        fwType = fwType_t::bootloader;
    }
    else
    {
        uint8_t magicValue = _hwa.magicBootValue();

        switch (magicValue)
        {
        case static_cast<uint8_t>(fwType_t::cdc):
            fwType = fwType_t::cdc;
            break;

        case static_cast<uint8_t>(fwType_t::bootloader):
            fwType = fwType_t::bootloader;
            break;

        default:
            //assume application here
            fwType = isAppValid() ? fwType_t::application : fwType_t::bootloader;
            break;
        }
    }

    //always reset soft trigger after reading it back to application
    _hwa.setMagicBootValue(static_cast<uint8_t>(fwType_t::application));
    _hwa.load(fwType);
}

bool FwSelector::isAppValid()
{
#ifdef FW_SELECTOR_NO_VERIFY_CRC
    return true;
#else
    //verify app crc
    //crc is calculated across retrieved firmware address range
    //last retrieved firmware address should contain valid application CRC
    //consider application valid if calculated and retrieved CRC are equal
    uint32_t firstFwAddr;
    uint32_t lastFwAddr;

    _hwa.appAddrBoundary(firstFwAddr, lastFwAddr);

    //sanity checks
    if (!lastFwAddr || (firstFwAddr == lastFwAddr) || (firstFwAddr > lastFwAddr))
        return false;

    volatile uint16_t crcActual = _hwa.readFlash(lastFwAddr + 1);
    crcActual <<= 8;
    crcActual |= _hwa.readFlash(lastFwAddr);

    volatile uint16_t crcCalculated = 0x0000;

    for (uint32_t i = firstFwAddr; i < lastFwAddr; i++)
    {
        uint8_t data  = _hwa.readFlash(i);
        crcCalculated = core::crc::xmodem(crcCalculated, data);
    }

    return (crcCalculated == crcActual);
#endif
}