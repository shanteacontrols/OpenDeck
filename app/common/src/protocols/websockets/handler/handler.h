/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace opendeck::common::protocols::websockets
{
    /**
     * @brief Endpoint exposed to WebSockets handlers.
     */
    class HandlerEndpoint
    {
        public:
        virtual ~HandlerEndpoint() = default;

        /**
         * @brief Checks whether a WebSockets client session is still active.
         *
         * @param session_id WebSockets session id.
         *
         * @return `true` when the session is still active.
         */
        virtual bool session_active(uint32_t session_id) = 0;

        /**
         * @brief Queues one frame for a WebSockets client session.
         *
         * @param data       Complete frame payload.
         * @param session_id WebSockets session id.
         */
        virtual void queue_frame(std::span<const uint8_t> data, uint32_t session_id) = 0;

        /**
         * @brief Queues one frame for the active WebSockets client session.
         *
         * @param data Complete frame payload.
         */
        virtual void queue_frame(std::span<const uint8_t> data) = 0;
    };

    /**
     * @brief Base for WebSockets protocol extensions.
     */
    class Handler
    {
        public:
        /**
         * @brief Registers this handler in the WebSockets handler list.
         */
        Handler();

        virtual ~Handler() = default;

        /**
         * @brief Returns registered WebSockets handlers.
         *
         * @return Registered handlers.
         */
        static std::span<Handler*> handlers();

        /**
         * @brief Clears the registered WebSockets handler list.
         */
        static void clear_handlers();

        /**
         * @brief Resets transient state owned by this handler.
         */
        virtual void reset()
        {}

        /**
         * @brief Initializes the handler with its endpoint.
         *
         * @param endpoint WebSockets endpoint used to queue frames.
         */
        virtual void init([[maybe_unused]] HandlerEndpoint& endpoint)
        {}

        /**
         * @brief Handles WebSockets client session close.
         *
         * @param session_id Closed WebSockets session id.
         */
        virtual void on_close_session([[maybe_unused]] uint32_t session_id)
        {}

        /**
         * @brief Handles one WebSockets frame.
         *
         * @param data       Frame payload bytes.
         * @param session_id Active WebSockets session id.
         *
         * @return Response bytes when this handler accepted the command, an empty span when it accepted
         *         the command without a response, otherwise `std::nullopt`.
         */
        virtual std::optional<std::span<const uint8_t>> handle_frame(
            [[maybe_unused]] std::span<const uint8_t> data,
            [[maybe_unused]] uint32_t                 session_id)
        {
            return std::nullopt;
        }

        private:
        static inline std::vector<Handler*> registered_handlers = {};
    };
}    // namespace opendeck::common::protocols::websockets
