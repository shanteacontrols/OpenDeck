vpath modules/%.c ../
vpath modules/%.cpp ../
vpath modules/%.s ../
vpath modules/%.S ../

# Common include dirs
INCLUDE_DIRS += \
-I"firmware" \
-I"$(MCU_GEN_DIR)" \
-I"$(TARGET_GEN_DIR)/" \
-I"../modules/" \
-I"../modules/EmuEEPROM/include"

TARGET_GEN_HEADER := $(TARGET_GEN_DIR)/Target.h

ifneq (,$(wildcard $(DEF_FILE_TSCREEN)))
    TSCREEN_GEN_SOURCE := $(APPLICATION_GEN_DIR_TARGET)/Touchscreen.cpp
endif

SYSEX_BINARY_SUFFIX := _sysex

GEN_FILES += \
$(TARGET_GEN_HEADER) \
$(TSCREEN_GEN_SOURCE)

# Make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
# Also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

# Include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)