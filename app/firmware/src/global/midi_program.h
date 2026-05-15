/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/util/incdec/inc_dec.h"
#include "firmware/src/signaling/signaling.h"

namespace opendeck::global
{
    /** @brief Number of MIDI channels tracked by the global program store. */
    constexpr inline uint8_t MIDI_PROGRAM_CHANNEL_COUNT = 16;
    /** @brief Highest valid MIDI program number. */
    constexpr inline uint8_t MIDI_PROGRAM_MAX_VALUE = 127;

    /**
     * @brief Singleton that tracks per-channel MIDI program numbers and a global offset.
     */
    class MidiProgram
    {
        public:
        /**
         * @brief Returns the process-wide MIDI program singleton.
         *
         * @return MIDI program singleton instance.
         */
        static MidiProgram& instance()
        {
            static MidiProgram program;
            return program;
        }

        /**
         * @brief Increments the stored program for one channel.
         *
         * @param channel MIDI channel in the internal 1-16 range.
         * @param steps Number of increment steps to apply before offset handling.
         *
         * @return `true` if the stored program changed, otherwise `false`.
         */
        bool increment_program(uint8_t channel, uint8_t steps)
        {
            if (channel > MIDI_PROGRAM_CHANNEL_COUNT)
            {
                return false;
            }

            auto new_program = ProgramIncDec::increment(_program[channel],
                                                        steps + _offset,
                                                        ProgramIncDec::Type::Edge);

            if (new_program != _program[channel])
            {
                _program[channel] = new_program;
                return true;
            }

            return false;
        }

        /**
         * @brief Decrements the stored program for one channel.
         *
         * @param channel MIDI channel in the internal 1-16 range.
         * @param steps Number of decrement steps to apply before offset handling.
         *
         * @return `true` if the stored program changed, otherwise `false`.
         */
        bool decrement_program(uint8_t channel, uint8_t steps)
        {
            if (channel > MIDI_PROGRAM_CHANNEL_COUNT)
            {
                return false;
            }

            auto new_program = ProgramIncDec::decrement(_program[channel],
                                                        steps + _offset,
                                                        ProgramIncDec::Type::Edge);

            if (new_program != _program[channel])
            {
                _program[channel] = new_program;
                return true;
            }

            return false;
        }

        /**
         * @brief Returns the stored program for one channel.
         *
         * @param channel MIDI channel in the internal 1-16 range.
         *
         * @return Stored program value, or `0` when the channel is invalid.
         */
        uint8_t program(uint8_t channel)
        {
            if (channel > MIDI_PROGRAM_CHANNEL_COUNT)
            {
                return 0;
            }

            return _program[channel];
        }

        /**
         * @brief Stores a program value for one channel after applying the current offset.
         *
         * @param channel MIDI channel in the internal 1-16 range.
         * @param program Base program value before offset is applied.
         *
         * @return `true` if the program was stored, otherwise `false`.
         */
        bool set_program(uint8_t channel, uint8_t program)
        {
            if (channel > MIDI_PROGRAM_CHANNEL_COUNT)
            {
                return false;
            }

            program += _offset;

            if (program > MIDI_PROGRAM_MAX_VALUE)
            {
                return false;
            }

            _program[channel] = program;
            return true;
        }

        /**
         * @brief Increments the global program offset.
         *
         * @param steps Number of increment steps to apply.
         *
         * @return `true` if the offset changed, otherwise `false`.
         */
        bool increment_offset(uint8_t steps)
        {
            auto new_offset = OffsetIncDec::increment(_offset,
                                                      steps,
                                                      OffsetIncDec::Type::Edge);

            if (new_offset != _offset)
            {
                set_offset(new_offset);
                return true;
            }

            return false;
        }

        /**
         * @brief Decrements the global program offset.
         *
         * @param steps Number of decrement steps to apply.
         *
         * @return `true` if the offset changed, otherwise `false`.
         */
        bool decrement_offset(uint8_t steps)
        {
            auto new_offset = OffsetIncDec::decrement(_offset,
                                                      steps,
                                                      OffsetIncDec::Type::Edge);

            if (new_offset != _offset)
            {
                set_offset(new_offset);
                return true;
            }

            return false;
        }

        /**
         * @brief Stores a new global program offset and publishes the corresponding system signal.
         *
         * @param offset New program offset to store.
         */
        void set_offset(uint8_t offset)
        {
            _offset = offset;

            signaling::SystemSignal signal = {};
            signal.system_event            = signaling::SystemEvent::MidiProgramOffsetChange;
            signal.value                   = offset;

            signaling::publish(signal);
        }

        /**
         * @brief Returns the current global program offset.
         *
         * @return Stored program offset.
         */
        uint8_t offset()
        {
            return _offset;
        }

        private:
        MidiProgram() = default;

        using ProgramIncDec                                          = util::IncDec<uint8_t, 0, MIDI_PROGRAM_MAX_VALUE>;
        using OffsetIncDec                                           = util::IncDec<uint8_t, 0, MIDI_PROGRAM_MAX_VALUE>;
        std::array<uint8_t, MIDI_PROGRAM_CHANNEL_COUNT + 1> _program = {};
        uint8_t                                             _offset  = 0;
    };
}    // namespace opendeck::global

#define MidiProgram global::MidiProgram::instance()
