#pragma once

#include "util/messaging/Messaging.h"

class Listener
{
    public:
    Listener() = default;

    void messageListener(const Util::MessageDispatcher::message_t& dispatchMessage)
    {
        _dispatchMessage.push_back(dispatchMessage);
    }

    std::vector<Util::MessageDispatcher::message_t> _dispatchMessage = {};
};