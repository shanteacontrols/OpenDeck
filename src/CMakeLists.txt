include(ExternalProject)

ExternalProject_Add(firmware
    SOURCE_DIR      ${CMAKE_CURRENT_LIST_DIR}/firmware
    CMAKE_ARGS      -DTARGET=${TARGET}
    BUILD_ALWAYS    TRUE
    INSTALL_COMMAND ""
)
