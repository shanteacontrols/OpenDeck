/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fw_selector.h"

fw_selector::Selection fw_selector::FwSelector::select()
{
    Selection selection = {};

    if (_hwa.is_hw_trigger_active())
    {
        selection.firmware = FwType::Bootloader;
        selection.trigger  = Trigger::Hardware;

        return selection;
    }

    if (_hwa.is_sw_trigger_active())
    {
        selection.firmware = FwType::Bootloader;
        selection.trigger  = Trigger::Software;

        return selection;
    }

    if (_hwa.is_app_valid())
    {
        selection.firmware = FwType::Application;
        selection.trigger  = Trigger::None;

        return selection;
    }

    selection.firmware = FwType::Bootloader;
    selection.trigger  = Trigger::InvalidApp;

    return selection;
}
