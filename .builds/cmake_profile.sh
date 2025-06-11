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

if [ ! -d "$PARENT_DIR/cmake-build-$PROFILE" ]; then
    echo "Cannot locate profile build dir"
    bash "$SCRIPT_DIR/cmake_build.sh"
fi

ln -sfn "$SCRIPT_DIR/profile_$PROFILE" "$PARENT_DIR/.clangd"
ln -sfn "$PARENT_DIR/cmake-build-$PROFILE" "$PARENT_DIR/cmake-build"

echo "Switched to $PROFILE profile"
