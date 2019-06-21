#include <gtest/gtest.h>
#include "interface/analog/Analog.h"
#include "interface/digital/output/leds/LEDs.h"
#include "interface/CInfo.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Misc.h"
#include "stubs/database/DB_ReadWrite.h"
#include "board/Board.h"

namespace Board
{
    namespace detail
    {
        uint32_t adcReturnValue;
    }

    namespace interface
    {
        namespace analog
        {
            int16_t readValue(uint8_t analogID)
            {
                return detail::adcReturnValue;
            }

            void continueReadout()
            {

            }

            bool isDataAvailable()
            {
                return true;
            }
        }

        namespace digital
        {
            namespace output
            {
                uint8_t getRGBID(uint8_t ledID)
                {
                    return 0;
                }

                uint8_t getRGBaddress(uint8_t rgbID, Interface::digital::output::LEDs::rgbIndex_t index)
                {
                    return 0;
                }

                bool setLEDfadeSpeed(uint8_t transitionSpeed)
                {
                    return true;
                }
            }
        }
    }
}

namespace MIDIstub
{
    namespace detail
    {
        uint32_t messageCounter = 0;
        MIDI::USBMIDIpacket_t midiPacket[MAX_NUMBER_OF_ANALOG];
    }

    void reset()
    {
        using namespace detail;
        messageCounter = 0;
    }
}

bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
{
    MIDIstub::detail::midiPacket[MIDIstub::detail::messageCounter] = USBMIDIpacket;
    MIDIstub::detail::messageCounter++;
    return true;
}

class PotentiometerTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {
        //init checks - no point in running further tests if these conditions fail
        EXPECT_TRUE(database.init());
        EXPECT_TRUE(database.isSignatureValid());
        midi.handleUSBwrite(midiDataHandler);
        #ifdef DISPLAY_SUPPORTED
        EXPECT_TRUE(display.init(displayController_ssd1306, displayRes_128x64));
        #endif
    }

    virtual void TearDown()
    {
        
    }

    Database database = Database(DatabaseStub::read, DatabaseStub::write, EEPROM_SIZE-3);
    MIDI midi;
    ComponentInfo cInfo;

    #ifdef LEDS_SUPPORTED
    Interface::digital::output::LEDs leds = Interface::digital::output::LEDs(database);
    #endif

    #ifdef DISPLAY_SUPPORTED
    Interface::Display display;
    #endif

    #ifdef LEDS_SUPPORTED
    #ifndef DISPLAY_SUPPORTED
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, leds, cInfo);
    #else
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, leds, display, cInfo);
    #endif
    #else
    #ifdef DISPLAY_SUPPORTED
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, display, cInfo);
    #else
    Interface::analog::Analog analog = Interface::analog::Analog(database, midi, cInfo);
    #endif
    #endif
};

TEST_F(PotentiometerTest, CCtest)
{
    using namespace Interface::analog;

    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with CC MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_7_BIT_VALUE_MAX), true);

        //midi channel
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, i, 1), true);
    }

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    MIDIstub::reset();
    expectedValue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::midiPacket[i].Data3, expectedValue);

    EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

    //reset all values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;

    MIDIstub::reset();
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::midiPacket[i].Data3, MIDI_7_BIT_VALUE_MAX);

    EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);
}

TEST_F(PotentiometerTest, PitchBendTest)
{
    using namespace Interface::analog;

    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, static_cast<int32_t>(Analog::type_t::pitchBend)), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_14_BIT_VALUE_MAX), true);

        //midi channel
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, i, 1), true);
    }

    uint16_t expectedValue;

    Board::detail::adcReturnValue = 1000;

    MIDIstub::reset();
    expectedValue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;
        pitchBendValue.low = MIDIstub::detail::midiPacket[i].Data2;
        pitchBendValue.high = MIDIstub::detail::midiPacket[i].Data3;
        pitchBendValue.mergeTo14bit();
        EXPECT_EQ(pitchBendValue.value, expectedValue);
    }

    EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

    //call update again, verify no values have been sent
    MIDIstub::reset();

    analog.update();
    EXPECT_EQ(MIDIstub::detail::messageCounter, 0);

    //try with max value
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;
        pitchBendValue.low = MIDIstub::detail::midiPacket[i].Data2;
        pitchBendValue.high = MIDIstub::detail::midiPacket[i].Data3;
        pitchBendValue.mergeTo14bit();
        EXPECT_EQ(pitchBendValue.value, MIDI_14_BIT_VALUE_MAX);
    }
}

TEST_F(PotentiometerTest, ScalingAndInversion)
{
    using namespace Interface::analog;

    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with CC MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_14_BIT_VALUE_MAX), true);

        //midi channel
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, i, 1), true);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        uint16_t expectedValue = core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            EXPECT_EQ(MIDIstub::detail::midiPacket[j].Data3, expectedValue);
            //reset debouncing state for easier testing - otherwise Board::detail::adcReturnValue
            //won't be accepted in next analog.update() call
            analog.debounceReset(j);
        }
    }

    //now enable inversion
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable inversion for all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 1), true);
        analog.debounceReset(i);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        uint32_t expectedValue = static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX) - core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            EXPECT_EQ(MIDIstub::detail::midiPacket[j].Data3, expectedValue);
            analog.debounceReset(j);
        }
    }

    //now do the same thing for pitch bend analog type
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, static_cast<int32_t>(Analog::type_t::pitchBend)), true);

        analog.debounceReset(i);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        auto expectedValue = core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            MIDI::encDec_14bit_t pitchBendValue;
            pitchBendValue.low = MIDIstub::detail::midiPacket[j].Data2;
            pitchBendValue.high = MIDIstub::detail::midiPacket[j].Data3;
            pitchBendValue.mergeTo14bit();
            EXPECT_EQ(pitchBendValue.value, expectedValue);
            analog.debounceReset(j);
        }
    }

    //now enable inversion
    for (uint32_t i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 1), true);
        analog.debounceReset(i);
    }

    for (uint32_t i=ADC_MAX_VALUE+1; i-- > 0; )
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue = i;
        uint16_t expectedValue = MIDI_14_BIT_VALUE_MAX - core::misc::mapRange(i, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_14_BIT_VALUE_MAX));

        analog.update();
        EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

        for (int j=0; j<MAX_NUMBER_OF_ANALOG; j++)
        {
            MIDI::encDec_14bit_t pitchBendValue;
            pitchBendValue.low = MIDIstub::detail::midiPacket[j].Data2;
            pitchBendValue.high = MIDIstub::detail::midiPacket[j].Data3;
            pitchBendValue.mergeTo14bit();
            EXPECT_EQ(pitchBendValue.value, expectedValue);
            analog.debounceReset(j);
        }
    }

    //try with scaled upper value
    //still using pitch bend message type

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    const uint32_t scaledUpperValue = 100;

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, scaledUpperValue), true);
    }

    MIDIstub::reset();
    Board::detail::adcReturnValue = ADC_MAX_VALUE;
    auto expectedValue = scaledUpperValue;
    analog.update();
    EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        MIDI::encDec_14bit_t pitchBendValue;
        pitchBendValue.low = MIDIstub::detail::midiPacket[i].Data2;
        pitchBendValue.high = MIDIstub::detail::midiPacket[i].Data3;
        pitchBendValue.mergeTo14bit();
        EXPECT_EQ(pitchBendValue.value, expectedValue);
    }
}

TEST_F(PotentiometerTest, DebouncingSetup)
{
    //verify that the step diff is properly configured
    //don't allow step difference which would result in scenario where total number
    //of generated values would be less than 127
    ASSERT_TRUE((ADC_MAX_VALUE/ANALOG_STEP_MIN_DIFF_7_BIT) >= 127);
}

TEST_F(PotentiometerTest, Debouncing)
{
    using namespace Interface::analog;

    //set known state
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_enable, i, 1), true);

        //disable invert state
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_invert, i, 0), true);

        //configure all analog components as potentiometers with CC MIDI message
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)), true);

        //set all lower limits to 0
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_lowerLimit, i, 0), true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_upperLimit, i, MIDI_14_BIT_VALUE_MAX), true);

        //midi channel
        EXPECT_EQ(database.update(DB_BLOCK_ANALOG, dbSection_analog_midiChannel, i, 1), true);
    }

    uint32_t expectedValue;
    uint32_t newMIDIvalue;

    MIDIstub::reset();
    Board::detail::adcReturnValue = 0;
    expectedValue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
    analog.update();

    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
        EXPECT_EQ(MIDIstub::detail::midiPacket[i].Data3, expectedValue);

    EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

    //try again with the same values
    //no values should be sent
    MIDIstub::reset();
    analog.update();

    EXPECT_EQ(MIDIstub::detail::messageCounter, 0);

    //now test using different raw adc value which results in same MIDI value
    MIDIstub::reset();

    if ((ADC_MAX_VALUE/ANALOG_STEP_MIN_DIFF_7_BIT) > 127)
    {
        //it is possible that internally, ANALOG_STEP_MIN_DIFF_7_BIT is defined in a way
        //which would still result in the same MIDI value being generated
        //analog class shouldn't sent the same MIDI value even in this case
        Board::detail::adcReturnValue += ANALOG_STEP_MIN_DIFF_7_BIT;
        //verify that the newly generated value is indeed the same as the last one
        EXPECT_EQ(expectedValue, core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX)));
        analog.update();
        //no values should be sent since the resulting midi value is the same
        EXPECT_EQ(MIDIstub::detail::messageCounter, 0);

        //now increase the raw value until the newly generated midi value differs from the last one
        uint32_t newMIDIvalue;
        do
        {
            Board::detail::adcReturnValue += 1;
            newMIDIvalue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
        }
        while (newMIDIvalue == expectedValue);

        expectedValue = newMIDIvalue;

        //verify that the values have been sent now
        MIDIstub::reset();
        analog.update();
        EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(MIDIstub::detail::midiPacket[i].Data3, expectedValue);
    }
    else
    {
        Board::detail::adcReturnValue += (ANALOG_STEP_MIN_DIFF_7_BIT-1);
        //verify that the newly generated value is indeed the same as the last one
        EXPECT_EQ(expectedValue, core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX)));
        analog.update();
        //no values should be sent since the resulting midi value is the same
        EXPECT_EQ(MIDIstub::detail::messageCounter, 0);

        Board::detail::adcReturnValue += 1;
        //verify that the newly generated value differs from the last one
        newMIDIvalue = core::misc::mapRange(Board::detail::adcReturnValue, static_cast<uint32_t>(ADC_MIN_VALUE), static_cast<uint32_t>(ADC_MAX_VALUE), static_cast<uint32_t>(0), static_cast<uint32_t>(MIDI_7_BIT_VALUE_MAX));
        EXPECT_FALSE(expectedValue == newMIDIvalue);
        expectedValue = newMIDIvalue;
        analog.update();
        //values should be sent now
        EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);

        for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
            EXPECT_EQ(MIDIstub::detail::midiPacket[i].Data3, expectedValue);
    }

    //verify that the debouncing works in both ways
    //change the direction

    for (int i=0; i<ANALOG_STEP_MIN_DIFF_7_BIT-1; i++)
    {
        MIDIstub::reset();
        Board::detail::adcReturnValue -= 1;
        analog.update();
        EXPECT_EQ(MIDIstub::detail::messageCounter, 0);
    }

    //this time, values should be sent
    MIDIstub::reset();
    Board::detail::adcReturnValue -= 1;
    analog.update();
    EXPECT_EQ(MIDIstub::detail::messageCounter, MAX_NUMBER_OF_ANALOG);
}