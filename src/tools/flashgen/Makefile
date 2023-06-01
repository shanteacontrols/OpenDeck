CC        := $(CC_NATIVE)
CXX       := $(CXX_NATIVE)
LD        := $(LD_NATIVE)
SIZE_TOOL := size
LDFLAGS   := 
LDLIBS    := 
CXXFLAGS  := -std=c++17
CPPFLAGS  :=

ifeq ($(DEBUG), 1)
    CPPFLAGS := -g -gdwarf-2
    OPT      := -O0
else
    OPT := -O3
endif

INCLUDE_DIRS := $(filter-out %avr-libstdcpp/include",$(INCLUDE_DIRS))

SOURCES :=

ifeq ($(CORE_MCU_ARCH),arm)
    SOURCES += $(shell $(FIND) firmware/application/database -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
    SOURCES += $(shell $(FIND) $(APPLICATION_GEN_DIR_TARGET) -type f -name "*.cpp")
    SOURCES += firmware/application/util/configurable/Configurable.cpp
    SOURCES += tools/flashgen/main.cpp
    SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
endif

# Same as app
DEFINES += FW_APP
DEFINES += CORE_MCU_STUB