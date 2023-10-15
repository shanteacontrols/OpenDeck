list(APPEND BOARD_DEFINES
    BOARD_I2C_CUSTOM_IMPL
)

if (TARGET usb)
    # For AVR, two variants of USB library are required:
    # one for bootloader, and the other one for application.
    # For bootloader, build the library with descriptors in RAM,
    # and for application, place the desciptors in flash.

    get_target_property(CORE_USB_SOURCES usb SOURCES)
    get_target_property(CORE_USB_INCLUDE_DIRS usb INCLUDE_DIRECTORIES)
    get_target_property(CORE_USB_DEFINITIONS usb COMPILE_DEFINITIONS)
    get_target_property(CORE_USB_COMPILE_OPTIONS usb COMPILE_OPTIONS)

    # usb library not needed anymore now
    set_target_properties(usb PROPERTIES EXCLUDE_FROM_ALL TRUE)

    add_library(usb_bootloader)
    add_library(usb_application)

    target_sources(usb_bootloader
        PRIVATE
        ${CORE_USB_SOURCES}
    )

    target_sources(usb_application
        PRIVATE
        ${CORE_USB_SOURCES}
    )

    target_include_directories(usb_bootloader
        PUBLIC
        ${CORE_USB_INCLUDE_DIRS}
    )

    target_include_directories(usb_application
        PUBLIC
        ${CORE_USB_INCLUDE_DIRS}
    )

    target_compile_definitions(usb_bootloader
        PUBLIC
        ${CORE_USB_DEFINITIONS}
        USE_RAM_DESCRIPTORS
    )

    target_compile_definitions(usb_application
        PUBLIC
        ${CORE_USB_DEFINITIONS}
        USE_FLASH_DESCRIPTORS
    )

    target_compile_options(usb_bootloader
        PUBLIC
        ${CORE_USB_COMPILE_OPTIONS}
    )

    target_compile_options(usb_application
        PUBLIC
        ${CORE_USB_COMPILE_OPTIONS}
    )
endif()