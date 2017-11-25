#include "Board.h"
#include "Config.h"

uint32_t midiIn_timeout, midiOut_timeout;

volatile bool    MIDImessageReceived;
volatile bool    MIDImessageSent;
volatile uint32_t rTime_ms;

ISR(TIMER0_COMPA_vect)
{
    if (MIDImessageReceived)
    {
        setLow(LED_IN_PORT, LED_IN_PIN);
        MIDImessageReceived = false;
        midiIn_timeout = MIDI_INDICATOR_TIMEOUT;
    }

    if (MIDImessageSent)
    {
        setLow(LED_OUT_PORT, LED_OUT_PIN);
        MIDImessageSent = false;
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