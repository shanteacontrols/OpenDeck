ifeq (,$(findstring gen,$(TYPE)))
    SOURCES += \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_gpio/gpio.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_stdlib/stdlib.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_claim/claim.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_platform/platform.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_sync/sync.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_uart/uart.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_divider/divider.S \
    modules/core/modules/pico-sdk/src/common/pico_time/time.c \
    modules/core/modules/pico-sdk/src/common/pico_time/timeout_helper.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_timer/timer.c \
    modules/core/modules/pico-sdk/src/common/pico_sync/sem.c \
    modules/core/modules/pico-sdk/src/common/pico_sync/lock_core.c \
    modules/core/modules/pico-sdk/src/common/pico_sync/mutex.c \
    modules/core/modules/pico-sdk/src/common/pico_sync/critical_section.c \
    modules/core/modules/pico-sdk/src/common/pico_util/datetime.c \
    modules/core/modules/pico-sdk/src/common/pico_util/pheap.c \
    modules/core/modules/pico-sdk/src/common/pico_util/queue.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_runtime/runtime.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_clocks/clocks.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_irq/irq.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_pll/pll.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_irq/irq_handler_chain.S \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_vreg/vreg.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_watchdog/watchdog.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_xosc/xosc.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_printf/printf.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_bootrom/bootrom.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_bit_ops/bit_ops_aeabi.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_divider/divider.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_double/double_aeabi.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_double/double_init_rom.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_double/double_math.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_double/double_v1_rom_shim.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_int64_ops/pico_int64_ops_aeabi.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_float/float_aeabi.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_float/float_init_rom.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_float/float_math.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_float/float_v1_rom_shim.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_malloc/pico_malloc.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_mem_ops/mem_ops_aeabi.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_standard_link/crt0.S \
    modules/core/modules/pico-sdk/src/rp2_common/pico_standard_link/new_delete.cpp \
    modules/core/modules/pico-sdk/src/rp2_common/pico_stdio/stdio.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_stdio_uart/stdio_uart.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_flash/flash.c \
    modules/core/modules/pico-sdk/src/rp2_common/pico_unique_id/unique_id.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_i2c/i2c.c \
    modules/core/modules/pico-sdk/src/rp2_common/hardware_adc/adc.c

    SOURCES += $(shell $(FIND) ../modules/core/modules/tinyusb/src -name "*.c" | sed "s|^\.\./||")
endif

DEFINES += \
USE_TINYUSB \
LIB_TINYUSB_DEVICE=1 \
PICO_COPY_TO_RAM=0 \
PICO_NO_FLASH=0

USE_UF2       := 1
UF2_FAMILY_ID := 0xe48bff56