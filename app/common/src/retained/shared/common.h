/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/retention/retention.h>

#include <cstddef>
#include <cstdint>

#define OPENDECK_RETENTION_NODE DT_NODELABEL(opendeck_retained)

namespace opendeck::retained
{
    static_assert(DT_NODE_EXISTS(OPENDECK_RETENTION_NODE),
                  "Node label 'opendeck_retained' is required for retained storage.");

    /**
     * @brief Typed retained variable stored in the configured retention area.
     *
     * The variable is backed by the Zephyr retention API so the same access
     * pattern works across targets with different retained-memory mechanisms.
     *
     * @tparam T Stored type.
     * @tparam Offset Offset within the retention area.
     */
    template<typename T, size_t Offset = 0>
    class Retained
    {
        public:
        /**
         * @brief Reads the current value from the retention area.
         *
         * If the retention device is not ready, a value-initialized object is
         * returned instead.
         *
         * @return Stored retained value or a default-initialized fallback.
         */
        T data() const
        {
            T value = {};

            if (device_is_ready(device()))
            {
                (void)retention_read(device(), Offset, reinterpret_cast<uint8_t*>(&value), sizeof(value));
            }

            return value;
        }

        /**
         * @brief Stores a new value in the retention area.
         *
         * @param value Value to persist across resets.
         */
        void set(T value)
        {
            if (device_is_ready(device()))
            {
                (void)retention_write(device(), Offset, reinterpret_cast<const uint8_t*>(&value), sizeof(value));
            }
        }

        private:
        static const struct device* device()
        {
            return DEVICE_DT_GET(OPENDECK_RETENTION_NODE);
        }
    };
}    // namespace opendeck::retained
