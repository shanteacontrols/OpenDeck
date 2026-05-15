/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "bootloader/src/fw_selector/common.h"
#include "bootloader/src/fw_selector/deps.h"

namespace opendeck::fw_selector
{
    /**
     * @brief Chooses which firmware image should be started.
     */
    class FwSelector
    {
        public:
        /**
         * @brief Constructs the selector around one hardware-abstraction instance.
         *
         * @param hwa Hardware abstraction used to inspect triggers and image validity.
         */
        explicit FwSelector(Hwa& hwa)
            : _hwa(hwa)
        {}

        /**
         * @brief Selects the firmware image and trigger reason to use.
         *
         * @return Firmware-selection result.
         */
        Selection select();

        private:
        Hwa& _hwa;

        /**
         * @brief Returns whether the installed application image is valid.
         *
         * @return `true` if the vector table and CRC validation pass, otherwise `false`.
         */
        bool is_app_valid();

        /**
         * @brief Reads and validates the application vector table.
         *
         * @param stack_pointer Application stack pointer.
         * @param reset_vector Application reset vector.
         *
         * @return `true` if the vector table is valid, otherwise `false`.
         */
        bool is_app_vector_valid(uint32_t& stack_pointer, uint32_t& reset_vector);

        /**
         * @brief Finds a matching installed-image validation record.
         *
         * @return `true` if a validation record was found and its CRC matches, otherwise `false`.
         */
        bool find_app_validation_record();

        /**
         * @brief Calculates the installed application payload CRC.
         *
         * @param size Payload size in bytes.
         * @param crc Calculated CRC.
         *
         * @return `true` if the CRC was calculated, otherwise `false`.
         */
        bool calculate_app_crc(uint32_t size, uint32_t& crc);
    };
}    // namespace opendeck::fw_selector
