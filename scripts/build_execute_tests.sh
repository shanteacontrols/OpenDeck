#!/bin/bash

#exit on error
set -e

make pre-build

#tests
echo "Running tests for OpenDeck board"
make clean && make all BOARD_TYPE=opendeck && make exec
echo "Running tests for Leonardo/Pro Micro board"
make clean && make all BOARD_TYPE=leonardo && make exec
echo "Running tests for Uno board"
make clean && make all BOARD_TYPE=uno && make exec
echo "Running tests for Mega board"
make clean && make all BOARD_TYPE=mega && make exec
echo "Running tests for Mega6mux board"
make clean && make all BOARD_TYPE=mega6mux && make exec
echo "Running tests for Teensy++ 2.0 board"
make clean && make all BOARD_TYPE=teensy2pp && make exec
echo "Running tests for Tannin board"
make clean && make all BOARD_TYPE=tannin && make exec
echo "Running tests for Bergamot board"
make clean && make all BOARD_TYPE=bergamot && make exec
echo "Running tests for Kodama board"
make clean && make all BOARD_TYPE=kodama && make exec
