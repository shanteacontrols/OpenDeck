/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstddef>
#include <stdint.h>

namespace opendeck::io::i2c::sensor_bno085
{
    static constexpr std::array<uint8_t, 2> I2C_ADDRESSES                     = { 0x4A, 0x4B };
    static constexpr std::array<uint8_t, 5> SHTP_SOFT_RESET                   = { 0x05, 0x00, 0x01, 0x00, 0x01 };
    static constexpr uint8_t                SHTP_HEADER_SIZE                  = 4;
    static constexpr uint8_t                SHTP_CONTROL_CHANNEL              = 0x02;
    static constexpr uint8_t                SHTP_SET_FEATURE_COMMAND          = 0xFD;
    static constexpr uint16_t               SHTP_CONTINUATION                 = 0x8000;
    static constexpr uint16_t               SHTP_PACKET_SIZE_MAX              = 384;
    static constexpr uint8_t                SET_FEATURE_COMMAND_SIZE          = 21;
    static constexpr uint8_t                SENSOR_REPORT_SIZE                = 21;
    static constexpr uint8_t                SENSOR_REPORT_ID_OFFSET           = 9;
    static constexpr uint8_t                SENSOR_REPORT_DATA_OFFSET         = 13;
    static constexpr uint8_t                SET_FEATURE_INTERVAL_OFFSET       = 9;
    static constexpr uint8_t                GYROSCOPE_REPORT_ID               = 0x02;
    static constexpr uint8_t                LINEAR_ACCEL_REPORT_ID            = 0x04;
    static constexpr uint8_t                GRAVITY_REPORT_ID                 = 0x06;
    static constexpr uint8_t                ROTATION_VECTOR_REPORT_ID         = 0x28;
    static constexpr int32_t                ROTATION_VECTOR_SCALE             = 1 << 14;
    static constexpr int32_t                GYROSCOPE_SCALE                   = 1 << 9;
    static constexpr int32_t                ACCELERATION_SCALE                = 1 << 8;
    static constexpr float                  RADIANS_TO_DEGREES                = 57.2957795F;
    static constexpr int16_t                ROTATION_VECTOR_PUBLISH_THRESHOLD = 4;
    static constexpr int16_t                GYROSCOPE_PUBLISH_THRESHOLD       = 4;
    static constexpr int16_t                LINEAR_ACCEL_PUBLISH_THRESHOLD    = 32;
    static constexpr int16_t                GRAVITY_PUBLISH_THRESHOLD         = 16;
    static constexpr uint8_t                PUBLISH_CONFIRMATION_SAMPLES      = 1;
    static constexpr uint32_t               REPORT_INTERVAL_US                = 10000;
    static constexpr int32_t                SOFT_RESET_DELAY_MS               = 300;
    static constexpr int32_t                FEATURE_ENABLE_DELAY_MS           = 10;
    static constexpr uint8_t                STARTUP_READ_RETRIES              = 10;
    static constexpr int32_t                STARTUP_READ_DELAY_MS             = 20;
    static constexpr size_t                 REPORT_COUNT                      = 4;
}    // namespace opendeck::io::i2c::sensor_bno085
