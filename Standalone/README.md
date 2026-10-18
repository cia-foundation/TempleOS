# TempleOS as a desktop app

Run TempleOS on Windows or macOS like any other program. Download one file,
double-click it, and TempleOS boots in a window. You don't install a virtual
machine or set one up, and you don't run an installer inside TempleOS.

| Platform | Download | What it is |
|---|---|---|
| Windows 10/11 (x64) | `TempleOS-Windows-x64.exe` | A single portable `.exe`. |
| macOS 12+ (Apple Silicon) | `TempleOS-macOS-arm64.dmg` | `TempleOS.app`; drag it to Applications. |
| macOS 12+ (Intel) | `TempleOS-macOS-x64.dmg` | Same, for Intel Macs. |

The builds come from the [Standalone apps workflow](../.github/workflows/standalone.yml):
each run's artifacts, and a GitHub release for every `v*` tag.

## How it works

TempleOS is a bare-metal x86-64 operating system: it runs in ring 0, drives
the hardware directly and owns the page tables and interrupts. It can't run
as a normal process, so each app carries a private, trimmed-down copy of
[QEMU](https://www.qemu.org/) and starts it with the PC TempleOS expects
(IDE disk, standard VGA, PS/2 keyboard and mouse, PC speaker). The app
contains everything; it uses nothing installed on the computer.

The TempleOS inside is built from this repository:

1. `mkiso.py` writes a bootable TempleOS RedSea ISO from this source tree.
   It reimplements TempleOS's own `RedSeaISO()` (`::/Adam/Opt/Boot/DskISORedSea.HC`)
   so it runs on any host, using `boot/BootDVD.asm`, a NASM port of the
   El Torito loader in `::/Adam/Opt/Boot/BootDVD.HC`.
2. `mkdisk.py` boots that ISO with `autoinstall/Once.HC` and lets TempleOS's
   own installer (`::/Misc/OSInstall.HC`) set up drives C: and D: and compile
   its kernel, unattended. It then swaps TempleOS's boot menu, which waits
   for a key forever, for `boot/MBR.asm`, which boots C: directly.
3. `launcher/` is a small Go program, the app itself. On first launch it
   copies the installed disk to your user data folder. Each later launch
   boots that copy, so whatever you do inside TempleOS persists.

On Windows the launcher, QEMU and the disk are one self-extracting `.exe`.
It unpacks the runtime to `%LOCALAPPDATA%\TempleOS` the first time it runs.

## Using it

- **Windows:** run `TempleOS-Windows-x64.exe`. SmartScreen may warn about an
  unrecognized app because the build isn't code-signed. Choose *More info >
  Run anyway*.
- **macOS:** open the `.dmg` and drag `TempleOS.app` to Applications. The app
  is ad-hoc signed, not notarized. On first launch macOS will refuse to open
  it. Open *System Settings > Privacy & Security* and click *Open Anyway*.
  Or run `xattr -dr com.apple.quarantine /Applications/TempleOS.app`.
- Click in the window to give TempleOS the mouse. Press **Ctrl+Alt+G**
  (Windows) or **Ctrl+Option+G** (macOS) to get it back.
- Your TempleOS disk lives in `%LOCALAPPDATA%\TempleOS\TempleOS.qcow2` on
  Windows or `~/Library/Application Support/TempleOS/TempleOS.qcow2` on
  macOS. To start over, run the app with `-reset`.

Command-line flags (all optional):

```
TempleOS [-reset] [-memory MiB] [-cpus N] [-fullscreen] [-accel tcg|kvm|hvf|whpx]
         [-data DIR] [-- extra QEMU args]
```

Speed: the apps use QEMU's emulator (TCG). TempleOS is small enough that
this is comfortably fast. Hardware acceleration is opt-in because it is
unreliable with TempleOS: `-accel hvf` on Intel Macs currently crashes
QEMU during boot, and `-accel whpx` on Windows needs the optional Windows
Hypervisor Platform feature. On Linux the launcher uses KVM when
`/dev/kvm` is available.

## Building

Linux (or WSL) builds the TempleOS images. It needs `python3`, `nasm`,
`qemu-system-x86` and `qemu-utils`:

```sh
python3 Standalone/mkiso.py -o dist/TempleOS.ISO      # bootable live ISO
python3 Standalone/mkdisk.py -o dist/TempleOS.qcow2   # installed disk (~2 min)
```

Then package on the target OS (Go 1.22+):

```sh
# Windows, MSYS2 UCRT64 shell with mingw-w64-ucrt-x86_64-qemu and zip:
Standalone/package/windows.sh dist/TempleOS.qcow2 out/TempleOS.exe
# macOS, with `brew install qemu dylibbundler`:
Standalone/package/macos.sh dist/TempleOS.qcow2 out/TempleOS.dmg
```

`TempleOS -self-test 120s` boots the disk headless and checks that TempleOS
reaches its desktop. CI runs it on every platform. On Linux the launcher
also works without packaging: put `TempleOS.qcow2` next to the binary, and it
uses the system QEMU.

## Damaged sprite data in this tree

Many DolDoc files in this repository have bytes missing from the binary
data after their text: sprites, and the debug info in `Kernel/Kernel.MAP`
and `Compiler/Compiler.MAP`. Every historical version of these files in git
has the same problem, so the damage predates this tooling. TempleOS reacts
by pausing 3 seconds for each missing piece ("Bin Not Found"). For
Kernel.MAP alone that would add about half an hour to every boot.

`mkiso.py` works around this in the image only; the files in the repository
are not changed. It keeps the binary entries that are still intact. For lost
ones, it removes the debug-info reference from `$LK` links and turns sprite
menu items into text items. The sprites themselves can't be recovered, so
some demos and games that draw them won't work. Pass `--no-repair` to
copy the files exactly as they are.
