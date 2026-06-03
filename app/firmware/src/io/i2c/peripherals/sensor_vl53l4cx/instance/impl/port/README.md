# VL53L4CX Port Layer

This directory contains the OpenDeck/Zephyr glue for the upstream ST VL53L4CX
driver imported through west.

The ST Arduino package keeps its platform boundary inside `vl53l4cx_class.h` and
expects `Arduino.h`, `Wire.h`, timing helpers, and I2C transfer functions. To keep
the west module unmodified, this directory provides the minimal names the driver
expects and routes them to OpenDeck APIs.

- `Wire.h` maps ST's `TwoWire` pointer to OpenDeck's I2C `Hwa`.
- `Arduino.h` provides only the tiny timing/GPIO surface required by the ST
  header. XSHUT GPIO calls are intentionally no-ops so the sensor can remain
  STEMMA QT / I2C-only.
- `vl53l4cx_port.cpp` replaces the upstream `vl53l4cx_class.cpp` platform
  implementation with OpenDeck I2C, Zephyr timing, and wait helpers.

The sensor behavior and ST ranging algorithms still come from the west module.
OpenDeck-specific setup, reset, filtering, logging, and OSC publishing stays
in `sensor_vl53l4cx.*` in this repository.
