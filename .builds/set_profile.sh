#!/bin/sh
PROFILE=$1

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

if [ -z "$PROFILE" ]; then
    echo "Usage: $0 [debug|release|valgrind]"
    exit 1
fi

if [ ! -f "$SCRIPT_DIR/profile_$PROFILE" ]; then
    echo "Profile $PROFILE not found, cant locate $SCRIPT_DIR/profile_$PROFILE"
    exit 1
fi

echo "ParentDIR: $PARENT_DIR"

cmake -S "$PARENT_DIR" -B "cmake-build-$PROFILE" -G Ninja || exit 3

ln -sfn "$SCRIPT_DIR/profile_$PROFILE" "$PARENT_DIR/.clangd"
ln -sfn "$PARENT_DIR/cmake-build-$PROFILE" "$PARENT_DIR/cmake-build"

echo "Switched to $PROFILE profile"
