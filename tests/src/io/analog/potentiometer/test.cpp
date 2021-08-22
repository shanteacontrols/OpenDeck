#include "unity/Framework.h"
#include "io/analog/Analog.h"
#include "io/leds/LEDs.h"
#include "io/common/CInfo.h"
#include "database/Database.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "core/src/general/Helpers.h"
#include "stubs/database/DB_ReadWrite.h"

#ifdef ANALOG_SUPPORTED

namespace
{
    class HWAMIDI : public MIDI::HWA
    {
        public:
        HWAMIDI() = default;

        bool init(MIDI::interface_t interface) override
        {
            return true;
        }

        bool deInit(MIDI::interface_t interface) override
        {
            return true;
        }

        bool dinRead(uint8_t& data) override
        {
            return false;
        }

        bool dinWrite(uint8_t data) override
        {
            return false;
        }

        bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            return false;
        }

        bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override
        {
            midiPacket.push_back(USBMIDIpacket);
            return true;
        }

        std::vector<MIDI::USBMIDIpacket_t> midiPacket;
    } hwaMIDI;

    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs() {}

        void setState(size_t index, IO::LEDs::brightness_t brightness) override
        {
        }

        size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return 0;
        }
    } hwaLEDs;

    class HWAAnalog : public IO::Analog::HWA
    {
        public:
        HWAAnalog() {}

        bool value(size_t index, uint16_t& value) override
        {
            value = adcReturnValue;
            return true;
        }

        uint32_t adcReturnValue;
    } hwaAnalog;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbStorageMock, true);
    MIDI          midi(hwaMIDI);
    ComponentInfo cInfo;

    IO::LEDs leds(hwaLEDs, database);

    class HWAU8X8 : public IO::U8X8::HWAI2C
    {
        public:
        HWAU8X8() {}

        bool init() override
        {
            return true;
        }

        bool deInit() override
        {
            return true;
        }

        bool write(uint8_t address, uint8_t* data, size_t size) override
        {
            return true;
        }
    } hwaU8X8;

    class AnalogFilterStub : public IO::Analog::Filter
    {
        public:
        AnalogFilterStub() {}

        IO::Analog::adcType_t adcType() override
        {
#ifdef ADC_12_BIT
            return IO::Analog::adcType_t::adc12bit;
#else
            return IO::Analog::adcType_t::adc10bit;
#endif
        }

        bool isFiltered(size_t index, IO::Analog::type_t type, uint16_t value, uint16_t& filteredValue) override
        {
            filteredValue = value;
            return true;
        }

        void reset(size_t index) override
        {
        }
    } analogFilter;

    IO::U8X8    u8x8(hwaU8X8);
    IO::Display display(u8x8, database);

#ifdef ADC_12_BIT
#define ADC_RESOLUTION IO::Analog::adcType_t::adc12bit
#else
#define ADC_RESOLUTION IO::Analog::adcType_t::adc10bit
#endif

    IO::Analog analog(hwaAnalog, analogFilter, database, midi, leds, display, cInfo);
}    // namespace

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    TEST_ASSERT(database.factoryReset() == true);

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        analog.debounceReset(i);

    hwaMIDI.midiPacket.clear();
    midi.init(MIDI::interface_t::usb);
}

TEST_CASE(CCtest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_7_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_7_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //feed all the values from minimum to maximum
    //expect the following:
    //first value is 0
    //last value is 127

    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), hwaMIDI.midiPacket.size());

    //all received messages should be control change
    for (int i = 0; i < hwaMIDI.midiPacket.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(static_cast<uint8_t>(MIDI::messageType_t::controlChange), hwaMIDI.midiPacket.at(i).Event << 4);

    uint8_t expectedMIDIvalue = 0;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, hwaMIDI.midiPacket.at(index).Data3);
        }

        expectedMIDIvalue++;
    }

    //now go backward

    hwaMIDI.midiPacket.clear();

    for (int i = 127; i >= 0; i--)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //expect one message less since the last one was 127
    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 127), hwaMIDI.midiPacket.size());

    expectedMIDIvalue = 126;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, hwaMIDI.midiPacket.at(index).Data3);
        }

        expectedMIDIvalue--;
    }

    hwaMIDI.midiPacket.clear();

    //try to feed value larger than 127
    //no response should be received
    hwaAnalog.adcReturnValue = 10000;
    analog.update();

    TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.midiPacket.size());
}

TEST_CASE(PitchBendTest)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with Pitch Bend MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::pitchBend)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    for (int i = 0; i <= 16383; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 16384), hwaMIDI.midiPacket.size());

    //all received messages should be pitch bend
    for (int i = 0; i < hwaMIDI.midiPacket.size(); i++)
        TEST_ASSERT_EQUAL_UINT32(hwaMIDI.midiPacket.at(i).Event << 4, static_cast<uint8_t>(MIDI::messageType_t::pitchBend));

    uint16_t expectedMIDIvalue = 0;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;

            MIDI::Merge14bit pitchBendValue;

            pitchBendValue.merge(hwaMIDI.midiPacket.at(index).Data3, hwaMIDI.midiPacket.at(index).Data2);
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, pitchBendValue.value());
        }

        expectedMIDIvalue++;
    }

    //now go backward
    hwaMIDI.midiPacket.clear();

    for (int i = 16383; i >= 0; i--)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    //one message less
    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 16383), hwaMIDI.midiPacket.size());

    expectedMIDIvalue = 16382;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;

            MIDI::Merge14bit pitchBendValue;

            pitchBendValue.merge(hwaMIDI.midiPacket.at(index).Data3, hwaMIDI.midiPacket.at(index).Data2);
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, pitchBendValue.value());
        }

        expectedMIDIvalue--;
    }
}

TEST_CASE(Inversion)
{
    using namespace IO;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to MIDI_14_BIT_VALUE_MAX
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, MIDI_14_BIT_VALUE_MAX) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    //enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);

    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), hwaMIDI.midiPacket.size());

    //first value should be 127
    //last value should be 0

    uint16_t expectedMIDIvalue = 127;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, hwaMIDI.midiPacket.at(index).Data3);
        }

        expectedMIDIvalue--;
    }

    hwaMIDI.midiPacket.clear();

    //funky setup: set lower limit to 127, upper to 0 while inversion is enabled
    //result should be the same as when default setup is used (no inversion / 0 as lower limit, 127 as upper limit)

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 127) == true);
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, 0) == true);
        analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), hwaMIDI.midiPacket.size());

    expectedMIDIvalue = 0;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, hwaMIDI.midiPacket.at(index).Data3);
        }

        expectedMIDIvalue++;
    }

    hwaMIDI.midiPacket.clear();

    //now disable inversion
    hwaMIDI.midiPacket.clear();

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);
        analog.debounceReset(i);
    }

    //feed all the values again
    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT_EQUAL_UINT32((MAX_NUMBER_OF_ANALOG * 128), hwaMIDI.midiPacket.size());

    expectedMIDIvalue = 127;

    for (int i = 0; i < hwaMIDI.midiPacket.size(); i += MAX_NUMBER_OF_ANALOG)
    {
        for (int j = 0; j < MAX_NUMBER_OF_ANALOG; j++)
        {
            size_t index = i + j;
            TEST_ASSERT_EQUAL_UINT32(expectedMIDIvalue, hwaMIDI.midiPacket.at(index).Data3);
        }

        expectedMIDIvalue--;
    }
}

TEST_CASE(Scaling)
{
    using namespace IO;

    const uint32_t scaledLower = 11;
    const uint32_t scaledUpper = 100;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        //enable all analog components
        TEST_ASSERT(database.update(Database::Section::analog_t::enable, i, 1) == true);

        //disable invert state
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 0) == true);

        //configure all analog components as potentiometers with CC MIDI message
        TEST_ASSERT(database.update(Database::Section::analog_t::type, i, static_cast<int32_t>(Analog::type_t::potentiometerControlChange)) == true);

        //set all lower limits to 0
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, 0) == true);

        //set all upper limits to 100
        TEST_ASSERT(database.update(Database::Section::analog_t::upperLimit, i, scaledUpper) == true);

        //midi channel
        TEST_ASSERT(database.update(Database::Section::analog_t::midiChannel, i, 1) == true);
    }

    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT((MAX_NUMBER_OF_ANALOG * 128) > hwaMIDI.midiPacket.size());

    //first value should be 0
    //last value should match the configured scaled value (scaledUpper)
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT_EQUAL_UINT32(0, hwaMIDI.midiPacket.at(i).Data3);
        TEST_ASSERT_EQUAL_UINT32(scaledUpper, hwaMIDI.midiPacket.at(hwaMIDI.midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3);
    }

    //now scale minimum value as well
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
        TEST_ASSERT(database.update(Database::Section::analog_t::lowerLimit, i, scaledLower) == true);

    hwaMIDI.midiPacket.clear();

    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    TEST_ASSERT((MAX_NUMBER_OF_ANALOG * 128) > hwaMIDI.midiPacket.size());

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(hwaMIDI.midiPacket.at(i).Data3 >= scaledLower);
        TEST_ASSERT(hwaMIDI.midiPacket.at(hwaMIDI.midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3 <= scaledUpper);
    }

    //now enable inversion
    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(database.update(Database::Section::analog_t::invert, i, 1) == true);
        analog.debounceReset(i);
    }

    hwaMIDI.midiPacket.clear();

    for (int i = 0; i <= 127; i++)
    {
        hwaAnalog.adcReturnValue = i;
        analog.update();
    }

    for (int i = 0; i < MAX_NUMBER_OF_ANALOG; i++)
    {
        TEST_ASSERT(hwaMIDI.midiPacket.at(i).Data3 >= scaledUpper);
        TEST_ASSERT(hwaMIDI.midiPacket.at(hwaMIDI.midiPacket.size() - MAX_NUMBER_OF_ANALOG + i).Data3 <= scaledLower);
    }
}

#endif