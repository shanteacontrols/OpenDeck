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

#pragma once

#include "core/util/Logger.h"
#include "board/Board.h"

#ifdef BOARD_USE_LOGGER
#ifdef FW_APP
CORE_LOGGER_DECLARE(BOARD_LOGGER, LOGGER_BUFFER_SIZE);

#define LOG_INFO(...)  CORE_LOG_INFO(BOARD_LOGGER, __VA_ARGS__)
#define LOG_WARN(...)  CORE_LOG_WARN(BOARD_LOGGER, __VA_ARGS__)
#define LOG_ERROR(...) CORE_LOG_ERROR(BOARD_LOGGER, __VA_ARGS__)
#else
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#endif
#else
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#endif