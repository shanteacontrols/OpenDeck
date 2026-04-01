if("PROJECT_TARGET_HW_TESTS_SUPPORTED" IN_LIST PROJECT_TARGET_DEFINES)
    add_executable(hw)
    set(CURRENT_FW_RELEASE_DL_DIR /tmp/latest_github_release/${TARGET})

    FetchContent_Declare(
        current_fw_release_bin
        URL https://github.com/shanteacontrols/OpenDeck/releases/latest/download/${TARGET}.bin
        DOWNLOAD_DIR ${CURRENT_FW_RELEASE_DL_DIR}
        DOWNLOAD_NO_EXTRACT ON
        DOWNLOAD_NAME merged.bin
    )

    FetchContent_Declare(
        current_fw_release_hex
        URL https://github.com/shanteacontrols/OpenDeck/releases/latest/download/${TARGET}.hex
        DOWNLOAD_DIR ${CURRENT_FW_RELEASE_DL_DIR}
        DOWNLOAD_NO_EXTRACT ON
        DOWNLOAD_NAME merged.hex
    )

    FetchContent_MakeAvailable(
        current_fw_release_bin
        current_fw_release_hex
    )

    target_sources(hw
        PRIVATE
        test.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/database/database.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/database/custom_init.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/system/system.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/util/cinfo/cinfo.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/protocol/midi/midi.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/io/buttons/buttons.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/io/encoders/encoders.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/io/leds/leds.cpp
        $ENV{ZEPHYR_PROJECT}/src/firmware/application/io/analog/analog.cpp
    )

    target_compile_definitions(hw
        PUBLIC
        SW_VERSION_MAJOR=0
        SW_VERSION_MINOR=0
        SW_VERSION_REVISION=0
    )

    target_link_libraries(hw
        PUBLIC
        common
    )

    add_test(
        NAME hw
        COMMAND $<TARGET_FILE:hw>
        WORKING_DIRECTORY $ENV{ZEPHYR_PROJECT}
    )
endif()