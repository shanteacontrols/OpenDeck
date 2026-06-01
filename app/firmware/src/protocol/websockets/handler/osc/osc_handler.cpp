/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "firmware/src/protocol/websockets/handler/osc/osc_handler.h"
#include "firmware/src/protocol/osc/packet/packet.h"
#include "firmware/src/signaling/signaling.h"

using namespace opendeck::protocol::websockets::osc;
using namespace opendeck::firmware;

namespace
{
    template<typename Signal>
    void forward_osc_event(opendeck::common::protocols::websockets::HandlerEndpoint& endpoint, const Signal& signal)
    {
        if (signal.direction != signaling::SignalDirection::Out)
        {
            return;
        }

        opendeck::protocol::osc::PacketBuffer packet = {};
        const auto                            size   = opendeck::protocol::osc::make_packet(packet, signal);

        if (!size)
        {
            return;
        }

        endpoint.queue_frame(std::span<const uint8_t>(packet.data(), *size));
    }
}    // namespace

void OscHandler::init(opendeck::common::protocols::websockets::HandlerEndpoint& endpoint)
{
    signaling::subscribe<signaling::OscIoSignal>(
        [&endpoint](const signaling::OscIoSignal& event)
        {
            forward_osc_event(endpoint, event);
        });

    signaling::subscribe<signaling::OscSensorSignal>(
        [&endpoint](const signaling::OscSensorSignal& event)
        {
            forward_osc_event(endpoint, event);
        });
}
