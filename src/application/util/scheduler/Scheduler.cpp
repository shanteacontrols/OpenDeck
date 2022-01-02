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

#include "Scheduler.h"
#include "core/src/general/Timing.h"

using namespace Util;

Scheduler::Scheduler()
{
    init();
}

bool Scheduler::init()
{
    for (size_t i = 0; i < MAX_TASKS; i++)
    {
        _tasks[i].function = nullptr;
        _tasks[i].timeout  = 0;
    }

    return true;
}

void Scheduler::update()
{
    for (size_t i = 0; i < MAX_TASKS; i++)
    {
        if (_tasks[i].function != nullptr)
        {
            if (core::timing::currentRunTimeMs() >= _tasks[i].timeout)
            {
                _tasks[i].function();
                _tasks[i].function = nullptr;
            }
        }
    }
}

bool Scheduler::registerTask(task_t&& task)
{
    size_t index = MAX_TASKS;

    for (size_t i = 0; i < MAX_TASKS; i++)
    {
        // if the id is already registered, cancel its timeout and reassign
        if (_tasks[i].function == nullptr || _tasks[i].id == task.id)
        {
            index = i;
            break;
        }
    }

    if (index >= MAX_TASKS)
        return false;

    _tasks[index].function = std::move(task.function);
    _tasks[index].timeout  = core::timing::currentRunTimeMs() + task.timeout;

    return true;
}