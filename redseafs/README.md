# redseafs
FUSE implementation of TempleOS RedSea file system


This is a proof-of-concept, it will probably get better. (Time zones are not handled correctly.)


Currently, you can use redseafs to create/modify/read RedSea ISO.C files on any system that supports FUSE.


# Commands

`isoc-mount [--rw] <filename.ISO.C> <mount_point>` will mount an ISO.C image on `mount_point`

Specify `--rw` to commit writes to ISO.C file, otherwise discarded on unmount.

Specify `--2k` to pad ISO.C file to multiple of 2048 bytes, for compatibility with VirtualBox virtual CD or physical disc ONLY 

(2k padded ISO.C files will not mount with TempleOS `MountFile()`, you will get `ERROR: Not RedSea`)

If the ISO.C file does not exist, a blank filesystem will be created (and written on unmount if `--rw` specified.)

`fusermount -u <mount_point>` to unmount

# Installation

Clone the repo, move `isoc-mount` and `isoc.py` to `/usr/bin`, `chmod +x`.

On a Debian/Ubuntu system: `sudo apt install fuse; sudo apt install python-pip; sudo pip install fusepy`

NOTE: This will install fusepy globally, if that's not what you want... then you probably don't need instructions anyway :P

# Prerequisites

- FUSE
- pip install: fusepy
