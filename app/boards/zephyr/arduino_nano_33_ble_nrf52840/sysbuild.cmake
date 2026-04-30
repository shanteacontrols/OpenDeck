set(opendeck_arduino_bossac /usr/bin/arduino-bossac)

if(EXISTS ${opendeck_arduino_bossac})
    message(STATUS "Using Arduino Nano 33 BLE bossac: ${opendeck_arduino_bossac}")
    set(BOSSAC "${opendeck_arduino_bossac}" CACHE FILEPATH "Arduino nRF BOSSA executable" FORCE)
else()
    message(WARNING
        "Arduino Nano 33 BLE flashing requires Arduino's nRF BOSSA fork. "
        "Expected it at ${opendeck_arduino_bossac}.")
    set(BOSSAC "BOSSAC-NOTFOUND" CACHE FILEPATH "Arduino nRF BOSSA executable" FORCE)
endif()
