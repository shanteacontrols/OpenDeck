/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bootloader/src/webusb/instance/impl/transport.h"

#ifdef CONFIG_PROJECT_BOOTLOADER_SUPPORT_USB_DFU
#include "bootloader/src/updater/builder/builder.h"

#include "zlibs/drivers/usb/usb_hw.h"

#include <zephyr/device.h>
#include <zephyr/drivers/usb/udc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usbd.h>

#include <cstring>

using namespace opendeck;

namespace
{
    LOG_MODULE_REGISTER(opendeck_bootloader_webusb, LOG_LEVEL_INF);    // NOLINT

    constexpr uint8_t WEBUSB_VENDOR_CODE     = 0x01U;
    constexpr uint8_t WEBUSB_LANDING_PAGE    = 0x01U;
    constexpr uint8_t BULK_OUT_EP_ADDRESS    = 0x01U;
    constexpr uint8_t BULK_IN_EP_ADDRESS     = 0x81U;
    constexpr size_t  HS_BUFFER_SIZE         = 512U;
    constexpr size_t  FS_BUFFER_SIZE         = 64U;
    constexpr size_t  STATUS_BUFFER_SIZE     = 128U;
    constexpr size_t  STATUS_QUEUE_DEPTH     = 16U;
    constexpr size_t  WEBUSB_URL_LENGTH      = 0x11U;
    constexpr uint8_t WEBUSB_URL_DESC_TYPE   = 0x03U;
    constexpr uint8_t WEBUSB_URL_SCHEME_HTTP = 0x00U;

    const device* const         udc_device = DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0));
    zlibs::drivers::usb::UsbHw& usb_device = zlibs::drivers::usb::UsbHw::instance(udc_device);

    void cleanup_callback()
    {
        (void)usb_device.deinit();
    }

    updater::Builder updater_builder(cleanup_callback);

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

    NET_BUF_POOL_FIXED_DEFINE(webusb_pool,
                              1,
                              0,
                              sizeof(udc_buf_info),
                              nullptr);

    NET_BUF_POOL_FIXED_DEFINE(webusb_status_pool,
                              16,
                              STATUS_BUFFER_SIZE,
                              sizeof(udc_buf_info),
                              nullptr);

    K_MSGQ_DEFINE(webusb_status_msgq, STATUS_BUFFER_SIZE, STATUS_QUEUE_DEPTH, 4);

    UDC_STATIC_BUF_DEFINE(webusb_buffer, HS_BUFFER_SIZE);

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
        FunctionDesc*     desc;
        usb_desc_header** fs_desc;
        usb_desc_header** hs_desc;
        atomic_t          enabled;
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

    FunctionData function_data = {
        .desc    = &function_desc,
        .fs_desc = function_fs_desc,
        .hs_desc = function_hs_desc,
        .enabled = ATOMIC_INIT(0),
    };

    usbd_class_data* active_class_data = nullptr;
    bool             status_saturated  = false;
    bool             status_in_flight  = false;
    struct k_work    status_work;

    uint8_t bulk_in_ep(usbd_class_data* const class_data);

    void flush_status_queue(struct k_work*)
    {
        if ((active_class_data == nullptr) ||
            !atomic_test_bit(&function_data.enabled, 0) ||
            status_saturated ||
            status_in_flight)
        {
            return;
        }

        usbd_context* const context = usbd_class_get_ctx(active_class_data);
        net_buf* const      buffer  = net_buf_alloc(&webusb_status_pool, K_NO_WAIT);

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
        udc_get_buf_info(buffer)->ep = bulk_in_ep(active_class_data);

        const size_t message_length = strnlen(message, STATUS_BUFFER_SIZE - 2U);
        net_buf_add_mem(buffer, message, message_length);
        net_buf_add_u8(buffer, '\n');

        if (usbd_ep_enqueue(active_class_data, buffer))
        {
            usbd_ep_buf_free(context, buffer);
            return;
        }

        status_in_flight = true;
    }

    void clear_status_queue()
    {
        char message[STATUS_BUFFER_SIZE];

        while (k_msgq_get(&webusb_status_msgq, message, K_NO_WAIT) == 0)
        {
        }
    }

    uint8_t bulk_out_ep(usbd_class_data* const class_data)
    {
        usbd_context* const context = usbd_class_get_ctx(class_data);

        if (usbd_bus_speed(context) == USBD_SPEED_HS)
        {
            return function_desc.if0_hs_out_ep.bEndpointAddress;
        }

        return function_desc.if0_out_ep.bEndpointAddress;
    }

    uint8_t bulk_in_ep(usbd_class_data* const class_data)
    {
        usbd_context* const context = usbd_class_get_ctx(class_data);

        if (usbd_bus_speed(context) == USBD_SPEED_HS)
        {
            return function_desc.if0_hs_in_ep.bEndpointAddress;
        }

        return function_desc.if0_in_ep.bEndpointAddress;
    }

    net_buf* buffer_alloc(usbd_class_data* const class_data)
    {
        usbd_context* const context = usbd_class_get_ctx(class_data);
        const size_t        size    = usbd_bus_speed(context) == USBD_SPEED_HS ? HS_BUFFER_SIZE : FS_BUFFER_SIZE;

        net_buf* buffer = net_buf_alloc_with_data(&webusb_pool, webusb_buffer, size, K_NO_WAIT);

        if (buffer == nullptr)
        {
            return nullptr;
        }

        net_buf_reset(buffer);
        udc_get_buf_info(buffer)->ep = bulk_out_ep(class_data);
        return buffer;
    }

    int request_handler(usbd_class_data* class_data, net_buf* buffer, int err)
    {
        usbd_context* const context  = usbd_class_get_ctx(class_data);
        udc_buf_info* const buf_info = udc_get_buf_info(buffer);

        if (!atomic_test_bit(&function_data.enabled, 0) || (err != 0))
        {
            LOG_WRN("Dropping WebUSB request: enabled=%d err=%d",
                    static_cast<int>(atomic_test_bit(&function_data.enabled, 0)),
                    err);
            usbd_ep_buf_free(context, buffer);
            return 0;
        }

        if (buf_info->ep == bulk_out_ep(class_data))
        {
            for (uint16_t i = 0; i < buffer->len; i++)
            {
                updater_builder.instance().feed(buffer->data[i]);
            }

            net_buf_reset(buffer);
            buf_info->ep = bulk_out_ep(class_data);

            if (usbd_ep_enqueue(class_data, buffer))
            {
                LOG_ERR("Failed to requeue WebUSB OUT buffer");
                usbd_ep_buf_free(context, buffer);
            }
        }
        else
        {
            status_in_flight = false;
            status_saturated = false;
            usbd_ep_buf_free(context, buffer);
            k_work_submit(&status_work);
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
        if (atomic_test_and_set_bit(&function_data.enabled, 0))
        {
            return;
        }

        status_saturated = false;
        status_in_flight = false;
        clear_status_queue();
        active_class_data = class_data;
        net_buf* buffer   = buffer_alloc(class_data);

        if (buffer == nullptr)
        {
            LOG_ERR("Failed to allocate initial WebUSB OUT buffer");
            return;
        }

        if (usbd_ep_enqueue(class_data, buffer))
        {
            LOG_ERR("Failed to enqueue initial WebUSB OUT buffer");
            usbd_ep_buf_free(usbd_class_get_ctx(class_data), buffer);
        }

        webusb::status("WebUSB connected");
    }

    void disable(usbd_class_data* const)
    {
        atomic_clear_bit(&function_data.enabled, 0);
        status_saturated = false;
        status_in_flight = false;
        clear_status_queue();
        active_class_data = nullptr;
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

    USBD_DEFINE_CLASS(opendeck_bootloader_webusb, &class_api, &function_data, nullptr);

}    // namespace

void webusb::status(const char* message)
{
    if (message == nullptr)
    {
        return;
    }

    printk("%s\n", message);

    if ((active_class_data == nullptr) ||
        !atomic_test_bit(&function_data.enabled, 0) ||
        status_saturated)
    {
        return;
    }

    char         status_message[STATUS_BUFFER_SIZE] = {};
    const size_t message_length                     = strnlen(message, STATUS_BUFFER_SIZE - 2U);

    std::memcpy(status_message, message, message_length);
    status_message[message_length] = '\0';

    if (k_msgq_put(&webusb_status_msgq, status_message, K_NO_WAIT) != 0)
    {
        status_saturated = true;
        return;
    }

    k_work_submit(&status_work);
}

bool webusb::Transport::init()
{
    k_work_init(&status_work, flush_status_queue);

    usbd_desc_node* descriptors[] = {
        &bootloader_webusb_bos,
    };

    return usb_device.init({
        .extra_descriptors = descriptors,
    });
}

bool webusb::Transport::deinit()
{
    const bool deinitialized = usb_device.deinit();
    return deinitialized;
}
#else
namespace opendeck::webusb
{
    void status(const char*)
    {}

    bool Transport::init()
    {
        return false;
    }

    bool Transport::deinit()
    {
        return true;
    }
}    // namespace opendeck::webusb
#endif
