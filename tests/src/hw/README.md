# Hardware tests

These tests are run on actual OpenDeck boards. The files in `config/hw-test` directory are the configuration files for all boards which are connected to the computer. Tests will be run on those boards. Filename in that directory should match the one in `config/target` directory in repository.

Test rig consists of LattePanda Delta small PC with integrated Arduino Leonardo. Leonardo serves as "controller" accepting serial commands used to either read specified pin or to set pin state (low or high). Source code for Arduino is `ArduinoCTL` directory. Rig schematics can be found in `bin/sch/hwtestrig`. The rig currently tests the following boards:

* Arduino Mega2560
* Teensy++ 2.0
* STM32F401 Black Pill
* Official OpenDeck v2 board