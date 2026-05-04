#!/bin/bash
# SimCoupe WASM build of SimCoupe using Emscripten (Linux/macOS version)
set -e

BUILD_DIR="build-wasm"
REPO_ROOT=$(pwd)

# 1. Check for basic tools
for cmd in cmake curl unzip git; do
    if ! command -v $cmd &> /dev/null; then
        echo "Error: $cmd is not installed."
        exit 1
    fi
done

# 2. Initialize Emscripten
EMSDK_PATH="$REPO_ROOT/emsdk-wasm"
if [ ! -d "$EMSDK_PATH" ]; then
    echo "EMSDK not found. Installing into repo: $EMSDK_PATH"
    git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_PATH"
    cd "$EMSDK_PATH"
    ./emsdk install latest
    ./emsdk activate latest
    cd "$REPO_ROOT"
fi

# Load Emscripten environment
source "$EMSDK_PATH/emsdk_env.sh"

# 3. Prepare Build Directory and Dependencies
mkdir -p "$BUILD_DIR/deps"
mkdir -p "$BUILD_DIR/_deps"

declare -A DEPS=(
    ["z80"]="https://github.com/kosarev/z80/archive/9917a37.zip"
    ["fmt"]="https://github.com/fmtlib/fmt/archive/e69e5f97.zip"
    ["resid"]="https://github.com/simonowen/resid/archive/9ac2b4b.zip"
    ["spectrum"]="https://github.com/simonowen/libspectrum/archive/eb93bd3.zip"
    ["saasound"]="https://github.com/simonowen/SAASound/archive/7ceef2a.zip"
)

for dep in "${!DEPS[@]}"; do
    DEST="$BUILD_DIR/deps/$dep.zip"
    EXTRACT_TO="$BUILD_DIR/_deps/$dep-src"
    
    if [ ! -f "$DEST" ] || [ ! -s "$DEST" ]; then
        echo "Downloading $dep..."
        curl -L "${DEPS[$dep]}" -o "$DEST"
    fi
    
    if [ ! -d "$EXTRACT_TO" ]; then
        echo "Extracting $dep..."
        TEMP_EXTRACT="$BUILD_DIR/_deps/temp_$dep"
        mkdir -p "$TEMP_EXTRACT"
        unzip -q "$DEST" -d "$TEMP_EXTRACT"
        ROOT_FOLDER=$(ls -d "$TEMP_EXTRACT"/*/)
        mv "$ROOT_FOLDER"* "$EXTRACT_TO" 2>/dev/null || mv "$ROOT_FOLDER" "$EXTRACT_TO"
        rm -rf "$TEMP_EXTRACT"
        
        # Cleanup and Patch
        rm -rf "$EXTRACT_TO/tests" "$EXTRACT_TO/examples" "$EXTRACT_TO/test" "$EXTRACT_TO/Example"
        
        DEP_CMAKE="$EXTRACT_TO/CMakeLists.txt"
        if [ -f "$DEP_CMAKE" ]; then
            # Remove test/example subdirectories from CMake
            sed -i 's/add_subdirectory.*\(tests\|examples\|test\|Example\)//g' "$DEP_CMAKE" || true
            
            if [ "$dep" == "z80" ]; then
                if ! grep -q "add_library(z80 INTERFACE)" "$DEP_CMAKE"; then
                    echo -e "\nadd_library(z80 INTERFACE)\ntarget_include_directories(z80 INTERFACE .)" >> "$DEP_CMAKE"
                fi
            fi
        fi
    fi
done

# 4. Build
cd "$BUILD_DIR"
echo "Configuring with emcmake..."
emcmake cmake ../wasm -DCMAKE_BUILD_TYPE=Release -DBUILD_BACKEND=sdl \
    -DCMAKE_C_FLAGS="-Dstrnicmp=strncasecmp -D_stricmp=strcasecmp"

echo "Building..."
cmake --build .

# 5. Copy artifacts to wasm/deploy
echo "Copying artifacts to wasm/deploy..."
DEPLOY_DIR="$REPO_ROOT/wasm/deploy"
mkdir -p "$DEPLOY_DIR"

ARTIFACTS=("simcoupe.wasm" "simcoupe.js" "simcoupe.data" "simcoupe.wasm.map")
for art in "${ARTIFACTS[@]}"; do
    if [ -f "$art" ]; then
        echo "Copying $art..."
        cp "$art" "$DEPLOY_DIR/"
    fi
done

echo "WASM build completed successfully!"
