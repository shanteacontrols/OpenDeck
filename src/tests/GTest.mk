#gtest specific

GTEST_DIR := ../modules/googletest/googletest
GTEST_HEADERS := $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ := $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

build/gtest-all.o: $(GTEST_SRCS_)
	@mkdir -p $(@D)
	@echo Building gtest-all.cc
	@$(CXX) $(COMMON_FLAGS) $(CPP_FLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o "$@"

build/gtest_main.o: $(GTEST_SRCS_)
	@mkdir -p $(@D)
	@echo Building gtest_main.cc
	@$(CXX) $(COMMON_FLAGS) $(CPP_FLAGS) -c $(GTEST_DIR)/src/gtest_main.cc -o "$@"

build/gtest_main.a: build/gtest-all.o build/gtest_main.o
	@mkdir -p $(@D)
	@echo Creating gtest-all.cc
	@$(AR) r $@ $^ > /dev/null 2>&1