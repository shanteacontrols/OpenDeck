# Boards

OpenDeck board support is split into two layers:

- `app/boards/opendeck/`
- `app/boards/zephyr/`

They describe different things.

- `app/boards/opendeck/` describes the actual OpenDeck hardware topology.
- `app/boards/zephyr/` adapts an upstream Zephyr board so it can host that OpenDeck target.

## What Each Layer Contains

`app/boards/opendeck/` is the user-facing hardware description.

This layer defines things like:

- switches
- analog inputs
- outputs
- displays
- touchscreens
- DIN MIDI
- bus connections between those features
- target metadata used by build and test helpers
- target-specific OpenDeck resource aliases in `firmware.overlay`, when needed

`app/boards/zephyr/` is the Zephyr board adaptation layer.

This layer defines things like:

- partition layout
- low-level board-specific peripheral setup
- bootloader-specific overlay and config
- firmware-specific overlay and config
- optional board-specific sysbuild adjustments

## How Target Selection Works

OpenDeck targets are selected with `TARGET`.

Example:

```sh
make TARGET=pico
```

The build system loads:

- `app/boards/opendeck/<target>/firmware.overlay`

and reads the upstream Zephyr board identity from that file.

## Required Metadata

Each `app/boards/opendeck/<target>/firmware.overlay` must contain:

```dts
opendeck_metadata: opendeck-metadata {
    compatible = "opendeck,metadata";
    zephyr-board = "rpi_pico/rp2040";
};
```

This tells the build system which upstream Zephyr board should be used as the base for the target.

The `zephyr-board` value must map to a board layer under `app/boards/zephyr/`, with `/` replaced by `_`.

The metadata node can also define:

```dts
board-name = "My Custom Board";
```

When present, `board-name` overrides the runtime-facing `TARGET` name used for:

- USB MIDI product and label naming
- BLE MIDI device naming
- DFU / bootloader USB product naming

Examples:

- `rpi_pico/rp2040` -> `app/boards/zephyr/rpi_pico_rp2040/`
- `nrf54h20dk/nrf54h20/cpuapp` -> `app/boards/zephyr/nrf54h20dk_nrf54h20_cpuapp/`

## Optional Helper Metadata

Targets can also define helper-only metadata nodes.

Test capability node:

```dts
opendeck_tests: opendeck-tests {
    compatible = "opendeck,tests";
    host;
    hardware;
};
```

This describes whether the target supports:

- host tests
- hardware-backed tests

Bulk helper node:

```dts
opendeck_bulk_build: opendeck-bulk-build {
    compatible = "opendeck,bulk-build";
    app;
    host-test;
    hardware-test;
    lint;
};
```

This controls whether helper scripts such as `scripts/bulk_build.sh` include the target in:

- bulk app builds
- bulk host test runs
- bulk hardware test runs
- bulk lint runs

## Testcase Tags

Test suites under `tests/src/**/testcase.yaml` can also control how bulk helpers schedule them.

- `host` or `hardware` selects the test mode.
- `preset` tells `scripts/bulk_build.sh` to run that suite once per eligible target.

If `preset` is omitted, bulk helpers run the suite once using the first eligible target as the
representative target. This is useful for tests whose behavior does not actually vary by target.

## File Layout

For a target named `pico`, the layout looks like this:

```text
app/boards/
├── opendeck/
│   └── pico/
│       ├── firmware.overlay
│       ├── bootloader.overlay
└── zephyr/
    └── rpi_pico_rp2040/
        ├── partitions.overlay
        ├── firmware.overlay
        ├── firmware.conf
        ├── bootloader.overlay
        ├── bootloader.conf
        └── sysbuild.cmake
```

`sysbuild.cmake` in the board layer is optional. Target-level `firmware.overlay` is
required and owns the OpenDeck target metadata plus the firmware hardware model.
Target-level `bootloader.overlay` is required and owns recovery/bootloader-specific
hardware such as the MCUboot entry button. Target-level `.conf` files are only needed
when the target has OpenDeck-specific support flags or target-local config.

Use board-layer `.conf` files for low-level Zephyr board setup. Use target-level `.conf`
files for OpenDeck target support decisions, such as disabling USB MIDI or USB DFU on a
target that uses a board family where other targets may still enable it.

Board-layer `bootloader.overlay` files under `app/boards/zephyr/<board>/` are only
needed for low-level bootloader image DTS changes shared by every OpenDeck target using
that Zephyr board.

## Presets File

`app/presets.yml` is the build-preset file used by `zenv`, the Zephyr-oriented
build wrapper used by this project. When `make` runs, `zenv` parses that file
and turns the selected preset into the final build configuration inputs for
`west build`.

The project currently defines only one build preset, `default`. In
`app/presets.yml`, that preset groups config files into:

- `common`
  - always included
  - currently: `common.conf`
- `release`
  - used for normal OpenDeck builds
  - currently: `release.conf`
- `debug`
  - used instead of `release` when passing `DEBUG=1` to `make`
  - currently: `debug.conf`
  - enables console and UART logging support

## Overlay Merge Order

Firmware image overlay order:

1. upstream Zephyr board DTS
2. `app/boards/zephyr/<board>/partitions.overlay`
3. `app/boards/zephyr/<board>/firmware.overlay`
4. `app/boards/opendeck/<target>/firmware.overlay`
5. generated firmware USB MIDI label overlay

Bootloader image overlay order:

1. upstream Zephyr board DTS
2. `app/boards/zephyr/<board>/partitions.overlay`
3. `app/boards/zephyr/<board>/bootloader.overlay`
4. `app/boards/opendeck/<target>/bootloader.overlay`
5. `app/bootloader/firmware_loader.overlay`

Preset host tests do not merge target devicetree overlays. They consume the resolved target
facts generated by the matching application build:

1. native test board DTS, usually `native_sim`
2. `build/app/default/release/<target>/generated/target.kconfig`
3. `build/app/default/release/<target>/generated/target.conf`

## Config Merge Order

Firmware config order:

1. shared build config from `app/presets.yml`
2. `app/firmware/common.conf`
3. `app/firmware/release.conf` or `app/firmware/debug.conf`
4. `app/boards/zephyr/<board>/firmware.conf`
5. `app/common/network.conf`, when `opendeck_transports` enables network
6. `app/boards/zephyr/<board>/network.conf`, if the file exists and network is enabled
7. `app/common/usb.conf`, when `opendeck_transports` enables USB
8. `app/boards/zephyr/<board>/usb.conf`, if the file exists and USB is enabled
9. `app/firmware/usb.conf`, when `opendeck_transports` enables USB
10. generated firmware USB product config, when USB is enabled
11. `app/firmware/ble.conf`, when `opendeck_transports` enables BLE
12. generated firmware BLE device-name config, when BLE is enabled
13. `app/boards/opendeck/<target>/firmware.conf`, if the file exists

The target `firmware.overlay` is consulted by sysbuild before finalizing this
list. Its `opendeck_transports` node controls whether USB, BLE, and network transport
fragments are included.

Bootloader config order:

1. shared build config from `app/presets.yml`
2. `app/bootloader/common.conf`
3. `app/bootloader/release.conf` or `app/bootloader/debug.conf`
4. `app/boards/zephyr/<board>/bootloader.conf`
5. `app/common/network.conf`, when `opendeck_transports` enables network
6. `app/boards/zephyr/<board>/network.conf`, if the file exists and network is enabled
7. `app/common/usb.conf`, when `opendeck_transports` enables USB
8. `app/boards/zephyr/<board>/usb.conf`, if the file exists and USB is enabled
9. `app/bootloader/usb.conf`, when `opendeck_transports` enables USB
10. generated bootloader USB product config, when USB is enabled
11. `app/boards/opendeck/<target>/bootloader.conf`, if the file exists

The target `bootloader.overlay` is consulted by sysbuild before finalizing this
list. Its `opendeck_transports` node controls whether USB and network transport
fragments are included.

## Index Remapping

If logical component ordering differs from the physical scan order, these nodes can define:

- `index-remap`

This is currently supported on:

- `opendeck-switches`
- `opendeck-analog`
- `opendeck-outputs`

Use it only when a target needs a physical-to-logical reorder table.

## Adding A New Board

To add a new target:

1. Pick an upstream Zephyr board that matches your MCU or module.
2. Create the board layer under `app/boards/zephyr/<board>/`.
3. Add the normal board-layer files:
   - `partitions.overlay`
   - `firmware.overlay`
   - `firmware.conf`
   - `bootloader.conf`
   - `bootloader.overlay`, only if the bootloader image needs real DTS changes beyond generated alias labels
4. Add `app/boards/opendeck/<target>/firmware.overlay`.
5. Add `app/boards/opendeck/<target>/bootloader.overlay`.
6. Add `app/boards/opendeck/<target>/firmware.conf` or `bootloader.conf` only if the target needs OpenDeck-specific support flags or target-local config.
7. Add the required `opendeck,metadata` node with the correct `zephyr-board` value.
8. Describe the actual OpenDeck firmware hardware in `firmware.overlay`.
9. Add optional `opendeck,tests` and `opendeck,bulk-build` nodes if the target should participate in test or helper-script flows.

In practice:

- `app/boards/opendeck/<target>/firmware.overlay` should own firmware `opendeck_*` hardware nodes and firmware bus aliases
- `app/boards/zephyr/<board>/firmware.overlay` should own low-level peripheral enablement and common board setup
- `app/boards/opendeck/<target>/bootloader.overlay` should own bootloader-only hardware, including the `opendeck,bootloader` entry button node

For a fuller walkthrough, see the wiki page on creating a custom board variant:
https://github.com/shanteacontrols/OpenDeck/wiki/Creating-custom-board-variant
