#!/bin/bash

echo "Please type in correct tty port on which ArduinoISP is connected (ie. ttyUSB0):"
read port

echo "Please select MCU you want to flash and then press enter:"
echo "1 - ATmega8u2 on Arduino Uno or Arduino Mega"
echo "2 - ATmega16u2 on Arduino Uno"
echo "3 - ATmega16u2 on Arduino Mega"
echo "4 - ATmega328P on Uno"
echo "5 - ATmega2560 on Mega"
echo "6 - ATmega32u4 on Leonardo"
echo "7 - ATmega32u4 on Pro Micro"
echo "8 - AT90USB1286 on Teensy++ 2.0"

read board

if [ $board == 1 ]
then
    echo "Connect programmer to ATmega8u2 programming header on the board and then press enter."
    read -n1 KEY
    avrdude -p atmega8u2 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xf8:m -U hfuse:w:0xd3:m -U lfuse:w:0xff:m
    avrdude -p atmega8u2 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw/fw_8u2.hex
    avrdude -p atmega8u2 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 2 ]
then
    echo "Connect programmer to ATmega16u2 programming header on Arduino Uno and then press enter."
    read -n1 KEY
    avrdude -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xf8:m -U hfuse:w:0xd0:m -U lfuse:w:0xff:m
    avrdude -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_16u2_uno.hex
    avrdude -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 3 ]
then
    echo "Connect programmer to ATmega16u2 programming header on Arduino Mega and then press enter."
    read -n1 KEY
    avrdude -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xf8:m -U hfuse:w:0xd0:m -U lfuse:w:0xff:m
    avrdude -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_16u2_mega.hex
    avrdude -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 4 ]
then
    echo "Connect programmer to ATmega328P programming header on the Arduino Uno board and then press enter."
    read -n1 KEY
    avrdude -p atmega328p -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xfc:m -U hfuse:w:0xd2:m -U lfuse:w:0xff:m
    avrdude -p atmega328p -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_uno.hex
    avrdude -p atmega328p -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 5 ]
then
    echo "Connect programmer to ATmega2560 programming header on the Arduino Mega2560 board and then press enter."
    read -n1 KEY
    avrdude -p atmega2560 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xfc:m -U hfuse:w:0xd2:m -U lfuse:w:0xff:m
    avrdude -p atmega2560 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_mega.hex
    avrdude -p atmega2560 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 6 ]
then
    echo "Connect programmer to ATmega32u4 programming header on the Arduino Leonardo board and then press enter."
    read -n1 KEY
    avrdude -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xc8:m -U hfuse:w:0xd0:m -U lfuse:w:0xff:m
    avrdude -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_leonardo.hex
    avrdude -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 7 ]
then
    echo "Connect programmer to the Arduino Pro Micro board and then press enter."
    read -n1 KEY
    avrdude -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xc8:m -U hfuse:w:0xd0:m -U lfuse:w:0xff:m
    avrdude -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_pro_micro.hex
    avrdude -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
elif [ $board == 8 ]
then
    echo "Connect programmer to the Teensy++ 2.0 board and then press enter."
    read -n1 KEY
    avrdude -p at90usb1286 -P /dev/$port -b 19200 -c avrisp -e -V -u -U lock:w:0xff:m -U efuse:w:0xf8:m -U hfuse:w:0xd2:m -U lfuse:w:0xff:m
    avrdude -p at90usb1286 -P /dev/$port -b 19200 -c avrisp -U flash:w:../bin/compiled/arduino+teensy/fw+boot/fw_boot_teensy2pp.hex
    avrdude -p at90usb1286 -P /dev/$port -b 19200 -c avrisp -V -u -U lock:w:0xef:m
else
    echo "Incorrect board number selected."
fi
