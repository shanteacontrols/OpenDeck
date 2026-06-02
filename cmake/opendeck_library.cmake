macro(opendeck_library_named name)
    unset(OPENDECK_LIBRARY_TYPE)
    unset(OPENDECK_LIBRARY_UNPARSED_ARGUMENTS)

    cmake_parse_arguments(OPENDECK_LIBRARY "" "TYPE" "" ${ARGN})

    if(OPENDECK_LIBRARY_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments for ${name}: ${OPENDECK_LIBRARY_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT OPENDECK_LIBRARY_TYPE)
        set(OPENDECK_LIBRARY_TYPE STATIC)
    endif()

    if(OPENDECK_LIBRARY_TYPE STREQUAL INTERFACE)
        add_library(${name} INTERFACE)
        target_link_libraries(${name} INTERFACE zephyr_interface)
    else()
        set(ZEPHYR_CURRENT_LIBRARY ${name})
        add_library(${name} ${OPENDECK_LIBRARY_TYPE} "")
        target_link_libraries(${name} PUBLIC zephyr_interface)
    endif()

    add_dependencies(${name} zephyr_generated_headers)
endmacro()

macro(opendeck_interface_library_named name)
    unset(OPENDECK_LIBRARY_TYPE)
    unset(OPENDECK_LIBRARY_UNPARSED_ARGUMENTS)

    cmake_parse_arguments(OPENDECK_LIBRARY "" "TYPE" "" ${ARGN})

    if(OPENDECK_LIBRARY_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments for ${name}: ${OPENDECK_LIBRARY_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT OPENDECK_LIBRARY_TYPE)
        set(OPENDECK_LIBRARY_TYPE INTERFACE)
    endif()

    if(OPENDECK_LIBRARY_TYPE STREQUAL INTERFACE)
        add_library(${name} INTERFACE)
        target_link_libraries(${name} INTERFACE zephyr_interface)
    else()
        set(ZEPHYR_CURRENT_LIBRARY ${name})
        add_library(${name} ${OPENDECK_LIBRARY_TYPE} "")
        target_link_libraries(${name} PUBLIC zephyr_interface)
    endif()

    add_dependencies(${name} zephyr_generated_headers)
endmacro()
