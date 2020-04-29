#pragma once

#include <inttypes.h>
#include "io/touchscreen/Touchscreen.h"
#include "core/src/general/RingBuffer.h"

class Nextion : public IO::Touchscreen::Model
{
    public:
    Nextion(IO::Touchscreen::Model::HWA& hwa)
        : hwa(hwa)
    {}

    bool init() override;
    bool setScreen(size_t screenID) override;
    bool update(size_t& buttonID, bool& state) override;
    void setIconState(IO::Touchscreen::icon_t& icon, bool state) override;

    private:
    IO::Touchscreen::Model::HWA&          hwa;
    static const size_t                   bufferSize = 100;
    char                                  commandBuffer[bufferSize];
    core::RingBuffer<uint8_t, bufferSize> rxBuffer;
    size_t                                endCounter = 0;

    bool getIcon(size_t index, IO::Touchscreen::icon_t& icon);
    bool writeCommand(const char* line, ...);
    bool endCommand();
};