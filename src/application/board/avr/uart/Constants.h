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

#if defined(BOARD_A_MEGA) || defined(BOARD_A_UNO) || defined(BOARD_A_xu2)
#undef MIDI_BAUD_RATE
#define MIDI_BAUD_RATE 38400
#endif

#ifdef UCSR0A
#define UCSRA UCSR0A
#elif defined UCSR1A
#define UCSRA UCSR1A
#endif

#ifdef UCSR0B
#define UCSRB UCSR0B
#elif defined UCSR1B
#define UCSRB UCSR1B
#endif

#ifdef UCSR0C
#define UCSRC UCSR0C
#elif defined UCSR1C
#define UCSRC UCSR1C
#endif

#ifdef UDRE0
#define UDRE UDRE0
#elif defined UDRE1
#define UDRE UDRE1
#endif

#ifdef UDR0
#define UDR UDR0
#elif defined UDR1
#define UDR UDR1
#endif

#ifdef UDRIE0
#define UDRIE UDRIE0
#elif defined UDRIE1
#define UDRIE UDRIE1
#endif

#ifdef UBRR0
#define UBRR UBRR0
#elif defined UBRR1
#define UBRR UBRR1
#endif

#ifdef U2X0
#define U2X U2X0
#elif defined U2X1
#define U2X U2X1
#endif

#ifdef UCSZ00
#define UCSZ0 UCSZ00
#elif defined UCSZ10
#define UCSZ0 UCSZ10
#endif

#ifdef UCSZ01
#define UCSZ1 UCSZ01
#elif defined UCSZ11
#define UCSZ1 UCSZ11
#endif

#ifdef RXEN0
#define RXEN RXEN0
#elif defined RXEN1
#define RXEN RXEN1
#endif

#ifdef TXEN0
#define TXEN TXEN0
#elif defined TXEN1
#define TXEN TXEN1
#endif

#ifdef RXCIE0
#define RXCIE RXCIE0
#elif defined RXCIE1
#define RXCIE RXCIE1
#endif

#ifdef USART0_RX_vect
#define USART_RX_vect USART0_RX_vect
#elif defined USART1_RX_vect
#define USART_RX_vect USART1_RX_vect
#endif

#ifdef USART0_UDRE_vect
#define USART_UDRE_vect USART0_UDRE_vect
#elif defined USART1_UDRE_vect
#define USART_UDRE_vect USART1_UDRE_vect
#endif
