/*
    Notice:

    Portions of this software were developed at http://www.pjrc.com/
    Those portions licensed under MIT License Agreement, (the "License"); You may not use these files except in compliance with the License.
    You may obtain a copy of the License at: http://opensource.org/licenses/MIT
    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
    and limitations under the License.
*/


#ifndef usb_serial_h__
#define usb_serial_h__

#include <stdint.h>
#include "VID_PID.h"
#include "Endpoints.h"

#ifdef __cplusplus
extern "C"{
#endif

//setup
void usb_init(void);            //initialize everything
void usb_shutdown(void);        //shut off USB

void usbSend(uint8_t usbByte0, uint8_t usbByte1, uint8_t usbByte2, uint8_t usbByte3);

//variables
extern volatile uint8_t usb_configuration;
extern volatile uint8_t usb_suspended;

#ifdef __cplusplus
} // extern "C"
#endif

#endif