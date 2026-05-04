#!/bin/bash
# SimCoupe WASM Build Entry Point for Linux/macOS
# This script calls the full release build logic in package/make_wasm_release.sh

# Ensure script is run from project root
cd "$(dirname "$0")"

if [ -f "./package/make_wasm_release.sh" ]; then
    chmod +x ./package/make_wasm_release.sh
    ./package/make_wasm_release.sh
else
    echo "Error: ./package/make_wasm_release.sh not found!"
    exit 1
fi
