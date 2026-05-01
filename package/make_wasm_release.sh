#!/bin/bash
set -e

# SimCoupe WASM Build Script for Linux/Action
# Developer: Antigravity

# 1. Setup Directories
BUILD_DIR="wasm-build-linux"
DEPLOY_DIR="wasm/deploy"
mkdir -p $BUILD_DIR

# 2. Configure and Build
echo "Configuring with Emscripten..."
emcmake cmake -B $BUILD_DIR -S wasm -DCMAKE_BUILD_TYPE=Release

echo "Building SimCoupe..."
cmake --build $BUILD_DIR --config Release -j$(nproc)

# 3. Deploy artifacts
echo "Deploying artifacts to $DEPLOY_DIR..."
mkdir -p $DEPLOY_DIR
cp $BUILD_DIR/simcoupe.js $DEPLOY_DIR/
cp $BUILD_DIR/simcoupe.wasm $DEPLOY_DIR/
cp $BUILD_DIR/simcoupe.data $DEPLOY_DIR/

echo "Build Success!"
