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
- `firmware/firmware.conf` configures the firmware image.
- `bootloader/bootloader.conf` configures the bootloader image.
- `firmware/usb.conf` and `bootloader/usb.conf` enable image-specific USB
  transport support when sysbuild selects them for a target.

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
- `builder/stub/` is used when the feature is compiled in but intentionally has
  no real implementation for the current target.

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

`shared/` contains module contracts and small common definitions used across
builders, HWA variants, instances, tests, and other modules. Typical files are:

- `common.h` for constants, enums, lightweight value types, and collection
  aliases.
- `deps.h` for interfaces consumed by an instance or implemented by HWA.
- Specialized shared headers such as `paths.h` or `frame_store.h` when the name
  is clearer than putting everything in `common.h`.

Root-level `common.h` and `deps.h` files should be avoided. If a header is part
of the module contract, put it under `shared/`.

## Drivers and Counts

Driver topology belongs under `drivers/`. Aggregation headers such as
`drivers/count.h` stay with the driver family because they describe hardware or
devicetree topology, not the module's public contract.

Current examples:

- `io/analog/drivers/count.h`
- `io/digital/drivers/count.h`
- `io/outputs/drivers/count.h`
- `io/touchscreen/drivers/count.h`

Shared module headers may include these count headers when collection sizes or
cross-module layout depend on them.

## Common Image Code

`common/src/` is for code shared by firmware and bootloader. It should be used
when both images need the same low-level contract or HWA, but not necessarily
the same behavior.

For example, indicators share HWA and dependency types through
`common/src/indicators/`, while firmware and bootloader keep separate indicator
instances because their runtime behavior is different.

## Include Style

Project includes are written relative to `app/`:

```cpp
#include "firmware/src/system/builder/builder.h"
#include "common/src/indicators/shared/deps.h"
```

This avoids ambiguity between firmware, bootloader, common, and test include
roots.
