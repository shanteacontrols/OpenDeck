/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "util/incdec/inc_dec.h"
#include "signaling/signaling.h"

namespace opendeck::global
{
    /** @brief Number of microseconds in one minute. */
    constexpr inline uint32_t USEC_PER_MINUTE = 60000000;
    /** @brief Lowest supported MIDI BPM value. */
    constexpr inline uint8_t BPM_MIN_VALUE = 10;
    /** @brief Highest supported MIDI BPM value. */
    constexpr inline uint8_t BPM_MAX_VALUE = 255;
    /** @brief Default BPM used at startup. */
    constexpr inline uint8_t BPM_DEFAULT = 120;

    /**
     * @brief Singleton that stores and publishes the current global MIDI clock BPM.
     */
    class Bpm
    {
        public:
        /**
         * @brief Returns the process-wide BPM singleton.
         *
         * @return BPM singleton instance.
         */
        static Bpm& instance()
        {
            static Bpm bpm;
            return bpm;
        }

        /**
         * @brief Increments the current BPM by the requested number of steps.
         *
         * @param steps Number of increment steps to apply.
         *
         * @return `true` if the BPM changed, otherwise `false`.
         */
        bool increment(uint8_t steps)
        {
            auto new_bpm = BpmIncDec::increment(_bpm,
                                                steps,
                                                BpmIncDec::Type::Edge);

            if (new_bpm != _bpm)
            {
                set(new_bpm);
                return true;
            }

            return false;
        }

        /**
         * @brief Decrements the current BPM by the requested number of steps.
         *
         * @param steps Number of decrement steps to apply.
         *
         * @return `true` if the BPM changed, otherwise `false`.
         */
        bool decrement(uint8_t steps)
        {
            auto new_bpm = BpmIncDec::decrement(_bpm,
                                                steps,
                                                BpmIncDec::Type::Edge);

            if (new_bpm != _bpm)
            {
                set(new_bpm);
                return true;
            }

            return false;
        }

        /**
         * @brief Returns the current BPM value.
         *
         * @return Current BPM.
         */
        uint8_t value()
        {
            return _bpm;
        }

        /**
         * @brief Converts a BPM value into the MIDI clock period in microseconds.
         *
         * @param bpm BPM value to convert.
         *
         * @return Microseconds per MIDI clock tick.
         */
        uint32_t bpm_to_usec(uint32_t bpm)
        {
            return USEC_PER_MINUTE / PPQN / bpm;
        }

        private:
        Bpm() = default;

        using BpmIncDec = util::IncDec<uint8_t, BPM_MIN_VALUE, BPM_MAX_VALUE>;

        static constexpr uint32_t PPQN = 24;

        uint8_t _bpm = BPM_DEFAULT;

        /**
         * @brief Stores a new BPM value and publishes the corresponding system signal.
         *
         * @param bpm New BPM value to store.
         */
        void set(uint8_t bpm)
        {
            _bpm = bpm;

            signaling::SystemSignal signal = {};
            signal.system_event            = signaling::SystemEvent::MidiBpmChange;
            signal.value                   = bpm;

            signaling::publish(signal);
        }
    };
}    // namespace opendeck::global

#define Bpm global::Bpm::instance()
