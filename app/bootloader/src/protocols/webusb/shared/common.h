/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

struct usbd_class_data;
struct net_buf;

namespace opendeck::bootloader::protocols::webusb
{
    /** @brief Full-speed WebUSB bulk endpoint packet size. */
    constexpr inline uint16_t FS_PACKET_SIZE = 64U;

    /** @brief High-speed WebUSB bulk endpoint packet size. */
    constexpr inline uint16_t HS_PACKET_SIZE = 512U;

    /** @brief Maximum raw WebUSB OUT payload size accepted per USB packet. */
    constexpr inline size_t USB_BUFFER_SIZE = HS_PACKET_SIZE;

    /**
     * @brief Received WebUSB DFU buffer owned by the hardware adapter.
     */
    struct DfuRxChunk
    {
        std::span<const uint8_t> data       = {};
        usbd_class_data*         class_data = nullptr;
        net_buf*                 buffer     = nullptr;
    };
}    // namespace opendeck::bootloader::protocols::webusb
