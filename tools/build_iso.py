#!/usr/bin/env python3
"""Build a bootable TempleOS ISO from the source tree.

Creates a RedSea filesystem image wrapped in ISO 9660 with El Torito boot
headers. The precompiled kernel binary (0000Kernel.BIN.C) is embedded and
the boot loader is patched with its LBA.

Usage:
    python3 build_iso.py [source_dir] [output.iso]
"""

import os
import struct
import sys
import time
from pathlib import Path

# TempleOS constants
BLK_SIZE_BITS = 9
BLK_SIZE = 1 << BLK_SIZE_BITS  # 512
DVD_BLK_SIZE = 4 * BLK_SIZE  # 2048
DVD_BOOT_LOADER_SIZE = DVD_BLK_SIZE  # 2048

# RedSea filesystem constants
CDIR_SIZE = 64
CDIR_FILENAME_LEN = 38
MBR_PT_REDSEA = 0x88

# RedSea attributes
RS_ATTR_READ_ONLY = 0x01
RS_ATTR_HIDDEN = 0x02
RS_ATTR_SYSTEM = 0x04
RS_ATTR_VOL_ID = 0x08
RS_ATTR_DIR = 0x10
RS_ATTR_ARCHIVE = 0x20
RS_ATTR_CONTIGUOUS = 0x800

# ISO 9660 types
ISOT_BOOT_RECORD = 0
ISOT_PRI_VOL_DESC = 1
ISOT_SUPPLEMENTARY_DESC = 2
ISOT_TERMINATOR = 255

# Boot loader patch offsets (from NASM assembly)
# These are the offsets of the patchable fields within boot_dvd.bin
BDVD_BLK_LO_OFFSET = 0x83
BDVD_BLK_HI_OFFSET = 0x85
BDVD_BLK_CNT_OFFSET = 0x87
BDVD_SHIFT_BLKS_OFFSET = 0x89


def ceil_to(value: int, alignment: int) -> int:
    return (value + alignment - 1) // alignment * alignment


def ceil_div(a: int, b: int) -> int:
    return (a + b - 1) // b


def make_palindrome_u16(value: int) -> bytes:
    """TempleOS both-endian U16 (big-endian first, little-endian second)."""
    be = struct.pack(">H", value & 0xFFFF)
    le = struct.pack("<H", value & 0xFFFF)
    return be + le


def make_palindrome_u32(value: int) -> bytes:
    """TempleOS both-endian U32 (big-endian first, little-endian second)."""
    be = struct.pack(">I", value & 0xFFFFFFFF)
    le = struct.pack("<I", value & 0xFFFFFFFF)
    return be + le


def make_dir_entry(
    name: str,
    attr: int,
    clus: int,
    size: int,
    datetime: int = 0,
) -> bytes:
    """Create a 64-byte RedSea directory entry."""
    name_bytes = name.encode("ascii")[:CDIR_FILENAME_LEN - 1]
    name_bytes = name_bytes + b"\x00" * (CDIR_FILENAME_LEN - len(name_bytes))
    return struct.pack(
        "<H38sqqq",
        attr,
        name_bytes,
        clus,
        size,
        datetime,
    )


class FileEntry:
    """Represents a file to be written to the RedSea filesystem."""

    def __init__(self, host_path: str, name: str, size: int) -> None:
        self.host_path = host_path
        self.name = name
        self.size = size
        self.blocks = ceil_div(size, BLK_SIZE) if size > 0 else 0
        self.clus = 0  # assigned during layout


class DirEntry:
    """Represents a directory in the RedSea filesystem."""

    def __init__(self, name: str) -> None:
        self.name = name
        self.files: list[FileEntry] = []
        self.subdirs: list["DirEntry"] = []
        self.clus = 0  # assigned during layout
        self.size = 0  # in bytes, assigned during layout
        self.blocks = 0  # assigned during layout


def scan_directory(host_path: str, name: str) -> DirEntry:
    """Recursively scan a host directory into our tree structure."""
    d = DirEntry(name)

    try:
        entries = sorted(os.listdir(host_path))
    except PermissionError:
        return d

    for entry_name in entries:
        full_path = os.path.join(host_path, entry_name)

        # Skip hidden files, .git, our tools directory, and generated ISOs
        if (entry_name.startswith(".")
            or entry_name == "tools"
            or entry_name.endswith(".iso")):
            continue

        # TempleOS filenames are max 37 chars
        if len(entry_name) > CDIR_FILENAME_LEN - 1:
            print(f"  Warning: skipping '{entry_name}' (name too long)")
            continue

        if os.path.isdir(full_path):
            subdir = scan_directory(full_path, entry_name)
            d.subdirs.append(subdir)
        elif os.path.isfile(full_path):
            size = os.path.getsize(full_path)
            d.files.append(FileEntry(full_path, entry_name, size))

    return d


def compute_dir_size(d: DirEntry) -> None:
    """Compute directory entry block sizes recursively."""
    # Each directory has: ".", "..", entries..., null terminator
    entry_count = 3 + len(d.files) + len(d.subdirs)
    d.size = ceil_to(entry_count * CDIR_SIZE, BLK_SIZE)
    d.blocks = d.size // BLK_SIZE

    for subdir in d.subdirs:
        compute_dir_size(subdir)


def count_data_blocks(d: DirEntry) -> int:
    """Count total data blocks needed for a directory tree."""
    total = d.blocks  # this directory's entries

    for f in d.files:
        total += f.blocks

    for subdir in d.subdirs:
        total += count_data_blocks(subdir)

    return total


def assign_clusters(
    d: DirEntry,
    next_clus: int,
    parent_clus: int,
) -> int:
    """Assign cluster (block) numbers to all entries. Returns next free cluster."""
    d.clus = next_clus
    next_clus += d.blocks

    for f in d.files:
        if f.blocks > 0:
            f.clus = next_clus
            next_clus += f.blocks
        else:
            f.clus = 0

    for subdir in d.subdirs:
        next_clus = assign_clusters(subdir, next_clus, d.clus)

    return next_clus


def build_dir_data(d: DirEntry, parent_clus: int) -> bytes:
    """Build the raw bytes for a directory's entry table."""
    entries = bytearray()

    # "." entry
    entries += make_dir_entry(
        ".", RS_ATTR_DIR | RS_ATTR_CONTIGUOUS, d.clus, d.size
    )

    # ".." entry
    entries += make_dir_entry(
        "..", RS_ATTR_DIR | RS_ATTR_CONTIGUOUS, parent_clus, 0
    )

    # File entries
    for f in d.files:
        attr = RS_ATTR_CONTIGUOUS
        entries += make_dir_entry(f.name, attr, f.clus, f.size)

    # Subdirectory entries
    for subdir in d.subdirs:
        entries += make_dir_entry(
            subdir.name,
            RS_ATTR_DIR | RS_ATTR_CONTIGUOUS,
            subdir.clus,
            subdir.size,
        )

    # Pad to directory size (null terminator entry is implicit - zeroed padding)
    entries += b"\x00" * (d.size - len(entries))

    return bytes(entries)


def write_dir_tree(
    iso: bytearray,
    d: DirEntry,
    parent_clus: int,
) -> None:
    """Recursively write directory entries and file data into the ISO image."""
    # Write this directory's entries
    dir_data = build_dir_data(d, parent_clus)
    offset = d.clus * BLK_SIZE
    iso[offset : offset + len(dir_data)] = dir_data

    # Write file contents
    for f in d.files:
        if f.size > 0:
            with open(f.host_path, "rb") as fh:
                data = fh.read()
            offset = f.clus * BLK_SIZE
            iso[offset : offset + f.size] = data

    # Recurse into subdirectories
    for subdir in d.subdirs:
        write_dir_tree(iso, subdir, d.clus)


def build_bitmap(
    data_area: int,
    total_data_blocks: int,
    bitmap_blks: int,
) -> bytearray:
    """Build the allocation bitmap. Bit N = block (data_area + N) is allocated."""
    bitmap_size = bitmap_blks * BLK_SIZE
    bitmap = bytearray(bitmap_size)

    # Mark all used data blocks as allocated
    # Data blocks run from data_area to data_area + total_data_blocks - 1
    for i in range(total_data_blocks):
        byte_idx = i // 8
        bit_idx = i % 8
        if byte_idx < bitmap_size:
            bitmap[byte_idx] |= 1 << bit_idx

    return bitmap


def build_redsea_boot_sector(
    drv_offset: int,
    sects: int,
    root_clus: int,
    bitmap_sects: int,
) -> bytes:
    """Build the 512-byte RedSea boot sector (CRedSeaBoot)."""
    sector = bytearray(BLK_SIZE)

    # jump_and_nop[3] - short jump past header + nop
    sector[0] = 0xEB
    sector[1] = 0x1E  # jump +30
    sector[2] = 0x07  # nop-equivalent (POP ES in real mode)

    # signature (offset 3)
    sector[3] = MBR_PT_REDSEA  # 0x88

    # reserved[4] (offset 4-7)
    # Leave as zeros

    # drv_offset (I64, offset 8)
    struct.pack_into("<q", sector, 8, drv_offset)

    # sects (I64, offset 16)
    struct.pack_into("<q", sector, 16, sects)

    # root_clus (I64, offset 24)
    struct.pack_into("<q", sector, 24, root_clus)

    # bitmap_sects (I64, offset 32)
    struct.pack_into("<q", sector, 32, bitmap_sects)

    # unique_id (I64, offset 40) - use current time
    struct.pack_into("<q", sector, 40, int(time.time()) & 0x7FFFFFFFFFFFFFFF)

    # code[462] (offset 46-507)
    # Leave as zeros (not bootable from this sector directly)

    # signature2 (U16, offset 510)
    struct.pack_into("<H", sector, 510, 0xAA55)

    return bytes(sector)


def build_iso9660_headers(
    vol_space_size_dvd: int,
    root_clus: int,
) -> tuple[bytes, bytes, bytes, bytes, bytes]:
    """Build ISO 9660 descriptors and El Torito catalog.

    Returns (pvd, boot_record, svd, terminator, el_torito_catalog).
    Each is DVD_BLK_SIZE (2048) bytes.
    """
    # Primary Volume Descriptor
    # Field offsets match TempleOS CISOPriDesc struct, which differs from
    # ISO 9660 standard after offset 132 (pad[20] instead of pad[24]).
    pvd = bytearray(DVD_BLK_SIZE)
    pvd[0] = ISOT_PRI_VOL_DESC  # type (offset 0)
    pvd[1:6] = b"CD001"  # id (offset 1)
    pvd[6] = 1  # version (offset 6)
    # vol_space_size at offset 80 (CPalindromeU32)
    pvd[80:88] = make_palindrome_u32(vol_space_size_dvd)
    # vol_set_size at offset 120 (CPalindromeU16)
    pvd[120:124] = make_palindrome_u16(1)
    # vol_seq_num at offset 124 (CPalindromeU16)
    pvd[124:128] = make_palindrome_u16(1)
    # log_block_size at offset 128 (CPalindromeU16)
    pvd[128:132] = make_palindrome_u16(DVD_BLK_SIZE)
    # root_dir_record at offset 152 (CISODirEntry: pad[2] + CPalindromeU32 loc)
    # loc is at offset 154
    pvd[154:162] = make_palindrome_u32(root_clus)
    # publisher_id at offset 314 (TempleOS struct offset, NOT ISO 9660's 318)
    pub_id = b"TempleOS RedSea"
    pvd[314 : 314 + len(pub_id)] = pub_id
    # file_structure_version at offset 877 (TempleOS struct offset)
    pvd[877] = 1

    # Boot Record (El Torito)
    boot_rec = bytearray(DVD_BLK_SIZE)
    boot_rec[0] = ISOT_BOOT_RECORD
    boot_rec[1:6] = b"CD001"
    boot_rec[6] = 1
    boot_rec[7 : 7 + len(b"EL TORITO SPECIFICATION")] = (
        b"EL TORITO SPECIFICATION"
    )
    # Pointer to El Torito catalog at offset 0x47 (71)
    # Catalog is at DVD sector 20
    struct.pack_into("<I", boot_rec, 0x47, 20)

    # Supplementary Volume Descriptor (copy of PVD with different type)
    svd = bytearray(pvd)
    svd[0] = ISOT_SUPPLEMENTARY_DESC

    # Terminator
    term = bytearray(DVD_BLK_SIZE)
    term[0] = ISOT_TERMINATOR
    term[1:6] = b"CD001"
    term[6] = 1

    # El Torito Catalog
    et = bytearray(DVD_BLK_SIZE)
    # Validation entry (32 bytes)
    struct.pack_into("<H", et, 0, 1)  # Header ID
    # Platform ID etc at bytes 2-3 (zero = x86)
    et_id = b"TempleOS"
    et[4 : 4 + len(et_id)] = et_id
    # Checksum at offset 28 (word 14) and key bytes at offset 30 (word 15)
    et[30] = 0x55
    et[31] = 0xAA
    # Calculate checksum so all 16 words sum to 0
    checksum = 0
    for i in range(0, 32, 2):
        checksum += struct.unpack_from("<H", et, i)[0]
    checksum = (-checksum) & 0xFFFF
    # Fix: subtract current word at offset 28 and replace with checksum
    old_w14 = struct.unpack_from("<H", et, 28)[0]
    checksum_adjust = (checksum - old_w14) & 0xFFFF
    # Recalculate properly
    et[28] = 0
    et[29] = 0
    total = 0
    for i in range(0, 32, 2):
        total += struct.unpack_from("<H", et, i)[0]
    et[28:30] = struct.pack("<H", (-total) & 0xFFFF)

    # Initial/Default Entry (32 bytes starting at offset 32)
    et[32] = 0x88  # Bootable
    et[33] = 0  # No emulation
    struct.pack_into("<H", et, 34, 0)  # Load segment (0 = 0x7C0)
    et[36] = 0  # System type
    et[37] = 0  # Unused
    struct.pack_into("<H", et, 38, 4)  # Sector count (4 * 512 = 2048 bytes)
    # Load RBA = DVD sector of boot loader = sector 21
    struct.pack_into("<I", et, 40, 21)

    return bytes(pvd), bytes(boot_rec), bytes(svd), bytes(term), bytes(et)


def find_kernel_entry(root: DirEntry) -> FileEntry | None:
    """Find the kernel binary in the directory tree."""
    for subdir in root.subdirs:
        if subdir.name == "0000Boot":
            for f in subdir.files:
                if f.name == "0000Kernel.BIN.C":
                    return f
    return None


def patch_boot_loader(
    boot_loader: bytearray,
    kernel_clus: int,
    kernel_size: int,
) -> None:
    """Patch the boot loader with kernel location on the ISO.

    The boot loader reads DVD blocks (2048 bytes). We convert the kernel's
    512-byte block address to a DVD LBA.
    """
    dvd_lba = kernel_clus >> 2
    dvd_blk_cnt = ceil_div(kernel_size, DVD_BLK_SIZE)
    shift_blks = kernel_clus & 3

    if shift_blks:
        dvd_blk_cnt += 1

    struct.pack_into("<H", boot_loader, BDVD_BLK_LO_OFFSET, dvd_lba & 0xFFFF)
    struct.pack_into(
        "<H", boot_loader, BDVD_BLK_HI_OFFSET, (dvd_lba >> 16) & 0xFFFF
    )
    struct.pack_into("<H", boot_loader, BDVD_BLK_CNT_OFFSET, dvd_blk_cnt)
    struct.pack_into("<H", boot_loader, BDVD_SHIFT_BLKS_OFFSET, shift_blks)


def build_iso(source_dir: str, output_path: str) -> None:
    tools_dir = os.path.dirname(os.path.abspath(__file__))
    boot_loader_path = os.path.join(tools_dir, "boot_dvd.bin")

    if not os.path.exists(boot_loader_path):
        print("Error: boot_dvd.bin not found. Assemble it first:")
        print("  nasm -f bin -o tools/boot_dvd.bin tools/boot_dvd.asm")
        sys.exit(1)

    with open(boot_loader_path, "rb") as f:
        boot_loader = bytearray(f.read())

    print(f"Scanning source directory: {source_dir}")
    root = scan_directory(source_dir, "/")

    # Compute directory sizes
    compute_dir_size(root)

    # Count total data blocks needed
    data_blocks = count_data_blocks(root)
    print(f"  Files/dirs need {data_blocks} blocks ({data_blocks * BLK_SIZE} bytes)")

    # ISO layout: 0-15 system area, 16 PVD, 17 boot rec, 18 SVD, 19 term,
    # 20 El Torito catalog, 21 boot loader
    # drv_offset = 19*4 + (DVD_BLK_SIZE*2 + DVD_BOOT_LOADER_SIZE) / BLK_SIZE
    drv_offset = 19 * 4 + (DVD_BLK_SIZE * 2 + DVD_BOOT_LOADER_SIZE) // BLK_SIZE
    # = 76 + 12 = 88

    # Calculate bitmap blocks iteratively
    # bitmap must track: data_blocks + bitmap_blks (since bitmap is in data area)
    bitmap_blks = 1
    while True:
        new_bitmap_blks = ceil_div(data_blocks + bitmap_blks, BLK_SIZE * 8)
        if new_bitmap_blks == bitmap_blks:
            break
        bitmap_blks = new_bitmap_blks

    # data_area = drv_offset + bitmap_blks
    data_area = drv_offset + bitmap_blks

    # Total data includes: bitmap_blks (self-tracked) + actual data
    total_data_in_area = bitmap_blks + data_blocks

    # max_blk (inclusive, aligned to 4-block DVD boundary)
    max_blk = ceil_to(drv_offset + 1 + bitmap_blks + data_blocks, 4) - 1
    sects = max_blk - drv_offset + 1

    total_iso_blocks = max_blk + 1
    total_iso_bytes = total_iso_blocks * BLK_SIZE

    print(f"  drv_offset={drv_offset}, bitmap_blks={bitmap_blks}, "
          f"data_area={data_area}")
    print(f"  sects={sects}, max_blk={max_blk}")
    print(f"  ISO size: {total_iso_bytes} bytes "
          f"({total_iso_bytes / (1024 * 1024):.1f} MB)")

    # Assign clusters to all files and directories
    # First data block after bitmap self-allocation
    first_data_clus = data_area + bitmap_blks  # skip bitmap's own blocks
    # Actually: data_area is where data starts, and the first bitmap_blks
    # blocks in the data area are the bitmap itself. So real data starts at
    # data_area + bitmap_blks? No...
    #
    # Per TempleOS: data_area = drv_offset + bitmap_sects. The bitmap lives at
    # blocks [drv_offset+1, drv_offset+bitmap_sects]. The first data block is
    # at data_area (= drv_offset + bitmap_sects).
    #
    # In RedSeaFmt, ClusAlloc(0,1,FALSE) allocates the first data block (at
    # data_area). Then root_clus gets the next allocation.
    #
    # But the "Alloc #1" block at data_area overlaps with the last bitmap block.
    # For our purposes, we skip 1 block (the reserved/overlap block) and start
    # real data at data_area + 1.

    first_data_clus = data_area + 1  # skip the reserved "Alloc #1" block
    next_clus = assign_clusters(root, first_data_clus, first_data_clus)

    # Find kernel binary for boot loader patching
    kernel_entry = find_kernel_entry(root)
    if kernel_entry is None:
        print("Error: Could not find 0000Boot/0000Kernel.BIN.C in source tree")
        sys.exit(1)

    print(f"  Kernel at cluster {kernel_entry.clus} "
          f"({kernel_entry.size} bytes, {kernel_entry.blocks} blocks)")

    # Patch boot loader
    patch_boot_loader(boot_loader, kernel_entry.clus, kernel_entry.size)

    # Build the ISO image
    print("Building ISO image...")
    iso = bytearray(total_iso_bytes)

    # Write boot loader at DVD sector 21 (512-byte block 84)
    boot_loader_block = 21 * 4  # = 84
    iso[
        boot_loader_block * BLK_SIZE : boot_loader_block * BLK_SIZE
        + DVD_BOOT_LOADER_SIZE
    ] = boot_loader

    # Write RedSea boot sector
    boot_sector = build_redsea_boot_sector(
        drv_offset, sects, root.clus, bitmap_blks
    )
    iso[drv_offset * BLK_SIZE : drv_offset * BLK_SIZE + BLK_SIZE] = boot_sector

    # Build and write allocation bitmap
    # Bitmap tracks from data_area onwards. We need to mark:
    # - Block at data_area (the reserved "Alloc #1" block)
    # - All file/directory data blocks
    bitmap = build_bitmap(data_area, next_clus - data_area, bitmap_blks)
    bitmap_offset = (drv_offset + 1) * BLK_SIZE
    iso[bitmap_offset : bitmap_offset + len(bitmap)] = bitmap

    # Write directory tree and file data
    write_dir_tree(iso, root, root.clus)

    # Write ISO 9660 headers
    vol_space_size_dvd = total_iso_bytes // DVD_BLK_SIZE
    pvd, boot_rec, svd, term, et_catalog = build_iso9660_headers(
        vol_space_size_dvd, root.clus
    )
    iso[16 * DVD_BLK_SIZE : 17 * DVD_BLK_SIZE] = pvd
    iso[17 * DVD_BLK_SIZE : 18 * DVD_BLK_SIZE] = boot_rec
    iso[18 * DVD_BLK_SIZE : 19 * DVD_BLK_SIZE] = svd
    iso[19 * DVD_BLK_SIZE : 20 * DVD_BLK_SIZE] = term
    iso[20 * DVD_BLK_SIZE : 21 * DVD_BLK_SIZE] = et_catalog

    # Write output
    print(f"Writing {output_path}...")
    with open(output_path, "wb") as f:
        f.write(iso)

    print(f"Done! ISO written to {output_path}")
    print(f"Boot with: qemu-system-x86_64 -cdrom {output_path} -m 512")


def main() -> None:
    if len(sys.argv) < 2:
        # Default: source is parent of tools/ dir
        source_dir = os.path.dirname(
            os.path.dirname(os.path.abspath(__file__))
        )
    else:
        source_dir = sys.argv[1]

    if len(sys.argv) < 3:
        output_path = os.path.join(source_dir, "TempleOS.iso")
    else:
        output_path = sys.argv[2]

    build_iso(source_dir, output_path)


if __name__ == "__main__":
    main()
