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
/// \brief Setup and manipulation of ADC peripheral.
/// \defgroup adc ADC
/// \ingroup core
/// @{

///
/// \brief Starts ADC conversion by setting ADSC bit in ADCSRA register.
///
#define startADCconversion() (ADCSRA |= (1<<ADSC))

///
/// \brief Enables ADC interrupts by setting ADIE bit in ADCSRA register.
///
#define adcInterruptEnable() (ADCSRA |= (1<<ADIE))

///
/// \brief Sets up ADC according to parameters specified in Config.h
///
void setUpADC();

///
/// \brief Sets active ADC channel
/// @param[in] channel ADC Channel.
///
void setADCchannel(uint8_t channel);

///
/// \brief Get ADC value from last set ADC channel.
/// \return Value from ADC registers (ADCH and ADCL).
///
uint16_t getADCvalue();

///
/// \brief Disable digital input circuitry on specified ADC channel to reduce noise.
/// @param[in] channel ADC Channel.
///
void disconnectDigitalInADC(uint8_t channel);

/// @}