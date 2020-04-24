#pragma once

#include <inttypes.h>
#include "io/touchscreen/Touchscreen.h"

class Nextion : public IO::Touchscreen::Model
{
    public:
    typedef struct
    {
        uint16_t xPos;
        uint16_t yPos;
        uint16_t width;
        uint16_t height;
        uint16_t onPage;
        uint16_t offPage;
    } icon_t;

    Nextion(IO::Touchscreen::Model::HWA& hwa)
        : hwa(hwa)
    {}

    bool init() override;
    void setScreen(uint8_t screenID) override;
    bool update(uint8_t& buttonID, bool& state) override;
    void setButtonState(uint8_t index, bool state) override;

    private:
    IO::Touchscreen::Model::HWA& hwa;

    bool getIcon(size_t index, icon_t& icon);
};