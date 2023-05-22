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

SYSEX_BINARY_SUFFIX := _sysex

GEN_FILES += \
$(TARGET_GEN_HEADER)

# Make sure all objects are located in build directory
OBJECTS := $(addprefix $(BUILD_DIR)/,$(SOURCES))
# Also make sure objects have .o extension
OBJECTS := $(addsuffix .o,$(OBJECTS))

# Include generated dependency files to allow incremental build when only headers change
-include $(OBJECTS:%.o=%.d)