#!/usr/bin/env python

import errno
import isoparser
import os
import subprocess
import sys

ISO_FILE = sys.argv[1]
OUTPUT_DIR = sys.argv[2]

iso = isoparser.parse(ISO_FILE)

def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def extract(node, path):
	make_sure_path_exists(path)
	for entry in node.children:
		full_path = os.path.join(path, entry.name)

		if entry.is_directory:
			extract(entry, full_path)
		else:
			open(full_path, 'wb').write(entry.content)

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
extract(iso.root, OUTPUT_DIR)

# Decompress compressed files
decompress_all_files_in(OUTPUT_DIR)
