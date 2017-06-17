#!/usr/bin/env python3

import sys
sys.path.append('redseafs')

import errno
from isoc import RedSea
import os
import subprocess

ISO_FILE = sys.argv[1]
OUTPUT_DIR = sys.argv[2]

S_IFDIR  = 0o040000  # directory

iso = RedSea(ISO_FILE)

def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def extract(iso_path, path):
    make_sure_path_exists(path)

    for entry in iso.readdir(iso_path if len(iso_path) else '/', -1):
        if entry[0] == '.': continue

        iso_full_path = iso_path + '/' + entry
        full_path = os.path.join(path, entry)
        stat = iso.getattr(iso_full_path)

        if stat['st_mode'] & S_IFDIR:
            extract(iso_full_path, full_path)
        else:
            open(full_path, 'wb').write(iso.read(iso_full_path, stat['st_size'], 0, -1))

def decompress_all_files_in(path):
    for item in os.listdir(path):
        full_path = os.path.join(path, item)

        if os.path.isdir(full_path):
            decompress_all_files_in(full_path)
        elif full_path.endswith('.Z'):
            decompressed_path = full_path[0:len(full_path)-2]
            subprocess.check_call(['./TOSZ', '-ascii', full_path, decompressed_path])
            os.remove(full_path)

# Extract TempleOS disk tree
extract('', OUTPUT_DIR)

# Decompress compressed files
decompress_all_files_in(OUTPUT_DIR)
