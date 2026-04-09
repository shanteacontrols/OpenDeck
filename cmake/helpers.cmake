include_guard(GLOBAL)

function(read_config_value config_file variable_name output_var)
    if(NOT EXISTS "${config_file}")
        unset(${output_var} PARENT_SCOPE)
        return()
    endif()

    file(STRINGS "${config_file}" config_lines REGEX "^${variable_name}=")

    if(NOT config_lines)
        unset(${output_var} PARENT_SCOPE)
        return()
    endif()

    list(GET config_lines 0 config_entry)
    string(REGEX REPLACE "^[^=]*=" "" config_value "${config_entry}")
    string(REGEX REPLACE "^\"([^\"]*)\"$" "\\1" config_value "${config_value}")
    set(${output_var} "${config_value}" PARENT_SCOPE)
endfunction()
