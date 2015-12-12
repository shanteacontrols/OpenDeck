#ifndef LEDS_H_
#define LEDS_H_

#include "hardware/Board.h"
#include "..\BitManipulation.h"
#include "eeprom/EEPROMsettings.h"

//blink time
#define BLINK_TIME_MIN                  0x02
#define BLINK_TIME_MAX                  0x0F

//LED switch time on start-up
#define START_UP_SWITCH_TIME_MIN        0x01
#define START_UP_SWITCH_TIME_MAX        0x78

#define FADE_TIME_MIN                   0x00
#define FADE_TIME_MAX                   0x0A

//LED blink/constant state determination
#define LED_VELOCITY_C_OFF              0x00
#define LED_VELOCITY_B_OFF              0x3F


class LEDs {

    public:
    LEDs();
    void init();
    void getConfiguration();
    uint8_t getParameter(uint8_t messageType, uint8_t parameter);
    bool setParameter(uint8_t messageType, uint8_t parameter, uint8_t newParameter);
    void noteToLEDstate(uint8_t receivedNote, uint8_t receivedVelocity);
    void allLEDsOn();
    void allLEDsOff();
    void allLEDsBlink();
    void update();

    inline bool rgbLEDenabled(uint8_t ledID)    {

        uint8_t arrayIndex = ledID/8;
        uint8_t ledIndex = ledID - 8*arrayIndex;
        uint8_t rgbEnabledArray = eeprom_read_byte((uint8_t*)EEPROM_LEDS_RGB_ENABLED_START+arrayIndex);

        return (bitRead(rgbEnabledArray, ledIndex));

    }

    private:
    //data processing
    void handleLED(bool newLEDstate, bool blinkMode, uint8_t ledNumber);
    bool checkLEDsOn();
    bool checkLEDsOff();
    bool checkLEDstartUpNumber(uint8_t ledID);
    bool checkLEDactivationNote(uint8_t ledID);
    void checkBlinkLEDs();

    //get
    uint8_t getLEDHwParameter(uint8_t parameter);
    uint8_t getLEDActivationNote(uint8_t);
    uint8_t getLEDid(uint8_t midiID);

    //set
    bool setLEDHwParameter(uint8_t parameter, uint8_t newParameter);
    bool setLEDActivationNote(uint8_t, uint8_t);
    bool setLEDstartNumber(uint8_t, uint8_t);

    //animation
    void oneByOneLED(bool, bool, bool);
    void startUpRoutine();

};

extern LEDs leds;

#endif