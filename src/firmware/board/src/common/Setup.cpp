/*

Copyright 2015-2022 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board/Board.h"
#include "Internal.h"
#include "core/Timing.h"
#include "core/MCU.h"

namespace core::timing::detail
{
    /// Implementation of core variable used to keep track of run time in milliseconds.
    volatile uint32_t ms;
}    // namespace core::timing::detail

namespace
{
    constexpr uint32_t MAIN_TIMER_TIMEOUT_US = 1000;
#ifdef FW_APP
#if defined(BOARD_USE_FAST_SOFT_PWM_TIMER) && defined(PROJECT_TARGET_SUPPORT_SOFT_PWM)
    constexpr uint32_t SOFT_PWM_TIMER_TIMEOUT_US = 200;
#endif
#endif

    bool _usbInitialized;
}    // namespace

namespace board
{
    void init()
    {
#if defined(FW_APP)
        detail::setup::application();

        size_t mainTimerIndex = 0;

        core::mcu::timers::allocate(mainTimerIndex, []()
                                    {
                                        core::timing::detail::ms++;
                                        detail::io::indicators::update();
#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST
                                        detail::io::digitalIn::update();
#ifndef BOARD_USE_FAST_SOFT_PWM_TIMER
#if PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS > 0
                                        detail::io::digitalOut::update();
#endif
#endif
#endif
                                    });

        core::mcu::timers::setPeriod(mainTimerIndex, MAIN_TIMER_TIMEOUT_US);
        core::mcu::timers::start(mainTimerIndex);

#if defined(BOARD_USE_FAST_SOFT_PWM_TIMER) && defined(PROJECT_TARGET_SUPPORT_SOFT_PWM)
        size_t pwmTimerIndex = 0;

        core::mcu::timers::allocate(pwmTimerIndex, []()
                                    {
#ifdef FW_APP
#ifndef PROJECT_TARGET_USB_OVER_SERIAL_HOST
#if PROJECT_TARGET_MAX_NR_OF_DIGITAL_OUTPUTS > 0
                                        detail::io::digitalOut::update();
#endif
#endif
#endif
                                    });

        core::mcu::timers::setPeriod(pwmTimerIndex, SOFT_PWM_TIMER_TIMEOUT_US);
        core::mcu::timers::start(pwmTimerIndex);
#endif
#elif defined(FW_BOOT)
        detail::setup::bootloader();

        size_t mainTimerIndex = 0;

        core::mcu::timers::allocate(mainTimerIndex, []()
                                    {
                                        core::timing::detail::ms++;
                                        detail::io::indicators::update();
                                    });

        // don't start the timers yet - if the app will be run immediately, it's not needed
        core::mcu::timers::setPeriod(mainTimerIndex, MAIN_TIMER_TIMEOUT_US);
#endif
    }

    namespace usb
    {
        initStatus_t init()
        {
            // allow usb init only once
            if (!_usbInitialized)
            {
                detail::usb::init();
                _usbInitialized = true;

                return initStatus_t::OK;
            }

            return initStatus_t::ALREADY_INIT;
        }

        void deInit()
        {
            if (_usbInitialized)
            {
                detail::usb::deInit();
                _usbInitialized = false;
            }
        }

        bool isInitialized()
        {
            return _usbInitialized;
        }
    }    // namespace usb

    namespace detail::setup
    {
#ifdef PROJECT_TARGET_USB_OVER_SERIAL_DEVICE
        void waitUsbLink()
        {
            usbOverSerial::internalCMD_t cmd;

            uint8_t data[1] = {
                static_cast<uint8_t>(usbOverSerial::internalCMD_t::LINK_READY),
            };

            usbOverSerial::USBWritePacket packet(usbOverSerial::packetType_t::INTERNAL,
                                                 data,
                                                 1,
                                                 1);

            while (1)
            {
                usbOverSerial::write(PROJECT_TARGET_UART_CHANNEL_USB_LINK, packet);

                if (detail::usb::readInternal(cmd))
                {
                    if (cmd == usbOverSerial::internalCMD_t::LINK_READY)
                    {
                        break;
                    }
                }

                core::timing::waitMs(50);
            }
        }
#endif

        void bootloader()
        {
            // partial initialization - init the rest in runBootloader() if it's determined that bootloader should really run

            core::mcu::init(core::mcu::initType_t::BOOT);
            core::mcu::timers::init();
            detail::io::init();

#ifdef PROJECT_TARGET_USB_OVER_SERIAL
            board::uart::init(PROJECT_TARGET_UART_CHANNEL_USB_LINK, board::detail::usb::USB_OVER_SERIAL_BAUDRATE);
#endif
        }

        void application()
        {
            core::mcu::init(core::mcu::initType_t::APP);
            core::mcu::timers::init();

#ifdef PROJECT_TARGET_USB_OVER_SERIAL
            board::uart::init(PROJECT_TARGET_UART_CHANNEL_USB_LINK, board::detail::usb::USB_OVER_SERIAL_BAUDRATE);
#endif

            detail::io::init();
            detail::io::indicators::indicateApplicationLoad();

#ifdef PROJECT_TARGET_USB_OVER_SERIAL_DEVICE
            // do not proceed with application load until usb link is ready
            waitUsbLink();
#endif
        }
    }    // namespace detail::setup
}    // namespace board
