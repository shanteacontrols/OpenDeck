/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ble_service.h"

#include <cerrno>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

namespace
{
    LOG_MODULE_REGISTER(midi_ble_service, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    using opendeck::protocol::midi::ble_service::PacketHandler;
    using opendeck::protocol::midi::ble_service::ReadyHandler;

#define BT_UUID_BLE_MIDI_SERVICE_VAL \
    BT_UUID_128_ENCODE(0x03B80E5A, 0xEDE8, 0x4B33, 0xA751, 0x6CE34EC4C700)
#define BT_UUID_BLE_MIDI_CHAR_VAL \
    BT_UUID_128_ENCODE(0x7772E5DB, 0x3868, 0x4112, 0xA1A9, 0xF2669D106BF3)

    static constexpr uint16_t BLE_MIDI_CONN_INTERVAL_MIN    = 0x0006;
    static constexpr uint16_t BLE_MIDI_CONN_INTERVAL_MAX    = 0x0006;
    static constexpr uint16_t BLE_MIDI_CONN_LATENCY         = 0;
    static constexpr uint16_t BLE_MIDI_CONN_TIMEOUT         = 200;
    static constexpr uint32_t BLE_MIDI_CONN_INTERVAL_MIN_US = BLE_MIDI_CONN_INTERVAL_MIN * 1250U;

    static bt_le_conn_param ble_midi_conn_param = BT_LE_CONN_PARAM_INIT(BLE_MIDI_CONN_INTERVAL_MIN,
                                                                        BLE_MIDI_CONN_INTERVAL_MAX,
                                                                        BLE_MIDI_CONN_LATENCY,
                                                                        BLE_MIDI_CONN_TIMEOUT);

    ReadyHandler  ready_handler    = nullptr;
    PacketHandler packet_handler   = nullptr;
    bool          is_initialized   = false;
    bool          is_connected     = false;
    bool          notify_enabled   = false;
    bool          last_ready_state = false;

    void publish_ready_state()
    {
        const bool ready_now = is_connected && notify_enabled;

        if (ready_now == last_ready_state)
        {
            return;
        }

        last_ready_state = ready_now;
        LOG_INF("BLE MIDI ready state: %s", ready_now ? "ready" : "not ready");

        if (ready_handler != nullptr)
        {
            ready_handler(ready_now);
        }
    }

    ssize_t midi_read_cb([[maybe_unused]] struct bt_conn*            conn,
                         [[maybe_unused]] const struct bt_gatt_attr* attr,
                         [[maybe_unused]] void*                      buf,
                         [[maybe_unused]] uint16_t                   len,
                         [[maybe_unused]] uint16_t                   offset)
    {
        return 0;
    }

    ssize_t midi_write_cb([[maybe_unused]] struct bt_conn*            conn,
                          [[maybe_unused]] const struct bt_gatt_attr* attr,
                          const void*                                 buf,
                          uint16_t                                    len,
                          uint16_t                                    offset,
                          [[maybe_unused]] uint8_t                    flags)
    {
        LOG_DBG("BLE MIDI RX packet: len=%u offset=%u", len, offset);

        if (packet_handler != nullptr)
        {
            const auto* bytes = static_cast<const uint8_t*>(buf) + offset;
            packet_handler(bytes, len - offset);
        }

        return len;
    }

    void midi_ccc_cfg_changed([[maybe_unused]] const struct bt_gatt_attr* attr, uint16_t value)
    {
        notify_enabled = value == BT_GATT_CCC_NOTIFY;
        LOG_INF("BLE MIDI notifications %s", notify_enabled ? "enabled" : "disabled");
        publish_ready_state();
    }

#define BT_UUID_MIDI_SERVICE BT_UUID_DECLARE_128(BT_UUID_BLE_MIDI_SERVICE_VAL)
#define BT_UUID_MIDI_CHRC    BT_UUID_DECLARE_128(BT_UUID_BLE_MIDI_CHAR_VAL)

    BT_GATT_SERVICE_DEFINE(opendeck_ble_midi_service,
                           BT_GATT_PRIMARY_SERVICE(BT_UUID_MIDI_SERVICE),
                           BT_GATT_CHARACTERISTIC(BT_UUID_MIDI_CHRC,
                                                  BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                                                  BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                                                  midi_read_cb,
                                                  midi_write_cb,
                                                  nullptr),
                           BT_GATT_CCC(midi_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE));

    void on_notify_done([[maybe_unused]] struct bt_conn* conn, [[maybe_unused]] void* user_data)
    {
    }

    void on_mtu_changed(struct bt_conn* conn, uint16_t mtu_size)
    {
        LOG_INF("BLE MIDI ATT MTU: %u", mtu_size);
        LOG_DBG("BLE MIDI ATT MTU source conn=%p", static_cast<void*>(conn));
    }

    void att_mtu_updated_cb(struct bt_conn* conn, uint16_t tx, uint16_t rx)
    {
        LOG_INF("BLE MIDI MTU updated: tx=%u rx=%u", tx, rx);
        on_mtu_changed(conn, bt_gatt_get_mtu(conn));
    }

    void le_param_updated(struct bt_conn* conn, uint16_t interval, uint16_t latency, uint16_t timeout)
    {
        LOG_INF("BLE MIDI conn params updated: interval=%u ms latency=%u timeout=%u",
                BT_CONN_INTERVAL_TO_MS(interval),
                latency,
                timeout);
        LOG_DBG("BLE MIDI conn param source conn=%p", static_cast<void*>(conn));
    }

    void on_connected([[maybe_unused]] struct bt_conn* conn, uint8_t err)
    {
        if (err != 0)
        {
            LOG_WRN("BLE MIDI connect failed: err=%u", err);
            return;
        }

        is_connected = true;
        LOG_INF("BLE MIDI central connected");
        on_mtu_changed(conn, bt_gatt_get_mtu(conn));

        bt_conn_info info     = {};
        const int    info_ret = bt_conn_get_info(conn, &info);

        if ((info_ret == 0) && (info.type == BT_CONN_TYPE_LE))
        {
            if (info.le.interval_us > BLE_MIDI_CONN_INTERVAL_MIN_US)
            {
                LOG_INF("BLE MIDI conn interval %u ms, requested %u ms: %d",
                        info.le.interval_us / 1000U,
                        BLE_MIDI_CONN_INTERVAL_MIN_US / 1000U,
                        bt_conn_le_param_update(conn, &ble_midi_conn_param));
            }
            else
            {
                LOG_INF("BLE MIDI conn interval already %u ms", info.le.interval_us / 1000U);
            }
        }
        else
        {
            LOG_WRN("BLE MIDI failed to read conn info: %d", info_ret);
        }

        publish_ready_state();
    }

    void on_disconnected([[maybe_unused]] struct bt_conn* conn, [[maybe_unused]] uint8_t reason)
    {
        is_connected   = false;
        notify_enabled = false;
        LOG_INF("BLE MIDI central disconnected: reason=%u (%s)", reason, bt_hci_err_to_str(reason));
        publish_ready_state();
    }

    BT_CONN_CB_DEFINE(opendeck_ble_midi_conn_callbacks) = {
        .connected        = on_connected,
        .disconnected     = on_disconnected,
        .le_param_updated = le_param_updated,
    };

    bt_gatt_cb gatt_callbacks = {
        .att_mtu_updated = att_mtu_updated_cb,
    };

    static const bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
        BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
    };

    static const bt_data sd[] = {
        BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_BLE_MIDI_SERVICE_VAL),
    };
}    // namespace

namespace opendeck::protocol::midi::ble_service
{
    bool init(ReadyHandler new_ready_handler, PacketHandler new_packet_handler)
    {
        ready_handler  = new_ready_handler;
        packet_handler = new_packet_handler;

        if (is_initialized)
        {
            LOG_DBG("BLE MIDI service already initialized");
            publish_ready_state();
            return true;
        }

        is_initialized   = true;
        is_connected     = false;
        notify_enabled   = false;
        last_ready_state = false;
        bt_gatt_cb_register(&gatt_callbacks);
        LOG_INF("BLE MIDI service initialized");

        return true;
    }

    bool start_advertising()
    {
        const int ret = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

        if (ret == 0)
        {
            LOG_INF("BLE MIDI advertising started");
        }
        else if (ret == -EALREADY)
        {
            LOG_DBG("BLE MIDI advertising already active");
        }
        else
        {
            LOG_ERR("BLE MIDI advertising start failed: %d", ret);
        }

        return ret == 0 || ret == -EALREADY;
    }

    bool stop_advertising()
    {
        const int ret = bt_le_adv_stop();

        if (ret == 0)
        {
            LOG_INF("BLE MIDI advertising stopped");
        }
        else if (ret == -EALREADY)
        {
            LOG_DBG("BLE MIDI advertising already stopped");
        }
        else
        {
            LOG_ERR("BLE MIDI advertising stop failed: %d", ret);
        }

        return ret == 0 || ret == -EALREADY;
    }

    bool send_packet(const uint8_t* bytes, uint16_t size)
    {
        if ((bytes == nullptr) || (size == 0) || !ready())
        {
            LOG_DBG("BLE MIDI TX rejected: bytes=%p size=%u ready=%d", bytes, size, ready());
            return false;
        }

        bt_gatt_notify_params notify_params = {
            .attr = &opendeck_ble_midi_service.attrs[1],
            .data = bytes,
            .len  = size,
            .func = on_notify_done,
        };

        const bool sent = bt_gatt_notify_cb(nullptr, &notify_params) == 0;

        if (sent)
        {
            LOG_DBG("BLE MIDI TX packet: len=%u", size);
        }
        else
        {
            LOG_WRN("BLE MIDI TX failed: len=%u", size);
        }

        return sent;
    }

    bool ready()
    {
        return is_connected && notify_enabled;
    }
}    // namespace opendeck::protocol::midi::ble_service
