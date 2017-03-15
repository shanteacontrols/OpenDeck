/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

///
/// \brief Helper macros for easier port and pin manipulation.
/// \defgroup corePinHelpers Pin manipulation
/// \ingroup coreHelpers
/// @{

///
/// \brief Definitions for active pin/signal logic.
/// @{
///
#define ACTIVE_HIGH             1
#define ACTIVE_LOW              0
/// @}

///
/// \brief I/O macros.
/// @{
///
#define setOutput(port, pin)    ((DDR(port)) |= (1 << (pin)))
#define setInput(port, pin)     ((DDR(port)) &= ~(1 << (pin)))
#define setLow(port, pin)       ((port) &= ~(1 << (pin)))
#define setHigh(port, pin)      ((port) |= (1 << (pin)))
#define readPin(port, pin)      (((PIN(port)) >> (pin)) & 0x01)
/// @}

///
/// \brief Workaround to avoid using DDR and PIN registers.
/// On most AVR models, DDR register is used to define pin direction within port (input or output),
/// and PIN port is used to read state of pin within port. To avoid defining DDR, PORT and PIN register for each
/// pin, these simple macros use the fact that DDR address = PORT address - 1 and PIN address = PORT address - 2.
/// Because of this, only PORT register must be defined, as well as specific pin.
/// Refer to specific AVR model datasheet for more info.
/// @{
///
#define DDR(x)                  (*(&x-1))
#define PIN(x)                  (*(&x-2))
/// @}

///
/// \brief Simple macro to create a high-to-low pulse.
///
#define pulseHightToLow(port, pin) do \
{ \
    setHigh((port), (pin)); \
    _NOP(); \
    setLow((port), (pin)); \
} while (0)

///
/// \brief Simple macro to create a low-to-high pulse.
///
#define pulseLowToHigh(port, pin) do \
{ \
    setLow((port), (pin)); \
    _NOP(); \
    setHigh((port), (pin)); \
} while (0)
/// @}