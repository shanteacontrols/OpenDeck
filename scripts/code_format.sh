#!/usr/bin/env bash

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
project_dir=$(realpath "${script_dir}"/..)

find "$project_dir" \
-regex '.*\.\(cpp\|hpp\|h\|cc\|cxx\|c\)' \
-not -path '**build/**/*' \
-not -path '**generated/**/*' \
-not -path '**modules/**/*' \
-exec clang-format -style=file -i {} +

git diff
git diff -s --exit-code