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

#pragma once

#include "app_util.h"
#include "app_timer.h"
#include "ble_gap.h"

#define APP_BLE_CONN_CFG_TAG           1                               /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO          3                               /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define MIN_CONN_INTERVAL              MSEC_TO_UNITS(11, UNIT_1_25_MS) /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL              MSEC_TO_UNITS(15, UNIT_1_25_MS) /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                  0                               /**< Slave latency. */
#define CONN_SUP_TIMEOUT               MSEC_TO_UNITS(4000, UNIT_10_MS) /**< Connection supervisory timeout (4 seconds). */
#define APP_ADV_INTERVAL               150                             /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_DURATION               18000                           /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000)           /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(30000)          /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT   3                               /**< Number of attempts before giving up the connection parameter negotiation. */
#define SEC_PARAM_BOND                 1                               /**< Perform bonding. */
#define SEC_PARAM_MITM                 0                               /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                 0                               /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS             0                               /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES      BLE_GAP_IO_CAPS_NONE            /**< No I/O capabilities. */
#define SEC_PARAM_OOB                  0                               /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE         7                               /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE         16                              /**< Maximum encryption key size. */