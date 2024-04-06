#!/usr/bin/env bash

# avrdude with Arduino as an programmer configuration

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project_root="$(realpath "${script_dir}"/../..)"
yaml_parser="dasel -n -p yaml --plain -f"
target=$1
target_config=$project_root/config/target/$target.yml
mcu=$($yaml_parser "$target_config" mcu)
mcu_config=$project_root/config/mcu/$mcu.yml

fuse_unlock=$($yaml_parser "$mcu_config" fuses.unlock)
fuse_lock=$($yaml_parser "$mcu_config" fuses.lock)
fuse_ext=$($yaml_parser "$mcu_config" fuses.ext)
fuse_high=$($yaml_parser "$mcu_config" fuses.high)
fuse_low=$($yaml_parser "$mcu_config" fuses.low)

sudo avrdude -p "$mcu" -P /dev/"${PORT}" -b 19200 -c avrisp -C /etc/avrdude.conf -e -V -u -U lock:w:"$fuse_unlock":m -U efuse:w:"$fuse_ext":m -U hfuse:w:"$fuse_high":m -U lfuse:w:"$fuse_low":m
sudo avrdude -p "$mcu" -P /dev/"${PORT}" -b 19200 -c avrisp -C /etc/avrdude.conf -U flash:w:"$PWD"/merged.hex
sudo avrdude -p "$mcu" -P /dev/"${PORT}" -b 19200 -c avrisp -C /etc/avrdude.conf -V -u -U lock:w:"$fuse_lock":m