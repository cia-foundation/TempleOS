#!/usr/bin/env python

from __future__ import print_function

import hashlib
import os
import subprocess
import sys
import urllib

ISO_URL = 'http://www.templeos.org/TempleOSCD.ISO'
FILE_NAME = 'TempleOSCD.iso'
EXTRACTED_ISO_DIR = 'TempleOSCD'

force = (len(sys.argv) > 1 and sys.argv[1] == '-f')

def hash_of_file(path):
    with open(path, 'rb') as f:
        sha1 = hashlib.sha1()
        while True:
            data = f.read(0x10000)
            if not data: break

            sha1.update(data)

        return sha1.hexdigest()

if os.path.exists(FILE_NAME) and not force:
    pre_hash = hash_of_file(FILE_NAME)
else:
    pre_hash = None

print('-- Pre-hash:', pre_hash)

print('-- Grabbing', ISO_URL)
urllib.urlretrieve(ISO_URL, FILE_NAME)

post_hash = hash_of_file(FILE_NAME)
print('-- Post-hash:', post_hash)

if pre_hash != post_hash:
    print('-- Hashes differ - updating code!')
    print()
    print('---- Extracting', FILE_NAME)
    subprocess.check_call(['./extract_templeos.py', FILE_NAME, EXTRACTED_ISO_DIR])

    print('---- Adding to git')
    subprocess.call(['git', 'add', '-A', EXTRACTED_ISO_DIR])

    # Check if anything changed
    if subprocess.call(['git', 'diff-index', '--quiet', 'HEAD', '--', EXTRACTED_ISO_DIR]) == 1:
        print('---- Commit & push')
        subprocess.check_call(['git', 'commit', '-m', 'Nightly update (ISO SHA-1 %s)' % post_hash[0:7]])
        subprocess.check_call(['git', 'push', 'origin', 'master'])
    else:
        print('---- No changes in source tree')
else:
    print('-- No changes detected.')
