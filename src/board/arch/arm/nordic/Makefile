ifeq (,$(findstring gen,$(TYPE)))
    INCLUDE_DIRS += \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/soc" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/mdk" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/hal" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/drivers/include" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src" \
    -I"../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src/prs" \
    -I"../modules/core/modules/tinyusb/hw/bsp/nrf" \
    -I"../modules/core/modules/tinyusb/hw" \
    -I"../modules/core/modules/tinyusb/src"

    SOURCES += $(shell $(FIND) ./board/arch/$(ARCH)/$(VENDOR)/variants/$(MCU_FAMILY) -maxdepth 1 -name "*.cpp")
    SOURCES += $(shell $(FIND) ../modules/core/modules/tinyusb/hw/mcu/nordic/nrfx/drivers/src -name "*.c" | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) ../modules/core/modules/tinyusb/src -name "*.c" | sed "s|^\.\./||")
endif

FLASH_TOOL := nrfjprog

DEFINES += \
CORE_VENDOR_NORDIC \
IO_REG_WIDTH=32 \
ADC_12_BIT \
USE_TINYUSB \
CFG_TUSB_MCU=OPT_MCU_NRF5X \
BOARD_USE_UPDATE_HOOKS \
CONFIG_NFCT_PINS_AS_GPIOS \
CONFIG_GPIO_AS_PINRESET