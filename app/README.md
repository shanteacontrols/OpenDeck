# OpenDeck App Layout

This directory contains the Zephyr application images and the shared code used by
those images. The split is intentional: firmware and bootloader behavior differ,
but some hardware access and data definitions are shared.

## Top-Level Directories

- `firmware/` contains the main OpenDeck runtime image.
- `bootloader/` contains the bootloader image.
- `common/` contains code shared by more than one image.
- `boards/` contains OpenDeck target metadata, overlays, and per-target config.
- `dts/` contains OpenDeck devicetree bindings.

## Image Configuration

Image configuration is split between common and image-specific `.conf` files:

- `common.conf` contains settings shared by both firmware and bootloader where
  applicable.
- `firmware/common.conf`, `firmware/release.conf`, and `firmware/debug.conf`
  configure the firmware image.
- `bootloader/common.conf`, `bootloader/release.conf`, and
  `bootloader/debug.conf` configure the bootloader image.
- `common/network.conf`, `common/usb.conf`, `firmware/usb.conf`,
  `firmware/ble.conf`, and `bootloader/usb.conf` provide transport-specific
  settings when sysbuild selects them from the target transport devicetree node.

Per-board and per-target configuration should stay in `boards/`.

## Module Layout

Most modules under `firmware/src/` and `bootloader/src/` follow the same shape:

```text
module/
  builder/
    builder.h
    hw/builder_hw.h
    stub/builder_stub.h
    test/builder_test.h
  hwa/
    hw/hwa_hw.h
    stub/hwa_stub.h
    test/hwa_test.h
  instance/
    impl/
    stub/
  shared/
```

Not every module needs every variant. For example, a module without a stub or a
test implementation should not grow empty directories just to match the shape.

## Builders

`builder/builder.h` is the selector header for a module. It chooses the concrete
builder variant for the current build:

- `builder/hw/` is used by real firmware or bootloader images.
- `builder/test/` is used by host tests when a module needs a test-specific
  object graph.
- `builder/stub/` is used when the selected target does not support the feature

Code outside the module should include only `module/builder/builder.h` unless it
has a very specific reason to depend on a concrete variant.

## Hardware Abstraction

`hwa/` contains hardware access implementations:

- `hwa/hw/` talks to Zephyr devices, devicetree, flash, network, USB, UART, or
  other real hardware services.
- `hwa/test/` provides mocks or deterministic test implementations.
- `hwa/stub/` provides no-op behavior for unsupported features.

Hardware-specific CMake dependencies belong with the hardware HWA library when
the module has one. The parent module should not link hardware-only libraries
unconditionally, because tests and stubs should not inherit those dependencies.

## Instances

`instance/impl/` contains the real module behavior: state machines, protocol
logic, IO processing, mapping, and runtime behavior.

`instance/stub/` contains the no-op module instance used when the feature is not
available on a target.

Instance code should depend on module interfaces and shared types, not directly
on hardware-specific HWA details. The builder owns object construction and wires
the correct implementation together.

## Shared Headers

`shared/` contains module contracts and small common definitions used outside
the module that owns them. If a header is consumed only by files inside the same
module, keep it with that module's implementation, usually under
`instance/impl/`, `impl/`, or the concrete subdirectory that owns it.

Typical shared files are:

- `common.h` for constants, enums, lightweight value types, and collection
  aliases.
- `deps.h` for interfaces that must cross module boundaries.
- Specialized shared headers such as `paths.h` or `frame_store.h` when the name
  is clearer than putting everything in `common.h`, but only when those headers
  are also used outside their owning module.

Root-level `common.h` and `deps.h` files should be avoided. If a header is part
of the module contract, put it under `shared/`.

## Drivers and Target Counts

Driver topology belongs under `drivers/`. Resolved target facts such as switch,
analog, output, touchscreen, and storage sizes are exposed through hidden
`CONFIG_PROJECT_TARGET_*` symbols generated from the selected target
devicetree.

Runtime code should use those Kconfig symbols directly when it needs fixed-size
collections or compile-time feature selection. For example, shared IO headers
define collection sizes from symbols such as:

- `CONFIG_PROJECT_TARGET_ANALOG_LOGICAL_COUNT`
- `CONFIG_PROJECT_TARGET_SWITCH_LOGICAL_COUNT`
- `CONFIG_PROJECT_TARGET_OUTPUT_LOGICAL_COUNT`
- `CONFIG_PROJECT_TARGET_TOUCHSCREEN_COMPONENT_COUNT`

This keeps the target topology resolved in one place: the build's
devicetree/Kconfig pass. Tests that need target-shaped data import the generated
target config from a built application image instead of parsing target overlays.

## Common Image Code

`common/src/` is for code shared by firmware and bootloader. It should be used
when both images need the same low-level contract or HWA, but not necessarily
the same behavior.

For example, indicators share HWA and dependency types through
`common/src/io/indicators/`, while firmware and bootloader keep separate indicator
instances because their runtime behavior is different.

## Include Style

Project includes are written relative to `app/`:

```cpp
#include "firmware/src/system/builder/builder.h"
#include "common/src/io/indicators/shared/common.h"
```

This avoids ambiguity between firmware, bootloader, common, and test include
roots.
