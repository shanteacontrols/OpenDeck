#ifndef HARDWARE_VERSION_H_
#define HARDWARE_VERSION_H_

#define HARDWARE_VERSION_MAJOR      1
#define HARDWARE_VERSION_MINOR      0
#define HARDWARE_VERSION_REVISION   1

const struct {

    uint8_t major;
    uint8_t minor;
    uint8_t revision;

} hardwareVersion = {

    HARDWARE_VERSION_MAJOR,
    HARDWARE_VERSION_MINOR,
    HARDWARE_VERSION_REVISION

};

#endif