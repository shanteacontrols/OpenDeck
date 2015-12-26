# OpenDeck Configuration

OpenDeck board hosts large amount of configurable persistent parameters. 
All configuration options are grouped in five blocks. Each block has its own sections.
Each section has a number of parameters which are possible to edit. Each parameter can be set to new value with defined range. Within sections,
settings can be applied either to single or to all parameters. All OpenDeck configuration is done using MIDI SysEx messages. Each block, section
and parameter have their own SysEx ID, used to specifically identify wanted option.

## 1. System Exclusive messages

*Note: each byte in this section is written in hex notation.*

Each MIDI SysEx message starts with `START` (`F0`) byte and ends with `STOP` (`F7`) byte. `START` byte *always* needs to be followed by three
manufacturer ID bytes. Manufacturer bytes ensure that SysEx message doesn't end up on wrong MIDI controller. OpenDeck uses these manufacturer ID bytes:

1) `M_ID_0`: `00`

2) `M_ID_1`: `53`

3) `M_ID_2`: `43`

If manufacturer bytes aren't specified (or they're wrong), OpenDeck won't respond.

There are two types of SysEx messages on OpenDeck:

1) Special messages

2) Configuration messages

OpenDeck SysEx protocol checks all incoming messages. If a request is wrong, protocol will return error code.

## 1.1 Special SysEx messages

Special messages have the following structure:

`START M_ID_0 M_ID_1 M_ID_2 SPECIAL_MESSAGE_ID END`

There are three special SysEx messages:

1) Hello message

2) Firmware update message

3) Factory reset message

Special messages are differentiated by `SPECIAL_MESSAGE_ID` byte.

### 1.1.1 Hello message

Hello message has `48` as `SPECIAL_MESSAGE_ID`.

Before attempting to use SysEx messages to configure OpenDeck board,
a "Hello" message must be sent to board. Hello message enables SysEx configuration.

Hello message: `F0 00 53 43 48 F7`

After hello message has been sent, controller will respond with following message:
`F0 00 53 43 41 48 01 00 00 F7`

Like request, every response always has three manufacturer ID bytes after `START` byte.

`41` is `ACK` byte and will always follow after manufacturer bytes if request was correct.

After `ACK` byte, original message request is copied back into response for easier determination of message meaning. In this case, message request
was "hello", or `48` byte.

Next three bytes determine firmware version currently running on board. At the time of writing this documentation, firmware version was v1.0.0.

Last byte is always SysEx `STOP` byte.

After the board responds to hello message, user can continue with board configuration for 1 minute. If no SysEx message is sent for more
than one minute, hello message needs to be sent again. Reasoning behind this is that while in SysEx configuration mode, board sends additional components IDs which aren't necessary during the normal operation, but are needed to identify component sending MIDI input. To reduce amount of data that is being sent, SysEx config gets disabled automatically after one minute.

### 1.1.2 Firmware update message

This message has `7F` as `SPECIAL_MESSAGE_ID`. When this message is sent to board, the board will reboot into firmware update mode, showing on PC as "OPENDECK" FAT32 removeable drive. The board doesn't send response to this message.

Firmware update message: `F0 00 53 43 7F F7`

### 1.1.2 Factory reset message

This message has `44` as `SPECIAL_MESSAGE_ID`. When this message is sent to board, the board will restore all configurable parameters back to default value. After this process is complete, board will reboot. The board doesn't send response to this message.

Factory reset message: `F0 00 53 43 44 F7`

## 1.2 Configuration messages

Configuration messages have the following structure:

`START M_ID_0 M_ID_1 M_ID_1 WISH AMOUNT BLOCK SECTION PARAMETER* NEW_PARAMETER* STOP`

`WISH` determines what the user wants with selected parameter (get, set or restore to default value).
`AMOUNT` determines how many parameters user wants to configure (single or all).
`BLOCK` and `SECTION` determine parameter category.
`PARAMETER` is the component ID that user wants to manipulate.
`NEW_PARAMETER` is the new value that user wants to set to selected parameter.

Items marked with asterisk aren't needed in certain scenarios.

### 1.2.1 `WISH` byte

`WISH` byte comes after three manufacturer ID bytes. There are three options available:

1) `GET`

2) `SET`

3) `RESTORE`

#### 1.2.1.1 `GET`

SysEx `WISH` ID: `0`

Used to retrieve one or more parameters from the board.

#### 1.2.1.2 `SET`

SysEx `WISH` ID: `1`

Used to set one or more parameters to new value.

#### 1.2.1.3 `RESTORE`

SysEx `WISH` ID: `2`

Used to restore one or more parameter to its default state.

### 1.2.2 `AMOUNT` byte
After `GET`, `SET` or `RESTORE` command, `AMOUNT` byte is needed. `AMOUNT` determines amount of parameters user wants to manipulate. There are two choices for `AMOUNT` byte:

1) `SINGLE`

2) `ALL`

#### 1.2.2.1 `SINGLE`

SysEx `AMOUNT` ID: `0`

Used to get, set or restore single parameter within block section.

#### 1.2.2.2 `ALL`

SysEx `AMOUNT` ID: `1`

Used to get, set or restore all parameters within block section.

### 1.2.3 `BLOCK`, `SECTION`, `PARAMETER` and `NEW_PARAMETER` bytes

`BLOCK` determines which part of the board user wants to configure. `BLOCK` byte comes after `AMOUNT` byte. There are five options available:

1) MIDI

2) Button

3) Encoder

4) Analog

5) LED

Each block has its own sections for precise determination of parameter user wants to configure.
Section is determined with `SECTION` byte.
Inside section, `PARAMETER` byte must be specified to determine parameter user wants to manipulate.
If `WISH` byte is `SET`, `NEW_PARAMETER` must be specified to set new value to selected parameter.

#### 1.2.3.1 MIDI block

SysEx `BLOCK` ID: `0`

MIDI block has the following sections:

1) Enable/disable features

2) Channel configuration

##### 1.2.3.1.1 MIDI features

SysEx `SECTION` ID: `0`

By default, all MIDI features are disabled.  Accepted values for `NEW_PARAMETER` byte are `1` (enabled) and `0` (disabled). These are configurable MIDI features (parameters):

###### 1.2.3.1.1.1 Standard note off

SysEx `PARAMETER` ID: `0`

When disabled, Note On with velocity 0 will be sent as note off. If enabled, true Note Off event will be sent instead.

###### 1.2.3.1.1.2 Running status

SysEx `PARAMETER` ID: `1`

This setting applies only to DIN MIDI out. This setting can cause issues on older MIDI gear so it's best to leave it disabled.

###### 1.2.3.1.1.3 DIN MIDI in to USB MIDI out

SysEx `PARAMETER` ID: `2`

When enabled, all data received via DIN MIDI in port will be translated to USB MIDI.

##### 1.2.3.1.2 MIDI channels

SysEx `SECTION` ID: `1`

There are several configurable MIDI channels (parameters):

1) Note channel

2) Program change channel

3) Continuous change channel

4) Input channel

All channels are set to 1 by default. Accepted values for `NEW_PARAMETER` byte range between 1-16.
###### 1.2.3.1.2.1 Note channel

SysEx `PARAMETER` ID: `0`

Used by buttons and analog inputs configured as FSR.

###### 1.2.3.1.2.2 Program change channel

SysEx `PARAMETER` ID: `1`

Used by buttons configured to send program change MIDI event instead of notes.

###### 1.2.3.1.2.3 Continuous change channel

SysEx `PARAMETER` ID: `2`

Used by analog inputs configured as potentiometers and encoders.

###### 1.2.3.1.2.4 Input channel

SysEx `PARAMETER` ID: `3`

Listening channel on which board receives MIDI messages.

#### 1.2.3.2 Button configuration block

SysEx `BLOCK` ID: `1`

This block has the following subsections:

1) Type

2) Program change state

3) MIDI ID

##### 1.2.3.2.1 Type

SysEx `SECTION` ID: `0`

Denotes button type. Type can be momentary, which means that note off is sent as soon as button is released, or latching, which means that note off is sent on second button press. All buttons are configured as momentary by default. Acceptable values for `NEW_PARAMETER` byte are `0` (momentary) and `1` (latching). `PARAMETER` byte can range between 0-63 (total of 64 values for 64 buttons).

##### 1.2.3.2.2 Program change state

SysEx `SECTION` ID: `1`

When enabled, instead of notes, button will send program change event. This option is disabled by default for all buttons.  Acceptable values for `NEW_PARAMETER` byte are `0` (disabled) and `1` (enabled). `PARAMETER` byte can range between 0-63 (total of 64 values for 64 buttons).

##### 1.2.3.2.3 MIDI ID

SysEx `SECTION` ID: `2`

Denotes note/program change MIDI number. Default value is button number (button 0 has default note 0, button 1 has default note 1 etc.).  Acceptable values for `NEW_PARAMETER` byte range between 0-127. `PARAMETER` byte can range between 0-63 (total of 64 values for 64 buttons).

#### 1.2.3.3 Encoder configuration block

SysEx `BLOCK` ID: `2`

This block has the following subsections:

1) Enabled/disabled

2) Invert state

3) Encoding mode

##### 1.2.3.3.1 Enabled/disabled

SysEx `SECTION` ID: `0`

Enables or disables encoder. All encoders are disabled by default. Acceptable values for `NEW_PARAMETER` byte are `0` (disabled) and `1` (enabled). `PARAMETER` byte can range between 0-31 (total of 32 values for 32 encoders).

##### 1.2.3.3.2 Invert state

SysEx `SECTION` ID: `1`

When enabled, encoder will send inverted MIDI CC messages in different directions. Default option is disabled for all encoders. Acceptable values for `NEW_PARAMETER` byte are `0` (not inverted) and `1` (inverted). `PARAMETER` byte can range between 0-31 (total of 32 values for 32 encoders).

##### 1.2.3.3.3 Encoding mode

SysEx `SECTION` ID: `2`

Denotes encoder encoding mode. There two options:

1) 7Fh01h encoding

2) 3Fh41h encoding

Default option is 7Fh01h for all encoders. Acceptable values for `NEW_PARAMETER` byte are `0` (7Fh01h) and `1` (3Fh41h). `PARAMETER` byte can range between 0-31 (total of 32 values for 32 encoders).

###### 1.2.3.3.3.1 7Fh01h encoding
When this mode is active, encoder will send CC message with value 127 in one direction, and value 1 in other direction.

###### 1.2.3.3.3.2 3Fh41h encoding

When this mode is active, encoder will send CC message with value 63 in one direction, and value 65 in other direction.

##### 1.2.3.3.4 MIDI ID

SysEx `SECTION` ID: `3`

Denotes the MIDI CC number for each encoder. Default value is same as encoder ID.  Acceptable values for `NEW_PARAMETER` byte range between 0-127. `PARAMETER` byte can range between 0-31 (total of 32 values for 32 encoders).

#### 1.2.3.4 Analog configuration block

SysEx `BLOCK` ID: `3`

This block has the following subsections:

1) Enabled/disabled

2) Invert state

3) Type

4) MIDI ID

5) Lower CC limit

6) Upper CC limit

##### 1.2.3.4.1 Enabled/disabled

SysEx `SECTION` ID: `0`

Enables or disables analog input. By default, all analog inputs are disabled. Acceptable values for `NEW_PARAMETER` byte are `0` (disabled) and `1` (enabled). `PARAMETER` byte can range between 0-31 (total of 32 values for 32 analog inputs).

##### 1.2.3.4.2 Invert

SysEx `SECTION` ID: `1`

When enabled, analog input will send inverted MIDI CC messages in each direction. By default, this option is disabled for all analog inputs. Acceptable values for `NEW_PARAMETER` byte are `0` (not inverted) and `1` (inverted). `PARAMETER` byte can range between 0-31 (total of 32 values for 32 analog inputs).

##### 1.2.3.4.3 Type

SysEx `SECTION` ID: `2`

Analog inputs can behave differently based on selected type. There are several analog types:

1) Potentiometer

2) FSR

Default option is potentiometer for all analog inputs. Acceptable values for `NEW_PARAMETER` byte are `0` (potentiometer) and `1` (FSR). `PARAMETER` byte can range between 0-31 (total of 32 values for 32 analog inputs).

###### 1.2.3.4.3.1 Potentiometer

SysEx `PARAMETER` ID: `0`

Most common analog input. Sends CC values in range 0-127 based on position.

###### 1.2.3.4.3.1 FSR

SysEx `PARAMETER` ID: `1`

Force-sensitive resistor. Sends MIDI note with velocity. Velocity depends on applied pressure.

##### 1.2.3.4.4 MIDI ID

SysEx `SECTION` ID: `3`

Denotes MIDI CC number. Default value is analog ID. Acceptable values for `NEW_PARAMETER` byte range between 0-127. `PARAMETER` byte can range between 0-31 (total of 32 values for 32 analog inputs).

##### 1.2.3.4.5 Lower CC limit

SysEx `SECTION` ID: `4`

Denotes minimum CC value. Default value is 0. If any other value is specified, CC range gets scaled. Acceptable values for `NEW_PARAMETER` byte range between 0-127. `PARAMETER` byte can range between 0-31 (total of 32 values for 32 analog inputs).

##### 1.2.3.4.6 Upper CC limit

SysEx `SECTION` ID: `5`

Denotes maximum CC value. Default value is 127. If any other value is specified, CC range gets scaled. Acceptable values for `NEW_PARAMETER` byte range between 0-127. `PARAMETER` byte can range between 0-31 (total of 32 values for 32 analog inputs).

#### 1.2.3.5 LED configuration block

SysEx `BLOCK` ID: `4`

This block has the following sections:

1) Hardware parameters

2) Activation note

3) Start-up number

4) LED state test

##### 1.2.3.5.1 Hardware parameters

SysEx `SECTION` ID: `0`

LED block has several hardware parameters which can be configured.

1) Total LED number

2) Blink time

3) Start-up switch time

4) Start-up routine

5) Fade time

###### 1.2.3.5.1.1 Total LED number

SysEx `PARAMETER` ID: `0`

Sets total amount of LEDs. Doesn't need to be specified if start-up animation isn't used. Acceptable values for `NEW_PARAMETER` byte range between 0-47 (board supports 48 LEDs).

###### 1.2.3.5.1.2 Blink time

SysEx `PARAMETER` ID: `1`

Sets amount of time LED is on/off. Specified value gets multiplied by 100, ie. if user sets blink time to 1, actual blink time is 100mS. Default value is 0, which means LED blinking is disabled.

###### 1.2.3.5.1.3 Start-up switch time

SysEx `PARAMETER` ID: `2`

Denotes the speed at which LEDs are turning on or off one by one in start-up animation. Specified value gets multiplied by 10, ie. if specified value is 10, LEDs get switched at 100mS rate. Must be specified if start-up routine is used.

###### 1.2.3.5.1.4 Start-up routine

Start-up animation. There are 5 animations available. Default routine is 0, which means that LED animation is disabled. Maximum value for `NEW_PARAMETER` byte is 5 (there are 5 defined animations).

###### 1.2.3.5.1.5 Fade time

SysEx `PARAMETER` ID: `3`

Speed at which LEDs transition between on and off state (PWM). Default value is 0, which means fading is disabled by default.

##### 1.2.3.5.2 Activation note

SysEx `SECTION` ID: `1`

Sets the note which turns the LED on. By default, activation note is "blank", which means it has to be specified by user. Acceptable values for `NEW_PARAMETER` byte range between 0-127. `PARAMETER` byte can range between 0-47 (total of 48 values for 48 LEDs). LED state depends on received velocity. If LED blinking is disabled, that is, if LED blink time hardware parameter is set to 0, there are two possible LED states:

* LED on (velocity > 0)
* LED off (velocity 0)

If LED blinking is enabled, LED can be in both states, depending on velocity:

* LED constant state off (velocity 0)
* LED constant state on (velocity 1-62)
* LED blink state off (velocity 63)
* LED blink state on (velocity 64-127)

If two modes are active at the same time, both need to be turned off to completely turn off the LED. LEDs "remember" their state. Example scenario:

1) User sends velocity 62 to LED, LED is in constant on state

2) User sends velocity 127 to same LED, LED starts blinking

3) User sends velocity 62 to same LED, LED stops blinking and returns to constant on state

4) User sends velocity 0 to same LED, LED is now off

##### 1.2.3.5.3 Start-up number

SysEx `SECTION` ID: `2`

Start-up animation switches the LEDs one-by-one in user-defined order. Change LED order with this setting. Acceptable values for `NEW_PARAMETER` byte range between 0-47. `PARAMETER` byte can range between 0-47 (total of 48 values for 48 LEDs).

##### 1.2.3.5.4 LED state testing

SysEx `SECTION` ID: `3`

This setting doesn't write anything to the board. Used only for testing LED states. `NEW_PARAMETER` (state) can have four values:

1) `0` - turn constant state off

2) `1` - turn constant state on

3) `2` - turn blink state off

4) `3` - turn blink state on

`PARAMETER` byte can range between 0-47 (total of 48 values for 48 LEDs).

## 2. Configuration examples

### 2.1 `GET` command

*Note: All `GET` examples assume default board configuration.*

Example #1:
User wants to find out MIDI CC number for analog component 5.

Since only one component info is needed, `AMOUNT` byte is set to `SINGLE`.

* Request:
`F0 00 53 43 00 00 03 03 05 F7`
* Response:
`F0 00 53 43 41 05 F7`

Response always has three manufacturer IDs followed by `ACK` byte. When `WISH` is `GET`, wanted parameters are listed after `ACK` byte. In this case, MIDI CC number for analog component 5 is 5, therefore, last byte is 5.

Example #2:
User wants to find out encoding mode for all encoders.

* Request:
`F0 00 53 43 00 01 02 02 F7`
* Response:
`F0 00 53 43 41 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F7`

Response returned 32 values for 32 encoders. Since all encoders have default encoding mode set to 7Fh01h (`0`), all 32  values are `0`.

### 2.2 `SET` command

Example 1:
User wants to configure button 6 to send program change event instead of note event.

* Request:
`F0 00 53 43 01 00 01 01 06 01 F7`
* Response:
`F0 00 53 43 41 F7`

When `WISH` is `SET`, response contains only manufacturer ID bytes and `ACK` byte.

Example 2: User wants to set all MIDI channels to channel 5.

* Request:
`F0 00 53 43 01 01 00 01 05 05 05 05 F7`
* Response:
`F0 00 53 43 41 F7`

When `WISH` byte is `SET`, and `AMOUNT` is `ALL`, all parameters within section must be specified. MIDI channel section has four channels, therefore four channels are listed after `SECTION` byte.

### 2.2 `RESTORE` command

Example 1:
User wants to restore Program change channel back to default.

* Request:
`F0 00 53 43 02 00 00 01 01 F7`
* Response:
`F0 00 53 43 41 F7`

Example 2:
User wants to restore restore all button program change states back to default (disable):

* Request:
`F0 00 53 43 02 01 01 01 F7`
* Response:
`F0 00 53 43 41 F7`

## 1.3 SysEx errors

If user types wrong byte in message request, the board will return specific error code based on what the user did wrong. There are 9 possible errors that can happen while sending SysEx request:

1) Handshake error

2) `WISH` error

3) `AMOUNT` error

4) `BLOCK` error

5) `SECTION` error

6) `PARAMETER` error

7) `NEW_PARAMETER` error

8) Message length error

9) EEPROM error

Error message structure:

`START M_ID_0 M_ID_1 M_ID_2 ERROR_CODE STOP`

### 1.3.1 Handshake error

Error code: `0`

This error is returned when request is correct, but hello message hasn't been sent to board.

Message: `F0 00 53 43 46 00 F7`

### 1.3.1 `WISH` error

Error code: `1`

This error is returned when `WISH` is anything other than `GET`, `SET` or `RESTORE`.

Message: `F0 00 53 43 46 01 F7`

### 1.3.1 `AMOUNT` error

Error code: `2`

This error is returned when `AMOUNT` is anything other than `SINGLE` or `ALL`.

Message: `F0 00 53 43 46 02 F7`

### 1.3.1 `BLOCK` error

Error code: `3`

This error is returned when `BLOCK` byte is incorrect.

Message: `F0 00 53 43 46 03 F7`

### 1.3.1 `SECTION` error

Error code: `4`

This error is returned when `SECTION` byte is incorrect.

Message: `F0 00 53 43 46 04 F7`

### 1.3.1 `PARAMETER` error

Error code: `5`

This error is returned when wanted parameter is incorrect.

Message: `F0 00 53 43 46 05 F7`

### 1.3.1 `NEW_PARAMETER` error

Error code: `6`

This error is returned when `NEW_PARAMETER` is incorrect.

Message: `F0 00 53 43 46 06 F7`

### 1.3.1 Message length error

Error code: `7`

This error is returned when request is too short.

Message: `F0 00 53 43 46 07 F7`

### 1.3.1 EEPROM error

Error code: `8`

This error is returned when writing new value to board has failed. If this occurs your board is damaged.

Message: `F0 00 53 43 46 08 F7`