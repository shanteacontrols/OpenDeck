#pragma once

#include "application/messaging/Messaging.h"

class Listener
{
    public:
    Listener() = default;

    void messageListener(const messaging::event_t& event)
    {
        _event.push_back(event);
    }

    std::vector<messaging::event_t> _event = {};
};