#!/usr/bin/env bash
# Build TempleOS.exe: one file holding the launcher, a private QEMU and the
# installed TempleOS disk. Run in an MSYS2 UCRT64 shell with
# mingw-w64-ucrt-x86_64-qemu and zip installed, and Go on PATH.
#
# Usage: windows.sh TempleOS.qcow2 OUT.exe
set -euo pipefail

disk=$(realpath "$1")
out=$(realpath -m "$2")
here=$(cd "$(dirname "$0")/.." && pwd)
prefix=${MINGW_PREFIX:-/ucrt64}
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
rt=$stage/runtime
mkdir -p "$rt/bin" "$rt/lib/qemu" "$rt/share/qemu"

for exe in qemu-system-x86_64w.exe qemu-system-x86_64.exe; do
  [ -f "$prefix/bin/$exe" ] && cp "$prefix/bin/$exe" "$rt/bin/"
done

# Display and sound may be loadable modules rather than built in.
shopt -s nullglob
for mod in "$prefix"/lib/qemu/{ui-sdl,audio-dsound,audio-sdl}*.dll; do
  cp "$mod" "$rt/lib/qemu/"
  cp "$mod" "$rt/bin/"
done
shopt -u nullglob

# Copy every MSYS2 DLL these need, until nothing new turns up.
while :; do
  added=0
  for f in "$rt"/bin/*.exe "$rt"/bin/*.dll; do
    for dll in $(ldd "$f" | awk '{print $3}' | grep -i "^$prefix/bin/" || true); do
      if [ ! -f "$rt/bin/$(basename "$dll")" ]; then
        cp "$dll" "$rt/bin/"
        added=1
      fi
    done
  done
  [ $added = 0 ] && break
done

# PC firmware only; TempleOS needs no option ROMs (no network card).
for fw in bios.bin bios-256k.bin vgabios-stdvga.bin kvmvapic.bin \
          linuxboot.bin linuxboot_dma.bin multiboot.bin multiboot_dma.bin \
          pvh.bin; do
  [ -f "$prefix/share/qemu/$fw" ] && cp "$prefix/share/qemu/$fw" "$rt/share/qemu/"
done
cp -r "$prefix/share/qemu/keymaps" "$rt/share/qemu/"
cp "$disk" "$stage/TempleOS.qcow2"

payload=$here/launcher/payload.zip
rm -f "$payload"
(cd "$stage" && zip -qr -9 "$payload" runtime TempleOS.qcow2)

cd "$here/launcher"
go run github.com/tc-hib/go-winres@v0.3.3 simply \
  --icon "$here/package/icon.png" --manifest gui \
  --product-name TempleOS --file-description TempleOS
go build -tags payload -trimpath -ldflags "-H=windowsgui -s -w" -o "$out" .
rm -f "$payload" rsrc_windows_*.syso
du -h "$out"
