ExternalProject_Add(firmware
    SOURCE_DIR      ${CMAKE_CURRENT_LIST_DIR}/firmware
    CMAKE_ARGS      -DTARGET=${TARGET}
    CMAKE_ARGS      -DCMAKE_USE_STUB_MCU=0
    BUILD_ALWAYS    TRUE
    INSTALL_COMMAND ""
)
