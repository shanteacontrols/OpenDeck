/*

Copyright 2015-2021 Igor Petrovic

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

namespace Util
{
    // a simple scheduler used to run one-off tasks specified time from now
    class Scheduler
    {
        public:
        Scheduler();

        struct task_t
        {
            std::function<void()> function = nullptr;
            uint32_t              timeout  = 0;

            task_t() = default;
        };

        void update();
        bool registerTask(task_t&& task);

        private:
        static constexpr size_t MAX_TASKS         = 10;
        task_t                  _tasks[MAX_TASKS] = {};
    };
}    // namespace Util
