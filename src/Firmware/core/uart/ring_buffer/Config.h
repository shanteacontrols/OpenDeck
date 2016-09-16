/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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

#ifndef RING_BUFFER_CONFIG_H_
#define RING_BUFFER_CONFIG_H_

/* Defines: */
		/** Size of each ring buffer, in data elements - must be between 1 and 255. */
		#define BUFFER_SIZE            32
		
		/** Type of data to store into the buffer. */
		#define RingBuff_Data_t        uint8_t

		/** Data type which may be used to store the count of data stored in a buffer, retrieved
		 *  via a call to \ref RingBuffer_GetCount().
		 */
		#if (BUFFER_SIZE <= 0xFF)
			#define RingBuff_Count_t   uint8_t
		#else
			#define RingBuff_Count_t   uint16_t
		#endif

#endif
