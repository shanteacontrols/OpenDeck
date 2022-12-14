vpath modules/%.c ../
vpath modules/%.cpp ../
vpath modules/%.s ../
vpath modules/%.S ../

# Common include dirs
INCLUDE_DIRS += \
-I"./" \
-I"$(BOARD_GEN_DIR_MCU_BASE)/$(CORE_MCU_MODEL)" \
-I"$(BOARD_GEN_DIR_TARGET)/" \
-I"board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common" \
-I"board/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common" \
-I"../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/common" \
-I"../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/common" \
-I"board/common" \
-I"../modules/"

ifneq (,$(findstring USE_TINYUSB,$(DEFINES)))
    INCLUDE_DIRS += \
    -I"board/common/communication/usb/tinyusb" \
    -I"../modules/core/modules/tinyusb/hw" \
    -I"../modules/core/modules/tinyusb/src"
endif

LINKER_FILE       := ../modules/core/src/arch/$(CORE_MCU_ARCH)/$(CORE_MCU_VENDOR)/variants/$(CORE_MCU_FAMILY)/$(CORE_MCU_MODEL)/$(CORE_MCU_MODEL).ld
TARGET_GEN_HEADER := $(BOARD_GEN_DIR_TARGET)/Target.h

ifneq (,$(wildcard $(DEF_FILE_TSCREEN)))
    TSCREEN_GEN_SOURCE += $(APP_GEN_DIR_TARGET)/Touchscreen.cpp
endif

GEN_FILES += \
$(TARGET_GEN_HEADER) \
$(TSCREEN_GEN_SOURCE)

# Make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
# Also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

# Include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)