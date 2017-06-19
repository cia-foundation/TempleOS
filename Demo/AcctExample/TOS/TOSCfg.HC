#help_index "Misc/TOS/Cfg"

#define SLOP	BLK_SIZE

U0 TOSDbgDistro1()
{
  CBinFile *bfh=mem_boot_base-sizeof(CBinFile);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint("TOS_DBG_DISTRO","0x%X",
	CeilI64(SYS_KERNEL_END+SLOP,BLK_SIZE));
  DefinePrint("TOS_DBG_DISTRO_END",  "0x%X",FloorI64(
	(BOOT_RAM_LIMIT-(BOOT_STK_SIZE+DVD_BOOT_LOADER_SIZE))>>4<<4-
	(bfh(I64)-SYS_KERNEL_END)-SLOP,BLK_SIZE));
  DefinePrint("CFG_DBG_DISTRO_FILE","\"/Tmp/DbgDistro.BIN.Z\"");
} TOSDbgDistro1;

U0 TOSDbgDistro2()
{
  DefinePrint("CFG_DBG_DISTRO","\"a0x%X\n0x%X\n\"",TOS_DBG_DISTRO,
	(TOS_DBG_DISTRO_END-TOS_DBG_DISTRO)/BLK_SIZE);
  DefinePrint("CFG_DBG_DISTRO_START","\"0x%X\"",TOS_DBG_DISTRO);
} TOSDbgDistro2;

U0 TOSInit()
{
  switch (INS_REG_MACHINE_NUM) {
    case 1: //TAD Native Machine
      DefinePrint("CFG_RAM_DRVS",
	    "\"B\nScale2Mem(2048,0x100000,4*1024*1024*1024)\n\"");
      DefinePrint("CFG_HARD_DRVS","\"C\ns0xFE00\n0xFE10\n0\"");
      DefinePrint("CFG_DVD_DRVS","\"Ts0x1F0\n1\"");
      DefinePrint("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      DefinePrint("TOS_HDS","\"CD\"");
      DefinePrint("TOS_MASTER_BOOT_DRVS","\"C\"");
      break;
    default:
      DefinePrint("CFG_RAM_DRVS",
	    "\"B\nScale2Mem(2048,0x100000,4*1024*1024*1024)\n\"");
      DefinePrint("CFG_HARD_DRVS","\"\"");
      DefinePrint("CFG_DVD_DRVS","\"\"");
      DefinePrint("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      DefinePrint("TOS_HDS","\"CD\"");
      DefinePrint("TOS_MASTER_BOOT_DRVS","\"C\"");
  }
} TOSInit;

#define TOS_ISO_NAME	"B:/TOS_Distro.ISO.C"
#define TOS_DISTRO_DIR	"B:/Distro"
#define CFG_OPTS	"StaffMode\nMountIDEAuto\nCT\n"
#define CFG_DBG_OPTS	"StaffMode\nMountIDEAuto\nCT"\
			"HeapInit\n130\nMemInit\n131\nVarInit\n132\n\n"
#define CFG_DBGZ_OPTS	"StaffMode\nMountIDEAuto\nCT"\
			"HeapInit\n0\nMemInit\n0\nVarInit\n0\n\n"
#define TOS_CFG		"\n" CFG_RAM_DRVS CFG_DVD_DRVS CFG_HARD_DRVS "\n"\
			CFG_DSK_CACHE CFG_OPTS
#define TOS_DVD_CFG	"TB\n0x20000\nT \n\n\nStaffMode\nMountIDEAuto\nCT\n"
#define TOS_DVD_DBG_CFG "A" CFG_DBG_DISTRO\
			"B\nScale2Mem(2048,0x40000)\n\n\n"\
			"NoMP\nTextMode\nDontProbe\nDbgDistro\n"\
			"C:" CFG_DBG_DISTRO_FILE "\n"\
			CFG_DBG_DISTRO_START "\n\n"
