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

#ifdef PROJECT_TARGET_SUPPORT_USB

#include "internal.h"

#include "core/mcu.h"

#include <string.h>
#include <stdio.h>

namespace
{
    core::mcu::usb::descriptorUIDString_t signatureDescriptorInternal;
}

namespace board::detail::usb
{
    const void* serialIDString(uint16_t* size, uint8_t* uid)
    {
        signatureDescriptorInternal.header.type = core::mcu::usb::DESC_TYPE_STRING;
        signatureDescriptorInternal.header.size = core::mcu::usb::STRING_LEN(CORE_MCU_UID_BITS / 4);

        uint8_t uidIndex = 0;

        for (size_t i = 0; i < CORE_MCU_UID_BITS / 4; i++)
        {
            uint8_t uidByte = uid[uidIndex];

            if (i & 0x01)
            {
                uidIndex++;
            }
            else
            {
                uidByte >>= 4;
            }

            uidByte &= 0x0F;

            signatureDescriptorInternal.unicodeString[i] = (uidByte >= 10) ? (('A' - 10) + uidByte) : ('0' + uidByte);
        }

        *size = sizeof(core::mcu::usb::descriptorUIDString_t);
        return &signatureDescriptorInternal;
    }
}    // namespace board::detail::usb

#endif