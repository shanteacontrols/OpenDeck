/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/dfu/direct_update_writer/instance/impl/deps.h"
#include "common/src/mcu/shared/deps.h"
#include "common/src/system/shared/common.h"

#include "zlibs/utils/misc/kwork_delayable.h"

#include <zephyr/kernel.h>

namespace opendeck::bootloader::dfu::direct_update_writer
{
    /**
     * @brief Hardware-backed direct-update writer backend that applies the completed firmware image.
     */
    class HwaHw : public Hwa
    {
        public:
        /**
         * @brief Constructs the hardware direct-update writer backend.
         *
         * @param mcu Shared MCU services.
         */
        explicit HwaHw(opendeck::common::mcu::Hwa& mcu)
            : _mcu(mcu)
            , _reboot_work([this]()
                           {
                               reboot();
                           })
        {
        }

        /**
         * @brief Finalizes the update and reboots when no flash errors occurred.
         */
        void apply() override
        {
            _reboot_work.reschedule(opendeck::common::system::REBOOT_DELAY_MS);
        }

        private:
        /**
         * @brief Performs the delayed reboot after a successful firmware update.
         */
        void reboot()
        {
            _mcu.reboot(opendeck::common::mcu::BootTarget::Application);
        }

        opendeck::common::mcu::Hwa&        _mcu;
        zlibs::utils::misc::KworkDelayable _reboot_work;
    };
}    // namespace opendeck::bootloader::dfu::direct_update_writer
