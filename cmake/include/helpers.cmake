# Add a specified module to the build
function(add_module module_name)
    add_subdirectory(${WORKSPACE_ROOT}/modules/${module_name} ${CMAKE_CURRENT_BINARY_DIR}/modules/${module_name})
endfunction()