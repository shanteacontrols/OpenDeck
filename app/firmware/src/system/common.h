/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace opendeck::sys
{
    /**
     * @brief Delay before the system reboots into the selected firmware target.
     */
    constexpr inline uint32_t REBOOT_DELAY_MS = 1000;

    /**
     * @brief Delay before top-level I/O processing resumes after system initialization completes.
     *
     * This gives transports and dependent subsystems time to settle before live I/O scanning starts.
     */
    constexpr inline uint32_t INITIAL_IO_RESUME_DELAY_MS = 1000;

    /**
     * @brief Delay before a requested factory reset is executed.
     *
     * This lets the SysEx response leave the active MIDI transport before
     * protocol and I/O teardown begins.
     */
    constexpr inline uint32_t FACTORY_RESET_DELAY_MS = 500;

    /**
     * @brief Delay before a forced refresh starts after USB MIDI becomes ready.
     *
     * This allows the host side to settle before the firmware republishes current component state.
     */
    constexpr inline uint32_t USB_CHANGE_FORCED_REFRESH_DELAY = 3000;

    /**
     * @brief Delay before a forced refresh starts after network identity becomes available.
     *
     * This allows discovery and network socket setup to settle before the firmware republishes
     * current component state over network protocols.
     */
    constexpr inline uint32_t NETWORK_CHANGE_FORCED_REFRESH_DELAY = 500;

    /**
     * @brief Delay before a forced refresh starts after the active preset changes.
     */
    constexpr inline uint32_t PRESET_CHANGE_FORCED_REFRESH_DELAY = 500;

    /**
     * @brief Inactivity timeout for an open SysEx configuration session.
     */
    constexpr inline uint32_t SYSEX_CONFIGURATION_TIMEOUT_MS = 4000;

    /**
     * @brief Maximum number of hardware serial-number bytes exposed by system APIs.
     */
    constexpr inline size_t SERIAL_NUMBER_BUFFER_SIZE = 16;

    /**
     * @brief Identifies why the current staged forced-refresh session was scheduled.
     */
    enum class ForcedRefreshType : uint8_t
    {
        UsbInit,
        NetworkInit,
        OscRequest,
        PresetChange
    };

    /**
     * @brief Maximum number of component indexes processed during one staged forced-refresh step.
     */
    constexpr inline size_t FORCED_UPDATE_MAX_COMPONENTS_PER_RUN = 16;

    /**
     * @brief Delay between consecutive staged forced-refresh steps.
     *
     * This spaces out refresh traffic bursts while still completing the full refresh quickly enough
     * for UI state synchronization.
     */
    constexpr inline uint32_t FORCED_REFRESH_STAGE_DELAY_MS = 5;
}    // namespace opendeck::sys
