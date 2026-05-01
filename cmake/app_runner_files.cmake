if(TARGET runners_yaml_props_target)
    function(opendeck_runner_file type path)
        file(RELATIVE_PATH path ${ZEPHYR_BINARY_DIR} ${path})
        set_target_properties(runners_yaml_props_target PROPERTIES "${type}_file" "${path}")
    endfunction()

    opendeck_runner_file(bin ${APPLICATION_BINARY_DIR}/app_validated.bin)
    opendeck_runner_file(hex ${APPLICATION_BINARY_DIR}/app_validated.hex)
endif()
