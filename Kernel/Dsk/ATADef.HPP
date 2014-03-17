//See $LK,"::/Doc/Acknowledgements.TXT"$.

#define ATA_NOP			0x00
#define ATA_DEV_RST		0x08
#define ATA_PACKET		0xA0
#define ATA_READ_NATIVE_MAX	0xF8
#define ATA_READ_NATIVE_MAX_EXT	0x27
#define ATA_SET_MAX		0xF9
#define ATA_SET_MAX_EXT		0x37
#define ATA_READ_MULTI		0xC4
#define ATA_READ_MULTI_EXT	0x29
#define ATA_WRITE_MULTI		0xC5
#define ATA_WRITE_MULTI_EXT	0x39
#define ATA_ID_DEV		0xEC

#define ATAS_ERR	0x01
#define ATAS_DRQ	0x08
#define ATAS_DF		0x20
#define ATAS_DRDY	0x40
#define ATAS_BSY	0x80

#define ATAR0_DATA	0
#define ATAR0_FEAT	1
#define ATAR0_NSECT	2
#define ATAR0_SECT	3
#define ATAR0_LCYL	4
#define ATAR0_HCYL	5
#define ATAR0_SELECT	6
#define ATAR0_STAT	7
#define ATAR0_CMD	7
#define ATAR1_CTRL	2
