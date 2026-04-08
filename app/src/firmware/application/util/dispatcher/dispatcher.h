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

#include <vector>
#include <functional>

namespace util
{
    template<typename Source, typename Event>
    class Dispatcher
    {
        public:
        using messageCallback_t = std::function<void(const Event& event)>;

        static Dispatcher& instance()
        {
            static Dispatcher instance;
            return instance;
        }

        void listen(Source source, messageCallback_t&& callback)
        {
            _listener.push_back({ source, std::move(callback) });
        }

        void notify(Source source, Event const& event)
        {
            for (size_t i = 0; i < _listener.size(); i++)
            {
                if (_listener[i].source == source)
                {
                    if (_listener[i].callback != nullptr)
                    {
                        _listener[i].callback(event);
                    }
                }
            }
        }

        void clear()
        {
            _listener.clear();
        }

        private:
        Dispatcher() = default;

        struct Listener
        {
            Source            source;
            messageCallback_t callback = nullptr;
        };

        std::vector<Listener> _listener = {};
    };
}    // namespace util