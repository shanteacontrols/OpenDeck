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

#include "digital.h"

using namespace io::digital;

Digital::Digital(drivers::DriverBase&    driver,
                 FrameStore&             frameStore,
                 io::buttons::Buttons&   buttons,
                 io::encoders::Encoders& encoders)
    : _driver(driver)
    , _frameStore(frameStore)
    , _buttons(buttons)
    , _encoders(encoders)
    , _thread([&]()
              {
                  while (1)
                  {
                      auto frame = _driver.read();

                      if (!frame.has_value())
                      {
                          goto sleep;
                      }

                      _frameStore.setFrame(frame.value());
                      _buttons.updateAll();
                      _encoders.updateAll();
                      _frameStore.clear();

                  sleep:
                      k_msleep(THREAD_SLEEP_TIME_MS);
                  }
              })
{
}

bool Digital::init()
{
    auto ret = _driver.init() && _buttons.init() && _encoders.init();

    if (!ret)
    {
        return false;
    }

    _thread.run();

    return true;
}

void Digital::updateSingle([[maybe_unused]] size_t index, [[maybe_unused]] bool forceRefresh)
{
}

void Digital::updateAll([[maybe_unused]] bool forceRefresh)
{
}

size_t Digital::maxComponentUpdateIndex()
{
    return 1;
}
