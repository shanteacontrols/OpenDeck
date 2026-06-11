# VL53L5CX Raw Monitor

Displays raw VL53L5CX row data and frame-to-frame changes with no Processing-side filtering. Use it to verify firmware smoothing, invalid zones, and output rate.

## Requirements

- Configure the board OSC destination to the computer running Processing.
- Set VL53L5CX output mode to **Grid**.
- VL53L5CX resolution can be 4x4 or 8x8.
- Install the Processing oscP5 library.
