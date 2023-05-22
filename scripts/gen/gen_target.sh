#!/usr/bin/env bash

for arg in "$@"; do
    case "$arg" in
        --project=*)
            project=${arg#--project=}
            ;;

        --target-config=*)
            yaml_file=${arg#--target-config=}
            ;;

        --gen-dir-target=*)
            gen_dir=${arg#--gen-dir-target=}
            ;;

        --touchscreen-config=*)
            touchscreen_config=${arg#--touchscreen-config=}
            ;;

        --gen-dir-touchscreen=*)
            gen_dir_touchscreen=${arg#--gen-dir-touchscreen=}
            ;;

        --touchscreen-img-dir=*)
            touchscreen_img_dir=${arg#--touchscreen-img-dir=}
            ;;

        --base-gen-dir-mcu=*)
            base_mcu_gen_dir=${arg#--base-gen-dir-mcu=}
            ;;
    esac
done

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
yaml_parser="dasel -n -p yaml --plain -f"
out_header="$gen_dir"/Target.h
out_makefile="$gen_dir"/Makefile
mcu=$($yaml_parser "$yaml_file" mcu)
target_name=$(basename "$yaml_file" .yml)
mcu_gen_dir=$base_mcu_gen_dir/$mcu
hw_test_yaml_file=$(dirname "$yaml_file")/../hw-test/$target_name.yml
extClockMhz=$($yaml_parser "$yaml_file" extClockMhz)

if [[ ! -d $mcu_gen_dir ]]
then
    if ! "$script_dir"/gen_mcu.sh "$mcu" "$mcu_gen_dir" "$extClockMhz"
    then
        exit 1
    fi
fi

if [[ ! -d $gen_dir ]]
then
    echo "Generating target definitions..."

    mkdir -p "$gen_dir"
    echo "" > "$out_header"
    echo "" > "$out_makefile"

    source "$script_dir"/target/main.sh

    if [[ -f $hw_test_yaml_file ]]
    then
        # HW config files go into the same dir as target ones
        if ! "$script_dir"/gen_hwconfig.sh "$project" "$hw_test_yaml_file" "$gen_dir"
        then
            exit 1
        else
            printf "%s\n" "PROJECT_TARGET_DEFINES += PROJECT_TARGET_SUPPORT_HW_TESTS" >> "$out_makefile"
        fi
    fi
fi

if [[ -f $touchscreen_config ]]
then
    "$script_dir"/gen_touchscreen.sh \
    --config="$touchscreen_config" \
    --gen-dir="$gen_dir_touchscreen"\
    --img-dir="$touchscreen_img_dir"
fi