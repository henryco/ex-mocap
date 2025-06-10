#!/bin/sh
PROFILE=$1

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

echo "ParentDIR: $PARENT_DIR"

# UPDATE ALL BUILDS
shopt -s nullglob
for file in "$SCRIPT_DIR"/profile_*; do
    if [ -f "$file" ]; then
        basename=$(basename "$file")
        profile="${basename#profile_*}"

        echo "Update cmake profile: $profile"

        cmake -S "$PARENT_DIR" -B "$PARENT_DIR/cmake-build-$profile" -G Ninja || exit 3
    fi
done
shopt -u nullglob


if [ ! -z "$PROFILE" ]; then
    echo "Proile not specified, using current active one"
    if [ ! -f "$PARENT_DIR/cmake-build" ]; then
        echo "Cannot locate file $PARENT_DIR/cmake-build"
        exit 1
    fi
    ninja -C "$PARENT_DIR/cmake-build"
    exit 0
fi

CURRENT_ONE="$PARENT_DIR/cmake-build-$PROFILE"
if [ ! -f "$CURRENT_ONE" ]; then
    echo "Cannot locate file $CURRENT_ONE"
    exit 1
fi

ninja -C "$CURRENT_ONE"
