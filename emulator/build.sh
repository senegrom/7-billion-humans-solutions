#!/usr/bin/env bash
# Build the emulator. Requires gcc (MinGW-w64 on Windows works).
set -euo pipefail
cd "$(dirname "$0")"
gcc -std=c11 -O2 -Wall -Wextra -o emu.exe emu.c
echo "built ./emu.exe"
