#!/usr/bin/env bash

# J-Link through nrfjprog tool

set -e

nrfjprog -f nrf52 --program "$PWD"/merged.hex --sectorerase
nrfjprog -f nrf52 --reset