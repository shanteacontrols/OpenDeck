ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/core/modules/tinyusb/hw/mcu/st/cmsis_device_f4/Include" \
    -I"../modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Inc" \
    -I"../modules/core/modules/tinyusb/hw/bsp/stm32f4" \
    -I"../modules/core/modules/tinyusb/hw" \
    -I"../modules/core/modules/tinyusb/src"

    SOURCES += \
    modules/core/modules/tinyusb/hw/mcu/st/cmsis_device_f4/Source/Templates/system_stm32f4xx.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_adc.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_cortex.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash_ex.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_gpio.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2c.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc_ex.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim_ex.c \
    modules/core/modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_uart.c \
    modules/core/modules/tinyusb/src/class/audio/audio_device.c \
    modules/core/modules/tinyusb/src/class/cdc/cdc_device.c \
    modules/core/modules/tinyusb/src/class/dfu/dfu_device.c \
    modules/core/modules/tinyusb/src/class/dfu/dfu_rt_device.c \
    modules/core/modules/tinyusb/src/class/hid/hid_device.c \
    modules/core/modules/tinyusb/src/class/midi/midi_device.c \
    modules/core/modules/tinyusb/src/class/msc/msc_device.c \
    modules/core/modules/tinyusb/src/class/net/ecm_rndis_device.c \
    modules/core/modules/tinyusb/src/class/net/ncm_device.c \
    modules/core/modules/tinyusb/src/class/usbtmc/usbtmc_device.c \
    modules/core/modules/tinyusb/src/class/vendor/vendor_device.c \
    modules/core/modules/tinyusb/src/class/video/video_device.c \
    modules/core/modules/tinyusb/src/common/tusb_fifo.c \
    modules/core/modules/tinyusb/src/device/usbd.c \
    modules/core/modules/tinyusb/src/device/usbd_control.c \
    modules/core/modules/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c \
    modules/core/modules/tinyusb/src/tusb.c

    SOURCES := $(filter-out %_template.c,$(SOURCES))
endif

FLASH_TOOL := stlink

DEFINES += \
CORE_VENDOR_ST \
IO_REG_WIDTH=16 \
USE_HAL_DRIVER \
USE_USB_FS \
DEVICE_FS=0 \
DEVICE_HS=1 \
ADC_12_BIT \
USE_TINYUSB \
CFG_TUSB_MCU=OPT_MCU_STM32F4 \
BOARD_USE_UPDATE_HOOKS