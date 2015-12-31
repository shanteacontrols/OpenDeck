# Updating firmware on OpenDeck board

OpenDeck uses LUFA Mass storage bootloader to update the firmware on board. When new firmware version is relased, you can update your board easily using this bootloader which doesn't require any additional software on your PC.

Bootloader mode is triggered by sending special SysEx command to OpenDeck board.

Command:
`F0 00 53 43 44 F7`

*Note: hello message must be sent before sending any other SysEx commands. See `configuration.md` in `doc` directory.*

Once the board has received this message, the board will first slowly fade out any turned on LEDs untill they're off. After that, board will reboot and show on your PC as removable drive called "OPENDECK". When in bootloader mode, green LED on board will be active (BTLDR LED).

![Windows OS showing the OPENDECK drive.](https://dl.dropboxusercontent.com/u/2777613/OpenDeck/bootloader_drive.png)

The drive contains two files: `EEPROM.BIN` and `FLASH.BIN`.

![OPENDECK drive contents](https://dl.dropboxusercontent.com/u/2777613/OpenDeck/bootloader_drive_content.png)

`EEPROM.BIN` file contains all configuration data stored on board. `FLASH.BIN` file is firmware currently on board.

## Updating firmware

When updating new firmware, several steps are neccessary:

1) Delete the existing `FLASH.BIN` file

2) Copy new `FLASH.BIN` on drive. If any warnings appear, ignore them.

3) Eject the drive - **Do not** just unplug the board from USB port because the firmware will not be updated that way. If using Windows, please use "Eject" command available when right-clicking the drive. When using other OS, use the appropriate command. If the green LED blinks four times and then turns off when clicking "Eject", the firmware update has been successful.


