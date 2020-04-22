#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "interface/digital/input/buttons/Buttons.h"
#include "interface/digital/output/leds/LEDs.h"
#include "interface/CInfo.h"
#include "midi/src/MIDI.h"
#include "core/src/general/Timing.h"
#include "database/Database.h"
#include "stubs/database/DB_ReadWrite.h"

namespace
{
    uint32_t              messageCounter                     = 0;
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

    bool midiDataHandler(MIDI::USBMIDIpacket_t& USBMIDIpacket)
    {
        midiPacket[messageCounter].Event = USBMIDIpacket.Event;
        midiPacket[messageCounter].Data1 = USBMIDIpacket.Data1;
        midiPacket[messageCounter].Data2 = USBMIDIpacket.Data2;
        midiPacket[messageCounter].Data3 = USBMIDIpacket.Data3;

        messageCounter++;

        return true;
    }

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers() {}

        void presetChange(uint8_t preset) override
        {
            if (presetChangeHandler != nullptr)
                presetChangeHandler(preset);
        }

        void factoryResetStart() override
        {
            if (factoryResetStartHandler != nullptr)
                factoryResetStartHandler();
        }

        void factoryResetDone() override
        {
            if (factoryResetDoneHandler != nullptr)
                factoryResetDoneHandler();
        }

        void initialized() override
        {
            if (initHandler != nullptr)
                initHandler();
        }

        //actions which these handlers should take depend on objects making
        //up the entire system to be initialized
        //therefore in interface we are calling these function pointers which
        // are set in application once we have all objects ready
        void (*presetChangeHandler)(uint8_t preset) = nullptr;
        void (*factoryResetStartHandler)()          = nullptr;
        void (*factoryResetDoneHandler)()           = nullptr;
        void (*initHandler)()                       = nullptr;
    } dbHandlers;

    class LEDsHWA : public Interface::digital::output::LEDs::HWA
    {
        public:
        LEDsHWA() {}

        void setState(size_t index, bool state) override
        {
        }

        size_t rgbSingleComponentIndex(size_t rgbIndex, Interface::digital::output::LEDs::rgbIndex_t rgbComponent) override
        {
            return 0;
        }

        size_t rgbIndex(size_t singleLEDindex) override
        {
            return 0;
        }

        void setFadeSpeed(size_t transitionSpeed) override
        {
        }
    } ledsHWA;

    DBstorageMock dbStorageMock;
    Database      database = Database(dbHandlers, dbStorageMock);
    MIDI          midi;
    ComponentInfo cInfo;

#ifdef DISPLAY_SUPPORTED
    Interface::Display display(database);
#endif

    Interface::digital::output::LEDs leds(ledsHWA, database);

#ifdef DISPLAY_SUPPORTED
    Interface::digital::input::Buttons buttons = Interface::digital::input::Buttons(database, midi, leds, display, cInfo);
#else
    Interface::digital::input::Buttons buttons = Interface::digital::input::Buttons(database, midi, leds, cInfo);
#endif

    void stateChangeRegister(bool state)
    {
        uint8_t debouncingState = BUTTON_DEBOUNCE_COMPARE;
        messageCounter          = 0;

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            setButtonState(i, state);

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
        while (true)
        {
            debouncingState = (debouncingState << (uint8_t)1) | (uint8_t)1 | BUTTON_DEBOUNCE_COMPARE;

            if (debouncingState == 0xFF)
                break;

            buttons.update();

            //due to the debouncing, no messages should be sent yet
            TEST_ASSERT(messageCounter == 0);
        }

        // messages should now be sent
        buttons.update();
    }
}    // namespace

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
            return buttonState[buttonIndex];
        }
    }    // namespace io
}    // namespace Board

TEST_SETUP()
{
    //init checks - no point in running further tests if these conditions fail
    TEST_ASSERT(database.init() == true);
    //always start from known state
    database.factoryReset(LESSDB::factoryResetType_t::full);
    TEST_ASSERT(database.isSignatureValid() == true);
    midi.handleUSBwrite(midiDataHandler);
    midi.setChannelSendZeroStart(true);
}

TEST_CASE(Debouncing)
{
    using namespace Interface::digital::input;

    //set known state
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

        //make all buttons send note messages
        TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::note)) == true);
    }

    //stateChangeRegister performs changing of button state
    //check stateChangeRegister function to see description of how debouncing works

    //simulate button press
    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    //try another press, verify that no new messages have been sent
    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == 0);

    //simulate button release
    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);
}

TEST_CASE(Note)
{
    using namespace Interface::digital::input;

    auto test = [&](uint8_t channel, uint8_t velocityValue) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            //configure all buttons as momentary
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

            //make all buttons send note messages
            TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::note)) == true);

            //set passed velocity value
            TEST_ASSERT(database.update(Database::Section::button_t::velocity, i, velocityValue) == true);

            database.update(Database::Section::button_t::midiChannel, i, channel);
            buttons.reset(i);
        }

        auto verifyValue = [&](bool state) {
            // verify all received messages
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            {
                uint8_t midiMessage = midiPacket[i].Event << 4;
                TEST_ASSERT(midiMessage == (state ? static_cast<uint8_t>(MIDI::messageType_t::noteOn)
                                                  : midi.getNoteOffMode() == MIDI::noteOffType_t::standardNoteOff ? static_cast<uint8_t>(MIDI::messageType_t::noteOff)
                                                                                                                  : static_cast<uint8_t>(MIDI::messageType_t::noteOn)));

                TEST_ASSERT(midiPacket[i].Data3 == (state ? velocityValue : 0));
                TEST_ASSERT(channel == midi.getChannelFromStatusByte(midiPacket[i].Data1));

                //also verify MIDI ID
                //it should be equal to button ID by default
                TEST_ASSERT(i == midiPacket[i].Data2);
            }
        };

        //simulate button press
        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);
        verifyValue(true);

        //simulate button release
        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);
        verifyValue(false);

        // try with the latching mode
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::latching)) == true);

        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);
        verifyValue(true);

        //nothing should happen on release
        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == 0);

        //press again, new messages should arrive
        stateChangeRegister(true);
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

TEST_CASE(ProgramChange)
{
    using namespace Interface::digital::input;

    auto testProgramChange = [&](uint8_t channel) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            //configure all buttons as momentary
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

            //make all buttons send program change messages
            TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::programChange)) == true);

            //configure the specifed midi channel
            TEST_ASSERT(database.update(Database::Section::button_t::midiChannel, i, channel) == true);

            buttons.reset(i);
        }

        auto verifyMessage = [&]() {
            // verify all received messages are program change
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            {
                uint8_t midiMessage = midiPacket[i].Event << 4;
                TEST_ASSERT(midiMessage == static_cast<uint8_t>(MIDI::messageType_t::programChange));

                //byte 3 on program change should be 0
                TEST_ASSERT(0 == midiPacket[i].Data3);

                //verify channel
                TEST_ASSERT(channel == midi.getChannelFromStatusByte(midiPacket[i].Data1));

                //also verify MIDI ID/program
                //it should be equal to button ID by default
                TEST_ASSERT(i == midiPacket[i].Data2);
            }
        };

        //simulate button press
        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);
        verifyMessage();

        //program change shouldn't be sent on release
        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == 0);

        //repeat the entire test again, but with buttons configured as latching types
        //behaviour should be the same
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::latching)) == true);

        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);
        verifyMessage();

        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == 0);
    };

    //test for all channels
    for (int i = 0; i < 16; i++)
        testProgramChange(i);

    //test programChangeInc/programChangeDec
    //revert to default database state first
    database.factoryReset(LESSDB::factoryResetType_t::full);
    stateChangeRegister(false);

    auto configurePCbutton = [&](uint8_t buttonID, uint8_t channel, bool increase) {
        TEST_ASSERT(database.update(Database::Section::button_t::type, buttonID, static_cast<int32_t>(Buttons::type_t::momentary)) == true);
        TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, buttonID, increase ? static_cast<int32_t>(Buttons::messageType_t::programChangeInc) : static_cast<int32_t>(Buttons::messageType_t::programChangeDec)) == true);
        TEST_ASSERT(database.update(Database::Section::button_t::midiChannel, buttonID, channel) == true);

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            buttons.reset(i);
    };

    auto verifyProgramChange = [&](uint8_t buttonID, uint8_t channel, uint8_t program) {
        uint8_t midiMessage = midiPacket[buttonID].Event << 4;
        TEST_ASSERT(midiMessage == static_cast<uint8_t>(MIDI::messageType_t::programChange));

        //byte 3 on program change should be 0
        TEST_ASSERT(0 == midiPacket[buttonID].Data3);

        //verify channel
        TEST_ASSERT(channel == midi.getChannelFromStatusByte(midiPacket[buttonID].Data1));

        TEST_ASSERT(program == midiPacket[buttonID].Data2);
    };

    configurePCbutton(0, 0, true);

    //verify that the received program change was 1 for button 0
    stateChangeRegister(true);
    verifyProgramChange(0, 0, 1);
    stateChangeRegister(false);

    //after this, verify that the received program change was 2 for button 0
    stateChangeRegister(true);
    verifyProgramChange(0, 0, 2);
    stateChangeRegister(false);

    //after this, verify that the received program change was 3 for button 0
    stateChangeRegister(true);
    verifyProgramChange(0, 0, 3);
    stateChangeRegister(false);

    //now, revert all buttons back to default
    database.factoryReset(LESSDB::factoryResetType_t::full);

    //configure some other button to programChangeInc
    configurePCbutton(4, 0, true);

    //verify that the program change is continuing to increase
    stateChangeRegister(true);
    verifyProgramChange(4, 0, 4);
    stateChangeRegister(false);

    stateChangeRegister(true);
    verifyProgramChange(4, 0, 5);
    stateChangeRegister(false);

    //now configure two buttons to send program change/inc
    configurePCbutton(0, 0, true);

    stateChangeRegister(true);
    //program change should be increased by 1, first by button 0
    verifyProgramChange(0, 0, 6);
    //then by button 4
    verifyProgramChange(4, 0, 7);
    stateChangeRegister(false);

    //configure another button to programChangeInc, but on other channel
    configurePCbutton(1, 4, true);

    stateChangeRegister(true);
    //program change should be increased by 1, first by button 0
    verifyProgramChange(0, 0, 8);
    //then by button 4
    verifyProgramChange(4, 0, 9);
    //program change should be sent on channel 4 by button 1
    verifyProgramChange(1, 4, 1);
    stateChangeRegister(false);

    //revert to default again
    database.factoryReset(LESSDB::factoryResetType_t::full);

    //now configure button 0 for programChangeDec
    configurePCbutton(0, 0, false);

    stateChangeRegister(true);
    //program change should decrease by 1
    verifyProgramChange(0, 0, 8);
    stateChangeRegister(false);

    stateChangeRegister(true);
    //program change should decrease by 1 again
    verifyProgramChange(0, 0, 7);
    stateChangeRegister(false);

    //configure another button for programChangeDec
    configurePCbutton(1, 0, false);

    stateChangeRegister(true);
    //button 0 should decrease the value by 1
    verifyProgramChange(0, 0, 6);
    //button 1 should decrease it again
    verifyProgramChange(1, 0, 5);
    stateChangeRegister(false);

    //configure another button for programChangeDec
    configurePCbutton(2, 0, false);

    stateChangeRegister(true);
    //button 0 should decrease the value by 1
    verifyProgramChange(0, 0, 4);
    //button 1 should decrease it again
    verifyProgramChange(1, 0, 3);
    //button 2 should decrease it again
    verifyProgramChange(2, 0, 2);
    stateChangeRegister(false);

    //reset all received messages first
    resetReceived();

    //only two program change messages should be sent
    //program change value is 0 after the second button decreases it
    //once the value is 0 no further messages should be sent in dec mode

    stateChangeRegister(true);
    //button 0 should decrease the value by 1
    verifyProgramChange(0, 0, 1);
    //button 1 should decrease it again
    verifyProgramChange(1, 0, 0);

    //verify that only two program change messages have been received
    uint8_t pcCounter = 0;

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //no program change messages should be sent
        uint8_t midiMessage = midiPacket[i].Event << 4;

        if (midiMessage == static_cast<uint8_t>(MIDI::messageType_t::programChange))
            pcCounter++;
    }

    TEST_ASSERT(pcCounter == 2);

    stateChangeRegister(false);

    //revert all buttons to default
    database.factoryReset(LESSDB::factoryResetType_t::full);

    configurePCbutton(0, 0, true);

    stateChangeRegister(true);
    //button 0 should increase the last value by 1
    verifyProgramChange(0, 0, 1);
    stateChangeRegister(false);
}

TEST_CASE(ControlChange)
{
    using namespace Interface::digital::input;

    auto controlChangeTest = [&](uint8_t controlValue) {
        //set known state
        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        {
            //configure all buttons as momentary
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

            //make all buttons send control change messages
            TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChange)) == true);

            //set passed control value
            TEST_ASSERT(database.update(Database::Section::button_t::velocity, i, controlValue) == true);

            buttons.reset(i);
        }

        auto verifyMessage = [&](uint8_t midiValue) {
            // verify all received messages are control change
            for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            {
                uint8_t midiMessage = midiPacket[i].Event << 4;
                TEST_ASSERT(midiMessage == static_cast<uint8_t>(MIDI::messageType_t::controlChange));

                TEST_ASSERT(midiValue == midiPacket[i].Data3);

                //verify channel
                TEST_ASSERT(0 == midi.getChannelFromStatusByte(midiPacket[i].Data1));

                //also verify MIDI ID
                //it should be equal to button ID by default
                TEST_ASSERT(i == midiPacket[i].Data2);
            }
        };

        //simulate button press
        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        //no messages should be sent on release
        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == 0);

        //change to latching type
        //behaviour should be the same

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::latching)) == true);

        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == 0);

        // change type to control change with 0 on reset, momentary mode
        // this means CC value 0 should be sent on release

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChangeReset)) == true);

        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

        verifyMessage(0);

        //same test again, but in latching mode
        //now, on press, messages should be sent
        //on release, nothing should happen
        //on second press reset should be sent (CC with value 0)

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
            TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::latching)) == true);

        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

        verifyMessage(controlValue);

        stateChangeRegister(false);
        TEST_ASSERT(messageCounter == 0);

        stateChangeRegister(true);
        TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

        verifyMessage(0);
    };

    //verify with all control values
    //value 0 is normally blocked to configure for users (via sysex)
    for (int i = 1; i < 128; i++)
        controlChangeTest(i);
}

TEST_CASE(NoMessages)
{
    using namespace Interface::digital::input;

    //configure all buttons to messageType_t::none so that midi messages aren't sent

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

        //don't send any message
        TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::none)) == true);

        buttons.reset(i);
    }

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == 0);

    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == 0);

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::latching)) == true);

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == 0);

    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == 0);

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == 0);

    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == 0);
}

#if MAX_NUMBER_OF_LEDS > 0
TEST_CASE(LocalLEDcontrol)
{
    using namespace Interface::digital::input;
    using namespace Interface::digital::output;

    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

        //send note messages
        TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::note)) == true);

        TEST_ASSERT(database.update(Database::Section::button_t::velocity, i, 127) == true);

        buttons.reset(i);
    }

    //configure one of the leds in local control mode
    TEST_ASSERT(database.update(Database::Section::leds_t::controlType, 0, static_cast<int32_t>(LEDs::controlType_t::localNoteForStateNoBlink)) == true);
    //set 127 as activation value, 0 as activation ID
    TEST_ASSERT(database.update(Database::Section::leds_t::activationValue, 0, 127) == true);
    TEST_ASSERT(database.update(Database::Section::leds_t::activationID, 0, 0) == true);

    //all leds should be off initially
    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
        TEST_ASSERT(leds.getColor(i) == LEDs::color_t::off);

    //simulate the press of all buttons
    //since led 0 is configured in local control mode, it should be on now
    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    TEST_ASSERT(leds.getColor(0) != LEDs::color_t::off);

    //all other leds should remain off
    for (int i = 1; i < MAX_NUMBER_OF_LEDS; i++)
        TEST_ASSERT(leds.getColor(i) == LEDs::color_t::off);

    //now release the button and verify that the led is off again
    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    for (int i = 0; i < MAX_NUMBER_OF_LEDS; i++)
        TEST_ASSERT(leds.getColor(i) == LEDs::color_t::off);

    //test again in latching mode
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::latching)) == true);

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    TEST_ASSERT(leds.getColor(0) != LEDs::color_t::off);

    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == 0);

    //led should remain on
    TEST_ASSERT(leds.getColor(0) != LEDs::color_t::off);

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    TEST_ASSERT(leds.getColor(0) == LEDs::color_t::off);

    //test again in control change mode
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

        //send cc messages
        TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChange)) == true);

        buttons.reset(i);
    }

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    //led should be off since it's configure to react on note messages and not on control change
    TEST_ASSERT(leds.getColor(0) == LEDs::color_t::off);

    TEST_ASSERT(database.update(Database::Section::leds_t::controlType, 0, static_cast<int32_t>(LEDs::controlType_t::localCCforStateNoBlink)) == true);

    //no messages being sent on release in CC mode
    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == 0);

    //nothing should happen on release yet
    TEST_ASSERT(leds.getColor(0) == LEDs::color_t::off);

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    //led should be on now
    TEST_ASSERT(leds.getColor(0) != LEDs::color_t::off);

    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == 0);

    //no messages sent - led must remain on
    TEST_ASSERT(leds.getColor(0) != LEDs::color_t::off);

    //change the control value for button 0 to something else
    TEST_ASSERT(database.update(Database::Section::button_t::velocity, 0, 126) == true);

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    //led should be off now - it has received velocity 126 differing from activating one which is 127
    TEST_ASSERT(leds.getColor(0) == LEDs::color_t::off);

    //try similar thing - cc with reset 0
    for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
    {
        //configure all buttons as momentary
        TEST_ASSERT(database.update(Database::Section::button_t::type, i, static_cast<int32_t>(Buttons::type_t::momentary)) == true);

        //send cc messages with reset
        TEST_ASSERT(database.update(Database::Section::button_t::midiMessage, i, static_cast<int32_t>(Buttons::messageType_t::controlChangeReset)) == true);

        TEST_ASSERT(database.update(Database::Section::button_t::velocity, i, 127) == true);

        buttons.reset(i);
    }

    stateChangeRegister(true);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    TEST_ASSERT(leds.getColor(0) != LEDs::color_t::off);

    stateChangeRegister(false);
    TEST_ASSERT(messageCounter == MAX_NUMBER_OF_BUTTONS);

    TEST_ASSERT(leds.getColor(0) == LEDs::color_t::off);
}
#endif