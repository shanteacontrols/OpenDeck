/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/mcu/shared/common.h"
#include "firmware/src/mcu/shared/deps.h"
#include "common/src/retained/retained.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/reboot.h>

#include <array>

namespace opendeck::mcu
{
    /**
     * @brief Zephyr-backed MCU services.
     */
    class HwaHw : public Hwa
    {
        public:
        HwaHw() = default;

        /**
         * @brief Reads and caches Zephyr's hardware serial number.
         *
         * @return View into the cached serial-number bytes, or an empty span when unavailable.
         */
        std::span<uint8_t> serial_number() override
        {
            const zlibs::utils::misc::LockGuard lock(_serial_mutex);

            if (!_serial_loaded)
            {
                const auto size = hwinfo_get_device_id(_serial_number.data(), _serial_number.size());

                if (size > 0)
                {
                    _serial_size = static_cast<size_t>(size) > _serial_number.size()
                                       ? _serial_number.size()
                                       : static_cast<size_t>(size);
                }

                _serial_loaded = true;
            }

            return std::span<uint8_t>(_serial_number).first(_serial_size);
        }

        /**
         * @brief Selects the next boot target and performs a cold reboot.
         *
         * @param type Firmware target to reboot into.
         */
        void reboot(fw_selector::FwType type) override
        {
            retained::data.boot_mode.set(static_cast<uint32_t>(type));
            sys_reboot(SYS_REBOOT_COLD);
        }

        private:
        std::array<uint8_t, SERIAL_NUMBER_BUFFER_SIZE> _serial_number = {};
        size_t                                         _serial_size   = 0;
        bool                                           _serial_loaded = false;
        zlibs::utils::misc::Mutex                      _serial_mutex;
    };
}    // namespace opendeck::mcu
