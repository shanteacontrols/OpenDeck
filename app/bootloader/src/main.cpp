/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fw_selector/fw_selector.h"
#include "fw_selector/hwa_hw.h"
#include "indicators.h"
#ifdef CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE
#include "staged_update/staged_update.h"
#endif
#include "webusb/transport.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

using namespace opendeck;

namespace
{
    fw_selector::HwaHw      hwa_fw_selector = {};
    fw_selector::FwSelector selector(hwa_fw_selector);
    webusb::Transport       webusb_transport;
}    // namespace

int main()
{
#ifdef CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE
    if (staged_update::apply())
    {
        sys_reboot(SYS_REBOOT_COLD);
    }
#endif

    const auto selection = selector.select();

    if ((selection.firmware == fw_selector::FwType::Application) &&
        (selection.trigger == fw_selector::Trigger::None))
    {
        hwa_fw_selector.load(fw_selector::FwType::Application);

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    indicators::init();

    if (!webusb_transport.init())
    {
        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    switch (selection.trigger)
    {
    case fw_selector::Trigger::Software:
    {
        webusb::status("Software bootloader request");
    }
    break;

    case fw_selector::Trigger::Hardware:
    {
        webusb::status("Hardware bootloader request");
    }
    break;

    case fw_selector::Trigger::InvalidApp:
    {
        webusb::status("Application image invalid");
    }
    break;

    case fw_selector::Trigger::None:
    default:
        break;
    }

    while (true)
    {
        k_sleep(K_SECONDS(1));
    }
}
