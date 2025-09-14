#!/usr/bin/env bash
set -e  # bail on first error

# Clean + configure with compile_commands.json export
cmake -E rm -rf build
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

# Symlink (or copy if symlinks not supported on Windows Git Bash)
if [[ -f build/compile_commands.json ]]; then
    ln -sf build/compile_commands.json .
    echo "[✔] compile_commands.json linked to project root"
else
    echo "[✘] compile_commands.json not found, something went wrong."
fi

