#pragma once

#include "DataTypes.h"
#include "Variables.h"
#include "constants/Constants.h"

///
/// \brief Common interface for all boards.
/// \ingroup board
/// @{
///

class BoardInterface
{
    public:
    virtual void init() = 0;
    virtual void reboot(rebootType_t type);
    virtual bool getButtonState(uint8_t buttonIndex) = 0;
    virtual bool analogDataAvailable() = 0;
    virtual bool digitalInputDataAvailable() = 0;
    virtual int16_t getAnalogValue(uint8_t analogID) = 0;
    virtual int8_t getEncoderState(uint8_t encoderID) = 0;
};

/// @}