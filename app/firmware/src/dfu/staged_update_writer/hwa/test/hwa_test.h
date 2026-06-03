/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "common/src/dfu/flash_area/hwa/test/hwa_test.h"

namespace opendeck::firmware::dfu::staged_update_writer
{
    /**
     * @brief In-memory staged DFU storage used by writer tests.
     */
    class HwaTest : public opendeck::common::dfu::flash_area::HwaTest
    {
        public:
        bool init_called() const
        {
            return open_called();
        }

        void set_init_result(bool result)
        {
            set_open_result(result);
        }
    };
}    // namespace opendeck::firmware::dfu::staged_update_writer
