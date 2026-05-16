/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/protocol/midi/shared/common.h"
#include "firmware/src/io/common/shared/common.h"
#include "firmware/src/database/instance/impl/database.h"

#include <zephyr/kernel.h>

#include <algorithm>
#include <array>
#include <optional>
#include <span>

namespace opendeck::protocol::midi
{
    /**
     * @brief Database view used by the MIDI subsystem for global settings.
     */
    using Database = database::User<database::Config::Section::Global>;

    /**
     * @brief Hardware abstraction for the USB MIDI transport.
     */
    class HwaUsb : public zlibs::utils::midi::usb::Hwa
    {
        public:
        ~HwaUsb() override = default;
        using zlibs::utils::midi::Transport::write;

        /**
         * @brief Returns the poll signal raised when USB MIDI input becomes available.
         *
         * @return Poll signal used to wait for USB MIDI data.
         */
        virtual k_poll_signal* data_available_signal() = 0;

        /**
         * @brief Returns whether USB MIDI is ready for packet exchange.
         *
         * @return `true` when the USB MIDI function is ready for TX/RX.
         */
        virtual bool ready() = 0;

        /**
         * @brief Writes a burst of UMP packets through the USB backend.
         *
         * The default implementation forwards packets one by one through
         * `write(const midi_ump&)`.
         *
         * @param packets UMP packets to transmit.
         *
         * @return `true` if all packets were sent successfully, otherwise `false`.
         */
        virtual bool write(std::span<const midi_ump> packets)
        {
            for (const auto& packet : packets)
            {
                if (!this->write(packet))
                {
                    return false;
                }
            }

            return true;
        }
    };

    /**
     * @brief Hardware abstraction for the DIN/serial MIDI transport.
     */
    class HwaSerial : public io::common::Allocatable, public zlibs::utils::midi::serial::Hwa
    {
        public:
        ~HwaSerial() override = default;

        /**
         * @brief Enables or disables transport loopback.
         *
         * @param state Desired loopback state.
         *
         * @return `true` if the state was applied, otherwise `false`.
         */
        virtual bool set_loopback(bool state) = 0;

        /**
         * @brief Returns the poll signal raised when serial MIDI input becomes available.
         *
         * @return Poll signal used to wait for serial MIDI data.
         */
        virtual k_poll_signal* data_available_signal() = 0;
    };

    /**
     * @brief Hardware abstraction for the BLE MIDI transport.
     */
    class HwaBle : public zlibs::utils::midi::ble::Hwa
    {
        public:
        ~HwaBle() override = default;

        /**
         * @brief Returns the poll signal raised when BLE MIDI input becomes available.
         *
         * @return Poll signal used to wait for BLE MIDI data.
         */
        virtual k_poll_signal* data_available_signal() = 0;

        /**
         * @brief Returns whether the BLE MIDI transport is ready for packet exchange.
         *
         * @return `true` when BLE MIDI is connected and notifications are enabled.
         */
        virtual bool ready() = 0;
    };

    /**
     * @brief OpenDeck transport capability extension.
     */
    class OpenDeckTransport : public virtual zlibs::utils::midi::Base
    {
        public:
        ~OpenDeckTransport() override = default;

        /**
         * @brief Returns whether this transport is ready for packet exchange.
         *
         * @return `true` when ready, otherwise `false`.
         */
        virtual bool ready() const
        {
            return true;
        }

        /**
         * @brief Wakes any thread waiting for this transport.
         */
        void wake()
        {
            k_poll_signal_raise(_poll_event.signal, 0);
        }

        /**
         * @brief Waits until one transport has pending input.
         *
         * @tparam TransportCount Number of transports in the array.
         *
         * @param transports Transports to wait on.
         *
         * @return Ready transport index, or `std::nullopt` when polling fails.
         */
        template<size_t TransportCount>
        static std::optional<size_t> wait_for_data(const std::array<OpenDeckTransport*, TransportCount>& transports)
        {
            std::array<k_poll_event, TransportCount> poll_events = {};

            for (size_t i = 0; i < transports.size(); i++)
            {
                poll_events[i]       = transports[i]->_poll_event;
                poll_events[i].state = K_POLL_STATE_NOT_READY;
            }

            if (k_poll(poll_events.data(), static_cast<int>(poll_events.size()), K_FOREVER) != 0)
            {
                return {};
            }

            for (size_t i = 0; i < poll_events.size(); i++)
            {
                if (poll_events[i].state != K_POLL_STATE_SIGNALED)
                {
                    continue;
                }

                k_poll_signal_reset(poll_events[i].signal);
                return i;
            }

            return {};
        }

        /**
         * @brief Wakes every transport in the array.
         *
         * @tparam TransportCount Number of transports in the array.
         *
         * @param transports Transports to wake.
         */
        template<size_t TransportCount>
        static void wake_all(const std::array<OpenDeckTransport*, TransportCount>& transports)
        {
            for (auto* transport : transports)
            {
                transport->wake();
            }
        }

        protected:
        /**
         * @brief Initializes the poll event with the transport input signal.
         *
         * @param data_available_signal Poll signal raised when transport input is available.
         */
        void init_poll_event(k_poll_signal* data_available_signal)
        {
            k_poll_event_init(
                &_poll_event,
                K_POLL_TYPE_SIGNAL,
                K_POLL_MODE_NOTIFY_ONLY,
                data_available_signal);
        }

        /**
         * @brief Tracks a thru destination registered for this transport.
         *
         * @param destination Destination thru sink.
         *
         * @return `true` if the destination is already tracked or was added,
         *         otherwise `false`.
         */
        bool add_thru_destination(zlibs::utils::midi::Thru& destination)
        {
            if (thru_destination_registered(destination))
            {
                return true;
            }

            auto empty_destination = std::find(_thru_destinations.begin(), _thru_destinations.end(), nullptr);

            if (empty_destination == _thru_destinations.end())
            {
                return false;
            }

            *empty_destination = &destination;
            return true;
        }

        /**
         * @brief Stops tracking a thru destination registered for this transport.
         *
         * @param destination Destination thru sink.
         *
         * @return `true` if the destination was tracked and removed, otherwise `false`.
         */
        bool remove_thru_destination(zlibs::utils::midi::Thru& destination)
        {
            auto registered_destination = std::find(_thru_destinations.begin(), _thru_destinations.end(), &destination);

            if (registered_destination == _thru_destinations.end())
            {
                return false;
            }

            *registered_destination = nullptr;
            return true;
        }

        /**
         * @brief Returns whether a thru destination is currently tracked.
         *
         * @param destination Destination thru sink.
         *
         * @return `true` if the destination is tracked, otherwise `false`.
         */
        bool thru_destination_registered(zlibs::utils::midi::Thru& destination) const
        {
            return std::find(_thru_destinations.begin(), _thru_destinations.end(), &destination) != _thru_destinations.end();
        }

        /**
         * @brief Returns the number of currently tracked thru destinations.
         *
         * @return Number of active destinations.
         */
        size_t thru_destination_count() const
        {
            return static_cast<size_t>(std::count_if(_thru_destinations.begin(),
                                                     _thru_destinations.end(),
                                                     [](auto* destination)
                                                     {
                                                         return destination != nullptr;
                                                     }));
        }

        private:
        using DestinationList = std::array<zlibs::utils::midi::Thru*, CONFIG_ZLIBS_UTILS_MIDI_MAX_THRU_INTERFACES>;

        DestinationList _thru_destinations = {};
        k_poll_event    _poll_event        = {};
    };

    /**
     * @brief OpenDeck USB MIDI transport.
     */
    class UsbMidi : public Usb, public OpenDeckTransport
    {
        public:
        explicit UsbMidi(HwaUsb& hwa)
            : Usb(hwa)
            , _hwa(hwa)
        {
            init_poll_event(_hwa.data_available_signal());
        }

        bool ready() const override
        {
            return _hwa.ready();
        }

        bool write(std::span<const midi_ump> packets)
        {
            return _hwa.write(packets);
        }

        private:
        HwaUsb& _hwa;
    };

    /**
     * @brief OpenDeck DIN/serial MIDI transport.
     */
    class SerialMidi : public Serial, public OpenDeckTransport
    {
        public:
        explicit SerialMidi(HwaSerial& hwa)
            : Serial(hwa)
            , _hwa(hwa)
        {
            init_poll_event(_hwa.data_available_signal());
        }

        bool allocated(io::common::Allocatable::Interface interface) const
        {
            return _hwa.allocated(interface);
        }

        protected:
        bool enable_thru_route(zlibs::utils::midi::Thru& destination) override
        {
            if (!add_thru_destination(destination))
            {
                return false;
            }

            if (apply_loopback_state())
            {
                return true;
            }

            remove_thru_destination(destination);
            return false;
        }

        void disable_thru_route(zlibs::utils::midi::Thru& destination) override
        {
            if (!remove_thru_destination(destination))
            {
                return;
            }

            apply_loopback_state();
        }

        bool has_thru_bypass(zlibs::utils::midi::Thru& destination) override
        {
            return (&destination == &thru_interface()) && _loopback_enabled;
        }

        private:
        bool loopback_route_active()
        {
            return thru_destination_registered(thru_interface()) && (thru_destination_count() == 1);
        }

        bool apply_loopback_state()
        {
            const bool loopback_requested = loopback_route_active();

            if (loopback_requested == _loopback_enabled)
            {
                return true;
            }

            if (!_hwa.set_loopback(loopback_requested))
            {
                return false;
            }

            _loopback_enabled = loopback_requested;
            return true;
        }

        HwaSerial& _hwa;
        bool       _loopback_enabled = false;
    };

    /**
     * @brief OpenDeck BLE MIDI transport.
     */
    class BleMidi : public Ble, public OpenDeckTransport
    {
        public:
        explicit BleMidi(HwaBle& hwa)
            : Ble(hwa)
            , _hwa(hwa)
        {
            init_poll_event(_hwa.data_available_signal());
        }

        bool ready() const override
        {
            return _hwa.ready();
        }

        private:
        HwaBle& _hwa;
    };
}    // namespace opendeck::protocol::midi
