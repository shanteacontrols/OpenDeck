/*

Copyright Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifdef OPENDECK_FW_APP

#include "board/board.h"
#include "common.h"
#include "internal.h"
#include "services/midi/midi.h"
#include "common/logger/logger.h"
#include "config.h"

#include "nrfx.h"
#include "nrf_sdm.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nrfx_clock.h"
#include "nrfx_saadc.h"
#include "nrf_gpio.h"
#include "nrfx_power.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "ble_conn_params.h"
#include "nrf_nvmc.h"
#include "fds.h"
#include "core/error_handler.h"

namespace
{
    // handle of the current connection
    uint16_t connHandle = BLE_CONN_HANDLE_INVALID;

    ble_uuid_t advUuiDs[] = {
        {
            BLE_UUID_DEVICE_INFORMATION_SERVICE,
            BLE_UUID_TYPE_BLE,
        }
    };

    NRF_BLE_GATT_DEF(_gatt);              // NOLINT
    NRF_BLE_QWR_DEF(_qwr);                // NOLINT
    BLE_ADVERTISING_DEF(_advertising);    // NOLINT

    void bleEvtHandler(ble_evt_t const* event, void* context)
    {
        switch (event->header.evt_id)
        {
        case BLE_GAP_EVT_CONNECTED:
        {
            LOG_INF("Connected to peer");
            connHandle = event->evt.gap_evt.conn_handle;
            CORE_ERROR_CHECK(nrf_ble_qwr_conn_handle_assign(&_qwr, connHandle), NRF_SUCCESS);
        }
        break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            LOG_INF("PHY update request.");

            ble_gap_phys_t const PHYS = {
                .tx_phys = BLE_GAP_PHY_AUTO,
                .rx_phys = BLE_GAP_PHY_AUTO,
            };

            CORE_ERROR_CHECK(sd_ble_gap_phy_update(event->evt.gap_evt.conn_handle, &PHYS), NRF_SUCCESS);
        }
        break;

        case BLE_GATTC_EVT_TIMEOUT:
        {
            // disconnect on GATT Client timeout event
            LOG_INF("GATT Client Timeout.");
            CORE_ERROR_CHECK(sd_ble_gap_disconnect(event->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION), NRF_SUCCESS);
        }
        break;

        case BLE_GATTS_EVT_TIMEOUT:
        {
            // disconnect on GATT Server timeout event
            LOG_INF("GATT Server Timeout.");
            CORE_ERROR_CHECK(sd_ble_gap_disconnect(event->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION), NRF_SUCCESS);
        }
        break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        {
            LOG_INF("Connection parameters update success");
        }
        break;

        case BLE_GAP_EVT_AUTH_STATUS:
        {
            if (event->evt.gap_evt.params.auth_status.auth_status == BLE_GAP_SEC_STATUS_SUCCESS)
            {
                LOG_INF("Authorization succeeded!");
            }
            else
            {
                LOG_INF("Authorization failed with code: %u!",
                        event->evt.gap_evt.params.auth_status.auth_status);
            }
        }
        break;

        case BLE_GAP_EVT_CONN_SEC_UPDATE:
        {
            LOG_INF("Connection security updated");
        }
        break;

        case BLE_GAP_EVT_SEC_INFO_REQUEST:
        {
            LOG_INF("Request to provide security information");
        }
        break;

        case BLE_GAP_EVT_ADV_SET_TERMINATED:
        {
            LOG_INF("Advertising set terminated");
        }
        break;

        case BLE_GAP_OPT_AUTH_PAYLOAD_TIMEOUT:
        {
            LOG_INF("Set Authenticated payload timeout");
        }
        break;

        default:
        {
            // LOG_INF("Common/Unhandled BLE event received. ID = %d", event->header.evt_id);
        }
        break;
        }
    }

    void bleStackInit()
    {
        CORE_ERROR_CHECK(nrf_sdh_enable_request(), NRF_SUCCESS);

        // Configure the BLE stack using the default settings.
        // Fetch the start address of the application RAM.
        uint32_t ramStart = 0;
        CORE_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ramStart), NRF_SUCCESS);

        // Enable BLE stack.
        CORE_ERROR_CHECK(nrf_sdh_ble_enable(&ramStart), NRF_SUCCESS);

        // Register a handler for BLE events.
        NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, bleEvtHandler, NULL);
    }

    void gapParamsInit()
    {
        ble_gap_conn_params_t   gapConnParams = {};
        ble_gap_conn_sec_mode_t secMode       = {};

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&secMode);

        CORE_ERROR_CHECK(sd_ble_gap_device_name_set(&secMode, (const uint8_t*)PROJECT_TARGET_BLE_NAME, strlen(PROJECT_TARGET_BLE_NAME)), NRF_SUCCESS);

        memset(&gapConnParams, 0, sizeof(gapConnParams));

        gapConnParams.min_conn_interval = MIN_CONN_INTERVAL;
        gapConnParams.max_conn_interval = MAX_CONN_INTERVAL;
        gapConnParams.slave_latency     = SLAVE_LATENCY;
        gapConnParams.conn_sup_timeout  = CONN_SUP_TIMEOUT;

        CORE_ERROR_CHECK(sd_ble_gap_ppcp_set(&gapConnParams), NRF_SUCCESS);
    }

    void gattInit()
    {
        CORE_ERROR_CHECK(nrf_ble_gatt_init(&_gatt, NULL), NRF_SUCCESS);
    }

    void advertisingInit()
    {
        ble_advertising_init_t init = {};

        memset(&init, 0, sizeof(init));

        init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
        init.advdata.include_appearance      = true;
        init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
        init.advdata.uuids_complete.uuid_cnt = sizeof(advUuiDs) / sizeof(advUuiDs[0]);
        init.advdata.uuids_complete.p_uuids  = advUuiDs;

        init.config.ble_adv_fast_enabled  = true;
        init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
        init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

        init.evt_handler = [](ble_adv_evt_t advEvent)
        {
            switch (advEvent)
            {
            case BLE_ADV_EVT_FAST:
            {
                LOG_INF("Fast advertising.");
            }
            break;

            case BLE_ADV_EVT_IDLE:
            {
                LOG_INF("Shutting down advertising.");
            }
            break;

            default:
                break;
            }
        };

        CORE_ERROR_CHECK(ble_advertising_init(&_advertising, &init), NRF_SUCCESS);

        ble_advertising_conn_cfg_tag_set(&_advertising, APP_BLE_CONN_CFG_TAG);
    }

    bool servicesInit()
    {
        nrf_ble_qwr_init_t qwrInit = {};

        // Initialize Queued Write Module.
        qwrInit.error_handler = [](uint32_t error)
        {
            core::errorHandler();
        };

        CORE_ERROR_CHECK(nrf_ble_qwr_init(&_qwr, &qwrInit), NRF_SUCCESS);

        if (!board::detail::ble::midi::init())
        {
            return false;
        }

        return true;
    }

    void connParamsInit()
    {
        ble_conn_params_init_t cpInit = {};

        memset(&cpInit, 0, sizeof(cpInit));

        cpInit.p_conn_params                  = NULL;
        cpInit.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
        cpInit.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
        cpInit.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
        cpInit.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
        cpInit.disconnect_on_fail             = true;
        cpInit.evt_handler                    = NULL;
        cpInit.error_handler                  = [](uint32_t error)
        {
            core::errorHandler();
        };

        CORE_ERROR_CHECK(ble_conn_params_init(&cpInit), NRF_SUCCESS);
    }

    void pmHandler(pm_evt_t const* pEvt)
    {
        pm_handler_on_pm_evt(pEvt);
        pm_handler_disconnect_on_sec_failure(pEvt);
        pm_handler_flash_clean(pEvt);

        switch (pEvt->evt_id)
        {
        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            LOG_WRN("Pairing request from an already bonded peer: allowing reconnect");

            pm_conn_sec_config_t config;
            config.allow_repairing = true;
            pm_conn_sec_config_reply(pEvt->conn_handle, &config);
        }
        break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            LOG_INF("PM_EVT_PEERS_DELETE_SUCCEEDED peer manager event.");
            CORE_ERROR_CHECK(ble_advertising_start(&_advertising, BLE_ADV_MODE_FAST), NRF_SUCCESS);
        }
        break;

        default:
            break;
        }
    }

    void peerManagerInit()
    {
        ble_gap_sec_params_t secParam = {};

        CORE_ERROR_CHECK(pm_init(), NRF_SUCCESS);

        memset(&secParam, 0, sizeof(ble_gap_sec_params_t));

        // Security parameters to be used for all security procedures.
        secParam.bond           = SEC_PARAM_BOND;
        secParam.mitm           = SEC_PARAM_MITM;
        secParam.lesc           = SEC_PARAM_LESC;
        secParam.keypress       = SEC_PARAM_KEYPRESS;
        secParam.io_caps        = SEC_PARAM_IO_CAPABILITIES;
        secParam.oob            = SEC_PARAM_OOB;
        secParam.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
        secParam.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
        secParam.kdist_own.enc  = 1;
        secParam.kdist_own.id   = 1;
        secParam.kdist_peer.enc = 1;
        secParam.kdist_peer.id  = 1;

        CORE_ERROR_CHECK(pm_sec_params_set(&secParam), NRF_SUCCESS);
        CORE_ERROR_CHECK(pm_register(pmHandler), NRF_SUCCESS);
    }
}    // namespace

namespace board::ble
{
    bool init()
    {
        // tinyusb could be using power module at this point - make sure it's disabled
        // power is restricted module if sd is in use
        nrfx_power_uninit();
        bleStackInit();
        gapParamsInit();
        gattInit();
        advertisingInit();

        if (!servicesInit())
        {
            return false;
        }

        connParamsInit();
        peerManagerInit();

        // make sure tinyusb is now using sd apis
        sd_power_usbdetected_enable(true);
        sd_power_usbpwrrdy_enable(true);
        sd_power_usbremoved_enable(true);

        CORE_ERROR_CHECK(ble_advertising_start(&_advertising, BLE_ADV_MODE_FAST), NRF_SUCCESS);

        return true;
    }

    bool deInit()
    {
        if (connHandle == BLE_CONN_HANDLE_INVALID)
        {
            // not connected yet, stop advertising
            CORE_ERROR_CHECK(sd_ble_gap_adv_stop(_advertising.adv_handle), NRF_SUCCESS);
        }
        else
        {
            // disconnect
            CORE_ERROR_CHECK(sd_ble_gap_disconnect(connHandle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE), NRF_SUCCESS);
            connHandle = BLE_CONN_HANDLE_INVALID;
        }

        CORE_ERROR_CHECK(nrf_sdh_disable_request(), NRF_SUCCESS);

        return true;
    }
}    // namespace board::ble

#endif