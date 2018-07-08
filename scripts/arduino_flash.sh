#!/bin/bash

echo "Please type in correct tty port on which ArduinoISP is connected (ie. ttyUSB0):"
read port

echo "Please select MCU you want to flash and then press enter:"
echo "1 - ATmega8u2 on Uno or Mega"
echo "2 - ATmega16u2 on Uno or Mega"
echo "3 - ATmega328P on Uno"
echo "4 - ATmega2560 on Mega"
echo "5 - ATmega32u4 on Leonardo"
echo "6 - ATmega32u4 on Pro Micro"

read board

if [ $board == 1 ]
then
    echo "Connect programmer to ATmega8u2 programming header on the board and then press enter."
    read -n1 KEY
    avrdude -C /etc/avrdude.conf -p atmega8u2 -P /dev/$port -b 19200 -c avrisp -e -U lock:w:0xff:m -U efuse:w:0xf8:m -U hfuse:w:0xd3:m -U lfuse:w:0xff:m
    avrdude -C /etc/avrdude.conf -p atmega8u2 -P /dev/$port -b 19200 -c avrisp -U flash:w:../src/build/fw_8u2.bin -U lock:w:0xef:m
elif [ $board == 2 ]
then
    echo "Connect programmer to ATmega16u2 programming header on the board and then press enter."
    read -n1 KEY
    avrdude -C /etc/avrdude.conf -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -e -U lock:w:0xff:m -U efuse:w:0xf8:m -U hfuse:w:0xd3:m -U lfuse:w:0xff:m
    avrdude -C /etc/avrdude.conf -p atmega16u2 -P /dev/$port -b 19200 -c avrisp -U flash:w:../src/build/fw_16u2.bin -U lock:w:0xef:m
elif [ $board == 3 ]
then
    echo "Connect programmer to ATmega328P programming header on the Arduino Uno board and then press enter."
    read -n1 KEY
    avrdude -C /etc/avrdude.conf -p atmega328p -P /dev/$port -b 19200 -c avrisp -e -U lock:w:0xff:m -U efuse:w:0x04:m -U hfuse:w:0xd6:m -U lfuse:w:0xff:m
    avrdude -C /etc/avrdude.conf -p atmega328p -P /dev/$port -b 19200 -c avrisp -U flash:w:../src/build/fw_uno.bin -U lock:w:0xef:m
elif [ $board == 4 ]
then
    echo "Connect programmer to ATmega2560 programming header on the Arduino Mega2560 board and then press enter."
    read -n1 KEY
    avrdude -C /etc/avrdude.conf -p atmega2560 -P /dev/$port -b 19200 -c avrisp -e -U lock:w:0xff:m -U efuse:w:0xfc:m -U hfuse:w:0xd6:m -U lfuse:w:0xff:m
    avrdude -C /etc/avrdude.conf -p atmega2560 -P /dev/$port -b 19200 -c avrisp -U flash:w:../src/build/fw_mega.bin -U lock:w:0xef:m
elif [ $board == 5 ]
then
    echo "Connect programmer to ATmega32u4 programming header on the Arduino Leonardo board and then press enter."
    read -n1 KEY
    avrdude -C /etc/avrdude.conf -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -e -U lock:w:0xff:m -U efuse:w:0xf0:m -U hfuse:w:0xd3:m -U lfuse:w:0xff:m
    avrdude -C /etc/avrdude.conf -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -U flash:w:../src/build/fw_leonardo.bin -U lock:w:0xef:m
elif [ $board == 6 ]
then
    echo "Connect programmer to the Arduino Pro Micro board and then press enter."
    read -n1 KEY
    avrdude -C /etc/avrdude.conf -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -e -U lock:w:0xff:m -U efuse:w:0xc8:m -U hfuse:w:0xd0:m -U lfuse:w:0xff:m
    avrdude -C /etc/avrdude.conf -p atmega32u4 -P /dev/$port -b 19200 -c avrisp -U flash:w:../src/build/fw_pro_micro.bin -U lock:w:0xef:m
else
    echo "Incorrect board number selected."
fi
