/*

Copyright Igor Petrovic

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

#include "fw_selector.h"

#include "core/util/util.h"

using namespace fw_selector;

FwSelector::FwSelector(Hwa& hwa)
    : _hwa(hwa)
{}

void FwSelector::select()
{
    auto fwType = fwType_t::APPLICATION;

    if (_hwa.isHwTriggerActive())
    {
        fwType = fwType_t::BOOTLOADER;
    }
    else
    {
        auto magicValue = _hwa.magicBootValue();

        switch (magicValue)
        {
        case static_cast<uint32_t>(fwType_t::BOOTLOADER):
        {
            fwType = fwType_t::BOOTLOADER;
        }
        break;

        default:
        {
            // assume application here
            fwType = isAppValid() ? fwType_t::APPLICATION : fwType_t::BOOTLOADER;
        }
        break;
        }
    }

    // always reset soft trigger after reading it back to application
    _hwa.setMagicBootValue(static_cast<uint8_t>(fwType_t::APPLICATION));
    _hwa.load(fwType);
}

bool FwSelector::isAppValid()
{
#ifdef PROJECT_TARGET_BOOTLOADER_NO_VERIFY_CRC
    return true;
#else
    // verify app crc
    // crc is calculated across retrieved firmware address range
    // last retrieved firmware address should contain valid application CRC
    // consider application valid if calculated and retrieved CRC are equal
    uint32_t firstFwAddr;
    uint32_t lastFwAddr;

    _hwa.appAddrBoundary(firstFwAddr, lastFwAddr);

    // sanity checks
    if (!lastFwAddr || (firstFwAddr == lastFwAddr) || (firstFwAddr > lastFwAddr))
    {
        return false;
    }

    volatile uint16_t crcActual = _hwa.readFlash(lastFwAddr + 1);
    crcActual <<= 8;
    crcActual |= _hwa.readFlash(lastFwAddr);

    volatile uint16_t crcCalculated = 0x0000;

    for (uint32_t i = firstFwAddr; i < lastFwAddr; i++)
    {
        uint8_t data  = _hwa.readFlash(i);
        crcCalculated = core::util::XMODEM(crcCalculated, data);
    }

    return (crcCalculated == crcActual);
#endif
}