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
  U8 *acct_name;
  U8 *dsk_cache_size_exp;
  CDoc *add_dev;
  U8 *boot_rescue_drv_file,*boot_rescue_drv_start;
  Bool opts[CFG_NUM_OPTIONS];
  U8 mem_init_val,heap_init_val,var_init_val,
     sys_acct_drv,auto_mount_ide_hd_let,auto_mount_ide_cd_let;
  Bool cdrom_ports_pmt;
};

CDoc *KernelCfgAddDev(CKernelCfg *c)
{
  I64 ch,num_hints,first_prt_num,type,unit,cnt=0;
  U8 blks_buf[256],addr_buf[256],base0_buf[128],base1_buf[128];
  CATARep *root=NULL,*tempha;
  CDoc *doc=DocNew;
  Bool make_free;

	"\n\nIn anticipation of the drives you will\n"
	"define shortly, enter the drive letter\n"
	"of the drive with the account directory.\n"
	"\n($$PURPLE$$<ENTER>$$FG$$ for cur drv) Acct Drv:";
  ch=ToUpper(GetChar);
  if ('A'<=ch<='Z')
    c->sys_acct_drv=ch;
  else
    c->sys_acct_drv=Fs->cur_prt->drv_let;

	"\n\n$$BK,1$$$$PURPLE$$Mount drives so they will be present when "
	"you boot.$$FG$$$$BK,0$$\n";
  do {
	"\n****** Mount Drives ******\n"
	"The first drive letter is the letter\n"
	"of the first partition.  If adding\n"
	"a hard drive, you should pick 'C'.\n"
	"If more than one partition is present\n"
	"they will be lettered in sequence.\n"
	"\nFirst Drive Letter ($$PURPLE$$<ENTER>$$FG$$ to exit):";
    first_prt_num=ToUpper(GetChar)-'A';
    '\n';
    if (0<=first_prt_num<NUM_PRTS) {
	"\t$$PURPLE$$1$$FG$$=RAM\n\t$$PURPLE$$2$$FG$$=ATA(Harddisk)\n"
	"\t$$PURPLE$$3$$FG$$=ATAPI(CD-ROM)\nType:";
      type=GetChar-'0';
      '\n';

      unit=0;
      if (type==BDT_RAM) {
	"Addr of RAM disk ($$PURPLE$$<ENTER>$$FG$$ to MAlloc):";
	GetS(addr_buf,256);
	"Blks of 512 bytes:";
	GetS(blks_buf,256);
      } else if (type==BDT_ATA ||
	  type==BDT_ATAPI) {
	num_hints=ATARep(,,&root);
	if (type==BDT_ATAPI) {
	  "<ENTER> to use booted CD-ROM\n";
	  if (num_hints)
		"Enter dev number or\nport with $$PURPLE$$0x$$FG$$ prefix.\n"
		"I/O Port Base0:\n";
	  else
		"Include $$PURPLE$$0x$$FG$$ prefix.\nI/O Port Base0:\n";
	  GetS(base0_buf,128);
	} else
	  do {
	    if (num_hints)
		"Enter dev number or\nport with $$PURPLE$$0x$$FG$$ prefix.\n"
		"I/O Port Base0:\n";
	    else
		"Include $$PURPLE$$0x$$FG$$ prefix.\nI/O Port Base0:\n";
	    GetS(base0_buf,128);
	  } while (!Str2I64(base0_buf));
	if (1<=Str2I64(base0_buf)<=num_hints) {
	  tempha=ATARepFind(root,Str2I64(base0_buf));
	  StrPrint(base0_buf,"0x%X",tempha->base0);
	  StrPrint(base1_buf,"0x%X",tempha->base1);
	  unit=tempha->unit;
	} else if (type!=BDT_ATAPI || *base0_buf) {
	  if (type==BDT_ATAPI)
	    StrCpy(base1_buf,"0");
	  else
	    do {
		"I/O Port Base1:\n";
	      GetS(base1_buf,128);
	    } while (!Str2I64(base1_buf));
	  do {
		"\t$$PURPLE$$0$$FG$$=Master\n\t$$PURPLE$$1$$FG$$=Slave\nUnit:";
	    unit=GetChar-'0';
	  } while (!(0<=unit<=1));
	  '\n';
	}
	LinkedLstDel(root);
      }
      if (BDT_NULL<type<BDT_NUM_TYPES && 0<=unit<=1) {
	DocPrint(doc,"bdev=BlkDevNextFree(%d,%d);"
		      "bdev->unit=%d;",first_prt_num+'A',type,unit);
	if (type==BDT_RAM) {
	  if (!*addr_buf) StrCpy(addr_buf,"0");
	  DocPrint(doc,"bdev->RAM_dsk=%s;",addr_buf);
	  DocPrint(doc,"bdev->max_blk=(%s)-1;",blks_buf);
	}
	make_free=FALSE;
	if (type==BDT_ATA || type==BDT_ATAPI) {
	  if (type==BDT_ATAPI && !*base0_buf) {
	    c->cdrom_ports_pmt=TRUE;
	    DocPrint(doc,"dsk.boot_rescue_drv_pmt=GetBaseUnit(bdev);");
	    if (first_prt_num+'A'==c->sys_acct_drv)
	      make_free=TRUE;
	  } else
	    DocPrint(doc,"bdev->base0=%s;bdev->base1=%s;",
		base0_buf,base1_buf);
	}
	DocPrint(doc,"BlkDevAdd(bdev,FALSE,%d);",make_free);
	cnt++;
      } else
	"\n\n$$BK,1$$$$RED$$Operation Failed$$BK,0$$$$FG$$\n\n";
    }
  } while (!cnt || 0<=first_prt_num<NUM_PRTS);
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
"$$PURPLE$$MemInit$$FG$$ Initializes memory above 0x100000 to a val at boot.\n"
"$$PURPLE$$HeapInit$$FG$$ Initializes MAlloc() memory to a val.\n"
"$$PURPLE$$VarInit$$FG$$ Initializes glbl var memory to a val.\n"
"$$PURPLE$$NoMP$$FG$$ No multicore.\n"
"$$PURPLE$$TextMode$$FG$$ Text Mode (requires hard reboot).\n"
"$$PURPLE$$DontProbe$$FG$$ Just prompt CD-ROM ports, don't probe.\n"
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
  c->acct_name=GetStr("Start-up Acct ($$PURPLE$$<ENTER>$$FG$$ will prompt):");
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
  Free(c->acct_name);
  Free(c->dsk_cache_size_exp);
  Free(c);
}
