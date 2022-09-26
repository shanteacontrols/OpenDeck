DEFINES += \
TEST \
USE_LOGGER \
GLOG_CUSTOM_PREFIX_SUPPORT

TEST_DEFINES := 1

# Filter out arch symbols to avoid pulling MCU-specific headers
DEFINES := $(filter-out CORE_ARCH_%,$(DEFINES))
DEFINES := $(filter-out CORE_VENDOR_%,$(DEFINES))
DEFINES := $(filter-out DATABASE_INIT_DATA%,$(DEFINES))

DEFINES += DATABASE_INIT_DATA=1