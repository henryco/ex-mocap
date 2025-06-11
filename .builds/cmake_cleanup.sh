#!/bin/bash

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

echo "ParentDIR: $PARENT_DIR"

for file in "$SCRIPT_DIR"/profile_*; do
    if [ -f "$file" ]; then
        basename=$(basename "$file")
        profile="${basename#profile_*}"
        targetd="$PARENT_DIR/cmake-build-$profile"

        if [ -d "$targetd" ]; then
            echo "Cleanup: $targetd"
            rm -r "$targetd"
        fi
    fi
done

if [ -L "$PARENT_DIR/cmake-build" ]; then
    # ? remove symlink?
    echo "Cleanup profile symlink"
    rm "$PARENT_DIR/cmake-build"
fi
