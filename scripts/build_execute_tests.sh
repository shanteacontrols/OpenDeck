#!/bin/bash

#exit on error
set -e

make pre-build

#tests
echo "Running tests for OpenDeck board"
make clean && make all TARGETNAME=fw_opendeck && make exec
echo "Running tests for Leonardo/Pro Micro board"
make clean && make all TARGETNAME=fw_leonardo && make exec
echo "Running tests for Uno board"
make clean && make all TARGETNAME=fw_uno && make exec
echo "Running tests for Mega board"
make clean && make all TARGETNAME=fw_mega && make exec
echo "Running tests for Mega6mux board"
make clean && make all TARGETNAME=fw_mega6mux && make exec
echo "Running tests for Teensy++ 2.0 board"
make clean && make all TARGETNAME=fw_teensy2pp && make exec
echo "Running tests for Bergamot board"
make clean && make all TARGETNAME=fw_bergamot && make exec
echo "Running tests for Kodama board"
make clean && make all TARGETNAME=fw_kodama && make exec
