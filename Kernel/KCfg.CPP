//This is run in a $LK,"#exe",A="FF:::/Kernel/Kernel.PRJ,KCfg"${}.

U8 *kernel_cfg_options="MemInit\0HeapInit\0VarInit\0"
"NoMP\0TextMode\0DontProbe\0AutoMountIDE\0BootRescueDrv\0Help\0";

#define CFG_MEM_INIT		0
#define CFG_HEAP_INIT		1
#define CFG_VAR_INIT		2
#define CFG_NO_MP		3
#define CFG_TEXT_MODE		4
#define CFG_DONT_PROBE		5
#define CFG_AUTO_MOUNT_IDE	6
#define CFG_BOOT_RESCUE_DRV	7
#define CFG_NUM_OPTIONS		8

#define CFG_HELP		8

class CKernelCfg
{
  U8 *dsk_cache_size_exp;
  CDoc *add_dev;
  U8 *boot_rescue_drv_file,*boot_rescue_drv_start;
  Bool opts[CFG_NUM_OPTIONS];
  U8 mem_init_val,heap_init_val,var_init_val,
	boot_drv_let,auto_mount_ide_hd_let,auto_mount_ide_cd_let;
};

CDoc *KernelCfgAddDev(CKernelCfg *c)
{
  I64 ch,cnt=0;
  CDoc *doc=DocNew,*doc2;
  "\n\nIn anticipation of the drives you will\n"
	"define shortly, enter the drive letter\n"
	"of the drive with the account directory.\n"
	"\n($$PURPLE$$<ENTER>$$FG$$ for cur drv) Home Drv:";
  ch=ToUpper(GetChar);
  if ('A'<=ch<='Z')
    c->boot_drv_let=ch;
  else
    c->boot_drv_let=Drv2Let(Fs->cur_dv);
  "\n\n$$BK,1$$$$PURPLE$$Mount drives so they will be present when "
	"you boot.$$FG$$$$BK,0$$\n";
  do {
    if (doc2=BlkDevPmt(c->boot_drv_let)) {
      DocInsDoc(doc,doc2);
      DocDel(doc2);
      cnt++;
    }
  } while (!cnt || doc2);
  return doc;
}

U0 KernelCfgOptions(CKernelCfg *c)
{
  I64 i;
  U8 *st=NULL;
  Bool state;
  do {
    Free(st);
    for (i=0;i<CFG_NUM_OPTIONS;i++)
      "$$PURPLE$$%13tz$$FG$$:%Z\n",i,kernel_cfg_options,c->opts[i],"ST_OFF_ON";
    "\nType '$$PURPLE$$Help$$FG$$' for help.\n";
    st=GetStr("Option ($$PURPLE$$<ENTER>$$FG$$ when done):","");
    i=LstMatch(st,kernel_cfg_options,LMF_IGNORE_CASE);
    if (i==CFG_HELP)
      "\n"
	    "$$PURPLE$$MemInit$$FG$$ Initializes memory above 0x100000 "
	    "to a val at boot.\n"
	    "$$PURPLE$$HeapInit$$FG$$ Initializes MAlloc() memory to a val.\n"
	    "$$PURPLE$$VarInit$$FG$$ Initializes glbl var memory to a val.\n"
	    "$$PURPLE$$NoMP$$FG$$ No multicore.\n"
	    "$$PURPLE$$TextMode$$FG$$ Text Mode (requires hard reboot).\n"
	    "$$PURPLE$$DontProbe$$FG$$ Just prompt CD/DVD ports, don't probe.\n"
	    "$$PURPLE$$AutoMountIDE$$FG$$ AutoMount IDE drives to 'C' and 'T'.\n"
	    "$$PURPLE$$BootRescueDrv$$FG$$ Include RAM Drv in Kernel.BIN.\n"
	    "\n";
    else
      if (0<=i<CFG_NUM_OPTIONS) {
	state=c->opts[i]=!c->opts[i];
	switch (i) {
	  case CFG_MEM_INIT:
	    if (state)
	      c->mem_init_val=GetI64("Val (0-255):",255,0,255);
	    break;
	  case CFG_HEAP_INIT:
	    if (state)
	      c->heap_init_val=GetI64("Val (0-255):",255,0,255);
	    break;
	  case CFG_VAR_INIT:
	    if (state)
	      c->var_init_val=GetI64("Val (0-255):",255,0,255);
	    break;
	  case CFG_AUTO_MOUNT_IDE:
	    if (state) {
	      "First HD Drive Let:";
	      c->auto_mount_ide_hd_let=ToUpper(GetChar);
	      if (!('A'<=c->auto_mount_ide_hd_let<='Z'))
		c->auto_mount_ide_hd_let=0;
	      '\n';
	      if (c->auto_mount_ide_hd_let)
		"First HD Drive:%C\n",c->auto_mount_ide_hd_let;
	      else
		"First HD Drive:%C\n",'C';

	      "First CD Drive Let:";
	      c->auto_mount_ide_cd_let=ToUpper(GetChar);
	      if (!('A'<=c->auto_mount_ide_cd_let<='Z'))
		c->auto_mount_ide_cd_let=0;
	      '\n';
	      if (c->auto_mount_ide_cd_let)
		"First CD Drive:%C\n",c->auto_mount_ide_cd_let;
	      else
		"First CD Drive:%C\n",'T';
	    } else {
	      c->auto_mount_ide_hd_let=0;
	      c->auto_mount_ide_cd_let=0;
	    }
	    break;
	  case CFG_BOOT_RESCUE_DRV:
	    Free(c->boot_rescue_drv_file);
	    c->boot_rescue_drv_file=0;
	    c->boot_rescue_drv_start=0;
	    if (state) {
	      c->boot_rescue_drv_file=GetStr("Boot Rescue Drv File:");
	      c->boot_rescue_drv_start=GetI64("Boot Rescue Drv Start:");
	    }
	    break;
	}
      }
  } while (*st);
  Free(st);
}

CKernelCfg *KernelCfgNew()
{
  CKernelCfg *c=CAlloc(sizeof(CKernelCfg));

  c->add_dev=KernelCfgAddDev(c);
  c->dsk_cache_size_exp=GetStr(
	"Disk Cache Size in Bytes,\n"
	"gets rounded-up funny,\n"
	"($$PURPLE$$<ENTER>$$FG$$ will use default.):",
	"Scale2Mem(0x80000,0x8000000)");
  KernelCfgOptions(c);
  return c;
}
 
U0 KernelCfgDel(CKernelCfg *c)
{
  DocDel(c->add_dev);
  Free(c->boot_rescue_drv_file);
  Free(c->dsk_cache_size_exp);
  Free(c);
}
