#!/bin/bash

run_dir="webui"

if [[ $(basename "`pwd`") != $run_dir ]]
then
    echo This script must be run from $run_dir directory!
    exit 1
fi
make clean && make PLATFORM=linux && mv build/OpenDeckConfigurator-linux-x64.zip ../OpenDeckConfigurator-linux-x64.zip
make clean && make PLATFORM=win32 && mv build/OpenDeckConfigurator-win32-x64.zip ../OpenDeckConfigurator-win32-x64.zip
make clean && make PLATFORM=darwin && mv build/OpenDeckConfigurator-darwin-x64.zip ../OpenDeckConfigurator-darwin-x64.zip
