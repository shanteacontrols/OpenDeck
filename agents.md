# Session Helper

Board support in this repository is split into:

- `app/boards/zephyr/`
- `app/boards/opendeck/`

Use [app/boards/README.md](/home/ubuntu/zephyr_ws/project/app/boards/README.md) as the source of truth for how that split works.

## Board Support State

Current targets can be inferred from `app/boards/opendeck/`:

```sh
find app/boards/opendeck -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort
```

OpenDeck boards are selected with `TARGET`.

Useful verification commands:

- `make TARGET=nrf54h20dk`
- `make TARGET=pico`
- `make TARGET=nucleo_h753zi`
- `make TARGET=olimex_esp32_poe`

Notes:

- `app/presets.yml` is now just a generic build preset file.
- It currently contains a single preset, `default`.
- Shared build-mode config comes from:
  - `app/common.conf`
  - `app/release.conf`
  - `app/debug.conf`

## Testing

Testing uses Twister from Zephyr wrapped inside `make tests`.

`tests` accepts:

- `TEST=<test>`: build or run one specific test
- `TAG=<tag>`: filter by Twister tag
- `RUN=<0/1>`: run the tests instead of build-only

Examples:

- `make tests TAG=host`
- `make tests TEST=digital TAG=host`
- `make tests TEST=analog TAG=host`
- `make tests TEST=bootloader TAG=host`
- `make tests RUN=1 TAG=host`

Guidelines:

- Always add `TAG=host` unless you are intentionally working with hardware-tagged tests.
- If Twister behaves oddly, run `make clean` and retry before changing code.
- When fixing failing tests, start with logs in the relevant firmware path instead of introducing timing workarounds.

## Bulk Verification

Bulk verification is handled by `scripts/bulk_build.sh`. It selects targets from
the `opendeck,bulk-build` metadata in `app/boards/opendeck/**/firmware.overlay`.

Examples:

- `./scripts/bulk_build.sh --type=app`
- `./scripts/bulk_build.sh --type=host-test`
- `./scripts/bulk_build.sh --type=hw-test`
- `./scripts/bulk_build.sh --type=lint`

Notes:

- Run `--type=app` before `--type=host-test` or `--type=hw-test`; bulk tests
  consume generated target configs from `build/app/default/release/<target>/`.
- `host-test` and `hw-test` run shared suites once using a representative target,
  then run `preset` suites once per eligible target.
- never run bulk build with `hw-test` unless explicitly instructed to do so.
- `lint` runs `make CHECK=1 TARGET=<target>` for targets opted into bulk lint.

## CodeChecker

Do not remove, trim, or "fix" rules in `.codechecker.yml` just because the local `CodeChecker` or `clang-tidy` reports invalid checker-option messages.

Those messages usually mean the local tool version does not match the project's expected checker-option set. They do not by themselves mean the repo config is wrong.

## Things To Check First In New Sessions

1. If board or image composition looks wrong, inspect [app/sysbuild.cmake](/home/ubuntu/zephyr_ws/project/app/sysbuild.cmake).
2. If board metadata or layering is unclear, inspect [app/boards/README.md](/home/ubuntu/zephyr_ws/project/app/boards/README.md).
