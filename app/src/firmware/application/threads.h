#pragma once

#include "zlibs/utils/threads/threads.h"

namespace threads
{
    using DigitalThread = zlibs::utils::threads::UserThread<zlibs::utils::misc::StringLiteral{ "io_digital" },
                                                            K_PRIO_PREEMPT(0),
                                                            2048>;
}    // namespace threads
