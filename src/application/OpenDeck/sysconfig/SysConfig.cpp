/*

Copyright 2015-2020 Igor Petrovic

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

#include "SysConfig.h"
#include "board/Board.h"
#include "Version.h"
#include "Layout.h"
#include "core/src/general/Timing.h"
#include "common/OpenDeckMIDIformat/OpenDeckMIDIformat.h"

Database::block_t SysConfig::dbBlock(uint8_t index)
{
    //sysex blocks and db blocks don't have 1/1 mapping

    auto sysExBlock = static_cast<block_t>(index);

    switch (sysExBlock)
    {
    case block_t::global:
        return Database::block_t::global;

    case block_t::buttons:
        return Database::block_t::buttons;

    case block_t::encoders:
        return Database::block_t::encoders;

    case block_t::analog:
        return Database::block_t::analog;

    case block_t::leds:
        return Database::block_t::leds;

    case block_t::display:
        return Database::block_t::display;

    default:
        return Database::block_t::AMOUNT;
    }
}

Database::Section::global_t SysConfig::dbSection(Section::global_t section)
{
    return sysEx2DB_global[static_cast<uint8_t>(section)];
}

Database::Section::button_t SysConfig::dbSection(Section::button_t section)
{
    return sysEx2DB_button[static_cast<uint8_t>(section)];
}

Database::Section::encoder_t SysConfig::dbSection(Section::encoder_t section)
{
    return sysEx2DB_encoder[static_cast<uint8_t>(section)];
}

Database::Section::analog_t SysConfig::dbSection(Section::analog_t section)
{
    return sysEx2DB_analog[static_cast<uint8_t>(section)];
}

Database::Section::leds_t SysConfig::dbSection(Section::leds_t section)
{
    return sysEx2DB_leds[static_cast<uint8_t>(section)];
}

Database::Section::display_t SysConfig::dbSection(Section::display_t section)
{
    return sysEx2DB_display[static_cast<uint8_t>(section)];
}

void SysConfig::handleSysEx(const uint8_t* array, size_t size)
{
    sysExConf.handleMessage(array, size);
}

SysConfig::result_t SysConfig::SysExDataHandler::customRequest(size_t request, CustomResponse& customResponse)
{
    using namespace Board;

    auto result = SysConfig::result_t::ok;

    auto appendSW = [&customResponse]() {
        customResponse.append(SW_VERSION_MAJOR);
        customResponse.append(SW_VERSION_MINOR);
        customResponse.append(SW_VERSION_REVISION);
    };

    auto appendHW = [&customResponse]() {
        customResponse.append((FW_UID >> 24) & static_cast<uint32_t>(0xFF));
        customResponse.append((FW_UID >> 16) & static_cast<uint32_t>(0xFF));
        customResponse.append((FW_UID >> 8) & static_cast<uint32_t>(0xFF));
        customResponse.append(FW_UID & static_cast<uint32_t>(0xFF));
    };

    switch (request)
    {
    case SYSEX_CR_FIRMWARE_VERSION:
    {
        appendSW();
    }
    break;

    case SYSEX_CR_HARDWARE_UID:
    {
        appendHW();
    }
    break;

    case SYSEX_CR_FIRMWARE_VERSION_HARDWARE_UID:
    {
        appendSW();
        appendHW();
    }
    break;

    case SYSEX_CR_FACTORY_RESET:
        sysConfig.database.factoryReset(LESSDB::factoryResetType_t::partial);
        break;

    case SYSEX_CR_REBOOT_APP:
    case SYSEX_CR_REBOOT_BTLDR:
    {
        if (request == SYSEX_CR_REBOOT_BTLDR)
            Board::reboot(rebootType_t::rebootBtldr);
        else
            Board::reboot(rebootType_t::rebootApp);
    }
    break;

    case SYSEX_CR_MAX_COMPONENTS:
    {
        customResponse.append(MAX_NUMBER_OF_BUTTONS + MAX_NUMBER_OF_ANALOG + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS);
        customResponse.append(MAX_NUMBER_OF_ENCODERS);
        customResponse.append(MAX_NUMBER_OF_ANALOG);
        customResponse.append(MAX_NUMBER_OF_LEDS + MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS);
        customResponse.append(MAX_NUMBER_OF_TOUCHSCREEN_BUTTONS);
    }
    break;

    case SYSEX_CR_SUPPORTED_PRESETS:
    {
        customResponse.append(sysConfig.database.getSupportedPresets());
    }
    break;

    case SYSEX_CR_BOOTLOADER_SUPPORT:
    {
#ifdef BOOTLOADER_SUPPORTED
        customResponse.append(1);
#else
        customResponse.append(0);
#endif
    }
    break;

#ifdef DIN_MIDI_SUPPORTED
    case SYSEX_CR_DAISY_CHAIN:
    {
        //received message from opendeck board in daisy chain configuration
        //check if this board is master

        if (sysConfig.midiMergeType() == midiMergeType_t::odSlave)
        {
            //slave
            //send sysex to next board in the chain on uart channel
            sysConfig.sendDaisyChainRequest();
            //now configure daisy-chain configuration
            sysConfig.configureMIDImerge(midiMergeType_t::odSlave);
        }
    }
    break;
#endif

    case SYSEX_CR_ENABLE_PROCESSING:
    {
        sysConfig.processingEnabled = true;
    }
    break;

    case SYSEX_CR_DISABLE_PROCESSING:
    {
        sysConfig.processingEnabled = false;
    }
    break;

    default:
    {
        result = SysConfig::result_t::error;
    }
    break;
    }

    if (result == SysConfig::result_t::ok)
        sysConfig.display.displayMIDIevent(IO::Display::eventType_t::in, IO::Display::event_t::systemExclusive, 0, 0, 0);

    return result;
}

void SysConfig::init()
{
    sysExConf.setLayout(sysExLayout, static_cast<uint8_t>(block_t::AMOUNT));
    sysExConf.setupCustomRequests(customRequests, NUMBER_OF_CUSTOM_REQUESTS);

    configureMIDI();
}

bool SysConfig::isProcessingEnabled()
{
    return processingEnabled;
}

#ifdef DIN_MIDI_SUPPORTED
void SysConfig::setupMIDIoverUART(uint32_t baudRate, bool initRX, bool initTX)
{
    Board::UART::init(UART_CHANNEL_DIN, baudRate);

    if (initRX)
    {
        midi.handleUARTread([](uint8_t& data) {
            return Board::UART::read(UART_CHANNEL_DIN, data);
        });
    }
    else
    {
        midi.handleUARTread(nullptr);
    }

    if (initTX)
    {
        midi.handleUARTwrite([](uint8_t data) {
            return Board::UART::write(UART_CHANNEL_DIN, data);
        });
    }
    else
    {
        midi.handleUARTwrite(nullptr);
    }
}
#endif

void SysConfig::setupMIDIoverUSB()
{
#ifdef USB_MIDI_SUPPORTED
    midi.handleUSBread(Board::USB::readMIDI);
    midi.handleUSBwrite(Board::USB::writeMIDI);
#else
    //enable uart-to-usb link when usb isn't supported directly
    Board::UART::init(UART_CHANNEL_USB_LINK, UART_BAUDRATE_MIDI_OD);

    midi.handleUSBread([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
        OpenDeckMIDIformat::packetType_t odPacketType;

        if (OpenDeckMIDIformat::read(UART_CHANNEL_USB_LINK, USBMIDIpacket, odPacketType))
        {
            if (odPacketType == OpenDeckMIDIformat::packetType_t::midi)
                return true;
        }

        return false;
    });

    midi.handleUSBwrite([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
        return OpenDeckMIDIformat::write(UART_CHANNEL_USB_LINK, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);
    });
#endif
}

bool SysConfig::sendCInfo(Database::block_t dbBlock, SysExConf::sysExParameter_t componentID)
{
    if (sysExConf.isConfigurationEnabled())
    {
        if ((core::timing::currentRunTimeMs() - lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)]) > COMPONENT_INFO_TIMEOUT)
        {
            SysExConf::sysExParameter_t cInfoMessage[] = {
                SYSEX_CM_COMPONENT_ID,
                static_cast<SysExConf::sysExParameter_t>(dbBlock),
                static_cast<SysExConf::sysExParameter_t>(componentID)
            };

            sysExConf.sendCustomMessage(cInfoMessage, 3);
            lastCinfoMsgTime[static_cast<uint8_t>(dbBlock)] = core::timing::currentRunTimeMs();
        }

        return true;
    }

    return false;
}

void SysConfig::configureMIDI()
{
    midi.setInputChannel(MIDI_CHANNEL_OMNI);
    midi.setNoteOffMode(isMIDIfeatureEnabled(midiFeature_t::standardNoteOff) ? MIDI::noteOffType_t::standardNoteOff : MIDI::noteOffType_t::noteOnZeroVel);
    midi.setRunningStatusState(isMIDIfeatureEnabled(midiFeature_t::runningStatus));
    midi.setChannelSendZeroStart(true);

    setupMIDIoverUSB();

#ifdef DIN_MIDI_SUPPORTED
    if (isMIDIfeatureEnabled(midiFeature_t::dinEnabled))
    {
        bool mergeEnabled = isMIDIfeatureEnabled(midiFeature_t::mergeEnabled);

        //use recursive parsing when merging is active
        midi.useRecursiveParsing(mergeEnabled);

        //only configure master
        if (mergeEnabled)
        {
            auto type = midiMergeType();

            if (midiMergeType() == midiMergeType_t::odSlave)
                setupMIDIoverUART(UART_BAUDRATE_MIDI_OD, true, false);    //init only uart read interface for now
            else
                configureMIDImerge(type);
        }
        else
        {
            setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true);
        }
    }
    else
    {
        Board::UART::deInit(UART_CHANNEL_DIN);
        midi.handleUARTread(nullptr);
        midi.handleUARTwrite(nullptr);
    }
#endif
}

#ifdef DIN_MIDI_SUPPORTED
void SysConfig::configureMIDImerge(midiMergeType_t mergeType)
{
    switch (mergeType)
    {
    case midiMergeType_t::odMaster:
    {
        Board::UART::setLoopbackState(UART_CHANNEL_DIN, false);
        Board::UART::init(UART_CHANNEL_DIN, UART_BAUDRATE_MIDI_OD);
        //before enabling master configuration, send slave request to other boards
        sendDaisyChainRequest();

        midi.handleUSBread([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            //dump everything from MIDI in to USB MIDI out
            MIDI::USBMIDIpacket_t            slavePacket;
            OpenDeckMIDIformat::packetType_t packetType;

            //use this function to forward all incoming data from other boards to usb
            if (OpenDeckMIDIformat::read(UART_CHANNEL_DIN, slavePacket, packetType))
            {
                if (packetType == OpenDeckMIDIformat::packetType_t::midi)
                {
#ifdef USB_MIDI_SUPPORTED
                    Board::USB::writeMIDI(slavePacket);
#else
                    OpenDeckMIDIformat::write(UART_CHANNEL_USB_LINK, slavePacket, OpenDeckMIDIformat::packetType_t::midi);
#endif
                }
            }

//read usb midi data and forward it to uart in od format
#ifdef USB_MIDI_SUPPORTED
            if (Board::USB::readMIDI(USBMIDIpacket))
            {
                return OpenDeckMIDIformat::write(UART_CHANNEL_DIN, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midiDaisyChain);
            }
#else
            if (OpenDeckMIDIformat::read(UART_CHANNEL_USB_LINK, USBMIDIpacket, packetType))
            {
                if (packetType == OpenDeckMIDIformat::packetType_t::midi)
                    return OpenDeckMIDIformat::write(UART_CHANNEL_DIN, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midiDaisyChain);
            }
#endif

            return false;
        });

#ifdef USB_MIDI_SUPPORTED
        midi.handleUSBwrite(Board::USB::writeMIDI);
#else
        midi.handleUSBwrite([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            return OpenDeckMIDIformat::write(UART_CHANNEL_USB_LINK, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);
        });
#endif
        //unused
        midi.handleUARTread(nullptr);
        midi.handleUARTwrite(nullptr);
    }
    break;

    case midiMergeType_t::odSlave:
    {
        Board::UART::setLoopbackState(UART_CHANNEL_DIN, false);
        Board::UART::init(UART_CHANNEL_DIN, UART_BAUDRATE_MIDI_OD);
        //forward all incoming messages to other boards
        midi.handleUSBread([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            OpenDeckMIDIformat::packetType_t packetType;

            if (OpenDeckMIDIformat::read(UART_CHANNEL_DIN, USBMIDIpacket, packetType))
            {
                if (packetType != OpenDeckMIDIformat::packetType_t::internalCommand)
                    return OpenDeckMIDIformat::write(UART_CHANNEL_DIN, USBMIDIpacket, packetType);
            }

            return false;
        });

        //write data to uart (opendeck format)
        midi.handleUSBwrite([](MIDI::USBMIDIpacket_t& USBMIDIpacket) {
            return OpenDeckMIDIformat::write(UART_CHANNEL_DIN, USBMIDIpacket, OpenDeckMIDIformat::packetType_t::midi);
        });

        //no need for uart handlers
        midi.handleUARTread(nullptr);
        midi.handleUARTwrite(nullptr);
    }
    break;

    case midiMergeType_t::odSlaveInitial:
    {
        //init only uart read interface for now
        setupMIDIoverUART(UART_BAUDRATE_MIDI_OD, true, false);
    }
    break;

    case midiMergeType_t::DINtoDIN:
    {
        //forward all incoming DIN MIDI data to DIN MIDI out
        //also send OpenDeck-generated traffic to DIN MIDI out
        setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, false, true);
        Board::UART::setLoopbackState(UART_CHANNEL_DIN, true);
    }
    break;

    case midiMergeType_t::DINtoUSB:
    {
        setupMIDIoverUSB();
        setupMIDIoverUART(UART_BAUDRATE_MIDI_STD, true, true);
    }
    break;

    default:
        break;
    }
}

void SysConfig::sendDaisyChainRequest()
{
    //send the message directly in custom request sysex format
    const uint8_t daisyChainSysEx[8] = {
        0xF0,
        SYSEX_MANUFACTURER_ID_0,
        SYSEX_MANUFACTURER_ID_1,
        SYSEX_MANUFACTURER_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x00,
        SYSEX_CR_DAISY_CHAIN,
        0xF7
    };

    for (int i = 0; i < 8; i++)
        Board::UART::write(UART_CHANNEL_DIN, daisyChainSysEx[i]);

    while (!Board::UART::isTxEmpty(UART_CHANNEL_DIN))
        ;
}

#endif

bool SysConfig::isMIDIfeatureEnabled(midiFeature_t feature)
{
    return database.read(Database::Section::global_t::midiFeatures, static_cast<size_t>(feature));
}

SysConfig::midiMergeType_t SysConfig::midiMergeType()
{
    return static_cast<midiMergeType_t>(database.read(Database::Section::global_t::midiMerge, static_cast<size_t>(midiMerge_t::mergeType)));
}