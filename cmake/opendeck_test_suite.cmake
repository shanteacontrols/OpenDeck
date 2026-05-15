include_guard(GLOBAL)

macro(opendeck_test_bootstrap)
    set(options APPLY_TARGET)
    set(multiValueArgs COMPILE_DEFINITIONS)
    cmake_parse_arguments(OPENDECK_TEST "${options}" "" "${multiValueArgs}" ${ARGN})

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig")
        set(KCONFIG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Kconfig)
    endif()

    list(APPEND CONF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/prj.conf")

    if(OPENDECK_TEST_APPLY_TARGET)
        list(APPEND DTS_ROOT "$ENV{ZEPHYR_PROJECT}/app")
        include($ENV{ZEPHYR_PROJECT}/cmake/opendeck_test_target.cmake)
        opendeck_test_apply_target()
    endif()

    include($ENV{ZEPHYR_WS}/zenv/cmake/test.cmake)

    project(test)

    zephyr_include_directories($ENV{ZEPHYR_PROJECT}/app)

    add_compile_definitions(OPENDECK_TEST)

    if(OPENDECK_TEST_COMPILE_DEFINITIONS)
        add_compile_definitions(${OPENDECK_TEST_COMPILE_DEFINITIONS})
    endif()
endmacro()

function(opendeck_test_add_module target source_path)
    add_subdirectory("$ENV{ZEPHYR_PROJECT}/${source_path}" "${CMAKE_CURRENT_BINARY_DIR}/${target}")
    add_dependencies(app "${target}")

    if(TARGET "${target}")
        target_link_libraries(app PUBLIC "${target}")
    endif()
endfunction()

function(opendeck_test_add_modules)
    set(args ${ARGN})
    list(LENGTH args arg_count)

    if(arg_count EQUAL 0)
        return()
    endif()

    math(EXPR remainder "${arg_count} % 2")

    if(NOT remainder EQUAL 0)
        message(FATAL_ERROR "opendeck_test_add_modules expects pairs of <target> <source_path>.")
    endif()

    while(args)
        list(POP_FRONT args target source_path)
        opendeck_test_add_module("${target}" "${source_path}")
    endwhile()
endfunction()
