/*
    Notice:

    Portions of this software were developed at http://www.pjrc.com/
    Those portions licensed under MIT License Agreement, (the "License"); You may not use these files except in compliance with the License.
    You may obtain a copy of the License at: http://opensource.org/licenses/MIT
    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
    and limitations under the License.
*/

#ifndef usb_common_h__
#define usb_common_h__

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_ENDPOINT                    6

#define LSB(n)                          (n & 255)
#define MSB(n)                          ((n >> 8) & 255)

#define EP_TYPE_CONTROL                 0x00
#define EP_TYPE_BULK_IN                 0x81
#define EP_TYPE_BULK_OUT                0x80
#define EP_SINGLE_BUFFER                0x02
#define EP_DOUBLE_BUFFER                0x06
#define EP_SIZE(s)                      ((s) == 64 ? 0x30 :     \
((s) == 32 ? 0x20 :     \
((s) == 16 ? 0x10 :     \
0x00)))

#define HW_CONFIG()                     (UHWCON = 0x01)
#define PLL_CONFIG()                    (PLLCSR = 0x12)
#define USB_CONFIG()                    (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE()                    (USBCON = ((1<<USBE)|(1<<FRZCLK)))

//standard control endpoint request types
#define GET_STATUS                      0
#define CLEAR_FEATURE                   1
#define SET_FEATURE                     3
#define SET_ADDRESS                     5
#define GET_DESCRIPTOR                  6
#define GET_CONFIGURATION               8
#define SET_CONFIGURATION               9

#define DESCRIPTOR_DEVICE_WVALUE        0x0100
#define DESCRIPTOR_CONFIGURATION_WVALUE 0x0200
#define DESCRIPTOR_STRING_WVALUE        0x0300

#define USB_LANGUAGE_ID                 0x0409 //US English

#define USB_STRING_DESCRIPTOR_TYPE      0x03

#define pgm_read_byte_postinc(val, addr) \
        asm ("lpm  %0, Z+\n" : "=r" (val), "+z" (addr) : )
#define pgm_read_word_postinc(val, addr) \
        asm ("lpm  %A0, Z+\n\tlpm  %B0, Z+\n" : "=r" (val), "+z" (addr) : )

#define read_word_lsbfirst(val, reg) \
        asm volatile( \
                "lds  %A0, %1\n\tlds  %B0, %1\n" \
                : "=r" (val) : "M" ((int)(&reg)) )

#define USBSTATE __attribute__ ((section (".noinit")))

#ifdef __cplusplus
} // extern "C"
#endif

#endif