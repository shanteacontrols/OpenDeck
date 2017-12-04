#include "Board.h"
#include "Config.h"

volatile uint32_t rTime_ms;
uint8_t midiIn_timeout, midiOut_timeout;

bool MIDIreceived, MIDIsent;

ISR(TIMER0_COMPA_vect)
{
    if (MIDIreceived)
    {
        setLow(LED_IN_PORT, LED_IN_PIN);
        MIDIreceived = false;
        midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (MIDIsent)
    {
        setLow(LED_OUT_PORT, LED_OUT_PIN);
        MIDIsent = false;
        midiOut_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (midiIn_timeout)
    {
        midiIn_timeout--;
    }
    else
    {
        setHigh(LED_IN_PORT, LED_IN_PIN);
    }

    if (midiOut_timeout)
    {
        midiOut_timeout--;
    }
    else
    {
        setHigh(LED_OUT_PORT, LED_OUT_PIN);
    }

    rTime_ms++;
}