# CAP1188 Touch Piano

Turns the eight CAP1188 capacitive touch inputs into a soft one-octave touch piano. Each electrode triggers one visual key and one generated note.

## Requirements

- Configure the board OSC destination to the computer running Processing.
- CAP1188 touch OSC output on `/opendeck/sensors/cap1188/touch/<0..7>`.
- Install the Processing oscP5 library.
- Install the Processing Sound library.
