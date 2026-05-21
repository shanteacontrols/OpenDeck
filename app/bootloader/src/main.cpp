/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/indicators/instance/impl/indicators.h"
#ifdef CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE
#include "bootloader/src/installer/builder/builder.h"
#include "bootloader/src/staged_update_reader/builder/builder.h"
#endif
#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU
#include "bootloader/src/webusb/instance/impl/transport.h"
#endif

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

using namespace opendeck;

namespace
{
#ifdef CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE
    staged_update_reader::Builder staged_update_reader_builder;
    installer::Builder            staged_update_installer_builder(nullptr);
#endif
#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU
    webusb::Transport webusb_transport;
#endif
}    // namespace

int main()
{
    printk("OpenDeck bootloader starting\n");

#ifdef CONFIG_PROJECT_BOOTLOADER_STAGED_UPDATE
    if (staged_update_reader_builder.instance().consume(staged_update_installer_builder.instance()))
    {
        sys_reboot(SYS_REBOOT_COLD);
    }
#endif

    indicators::init();

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU
    if (!webusb_transport.init())
    {
        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

#endif

    while (true)
    {
        k_sleep(K_SECONDS(1));
    }
}
