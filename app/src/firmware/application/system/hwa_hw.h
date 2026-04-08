/*

Copyright Igor Petrovic

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

#pragma once

#include "deps.h"
#include "application/messaging/messaging.h"
#include "board/board.h"

#include "core/mcu.h"

namespace sys
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw()
        {
            MidiDispatcher.listen(messaging::eventType_t::SYSTEM,
                                  [](const messaging::Event& event)
                                  {
                                      switch (event.systemMessage)
                                      {
                                      case messaging::systemMessage_t::FACTORY_RESET_START:
                                      {
                                          board::usb::deInit();
                                          board::io::indicators::indicateFactoryReset();
                                      }
                                      break;

                                      case messaging::systemMessage_t::FACTORY_RESET_END:
                                      {
                                          board::reboot();
                                      }
                                      break;

                                      default:
                                          break;
                                      }
                                  });
        }

        bool init() override
        {
            board::init();
            return true;
        }

        void update() override
        {
            board::update();

            static uint32_t lastCheckTime       = 0;
            static bool     lastConnectionState = false;

            if (core::mcu::timing::ms() - lastCheckTime > USB_CONN_CHECK_TIME)
            {
                bool newState = board::usb::isUsbConnected();

                if (newState)
                {
                    if (!lastConnectionState)
                    {
                        if (_usbConnectionHandler != nullptr)
                        {
                            _usbConnectionHandler();
                        }
                    }
                }

                lastConnectionState = newState;
                lastCheckTime       = core::mcu::timing::ms();
            }
        }

        void reboot(fw_selector::fwType_t type) override
        {
            auto value = static_cast<uint32_t>(type);

            board::bootloader::setMagicBootValue(value);
            board::reboot();
        }

        void registerOnUSBconnectionHandler(usbConnectionHandler_t&& usbConnectionHandler) override
        {
            _usbConnectionHandler = std::move(usbConnectionHandler);
        }

        private:
        static constexpr uint32_t USB_CONN_CHECK_TIME   = 2000;
        usbConnectionHandler_t    _usbConnectionHandler = nullptr;
    };
}    // namespace sys
