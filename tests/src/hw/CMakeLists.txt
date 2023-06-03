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
        ${PROJECT_ROOT}/src/firmware/application/database/Database.cpp
        ${PROJECT_ROOT}/src/firmware/application/database/CustomInit.cpp
        ${PROJECT_ROOT}/src/firmware/application/system/System.cpp
        ${PROJECT_ROOT}/src/firmware/application/util/cinfo/CInfo.cpp
        ${PROJECT_ROOT}/src/firmware/application/protocol/midi/MIDI.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/buttons/Buttons.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/encoders/Encoders.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/leds/LEDs.cpp
        ${PROJECT_ROOT}/src/firmware/application/io/analog/Analog.cpp
        ${PROJECT_ROOT}/modules/sysex/src/SysExConf.cpp
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
        WORKING_DIRECTORY ${PROJECT_ROOT}
    )
endif()