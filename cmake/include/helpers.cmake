function(check_required_executables)
    foreach(executable ${ARGV})
        find_program(${executable}_result ${executable})
        if(${executable}_result)
            message(STATUS "Required executable ${executable} found at: ${${executable}_result}")
        else()
            message(FATAL_ERROR "Required executable ${executable} not found!")
        endif()
    endforeach()
endfunction()

# Add a specified module to the build
function(add_module module_name)
    add_subdirectory(${PROJECT_ROOT}/modules/${module_name} ${CMAKE_CURRENT_BINARY_DIR}/modules/${module_name})
endfunction()

# Link and compile specified library with options for the mcu target
macro(add_mcu_library library_name)
    target_include_directories(${library_name}
        PRIVATE
        "$<TARGET_PROPERTY:mcu,INCLUDE_DIRECTORIES>"
    )

    target_compile_options(${library_name}
        PUBLIC
        "$<TARGET_PROPERTY:mcu,COMPILE_OPTIONS>"
    )

    target_link_options(${library_name}
        PUBLIC
        "$<TARGET_PROPERTY:mcu,LINK_OPTIONS>"
    )
endmacro()