/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

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

namespace Board
{
    namespace detail
    {
        ///
        /// \brief Common register names for MCUs with single UART channel.
        /// @{

        #if defined(__AVR_ATmega16U2__) || defined(__AVR_ATmega8U2__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) \
        || defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB1286__)

        #ifdef UCSR0A
        #define UCSRA_0 UCSR0A
        #elif defined UCSR1A
        #define UCSRA_0 UCSR1A
        #endif

        #ifdef UCSR0B
        #define UCSRB_0 UCSR0B
        #elif defined UCSR1B
        #define UCSRB_0 UCSR1B
        #endif

        #ifdef UCSR0C
        #define UCSRC_0 UCSR0C
        #elif defined UCSR1C
        #define UCSRC_0 UCSR1C
        #endif

        #ifdef UDRE0
        #define UDRE_0 UDRE0
        #elif defined UDRE1
        #define UDRE_0 UDRE1
        #endif

        #ifdef UDR0
        #define UDR_0 UDR0
        #elif defined UDR1
        #define UDR_0 UDR1
        #endif

        #ifdef UDRIE0
        #define UDRIE_0 UDRIE0
        #elif defined UDRIE1
        #define UDRIE_0 UDRIE1
        #endif

        #ifdef UBRR0
        #define UBRR_0 UBRR0
        #elif defined UBRR1
        #define UBRR_0 UBRR1
        #endif

        #ifdef U2X0
        #define U2X_0 U2X0
        #elif defined U2X1
        #define U2X_0 U2X1
        #endif

        #ifdef UCSZ00
        #define UCSZ0_0 UCSZ00
        #elif defined UCSZ10
        #define UCSZ0_0 UCSZ10
        #endif

        #ifdef UCSZ01
        #define UCSZ1_0 UCSZ01
        #elif defined UCSZ11
        #define UCSZ1_0 UCSZ11
        #endif

        #ifdef RXEN0
        #define RXEN_0 RXEN0
        #elif defined RXEN1
        #define RXEN_0 RXEN1
        #endif

        #ifdef TXEN0
        #define TXEN_0 TXEN0
        #elif defined TXEN1
        #define TXEN_0 TXEN1
        #endif

        #ifdef RXCIE0
        #define RXCIE_0 RXCIE0
        #elif defined RXCIE1
        #define RXCIE_0 RXCIE1
        #endif

        #ifdef TXCIE0
        #define TXCIE_0 TXCIE0
        #elif defined TXCIE1
        #define TXCIE_0 TXCIE1
        #endif

        #ifdef USART0_RX_vect
        #define USART_RX_vect_0 USART0_RX_vect
        #elif defined USART1_RX_vect
        #define USART_RX_vect_0 USART1_RX_vect
        #elif defined USART_RX_vect
        #define USART_RX_vect_0 USART_RX_vect
        #endif

        #ifdef USART0_TX_vect
        #define USART_TX_vect_0 USART0_TX_vect
        #elif defined USART1_TX_vect
        #define USART_TX_vect_0 USART1_TX_vect
        #elif defined USART_TX_vect
        #define USART_TX_vect_0 USART_TX_vect
        #endif

        #ifdef USART0_UDRE_vect
        #define USART_UDRE_vect_0 USART0_UDRE_vect
        #elif defined USART1_UDRE_vect
        #define USART_UDRE_vect_0 USART1_UDRE_vect
        #elif defined USART_UDRE_vect
        #define USART_UDRE_vect_0 USART_UDRE_vect
        #endif

        #endif

        /// @}

        ///
        /// \brief Common register names for MCUs with multiple UART channels.
        /// @{

        #ifdef __AVR_ATmega2560__

        #ifdef UCSR0A
        #define UCSRA_0 UCSR0A
        #endif

        #if defined UCSR1A
        #define UCSRA_1 UCSR1A
        #endif

        #ifdef UCSR0B
        #define UCSRB_0 UCSR0B
        #endif

        #if defined UCSR1B
        #define UCSRB_1 UCSR1B
        #endif

        #ifdef UCSR0C
        #define UCSRC_0 UCSR0C
        #endif

        #if defined UCSR1C
        #define UCSRC_1 UCSR1C
        #endif

        #ifdef UDRE0
        #define UDRE_0 UDRE0
        #endif

        #if defined UDRE1
        #define UDRE_1 UDRE1
        #endif

        #ifdef UDR0
        #define UDR_0 UDR0
        #endif

        #if defined UDR1
        #define UDR_1 UDR1
        #endif

        #ifdef UDRIE0
        #define UDRIE_0 UDRIE0
        #endif

        #if defined UDRIE1
        #define UDRIE_1 UDRIE1
        #endif

        #ifdef UBRR0
        #define UBRR_0 UBRR0
        #endif

        #if defined UBRR1
        #define UBRR_1 UBRR1
        #endif

        #ifdef U2X0
        #define U2X_0 U2X0
        #endif

        #if defined U2X1
        #define U2X_1 U2X1
        #endif

        #ifdef UCSZ00
        #define UCSZ0_0 UCSZ00
        #endif

        #if defined UCSZ10
        #define UCSZ0_1 UCSZ10
        #endif

        #ifdef UCSZ01
        #define UCSZ1_0 UCSZ01
        #endif

        #if defined UCSZ11
        #define UCSZ1_1 UCSZ11
        #endif

        #ifdef RXEN0
        #define RXEN_0 RXEN0
        #endif

        #if defined RXEN1
        #define RXEN_1 RXEN1
        #endif

        #ifdef TXEN0
        #define TXEN_0 TXEN0
        #endif

        #if defined TXEN1
        #define TXEN_1 TXEN1
        #endif

        #ifdef RXCIE0
        #define RXCIE_0 RXCIE0
        #endif

        #if defined RXCIE1
        #define RXCIE_1 RXCIE1
        #endif

        #ifdef TXCIE0
        #define TXCIE_0 TXCIE0
        #endif

        #if defined TXCIE1
        #define TXCIE_1 TXCIE1
        #endif

        #ifdef USART0_RX_vect
        #define USART_RX_vect_0 USART0_RX_vect
        #endif

        #if defined USART1_RX_vect
        #define USART_RX_vect_1 USART1_RX_vect
        #endif

        #ifdef USART0_TX_vect
        #define USART_TX_vect_0 USART0_TX_vect
        #endif

        #if defined USART1_TX_vect
        #define USART_TX_vect_1 USART1_TX_vect
        #endif

        #ifdef USART0_UDRE_vect
        #define USART_UDRE_vect_0 USART0_UDRE_vect
        #endif

        #if defined USART1_UDRE_vect
        #define USART_UDRE_vect_1 USART1_UDRE_vect
        #endif

        #endif

        /// @}
    }
}