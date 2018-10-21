Firmware update on OpenDeck boards is based on [LUFA HID bootloader](https://github.com/abcminiuser/lufa/tree/master/Bootloaders/HID). This directory contains two sub-directories:

* mcu - HID bootloader firmware for AVR boards
* pc - Modified version of the Python script found in LUFA HID repository

Changes from original source code found in LUFA repository:

* VID/PID numbers are changed
* Bootloader checks application CRC before loading it
* Added support for Arduino Mega and Uno
* Ability to trigger bootloader via software and hardware
