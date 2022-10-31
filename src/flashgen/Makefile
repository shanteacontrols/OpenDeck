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

ifeq ($(CORE_MCU_ARCH),arm)
    SOURCES += $(shell $(FIND) application/database -type f -name "*.cpp")
    SOURCES += $(shell $(FIND) ../modules/dbms/src -maxdepth 1 -type f -name "*.cpp" | sed "s|^\.\./||")
    SOURCES += $(TSCREEN_GEN_SOURCE)
    SOURCES += application/util/configurable/Configurable.cpp
    SOURCES += flashgen/main.cpp
    SOURCES += modules/EmuEEPROM/src/EmuEEPROM.cpp
endif

# Same as app
DEFINES += FW_APP
FLASH_START_ADDR := $(FLASH_ADDR_APP_START)
DEFINES += CORE_MCU_STUB