/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace protocol::midi::ble_service
{
    /**
     * @brief Callback invoked when BLE MIDI readiness changes.
     *
     * @param ready `true` when the BLE MIDI characteristic is ready for
     * communication, otherwise `false`.
     */
    using ReadyHandler = void (*)(bool ready);

    /**
     * @brief Callback invoked when one raw BLE MIDI packet is received.
     *
     * @param bytes Pointer to the received BLE MIDI packet bytes.
     * @param size Number of bytes in the packet.
     */
    using PacketHandler = void (*)(const uint8_t* bytes, uint16_t size);

    /**
     * @brief Initializes the BLE MIDI GATT service callbacks.
     *
     * @param ready_handler Callback notified when BLE MIDI readiness changes.
     * @param packet_handler Callback notified when one raw BLE MIDI packet arrives.
     *
     * @return `true` when the service callbacks were installed successfully.
     */
    bool init(ReadyHandler new_ready_handler, PacketHandler new_packet_handler);

    /**
     * @brief Starts connectable advertising for the BLE MIDI service.
     *
     * @return `true` when advertising is active or was already active.
     */
    bool start_advertising();

    /**
     * @brief Stops connectable advertising for the BLE MIDI service.
     *
     * @return `true` when advertising is stopped or was already inactive.
     */
    bool stop_advertising();

    /**
     * @brief Sends one already encoded BLE MIDI packet through the GATT characteristic.
     *
     * @param bytes Encoded BLE MIDI packet bytes.
     * @param size Number of bytes in the packet.
     *
     * @return `true` when the packet was queued successfully, otherwise `false`.
     */
    bool send_packet(const uint8_t* bytes, uint16_t size);

    /**
     * @brief Returns whether the BLE MIDI service is ready to exchange packets.
     *
     * @return `true` when connected and notifications are enabled.
     */
    bool ready();
}    // namespace protocol::midi::ble_service
