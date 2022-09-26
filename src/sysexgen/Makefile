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

SOURCES += sysexgen/main.cpp

DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))