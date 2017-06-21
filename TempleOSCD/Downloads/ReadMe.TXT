                           TempleOS

You can't do anything until you burn a TempleOS CD/DVD from the ISO file
and boot it, or you aim your virtual machine's CD/DVD at the ISO file
and boot.

TempleOS is 64-bit and will not run on 32-bit hardware.

TempleOS requires 512 Meg of RAM minimum and can have 256 Gig of RAM or more!

TempleOS files are compressed with a nonstandard LZW format and the source
code can only be compiled by the TempleOS compiler because it is HolyC, a
nonstandard C/C++ dialect.  You must boot TempleOS.  Then, you can compile it
because it is 100% open source and all source present on the distro.

If attempting to run on native hardware, TempleOS may require you to enter I/O
port addresses for the CD/DVD drive and the hard drive.  In Windows, you can
find I/O port info in the Accessories/System Tools/System Info/Hardware
Resources/I/O ports.  Look for and write down "IDE", "ATA" or "SATA" port numbers.
In Linux, use "lspci -v".  Then, boot the TempleOS CD and try all combinations.
(Sorry, it's too difficult for TempleOS to figure-out port numbers, automatically.)


