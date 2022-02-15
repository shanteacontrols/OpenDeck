ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/tinyusb/lib/CMSIS_5/CMSIS/Core/Include" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx/soc" \
    -I"../modules/tinyusb/hw/bsp/nrf/boards/pca10056" \
    -I"../modules/tinyusb/hw/mcu/nordic" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx/mdk" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx/hal" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx/drivers/include" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src/prs" \
    -I"../modules/tinyusb/hw/mcu/nordic/nrf5x/s140_nrf52_6.1.1_API/include" \
    -I"../modules/tinyusb/hw/bsp/nrf" \
    -I"../modules/tinyusb/hw" \
    -I"../modules/tinyusb/src"

    SOURCES += \
    modules/tinyusb/hw/mcu/nordic/nrfx/mdk/system_nrf52840.c \
    modules/tinyusb/hw/bsp/board.c

    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")
    SOURCES += $(shell $(FIND) ../modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src -name "*.c" | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/tinyusb/src -name "*.c" | sed "s|^\.\./||")
endif

FLASH_TOOL := nrfjprog

DEFINES += \
CORE_VENDOR_NORDIC \
IO_REG_WIDTH=32 \
UID_BITS=64 \
ADC_12_BIT \
USE_TINYUSB \
CFG_TUSB_MCU=OPT_MCU_NRF5X \
BOARD_USE_UPDATE_HOOKS \
CONFIG_NFCT_PINS_AS_GPIOS \
CONFIG_GPIO_AS_PINRESET