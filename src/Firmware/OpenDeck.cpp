/*
    OpenDeck MIDI platform firmware
    https://shanteacontrols.wordpress.com
    Copyright (c) 2015, 2016 Igor Petrovic
    Version: 1.0

    Released under GPLv3.
*/

#include "init/Init.h"

#define FIRMWARE_VERSION_STRING 0x56
#define HARDWARE_VERSION_STRING 0x42
#define REBOOT_STRING           0x7F
#define FACTORY_RESET_STRING    0x44

bool onCustom(uint8_t value) {

    switch(value)   {

        case FIRMWARE_VERSION_STRING:
        sysEx.addToResponse(getSWversion(swVersion_major));
        sysEx.addToResponse(getSWversion(swVersion_minor));
        sysEx.addToResponse(getSWversion(swVersion_revision));
        return true;

        case HARDWARE_VERSION_STRING:
        sysEx.addToResponse(hardwareVersion.major);
        sysEx.addToResponse(hardwareVersion.minor);
        sysEx.addToResponse(hardwareVersion.revision);
        return true;

        case REBOOT_STRING:
        leds.setFadeTime(1);
        leds.allOff();
        wait(1500);
        reboot();
        return true; //pointless, but we're making compiler happy

        case FACTORY_RESET_STRING:
        leds.setFadeTime(1);
        leds.allOff();
        wait(1500);
        configuration.factoryReset(factoryReset_partial);
        reboot();
        return true;

    }   return false;

}

sysExParameter_t onGet(uint8_t block, uint8_t section, uint16_t index) {

    switch(block)   {

        case CONF_BLOCK_LED:
        if (section == ledStateSection)    {

            return leds.getState(index);

        } else {

            return configuration.readParameter(block, section, index);

        }
        break;

        default:
        return configuration.readParameter(block, section, index);

    }

}

bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)   {

    bool returnValue = true;
    //don't write led states to eeprom
    if (block != CONF_BLOCK_LED)
        returnValue = configuration.writeParameter(block, section, index, newValue);

    if (returnValue)    {

        //special checks
        switch(block)   {

            case CONF_BLOCK_ANALOG:
            if (section == analogTypeSection)
                analog.debounceReset(index);
            break;

            case CONF_BLOCK_MIDI:
            if (section == midiFeatureSection)
                newValue ? hwMIDI.enableRunningStatus() : hwMIDI.disableRunningStatus();
            break;

            case CONF_BLOCK_LED:
            if (section == ledStateSection)  {

                switch ((ledStatesHardwareParameter)newValue)   {

                    case ledStateOff:
                    leds.setState(index, colorOff, false);
                    break;

                    case ledStateConstantWhite:
                    leds.setState(index, colorWhite, false);
                    break;

                    case ledStateConstantCyan:
                    leds.setState(index, colorCyan, false);
                    break;

                    case ledStateConstantMagenta:
                    leds.setState(index, colorMagenta, false);
                    break;

                    case ledStateConstantRed:
                    leds.setState(index, colorRed, false);
                    break;

                    case ledStateConstantBlue:
                    leds.setState(index, colorBlue, false);
                    break;

                    case ledStateConstantYellow:
                    leds.setState(index, colorYellow, false);
                    break;

                    case ledStateConstantGreen:
                    leds.setState(index, colorGreen, false);
                    break;

                    case ledStateBlinkWhite:
                    leds.setState(index, colorWhite, true);
                    break;

                    case ledStateBlinkCyan:
                    leds.setState(index, colorCyan, true);
                    break;

                    case ledStateBlinkMagenta:
                    leds.setState(index, colorMagenta, true);
                    break;

                    case ledStateBlinkRed:
                    leds.setState(index, colorRed, true);
                    break;

                    case ledStateBlinkBlue:
                    leds.setState(index, colorBlue, true);
                    break;

                    case ledStateBlinkYellow:
                    leds.setState(index, colorYellow, true);
                    break;

                    case ledStateBlinkGreen:
                    leds.setState(index, colorGreen, true);
                    break;

                    default:
                    return false;
                    break;

                }

            } else  {

                if (section == ledHardwareParameterSection) {

                    switch(index)    {

                        case ledHwParameterBlinkTime:
                        if ((newValue < BLINK_TIME_MIN) || (newValue > BLINK_TIME_MAX))
                        return false;
                        leds.setBlinkTime(newValue);
                        break;

                        case ledHwParameterFadeTime:
                        if ((newValue < FADE_TIME_MIN) || (newValue > FADE_TIME_MAX))
                        return false;
                        leds.setFadeTime(newValue);
                        break;

                        case ledHwParameterStartUpSwitchTime:
                        if ((newValue < START_UP_SWITCH_TIME_MIN) || (newValue > START_UP_SWITCH_TIME_MAX))
                        return false;
                        break;

                        case ledHwParameterStartUpRoutine:
                        if (newValue > NUMBER_OF_START_UP_ANIMATIONS)
                        return false;
                        break;

                    }

                }

                configuration.writeParameter(block, section, index, newValue);

            }
            break;

            default:
            break;

        }

    }

    return returnValue;

}

int main()  {

    globalInit();

    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);

    sysEx.addCustomRequest(FIRMWARE_VERSION_STRING);
    sysEx.addCustomRequest(HARDWARE_VERSION_STRING);
    sysEx.addCustomRequest(REBOOT_STRING);
    sysEx.addCustomRequest(FACTORY_RESET_STRING);

    while(1)    {

        midi.checkInput();
        buttons.update();
        analog.update();
        encoders.update();

    }

}