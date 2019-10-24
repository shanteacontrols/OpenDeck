#include "core/src/general/Timing.h"

namespace core
{
    namespace timing
    {
        namespace detail
        {
            volatile uint32_t rTime_ms;
        }
    }    // namespace timing

    namespace i2c
    {
        enum class transferType_t : uint8_t
        {
            write,
            read
        };

        void enable()
        {
        }

        void disable(bool force)
        {
        }

        bool startComm(uint8_t address, transferType_t type)
        {
            return true;
        }

        void stopComm()
        {
        }

        bool write(uint8_t data)
        {
            return true;
        }

        void read(uint8_t& data)
        {
        }
    }    // namespace i2c
}    // namespace core