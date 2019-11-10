#include <gtest/gtest.h>
#include "interface/digital/input/buttons/Buttons.h"
#include "interface/digital/output/leds/LEDs.h"
#include "interface/CInfo.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

namespace testing
{
    uint32_t              messageCounter = 0;
    bool                  buttonState[MAX_NUMBER_OF_BUTTONS] = {};
    MIDI::USBMIDIpacket_t midiPacket[MAX_NUMBER_OF_BUTTONS];

    void setButtonState(uint8_t buttonIndex, bool state)
    {
        buttonState[buttonIndex] = state;
    }

    void resetReceived()
    {
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            midiPacket[i].Event = 0;
            midiPacket[i].Data1 = 0;
            midiPacket[i].Data2 = 0;
            midiPacket[i].Data3 = 0;
        }

        messageCounter = 0;
    }
}    // namespace testing

bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
{
    testing::midiPacket[testing::messageCounter].Event = USBMIDIpacket.Event;
    testing::midiPacket[testing::messageCounter].Data1 = USBMIDIpacket.Data1;
    testing::midiPacket[testing::messageCounter].Data2 = USBMIDIpacket.Data2;
    testing::midiPacket[testing::messageCounter].Data3 = USBMIDIpacket.Data3;

    testing::messageCounter++;

    return true;
}

class ButtonsTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {
        //init checks - no point in running further tests if these conditions fail
        EXPECT_TRUE(database.init());
        //always start from known state
        database.factoryReset(LESSDB::factoryResetType_t::full);
        EXPECT_TRUE(database.isSignatureValid());
        midi.handleUSBwrite(midiDataHandler);
        midi.setChannelSendZeroStart(true);
#ifdef DISPLAY_SUPPORTED
        EXPECT_TRUE(display.init(displayController_ssd1306, displayRes_128x64));
#endif
    }

    virtual void TearDown()
    {
    }

    void stateChangeRegister(bool state)
    {
        uint8_t debouncingState = BUTTON_DEBOUNCE_COMPARE;
        testing::messageCounter = 0;

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            testing::setButtonState(i, state);

        //internally, Buttons class performs debouncing by shifting the current button state
        //into internal debounce counter for each button
        //if that variable equals 0xFF or BUTTON_DEBOUNCE_COMPARE constants (can vary depending on the board)
        //button is considered pressed or released (stable state)
        //in real application, buttons.update() is called only when there are new readings to process
        //new readings take place every 1ms, however, buttons.update doesn't check the timing of the signal
        //therefore, it is up to the caller to ensure that the readings are taken on constant time base
        //for tests, timing of the reading is irrelevant, however, debouncing still takes place
        //this function ensures that buttons.update is called enough times in order for buttons to register a state
        //stable state should be registered once debouncingState variable equals 0xFF
        while (1)
        {
            debouncingState = (debouncingState << (uint8_t)1) | (uint8_t)1 | BUTTON_DEBOUNCE_COMPARE;

            if (debouncingState == 0xFF)
                break;

            buttons.update();

            //due to the debouncing, no messages should be sent yet
            EXPECT_EQ(testing::messageCounter, 0);
        }

        // messages should now be sent
        buttons.update();
    };

    Database      database = Database(DatabaseStub::read, DatabaseStub::write, EEPROM_SIZE - 3);
    MIDI          midi;
    ComponentInfo cInfo;

#ifdef DISPLAY_SUPPORTED
    Interface::Display display;
#endif

#ifdef LEDS_SUPPORTED
    Interface::digital::output::LEDs leds = Interface::digital::output::LEDs(database);
#endif

#ifdef LEDS_SUPPORTED
#ifdef DISPLAY_SUPPORTED
    Interface::digital::input::Buttons buttons = Interface::digital::input::Buttons(database, midi, leds, display, cInfo);
#else
    Interface::digital::input::Buttons buttons = Interface::digital::input::Buttons(database, midi, leds, cInfo);
#endif
#else
#ifdef DISPLAY_SUPPORTED
    Interface::digital::input::Buttons buttons = Interface::digital::input::Buttons(database, midi, display, cInfo);
#else
    Interface::digital::input::Buttons buttons = Interface::digital::input::Buttons(database, midi, cInfo);
#endif
#endif
};

namespace Board
{
    namespace io
    {
        uint8_t getEncoderPair(uint8_t buttonID)
        {
            return 0;
        }

        bool getButtonState(uint8_t buttonIndex)
        {
            return testing::buttonState[buttonIndex];
        }

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

        void writeLEDstate(uint8_t ledID, bool state)
        {
        }
    }    // namespace io
}    // namespace Board

TEST_F(ButtonsTest, Debouncing)
{
    using namespace Interface::digital::input;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        EXPECT_EQ(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)), true);

        //make all buttons send note messages
        EXPECT_EQ(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::note)), true);
    }

    //stateChangeRegister performs changing of button state
    //check stateChangeRegister function to see description of how debouncing works

    //simulate button press
    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    //try another press, verify that no new messages have been sent
    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, 0);

    //simulate button release
    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);
}

TEST_F(ButtonsTest, Note)
{
    using namespace Interface::digital::input;

    auto test = [this](uint8_t channel, uint8_t velocityValue) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            //configure all buttons as momentary
            EXPECT_EQ(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)), true);

            //make all buttons send note messages
            EXPECT_EQ(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::note)), true);

            //set passed velocity value
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, i, velocityValue));

            database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, i, channel);
            buttons.reset(i);
        }

        auto verifyValue = [&](bool state) {
            // verify all received messages
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            {
                uint8_t midiMessage = testing::midiPacket[i].Event << 4;
                EXPECT_EQ(midiMessage, state ? static_cast<uint8_t>(MIDI::messageType_t::noteOn)
                                             : midi.getNoteOffMode() == MIDI::noteOffType_t::standardNoteOff ? static_cast<uint8_t>(MIDI::messageType_t::noteOff)
                                                                                                             : static_cast<uint8_t>(MIDI::messageType_t::noteOn));

                EXPECT_EQ(state ? velocityValue : 0, testing::midiPacket[i].Data3);
                EXPECT_EQ(channel, midi.getChannelFromStatusByte(testing::midiPacket[i].Data1));

                //also verify MIDI ID
                //it should be equal to button ID by default
                EXPECT_EQ(i, testing::midiPacket[i].Data2);
            }
        };

        //simulate button press
        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);
        verifyValue(true);

        //simulate button release
        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);
        verifyValue(false);

        // try with the latching mode
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::latching)));

        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);
        verifyValue(true);

        //nothing should happen on release
        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, 0);

        //press again, new messages should arrive
        this->stateChangeRegister(true);
        verifyValue(false);
    };

    //test for all midi channels
    for (int i = 0; i < 16; i++)
    {
        //test for all velocity values
        for (int j = 1; j < 128; j++)
        {
            midi.setNoteOffMode(MIDI::noteOffType_t::standardNoteOff);
            test(i, i);

            //repeat the test with note off being sent as note on with velocity 0
            midi.setNoteOffMode(MIDI::noteOffType_t::noteOnZeroVel);
            test(i, i);
        }
    }
}

TEST_F(ButtonsTest, ProgramChange)
{
    using namespace Interface::digital::input;

    auto testProgramChange = [&](uint8_t channel) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            //configure all buttons as momentary
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

            //make all buttons send program change messages
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::programChange)));

            //configure the specifed midi channel
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, i, channel));

            buttons.reset(i);
        }

        auto verifyMessage = [&]() {
            // verify all received messages are program change
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            {
                uint8_t midiMessage = testing::midiPacket[i].Event << 4;
                EXPECT_EQ(midiMessage, static_cast<uint8_t>(MIDI::messageType_t::programChange));

                //byte 3 on program change should be 0
                EXPECT_EQ(0, testing::midiPacket[i].Data3);

                //verify channel
                EXPECT_EQ(channel, midi.getChannelFromStatusByte(testing::midiPacket[i].Data1));

                //also verify MIDI ID/program
                //it should be equal to button ID by default
                EXPECT_EQ(i, testing::midiPacket[i].Data2);
            }
        };

        //simulate button press
        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);
        verifyMessage();

        //program change shouldn't be sent on release
        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, 0);

        //repeat the entire test again, but with buttons configured as latching types
        //behaviour should be the same
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::latching)));

        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);
        verifyMessage();

        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, 0);
    };

    //test for all channels
    for (int i = 0; i < 16; i++)
        testProgramChange(i);

    //test programChangeInc/programChangeDec
    //revert to default database state first
    database.factoryReset(LESSDB::factoryResetType_t::full);
    this->stateChangeRegister(false);

    auto configurePCbutton = [this](uint8_t buttonID, uint8_t channel, bool increase) {
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, buttonID, static_cast<int32_t>(Buttons::type_t::momentary)));
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, buttonID, increase ? static_cast<int32_t>(Buttons::messageType_t::programChangeInc) : static_cast<int32_t>(Buttons::messageType_t::programChangeDec)));
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiChannel, buttonID, channel));

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            buttons.reset(i);
    };

    auto verifyProgramChange = [&](uint8_t buttonID, uint8_t channel, uint8_t program) {
        uint8_t midiMessage = testing::midiPacket[buttonID].Event << 4;
        EXPECT_EQ(midiMessage, static_cast<uint8_t>(MIDI::messageType_t::programChange));

        //byte 3 on program change should be 0
        EXPECT_EQ(0, testing::midiPacket[buttonID].Data3);

        //verify channel
        EXPECT_EQ(channel, midi.getChannelFromStatusByte(testing::midiPacket[buttonID].Data1));

        EXPECT_EQ(program, testing::midiPacket[buttonID].Data2);
    };

    configurePCbutton(0, 0, true);

    //verify that the received program change was 0 for button 0
    this->stateChangeRegister(true);
    verifyProgramChange(0, 0, 0);
    this->stateChangeRegister(false);

    //after this, verify that the received program change was 1 for button 0
    this->stateChangeRegister(true);
    verifyProgramChange(0, 0, 1);
    this->stateChangeRegister(false);

    //after this, verify that the received program change was 2 for button 0
    this->stateChangeRegister(true);
    verifyProgramChange(0, 0, 2);
    this->stateChangeRegister(false);

    //now, revert all buttons back to default
    database.factoryReset(LESSDB::factoryResetType_t::full);

    //configure some other button to programChangeInc
    configurePCbutton(4, 0, true);

    //verify that the program change is continuing to increase
    this->stateChangeRegister(true);
    verifyProgramChange(4, 0, 3);
    this->stateChangeRegister(false);

    this->stateChangeRegister(true);
    verifyProgramChange(4, 0, 4);
    this->stateChangeRegister(false);

    //now configure two buttons to send program change/inc
    configurePCbutton(0, 0, true);

    this->stateChangeRegister(true);
    //program change should be increased by 1, first by button 0
    verifyProgramChange(0, 0, 5);
    //then by button 4
    verifyProgramChange(4, 0, 6);
    this->stateChangeRegister(false);

    //configure another button to programChangeInc, but on other channel
    configurePCbutton(1, 4, true);

    this->stateChangeRegister(true);
    //program change should be increased by 1, first by button 0
    verifyProgramChange(0, 0, 7);
    //then by button 4
    verifyProgramChange(4, 0, 8);
    //program change should be sent on channel 4 by button 1
    verifyProgramChange(1, 4, 0);
    this->stateChangeRegister(false);

    //revert to default again
    database.factoryReset(LESSDB::factoryResetType_t::full);

    //now configure button 0 for programChangeDec
    configurePCbutton(0, 0, false);

    this->stateChangeRegister(true);
    //program change should decrease by 1
    verifyProgramChange(0, 0, 7);
    this->stateChangeRegister(false);

    //configure another button for programChangeDec
    configurePCbutton(1, 0, false);

    this->stateChangeRegister(true);
    //button 0 should decrease the value by 1
    verifyProgramChange(0, 0, 6);
    //button 1 should decrease it again
    verifyProgramChange(1, 0, 5);
    this->stateChangeRegister(false);

    //configure another button for programChangeDec
    configurePCbutton(2, 0, false);

    this->stateChangeRegister(true);
    //button 0 should decrease the value by 1
    verifyProgramChange(0, 0, 4);
    //button 1 should decrease it again
    verifyProgramChange(1, 0, 3);
    //button 2 should decrease it again
    verifyProgramChange(2, 0, 2);
    this->stateChangeRegister(false);

    //reset all received messages first
    testing::resetReceived();

    //only two program change messages should be sent
    //program change value is 0 after the second button decreases it
    //once the value is 0 no further messages should be sent in dec mode

    this->stateChangeRegister(true);
    //button 0 should decrease the value by 1
    verifyProgramChange(0, 0, 1);
    //button 1 should decrease it again
    verifyProgramChange(1, 0, 0);

    //verify that only two program change messages have been received
    uint8_t pcCounter = 0;

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //no program change messages should be sent
        uint8_t midiMessage = testing::midiPacket[i].Event << 4;

        if (midiMessage == static_cast<uint8_t>(MIDI::messageType_t::programChange))
            pcCounter++;
    }

    EXPECT_EQ(pcCounter, 2);

    this->stateChangeRegister(false);

    //revert all buttons to default
    database.factoryReset(LESSDB::factoryResetType_t::full);

    configurePCbutton(0, 0, true);

    this->stateChangeRegister(true);
    //button 0 should increase the last value by 1
    verifyProgramChange(0, 0, 1);
    this->stateChangeRegister(false);
}

TEST_F(ButtonsTest, ControlChange)
{
    using namespace Interface::digital::input;

    auto controlChangeTest = [this](uint8_t controlValue) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            //configure all buttons as momentary
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

            //make all buttons send control change messages
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChange)));

            //set passed control value
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, i, controlValue));

            buttons.reset(i);
        }

        auto verifyMessage = [this](uint8_t midiValue) {
            // verify all received messages are control change
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            {
                uint8_t midiMessage = testing::midiPacket[i].Event << 4;
                EXPECT_EQ(midiMessage, static_cast<uint8_t>(MIDI::messageType_t::controlChange));

                EXPECT_EQ(midiValue, testing::midiPacket[i].Data3);

                //verify channel
                EXPECT_EQ(0, midi.getChannelFromStatusByte(testing::midiPacket[i].Data1));

                //also verify MIDI ID
                //it should be equal to button ID by default
                EXPECT_EQ(i, testing::midiPacket[i].Data2);
            }
        };

        //simulate button press
        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        //no messages should be sent on release
        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, 0);

        //change to latching type
        //behaviour should be the same

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::latching)));

        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, 0);

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChangeReset)));

        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

        verifyMessage(0);

        //same test again, but in latching mode
        //now, on press, messages should be sent
        //on release, nothing should happen
        //on second press reset should be sent (CC with value 0)

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::latching)));

        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        this->stateChangeRegister(false);
        EXPECT_EQ(testing::messageCounter, 0);

        this->stateChangeRegister(true);
        EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

        verifyMessage(0);
    };

    //verify with all control values
    //value 0 is normally blocked to configure for users (via sysex)
    for (int i = 1; i < 128; i++)
        controlChangeTest(i);
}

TEST_F(ButtonsTest, NoMessages)
{
    using namespace Interface::digital::input;

    //configure all buttons to messageType_t::none so that midi messages aren't sent

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

        //don't send any message
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::none)));

        buttons.reset(i);
    }

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, 0);

    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, 0);

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::latching)));

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, 0);

    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, 0);

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, 0);

    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, 0);
}

#ifdef LEDS_SUPPORTED
TEST_F(ButtonsTest, LocalLEDcontrol)
{
    using namespace Interface::digital::input;
    using namespace Interface::digital::output;

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

        //send note messages
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::note)));

        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, i, 127));

        buttons.reset(i);
    }

    //configure one of the leds in local control mode
    EXPECT_TRUE(database.update(DB_BLOCK_LEDS, dbSection_leds_controlType, 0, static_cast<int32_t>(LEDs::controlType_t::localNoteForStateNoBlink)));
    //set 127 as activation value, 0 as activation ID
    EXPECT_TRUE(database.update(DB_BLOCK_LEDS, dbSection_leds_activationValue, 0, 127));
    EXPECT_TRUE(database.update(DB_BLOCK_LEDS, dbSection_leds_activationID, 0, 0));

    //all leds should be off initially
    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
        EXPECT_EQ(leds.getColor(i), LEDs::color_t::off);

    //simulate the press of all buttons
    //since led 0 is configured in local control mode, it should be on now
    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    EXPECT_TRUE(leds.getColor(0) != LEDs::color_t::off);

    //all other leds should remain off
    for (int i = 1; i < MAX_NUMBER_OF_LEDS; i++)
        EXPECT_EQ(leds.getColor(i), LEDs::color_t::off);

    //now release the button and verify that the led is off again
    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
        EXPECT_EQ(leds.getColor(i), LEDs::color_t::off);

    //test again in latching mode
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::latching)));

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    EXPECT_TRUE(leds.getColor(0) != LEDs::color_t::off);

    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, 0);

    //led should remain on
    EXPECT_TRUE(leds.getColor(0) != LEDs::color_t::off);

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    EXPECT_TRUE(leds.getColor(0) == LEDs::color_t::off);

    //test again in control change mode
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

        //send cc messages
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChange)));

        buttons.reset(i);
    }

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    //led should be off since it's configure to react on note messages and not on control change
    EXPECT_TRUE(leds.getColor(0) == LEDs::color_t::off);

    EXPECT_TRUE(database.update(DB_BLOCK_LEDS, dbSection_leds_controlType, 0, static_cast<int32_t>(LEDs::controlType_t::localCCforStateNoBlink)));

    //no messages being sent on release in CC mode
    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, 0);

    //nothing should happen on release yet
    EXPECT_TRUE(leds.getColor(0) == LEDs::color_t::off);

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    //led should be on now
    EXPECT_TRUE(leds.getColor(0) != LEDs::color_t::off);

    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, 0);

    //no messages sent - led must remain on
    EXPECT_TRUE(leds.getColor(0) != LEDs::color_t::off);

    //change the control value for button 0 to something else
    EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, 0, 126));

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    //led should be off now - it has received velocity 126 differing from activating one which is 127
    EXPECT_TRUE(leds.getColor(0) == LEDs::color_t::off);

    //try similar thing - cc with reset 0
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_type, i, static_cast<int32_t>(Buttons::type_t::momentary)));

        //send cc messages with reset
        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChangeReset)));

        EXPECT_TRUE(database.update(DB_BLOCK_BUTTONS, dbSection_buttons_velocity, i, 127));

        buttons.reset(i);
    }

    this->stateChangeRegister(true);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    EXPECT_TRUE(leds.getColor(0) != LEDs::color_t::off);

    this->stateChangeRegister(false);
    EXPECT_EQ(testing::messageCounter, MAX_NUMBER_OF_BUTTONS);

    EXPECT_TRUE(leds.getColor(0) == LEDs::color_t::off);
}
#endif