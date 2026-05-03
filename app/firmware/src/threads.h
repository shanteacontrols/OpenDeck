/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "zlibs/utils/threads/threads.h"

namespace opendeck::threads
{
    /**
     * @brief Stack size, in bytes, assigned to OpenDeck worker threads.
     */
    constexpr inline size_t THREAD_STACK_SIZE = 2048;

    /**
     * @brief Thread type used for digital I/O processing.
     */
    using DigitalThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_digital" },
                                                            K_PRIO_PREEMPT(1),
                                                            THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for analog input processing.
     */
    using AnalogThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_analog" },
                                                           K_PRIO_PREEMPT(1),
                                                           THREAD_STACK_SIZE * 3>;

    /**
     * @brief Thread type used for LED state processing.
     */
    using LedsThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_leds" },
                                                         K_PRIO_PREEMPT(1),
                                                         THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for touchscreen event processing.
     */
    using TouchscreenThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_touchscreen" },
                                                                K_PRIO_PREEMPT(1),
                                                                THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for I2C peripheral processing.
     */
    using I2cThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_i2c" },
                                                        K_PRIO_PREEMPT(1),
                                                        THREAD_STACK_SIZE>;

    /**
     * @brief Thread type used for MIDI protocol processing.
     */
    using MidiThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "protocol_midi" },
                                                         K_PRIO_PREEMPT(0),
                                                         THREAD_STACK_SIZE>;

    /**
     * @brief Workqueue type used for deferred system tasks.
     */
    using SystemWorkqueue = zlibs::utils::threads::WorkqueueThread<zlibs::utils::misc::StringLiteral{ "system_work" },
                                                                   K_PRIO_PREEMPT(1),
                                                                   THREAD_STACK_SIZE>;

}    // namespace opendeck::threads
