/*

Copyright 2015-2021 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include <inttypes.h>
#include <functional>
#include "util/cinfo/CInfo.h"
#include "util/scheduler/Scheduler.h"
#include "sysex/src/SysExConf.h"
#include "CustomIDs.h"
#include "database/Database.h"
#include "midi/MIDI.h"
#include "io/buttons/Buttons.h"
#include "io/buttons/Filter.h"
#include "io/encoders/Encoders.h"
#include "io/encoders/Filter.h"
#include "io/analog/Analog.h"
#include "io/analog/Filter.h"
#include "io/leds/LEDs.h"
#include "io/display/Display.h"
#include "io/touchscreen/Touchscreen.h"
#include "bootloader/FwSelector/FwSelector.h"
#include "dmxusb/src/DMXUSBWidget.h"

#ifdef ADC_12_BIT
#define ADC_RESOLUTION IO::Analog::adcType_t::adc12bit
#else
#define ADC_RESOLUTION IO::Analog::adcType_t::adc10bit
#endif

class System
{
    public:
    enum class block_t : uint8_t
    {
        global,
        buttons,
        encoders,
        analog,
        leds,
        display,
        touchscreen,
        AMOUNT
    };

    enum class presetSetting_t : uint8_t
    {
        activePreset,
        presetPreserve,
        AMOUNT
    };

    enum class midiFeature_t : uint8_t
    {
        standardNoteOff,
        runningStatus,
        mergeEnabled,
        dinEnabled,
        passToDIN,
        AMOUNT
    };

    enum class midiMerge_t : uint8_t
    {
        mergeType,
        mergeUSBchannel,
        mergeDINchannel,
        AMOUNT
    };

    enum class midiMergeType_t
    {
        DINtoUSB,
        DINtoDIN,
        AMOUNT
    };

    enum class dmxSetting_t : uint8_t
    {
        enabled,
        AMOUNT
    };

    enum class serialPeripheral_t : uint8_t
    {
        dinMIDI,
        touchscreen,
        dmx
    };

    class Section
    {
        public:
        Section() = default;

        enum class global_t : uint8_t
        {
            midiFeatures,
            midiMerge,
            presets,
            dmx,
            AMOUNT
        };

        enum class button_t : uint8_t
        {
            type,
            midiMessage,
            midiID,
            velocity,
            midiChannel,
            AMOUNT
        };

        enum class encoder_t : uint8_t
        {
            enable,
            invert,
            mode,
            midiID,
            midiChannel,
            pulsesPerStep,
            acceleration,
            midiID_MSB,
            remoteSync,
            AMOUNT
        };

        enum class analog_t : uint8_t
        {
            enable,
            invert,
            type,
            midiID,
            midiID_MSB,
            lowerLimit,
            lowerLimit_MSB,
            upperLimit,
            upperLimit_MSB,
            midiChannel,
            AMOUNT
        };

        enum class leds_t : uint8_t
        {
            testColor,
            testBlink,
            global,
            activationID,
            rgbEnable,
            controlType,
            activationValue,
            midiChannel,
            AMOUNT
        };

        enum class display_t : uint8_t
        {
            features,
            setting,
            AMOUNT
        };

        enum class touchscreen_t : uint8_t
        {
            setting,
            xPos,
            yPos,
            width,
            height,
            onScreen,
            offScreen,
            pageSwitchEnabled,
            pageSwitchIndex,
            analogPage,
            analogStartXCoordinate,
            analogEndXCoordinate,
            analogStartYCoordinate,
            analogEndYCoordinate,
            analogType,
            analogResetOnRelease,
            AMOUNT
        };
    };

    using usbConnectionHandler_t = std::function<void()>;
    using uniqueID_t             = std::array<uint8_t, UID_BITS / 8>;

    // time in milliseconds after which all internal MIDI values will be forcefully resent when scheduled
    // done after preset change or on usb connection state change
    static constexpr uint32_t FORCED_VALUE_RESEND_DELAY = 500;

    class HWA
    {
        public:
        class IO
        {
            public:
            class LEDs
            {
                public:
                virtual bool   supported()                                                          = 0;
                virtual void   setState(size_t index, ::IO::LEDs::brightness_t brightness)          = 0;
                virtual size_t rgbIndex(size_t singleLEDindex)                                      = 0;
                virtual size_t rgbSignalIndex(size_t rgbIndex, ::IO::LEDs::rgbIndex_t rgbComponent) = 0;
            };

            class Analog
            {
                public:
                virtual bool supported()                          = 0;
                virtual bool value(size_t index, uint16_t& value) = 0;
            };

            class Buttons
            {
                public:
                virtual bool   supported()                                                      = 0;
                virtual bool   state(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
                virtual size_t buttonToEncoderIndex(size_t index)                               = 0;
            };

            class Encoders
            {
                public:
                virtual bool supported()                                                      = 0;
                virtual bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) = 0;
            };

            class Touchscreen
            {
                public:
                virtual bool supported()          = 0;
                virtual bool init()               = 0;
                virtual bool deInit()             = 0;
                virtual bool write(uint8_t value) = 0;
                virtual bool read(uint8_t& value) = 0;
            };

            class CDCPassthrough
            {
                public:
                virtual bool init()                                                       = 0;
                virtual bool deInit()                                                     = 0;
                virtual bool uartRead(uint8_t& value)                                     = 0;
                virtual bool uartWrite(uint8_t value)                                     = 0;
                virtual bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) = 0;
                virtual bool cdcWrite(uint8_t* buffer, size_t size)                       = 0;
            };

            class Display
            {
                public:
                virtual bool supported()                                          = 0;
                virtual bool init()                                               = 0;
                virtual bool deInit()                                             = 0;
                virtual bool write(uint8_t address, uint8_t* buffer, size_t size) = 0;
            };

            virtual LEDs&           leds()           = 0;
            virtual Analog&         analog()         = 0;
            virtual Buttons&        buttons()        = 0;
            virtual Encoders&       encoders()       = 0;
            virtual Touchscreen&    touchscreen()    = 0;
            virtual CDCPassthrough& cdcPassthrough() = 0;
            virtual Display&        display()        = 0;
        };

        class Protocol
        {
            public:
            class MIDI
            {
                public:
                MIDI() = default;

                virtual bool dinSupported()                                   = 0;
                virtual bool init(::MIDI::interface_t interface)              = 0;
                virtual bool deInit(::MIDI::interface_t interface)            = 0;
                virtual bool setDINLoopback(bool state)                       = 0;
                virtual bool dinRead(uint8_t& data)                           = 0;
                virtual bool dinWrite(uint8_t data)                           = 0;
                virtual bool usbRead(::MIDI::USBMIDIpacket_t& USBMIDIpacket)  = 0;
                virtual bool usbWrite(::MIDI::USBMIDIpacket_t& USBMIDIpacket) = 0;
            };

            class DMX
            {
                public:
                DMX() = default;

                virtual bool supported()                                                  = 0;
                virtual bool init()                                                       = 0;
                virtual bool deInit()                                                     = 0;
                virtual bool readUSB(uint8_t* buffer, size_t& size, const size_t maxSize) = 0;
                virtual bool writeUSB(uint8_t* buffer, size_t size)                       = 0;
                virtual bool updateChannel(uint16_t channel, uint8_t value)               = 0;
                virtual void packetComplete()                                             = 0;
            };

            virtual MIDI& midi() = 0;
            virtual DMX&  dmx()  = 0;
        };

        virtual bool      init()                                                                        = 0;
        virtual void      update()                                                                      = 0;
        virtual void      reboot(FwSelector::fwType_t type)                                             = 0;
        virtual void      registerOnUSBconnectionHandler(usbConnectionHandler_t&& usbConnectionHandler) = 0;
        virtual bool      serialPeripheralAllocated(serialPeripheral_t peripheral)                      = 0;
        virtual bool      uniqueID(uniqueID_t& uniqueID)                                                = 0;
        virtual IO&       io()                                                                          = 0;
        virtual Protocol& protocol()                                                                    = 0;
    };

    System(HWA&      hwa,
           Database& database)
        : _sysExConf(
              _sysExDataHandler,
              _sysExMID)
        , _hwa(hwa)
        , _database(database)
        , _sysExDataHandler(*this)
        , _dbHandlers(*this)
        , _touchScreenHandlers(*this)
        , _hwaAnalog(*this)
        , _hwaButtons(*this)
        , _hwaEncoders(*this)
        , _hwaTouchscreen(*this)
        , _hwaCDCPassthrough(*this)
        , _hwaU8X8(*this)
        , _hwaMIDI(*this)
        , _hwaDMX(*this)
        , _hwaLEDs(*this)
    {}

    bool init();
    void run();

    private:
    enum class initAction_t : uint8_t
    {
        asIs,
        init,
        deInit
    };

    class SysExDataHandler : public SysExConf::DataHandler
    {
        public:
        SysExDataHandler(System& system)
            : _system(system)
        {}

        uint8_t get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value) override;
        uint8_t set(uint8_t block, uint8_t section, uint16_t index, uint16_t newValue) override;
        uint8_t customRequest(uint16_t request, CustomResponse& customResponse) override;
        void    sendResponse(uint8_t* array, uint16_t size) override;

        private:
        System& _system;
    };

    class DBhandlers : public Database::Handlers
    {
        public:
        DBhandlers(System& system)
            : _system(system)
        {}

        void presetChange(uint8_t preset) override;
        void factoryResetStart() override;
        void factoryResetDone() override;
        void initialized() override;

        private:
        System& _system;
    };

    class TouchScreenHandlers : public IO::Touchscreen::EventNotifier
    {
        public:
        TouchScreenHandlers(System& system)
            : _system(system)
        {}

        void button(size_t index, bool state) override;
        void analog(size_t index, uint16_t value, uint16_t min, uint16_t max) override;
        void screenChange(size_t screenID) override;

        private:
        System& _system;
    };

    class HWALEDs : public IO::LEDs::HWA
    {
        public:
        HWALEDs(System& system)
            : _system(system)
        {}

        void   setState(size_t index, IO::LEDs::brightness_t brightness) override;
        size_t rgbIndex(size_t singleLEDindex) override;
        size_t rgbSignalIndex(size_t rgbIndex, IO::LEDs::rgbIndex_t rgbComponent) override;

        private:
        System& _system;
    };

    class HWAAnalog : public IO::Analog::HWA
    {
        public:
        HWAAnalog(System& system)
            : _system(system)
        {}

        bool value(size_t index, uint16_t& value) override;

        private:
        System& _system;
    };

    class HWAButtons : public IO::Buttons::HWA
    {
        public:
        HWAButtons(System& system)
            : _system(system)
        {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override;

        private:
        System& _system;
    };

    class HWAEncoders : public IO::Encoders::HWA
    {
        public:
        HWAEncoders(System& system)
            : _system(system)
        {}

        bool state(size_t index, uint8_t& numberOfReadings, uint32_t& states) override;

        private:
        System& _system;
    };

    class HWATouchscreen : public IO::TouchscreenBase::HWA
    {
        public:
        HWATouchscreen(System& system)
            : _system(system)
        {}

        bool init() override;
        bool deInit() override;
        bool write(uint8_t value) override;
        bool read(uint8_t& value) override;

        private:
        System& _system;
    };

    class HWACDCPassthrough : public IO::Touchscreen::CDCPassthrough
    {
        public:
        HWACDCPassthrough(System& system)
            : _system(system)
        {}

        bool init() override;
        bool deInit() override;
        bool uartRead(uint8_t& value) override;
        bool uartWrite(uint8_t value) override;
        bool cdcRead(uint8_t* buffer, size_t& size, const size_t maxSize) override;
        bool cdcWrite(uint8_t* buffer, size_t size) override;

        private:
        System& _system;
    };

    class HWAU8X8 : public IO::U8X8::HWAI2C
    {
        public:
        HWAU8X8(System& system)
            : _system(system)
        {}

        bool init() override;
        bool deInit() override;
        bool write(uint8_t address, uint8_t* buffer, size_t size) override;

        private:
        System& _system;
    };

    class HWAMIDI : public MIDI::HWA
    {
        public:
        HWAMIDI(System& system)
            : _system(system)
        {}

        bool init(MIDI::interface_t interface) override;
        bool deInit(MIDI::interface_t interface) override;
        bool dinRead(uint8_t& value) override;
        bool dinWrite(uint8_t value) override;
        bool usbRead(MIDI::USBMIDIpacket_t& USBMIDIpacket) override;
        bool usbWrite(MIDI::USBMIDIpacket_t& USBMIDIpacket) override;

        private:
        System& _system;
        bool    _dinMIDIenabled         = false;
        bool    _dinMIDIloopbackEnabled = false;
    };

    class HWADMX : public DMXUSBWidget::HWA
    {
        public:
        HWADMX(System& system)
            : _system(system)
        {}

        bool init() override;
        bool deInit() override;
        bool readUSB(uint8_t* buffer, size_t& size, const size_t maxSize) override;
        bool writeUSB(uint8_t* buffer, size_t size) override;
        bool updateChannel(uint16_t channel, uint8_t value) override;
        void packetComplete() override;

        private:
        System& _system;
    };

    bool                             isMIDIfeatureEnabled(midiFeature_t feature);
    midiMergeType_t                  midiMergeType();
    void                             checkComponents();
    void                             checkMIDI();
    void                             configureMIDI();
    bool                             onGet(uint8_t block, uint8_t section, size_t index, uint16_t& value);
    bool                             onSet(uint8_t block, uint8_t section, size_t index, uint16_t newValue);
    bool                             onCustomRequest(size_t value);
    void                             onWrite(uint8_t* sysExArray, size_t size);
    void                             backup();
    void                             forceComponentRefresh();
    Database::block_t                dbBlock(uint8_t index);
    Database::Section::global_t      dbSection(Section::global_t section);
    Database::Section::button_t      dbSection(Section::button_t section);
    Database::Section::encoder_t     dbSection(Section::encoder_t section);
    Database::Section::analog_t      dbSection(Section::analog_t section);
    Database::Section::leds_t        dbSection(Section::leds_t section);
    Database::Section::display_t     dbSection(Section::display_t section);
    Database::Section::touchscreen_t dbSection(Section::touchscreen_t section);
    uint8_t                          onGetGlobal(Section::global_t section, size_t index, uint16_t& value);
    uint8_t                          onGetButtons(Section::button_t section, size_t index, uint16_t& value);
    uint8_t                          onGetEncoders(Section::encoder_t section, size_t index, uint16_t& value);
    uint8_t                          onGetAnalog(Section::analog_t section, size_t index, uint16_t& value);
    uint8_t                          onGetLEDs(Section::leds_t section, size_t index, uint16_t& value);
    uint8_t                          onGetDisplay(Section::display_t section, size_t index, uint16_t& value);
    uint8_t                          onGetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t& value);
    uint8_t                          onSetGlobal(Section::global_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetButtons(Section::button_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetEncoders(Section::encoder_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetAnalog(Section::analog_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetLEDs(Section::leds_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetDisplay(Section::display_t section, size_t index, uint16_t newValue);
    uint8_t                          onSetTouchscreen(Section::touchscreen_t section, size_t index, uint16_t newValue);

    SysExConf               _sysExConf;
    HWA&                    _hwa;
    Util::MessageDispatcher _dispatcher;
    Database&               _database;
    SysExDataHandler        _sysExDataHandler;
    DBhandlers              _dbHandlers;
    TouchScreenHandlers     _touchScreenHandlers;
    HWAAnalog               _hwaAnalog;
    HWAButtons              _hwaButtons;
    HWAEncoders             _hwaEncoders;
    HWATouchscreen          _hwaTouchscreen;
    HWACDCPassthrough       _hwaCDCPassthrough;
    HWAU8X8                 _hwaU8X8;
    HWAMIDI                 _hwaMIDI;
    HWADMX                  _hwaDMX;
    HWALEDs                 _hwaLEDs;
    IO::EncodersFilter      _encodersFilter;
    IO::ButtonsFilter       _buttonsFilter;
    Util::Scheduler         _scheduler;
    Protocol::MIDI          _midi         = Protocol::MIDI(_hwaMIDI, _dispatcher);
    DMXUSBWidget            _dmx          = DMXUSBWidget(_hwaDMX);
    Util::ComponentInfo     _cInfo        = Util::ComponentInfo(_dispatcher);
    IO::AnalogFilter        _analogFilter = IO::AnalogFilter(ADC_RESOLUTION);
    IO::LEDs                _leds         = IO::LEDs(_hwaLEDs, _database, _dispatcher);
    IO::Analog              _analog       = IO::Analog(_hwaAnalog, _analogFilter, _database, _dispatcher);
    IO::Buttons             _buttons      = IO::Buttons(_hwaButtons, _buttonsFilter, _database, _dispatcher);
    IO::Encoders            _encoders     = IO::Encoders(_hwaEncoders, _encodersFilter, 1, _database, _dispatcher);
    IO::Touchscreen         _touchscreen  = IO::Touchscreen(_hwaTouchscreen, _database, _hwaCDCPassthrough);
    IO::U8X8                _u8x8         = IO::U8X8(_hwaU8X8);
    IO::Display             _display      = IO::Display(_u8x8, _database, _dispatcher);

    const SysExConf::manufacturerID_t _sysExMID = {
        SYSEX_MANUFACTURER_ID_0,
        SYSEX_MANUFACTURER_ID_1,
        SYSEX_MANUFACTURER_ID_2
    };

    static constexpr uint8_t SERIAL_PERIPHERAL_ALLOCATED_ERROR = 80;
    static constexpr uint8_t CDC_ALLOCATED_ERROR               = 81;

    enum class backupRestoreState_t : uint8_t
    {
        none,
        backup,
        restore
    };

    backupRestoreState_t _backupRestoreState = backupRestoreState_t::none;

    // map sysex sections to sections in db
    const Database::Section::global_t _sysEx2DB_global[static_cast<uint8_t>(Section::global_t::AMOUNT)] = {
        Database::Section::global_t::midiFeatures,
        Database::Section::global_t::midiMerge,
        Database::Section::global_t::AMOUNT,    // unused
        Database::Section::global_t::dmx,
    };

    const Database::Section::button_t _sysEx2DB_button[static_cast<uint8_t>(Section::button_t::AMOUNT)] = {
        Database::Section::button_t::type,
        Database::Section::button_t::midiMessage,
        Database::Section::button_t::midiID,
        Database::Section::button_t::velocity,
        Database::Section::button_t::midiChannel
    };

    const Database::Section::encoder_t _sysEx2DB_encoder[static_cast<uint8_t>(Section::encoder_t::AMOUNT)] = {
        Database::Section::encoder_t::enable,
        Database::Section::encoder_t::invert,
        Database::Section::encoder_t::mode,
        Database::Section::encoder_t::midiID,
        Database::Section::encoder_t::midiChannel,
        Database::Section::encoder_t::pulsesPerStep,
        Database::Section::encoder_t::acceleration,
        Database::Section::encoder_t::midiID,
        Database::Section::encoder_t::remoteSync
    };

    const Database::Section::analog_t _sysEx2DB_analog[static_cast<uint8_t>(Section::analog_t::AMOUNT)] = {
        Database::Section::analog_t::enable,
        Database::Section::analog_t::invert,
        Database::Section::analog_t::type,
        Database::Section::analog_t::midiID,
        Database::Section::analog_t::midiID,
        Database::Section::analog_t::lowerLimit,
        Database::Section::analog_t::lowerLimit,
        Database::Section::analog_t::upperLimit,
        Database::Section::analog_t::upperLimit,
        Database::Section::analog_t::midiChannel
    };

    const Database::Section::leds_t _sysEx2DB_leds[static_cast<uint8_t>(Section::leds_t::AMOUNT)] = {
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::AMOUNT,
        Database::Section::leds_t::global,
        Database::Section::leds_t::activationID,
        Database::Section::leds_t::rgbEnable,
        Database::Section::leds_t::controlType,
        Database::Section::leds_t::activationValue,
        Database::Section::leds_t::midiChannel,
    };

    const Database::Section::display_t _sysEx2DB_display[static_cast<uint8_t>(Section::display_t::AMOUNT)] = {
        Database::Section::display_t::features,
        Database::Section::display_t::setting,
    };

    const Database::Section::touchscreen_t _sysEx2DB_touchscreen[static_cast<uint8_t>(Section::touchscreen_t::AMOUNT)] = {
        Database::Section::touchscreen_t::setting,
        Database::Section::touchscreen_t::xPos,
        Database::Section::touchscreen_t::yPos,
        Database::Section::touchscreen_t::width,
        Database::Section::touchscreen_t::height,
        Database::Section::touchscreen_t::onScreen,
        Database::Section::touchscreen_t::offScreen,
        Database::Section::touchscreen_t::pageSwitchEnabled,
        Database::Section::touchscreen_t::pageSwitchIndex,
        Database::Section::touchscreen_t::analogPage,
        Database::Section::touchscreen_t::analogStartXCoordinate,
        Database::Section::touchscreen_t::analogEndXCoordinate,
        Database::Section::touchscreen_t::analogStartYCoordinate,
        Database::Section::touchscreen_t::analogEndYCoordinate,
        Database::Section::touchscreen_t::analogType,
        Database::Section::touchscreen_t::analogResetOnRelease,
    };
};