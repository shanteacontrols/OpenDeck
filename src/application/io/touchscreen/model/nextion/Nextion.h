#pragma once

#include <inttypes.h>
#include "io/touchscreen/Touchscreen.h"

class Nextion : public IO::Touchscreen::Model
{
    public:
    Nextion(IO::Touchscreen::Model::HWA& hwa)
        : hwa(hwa)
    {}

    bool init() override;
    void setScreen(uint8_t screenID) override;
    bool update(uint8_t& buttonID, bool& state) override;
    void setButtonState(uint8_t index, bool state) override;

    private:
    IO::Touchscreen::Model::HWA& hwa;
};