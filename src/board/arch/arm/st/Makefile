ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/tinyusb/lib/CMSIS_5/CMSIS/Core/Include" \
    -I"../modules/tinyusb/hw/mcu/st/cmsis_device_f4/Include" \
    -I"../modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Inc" \
    -I"../modules/tinyusb/hw/bsp/stm32f4" \
    -I"../modules/tinyusb/hw" \
    -I"../modules/tinyusb/src"

    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")

    SOURCES += \
    modules/tinyusb/hw/bsp/board.c \
    modules/tinyusb/hw/mcu/st/cmsis_device_f4/Source/Templates/system_stm32f4xx.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_adc.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_cortex.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_flash_ex.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_gpio.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_i2c.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_rcc_ex.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_tim_ex.c \
    modules/tinyusb/hw/mcu/st/stm32f4xx_hal_driver/Src/stm32f4xx_hal_uart.c \
    modules/tinyusb/src/class/audio/audio_device.c \
    modules/tinyusb/src/class/cdc/cdc_device.c \
    modules/tinyusb/src/class/dfu/dfu_device.c \
    modules/tinyusb/src/class/dfu/dfu_rt_device.c \
    modules/tinyusb/src/class/hid/hid_device.c \
    modules/tinyusb/src/class/midi/midi_device.c \
    modules/tinyusb/src/class/msc/msc_device.c \
    modules/tinyusb/src/class/net/ecm_rndis_device.c \
    modules/tinyusb/src/class/net/ncm_device.c \
    modules/tinyusb/src/class/usbtmc/usbtmc_device.c \
    modules/tinyusb/src/class/vendor/vendor_device.c \
    modules/tinyusb/src/class/video/video_device.c \
    modules/tinyusb/src/common/tusb_fifo.c \
    modules/tinyusb/src/device/usbd.c \
    modules/tinyusb/src/device/usbd_control.c \
    modules/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c \
    modules/tinyusb/src/tusb.c

    SOURCES := $(filter-out %_template.c,$(SOURCES))
endif

FLASH_TOOL := stlink

DEFINES += \
CORE_VENDOR_ST \
IO_REG_WIDTH=16 \
USE_HAL_DRIVER \
UID_BITS=96 \
USE_USB_FS \
DEVICE_FS=0 \
DEVICE_HS=1 \
ADC_12_BIT \
USE_TINYUSB \
CFG_TUSB_MCU=OPT_MCU_STM32F4 \
BOARD_USE_UPDATE_HOOKS