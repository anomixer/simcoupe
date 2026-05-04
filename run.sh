#!/bin/bash
# SimCoupe WASM Run Script for Linux/macOS
echo "Starting local web server at http://localhost:8000"
python3 -m http.server --directory wasm/deploy
