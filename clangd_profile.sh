#!/bin/sh
PROFILE=$1

if [ -z "$PROFILE" ]; then
    echo "Usage: $0 [debug|release|valgrind]"
    exit 1
fi

# Create the build directory if it doesn't exist
if [ ! -d "cmake-build-$PROFILE" ]; then
    echo "Creating build directory '$PROFILE'..."
    cmake -S . -B "cmake-build-$PROFILE" -G Ninja || exit 3
else
    echo "Build directory 'cmake-build-$PROFILE' already exists."
fi

ln -sf ".clangd-$PROFILE" .clangd
ln -sfn "cmake-build-$PROFILE" cmake-build
echo "Switched to $PROFILE profile"
