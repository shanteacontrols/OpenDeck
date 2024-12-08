TARGET := arduino_ctl
BOARD  := arduino:avr:leonardo
PORT   := /dev/ttyACM0

all:
	@arduino-cli compile --fqbn $(BOARD) $(TARGET).ino

flash:
	@arduino-cli upload -p $(PORT) --fqbn $(BOARD) $(TARGET).ino
