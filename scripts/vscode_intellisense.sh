#!/bin/bash

#note: must be run in src in order to work

#get compile_commands.json for firmware first
make clean
compiledb -d . make TARGET=$1

#next, generate compile_commands.json for tests
cd tests
make clean-all
compiledb -d . make TARGET=$1
#replace current directory with "tests"
sed -i 's#"."#"tests"#g' compile_commands.json
#replace previous directory with current one
sed -i 's#\.\./##g' compile_commands.json

#go back to src dir and merge two json files
cd ../
jq -s '.[0] + .[1]' compile_commands.json tests/compile_commands.json > merged.json
rm compile_commands.json
rm tests/compile_commands.json
mv merged.json compile_commands.json
