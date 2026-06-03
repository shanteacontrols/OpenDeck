/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace opendeck::firmware::io::i2c::sensor_apds9960
{
    constexpr inline std::array<uint8_t, 4> APDS9960_DEVICE_IDS = {
        0xAB,
        0x9C,
        0xA8,
        0x9E,
    };

    constexpr inline std::array<uint8_t, 1> I2C_ADDRESSES = {
        0x39,
    };

    constexpr inline uint8_t APDS9960_REGISTER_ENABLE     = 0x80;
    constexpr inline uint8_t APDS9960_REGISTER_ATIME      = 0x81;
    constexpr inline uint8_t APDS9960_REGISTER_WTIME      = 0x83;
    constexpr inline uint8_t APDS9960_REGISTER_AILTL      = 0x84;
    constexpr inline uint8_t APDS9960_REGISTER_AILTH      = 0x85;
    constexpr inline uint8_t APDS9960_REGISTER_AIHTL      = 0x86;
    constexpr inline uint8_t APDS9960_REGISTER_AIHTH      = 0x87;
    constexpr inline uint8_t APDS9960_REGISTER_PILT       = 0x89;
    constexpr inline uint8_t APDS9960_REGISTER_PIHT       = 0x8B;
    constexpr inline uint8_t APDS9960_REGISTER_PERS       = 0x8C;
    constexpr inline uint8_t APDS9960_REGISTER_CONFIG1    = 0x8D;
    constexpr inline uint8_t APDS9960_REGISTER_PPULSE     = 0x8E;
    constexpr inline uint8_t APDS9960_REGISTER_CONTROL    = 0x8F;
    constexpr inline uint8_t APDS9960_REGISTER_CONFIG2    = 0x90;
    constexpr inline uint8_t APDS9960_REGISTER_ID         = 0x92;
    constexpr inline uint8_t APDS9960_REGISTER_STATUS     = 0x93;
    constexpr inline uint8_t APDS9960_REGISTER_CDATAL     = 0x94;
    constexpr inline uint8_t APDS9960_REGISTER_RDATAL     = 0x96;
    constexpr inline uint8_t APDS9960_REGISTER_GDATAL     = 0x98;
    constexpr inline uint8_t APDS9960_REGISTER_BDATAL     = 0x9A;
    constexpr inline uint8_t APDS9960_REGISTER_PDATA      = 0x9C;
    constexpr inline uint8_t APDS9960_REGISTER_POFFSET_UR = 0x9D;
    constexpr inline uint8_t APDS9960_REGISTER_POFFSET_DL = 0x9E;
    constexpr inline uint8_t APDS9960_REGISTER_CONFIG3    = 0x9F;
    constexpr inline uint8_t APDS9960_REGISTER_GPENTH     = 0xA0;
    constexpr inline uint8_t APDS9960_REGISTER_GEXTH      = 0xA1;
    constexpr inline uint8_t APDS9960_REGISTER_GCONF1     = 0xA2;
    constexpr inline uint8_t APDS9960_REGISTER_GCONF2     = 0xA3;
    constexpr inline uint8_t APDS9960_REGISTER_GOFFSET_U  = 0xA4;
    constexpr inline uint8_t APDS9960_REGISTER_GOFFSET_D  = 0xA5;
    constexpr inline uint8_t APDS9960_REGISTER_GPULSE     = 0xA6;
    constexpr inline uint8_t APDS9960_REGISTER_GOFFSET_L  = 0xA7;
    constexpr inline uint8_t APDS9960_REGISTER_GOFFSET_R  = 0xA9;
    constexpr inline uint8_t APDS9960_REGISTER_GCONF3     = 0xAA;
    constexpr inline uint8_t APDS9960_REGISTER_GCONF4     = 0xAB;
    constexpr inline uint8_t APDS9960_REGISTER_GFLVL      = 0xAE;
    constexpr inline uint8_t APDS9960_REGISTER_GSTATUS    = 0xAF;
    constexpr inline uint8_t APDS9960_REGISTER_GFIFO_U    = 0xFC;

    constexpr inline uint8_t APDS9960_ENABLE_PON     = 0x01;
    constexpr inline uint8_t APDS9960_ENABLE_AEN     = 0x02;
    constexpr inline uint8_t APDS9960_ENABLE_PEN     = 0x04;
    constexpr inline uint8_t APDS9960_ENABLE_GEN     = 0x40;
    constexpr inline uint8_t APDS9960_ENABLE_GESTURE = APDS9960_ENABLE_PON | APDS9960_ENABLE_PEN | APDS9960_ENABLE_GEN;
    constexpr inline uint8_t APDS9960_STATUS_AVALID  = 0x01;
    constexpr inline uint8_t APDS9960_STATUS_PVALID  = 0x02;
    constexpr inline uint8_t APDS9960_GSTATUS_GVALID = 0x01;
    constexpr inline uint8_t APDS9960_GCONF4_GMODE   = 0x01;

    constexpr inline uint8_t APDS9960_CONTROL_LED_DRIVE_SHIFT      = 6;
    constexpr inline uint8_t APDS9960_CONTROL_PROXIMITY_GAIN_SHIFT = 2;

    constexpr inline uint8_t  APDS9960_DEFAULT_ATIME          = 219;
    constexpr inline uint8_t  APDS9960_DEFAULT_WTIME          = 246;
    constexpr inline uint8_t  APDS9960_DEFAULT_PPULSE         = 0x87;
    constexpr inline uint8_t  APDS9960_DEFAULT_POFFSET_UR     = 0;
    constexpr inline uint8_t  APDS9960_DEFAULT_POFFSET_DL     = 0;
    constexpr inline uint8_t  APDS9960_DEFAULT_CONFIG1        = 0x60;
    constexpr inline uint8_t  APDS9960_DEFAULT_ALS_GAIN       = 0x01;
    constexpr inline uint8_t  APDS9960_DEFAULT_LED_DRIVE      = 0x00;
    constexpr inline uint8_t  APDS9960_DEFAULT_PROXIMITY_GAIN = 0x02;
    constexpr inline uint8_t  APDS9960_DEFAULT_PILT           = 0;
    constexpr inline uint8_t  APDS9960_DEFAULT_PIHT           = 50;
    constexpr inline uint16_t APDS9960_DEFAULT_AILT           = 0xFFFF;
    constexpr inline uint16_t APDS9960_DEFAULT_AIHT           = 0;
    constexpr inline uint8_t  APDS9960_DEFAULT_PERS           = 0x11;
    constexpr inline uint8_t  APDS9960_DEFAULT_CONFIG2        = 0x01;
    constexpr inline uint8_t  APDS9960_DEFAULT_CONFIG3        = 0x00;
    constexpr inline uint8_t  APDS9960_DEFAULT_GCONF1         = 0x40;
    constexpr inline uint8_t  APDS9960_DEFAULT_GCONF2         = 0x41;
    constexpr inline uint8_t  APDS9960_DEFAULT_GCONF3         = 0x00;
    constexpr inline uint8_t  APDS9960_DEFAULT_GPENTH         = 0x28;
    constexpr inline uint8_t  APDS9960_DEFAULT_GEXTH          = 0x1E;
    constexpr inline uint8_t  APDS9960_DEFAULT_GPULSE         = 0xC9;
    constexpr inline uint8_t  APDS9960_DEFAULT_GOFFSET        = 0;
}    // namespace opendeck::firmware::io::i2c::sensor_apds9960
