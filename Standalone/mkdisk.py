#!/usr/bin/env python3
"""Build a ready-to-boot, already-installed TempleOS hard disk image.

Boots an auto-install ISO (mkiso.py --auto-install) in headless QEMU, lets
TempleOS's own installer (::/Misc/OSInstall.HC) partition the disk into C:
and D:, compile its kernel and copy itself over, then swaps in a direct-boot
MBR (boot/MBR.asm). The result is what the standalone app ships.

Usage: mkdisk.py [-o TempleOS.qcow2] [--size 2G] [--qemu qemu-system-x86_64]
Needs: python3, nasm, qemu-system-x86_64 (and qemu-img for .qcow2 output).
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mkiso  # noqa: E402

# OutU8(0xF4,0x10) in autoinstall/Once.HC; isa-debug-exit returns (v<<1)|1.
INSTALL_DONE_EXIT = (0x10 << 1) | 1
MBR_CODE_SIZE = 446


def parse_size(s):
    units = {"K": 1 << 10, "M": 1 << 20, "G": 1 << 30}
    if s[-1].upper() in units:
        return int(s[:-1]) * units[s[-1].upper()]
    return int(s)


def accel_args():
    if sys.platform.startswith("linux") and os.access("/dev/kvm", os.W_OK):
        return ["-accel", "kvm"]
    return ["-accel", "tcg"]


def nasm_bin(asm):
    nasm = shutil.which("nasm")
    if not nasm:
        sys.exit("mkdisk: nasm is required")
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "out.bin")
        subprocess.run([nasm, "-f", "bin", "-o", out, asm], check=True)
        with open(out, "rb") as f:
            return f.read()


def install(qemu, iso, disk, timeout):
    cmd = [qemu, *accel_args(), "-m", "1024", "-no-reboot",
           "-drive", f"file={disk},format=raw,if=ide,index=0",
           "-drive", f"file={iso},format=raw,media=cdrom,if=ide,index=2",
           "-boot", "order=cd", "-display", "none", "-vga", "std",
           "-nic", "none",
           "-device", "isa-debug-exit,iobase=0xf4,iosize=0x04"]
    print("mkdisk: running unattended install:", " ".join(cmd), flush=True)
    try:
        res = subprocess.run(cmd, timeout=timeout)
    except subprocess.TimeoutExpired:
        sys.exit(f"mkdisk: install did not finish within {timeout}s")
    if res.returncode != INSTALL_DONE_EXIT:
        sys.exit(f"mkdisk: QEMU exited with {res.returncode}, expected "
                 f"{INSTALL_DONE_EXIT} from the install script")


def finish_disk(disk, mbr_code):
    with open(disk, "r+b") as f:
        mbr = bytearray(f.read(512))
        if mbr[510:512] != b"\x55\xAA":
            sys.exit("mkdisk: installer left no MBR")
        part = mbr[446:462]
        if not part[4]:
            sys.exit("mkdisk: installer left no partition")
        lba = int.from_bytes(part[8:12], "little")
        f.seek(lba * 512)
        vbr = f.read(512)
        if vbr[510:512] != b"\x55\xAA":
            sys.exit("mkdisk: drive C has no boot record")
        mbr[:MBR_CODE_SIZE] = mbr_code[:MBR_CODE_SIZE]
        f.seek(0)
        f.write(mbr)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("-o", "--output", default="TempleOS.qcow2",
                    help=".qcow2 (compressed, needs qemu-img) or raw image")
    ap.add_argument("--size", default="2G")
    ap.add_argument("--qemu", default=shutil.which("qemu-system-x86_64"))
    ap.add_argument("--qemu-img", default=shutil.which("qemu-img"))
    ap.add_argument("--timeout", type=int, default=1800)
    ap.add_argument("--root", default=mkiso.REPO)
    args = ap.parse_args()
    if not args.qemu:
        sys.exit("mkdisk: qemu-system-x86_64 not found")
    qcow2 = args.output.lower().endswith(".qcow2")
    if qcow2 and not args.qemu_img:
        sys.exit("mkdisk: qemu-img is required for .qcow2 output")

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        iso = os.path.join(tmp, "AutoInstall.ISO")
        stage1 = nasm_bin(os.path.join(HERE, "boot", "BootDVD.asm"))
        mkiso.build(os.path.abspath(args.root), iso, stage1,
                    auto_install=True)
        raw = os.path.join(tmp, "disk.img")
        with open(raw, "wb") as f:
            f.truncate(parse_size(args.size))
        install(args.qemu, iso, raw, args.timeout)
        finish_disk(raw, nasm_bin(os.path.join(HERE, "boot", "MBR.asm")))
        if qcow2:
            subprocess.run([args.qemu_img, "convert", "-c", "-f", "raw",
                            "-O", "qcow2", raw, args.output], check=True)
        else:
            shutil.move(raw, args.output)
    print(f"mkdisk: wrote {args.output}")


if __name__ == "__main__":
    main()
