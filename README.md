OpenDeck
========

Ownduino library capable of controlling LED/button matrix and reading of pots (multiplexed using 4051 multiplexer as well).
Library works together with HardwareReadSpecific library, in which the hardware reading is defined. Library requires Ownduino
library to work correctly.

Notes
========

Library works only with shared columns for buttons and LEDs. It cannot control multiple matrices.
Library also assumes that when connecting multiple 4051 chips to ATmega their control pins (S0, S1 and S2)
are connected together.