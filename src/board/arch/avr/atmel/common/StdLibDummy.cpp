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

#include <stdio.h>
#include <stdlib.h>
#include "board/Internal.h"

extern "C" void cxa_pure_virtual()
{
    Board::detail::errorHandler();
}

void* operator new(std::size_t size)
{
    return malloc(size);
}

void operator delete(void* ptr)
{
    free(ptr);
}

namespace std
{
    void __throw_bad_function_call()
    {
        Board::detail::errorHandler();

        while (1)
        {
            ;
        }
    }

    void __throw_bad_alloc()
    {
        Board::detail::errorHandler();

        while (1)
        {
            ;
        }
    }

    void __throw_out_of_range(char const* __s)
    {
        Board::detail::errorHandler();

        while (1)
        {
            ;
        }
    }

    void __throw_length_error(char const* __s)
    {
        Board::detail::errorHandler();

        while (1)
        {
            ;
        }
    }
}    // namespace std