#include "Encoders.h"
#include "../../interface/midi/MIDI.h"
#include "../../eeprom/Configuration.h"
#include "../../sysex/SysEx.h"
#include "../../BitManipulation.h"
#include "../../interface/settings/EncoderSettings.h"

#define ENCODER_VALUE_LEFT_7FH01H   127
#define ENCODER_VALUE_RIGHT_7FH01H  1

#define ENCODER_VALUE_LEFT_3FH41H   63
#define ENCODER_VALUE_RIGHT_3FH41H  65

Encoders::Encoders()    {

    //def const

}

void Encoders::update()   {

    if (!board.encoderDataAvailable()) return;

    for (int i=0; i<MAX_NUMBER_OF_ENCODERS; i++)    {

        if (!configuration.readParameter(CONF_BLOCK_ENCODER, encoderEnabledSection, i)) continue;

        encoderPosition_t encoderState = board.getEncoderState(i);
        if (encoderState == encStopped) continue;

        if (configuration.readParameter(CONF_BLOCK_ENCODER, encoderInvertedSection, i))   {

            if (encoderState == encMoveLeft)
                encoderState = encMoveRight;

             else encoderState = encMoveLeft;

        }

        uint8_t encoderValue = 0;

        switch((encoderType_t)configuration.readParameter(CONF_BLOCK_ENCODER, encoderEncodingModeSection, i)) {

            case enc7Fh01h:
            if (encoderState == encMoveLeft) encoderValue = ENCODER_VALUE_LEFT_7FH01H;
            else encoderValue = ENCODER_VALUE_RIGHT_7FH01H;
            break;

            case enc3Fh41h:
            if (encoderState == encMoveLeft) encoderValue = ENCODER_VALUE_LEFT_3FH41H;
            else encoderValue = ENCODER_VALUE_RIGHT_3FH41H;
            break;

            default:
            break;

        }

        midi.sendControlChange(configuration.readParameter(CONF_BLOCK_ENCODER, encoderMIDIidSection, i), encoderValue);
        //if (sysEx.configurationEnabled())
            //sysEx.sendComponentID(CONF_BLOCK_ENCODER, i);

    }

}

Encoders encoders;
