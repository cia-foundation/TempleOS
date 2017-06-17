#!/usr/bin/env python
from __future__ import print_function, absolute_import, division

import base64
import binascii
import bz2
import calendar
import datetime
import logging
import os
import random

from collections import defaultdict
from errno import ENOENT
from stat import S_IFDIR, S_IFLNK, S_IFREG
from sys import argv, exit
from time import time
from time import localtime
from time import strftime

from fuse import FUSE, FuseOSError, Operations, LoggingMixIn

if not hasattr(__builtins__, 'bytes'):
    bytes = str

ISO9660_BOOT_BLK = bz2.decompress(base64.b64decode("""
QlpoOTFBWSZTWf7EvQUAAFv//fREJgRSAWAALyXeECYGQAQAQBkAABAgCACAEAAACLAAuSEpAk0bS
NNBpoNHqNNHqD1NDAGmho0YjIBoANDBFJINGjQAAAABo67j0bZyRUa5yTYIQ4kKTEIUwAXoAFqrSW
4rKEBSAhABlCEiGmEFTMYxS2moWhFsmA6oWhYpKyEGJZcHEC6HB+P3rMF7qCe92GE9TJlRStvcUio
nnIx8jGJrKA+I80GdJI5JNH6AqxFBNVCiJ+/i7kinChIf2JegoA==
"""))

epoch = datetime.datetime.utcfromtimestamp(0)

CDATE_YEAR_DAYS_INT =   36524225
CDIR_FILENAME_LEN   =   38
RS_ATTR_READ_ONLY   =   0x01      #R
RS_ATTR_HIDDEN      =   0x02      #H
RS_ATTR_SYSTEM      =   0x04      #S
RS_ATTR_VOL_ID      =   0x08      #V
RS_ATTR_DIR         =   0x10      #D
RS_ATTR_ARCHIVE     =   0x20      #A
RS_ATTR_DELETED     =   0x100     #X
RS_ATTR_RESIDENT    =   0x200     #T
RS_ATTR_COMPRESSED  =   0x400     #Z
RS_ATTR_CONTIGUOUS  =   0x800     #C
RS_ATTR_FIXED       =   0x1000    #F
RS_BLK_SIZE         =   512
RS_DRV_OFFSET	    =   0xB000
RS_ROOT_CLUS        =   0x5A

mon_start_days1=[
0,31,59,90,120,151,181,212,243,273,304,334]
mon_start_days2=[
0,31,60,91,121,152,182,213,244,274,305,335]

def roundup(x): return x if x % 2048 == 0 else x + 2048 - x % 2048

def write_iso_c(self, iso_c_file, pad):
    dirs = []
    dir_entries = {}    

    # Create dict for each directory
    for i in self.files:
        if self.files[i]['st_mode'] & 40000 == 64:
            dir_entries[i] = {}
            dir_entries[i]['clus'] = 0
            dir_entries[i]['files'] = []
            dir_entries[i]['files'].append({'filename':'.','clus':0,'st_size':0x400,'st_mode':64,'st_mtime':time()})
            dir_entries[i]['files'].append({'filename':'..','clus':0,'st_size':0x00,'st_mode':64,'st_mtime':time()})
            dirs.append(i)    

    # Place files in corresponding dicts
    for d in sorted(dirs, reverse=True):
        for i in self.files:    
            if i.find((d+"/").replace("//","/")) != -1 and i != d:
                if 'filename' not in self.files[i]:
                    self.files[i]['filename'] = i.split('/')[len(i.split('/'))-1]
                    self.files[i]['clus'] = 0
                    dir_entries[d]['files'].append(self.files[i])    

    # Calculate CDirEntry clusters
    de_tbl_size = 0
    de_clus_ctr = RS_ROOT_CLUS
    for d in sorted(dir_entries):
        ct_entries = 1+len(dir_entries[d]['files'])
        ct_size = RS_BLK_SIZE * int((1+(ct_entries*64) / RS_BLK_SIZE)) 
        de_tbl_size += ct_size
        dir_entries[d]['clus'] = de_clus_ctr
        de_clus_ctr += int((ct_size / RS_BLK_SIZE))    

    # Link nested CDirEntries
    for d in dirs:
        de_filename = d[d.rfind("/")+1:]
        if len(d.split("/")) > 2:
            de_parent = d[:d.rfind("/")] 
        else:
            de_parent = "/"
        de_idx = 0
        for de in dir_entries[de_parent]['files']:
            if de['filename'] == de_filename:
                dir_entries[de_parent]['files'][de_idx]['clus'] = dir_entries[(de_parent + "/" + de_filename).replace("//","/")]['clus']
            de_idx += 1        

    # Link dotted dirs ".", ".."
    for de_parent in dir_entries:
        de_idx = 0
        for de in dir_entries[de_parent]['files']:
            if de['filename'] == ".":
                    dir_entries[de_parent]['files'][de_idx]['clus'] = dir_entries[de_parent]['clus']
                    # Calculate length of this directory
                    dir_entries[de_parent]['files'][de_idx]['st_size'] = RS_BLK_SIZE * (1+int(64*(len(dir_entries[de_parent]['files'])) / RS_BLK_SIZE))
            if de['filename'] == "..":
                    dir_entries[de_parent]['files'][de_idx]['clus'] = dir_entries[("/" + de_parent[:de_parent.rfind("/")]).replace("//","/")]['clus']
            de_idx += 1    

    # Update size for subdir entries
    for de_parent in dir_entries:
        de_idx = 0
        for de in dir_entries[de_parent]['files']:
            if de['st_mode'] & 40000 == 64 and de['filename'] != "." and de['filename'] != "..":
                for sde in dir_entries[(de_parent+"/"+de['filename']).replace("//","/")]['files']:
                    if sde['filename'] == ".":
                        dir_entries[de_parent]['files'][de_idx]['st_size'] = sde['st_size']
            de_idx += 1

    # Calculate cluster offset for files
    for de_parent in dir_entries:
        de_idx = 0
        for de in dir_entries[de_parent]['files']:
            if de['clus'] == 0:
                dir_entries[de_parent]['files'][de_idx]['clus'] = de_clus_ctr
                ct_size = RS_BLK_SIZE * int((1+(de['st_size']) / RS_BLK_SIZE))
                de_clus_ctr += int((ct_size / RS_BLK_SIZE))
            de_idx += 1    

    file = open(iso_c_file, "wb")    

    # Write ISO9660 boot block
    file.seek(0)
    file.write(ISO9660_BOOT_BLK)    

    # Write CDirEntries
    for d in sorted(dirs):
        de_offset = int(dir_entries[d]['clus']*RS_BLK_SIZE)
        for f in sorted(dir_entries[d]['files'], key=lambda k: k['filename']):
            file.seek(de_offset)
            if f['st_mode'] & 40000 == 64:
                # Directory
                file.write('\x10')
            else:
                # File
                file.write('\x20')
            if f['filename'][-2:] == ".Z":
                file.write('\x0c')
            else:
                file.write('\x08')
            file.write(f['filename'].ljust(CDIR_FILENAME_LEN,'\x00'))            

            # Cluster
            file.seek(de_offset+0x28)
            clus_bige = binascii.unhexlify(format(int(f['clus']), '016X'))
            clus_bige_ctr = 0
            while clus_bige_ctr < 8:
                file.write(chr(ord(clus_bige[7-clus_bige_ctr])))
                clus_bige_ctr += 1    

            file.seek(de_offset+0x30)
            # Size
            size_bige = binascii.unhexlify(format(int(f['st_size']), '016X'))
            size_bige_ctr = 0
            while size_bige_ctr < 8:
                file.write(chr(ord(size_bige[7-size_bige_ctr])))
                size_bige_ctr += 1    

            # DateTime
            file.seek(de_offset+0x38)
            dt_bige = Unix2CDate(localtime(f['st_mtime']+1))
            dt_bige_ctr = 0
            while dt_bige_ctr < 8:
                file.write(chr(ord(dt_bige[7-dt_bige_ctr])))
                dt_bige_ctr += 1

            de_offset += 64    

    # Write files
    for d in sorted(dirs):
        de_offset = int(dir_entries[d]['clus']*RS_BLK_SIZE)
        for f in sorted(dir_entries[d]['files'], key=lambda k: k['filename']):
            file.seek(de_offset)
            if f['st_mode'] & 40000 != 64:
                file.seek(int(f['clus']*RS_BLK_SIZE))
                file.write(self.data[(d+'/'+f['filename']).replace("//","/")])    

    # If --2k, Set counter to byte multiple 2048
    if int(pad) == 1:
        de_clus_ctr=roundup(de_clus_ctr)

    # Write to EOF
    file.seek(RS_DRV_OFFSET+int(de_clus_ctr*RS_BLK_SIZE)-1)
    file.write(chr(0))    

    # Write boot sector    

    bs = [0x00]*RS_BLK_SIZE
    bs[0x003] = 0x88 # signature
    bs[0x008] = 0x58 # cdrom offset    

    # -- cluster count
    cl_sz = binascii.unhexlify(format(int(de_clus_ctr), '016X'))
    cl_sz_ctr = 0
    while cl_sz_ctr < 8:
        bs[0x10+cl_sz_ctr] = ord(cl_sz[7-cl_sz_ctr])
        cl_sz_ctr += 1
    bs[0x018] = 0x5A # root entry cluster [90]
    bs[0x020] = 0x01 # bitmap_sects (unused for ISO.C?)
    # -- unique id
    id_ctr = 0
    while id_ctr<8:
        bs[0x028+id_ctr] = int(random.random()*256) % 256
        id_ctr += 1
    bs[0x1FE] = 0x55 # signature
    bs[0x1FF] = 0xAA # signature    

    file.seek(RS_DRV_OFFSET)
    for byte in bs:
        file.write(chr(byte))
    file.close()

def CDate2Unix(c_date, c_time):
    year=int((c_date+1)*100000/CDATE_YEAR_DAYS_INT)

    c_year = int(year)

    i=YearStartDate(c_year)
    while i > c_date:
        c_year -= 1
        i=YearStartDate(c_year)
    c_date -= i

    c_date = int(c_date)
    if calendar.isleap(year) and c_date>29:
        c_date += 1

    k=(625*15*15*3*c_time) >> 21
    min = int(k/100/100/60 % 60) 
    hour = int(k/100/100/60/60)

    # lol, timezones
    i_dst = 0
    if strftime("%Z") == "EDT":
        chk_date = datetime.datetime(year, 1, 1, hour, min, 0)
        if is_dst(chk_date):
            i_dst = 1
    return (datetime.datetime(year, 1, 1, hour, min, 0)-epoch+datetime.timedelta(days=int(c_date), hours=i_dst+(-int(strftime("%z"))/100))).total_seconds()

def is_dst(dt):
    if dt.year < 2007:
        # huehuehue
        return False
    dst_start = datetime.datetime(dt.year, 3, 8, 2, 0)
    dst_start += datetime.timedelta(6 - dst_start.weekday())
    dst_end = datetime.datetime(dt.year, 11, 1, 2, 0)
    dst_end += datetime.timedelta(6 - dst_end.weekday())
    return dst_start <= dt < dst_end

def YearStartDate(year):
    y1=year-1
    yd4000=y1/4000
    yd400=y1/400
    yd100=y1/100
    yd4=y1/4;
    return year*365+yd4-yd100+yd400-yd4000

def Unix2CDate(dt):
    il=YearStartDate(dt.tm_year)
    i2=YearStartDate(dt.tm_year+1)
    if (i2-il==365):
        il += mon_start_days1[dt.tm_mon-1]
    else:
        il += mon_start_days2[dt.tm_mon-1]
    _date = il + (dt.tm_mday-1);
    _time=(100*(100*(dt.tm_sec+60*(dt.tm_min+60*dt.tm_hour)))<<21)/(15*15*3*625)
    return binascii.unhexlify(format(int(_date), '08X')) + binascii.unhexlify(format(int(_time), '08X'))

class RedSea(LoggingMixIn, Operations):

    def __init__(self, iso_c_file):

        self.files = {}
        self.data = defaultdict(bytes)
        self.fd = 0
        self.modified = False
        now = time()

        self.files['/'] = dict(st_mode=(S_IFDIR | 0o755), st_ctime=now,
                               st_mtime=now, st_atime=now, st_nlink=2)

        if os.path.exists(iso_c_file):
            f = open(iso_c_file, "rb").read()
            blkdev_offset = RS_DRV_OFFSET
            de_list = { '': blkdev_offset + int((1+int(f[blkdev_offset+0x20]))*RS_BLK_SIZE) }
        else:
            de_list = {}

        # If iso_c_file exists, read dirs/files
        while len(de_list) > 0 and os.path.exists(iso_c_file):
            dir = next(iter(de_list.keys())) + '/'
            ofs = de_list[next(iter(de_list.keys()))]

            del(de_list[next(iter(de_list.keys()))])
            # Go to first CDirEntry, skipping ".", ".."
            pos = 128
            while f[ofs+pos+2] != 0x00:
                ctr = int(ofs+pos)
                de_attrs = u16(f[ctr:ctr+2])
                ctr += 2
                de_filename = f[ctr:ctr+CDIR_FILENAME_LEN].replace(b'\x00',b'').decode('ascii')
                ctr += CDIR_FILENAME_LEN
                de_clus = i64(f[ctr:ctr+8])
                ctr += 8
                de_size = i64(f[ctr:ctr+8])
                ctr += 8
                de_time = i32(f[ctr:ctr+4])
                de_date = i32(f[ctr+4:ctr+8])
                ctr += 8
                de_mode = S_IFREG | 0o755

                if de_attrs & RS_ATTR_DIR:
                    de_mode = S_IFDIR | 0o755  
                    de_list[dir + de_filename] = ((int(de_clus))*RS_BLK_SIZE)

                self.files[dir + de_filename] = dict(st_mode=de_mode, st_ctime=CDate2Unix(de_date, de_time),
                                           st_mtime=CDate2Unix(de_date, de_time), st_atime=now, st_nlink=2, st_size=de_size) 
                if not de_attrs & RS_ATTR_DIR:
                    self.data[dir + de_filename] = f[RS_BLK_SIZE*de_clus:(RS_BLK_SIZE*de_clus)+de_size]
                pos += 64

        if os.path.exists(iso_c_file):
            del(f)

    def chmod(self, path, mode):
        self.files[path]['st_mode'] &= 0o770000
        self.files[path]['st_mode'] |= mode
        return 0

    def chown(self, path, uid, gid):
        self.files[path]['st_uid'] = uid
        self.files[path]['st_gid'] = gid

    def create(self, path, mode):
        self.files[path] = dict(st_mode=(S_IFREG | mode), st_nlink=1,
                                st_size=0, st_ctime=time(), st_mtime=time(),
                                st_atime=time())

        self.fd += 1
        self.modified = True
        return self.fd

    def getattr(self, path, fh=None):
        if path not in self.files:
            raise FuseOSError(ENOENT)
        return self.files[path]

    def getxattr(self, path, name, position=0):
        attrs = self.files[path].get('attrs', {})

        try:
            return attrs[name]
        except KeyError:
            return ''       # Should return ENOATTR

    def listxattr(self, path):
        attrs = self.files[path].get('attrs', {})
        return attrs.keys()

    def mkdir(self, path, mode):
        self.files[path] = dict(st_mode=(S_IFDIR | mode), st_nlink=2,
                                st_size=0, st_ctime=time(), st_mtime=time(),
                                st_atime=time())

        self.files['/']['st_nlink'] += 1
        self.modified = True

    def open(self, path, flags):
        self.fd += 1
        return self.fd

    def read(self, path, size, offset, fh):
        return self.data[path][offset:offset + size]

    def readdir(self, path, fh):
            dir = [ '.', '..' ]
            if path == '/':
                for f in self.files:
                    if f.rfind('/') == 0 and f != '/':
                        dir.append(str(f)[1:])
            else:
                for f in self.files:
                    if f.startswith(path):
                        if f[len(path):].rfind('/') == 0 and f[len(path):] != '/':
                            dir.append(str(f[len(path):])[1:])
            return dir

    def readlink(self, path):
        return self.data[path]

    def removexattr(self, path, name):
        attrs = self.files[path].get('attrs', {})

        try:
            del attrs[name]
        except KeyError:
            pass        # Should return ENOATTR

    def rename(self, old, new):
        self.files[new] = self.files.pop(old)
        self.modified = True

    def rmdir(self, path):
        self.files.pop(path)
        self.files['/']['st_nlink'] -= 1
        self.modified = True

    def setxattr(self, path, name, value, options, position=0):
        # Ignore options
        attrs = self.files[path].setdefault('attrs', {})
        attrs[name] = value

    def statfs(self, path):
        size = 256
        return dict(f_bsize=RS_BLK_SIZE, f_blocks=((size*1024)*2), f_bavail=((size*1024)*2))

    def symlink(self, target, source):
        self.files[target] = dict(st_mode=(S_IFLNK | 0o777), st_nlink=1,
                                  st_size=len(source))

        self.data[target] = source

    def truncate(self, path, length, fh=None):
        self.data[path] = self.data[path][:length]
        self.files[path]['st_size'] = length

    def unlink(self, path):
        self.files.pop(path)
        self.modified = True

    def utimens(self, path, times=None):
        now = time()
        atime, mtime = times if times else (now, now)
        self.files[path]['st_atime'] = atime
        self.files[path]['st_mtime'] = mtime

    def write(self, path, data, offset, fh):
        self.data[path] = self.data[path][:offset] + data
        self.files[path]['st_size'] = len(self.data[path])
        self.modified = True
        return len(data)

    def destroy(self, d):
        if self.modified and argv[3] == "rw":
            write_iso_c(self, argv[1], argv[4])

def u16(s):
    u = s[0]
    u += (256**1)*s[1]
    return u

def i32(s):
    # Not handled correctly (yet) but idgaf
    c = 1
    u = s[0]
    while c < 4:
        u += (256**c)*s[c]
        c += 1
    return u

def i64(s):
    # Not handled correctly (yet) but idgaf
    c = 1
    u = s[0]
    while c < 8:
        u += (256**c)*s[c]
        c += 1
    return u

if __name__ == '__main__':
    fuse = FUSE(RedSea(argv[1]), argv[2], foreground=True)
