#!/usr/bin/env bash
# Build TempleOS.app (with a private QEMU and the installed TempleOS disk)
# and a .dmg holding it. Run on macOS with Homebrew's qemu and
# dylibbundler installed, and Go on PATH. Builds for the host CPU.
#
# Usage: macos.sh TempleOS.qcow2 OUT.dmg
set -euo pipefail

disk=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
out=$(cd "$(dirname "$2")" && pwd)/$(basename "$2")
here=$(cd "$(dirname "$0")/.." && pwd)
prefix=$(brew --prefix qemu)
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
app=$stage/TempleOS.app
res=$app/Contents/Resources
rt=$res/runtime
mkdir -p "$app/Contents/MacOS" "$rt/bin" "$rt/lib" "$rt/share/qemu"

cp "$prefix/bin/qemu-system-x86_64" "$rt/bin/"
chmod u+w "$rt/bin/qemu-system-x86_64"
# Copy Homebrew dylibs into the bundle and point the binary at them.
dylibbundler -of -cd -b -x "$rt/bin/qemu-system-x86_64" \
  -d "$rt/lib" -p @executable_path/../lib/

if otool -L "$rt/bin/qemu-system-x86_64" "$rt"/lib/*.dylib \
    | grep -E '^\s+(/opt/homebrew|/usr/local)/'; then
  echo "error: bundle still links against Homebrew libraries" >&2
  exit 1
fi

for fw in bios.bin bios-256k.bin vgabios-stdvga.bin kvmvapic.bin \
          linuxboot.bin linuxboot_dma.bin multiboot.bin multiboot_dma.bin \
          pvh.bin; do
  [ -f "$prefix/share/qemu/$fw" ] && cp "$prefix/share/qemu/$fw" "$rt/share/qemu/"
done
cp -r "$prefix/share/qemu/keymaps" "$rt/share/qemu/"
cp "$disk" "$res/TempleOS.qcow2"

(cd "$here/launcher" && go build -trimpath -ldflags "-s -w" \
  -o "$app/Contents/MacOS/TempleOS" .)

iconset=$stage/TempleOS.iconset
mkdir "$iconset"
for s in 16 32 128 256 512; do
  sips -z $s $s "$here/package/icon.png" --out "$iconset/icon_${s}x${s}.png" >/dev/null
  sips -z $((s*2)) $((s*2)) "$here/package/icon.png" \
    --out "$iconset/icon_${s}x${s}@2x.png" >/dev/null
done
iconutil -c icns "$iconset" -o "$res/TempleOS.icns"
cp "$here/package/Info.plist" "$app/Contents/Info.plist"

# Ad-hoc signatures: required to run on Apple Silicon. QEMU also gets the
# entitlement for Hypervisor.framework acceleration on Intel Macs.
for lib in "$rt"/lib/*.dylib; do
  codesign --force -s - "$lib"
done
codesign --force -s - --entitlements "$here/package/qemu.entitlements" \
  "$rt/bin/qemu-system-x86_64"
codesign --force -s - "$app/Contents/MacOS/TempleOS"
codesign --force -s - "$app"

rm -f "$out"
ln -s /Applications "$stage/Applications"
hdiutil create -volname TempleOS -srcfolder "$stage" -format UDZO "$out" \
  -ov >/dev/null
rm "$stage/Applications"
mv "$app" "$(dirname "$out")/"
du -h "$out"
