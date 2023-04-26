-include $(BOARD_GEN_DIR)/Makefile
-include board/src/arch/common/Makefile
-include board/src/arch/$(CORE_MCU_ARCH)/Makefile
-include board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/Makefile
-include board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/Makefile
-include board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL)/Makefile

SOURCES += $(shell $(FIND) board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common/ -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
SOURCES += $(shell $(FIND) board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
SOURCES += $(shell $(FIND) board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL) -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
SOURCES += $(shell $(FIND) board/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/common -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
SOURCES += $(shell $(FIND) board/src/arch/$(CORE_MCU_ARCH)/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")
SOURCES += $(shell $(FIND) board/src/common -type f -regex '.*\.\(s\|c\|cpp\)' | sed "s|^\.\./||")

INCLUDE_DIRS += \
-I"board/include"