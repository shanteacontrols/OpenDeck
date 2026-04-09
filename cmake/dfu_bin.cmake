if(NOT DEFINED INPUT_FILE)
    message(FATAL_ERROR "dfu_bin.cmake requires INPUT_FILE")
endif()

if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "dfu_bin.cmake requires OUTPUT_FILE")
endif()

if(NOT DEFINED TARGET_UID)
    message(FATAL_ERROR "dfu_bin.cmake requires TARGET_UID")
endif()

if(NOT DEFINED FORMAT_VERSION)
    message(FATAL_ERROR "dfu_bin.cmake requires FORMAT_VERSION")
endif()

if(NOT DEFINED BEGIN_MAGIC)
    message(FATAL_ERROR "dfu_bin.cmake requires BEGIN_MAGIC")
endif()

if(NOT DEFINED END_MAGIC)
    message(FATAL_ERROR "dfu_bin.cmake requires END_MAGIC")
endif()

file(SIZE "${INPUT_FILE}" PAYLOAD_SIZE)
get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

function(opendeck_append_le32 OUTPUT VALUE)
    if("${VALUE}" MATCHES "^0x" OR "${VALUE}" MATCHES "^0X")
        math(EXPR _value "${VALUE}")
    else()
        set(_value "${VALUE}")
    endif()

    math(EXPR _byte0 "${_value} & 0xFF")
    math(EXPR _byte1 "(${_value} >> 8) & 0xFF")
    math(EXPR _byte2 "(${_value} >> 16) & 0xFF")
    math(EXPR _byte3 "(${_value} >> 24) & 0xFF")

    execute_process(
        COMMAND printf "%02x%02x%02x%02x" ${_byte0} ${_byte1} ${_byte2} ${_byte3}
        OUTPUT_VARIABLE _packed
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )

    set(${OUTPUT} "${${OUTPUT}}${_packed}" PARENT_SCOPE)
endfunction()

set(DFU_HEX_HEADER "")
opendeck_append_le32(DFU_HEX_HEADER "${BEGIN_MAGIC}")
opendeck_append_le32(DFU_HEX_HEADER "${FORMAT_VERSION}")
opendeck_append_le32(DFU_HEX_HEADER "${TARGET_UID}")
opendeck_append_le32(DFU_HEX_HEADER "${PAYLOAD_SIZE}")

set(DFU_HEX_FOOTER "")
opendeck_append_le32(DFU_HEX_FOOTER "${END_MAGIC}")

set(HEADER_FILE "${OUTPUT_FILE}.header")
set(FOOTER_FILE "${OUTPUT_FILE}.footer")
set(HEADER_HEX_FILE "${OUTPUT_FILE}.header.hex")
set(FOOTER_HEX_FILE "${OUTPUT_FILE}.footer.hex")

file(WRITE "${HEADER_HEX_FILE}" "${DFU_HEX_HEADER}")
file(WRITE "${FOOTER_HEX_FILE}" "${DFU_HEX_FOOTER}")

execute_process(
    COMMAND xxd -r -p "${HEADER_HEX_FILE}" "${HEADER_FILE}"
    COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
    COMMAND xxd -r -p "${FOOTER_HEX_FILE}" "${FOOTER_FILE}"
    COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
    COMMAND
    sh
    -c
    "cat \"$1\" \"$2\" \"$3\" > \"$4\""
    sh
    "${HEADER_FILE}"
    "${INPUT_FILE}"
    "${FOOTER_FILE}"
    "${OUTPUT_FILE}"
    COMMAND_ERROR_IS_FATAL ANY
)

file(REMOVE "${HEADER_FILE}" "${FOOTER_FILE}")
file(REMOVE "${HEADER_HEX_FILE}" "${FOOTER_HEX_FILE}")
