                           SparrowOS

You can't do much until you burn a SparrowOS CD-ROM from the ISO file
and boot it, or you aim your virtual machine at the ISO file and boot.
SparrowOS files are compressed and the source code can only be 
compiled by the SparrowOS compiler... which is available when you boot
the CD-ROM.  SparrowOS is 100% open source with all source present.

SparrowOS is 64-bit and will not run on 32-bit hardware.

SparrowOS may require you to enter I/O port addresses for the CD-ROM drive
and the hard drive.  In Windows, you can find I/O port info in the
Accessories folder, System Tools, System Info, Hardware Resources, 
I/O ports.  Look for and write down "IDE", "ATA" or "SATA" port numbers.
Then, boot the SparrowOS CD and try all combinations.  (Sorry, it's too
difficult for SparrowOS to figure-out port numbers, automatically.)

The source code can also be found at the SparrowOS web site, 
http://www.sparrowos.com but cannot be compiled outside SparrowOS because
it's a nonstandard language, a dialect of C/C++ and asm.
