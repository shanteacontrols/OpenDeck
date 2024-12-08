set(BOARD_DEFINES "")
set(BOARD_BOOTLOADER_LIBRARIES "")
set(BOARD_APPLICATION_LIBRARIES "")

include(${PROJECT_ROOT}/src/generated/target/${TARGET}/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/CMakeLists.txt OPTIONAL)
include(${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/${CORE_MCU_VENDOR}/CMakeLists.txt OPTIONAL)
include(${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/${CORE_MCU_VENDOR}/variants/${CORE_MCU_FAMILY}/CMakeLists.txt OPTIONAL)

file(GLOB_RECURSE BOARD_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/${CORE_MCU_VENDOR}/common/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/${CORE_MCU_VENDOR}/variants/${CORE_MCU_FAMILY}/common/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/${CORE_MCU_VENDOR}/variants/${CORE_MCU_FAMILY}/${CORE_MCU_MODEL}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/${CORE_MCU_VENDOR}/variants/common/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/arch/${CORE_MCU_ARCH}/common/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/common/*.cpp
)

target_compile_definitions(mcu
    PUBLIC
    ${PROJECT_MCU_DEFINES}
)

add_library(board_application OBJECT)
list(APPEND BOARD_APPLICATION_LIBRARIES board_application)

target_include_directories(board_application
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${PROJECT_ROOT}/src/generated/target/${TARGET}
    ${WORKSPACE_ROOT}/modules/libmidi/include
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_compile_definitions(board_application
    PUBLIC
    ${PROJECT_TARGET_DEFINES}
    ${PROJECT_USB_DEFINES}
    ${BOARD_DEFINES}
    FW_APP
)

target_sources(board_application
    PRIVATE
    ${BOARD_SOURCES}
)

target_link_libraries(board_application
    PUBLIC
    mcu
)

list(APPEND BOARD_APPLICATION_LIBRARIES mcu)

if (TARGET usb)
    target_compile_definitions(usb
        PUBLIC
        ${PROJECT_USB_DEFINES}
    )
endif()

if (TARGET usb_application)
    target_compile_definitions(usb_application
        PUBLIC
        ${PROJECT_USB_DEFINES}
    )

    target_link_libraries(board_application
        PUBLIC
        usb_application
    )

    list(APPEND BOARD_APPLICATION_LIBRARIES usb_application)
elseif (TARGET usb)
    target_link_libraries(board_application
        PUBLIC
        usb
    )

    list(APPEND BOARD_APPLICATION_LIBRARIES usb)
endif()

if (TARGET libemueeprom)
    target_link_libraries(board_application
        PUBLIC
        libemueeprom
    )

    list(APPEND BOARD_APPLICATION_LIBRARIES libemueeprom)
endif()

target_link_options(board_application
    PUBLIC
    -Wl,--defsym=CORE_MCU_FLASH_START_ADDR_USER=${PROJECT_MCU_FLASH_ADDR_APP_START}
    -Wl,--defsym=CORE_MCU_FLASH_SIZE_USER=${PROJECT_MCU_FLASH_APP_SIZE}
    -Wl,-Map=application.map,--cref
)

if("PROJECT_MCU_SUPPORT_BOOTLOADER" IN_LIST PROJECT_MCU_DEFINES)
    add_library(board_bootloader OBJECT)
    list(APPEND BOARD_BOOTLOADER_LIBRARIES board_bootloader)

    target_include_directories(board_bootloader
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${PROJECT_ROOT}/src/generated/target/${TARGET}
        ${WORKSPACE_ROOT}/modules/libmidi/include
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
    )

    target_compile_definitions(board_bootloader
        PUBLIC
        ${PROJECT_TARGET_DEFINES}
        ${PROJECT_USB_DEFINES}
        ${BOARD_DEFINES}
        FW_BOOT
    )

    target_sources(board_bootloader
        PRIVATE
        ${BOARD_SOURCES}
    )

    if (TARGET usb_bootloader)
        target_compile_definitions(usb_bootloader
            PUBLIC
            ${PROJECT_USB_DEFINES}
        )

        target_link_libraries(board_bootloader
            PUBLIC usb_bootloader
        )

        list(APPEND BOARD_BOOTLOADER_LIBRARIES usb_bootloader)
    elseif (TARGET usb)
        target_link_libraries(board_bootloader
            PUBLIC usb
        )

        list(APPEND BOARD_BOOTLOADER_LIBRARIES usb)
    endif()

    target_link_libraries(board_bootloader
        PUBLIC
        mcu
    )

    list(APPEND BOARD_BOOTLOADER_LIBRARIES mcu)

    if (TARGET libemueeprom)
        target_link_libraries(board_bootloader
            PUBLIC libemueeprom
        )

        list(APPEND BOARD_BOOTLOADER_LIBRARIES libemueeprom)
    endif()

    target_link_options(board_bootloader
        PUBLIC
        -Wl,--defsym=CORE_MCU_FLASH_START_ADDR_USER=${PROJECT_MCU_FLASH_ADDR_BOOT_START}
        -Wl,--defsym=CORE_MCU_FLASH_SIZE_USER=${PROJECT_MCU_FLASH_BOOT_SIZE}
        -Wl,-Map=bootloader.map,--cref
    )
endif()