@echo off
REM SimCoupe WASM Run Script for Windows
echo Starting local web server at http://localhost:8000
python -m http.server --directory wasm/deploy
