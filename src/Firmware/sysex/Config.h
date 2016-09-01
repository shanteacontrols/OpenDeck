#ifndef CONFIG_SYSEX_H_
#define CONFIG_SYSEX_H_

#define MAX_NUMBER_OF_BLOCKS    7
#define MAX_NUMBER_OF_SECTIONS  7
#define MAX_CUSTOM_REQUESTS     5

#define INVALID_VALUE           128

#define CONFIG_TIMEOUT          60000   //1 minute

#define PARAMETERS_PER_MESSAGE  32

//1 - one byte size for parameter index and new value (uint8_t)
//2 - two byte size (uint16_t)

#define PARAM_SIZE              1

//manufacturer ID bytes
#define SYS_EX_M_ID_0           0x00
#define SYS_EX_M_ID_1           0x53
#define SYS_EX_M_ID_2           0x43

#endif