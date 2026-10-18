#!/usr/bin/env python3
"""Build a bootable TempleOS RedSea ISO from this source tree, on any host OS.

This reimplements ::/Adam/Opt/Boot/DskISORedSea.HC (RedSeaISO) so the
distro image can be produced without already running TempleOS:

  CD sector 0-15   zero
  CD sector 16     ISO9660 primary volume descriptor
  CD sector 17     El Torito boot record
  CD sector 18     supplementary descriptor, publisher "TempleOS RedSea"
  CD sector 19     terminator
  CD sector 20     El Torito boot catalog
  CD sector 21     stage-1 loader (Standalone/boot/BootDVD.asm)
  blk 88 onward    RedSea partition (512-byte blks): boot rec, bitmap, data

Usage: mkiso.py [-o TempleOS.ISO] [--no-repair] [--auto-install]
"""

import argparse
import datetime
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

BLK = 512
DVD_BLK = 2048
DRV_OFFSET = 19 * 4 + (DVD_BLK * 2 + DVD_BLK) // BLK  # 88, as in RedSeaISO()
CDIR_SIZE = 64
CDIR_FILENAME_LEN = 38
MBR_PT_REDSEA = 0x88

RS_ATTR_DIR = 0x10
RS_ATTR_COMPRESSED = 0x400
RS_ATTR_CONTIGUOUS = 0x800

STAGE2 = ("0000Boot", "0000Kernel.BIN.C")

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)

# Same tree ::/Misc/DoDistro.HC puts on the distro DVD: every file in the
# root dir plus these dirs. Linux/ and host-side tooling are left out.
DISTRO_DIRS = ["0000Boot", "Home", "Adam", "Apps", "Compiler", "Demo", "Doc",
               "Kernel", "Misc", "Downloads"]
# Created empty when missing, like DirMk() in DoDistro.
EMPTY_DIRS = ["Home", "Tmp", "Tmp/ScrnShots"]
HOST_ONLY_FILES = {"README.md"}

REPAIR = True
REPAIRED = []

# See char_bmp_filename in ::/Doc/RedSea.DD.
CHAR_BMP_FILENAME = [0x0000000, 0x03FF73FB, 0xEFFFFFFF, 0x2FFFFFFF,
                     0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF]


def valid_name(name):
    b = name.encode("latin-1", "replace")
    if not b or len(b) >= CDIR_FILENAME_LEN:
        return False
    return all(CHAR_BMP_FILENAME[c >> 5] >> (c & 31) & 1 for c in b)


def cdate(ts):
    """Host timestamp -> TempleOS CDate (see Struct2Date in ::/Kernel/KDate.HC)."""
    dt = datetime.datetime.fromtimestamp(ts)

    def year_start(y):
        y1 = y - 1
        return y * 365 + y1 // 4 - y1 // 100 + y1 // 400 - y1 // 4000

    date = year_start(dt.year) + dt.timetuple().tm_yday - 1
    sec10000 = ((dt.hour * 60 + dt.minute) * 60 + dt.second) * 10000 \
        + dt.microsecond // 100
    time = (sec10000 << 21) // (15 * 15 * 3 * 625)
    return time & 0xFFFFFFFF, date


def pal16(buf, off, v):
    """CPalindromeU16: little-endian copy then big-endian copy."""
    struct.pack_into("<H", buf, off, v)
    struct.pack_into(">H", buf, off + 2, v)


def pal32(buf, off, v):
    struct.pack_into("<I", buf, off, v)
    struct.pack_into(">I", buf, off + 4, v)


DOC_BIN_HDR = 16  # CDocBin num,flags,size,use_cnt
DOC_BIN_REF = re.compile(rb"\$([A-Z]{2})([^$]*?),BI=(\d+)([^$]*)\$")


def repair_doldoc(data):
    """Work around DolDoc files whose binary section is damaged in this repo.

    Many files in this source tree have bytes missing from the binary data
    after the text's NUL (sprites, Kernel.MAP debug info). TempleOS then
    shows "Bin Not Found", pausing 3 s for each missing bin: about 30 min
    for Kernel.MAP alone at boot. This keeps the bins that still parse
    and, for references to lost bins, drops the debug info from $LK links
    and turns sprite menu items into text macros. Other references are left
    alone. Returns None if the file needs no changes.
    """
    nul = data.find(b"\0")
    if nul < 0 or not DOC_BIN_REF.search(data, 0, nul):
        return None
    bins = bytearray()
    nums = set()
    pos = nul + 1
    damaged = False
    while pos + DOC_BIN_HDR <= len(data):
        num, flags, size, _ = struct.unpack_from("<IIII", data, pos)
        if flags or size > len(data):
            damaged = True
            break
        end = pos + DOC_BIN_HDR + size
        if end > len(data):
            # Last bin is a few bytes short; TempleOS would read past the
            # buffer. Pad it so the bin and its trailing SPT_END survive.
            if end - len(data) > 8:
                damaged = True
                break
            bins += data[pos:] + bytes(end - len(data))
            nums.add(num)
            damaged = True
            pos = len(data)
            break
        bins += data[pos:end]
        nums.add(num)
        pos = end
    if pos != len(data):
        damaged = True
    if not damaged:
        return None

    def fix(m):
        if int(m.group(3)) in nums:
            return m.group(0)
        typ, rest = m.group(1), m.group(2) + m.group(4)
        if typ == b"LK":
            return b"$LK" + rest + b"$"
        if typ == b"SP" and b"LM=" in rest:
            return b"$MA" + rest + b"$"
        return m.group(0)
    text = DOC_BIN_REF.sub(fix, data[:nul])
    return text + b"\0" + bytes(bins)


def ceil_blks(n):
    return (n + BLK - 1) // BLK


class Node:
    def __init__(self, name, path, is_dir, mtime=None):
        self.name = name
        self.path = path
        self.is_dir = is_dir
        self.children = []
        self.size = 0
        self.clus = 0
        self.mtime = os.path.getmtime(path) if mtime is None else mtime

    def attr(self):
        if self.is_dir:
            return RS_ATTR_DIR | RS_ATTR_CONTIGUOUS
        a = RS_ATTR_CONTIGUOUS  # RedSea files are always contiguous
        if self.name.upper().endswith(".Z"):
            a |= RS_ATTR_COMPRESSED
        return a

    def dir_size(self):
        # Same sizing as RedSeaISOPass1/RedSeaMkDir: entries + ".", "..", end.
        return ceil_blks((len(self.children) + 3) * CDIR_SIZE) * BLK


def scan(path, name, top=False):
    node = Node(name, path, True)
    for entry in sorted(os.listdir(path), key=str.upper):
        if entry.startswith("."):
            continue
        full = os.path.join(path, entry)
        if top and (entry in HOST_ONLY_FILES or
                    os.path.isdir(full) and entry not in DISTRO_DIRS):
            continue
        if os.path.islink(full):
            continue
        if not valid_name(entry):
            sys.exit(f"mkiso: '{full}' is not a valid RedSea filename")
        if os.path.isdir(full):
            node.children.append(scan(full, entry))
        elif os.path.isfile(full):
            child = Node(entry, full, False)
            with open(full, "rb") as f:
                child.data = f.read()
            if REPAIR:
                fixed = repair_doldoc(child.data)
                if fixed is not None:
                    REPAIRED.append(os.path.relpath(full, REPO))
                    child.data = fixed
            child.size = len(child.data)
            node.children.append(child)
    return node


def add_auto_install(tree):
    home = next(c for c in tree.children if c.name == "Home")
    path = os.path.join(HERE, "autoinstall", "Once.HC")
    node = Node("Once.HC", path, False)
    with open(path, "rb") as f:
        node.data = f.read()
    node.size = len(node.data)
    home.children = [c for c in home.children if c.name != node.name]
    home.children.append(node)
    home.children.sort(key=lambda c: c.name.upper())


def add_empty_dirs(tree):
    for path in EMPTY_DIRS:
        node = tree
        for part in path.split("/"):
            child = next((c for c in node.children
                          if c.name.upper() == part.upper()), None)
            if child is None:
                child = Node(part, None, True, mtime=tree.mtime)
                node.children.append(child)
                node.children.sort(key=lambda c: c.name.upper())
            node = child


def assemble_stage1(asm):
    nasm = shutil.which("nasm")
    if not nasm:
        sys.exit("mkiso: nasm is required (or pass --stage1 with a prebuilt "
                 "BootDVD.bin)")
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "BootDVD.bin")
        subprocess.run([nasm, "-f", "bin", "-o", out, asm], check=True)
        with open(out, "rb") as f:
            return f.read()


def build(root, out_path, stage1, auto_install=False):
    tree = scan(root, ".", top=True)
    add_empty_dirs(tree)
    if auto_install:
        add_auto_install(tree)

    # Lay out every directory and file contiguously, directory first.
    def total_blks(node):
        n = node.dir_size() // BLK
        for c in node.children:
            n += total_blks(c) if c.is_dir else ceil_blks(c.size)
        return n

    data_blks = total_blks(tree)
    bitmap_blks = 1
    while True:
        prev = bitmap_blks
        bitmap_blks = (data_blks + bitmap_blks + BLK * 8 - 1) // (BLK * 8)
        if bitmap_blks == prev:
            break
    max_blk = -(-(DRV_OFFSET + 1 + bitmap_blks + data_blks) // 4) * 4 - 1
    sects = max_blk + 1 - DRV_OFFSET
    bitmap_sects = (sects + BLK * 8 - 1) // (BLK * 8)
    data_area = DRV_OFFSET + bitmap_sects
    # Clus data_area is reserved ("Alloc #1" in RedSeaFmt).
    next_free = [data_area + 1]

    def alloc(n):
        c = next_free[0]
        next_free[0] += n
        return c

    def place(node):
        node.clus = alloc(node.dir_size() // BLK)
        for c in node.children:
            if c.is_dir:
                place(c)
            else:
                c.clus = alloc(max(1, ceil_blks(c.size))) if c.size else 0
    place(tree)
    used_end = next_free[0]
    if used_end > max_blk + 1:
        sys.exit("mkiso: internal layout error")

    img = bytearray((max_blk + 1) * BLK)

    def entry(attr, name, clus, size, mtime):
        t, d = cdate(mtime)
        return struct.pack("<H38sqqIi", attr, name.encode("latin-1"),
                           clus, size, t, d)

    def write_dir(node, parent_clus):
        buf = bytearray(node.dir_size())
        self_name = "." if node is tree else node.name
        buf[0:64] = entry(node.attr(), self_name, node.clus, node.dir_size(),
                          node.mtime)
        buf[64:128] = entry(RS_ATTR_DIR | RS_ATTR_CONTIGUOUS, "..",
                            parent_clus, 0, node.mtime)
        off = 128
        for c in node.children:
            size = c.dir_size() if c.is_dir else c.size
            buf[off:off + 64] = entry(c.attr(), c.name, c.clus, size, c.mtime)
            off += 64
        img[node.clus * BLK:node.clus * BLK + len(buf)] = buf
        for c in node.children:
            if c.is_dir:
                write_dir(c, node.clus)
            elif c.size:
                img[c.clus * BLK:c.clus * BLK + c.size] = c.data
    write_dir(tree, tree.clus)

    # Allocation bitmap: bit i <-> blk data_area+i, LSB first.
    fat1 = DRV_OFFSET + 1
    for blk in range(data_area, used_end):
        i = blk - data_area
        img[fat1 * BLK + (i >> 3)] |= 1 << (i & 7)

    # RedSea boot record.
    br = bytearray(BLK)
    br[3] = MBR_PT_REDSEA
    struct.pack_into("<qqqqq", br, 8, DRV_OFFSET, sects, tree.clus,
                     bitmap_sects, int.from_bytes(os.urandom(8), "little") >> 1)
    struct.pack_into("<H", br, 510, 0xAA55)
    img[DRV_OFFSET * BLK:(DRV_OFFSET + 1) * BLK] = br

    # ISO9660 descriptors (RedSeaISO9660()).
    iso_sects = (max_blk + 1) // 4

    def sect(n):
        return n * DVD_BLK

    pri = bytearray(DVD_BLK)
    pri[0] = 1
    pri[1:6] = b"CD001"
    pri[6] = 1
    pal32(pri, 80, iso_sects)
    pal16(pri, 120, 1)
    pal16(pri, 124, 1)
    pal16(pri, 128, DVD_BLK)
    pal32(pri, 152, tree.clus)
    # Offsets follow CISOPriDesc in ::/Kernel/KernelA.HH, not ISO9660 proper.
    pri[314:314 + 15] = b"TempleOS RedSea"
    pri[877] = 1
    sup = bytearray(pri)
    sup[0] = 2
    boot = bytearray(DVD_BLK)
    boot[0] = 0
    boot[1:6] = b"CD001"
    boot[6] = 1
    boot[7:7 + 23] = b"EL TORITO SPECIFICATION"
    struct.pack_into("<I", boot, 0x47, 20)
    term = bytearray(DVD_BLK)
    term[0] = 255
    term[1:6] = b"CD001"
    term[6] = 1
    for n, d in ((16, pri), (17, boot), (18, sup), (19, term)):
        img[sect(n):sect(n + 1)] = d

    # El Torito boot catalog.
    et = bytearray(DVD_BLK)
    w = [0] * 16
    w[0] = 1
    name = b"TempleOS".ljust(24, b"\0")
    for i in range(12):
        w[2 + i] = name[2 * i] | name[2 * i + 1] << 8
    w[15] = 0xAA55
    w[14] = -sum(w) & 0xFFFF
    struct.pack_into("<16H", et, 0, *w)
    struct.pack_into("<BBHBBHI", et, 32, 0x88, 0, 0, 0, 0, 4, 21)
    img[sect(20):sect(21)] = et

    # Stage 1, patched to load the kernel (RedSeaISO9660Stage1()).
    kernel = tree
    for part in STAGE2:
        kernel = next((c for c in kernel.children
                       if c.name.upper() == part.upper()), None)
        if kernel is None:
            sys.exit("mkiso: /" + "/".join(STAGE2) + " not found")
    s1 = bytearray(stage1[:DVD_BLK].ljust(DVD_BLK, b"\0"))
    blk_cnt = (kernel.size + DVD_BLK - 1) // DVD_BLK
    if kernel.clus & 3:
        blk_cnt += 1
    struct.pack_into("<IHH", s1, 2, kernel.clus >> 2, blk_cnt,
                     kernel.clus & 3)
    img[sect(21):sect(22)] = s1

    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    with open(out_path, "wb") as f:
        f.write(img)
    return len(img)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("-o", "--output", default="TempleOS.ISO")
    ap.add_argument("--root", default=REPO, help="TempleOS file tree")
    ap.add_argument("--stage1", help="prebuilt BootDVD.bin (skips nasm)")
    ap.add_argument("--no-repair", action="store_true",
                    help="copy damaged DolDoc binary sections as-is")
    ap.add_argument("--auto-install", action="store_true",
                    help="boot straight into an unattended install "
                         "(used by mkdisk.py)")
    args = ap.parse_args()
    global REPAIR
    REPAIR = not args.no_repair
    if args.stage1:
        with open(args.stage1, "rb") as f:
            stage1 = f.read()
    else:
        stage1 = assemble_stage1(os.path.join(HERE, "boot", "BootDVD.asm"))
    size = build(os.path.abspath(args.root), args.output, stage1,
                 args.auto_install)
    if REPAIRED:
        print(f"mkiso: worked around damaged DolDoc bins in "
              f"{len(REPAIRED)} files")
    print(f"mkiso: wrote {args.output} ({size // 1024} KiB)")


if __name__ == "__main__":
    main()
