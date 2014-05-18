# OpenDeck

Ownduino library capable of controlling LED/button matrix and reading of potentiometers (multiplexed using 4051 multiplexer as well).
Library works together with `HardwareReadSpecific` library, in which the hardware reading methods are defined. Library requires Ownduino
library to work correctly.

Library is primarily intended for use in MIDI controllers and for use with MIDI library. It's features are:

* Built-in button debouncing
* Long-press button mode (defineable time)
* Analogue debouncing
* 6-way switch mode for each potentiometer
* Blink and constant mode for each LED
* Callbacks


## Basic usage

To initialize library, call `openDeck.init()` inside setup.

Only several functions need to be placed inside Ownduinos `timedLoop` function:

* `nextColumn()` activates next column
* `checkLEDs()` checks if any LED on currently active column is activated
* `readButtons()` reads all button rows on current column

`readPots()` should be placed inside `loop()`.


## LED Handling

Library activates LEDs depending on received MIDI note, parsed by MIDI library. Note 0 will activate constant mode on LED 1, Note 2 LED 1 etc. If there are
total of 10 LEDs, Note 10 will activate blink mode on LED 1, Note 11 on LED 2 etc. To process received note two functions are available:

* `storeReceivedNote(uint8_t channel, uint8_t pitch, uint8_t velocity)` which stores received note with its 3 parameters (this function should be called inside
callback function for `setHandleNoteOn` callback handler found in MIDI library)

* `checkReceivedNote()` which checks for stored input midi data and handles LED if message is received

Checking for whether the note has been received should be done inside `loop()`.

Library allows returning to previous LED state. If LED is blinking, and constant mode note on is received, LED will stop blinking. Upon receiving Note off
for constant state, LED will continue blinking. Vice versa works the same way.

To assure correct LED handling, `TOTAL_NUMBER_OF_LEDS` inside HardwareReadSpecific.h must be defined, as well as number for each LED, ie:

`#define LED_1 0`

`#define LED_2 1`

`ledArray` in `HardwareReadSpecific.h` must also be defined. Each defined LED must be put into array, ie:

`const uint8_t ledArray[TOTAL_NUMBER_OF_LEDS] = { LED_1, LED_2 };`

To find out the LED numbers, `turnOnLED(uint8_t ledNumber)` function is available.

### `oneByOneLED(bool ledDirection, bool singleLED, bool turnOn)` function

Used primarily for start-up animation. Call inside `setup()`.

Function accepts three boolean arguments.
	 
* ledDirection: true means that LEDs will go from left to right, false from right to left

* singleLED: true means that only one LED will be active at the time, false means that LEDs
			   will turn on one by one until they're all lighted up
	
* turnOn: true means that LEDs will be turned on, with all previous LED states being 0
			false means that all LEDs are lighted up and they turn off one by one, depending
			on second argument
	 

## Button reading

OpenDeck provides callback handler setHandleButton for deciding what to do with button presses. Sent arguments are:
`(uint8_t buttonNumber, uint8_t buttonState)`

Button state can be any of the following:

* 0 (button released)
* 1 (button pressed)
* 2 (button long-pressed)
* 3 (long-pressed button released)


## Potentiometer reading

Library provides three callback handlers for processing data from potentiometers:

* `setHandlePotCC`
* `setHandlePotNoteOn`
* `setHandlePotNoteOff`

Sent arguments for each are:
* (uint8_t potNumber, uint8_t ccValue)
* (uint8_t note)
* (uint8_t note)


# Notes

Library works only with shared columns for buttons and LEDs. It cannot control multiple matrices.
Library also assumes that when connecting multiple 4051 chips to ATmega their control pins (S0, S1 and S2)
are connected together.