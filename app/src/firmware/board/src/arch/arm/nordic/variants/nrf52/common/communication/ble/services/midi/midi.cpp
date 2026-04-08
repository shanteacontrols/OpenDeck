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

#include "midi.h"
#include "board/board.h"
#include "internal.h"
#include "common/logger/logger.h"

#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "nrf_log.h"
#include "nrf_sdh_ble.h"
#include "core/util/ring_buffer.h"
#include "core/error_handler.h"

namespace
{
    constexpr uint32_t BLE_MIDI_BLE_OBSERVER_PRIO = 2;

    const ble_uuid128_t BLE_UUID_MIDI_SERVICE_BASE_UUID = {
        {
            0x00,
            0xC7,
            0xC4,
            0x4E,
            0xE3,
            0x6C,
            0x51,
            0xA7,
            0x33,
            0x4B,
            0xE8,
            0xED,
            0x5A,
            0x0E,
            0xB8,
            0x03,
        }
    };

    const ble_uuid128_t BLE_UUID_MIDI_DATA_IO_CHAR_BASE_UUID = {
        {
            0xF3,
            0x6B,
            0x10,
            0x9D,
            0x66,
            0xF2,
            0xA9,
            0xA1,
            0x12,
            0x41,
            0x68,
            0x38,
            0xDB,
            0xE5,
            0x72,
            0x77,
        }
    };

    constexpr uint16_t BLE_UUID_MIDI_SERVICE_UUID      = 0x0E5A;
    constexpr uint16_t BLE_UUID_MIDI_DATA_IO_CHAR_UUID = 0xE5DB;

    struct MidiService
    {
        uint16_t                 serviceHandle     = 0;
        ble_gatts_char_handles_t dataIOcharHandles = {};
        uint8_t                  uuidType          = 0;
        uint16_t                 connHandle        = 0;
    };

    MidiService                                                              midiService;
    core::util::RingBuffer<uint8_t, PROJECT_MCU_BUFFER_SIZE_BLE_MIDI_PACKET> rxBuffer;

    uint32_t dataIoCharAdd(MidiService& midiService)
    {
        ble_gatts_char_md_t charMetadata  = {};
        ble_gatts_attr_md_t cccdMetadata  = {};
        ble_gatts_attr_t    attrCharValue = {};
        ble_uuid_t          bleUuid       = {};
        ble_gatts_attr_md_t attrMetadata  = {};

        // Configure the CCCD which is needed for Notifications and Indications
        memset(&cccdMetadata, 0, sizeof(cccdMetadata));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMetadata.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMetadata.write_perm);
        cccdMetadata.vloc = BLE_GATTS_VLOC_STACK;

        // Configure the characteristic metadata.
        memset(&charMetadata, 0, sizeof(charMetadata));
        charMetadata.char_props.read          = 1;
        charMetadata.char_props.write_wo_resp = 1;
        charMetadata.char_props.notify        = 1;
        charMetadata.p_char_user_desc         = NULL;
        charMetadata.p_char_pf                = NULL;
        charMetadata.p_user_desc_md           = NULL;
        charMetadata.p_cccd_md                = &cccdMetadata;
        charMetadata.p_sccd_md                = NULL;

        // Add the MIDI Data I/O Characteristic UUID
        ble_uuid128_t baseUuid = BLE_UUID_MIDI_DATA_IO_CHAR_BASE_UUID;
        CORE_ERROR_CHECK(sd_ble_uuid_vs_add(&baseUuid, &midiService.uuidType), NRF_SUCCESS);

        bleUuid.type = midiService.uuidType;
        bleUuid.uuid = BLE_UUID_MIDI_DATA_IO_CHAR_UUID;

        // Configure the characteristic value's metadata
        memset(&attrMetadata, 0, sizeof(attrMetadata));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMetadata.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMetadata.write_perm);
        attrMetadata.vloc    = BLE_GATTS_VLOC_STACK;
        attrMetadata.rd_auth = 0;
        attrMetadata.wr_auth = 0;
        attrMetadata.vlen    = 1;

        // Configure the characteristic value
        memset(&attrCharValue, 0, sizeof(attrCharValue));
        attrCharValue.p_uuid    = &bleUuid;
        attrCharValue.p_attr_md = &attrMetadata;
        attrCharValue.init_len  = sizeof(uint8_t);
        attrCharValue.init_offs = 0;
        attrCharValue.max_len   = PROJECT_MCU_BUFFER_SIZE_BLE_MIDI_PACKET;
        attrCharValue.p_value   = NULL;

        return sd_ble_gatts_characteristic_add(midiService.serviceHandle, &charMetadata, &attrCharValue, &midiService.dataIOcharHandles);
    }

    void onBleEvent(ble_evt_t const* event, void* context)
    {
        auto midiServicePtr = (MidiService*)context;

        if (midiServicePtr == NULL || event == NULL)
        {
            return;
        }

        switch (event->header.evt_id)
        {
        case BLE_GAP_EVT_CONNECTED:
        {
            LOG_INF("Connected to peer");
            midiService.connHandle = event->evt.gap_evt.conn_handle;
        }
        break;

        case BLE_GAP_EVT_DISCONNECTED:
        {
            LOG_INF("Disconnected from peer");
            midiService.connHandle = BLE_CONN_HANDLE_INVALID;
        }
        break;

        case BLE_GATTS_EVT_WRITE:
        {
            LOG_INF("Write operation performed");
            auto writeEvent = (ble_gatts_evt_write_t*)&event->evt.gatts_evt.params.write;

            // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
            if (writeEvent->handle == midiService.dataIOcharHandles.cccd_handle)
            {
                if (writeEvent->len == 2)
                {
                    // CCCD written

                    if (ble_srv_is_notification_enabled(writeEvent->data))
                    {
                        LOG_INF("Notifications ENABLED on Data I/O Characteristic");
                    }
                    else
                    {
                        LOG_INF("Notifications DISABLED on Data I/O Characteristic");
                        rxBuffer.reset();
                    }
                }
            }
            else if (writeEvent->handle == midiService.dataIOcharHandles.value_handle)
            {
                // midi data
                LOG_INF("Received %d bytes: ", writeEvent->len);

                for (int i = 0; i < writeEvent->len; i++)
                {
                    LOG_INF("%d", writeEvent->data[i]);
                    rxBuffer.insert(writeEvent->data[i]);
                }
            }
        }
        break;

        case BLE_GAP_EVT_PHY_UPDATE:
        {
            ble_gap_evt_phy_update_t const* phyEvt = &event->evt.gap_evt.params.phy_update;

            if (phyEvt->status == BLE_HCI_STATUS_CODE_LMP_ERROR_TRANSACTION_COLLISION)
            {
                // Ignore LL collisions.
                LOG_INF("LL transaction collision during PHY update.");
                break;
            }

            LOG_INF("PHY update %s", (phyEvt->status == BLE_HCI_STATUS_CODE_SUCCESS) ? "accepted" : "rejected");
        }
        break;

        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
        {
            LOG_INF("Exchange MTU Request");
        }
        break;

        case BLE_GATTC_EVT_EXCHANGE_MTU_RSP:
        {
            LOG_INF("Exchange MTU Response");
        }
        break;

        default:
            break;
        }
    }
}    // namespace

NRF_SDH_BLE_OBSERVER(_midi_obs,
                     BLE_MIDI_BLE_OBSERVER_PRIO,
                     onBleEvent,
                     &midiService);

namespace board::ble::midi
{
    bool read(uint8_t* buffer, size_t& size, const size_t maxSize)
    {
        if (maxSize > PROJECT_MCU_BUFFER_SIZE_BLE_MIDI_PACKET)
        {
            return false;
        }

        size = 0;

        while (rxBuffer.size())
        {
            if (!rxBuffer.remove(buffer[size++]))
            {
                break;
            }

            if (size >= maxSize)
            {
                break;
            }
        }

        return size != 0;
    }

    bool write(uint8_t* buffer, size_t size)
    {
        uint32_t          retVal     = NRF_SUCCESS;
        ble_gatts_value_t gattsValue = {};

        memset(&gattsValue, 0, sizeof(gattsValue));

        gattsValue.len     = size;
        gattsValue.offset  = 0;
        gattsValue.p_value = buffer;

        // Update database.
        retVal = sd_ble_gatts_value_set(midiService.connHandle,
                                        midiService.dataIOcharHandles.value_handle,
                                        &gattsValue);
        if (retVal != NRF_SUCCESS)
        {
            return false;
        }

        // Send value if connected and notifying.
        if ((midiService.connHandle != BLE_CONN_HANDLE_INVALID))
        {
            ble_gatts_hvx_params_t hvxParams;

            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = midiService.dataIOcharHandles.value_handle;
            hvxParams.type   = BLE_GATT_HVX_NOTIFICATION;
            hvxParams.offset = gattsValue.offset;
            hvxParams.p_len  = &gattsValue.len;
            hvxParams.p_data = gattsValue.p_value;

            retVal = sd_ble_gatts_hvx(midiService.connHandle, &hvxParams);

            if (retVal != 0)
            {
                LOG_INF("MIDI BLE sending failed");
            }
        }
        else
        {
            retVal = NRF_ERROR_INVALID_STATE;
            LOG_INF("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE");
        }

        return retVal == NRF_SUCCESS;
    }
}    // namespace board::ble::midi

namespace board::detail::ble::midi
{
    bool init()
    {
        uint32_t   retVal  = NRF_SUCCESS;
        ble_uuid_t bleUuid = {};

        // Initialize service structure
        midiService.connHandle = BLE_CONN_HANDLE_INVALID;

        // Add service
        ble_uuid128_t baseUuid = BLE_UUID_MIDI_SERVICE_BASE_UUID;
        retVal                 = sd_ble_uuid_vs_add(&baseUuid, &midiService.uuidType);

        if (retVal != NRF_SUCCESS)
        {
            return false;
        }

        bleUuid.type = midiService.uuidType;
        bleUuid.uuid = BLE_UUID_MIDI_SERVICE_UUID;

        retVal = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &bleUuid, &midiService.serviceHandle);

        if (retVal != NRF_SUCCESS)
        {
            return false;
        }

        retVal = dataIoCharAdd(midiService);

        if (retVal != NRF_SUCCESS)
        {
            return false;
        }

        return true;
    }
}    // namespace board::detail::ble::midi

#endif