# Indicators

This directory contains the indicator types and HWA implementations shared by
the firmware and bootloader images.

Only the low-level indicator backend is shared here. It knows how to configure
and drive the indicator GPIOs declared by the target devicetree.

The higher-level behavior is intentionally owned by each image:

- Firmware uses `app/firmware/src/io/indicators` for MIDI/network traffic
  indication and timeout handling.
- Bootloader uses `app/bootloader/src/indicators` for boot/update status
  indication.

Keep policy and image-specific behavior out of this common directory.
