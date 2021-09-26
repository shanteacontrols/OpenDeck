#pragma once

#include "util/messaging/Messaging.h"

class Listener
{
    public:
    Listener() = default;

    void messageListener(const IO::MessageDispatcher::message_t& dispatchMessage)
    {
        _dispatchMessage.push_back(dispatchMessage);
    }

    std::vector<IO::MessageDispatcher::message_t> _dispatchMessage = {};
};