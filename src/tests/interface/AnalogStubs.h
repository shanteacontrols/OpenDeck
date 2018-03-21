#pragma once

#define LESSDB_SIZE 1021
#define _INTERFACE_DIGITAL_INPUT_BUTTONS_
#define _INTERFACE_DIGITAL_OUTPUT_LEDS_
#define _DATABASE_
#define _BOARD_
#define _SYSEX_
#define MIDI_SYSEX_ARRAY_SIZE 45

#define TESTDEF

#include "../../modules/core/src/Core.h"
#include "../../modules/midi/src/DataTypes.h"
#include "../../modules/midi/src/Constants.h"
#include "../../modules/sysex/src/Config.h"
#include "../../firmware/sysExConf/CustomStrings.h"
#include "../../firmware/board/common/constants/Constants.h"
#include "../../firmware/board/avr/variants/Common.h"
#include "../../firmware/board/avr/variants/opendeck/Hardware.h"
#include "../../firmware/database/blocks/Blocks.h"

extern MIDImessage_t    dinMessage,
                        usbMessage;

typedef struct
{
    int32_t read(uint8_t block, uint8_t section, uint16_t index)
    {
        return 1;
    };
} database_t;

typedef struct
{
    uint16_t scaleADC(uint16_t value, uint16_t maxValue)
    {
        if (maxValue == 1023)
        {
            return value;
        }
        else if (maxValue == 127)
        {
            return value >> 3;
        }
        else
        {
            //use mapRange_uint32 to avoid overflow issues
            return 0;
        }
    };

    bool analogDataAvailable()
    {
        return true;
    };

    int16_t getAnalogValue(uint8_t analogID)
    {
        return 1023;
    };

    void continueADCreadout()
    {

    };
} board_t;

typedef struct
{
    void processButton(uint8_t buttonID, bool state, bool debounce)
    {

    };
} buttons_t;

typedef struct
{
    void sendControlChange(uint8_t inControlNumber, uint8_t inControlValue, uint8_t inChannel)
    {

    };

    void sendNoteOn(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel)
    {

    };

    void sendNoteOff(uint8_t inNoteNumber, uint8_t inVelocity, uint8_t inChannel)
    {

    };
} midi_t;

typedef struct
{
    void sendCustomMessage(uint8_t *responseArray, int8_t *values, uint8_t size)
    {

    };

    bool isConfigurationEnabled()
    {
        return false;
    };
} sysEx_t;

typedef struct
{
    void noteToState(uint8_t receivedNote, uint8_t receivedVelocity, uint8_t receivedChannel, bool local)
    {

    };
} leds_t;

extern database_t database;
extern board_t board;
extern midi_t midi;
extern sysEx_t sysEx;
extern buttons_t buttons;
extern leds_t leds;