#include <gtest/gtest.h>

#include "../../firmware/interface/analog/Analog.h"

Analog analog;

class InterfaceAnalogTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
        
    }
};

TEST_F(InterfaceAnalogTest, DebounceReset)
{
    // analog.debounceReset(0);
}