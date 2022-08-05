/*

Copyright 2015-2022 Igor Petrovic

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

#ifdef HW_SUPPORT_UART

#include "board/Board.h"
#include "board/Internal.h"
#include "core/src/util/RingBuffer.h"
#include "core/src/util/Util.h"
#include "core/src/MCU.h"

// generic UART driver, arch-independent

namespace
{
    /// Flag determining whether or not UART loopback functionality is enabled.
    /// When enabled, all incoming UART traffic is immediately passed on to UART TX.
    volatile bool _loopbackEnabled[CORE_MCU_MAX_UART_INTERFACES];

    /// Flag holding the state of UART interface (whether it's _initialized or not).
    bool _initialized[CORE_MCU_MAX_UART_INTERFACES];

    /// Buffer in which outgoing UART data is stored.
    core::util::RingBuffer<uint8_t, BUFFER_SIZE_UART_TX> _txBuffer[CORE_MCU_MAX_UART_INTERFACES];

    /// Buffer in which incoming UART data is stored.
    core::util::RingBuffer<uint8_t, BUFFER_SIZE_UART_RX> _rxBuffer[CORE_MCU_MAX_UART_INTERFACES];

#ifdef HW_SUPPORT_DMX
    Board::UART::dmxBuffer_t* _dmxBuffer;
    Board::UART::dmxBuffer_t* _dmxBufferQueued;
#endif
}    // namespace

namespace Board
{
    namespace UART
    {
        void setLoopbackState(uint8_t channel, bool state)
        {
            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return;
            }

            _loopbackEnabled[channel] = state;
        }

        bool deInit(uint8_t channel)
        {
            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return false;
            }

            if (Board::detail::UART::MCU::deInit(channel))
            {
                setLoopbackState(channel, false);

                _rxBuffer[channel].reset();
                _txBuffer[channel].reset();

                _initialized[channel] = false;

                return true;
            }

            return false;
        }

        initStatus_t init(uint8_t channel, config_t& config, bool force)
        {
            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return initStatus_t::ERROR;
            }

            if (isInitialized(channel) && !force)
            {
                return initStatus_t::ALREADY_INIT;    // interface already initialized
            }

            if (deInit(channel))
            {
                if (Board::detail::UART::MCU::init(channel, config))
                {
                    _initialized[channel] = true;
                    return initStatus_t::OK;
                }
            }

            return initStatus_t::ERROR;
        }

        bool isInitialized(uint8_t channel)
        {
            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return false;
            }

            return _initialized[channel];
        }

        bool read(uint8_t channel, uint8_t* buffer, size_t& size, const size_t maxSize)
        {
            size = 0;

            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return false;
            }

            while (_rxBuffer[channel].remove(buffer[size++]))
            {
                if (size >= maxSize)
                {
                    break;
                }
            }

            return size > 0;
        }

        bool read(uint8_t channel, uint8_t& value)
        {
            value = 0;

            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return false;
            }

            return _rxBuffer[channel].remove(value);
        }

        bool write(uint8_t channel, uint8_t* buffer, size_t size)
        {
            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return false;
            }

            for (size_t i = 0; i < size; i++)
            {
                while (!_txBuffer[channel].insert(buffer[i]))
                {
                    ;
                }

                Board::detail::UART::MCU::startTx(channel);
            }

            return true;
        }

        bool write(uint8_t channel, uint8_t value)
        {
            return write(channel, &value, 1);
        }

        bool isTxComplete(uint8_t channel)
        {
            if (channel >= CORE_MCU_MAX_UART_INTERFACES)
            {
                return false;
            }

            return Board::detail::UART::MCU::isTxComplete(channel);
        }

#ifdef HW_SUPPORT_DMX
        bool updateDmxBuffer(dmxBuffer_t& buffer)
        {
            CORE_MCU_ATOMIC_SECTION
            {
                // switch to the new buffer only once the current frame is fully sent
                _dmxBufferQueued = &buffer;

                // first init - assign it to _dmxBuffer as well
                if (_dmxBuffer == nullptr)
                {
                    _dmxBuffer = &buffer;
                }
            }

            return true;
        }
#endif
    }    // namespace UART

    namespace detail::UART
    {
        void storeIncomingData(uint8_t channel, uint8_t data)
        {
            if (!_loopbackEnabled[channel])
            {
                _rxBuffer[channel].insert(data);
            }
            else
            {
                if (_txBuffer[channel].insert(data))
                {
                    Board::detail::UART::MCU::startTx(channel);

                    // indicate loopback here since it's run inside interrupt, ie. not visible to the user application
                    Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::UART,
                                                           Board::IO::indicators::direction_t::OUTGOING);

                    Board::IO::indicators::indicateTraffic(Board::IO::indicators::source_t::UART,
                                                           Board::IO::indicators::direction_t::INCOMING);
                }
            }
        }

        bool getNextByteToSend(uint8_t channel, uint8_t& data, size_t& remainingBytes)
        {
            if (_txBuffer[channel].remove(data))
            {
                remainingBytes = _txBuffer[channel].size();
                return true;
            }

            remainingBytes = 0;
            return false;
        }

        bool bytesToSendAvailable(uint8_t channel)
        {
            return _txBuffer[channel].size();
        }

#ifdef HW_SUPPORT_DMX
        Board::UART::dmxBuffer_t* dmxBuffer()
        {
            return _dmxBuffer;
        }

        void switchDmxBuffer()
        {
            // check if buffer needs to be switched
            if (_dmxBufferQueued != nullptr)
            {
                _dmxBuffer       = _dmxBufferQueued;
                _dmxBufferQueued = nullptr;
            }

            // else leave as is
        }
#endif
    }    // namespace detail::UART
}    // namespace Board

#endif