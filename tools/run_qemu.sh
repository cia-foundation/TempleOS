#!/bin/bash
# Launch TempleOS in QEMU
# Usage: ./run_qemu.sh [path/to/TempleOS.iso]

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(dirname "$SCRIPT_DIR")"
ISO="${1:-$REPO_DIR/TempleOS.iso}"

if [ ! -f "$ISO" ]; then
    echo "ISO not found: $ISO"
    echo "Build it first: python3 $SCRIPT_DIR/build_iso.py"
    exit 1
fi

# Try HVF acceleration on macOS, fall back to TCG
if qemu-system-x86_64 -accel help 2>&1 | grep -q hvf; then
    ACCEL="-machine accel=hvf"
else
    ACCEL=""
fi

qemu-system-x86_64 \
    -cdrom "$ISO" \
    -m 512 \
    $ACCEL \
    -serial stdio \
    "$@"
