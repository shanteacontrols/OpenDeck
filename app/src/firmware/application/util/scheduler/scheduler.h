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

#include <inttypes.h>
#include <functional>
#include <stddef.h>

namespace util
{
    // a simple scheduler used to run one-off tasks specified time from now
    class Scheduler
    {
        public:
        Scheduler();

        struct Task
        {
            size_t                id       = 0;
            uint32_t              timeout  = 0;
            std::function<void()> function = nullptr;

            Task() = default;

            Task(size_t id, uint32_t timeout, std::function<void()> function)
                : id(id)
                , timeout(timeout)
                , function(std::move(function))
            {}
        };

        bool init();
        void update();
        bool registerTask(Task&& task);

        private:
        static constexpr size_t MAX_TASKS = 10;

        Task _tasks[MAX_TASKS] = {};
    };
}    // namespace util
