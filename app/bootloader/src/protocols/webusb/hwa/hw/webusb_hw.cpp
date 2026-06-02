/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/protocols/webusb/hwa/hw/webusb_hw.h"

#include "zlibs/drivers/usb/usb_hw.h"
#include "zlibs/utils/misc/kwork_delayable.h"

#include <zephyr/device.h>
#include <zephyr/drivers/usb/udc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/usb/usbd.h>

#include <algorithm>
#include <cstring>

using namespace opendeck::bootloader;
using namespace opendeck::bootloader::protocols;

namespace opendeck::bootloader::protocols::webusb
{
    struct DfuRxChunk
    {
        usbd_class_data* class_data = nullptr;
        net_buf*         buffer     = nullptr;
        bool             stop       = false;
    };

    struct WebUsbHwAccess
    {
        /**
         * @brief Queues one received DFU buffer through WebUsbHw's private RX path.
         *
         * @param instance WebUSB hardware instance that owns the DFU stream parser.
         * @param chunk Received USB buffer and owning class data.
         *
         * @return `true` when the buffer was queued, otherwise `false`.
         */
        static bool feed(WebUsbHw& instance, DfuRxChunk chunk)
        {
            return instance.feed(chunk);
        }
    };
}    // namespace opendeck::bootloader::protocols::webusb

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_webusb, CONFIG_OPENDECK_LOG_LEVEL);    // NOLINT

    constexpr uint8_t WEBUSB_VENDOR_CODE     = 0x01U;
    constexpr uint8_t WEBUSB_LANDING_PAGE    = 0x01U;
    constexpr uint8_t BULK_OUT_EP_ADDRESS    = 0x01U;
    constexpr uint8_t BULK_IN_EP_ADDRESS     = 0x81U;
    constexpr size_t  HS_BUFFER_SIZE         = 512U;
    constexpr size_t  STATUS_BUFFER_SIZE     = 128U;
    constexpr size_t  STATUS_QUEUE_DEPTH     = 16U;
    constexpr size_t  RX_BUFFER_COUNT        = 4U;
    constexpr size_t  RX_QUEUE_DEPTH         = RX_BUFFER_COUNT;
    constexpr size_t  WEBUSB_URL_LENGTH      = 0x11U;
    constexpr uint8_t WEBUSB_URL_DESC_TYPE   = 0x03U;
    constexpr uint8_t WEBUSB_URL_SCHEME_HTTP = 0x00U;

    const device* const         udc_device = DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0));
    zlibs::drivers::usb::UsbHw& usb_device = zlibs::drivers::usb::UsbHw::instance(udc_device);

    struct UsbBosWebusbDesc
    {
        usb_bos_platform_descriptor platform;
        usb_bos_capability_webusb   capability;
    } __packed;

    static const UsbBosWebusbDesc bos_webusb_capability = {
        .platform = {
            .bLength                = sizeof(usb_bos_platform_descriptor) + sizeof(usb_bos_capability_webusb),
            .bDescriptorType        = USB_DESC_DEVICE_CAPABILITY,
            .bDevCapabilityType     = USB_BOS_CAPABILITY_PLATFORM,
            .bReserved              = 0,
            .PlatformCapabilityUUID = {
                0x38,
                0xB6,
                0x08,
                0x34,
                0xA9,
                0x09,
                0xA0,
                0x47,
                0x8B,
                0xFD,
                0xA0,
                0x76,
                0x88,
                0x15,
                0xB6,
                0x65,
            },
        },
        .capability = {
            .bcdVersion   = sys_cpu_to_le16(0x0100),
            .bVendorCode  = WEBUSB_VENDOR_CODE,
            .iLandingPage = WEBUSB_LANDING_PAGE,
        },
    };

    static const uint8_t webusb_url[] = {
        WEBUSB_URL_LENGTH,
        WEBUSB_URL_DESC_TYPE,
        WEBUSB_URL_SCHEME_HTTP,
        'l',
        'o',
        'c',
        'a',
        'l',
        'h',
        'o',
        's',
        't',
        ':',
        '8',
        '0',
        '0',
        '0',
    };

    static int webusb_to_host_cb(const usbd_context* const,
                                 const usb_setup_packet* const setup,
                                 net_buf* const                buffer)
    {
        constexpr uint8_t WEBUSB_REQ_GET_URL = 0x02U;

        if (setup->wIndex != WEBUSB_REQ_GET_URL)
        {
            return -ENOTSUP;
        }

        const uint8_t index = USB_GET_DESCRIPTOR_INDEX(setup->wValue);

        if (index != WEBUSB_LANDING_PAGE)
        {
            return -ENOTSUP;
        }

        net_buf_add_mem(buffer, &webusb_url, MIN(net_buf_tailroom(buffer), sizeof(webusb_url)));

        return 0;
    }

    USBD_DESC_BOS_VREQ_DEFINE(bootloader_webusb_bos,
                              sizeof(bos_webusb_capability),
                              &bos_webusb_capability,
                              WEBUSB_VENDOR_CODE,
                              webusb_to_host_cb,
                              nullptr);

    UDC_BUF_POOL_DEFINE(webusb_pool,
                        RX_BUFFER_COUNT,
                        HS_BUFFER_SIZE,
                        sizeof(udc_buf_info),
                        nullptr);

    NET_BUF_POOL_FIXED_DEFINE(webusb_status_pool,
                              16,
                              STATUS_BUFFER_SIZE,
                              sizeof(udc_buf_info),
                              nullptr);

    K_MSGQ_DEFINE(webusb_status_msgq, STATUS_BUFFER_SIZE, STATUS_QUEUE_DEPTH, alignof(char));
    K_MSGQ_DEFINE(webusb_rx_msgq, sizeof(webusb::DfuRxChunk), RX_QUEUE_DEPTH, alignof(webusb::DfuRxChunk));

    struct FunctionDesc
    {
        usb_if_descriptor if0;
        usb_ep_descriptor if0_out_ep;
        usb_ep_descriptor if0_in_ep;
        usb_ep_descriptor if0_hs_out_ep;
        usb_ep_descriptor if0_hs_in_ep;
        usb_desc_header   nil_desc;
    };

    struct FunctionData
    {
        FunctionDesc*     desc    = nullptr;
        usb_desc_header** fs_desc = nullptr;
        usb_desc_header** hs_desc = nullptr;
        atomic_t          enabled = ATOMIC_INIT(0);
    };

    struct WebUsbClassPrivate
    {
        FunctionData      function_data;
        usbd_class_data*  active_class_data = nullptr;
        bool              status_saturated  = false;
        bool              status_in_flight  = false;
        webusb::WebUsbHw* instance          = nullptr;
    };

    static FunctionDesc function_desc = {
        .if0 = {
            .bLength            = sizeof(usb_if_descriptor),
            .bDescriptorType    = USB_DESC_INTERFACE,
            .bInterfaceNumber   = 0,
            .bAlternateSetting  = 0,
            .bNumEndpoints      = 2,
            .bInterfaceClass    = USB_BCC_VENDOR,
            .bInterfaceSubClass = 0,
            .bInterfaceProtocol = 0,
            .iInterface         = 0,
        },
        .if0_out_ep = {
            .bLength          = sizeof(usb_ep_descriptor),
            .bDescriptorType  = USB_DESC_ENDPOINT,
            .bEndpointAddress = BULK_OUT_EP_ADDRESS,
            .bmAttributes     = USB_EP_TYPE_BULK,
            .wMaxPacketSize   = sys_cpu_to_le16(64U),
            .bInterval        = 0x00,
        },
        .if0_in_ep = {
            .bLength          = sizeof(usb_ep_descriptor),
            .bDescriptorType  = USB_DESC_ENDPOINT,
            .bEndpointAddress = BULK_IN_EP_ADDRESS,
            .bmAttributes     = USB_EP_TYPE_BULK,
            .wMaxPacketSize   = sys_cpu_to_le16(64U),
            .bInterval        = 0x00,
        },
        .if0_hs_out_ep = {
            .bLength          = sizeof(usb_ep_descriptor),
            .bDescriptorType  = USB_DESC_ENDPOINT,
            .bEndpointAddress = BULK_OUT_EP_ADDRESS,
            .bmAttributes     = USB_EP_TYPE_BULK,
            .wMaxPacketSize   = sys_cpu_to_le16(512U),
            .bInterval        = 0x00,
        },
        .if0_hs_in_ep = {
            .bLength          = sizeof(usb_ep_descriptor),
            .bDescriptorType  = USB_DESC_ENDPOINT,
            .bEndpointAddress = BULK_IN_EP_ADDRESS,
            .bmAttributes     = USB_EP_TYPE_BULK,
            .wMaxPacketSize   = sys_cpu_to_le16(512U),
            .bInterval        = 0x00,
        },
        .nil_desc = {
            .bLength         = 0,
            .bDescriptorType = 0,
        },
    };

    usb_desc_header* function_fs_desc[] = {
        reinterpret_cast<usb_desc_header*>(&function_desc.if0),
        reinterpret_cast<usb_desc_header*>(&function_desc.if0_in_ep),
        reinterpret_cast<usb_desc_header*>(&function_desc.if0_out_ep),
        reinterpret_cast<usb_desc_header*>(&function_desc.nil_desc),
    };

    usb_desc_header* function_hs_desc[] = {
        reinterpret_cast<usb_desc_header*>(&function_desc.if0),
        reinterpret_cast<usb_desc_header*>(&function_desc.if0_hs_in_ep),
        reinterpret_cast<usb_desc_header*>(&function_desc.if0_hs_out_ep),
        reinterpret_cast<usb_desc_header*>(&function_desc.nil_desc),
    };

    WebUsbClassPrivate webusb_class_private = {
        .function_data = {
            .desc    = &function_desc,
            .fs_desc = function_fs_desc,
            .hs_desc = function_hs_desc,
            .enabled = ATOMIC_INIT(0),
        },
        .active_class_data = nullptr,
        .status_saturated  = false,
        .status_in_flight  = false,
        .instance          = nullptr,
    };

    WebUsbClassPrivate& class_private()
    {
        return webusb_class_private;
    }

    WebUsbClassPrivate& class_private(const usbd_class_data* class_data)
    {
        return *static_cast<WebUsbClassPrivate*>(usbd_class_get_private(class_data));
    }

    FunctionData& function_data(const usbd_class_data* class_data)
    {
        return class_private(class_data).function_data;
    }

    webusb::WebUsbHw* webusb_instance(const usbd_class_data* class_data)
    {
        return class_private(class_data).instance;
    }

    uint8_t bulk_in_ep(usbd_class_data* const class_data);

    void flush_status_queue()
    {
        auto&       webusb_class = class_private();
        auto* const class_data   = webusb_class.active_class_data;

        if ((class_data == nullptr) ||
            !atomic_test_bit(&function_data(class_data).enabled, 0) ||
            webusb_class.status_saturated ||
            webusb_class.status_in_flight)
        {
            return;
        }

        auto* const context = usbd_class_get_ctx(class_data);
        auto* const buffer  = net_buf_alloc(&webusb_status_pool, K_NO_WAIT);

        if (buffer == nullptr)
        {
            return;
        }

        char message[STATUS_BUFFER_SIZE] = {};

        if (k_msgq_get(&webusb_status_msgq, message, K_NO_WAIT) != 0)
        {
            usbd_ep_buf_free(context, buffer);
            return;
        }

        net_buf_reset(buffer);
        udc_get_buf_info(buffer)->ep = bulk_in_ep(class_data);

        const size_t message_length = strnlen(message, STATUS_BUFFER_SIZE - 2U);
        net_buf_add_mem(buffer, message, message_length);
        net_buf_add_u8(buffer, '\n');

        if (usbd_ep_enqueue(class_data, buffer))
        {
            usbd_ep_buf_free(context, buffer);
            return;
        }

        webusb_class.status_in_flight = true;
    }

    zlibs::utils::misc::KworkDelayable status_work(flush_status_queue);

    void clear_status_queue()
    {
        char message[STATUS_BUFFER_SIZE];

        while (k_msgq_get(&webusb_status_msgq, message, K_NO_WAIT) == 0)
        {
        }
    }

    void clear_rx_queue()
    {
        webusb::DfuRxChunk chunk;

        while (k_msgq_get(&webusb_rx_msgq, &chunk, K_NO_WAIT) == 0)
        {
            if ((chunk.class_data != nullptr) && (chunk.buffer != nullptr))
            {
                usbd_ep_buf_free(usbd_class_get_ctx(chunk.class_data), chunk.buffer);
            }
        }
    }

    uint8_t bulk_out_ep(usbd_class_data* const class_data)
    {
        auto* const context = usbd_class_get_ctx(class_data);

        if (usbd_bus_speed(context) == USBD_SPEED_HS)
        {
            return function_desc.if0_hs_out_ep.bEndpointAddress;
        }

        return function_desc.if0_out_ep.bEndpointAddress;
    }

    uint8_t bulk_in_ep(usbd_class_data* const class_data)
    {
        auto* const context = usbd_class_get_ctx(class_data);

        if (usbd_bus_speed(context) == USBD_SPEED_HS)
        {
            return function_desc.if0_hs_in_ep.bEndpointAddress;
        }

        return function_desc.if0_in_ep.bEndpointAddress;
    }

    net_buf* buffer_alloc(usbd_class_data* const class_data)
    {
        auto* buffer = net_buf_alloc(&webusb_pool, K_NO_WAIT);

        if (buffer == nullptr)
        {
            return nullptr;
        }

        net_buf_reset(buffer);
        udc_get_buf_info(buffer)->ep = bulk_out_ep(class_data);
        return buffer;
    }

    bool enqueue_rx_buffer(usbd_class_data* const class_data)
    {
        auto* const buffer = buffer_alloc(class_data);

        if (buffer == nullptr)
        {
            LOG_ERR("Failed to allocate WebUSB OUT buffer");
            return false;
        }

        if (usbd_ep_enqueue(class_data, buffer) != 0)
        {
            LOG_ERR("Failed to enqueue WebUSB OUT buffer");
            usbd_ep_buf_free(usbd_class_get_ctx(class_data), buffer);
            return false;
        }

        return true;
    }

    bool enqueue_rx_buffers(usbd_class_data* const class_data)
    {
        bool queued = false;

        for (size_t i = 0; i < RX_BUFFER_COUNT; i++)
        {
            if (!enqueue_rx_buffer(class_data))
            {
                return queued;
            }

            queued = true;
        }

        return queued;
    }

    int request_handler(usbd_class_data* class_data, net_buf* buffer, int err)
    {
        auto* const context  = usbd_class_get_ctx(class_data);
        auto* const buf_info = udc_get_buf_info(buffer);

        if (!atomic_test_bit(&function_data(class_data).enabled, 0) || (err != 0))
        {
            LOG_WRN("Dropping WebUSB request: enabled=%d err=%d",
                    static_cast<int>(atomic_test_bit(&function_data(class_data).enabled, 0)),
                    err);
            usbd_ep_buf_free(context, buffer);
            return 0;
        }

        if (buf_info->ep == bulk_out_ep(class_data))
        {
            webusb::DfuRxChunk chunk = {
                .class_data = class_data,
                .buffer     = buffer,
            };

            auto* const self = webusb_instance(class_data);

            if ((self != nullptr) && webusb::WebUsbHwAccess::feed(*self, chunk))
            {
                return 0;
            }

            LOG_ERR("Failed to queue WebUSB OUT buffer");
            usbd_ep_buf_free(context, buffer);
        }
        else
        {
            auto& webusb_class = class_private(class_data);

            webusb_class.status_in_flight = false;
            webusb_class.status_saturated = false;
            usbd_ep_buf_free(context, buffer);
            status_work.reschedule(0);
        }

        return 0;
    }

    void* get_desc(usbd_class_data* const, const usbd_speed speed)
    {
        if (speed == USBD_SPEED_HS)
        {
            return static_cast<void*>(function_hs_desc);    // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
        }

        return static_cast<void*>(function_fs_desc);    // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    }

    void enable(usbd_class_data* const class_data)
    {
        if (atomic_test_and_set_bit(&function_data(class_data).enabled, 0))
        {
            return;
        }

        auto& webusb_class = class_private(class_data);

        webusb_class.status_saturated = false;
        webusb_class.status_in_flight = false;
        clear_status_queue();
        clear_rx_queue();
        webusb_class.active_class_data = class_data;

        if (!enqueue_rx_buffers(class_data))
        {
            LOG_ERR("Failed to enqueue initial WebUSB OUT buffers");
            return;
        }

        auto* const self = webusb_instance(class_data);

        if (self != nullptr)
        {
            self->status("WebUSB connected");
        }
    }

    void disable(usbd_class_data* const class_data)
    {
        atomic_clear_bit(&function_data(class_data).enabled, 0);
        auto& webusb_class = class_private(class_data);

        webusb_class.status_saturated  = false;
        webusb_class.status_in_flight  = false;
        webusb_class.active_class_data = nullptr;
        clear_status_queue();
        clear_rx_queue();
    }

    int init_class(usbd_class_data*)
    {
        return 0;
    }

    usbd_class_api class_api = {
        .request  = request_handler,
        .enable   = enable,
        .disable  = disable,
        .init     = init_class,
        .get_desc = get_desc,
    };

    USBD_DEFINE_CLASS(opendeck_bootloader_webusb, &class_api, &webusb_class_private, nullptr);
}    // namespace

webusb::WebUsbHw::WebUsbHw(bootloader::dfu::direct_update_writer::DirectUpdateWriter& direct_update_writer)
    : _dfu_stream(direct_update_writer)
    , _rx_thread([this]()
                 {
                     process_rx();
                 },
                 [this]()
                 {
                     stop_rx();
                 })
{
    webusb_class_private.instance = this;
}

void webusb::WebUsbHw::status(std::string_view message)
{
    auto&       webusb_class = class_private();
    auto* const class_data   = webusb_class.active_class_data;

    if (message.empty())
    {
        return;
    }

    if ((class_data == nullptr) ||
        !atomic_test_bit(&function_data(class_data).enabled, 0) ||
        webusb_class.status_saturated)
    {
        return;
    }

    char         status_message[STATUS_BUFFER_SIZE] = {};
    const size_t message_length                     = std::min(message.size(), STATUS_BUFFER_SIZE - 2U);

    std::memcpy(status_message, message.data(), message_length);
    status_message[message_length] = '\0';

    if (k_msgq_put(&webusb_status_msgq, status_message, K_NO_WAIT) != 0)
    {
        webusb_class.status_saturated = true;
        return;
    }

    status_work.reschedule(0);
}

bool webusb::WebUsbHw::feed(DfuRxChunk chunk)
{
    if ((chunk.class_data == nullptr) || (chunk.buffer == nullptr))
    {
        return false;
    }

    if (k_msgq_put(&webusb_rx_msgq, &chunk, K_NO_WAIT) != 0)
    {
        LOG_ERR("Failed to queue WebUSB DFU RX buffer");
        _dfu_stream.reset();
        return false;
    }

    return true;
}

bool webusb::WebUsbHw::init()
{
    usbd_desc_node* descriptors[] = {
        &bootloader_webusb_bos,
    };

    const bool initialized = usb_device.init({
        .extra_descriptors = descriptors,
    });

    if (initialized)
    {
        _rx_thread.run();
    }

    return initialized;
}

bool webusb::WebUsbHw::deinit()
{
    _rx_thread.destroy();

    const bool deinitialized = usb_device.deinit();
    return deinitialized;
}

void webusb::WebUsbHw::process_rx()
{
    while (true)
    {
        DfuRxChunk chunk;

        if (k_msgq_get(&webusb_rx_msgq, &chunk, K_FOREVER) != 0)
        {
            continue;
        }

        if (chunk.stop)
        {
            return;
        }

        const auto data = std::span<const uint8_t>(chunk.buffer->data, chunk.buffer->len);

        if (_dfu_stream.feed(data) == opendeck::common::dfu::dfu_stream_parser::StreamStatus::Invalid)
        {
            _dfu_stream.reset();
        }

        net_buf_reset(chunk.buffer);

        auto* const buf_info = udc_get_buf_info(chunk.buffer);
        buf_info->ep         = bulk_out_ep(chunk.class_data);

        if (atomic_test_bit(&function_data(chunk.class_data).enabled, 0) && (usbd_ep_enqueue(chunk.class_data, chunk.buffer) == 0))
        {
            continue;
        }

        usbd_ep_buf_free(usbd_class_get_ctx(chunk.class_data), chunk.buffer);
    }
}

void webusb::WebUsbHw::stop_rx()
{
    clear_rx_queue();

    DfuRxChunk chunk = {
        .stop = true,
    };

    k_msgq_put(&webusb_rx_msgq, &chunk, K_NO_WAIT);
}
