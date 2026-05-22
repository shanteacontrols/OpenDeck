# OpenDeck Bootloader

OpenDeck uses MCUboot as the first-stage bootloader on every target. The
OpenDeck bootloader is built as MCUboot's firmware-loader image and is placed in
`slot1_partition`. The main firmware application is placed in
`slot0_partition`.

## Boot Flow

On reset, MCUboot runs first.

If no bootloader entry request is pending, MCUboot validates and starts the
application from `slot0_partition`.

If bootloader entry is requested, MCUboot starts the OpenDeck bootloader from
`slot1_partition`. Entry can be requested by firmware through Zephyr bootmode
retention, or by the target's MCUboot button when the board provides one.

## Firmware Update Paths

There are two update paths.

The application update path is used by WebSockets on Ethernet targets with
enough flash for staging. The application receives `dfu.bin`, writes it to the
`staged_dfu_partition`, requests bootloader entry, and reboots. The OpenDeck
bootloader then reads the staged DFU package, installs the signed firmware image
into `slot0_partition`, clears the staged update, and reboots back through
MCUboot.

The recovery update path is used when the OpenDeck bootloader is entered
directly. In this mode the bootloader receives `dfu.bin` over its transport,
currently WebUSB on USB-capable targets or WebSockets on network targets, and
installs the signed firmware image directly into `slot0_partition`.

In both paths, MCUboot remains responsible for validating the image before it is
started.

## Slot Roles

`boot_partition` contains MCUboot.

`slot0_partition` contains the OpenDeck application firmware.

`slot1_partition` contains the OpenDeck bootloader/recovery image. It is not a
pending application-update slot in the OpenDeck layout.

`staged_dfu_partition`, when present, stores application-uploaded `dfu.bin`
packages until the OpenDeck bootloader consumes them.
