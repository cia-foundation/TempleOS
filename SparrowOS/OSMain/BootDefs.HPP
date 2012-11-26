//See $LK,"MP_VECT_ADDR","FF:::/SparrowOS/OSMain/Adam1a.HPP.Z,MP_VECT_ADDR"$ for something
//else used in low RAM.
//See $LK,"::/SparrowOS/Doc/Mem.TXT.Z","FI:::/SparrowOS/Doc/Mem.TXT.Z"$.

//At least $LK,"DFT_CACHE_LINE_WIDTH","MN:DFT_CACHE_LINE_WIDTH"$ alignment.
#define BOOT_RAM_BASE		0x07C00
#define BOOT_RAM_LIMIT		0x80000

//9FC00 is EBDA BIOS area
#define VGA_GRAPHIC_MEM_BASE	0xA0000
#define VGA_TEXT_MEM_BASE	0xB8000

#define BLK_SIZE_BITS	9
#define BLK_SIZE	(1<<BLK_SIZE_BITS)
#define CD_BLK_SIZE	(4*BLK_SIZE)
#define CD_FILE_OFFSET	80

#define CD_BOOT_LOADER_SIZE	CD_BLK_SIZE*1 //Max 4 CDBlks, I think

//These are the cfguration settings
//for a standard production CD-Image
#define PROD_CDROM_CFG	 "b1Scale2Mem(2048,0x40000)\nt3 \n\n\n\n\n"
#define SAFE_CDROM_CFG	 "b1Scale2Mem(2048,0x40000)\nt3 \n\n\n\nDontProbe\n\n"

#define BOOT_STK_SIZE	BLK_SIZE

#define BOOT_CODE_RAM		1
#define BOOT_CODE_HARDDRV	2
#define BOOT_CODE_CDROM		3

