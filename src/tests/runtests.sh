#!/bin/bash

set -e
for file in build/*.out; do $file 2>/dev/null; done
