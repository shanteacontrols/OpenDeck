#pragma once

#include "DataTypes.h"
#include "Import.h"

class BoardCommon
{
    public:
    virtual void reboot(rebootType_t type);
    virtual bool buttonDataAvailable() = 0;
    virtual bool getButtonState(uint8_t buttonIndex) = 0;
    virtual bool analogDataAvailable() = 0;
    virtual uint16_t getAnalogValue(uint8_t analogID) = 0;
    virtual bool encoderDataAvailable() = 0;
    virtual int8_t getEncoderState(uint8_t encoderID) = 0;
};
