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
- test-only overlay data
- optional board-specific sysbuild adjustments

## How Target Selection Works

OpenDeck targets are selected with `TARGET`.

Example:

```sh
make TARGET=pico
```

The build system loads:

- `app/boards/opendeck/<target>/opendeck.overlay`

and reads the upstream Zephyr board identity from that file.

## Required Metadata

Each `app/boards/opendeck/<target>/opendeck.overlay` must contain:

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
    hw;
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
    hw-test;
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

- `host` or `hw` selects the test mode.
- `preset` tells `scripts/bulk_build.sh` to run that suite once per eligible target.

If `preset` is omitted, bulk helpers run the suite once using the first eligible target as the
representative target. This is useful for tests whose behavior does not actually vary by target.

## File Layout

For a target named `pico`, the layout looks like this:

```text
app/boards/
├── opendeck/
│   └── pico/
│       ├── opendeck.overlay
│       ├── firmware.overlay
└── zephyr/
    └── rpi_pico_rp2040/
        ├── partitions.overlay
        ├── firmware.overlay
        ├── firmware.conf
        ├── bootloader.overlay
        ├── bootloader.conf
        ├── test.overlay
        └── sysbuild.cmake
```

`sysbuild.cmake` in the board layer is optional. Target-level `firmware.overlay` and
`firmware.conf` are also optional and only needed when the target has extra OpenDeck-specific
wiring or config on top of the shared Zephyr board layer. Target-level
`bootloader.overlay` and `bootloader.conf` follow the same rule for bootloader-specific
OpenDeck changes.

Use board-layer `.conf` files for low-level Zephyr board setup. Use target-level `.conf`
files for OpenDeck target support decisions, such as disabling USB MIDI or USB DFU on a
target that uses a board family where other targets may still enable it.

When a target only needs abstract OpenDeck UART/I2C labels for bootloader or host-test DTS
parsing, those alias lines are generated automatically from the target `firmware.overlay`.
Only targets with real bootloader- or test-specific DTS changes need dedicated board-layer
`bootloader.overlay` or `test.overlay` files under `app/boards/zephyr/<board>/`.

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
4. `app/boards/opendeck/<target>/opendeck.overlay`
5. generated firmware USB MIDI label overlay
6. `app/boards/opendeck/<target>/firmware.overlay`, if the file exists

Bootloader image overlay order:

1. upstream Zephyr board DTS
2. `app/boards/zephyr/<board>/partitions.overlay`
3. `app/boards/zephyr/<board>/bootloader.overlay`
4. `app/boards/opendeck/<target>/opendeck.overlay`
5. generated bootloader alias overlay from `app/boards/opendeck/<target>/firmware.overlay`, if needed

Host-test overlay order:

1. native test board DTS, usually `native_sim`
2. `app/boards/zephyr/<board>/test.overlay`, if the file exists
3. generated test alias overlay from `app/boards/opendeck/<target>/firmware.overlay`, if needed
4. `app/boards/opendeck/<target>/opendeck.overlay`

## Config Merge Order

Firmware config order:

1. shared build config from `app/presets.yml`
2. `app/firmware/firmware.conf`
3. `app/boards/zephyr/<board>/firmware.conf`
4. `app/common/usb.conf`, when USB MIDI support is enabled
5. `app/firmware/usb.conf`, when USB MIDI support is enabled
6. generated firmware USB product config, when USB MIDI support is enabled
7. generated firmware BLE device-name config, when BLE peripheral support is enabled
8. `app/boards/opendeck/<target>/firmware.conf`, if the file exists

The target `firmware.conf` is also consulted by sysbuild before finalizing this list, so a
target can control whether USB MIDI support is enabled without changing the shared Zephyr
board layer.

Bootloader config order:

1. shared build config from `app/presets.yml`
2. `app/bootloader/bootloader.conf`
3. `app/boards/zephyr/<board>/bootloader.conf`
4. `app/common/usb.conf`, when bootloader USB DFU support is enabled
5. `app/bootloader/usb.conf`, when bootloader USB DFU support is enabled
6. generated bootloader USB product config, when bootloader USB DFU support is enabled
7. `app/boards/opendeck/<target>/bootloader.conf`, if the file exists

The target `bootloader.conf` is also consulted by sysbuild before finalizing this list, so a
target can control whether bootloader USB DFU support is enabled without changing the shared
Zephyr board layer.

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
   - `test.overlay`, only if host tests need shared emulated devices or partition labels for that board family
4. Add `app/boards/opendeck/<target>/opendeck.overlay`.
5. Add `app/boards/opendeck/<target>/firmware.overlay` only if the target needs extra OpenDeck-specific bus aliases or target-local DTS tweaks.
6. Add `app/boards/opendeck/<target>/firmware.conf` or `bootloader.conf` only if the target needs OpenDeck-specific support flags or target-local config.
7. Add the required `opendeck,metadata` node with the correct `zephyr-board` value.
8. Describe the actual OpenDeck hardware in `opendeck.overlay`.
9. Add optional `opendeck,tests` and `opendeck,bulk-build` nodes if the target should participate in test or helper-script flows.

In practice:

- `app/boards/opendeck/<target>/opendeck.overlay` should own all `opendeck_*` hardware nodes
- `app/boards/zephyr/<board>/firmware.overlay` should own low-level peripheral enablement and common board setup
- `app/boards/opendeck/<target>/firmware.overlay` should usually contain target-specific OpenDeck wiring on top of that shared board layer, most often abstract alias lines such as `opendeck_uart_din_midi: &uart1 {};`, `opendeck_uart_touchscreen: &uart2 {};`, or `opendeck_i2c_display: &i2c1 {};`
- custom targets can also use `app/boards/opendeck/<target>/firmware.overlay` to remap those OpenDeck resources to different UART or I2C instances than the shared board-layer default when the carrier wiring differs
- dedicated target `bootloader.overlay` files are usually unnecessary unless the target truly needs image-specific DTS changes

For a fuller walkthrough, see the wiki page on creating a custom board variant:
https://github.com/shanteacontrols/OpenDeck/wiki/Creating-custom-board-variant
