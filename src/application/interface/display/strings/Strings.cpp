#include "Strings.h"
#include <string.h>
#include <stdio.h>
#include "core/src/general/Helpers.h"

namespace
{
    const char opendeck_string[] PROGMEM = "OpenDeck";
    const char arduinoLeonardo_string[] PROGMEM = "Arduino Leo";
    const char arduinoMega_string[] PROGMEM = "Arduino Mega";
    const char arduinoProMicro_string[] PROGMEM = "Arduino PM";
    const char arduinoUno_string[] PROGMEM = "Arduino Uno";
    const char teensy2pp_string[] PROGMEM = "Teensy++ 2.0";
    const char dubfocus_string[] PROGMEM = "DubFocus";
    const char bergamot_string[] PROGMEM = "Bergamot";
    const char unknown_string[] PROGMEM = "Unknown";

    const char eventNoteOff_string[] PROGMEM = "Note Off";
    const char eventNoteOn_string[] PROGMEM = "Note On";
    const char eventCC_string[] PROGMEM = "CC";
    const char eventPC_string[] PROGMEM = "PC";
    const char eventCHAT_string[] PROGMEM = "CH AT";
    const char eventAT_string[] PROGMEM = "AT";
    const char eventPB_string[] PROGMEM = "Pitch Bend";
    const char eventSysExConfig_string[] PROGMEM = "SysEx Conf";
    const char eventRTclock_string[] PROGMEM = "RT Clock";
    const char eventRTstart_string[] PROGMEM = "RT Start";
    const char eventRTcontinue_string[] PROGMEM = "RT Continue";
    const char eventRTstop_string[] PROGMEM = "RT Stop";
    const char eventRTasens_string[] PROGMEM = "RT ASens";
    const char eventRTsysrst_string[] PROGMEM = "RT SysRst";
    const char eventMMCplay_string[] PROGMEM = "MMC Play";
    const char eventMMCstop_string[] PROGMEM = "MMC Stop";
    const char eventMMCpause_string[] PROGMEM = "MMC Pause";
    const char eventMMCrecordOn_string[] PROGMEM = "MMC Record On";
    const char eventMMCrecordOff_string[] PROGMEM = "MMC Record Off";
    const char eventNRPN_string[] PROGMEM = "NRPN";
    const char eventPresetChange_string[] PROGMEM = "Preset";

    const char noteC_string[] PROGMEM = "C";
    const char noteCSharp_string[] PROGMEM = "C#";
    const char noteD_string[] PROGMEM = "D";
    const char noteDSharp_string[] PROGMEM = "D#";
    const char noteE_string[] PROGMEM = "E";
    const char noteF_string[] PROGMEM = "F";
    const char noteFSharp_string[] PROGMEM = "F#";
    const char noteG_string[] PROGMEM = "G";
    const char noteGSharp_string[] PROGMEM = "G#";
    const char noteA_string[] PROGMEM = "A";
    const char noteASharp_string[] PROGMEM = "A#";
    const char noteB_strig[] PROGMEM = "B";

    //match with messageTypeDisplay_t
    STRING_PROGMEM_ARRAY(eventNameArray) = {
        eventNoteOff_string,
        eventNoteOn_string,
        eventCC_string,
        eventPC_string,
        eventCHAT_string,
        eventAT_string,
        eventPB_string,
        eventSysExConfig_string,
        nullptr,    //sys common - Time Code Quarter Frame
        nullptr,    //sys common - Song Position Pointer
        nullptr,    //sys common - Song Select
        nullptr,    //sys common - Tune Request
        eventRTclock_string,
        eventRTstart_string,
        eventRTcontinue_string,
        eventRTstop_string,
        eventRTasens_string,
        eventRTsysrst_string,
        eventMMCplay_string,
        eventMMCstop_string,
        eventMMCpause_string,
        eventMMCrecordOn_string,
        eventMMCrecordOff_string,
        eventNRPN_string,
        eventPresetChange_string
    };

    STRING_PROGMEM_ARRAY(noteNameArray) = {
        noteC_string,
        noteCSharp_string,
        noteD_string,
        noteDSharp_string,
        noteE_string,
        noteF_string,
        noteFSharp_string,
        noteG_string,
        noteGSharp_string,
        noteA_string,
        noteASharp_string,
        noteB_strig
    };

    char tempBuffer[50];
}    // namespace

const char* Strings::board()
{
#ifdef BOARD_OPEN_DECK
    strcpy_P(tempBuffer, opendeck_string);
    return tempBuffer;
#elif defined(BOARD_A_LEO)
    strcpy_P(tempBuffer, arduinoLeonardo_string);
#elif defined(BOARD_A_MEGA)
    strcpy_P(tempBuffer, arduinoMega_string);
#elif defined(BOARD_A_PRO_MICRO)
    strcpy_P(tempBuffer, arduinoProMicro_string);
#elif defined(BOARD_A_UNO)
    strcpy_P(tempBuffer, arduinoUno_string);
#elif defined(BOARD_T_2PP)
    strcpy_P(tempBuffer, teensy2pp_string);
#elif defined(BOARD_DUBFOCUS)
    strcpy_P(tempBuffer, dubfocus_string);
#elif defined(BOARD_BERGAMOT)
    strcpy_P(tempBuffer, bergamot_string);
#else
    strcpy_P(tempBuffer, unknown_string);
#endif

    return tempBuffer;
}

const char* Strings::midiMessage(Interface::Display::event_t event)
{
    strcpy_P(tempBuffer, READ_PROGMEM_ARRAY(eventNameArray[static_cast<uint8_t>(event)]));
    return tempBuffer;
}

const char* Strings::note(MIDI::note_t note)
{
    strcpy_P(tempBuffer, READ_PROGMEM_ARRAY(noteNameArray[static_cast<uint8_t>(note)]));
    return tempBuffer;
}