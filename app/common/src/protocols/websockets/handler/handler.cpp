/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "common/src/protocols/websockets/handler/handler.h"

using namespace opendeck::common::protocols::websockets;

Handler::Handler()
{
    registered_handlers.push_back(this);
}

std::span<Handler*> Handler::handlers()
{
    return std::span<Handler*>(registered_handlers.data(), registered_handlers.size());
}

void Handler::clear_handlers()
{
    registered_handlers.clear();
}
