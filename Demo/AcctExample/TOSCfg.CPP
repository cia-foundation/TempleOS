#help_index "TOS"

#define PERSONAL_WEB		"http:/" "/www.templeos.org/Wb"
#define PERSONAL_INITIALS	"TAD"
#define BLOG_BASE		"/Home/Web/" PERSONAL_INITIALS "/Blog"
#define DAILY_BASE		"/Home/Web/" PERSONAL_INITIALS "/DailyBlog"
#define BLOG_LOCAL_BASE		"::" BLOG_BASE
#define BLOG_WEB_BASE		PERSONAL_WEB BLOG_BASE

#define SLOP	BLK_SIZE
U0 TOSBootRescueDrv1()
{
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint("TOS_BOOT_RESCUE_DRV","0x%X",
	CeilI64(SYS_KERNEL_END+SLOP,BLK_SIZE));
  DefinePrint("TOS_BOOT_RESCUE_DRV_END",  "0x%X",FloorI64(
	(BOOT_RAM_LIMIT-(BOOT_STK_SIZE+DVD_BOOT_LOADER_SIZE))>>4<<4-
	(bfh(I64)-SYS_KERNEL_END)-SLOP,BLK_SIZE));
  DefinePrint("CFG_BOOT_RESCUE_DRV_FILE","\"/Temp/BootRescueDrv.BIN.Z\"");
} TOSBootRescueDrv1;

U0 TOSBootRescueDrv2()
{
  DefinePrint("CFG_BOOT_RESCUE_DRV","\"a0x%X\n0x%X\n\"",TOS_BOOT_RESCUE_DRV,
	(TOS_BOOT_RESCUE_DRV_END-TOS_BOOT_RESCUE_DRV)/BLK_SIZE);
  DefinePrint("CFG_BOOT_RESCUE_DRV_START","\"0x%X\"",TOS_BOOT_RESCUE_DRV);
} TOSBootRescueDrv2;

U0 TOSInit()
{
  DefinePrint("CFG_RAM_DRVS",
	"\"b\nScale2Mem(2048,0x100000,4*1024*1024*1024)\n\"");
  DefinePrint("CFG_HARD_DRVS","\"\"");
  DefinePrint("CFG_DVD_DRVS","\"\"");
  DefinePrint("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
  DefinePrint("TOS_HDS","\"CD\"");
  DefinePrint("TOS_MASTER_BOOT_DRVS","\"C\"");
} TOSInit;

#define TOS_ISO_NAME	"B:/TempleOSCD.ISO"
#define TOS_DISTRO_DIR	"B:/Distro"
#define CFG_OPTS	"AutoMountIDE\nCT\n"
#define CFG_DBG_OPTS	"AutoMountIDE\nCT"\
			"HeapInit\n130\nMemInit\n131\nVarInit\n132\n\n"
#define CFG_DBGZ_OPTS	"AutoMountIDE\nCT"\
			"HeapInit\n0\nMemInit\n0\nVarInit\n0\n\n"
#define TOS_CFG		"\n" CFG_RAM_DRVS CFG_DVD_DRVS CFG_HARD_DRVS "\n"\
			CFG_DSK_CACHE CFG_OPTS
#define TOS_DVD_CFG	"Tb\n0x20000\nt \n\n\nAutoMountIDE\nCT\n"
#define TOS_DVD_DBG_CFG "A" CFG_BOOT_RESCUE_DRV\
			"b\nScale2Mem(2048,0x40000)\n\n\n"\
			"DontProbe\nBootRescueDrv\n"\
			"C:" CFG_BOOT_RESCUE_DRV_FILE "\n"\
			CFG_BOOT_RESCUE_DRV_START "\n\n"
