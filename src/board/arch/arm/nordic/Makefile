ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += \
    modules/core/modules/tinyusb/src/portable/nordic/nrf5x/dcd_nrf5x.c \
    modules/core/modules/tinyusb/src/class/audio/audio_device.c \
    modules/core/modules/tinyusb/src/class/cdc/cdc_device.c \
    modules/core/modules/tinyusb/src/class/midi/midi_device.c \
    modules/core/modules/tinyusb/src/common/tusb_fifo.c \
    modules/core/modules/tinyusb/src/device/usbd.c \
    modules/core/modules/tinyusb/src/device/usbd_control.c \
    modules/core/modules/tinyusb/src/tusb.c

    SOURCES += $(shell $(FIND) ../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src -name "*.c" | sed "s|^\.\./||")
endif

FLASH_TOOL := nrfjprog

DEFINES += \
USE_TINYUSB \
CFG_TUSB_MCU=OPT_MCU_NRF5X \
CONFIG_NFCT_PINS_AS_GPIOS \
CONFIG_GPIO_AS_PINRESET