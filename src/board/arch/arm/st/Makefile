ifeq (,$(findstring gen,$(TYPE)))
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
    modules/core/modules/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c \
    modules/core/modules/tinyusb/src/class/audio/audio_device.c \
    modules/core/modules/tinyusb/src/class/cdc/cdc_device.c \
    modules/core/modules/tinyusb/src/class/midi/midi_device.c \
    modules/core/modules/tinyusb/src/common/tusb_fifo.c \
    modules/core/modules/tinyusb/src/device/usbd.c \
    modules/core/modules/tinyusb/src/device/usbd_control.c \
    modules/core/modules/tinyusb/src/tusb.c

    SOURCES := $(filter-out %_template.c,$(SOURCES))
endif

FLASH_TOOL := stlink

DEFINES += \
USE_TINYUSB \
CFG_TUSB_MCU=OPT_MCU_STM32F4 \
BOARD_USE_UPDATE_HOOKS