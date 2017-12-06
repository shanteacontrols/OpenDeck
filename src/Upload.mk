#---------------- Programming Options (avrdude) ----------------

AVR_DUDE_PATH := ../../../tools/avrdude/
AVR_DUDE_BIN := avrdude
AVRDUDE_PROGRAMMER = usbasp

#if set to 1, uploading will be started immediately after compiling
UPLOAD := 0