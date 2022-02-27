#pragma once

#include "messaging/Messaging.h"

class Listener
{
    public:
    Listener() = default;

    void messageListener(const Messaging::event_t& event)
    {
        _event.push_back(event);
    }

    std::vector<Messaging::event_t> _event = {};
};