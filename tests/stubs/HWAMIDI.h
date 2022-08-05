#pragma once

#include "protocol/midi/MIDI.h"

using namespace Protocol;

template<class Inherit, typename T>
class WriteParserHWA : public Inherit
{
    public:
    WriteParserHWA() = default;

    bool init() override
    {
        return true;
    }

    bool deInit() override
    {
        return true;
    }

    bool write(T& data) override
    {
        return false;
    }

    bool read(T& data) override
    {
        if (_toDecode.size())
        {
            data = _toDecode.at(0);
            _toDecode.erase(_toDecode.begin());

            return true;
        }

        return false;
    }

    uint32_t time()
    {
        return 0;
    }

    std::vector<T>               _toDecode;
    std::vector<MIDI::message_t> _decoded;
};

template<typename Inherit, typename T>
class WriteParser
{
    public:
    WriteParser()
    {
        _base.init();
    }

    void feed(T data)
    {
        _writeParserHWA._toDecode.push_back(data);

        while (_base.read())
        {
            _writeParserHWA._decoded.push_back(_base.message());
        }
    }

    std::vector<MIDI::message_t>& writtenMessages()
    {
        return _writeParserHWA._decoded;
    }

    size_t totalWrittenChannelMessages()
    {
        size_t cnt = 0;

        LOG(INFO) << "Checking for total amount of written channel messages";
        LOG(INFO) << "Total amount of packets written: " << _writeParserHWA._decoded.size();

        for (size_t i = 0; i < _writeParserHWA._decoded.size(); i++)
        {
            if (MIDIlib::Base::isChannelMessage(_writeParserHWA._decoded.at(i).type))
            {
                cnt++;
            }
        }

        return cnt;
    }

    void clear()
    {
        _writeParserHWA._toDecode.clear();
        _writeParserHWA._decoded.clear();
    }

    private:
    WriteParserHWA<typename Inherit::HWA, T> _writeParserHWA;
    Inherit                                  _base = Inherit(_writeParserHWA);
};

class HWAMIDIUSB : public Protocol::MIDI::HWAUSB
{
    public:
    HWAMIDIUSB() = default;

    bool supported() override
    {
        return true;
    }

    bool init() override
    {
        clear();
        return true;
    }

    bool deInit() override
    {
        clear();
        return true;
    }

    bool read(MIDI::usbMIDIPacket_t& packet) override
    {
        if (!_readPackets.size())
            return false;

        packet = _readPackets.at(0);
        _readPackets.erase(_readPackets.begin());

        return true;
    }

    bool write(MIDI::usbMIDIPacket_t& packet) override
    {
        _writePackets.push_back(packet);
        _writeParser.feed(packet);

        return true;
    }

    void clear()
    {
        _readPackets.clear();
        _writePackets.clear();
        _writeParser.clear();
    }

    std::vector<MIDI::usbMIDIPacket_t>                   _readPackets  = {};
    std::vector<MIDI::usbMIDIPacket_t>                   _writePackets = {};
    WriteParser<MIDIlib::USBMIDI, MIDI::usbMIDIPacket_t> _writeParser;
};

class HWAMIDIDIN : public Protocol::MIDI::HWADIN
{
    public:
    HWAMIDIDIN() = default;

    bool supported() override
    {
#ifdef HW_SUPPORT_DIN_MIDI
        return true;
#else
        return false;
#endif
    }

    MOCK_METHOD0(init, bool());
    MOCK_METHOD0(deInit, bool());
    MOCK_METHOD1(setLoopback, bool(bool state));

    bool read(uint8_t& data) override
    {
        if (!_readPackets.size())
            return false;

        data = _readPackets.at(0);
        _readPackets.erase(_readPackets.begin());

        return true;
    }

    bool write(uint8_t& data) override
    {
        _writePackets.push_back(data);
        _writeParser.feed(data);

        return true;
    }

    bool allocated(IO::Common::Allocatable::interface_t interface) override
    {
        return false;
    }

    void clear()
    {
        _readPackets.clear();
        _writePackets.clear();
        _writeParser.clear();

        _loopbackEnabled = false;
    }

    std::vector<uint8_t>                      _readPackets     = {};
    std::vector<uint8_t>                      _writePackets    = {};
    bool                                      _loopbackEnabled = false;
    WriteParser<MIDIlib::SerialMIDI, uint8_t> _writeParser;
};

class HWAMIDIBLE : public Protocol::MIDI::HWABLE
{
    public:
    HWAMIDIBLE() = default;

    bool supported() override
    {
#ifdef HW_SUPPORT_BLE
        return true;
#else
        return false;
#endif
    }

    MOCK_METHOD0(init, bool());
    MOCK_METHOD0(deInit, bool());

    bool write(MIDIlib::BLEMIDI::bleMIDIPacket_t& data) override
    {
        return false;
    }

    bool read(MIDIlib::BLEMIDI::bleMIDIPacket_t& data) override
    {
        return false;
    }

    uint32_t time() override
    {
        return 0;
    }

    void clear()
    {
        _readPackets.clear();
        _writePackets.clear();
    }

    std::vector<uint8_t>                                 _readPackets  = {};
    std::vector<uint8_t>                                 _writePackets = {};
    WriteParser<MIDIlib::BLEMIDI, MIDI::bleMIDIPacket_t> _writeParser;
};