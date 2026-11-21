#!/bin/bash
set -e

echo "=== Building DRMTRGRR (VST3 / AU / Standalone) ==="

BUILD_DIR="build"
CONFIG="Release"

cmake -B "$BUILD_DIR" -G "Ninja" -DCMAKE_BUILD_TYPE="$CONFIG"
cmake --build "$BUILD_DIR" --config "$CONFIG" --parallel

echo ""
echo "=== Build Successful! ==="
echo "VST3 Location:"
echo "  $(pwd)/build/DRMTRGRR_artefacts/$CONFIG/VST3/DRMTRGRR.vst3"
echo "Standalone Location:"
echo "  $(pwd)/build/DRMTRGRR_artefacts/$CONFIG/Standalone/DRMTRGRR.app"
if [ -d "build/DRMTRGRR_artefacts/$CONFIG/AU/DRMTRGRR.component" ]; then
    echo "AudioUnit (AU) Location:"
    echo "  $(pwd)/build/DRMTRGRR_artefacts/$CONFIG/AU/DRMTRGRR.component"
fi
