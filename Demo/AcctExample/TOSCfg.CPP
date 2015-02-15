#define SLOP	BLK_SIZE
U0 TOSBootRescueDrv1()
{
  CBinFile *bfh=sys_boot_base-sizeof(CBinFile);
  bfh(I64)+=bfh->file_size-1;
  DefinePrint("TOS_BOOT_RESCUE_DRV","0x%X",CeilI64(SYS_KERNEL_END+SLOP,BLK_SIZE));
  DefinePrint("TOS_BOOT_RESCUE_DRV_END",  "0x%X",FloorI64(
    (BOOT_RAM_LIMIT-(BOOT_STK_SIZE+CD_BOOT_LOADER_SIZE))>>4<<4-
    (bfh(I64)-SYS_KERNEL_END)-SLOP,BLK_SIZE));
  DefinePrint("CFG_BOOT_RESCUE_DRV_FILE","\"/Temp/BootRescueDrv.BIN.Z\"");
} TOSBootRescueDrv1;

U0 TOSBootRescueDrv2()
{
  DefinePrint("CFG_BOOT_RESCUE_DRV","\"a10x%X\n0x%X\n\"",TOS_BOOT_RESCUE_DRV,
    (TOS_BOOT_RESCUE_DRV_END-TOS_BOOT_RESCUE_DRV)/BLK_SIZE);
  DefinePrint("CFG_BOOT_RESCUE_DRV_START","\"0x%X\"",TOS_BOOT_RESCUE_DRV);
} TOSBootRescueDrv2;

U0 TOSInit()
{
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL;
  CBlkDev *bd;
  switch (tos_cid) {
    case CID_DFT:
      DefinePrint("CFG_RAM_DRVS","\"b1\nScale2Mem(2048,0x280000,4*1024*1024*1024)\n\"");
      if (dsk.auto_mount_ide_cnt) {
	//If AutoMount was successful, No need to manually mount in next compile
	DefinePrint("CFG_HARD_DRVS","\"\"");
	DefinePrint("CFG_CDROM_DRVS","\"\"");
      } else {

	//These two are a crazy guess.	ATA:IDE Primary  ATAPI:IDE Secondary
	DefinePrint("CFG_HARD_DRVS","\"c2s0x1F0\n0x3F4\n0\"");
	DefinePrint("CFG_CDROM_DRVS","\"t3s0x170\n0\"");

	bd=Let2BlkDev(':');
	if (bd->type==BDT_ATAPI) {
	  ATARep(FALSE,,&root);
	  ATAIDDrvs(root,dsk.boot_drv_let,&ata_drv,&atapi_drv);
	  if (ata_drv)
	    DefinePrint("CFG_HARD_DRVS","\"c2p%d\n\"",ata_drv->num);
	  if (atapi_drv)
	    DefinePrint("CFG_CDROM_DRVS","\"t3p%d\n\"",atapi_drv->num);
	} else {
	  try {
	    bd=Let2BlkDev('C');
	    DefinePrint("CFG_HARD_DRVS","\"c2s0x%X\n0x%X\n%d\"",
	      bd->base0,bd->base1,bd->unit);
	  } catch
	    Fs->catch_except=TRUE;
	  try {
	    bd=Let2BlkDev('T');
	    DefinePrint("CFG_CDROM_DRVS","\"t3s0x%X\n%d\"",bd->base0,bd->unit);
	  } catch
	    Fs->catch_except=TRUE;
	}
      }
      DefinePrint("CFG_DSK_CACHE","\"Scale2Mem(0x80000,0x8000000)\n\"");
      DefinePrint("TOS_HDS","\"CD\"");
      DefinePrint("TOS_MASTER_BOOT_DRVS","\"C\"");
  }
} TOSInit;

U0 TOSDrvTextAttrSet()
{
  DrvTextAttrSet('C', WHITE <<4|BLUE);	//Main
  DrvTextAttrSet('D', WHITE <<4|RED);	//Back-up
  DrvTextAttrSet('A', BLUE  <<4|BROWN);	//Rescue
  DrvTextAttrSet('B', BLUE  <<4|YELLOW);//RAM
  DrvTextAttrSet('T', LTGRAY<<4|BLUE);	//CDROM
} TOSDrvTextAttrSet;

#define TOS_ISO_NAME	"B:/TempleOSCD.ISO"
#define TOS_DISTRO_DIR	"B:/Distro"
#define CFG_OPTS	"AutoMountIDE\nCT\n"
#define CFG_DBG_OPTS	"AutoMountIDE\nCT"\
			"HeapInit\n130\nMemInit\n131\nVarInit\n132\n\n"
#define CFG_DBGZ_OPTS	"AutoMountIDE\nCT"\
			"HeapInit\n0\nMemInit\n0\nVarInit\n0\n\n"
#define TOS_CFG		"\n" CFG_RAM_DRVS CFG_CDROM_DRVS CFG_HARD_DRVS "\n" CFG_DSK_CACHE CFG_OPTS
#define TOS_CDROM_CFG	"Tb1\n0x20000\nt3 \n\n\nAutoMountIDE\nCT\n"
#define TOS_CDROM_DBG_CFG "A" CFG_BOOT_RESCUE_DRV "b1\nScale2Mem(2048,0x40000)\n\n\n"\
			"DontProbe\nBootRescueDrv\n" "C:" CFG_BOOT_RESCUE_DRV_FILE "\n" CFG_BOOT_RESCUE_DRV_START "\n\n"
