#!/bin/bash

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

echo "ParentDIR: $PARENT_DIR"

# shopt -s nullglob
for file in "$SCRIPT_DIR"/profile_*; do
    if [ -f "$file" ]; then
        basename=$(basename "$file")
        profile="${basename#profile_*}"

        echo "Update cmake profile: $profile"

        cmake -S "$PARENT_DIR" -B "$PARENT_DIR/cmake-build-$profile" -G Ninja || exit 3
    fi
done
# shopt -u nullglob
