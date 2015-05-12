/*
 * Callbacks.cpp
 *
 * Created: 8.5.2015. 14:46:41
 *  Author: Igor
 */ 

#include "OpenDeck.h"

void OpenDeck::setHandleNoteSend(void (*fptr)(uint8_t note, bool state, uint8_t channel))   {

    sendNoteCallback = fptr;

}

void OpenDeck::setHandleProgramChangeSend(void (*fptr)(uint8_t channel, uint8_t program))   {

    sendProgramChangeCallback = fptr;

}

void OpenDeck::setHandleControlChangeSend(void (*fptr)(uint8_t ccNumber, uint8_t ccValue, uint8_t channel))    {

    sendControlChangeCallback = fptr;

}

void OpenDeck::setHandlePitchBendSend(void (*fptr)(uint16_t pitchBendValue, uint8_t channel))   {

    sendPitchBendCallback = fptr;

}

void OpenDeck::setHandleSysExSend(void (*fptr)(uint8_t *sysExArray, uint8_t size))  {

    sendSysExCallback = fptr;

}