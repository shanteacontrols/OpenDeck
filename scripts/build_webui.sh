#!/bin/bash

# note: must be run from webui directory

make clean && make PLATFORM=linux && mv build/OpenDeckConfigurator-linux-x64.zip ../OpenDeckConfigurator-linux-x64.zip
make clean && make PLATFORM=win32 && mv build/OpenDeckConfigurator-win32-x64.zip ../OpenDeckConfigurator-win32-x64.zip
make clean && make PLATFORM=darwin && mv build/OpenDeckConfigurator-darwin-x64.zip ../OpenDeckConfigurator-darwin-x64.zip
