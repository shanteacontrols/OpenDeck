/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <inttypes.h>
#include <stddef.h>
#include <span>
#include <string_view>

namespace opendeck::sys
{
    /**
     * @brief Number of 14-bit words in the configuration unlock token.
     */
    constexpr inline size_t CONFIG_UNLOCK_TOKEN_WORDS = 4;

    /**
     * @brief Fixed-size SysEx-safe configuration unlock token.
     */
    using ConfigUnlockToken = std::array<uint16_t, CONFIG_UNLOCK_TOKEN_WORDS>;

    /**
     * @brief Maximum value carried by one 14-bit SysEx configuration parameter.
     */
    constexpr inline uint16_t CONFIG_UNLOCK_TOKEN_WORD_MASK = 0x3FFF;

    /**
     * @brief Number of hash bits carried by one configuration unlock token word.
     */
    constexpr inline uint32_t CONFIG_UNLOCK_TOKEN_WORD_BITS = 14U;

    /**
     * @brief FNV-1a offset basis used for deriving the configuration unlock token.
     */
    constexpr inline uint32_t CONFIG_UNLOCK_FNV_OFFSET_BASIS = 2166136261UL;

    /**
     * @brief FNV-1a prime used for deriving the configuration unlock token.
     */
    constexpr inline uint32_t CONFIG_UNLOCK_FNV_PRIME = 16777619UL;

    /**
     * @brief First fixed seed mixed with the serial when deriving unlock token words.
     */
    constexpr inline std::string_view CONFIG_UNLOCK_TOKEN_SEED_A = "opendeck-config-unlock-v1-a";

    /**
     * @brief Second fixed seed mixed with the serial when deriving unlock token words.
     */
    constexpr inline std::string_view CONFIG_UNLOCK_TOKEN_SEED_B = "opendeck-config-unlock-v1-b";

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

    /**
     * @brief Mixes one byte into a configuration unlock FNV-1a hash.
     *
     * @param hash Current hash state.
     * @param value Byte to mix into the hash.
     *
     * @return Updated hash state.
     */
    constexpr inline uint32_t config_unlock_hash_update(uint32_t hash, uint8_t value)
    {
        hash ^= value;
        hash *= CONFIG_UNLOCK_FNV_PRIME;

        return hash;
    }

    /**
     * @brief Builds one 32-bit configuration unlock hash from a fixed seed and serial bytes.
     *
     * @param seed Fixed token seed to hash before serial bytes.
     * @param serial MCU serial bytes.
     *
     * @return Derived 32-bit hash.
     */
    constexpr inline uint32_t config_unlock_hash(std::string_view seed, std::span<const uint8_t> serial)
    {
        auto hash = CONFIG_UNLOCK_FNV_OFFSET_BASIS;

        for (const auto character : seed)
        {
            hash = config_unlock_hash_update(hash, static_cast<uint8_t>(character));
        }

        for (const auto byte : serial)
        {
            hash = config_unlock_hash_update(hash, byte);
        }

        return hash;
    }

    /**
     * @brief Splits one 32-bit unlock hash into two 14-bit token words.
     *
     * @param hash Hash to split.
     *
     * @return Two SysEx-safe 14-bit token words.
     */
    constexpr inline std::array<uint16_t, 2> split_config_unlock_hash(uint32_t hash)
    {
        return {
            static_cast<uint16_t>(hash & CONFIG_UNLOCK_TOKEN_WORD_MASK),
            static_cast<uint16_t>((hash >> CONFIG_UNLOCK_TOKEN_WORD_BITS) & CONFIG_UNLOCK_TOKEN_WORD_MASK),
        };
    }

    /**
     * @brief Derives the expected configuration unlock token from MCU serial bytes.
     *
     * @param serial MCU serial bytes.
     *
     * @return Expected SysEx-safe unlock token.
     */
    constexpr inline ConfigUnlockToken make_config_unlock_token(std::span<const uint8_t> serial)
    {
        const auto hash_a  = config_unlock_hash(CONFIG_UNLOCK_TOKEN_SEED_A, serial);
        const auto hash_b  = config_unlock_hash(CONFIG_UNLOCK_TOKEN_SEED_B, serial);
        const auto words_a = split_config_unlock_hash(hash_a);
        const auto words_b = split_config_unlock_hash(hash_b);

        return ConfigUnlockToken{
            words_a.at(0),
            words_a.at(1),
            words_b.at(0),
            words_b.at(1),
        };
    }
}    // namespace opendeck::sys
