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

## DFU Upload Framing

The selected firmware file is the generated `dfu.bin` file. The bytes inside
that file are already wrapped with the OpenDeck DFU stream header and footer.
The transport upload protocol does not reinterpret that payload. It only splits
the `dfu.bin` bytes into command frames and sends those frames to the running
application or bootloader.

The same command framing is used by:

* WebUSB bootloader DFU
* network WebSockets bootloader DFU
* network WebSockets staged update from the running application

The upload starts with a `Begin` command, continues with one or more `Chunk`
commands, and ends with a `Finish` command. `Abort` can be sent to cancel an
active upload.

Command values:

| Command | Value | Payload |
| --- | ---: | --- |
| `Begin` | `0x01` | none |
| `Chunk` | `0x02` | `dfu.bin` bytes |
| `Finish` | `0x03` | none |
| `Abort` | `0x04` | none |

Command-only frames are one byte:

```text
[command]
```

Chunk frames contain a one-byte command, a little-endian `uint16` payload length,
and the raw payload bytes:

```text
[0x02][length low byte][length high byte][payload...]
```

The chunk payload length is the number of `dfu.bin` bytes in that frame. It does
not include the command byte or the two length bytes.

Example chunk frame carrying `4` payload bytes:

```text
02 04 00 aa bb cc dd
```

The maximum chunk payload accepted by firmware is `2048` bytes. WebUSB uses
smaller frames so that each transfer fits one full-speed USB packet:

| Transport | Chunk payload bytes | Frame bytes |
| --- | ---: | ---: |
| WebUSB | `61` | `64` |
| WebSockets | up to `2048` | up to `2051` |

After each handled command, the device returns an ACK:

```text
[0x81][command][status][bytes_written uint32 little-endian]
```

ACK fields:

| Field | Size | Meaning |
| --- | ---: | --- |
| response marker | `1` byte | Always `0x81` for firmware-upload ACKs. |
| command | `1` byte | Command being acknowledged. |
| status | `1` byte | Result of the command. |
| bytes written | `4` bytes | Number of accepted `dfu.bin` payload bytes. |

Status values:

| Status | Value | Meaning |
| --- | ---: | --- |
| `Ok` | `0x00` | Command accepted. |
| `Failed` | `0x01` | Command was understood but the upload failed. |
| `Unsupported` | `0x02` | Firmware upload is not available on this target or build. |
| `BadRequest` | `0x03` | Frame shape or command payload was invalid. |

`Begin` resets any previous partial upload before accepting a new one. This
allows a new upload attempt to start cleanly even if the previous browser tab or
network client disappeared before sending `Abort`.

`Finish` succeeds only after the entire `dfu.bin` stream has been received and
validated by the DFU parser. On success, the bootloader finalizes the image and
reboots into the application. In staged update mode, the running application
stores the upload in the staged update area and then reboots into the bootloader
so the bootloader can apply it.

## Slot Roles

`boot_partition` contains MCUboot.

`slot0_partition` contains the OpenDeck application firmware.

`slot1_partition` contains the OpenDeck bootloader/recovery image. It is not a
pending application-update slot in the OpenDeck layout.

`staged_dfu_partition`, when present, stores application-uploaded `dfu.bin`
packages until the OpenDeck bootloader consumes them.
