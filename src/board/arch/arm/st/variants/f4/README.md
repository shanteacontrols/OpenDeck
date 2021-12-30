Notes when adding support for new MCU:

## Linker file

* Set FLASH origin to FLASH_START
* Add fwMetadata section in text
* Add .noinit section

## Startup file:

* Replace SystemInit call with InitSystem