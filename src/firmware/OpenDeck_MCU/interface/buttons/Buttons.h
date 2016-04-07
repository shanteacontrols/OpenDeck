#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <avr/io.h>
#include "../../hardware/board/Board.h"

class Buttons {

    public:
    Buttons();

    void init();
    void update();
    uint8_t getParameter(uint8_t messageType, uint8_t parameterID);
    bool setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter);

    private:
    //variables
    uint8_t     previousButtonState[MAX_NUMBER_OF_BUTTONS/8+1],
                buttonPressed[MAX_NUMBER_OF_BUTTONS/8+1],
                buttonDebounceCounter[MAX_NUMBER_OF_BUTTONS];

    //data processing
    bool getButtonPressed(uint8_t buttonID);
    void setButtonPressed(uint8_t buttonID, bool state);
    void processMomentaryButton(uint8_t buttonID, bool buttonState);
    void processLatchingButton(uint8_t buttonID, bool buttonState);
    void processProgramChange(uint8_t buttonID, bool buttonState);
    void updateButtonState(uint8_t buttonID, uint8_t buttonState);
    bool getPreviousButtonState(uint8_t buttonID);
    bool buttonDebounced(uint8_t buttonID, bool buttonState);

    //get
    buttonType_t getButtonType(uint8_t buttonID);
    bool getButtonPCenabled(uint8_t buttonID);
    uint8_t getMIDIid(uint8_t buttonID);

    //set
    bool setButtonType(uint8_t buttonID, uint8_t type);
    bool setMIDIid(uint8_t buttonID, uint8_t midiID);
    bool setButtonPCenabled(uint8_t buttonID, uint8_t state);

};

extern Buttons buttons;

#endif
