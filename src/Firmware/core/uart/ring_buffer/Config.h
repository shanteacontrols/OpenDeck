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
