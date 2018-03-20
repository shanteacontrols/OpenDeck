# root of Google Test, relative to where this file is.
GTEST_DIR := ../../../googletest/googletest

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += --coverage
CPPFLAGS += -isystem $(GTEST_DIR)/include
CPPFLAGS += -include "interface/AnalogStubs.h"

#common include directories
INCLUDE_DIRS := \
-I"../modules/"

DEFINES := 

# Flags passed to the C++ compiler.
CXXFLAGS += -g -pthread
ARFLAGS := r

#all user sources
SOURCES := $(shell find . -name "*.cpp")
SOURCES += ../firmware/interface/analog/Potentiometer.cpp
SOURCES += ../firmware/interface/analog/Analog.cpp
SOURCES += ../firmware/interface/analog/FSR.cpp
SOURCES += ../firmware/interface/cinfo/CInfo.cpp

#make sure all objects are located in build directory
OBJECTS := $(addprefix build/,$(SOURCES))
#also make sure objects have .o extension
OBJECTS := $(OBJECTS:.cpp=.o)

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS := $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

Tests.out: $(OBJECTS) $(GTEST_HEADERS) build/gtest_main.a
	@echo Creating executable...
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@
	@echo Executable $@ created.

clean:
	@echo Cleaning up.
	@rm -rf *.o *.info *.gcda *.gcno *.a *.html *.png *.infoclear *.css build/ firmware/ *.out

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
build/gtest-all.o: $(GTEST_SRCS_)
	@echo Building: $<
	@mkdir -p $(@D)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o build/gtest-all.o

build/gtest_main.o: $(GTEST_SRCS_)
	@echo Building: $<
	@mkdir -p $(@D)
	@$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc -o build/gtest_main.o

build/gtest.a: build/gtest-all.o
	@echo Building: $<
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) $@ $^

build/gtest_main.a: build/gtest-all.o build/gtest_main.o
	@echo Building: $<
	@mkdir -p $(@D)
	@$(AR) $(ARFLAGS) $@ $^

# A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.
build/%.o: %.cpp
	@echo Building: $<
	mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(addprefix -D,$(DEFINES)) $(INCLUDE_DIRS) -std=c++11 $(CXXFLAGS) -c $< -o $@

#debugging
print-%:
	@echo '$*=$($*)'
