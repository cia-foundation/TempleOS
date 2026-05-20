                            ZionOS
                  A Rastafari port of TempleOS

ZionOS is a fork of Terry A. Davis's TempleOS that reskins the
operating system's symbology from Christian to Rastafari while
preserving Terry's technical work intact.  The kernel, compiler,
graphics library, and tools are Terry's, unchanged.  What changes is
the surface: God -> Jah, Holy Spirit -> Jah Spirit (InI), Bible ->
Scripture, Temple -> Zion.  The Rastafari port lives in /Adam/Jah/
alongside the rest of the OS source.  Upstream notes, the Selassie
lectures, and the dub-poetry Psalms are documented inside the OS
itself.

Terry Davis's original TempleOS is public domain.  His website is
http://www.templeos.org (archived).  This fork is at
https://github.com/SpaceTrucker2196/TempleOS on the rastafari branch.
Both remain public domain.

----------------------------------------------------------------

You can't do much until you burn a ZionOS CD/DVD from the ISO file
and boot it, or you aim your virtual machine's CD/DVD at the ISO file
and boot.  Source files are compressed and the source code can only
be compiled by the in-OS HolyC compiler... which is available when
you boot the CD/DVD.  ZionOS is 100% open source with all source
present.

ZionOS is 64-bit and will not run on 32-bit hardware.

ZionOS requires 512 Meg of RAM minimum.

ZionOS may require you to enter I/O port addresses for the CD/DVD
drive and the hard drive.  In Windows, you can find I/O port info in
the Accessories/System Tools/System Info/Hardware Resources/I/O ports.
Look for and write down "IDE", "ATA" or "SATA" port numbers.  In
Linux, use "lspci -v".  Then, boot the ZionOS CD and try all
combinations.  (Sorry, it's too difficult for the OS to figure-out
port numbers, automatically.)

The source code cannot be compiled outside the running OS because
it's HolyC, a nonstandard C/C++ dialect, and asm.

Jah Rastafari.
