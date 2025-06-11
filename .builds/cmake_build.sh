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

if [ -L "$PARENT_DIR/cmake-build" ]; then
    echo "Symlink to a profile exists, skip"
    exit 0
fi

profile=""
for f in "$SCRIPT_DIR"/profile_*; do
    if [ -f "$f" ]; then
        profb=$(basename "$f")
        profile="${basename#profile_*}"
        break
    fi
done

if [ -n "$profile" ]; then
    echo "Default cmake profile: $profile"
    ln -sfn "$SCRIPT_DIR/profile_$profile" "$PARENT_DIR/.clangd"
    ln -sfn "$PARENT_DIR/cmake-build-$profile" "$PARENT_DIR/cmake-build"
else
    echo "No cmake profiles found"
fi
