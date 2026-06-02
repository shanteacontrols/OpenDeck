/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/threads/threads.h"

#include <cstddef>

namespace opendeck::firmware::threads
{
    /** @brief Stack size, in bytes, assigned to digital I/O processing. */
    constexpr inline size_t DIGITAL_THREAD_STACK_SIZE = 2048;

    /** @brief Stack size, in bytes, assigned to analog input processing. */
    constexpr inline size_t ANALOG_THREAD_STACK_SIZE = 6144;

    /** @brief Stack size, in bytes, assigned to output state processing. */
    constexpr inline size_t OUTPUTS_THREAD_STACK_SIZE = 2048;

    /** @brief Stack size, in bytes, assigned to touchscreen event processing. */
    constexpr inline size_t TOUCHSCREEN_THREAD_STACK_SIZE = 2048;

    /** @brief Stack size, in bytes, assigned to I2C peripheral processing. */
    constexpr inline size_t I2C_THREAD_STACK_SIZE = 2048;

    /** @brief Stack size, in bytes, assigned to MIDI protocol processing. */
    constexpr inline size_t MIDI_THREAD_STACK_SIZE = 2048;

    /** @brief Stack size, in bytes, assigned to OSC packet reception. */
    constexpr inline size_t OSC_READ_THREAD_STACK_SIZE = 6144;

    /** @brief Stack size, in bytes, assigned to OSC packet transmission. */
    constexpr inline size_t OSC_SEND_THREAD_STACK_SIZE = 6144;

    /** @brief Stack size, in bytes, assigned to the system workqueue. */
    constexpr inline size_t SYSTEM_WORKQUEUE_STACK_SIZE = 6144;

    /**
     * @brief Thread type used for digital I/O processing.
     */
    using DigitalThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_digital" },
                                                            K_PRIO_PREEMPT(1),
                                                            DIGITAL_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for analog input processing.
     */
    using AnalogThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_analog" },
                                                           K_PRIO_PREEMPT(1),
                                                           ANALOG_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for output state processing.
     */
    using OutputsThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_outputs" },
                                                            K_PRIO_PREEMPT(1),
                                                            OUTPUTS_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for touchscreen event processing.
     */
    using TouchscreenThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_touchscreen" },
                                                                K_PRIO_PREEMPT(1),
                                                                TOUCHSCREEN_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for I2C peripheral processing.
     */
    using I2cThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_i2c" },
                                                        K_PRIO_PREEMPT(1),
                                                        I2C_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for MIDI protocol processing.
     */
    using MidiThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "protocol_midi" },
                                                         K_PRIO_PREEMPT(0),
                                                         MIDI_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for OSC packet reception.
     */
    using OscReadThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "protocol_osc_rx" },
                                                            K_PRIO_PREEMPT(1),
                                                            OSC_READ_THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for OSC packet transmission.
     */
    using OscSendThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "protocol_osc_tx" },
                                                            K_PRIO_PREEMPT(1),
                                                            OSC_SEND_THREAD_STACK_SIZE>;

    /**
     * @brief Workqueue type used for deferred system tasks.
     */
    using SystemWorkqueue = zlibs::utils::threads::WorkqueueThread<zlibs::utils::misc::StringLiteral{ "system_work" },
                                                                   K_PRIO_PREEMPT(1),
                                                                   SYSTEM_WORKQUEUE_STACK_SIZE>;

}    // namespace opendeck::firmware::threads
