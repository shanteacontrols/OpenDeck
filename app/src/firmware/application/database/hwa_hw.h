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

namespace database
{
    class HwaHw : public Hwa
    {
        public:
        HwaHw()
        {
            MidiDispatcher.listen(messaging::eventType_t::SYSTEM,
                                  [this](const messaging::Event& event)
                                  {
                                      switch (event.systemMessage)
                                      {
                                      case messaging::systemMessage_t::RESTORE_START:
                                      {
                                          _writeToCache = true;
                                      }
                                      break;

                                      case messaging::systemMessage_t::RESTORE_END:
                                      {
                                          board::usb::deInit();
                                          board::nvm::writeCacheToFlash();
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
            return board::nvm::init();
        }

        uint32_t size() override
        {
            return board::nvm::size();
        }

        bool clear() override
        {
#ifdef PROJECT_TARGET_SUPPORT_LED_INDICATORS
            // It's possible that LED indicators are still on since
            // this command is most likely given via USB.
            // Wait until all indicators are turned off
            core::mcu::timing::waitMs(board::io::indicators::LED_TRAFFIC_INDICATOR_TIMEOUT);
#endif

            return board::nvm::clear(0, board::nvm::size());
        }

        bool read(uint32_t address, uint32_t& value, database::Admin::sectionParameterType_t type) override
        {
            return board::nvm::read(address, value, boardParamType(type));
        }

        bool write(uint32_t address, uint32_t value, database::Admin::sectionParameterType_t type) override
        {
            return board::nvm::write(address, value, boardParamType(type), _writeToCache);
        }

        bool initializeDatabase() override
        {
            return PROJECT_MCU_DATABASE_INIT_DATA;
        }

        private:
        bool _writeToCache = false;

        board::nvm::parameterType_t boardParamType(database::Admin::sectionParameterType_t type)
        {
            switch (type)
            {
            case database::Admin::sectionParameterType_t::WORD:
                return board::nvm::parameterType_t::WORD;

            case database::Admin::sectionParameterType_t::DWORD:
                return board::nvm::parameterType_t::DWORD;

            default:
                return board::nvm::parameterType_t::BYTE;
            }
        }
    };
}    // namespace database