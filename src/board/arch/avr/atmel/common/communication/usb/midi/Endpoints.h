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

#pragma once

#define USB_ENDPOINT_ADDR_MIDI_IN     (core::mcu::usb::ENDPOINT_DIR_IN | 1)
#define USB_ENDPOINT_ADDR_MIDI_OUT    (core::mcu::usb::ENDPOINT_DIR_OUT | 2)
#define USB_ENDPOINT_SIZE_CONTROL     8
#define USB_ENDPOINT_SIZE_MIDI_IN_OUT 32