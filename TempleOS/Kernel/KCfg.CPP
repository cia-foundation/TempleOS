//This is run in a #exe{}.

U8 *kernel_cfg_options="ProtectLow\0Deterministic\0MemInit\0HeapInit\0VarInit\0"
"NoMP\0TextMode\0GrMode\0DontProbe\0AutoMountIDE\0Help\0";

#define CFG_PROTECT_LOW		0
#define CFG_DETERMINISTIC	1
#define CFG_MEM_INIT		2
#define CFG_HEAP_INIT		3
#define CFG_VAR_INIT		4
#define CFG_NO_MP		5
#define CFG_TEXT_MODE		6
#define CFG_GR_MODE		7
#define CFG_DONT_PROBE		8
#define CFG_AUTO_MOUNT_IDE	9
#define CFG_NUM_OPTIONS		10

#define CFG_HELP		10


class CKernelCfg
{
  I64 bios_gr_mode;
  U8 *acct_name;
  U8 *dsk_cache_size_exp;
  CDoc *add_dev;
  Bool opts[CFG_NUM_OPTIONS];
  U8 mem_init_val,heap_init_val,var_init_val,
     sys_acct_drv,auto_mount_ide_hd_let,auto_mount_ide_cd_let;
  Bool cdrom_ports_pmt;
};

CDoc *KernelCfgAddDev(CKernelCfg *c)
{
  I64 num_hints,first_prt_num,type,unit,cnt=0;
  U8 blks_buf[256],addr_buf[256],base0_buf[128],base1_buf[128];
  CATARep *root=NULL,*tempha;
  CDoc *doc=DocNew;
  Bool make_free;

  //See $LK,"InsBootDstDrv","FF:::/TempleOS/Adam/Boot/InsBoot.CPP,InsBootDstDrv"$
  if (!(c->sys_acct_drv=FramePtr("InsBootDstDrv")))
    c->sys_acct_drv=Fs->cur_prt->drv_let;

  PutDefine("ST_MOUNT_INTRO");
  do {
    PutDefine("ST_MOUNT_FIRST_DRV_LET");
    first_prt_num=ToUpper(GetChar)-'A';
    NewLine;
    if (0<=first_prt_num<NUM_PRTS) {
      PutDefine("ST_MOUNT_DRV_TYPE");
      type=GetChar-'0';
      NewLine;

      unit=0;
      if (type==BDT_RAM) {
	PutDefine("ST_MOUNT_ADDR");
	GetS(addr_buf,256);
	PutDefine("ST_MOUNT_BLKS");
	GetS(blks_buf,256);
      } else if (type==BDT_ATA ||
	  type==BDT_ATAPI) {
	num_hints=ATARep(,,&root);
	if (type==BDT_ATAPI) {
	  PutDefine("ST_MOUNT_BOOTED_CDROM");
	  if (num_hints)
	    PutDefine("ST_MOUNT_BASE0_HINT");
	  else
	    PutDefine("ST_MOUNT_BASE0");
	  GetS(base0_buf,128);
	} else
	  do {
	    if (num_hints)
	      PutDefine("ST_MOUNT_BASE0_HINT");
	    else
	      PutDefine("ST_MOUNT_BASE0");
	    GetS(base0_buf,128);
	  } while (!A2I(base0_buf));
	if (1<=A2I(base0_buf)<=num_hints) {
	  tempha=ATARepFind(root,A2I(base0_buf));
	  SPrint(base0_buf,"0x%X",tempha->base0);
	  SPrint(base1_buf,"0x%X",tempha->base1);
	  unit=tempha->unit;
	} else if (type!=BDT_ATAPI || *base0_buf) {
	  if (type==BDT_ATAPI)
	    StrCpy(base1_buf,"0");
	  else
	    do {
	      PutDefine("ST_MOUNT_BASE1");
	      GetS(base1_buf,128);
	    } while (!A2I(base1_buf));
	  do {
	    PutDefine("ST_MOUNT_UNIT");
	    unit=GetChar-'0';
	  } while (!(0<=unit<=1));
	  NewLine;
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
	    DocPrint(doc,"boot_drv_pmt=PmtBaseUnit(bdev);");
	    if (first_prt_num+'A'==c->sys_acct_drv)
	      make_free=TRUE;
	  } else
	    DocPrint(doc,"bdev->base0=%s;bdev->base1=%s;",
		base0_buf,base1_buf);
	}
	DocPrint(doc,"BlkDevAdd(bdev,FALSE,%d);",make_free);
	cnt++;
      } else
	PutDefine("ST_FAILED");
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
      "$$FG,PURPLE$$%13tz$$FG$$:%Z\n",i,kernel_cfg_options,c->opts[i],"ST_OFF_ON";
    "\nType '$$FG,PURPLE$$Help$$FG$$' for help.\n";
    st=PmtStr("Option ($$FG,PURPLE$$<ENTER>$$FG$$ when done):","");
    i=LstMatch(st,kernel_cfg_options,LMF_IGNORE_CASE);
    if (i==CFG_HELP)
"\n"
"$$FG,PURPLE$$ProtectLow$$FG$$ Marks memory at and below 0x%0X as not present.\n"
"$$FG,PURPLE$$Deterministic$$FG$$ Tries to keep deterministic.\n"
"$$FG,PURPLE$$MemInit$$FG$$ Initializes memory above 0x100000 to a val at boot.\n"
"$$FG,PURPLE$$HeapInit$$FG$$ Initializes MAlloc() memory to a val.\n"
"$$FG,PURPLE$$VarInit$$FG$$ Initializes glbl var memory to a val.\n"
"$$FG,PURPLE$$NoMP$$FG$$ No multicore.\n"
"$$FG,PURPLE$$TextMode$$FG$$ Text Mode (requires hard reboot).\n"
"$$FG,PURPLE$$GrMode$$FG$$ Graphics Mode (requires hard reboot).\n"
"$$FG,PURPLE$$DontProbe$$FG$$ Just prompt CD-ROM ports, don't probe.\n"
"$$FG,PURPLE$$AutoMountIDE$$FG$$ AutoMount IDE drives to 'C' and 'T'.\n"
"\n",PROTECTED_LOW_PAGE_LIMIT; //Should be stream val of prot_low_limit
    else
      if (0<=i<CFG_NUM_OPTIONS) {
      state=c->opts[i]=!c->opts[i];
      switch (i) {
	case CFG_MEM_INIT:
	  if (state)
	    c->mem_init_val=PmtI64("Val (0-255):",255,0,255);
	  break;
	case CFG_HEAP_INIT:
	  if (state)
	    c->heap_init_val=PmtI64("Val (0-255):",255,0,255);
	  break;
	case CFG_VAR_INIT:
	  if (state)
	    c->var_init_val=PmtI64("Val (0-255):",255,0,255);
	  break;
	case CFG_GR_MODE:
	  if (state)
	    c->bios_gr_mode=PmtI64(
	    "0x12 640x480x4\n"
	    "0x6A 800x600x4\n\n"
	    "BIOS Graphic Mode (0x%0X):",c->bios_gr_mode);
	  break;
	case CFG_AUTO_MOUNT_IDE:
	  if (state) {
	    "First HD Drive Let:";
	    c->auto_mount_ide_hd_let=ToUpper(GetChar);
	    if (!('A'<=c->auto_mount_ide_hd_let<='Z'))
	      c->auto_mount_ide_hd_let=0;
	    NewLine;
	    if (c->auto_mount_ide_hd_let)
	      "First HD Drive:%C\n",c->auto_mount_ide_hd_let;
	    else
	      "First HD Drive:%C\n",'C';

	    "First CD Drive Let:";
	    c->auto_mount_ide_cd_let=ToUpper(GetChar);
	    if (!('A'<=c->auto_mount_ide_cd_let<='Z'))
	      c->auto_mount_ide_cd_let=0;
	    NewLine;
	    if (c->auto_mount_ide_cd_let)
	      "First CD Drive:%C\n",c->auto_mount_ide_cd_let;
	    else
	      "First CD Drive:%C\n",'T';
	  } else {
	    c->auto_mount_ide_hd_let=0;
	    c->auto_mount_ide_cd_let=0;
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
  c->acct_name=PmtStr("Start-up Acct ($$FG,PURPLE$$<ENTER>$$FG$$ will prompt):");
  c->dsk_cache_size_exp=PmtStr("Disk Cache Size in Bytes, gets rounded-up funny, ($$FG,PURPLE$$<ENTER>$$FG$$ will use default of $$FG,PURPLE$$%s$$FG$$ Bytes):",
			       "Scale2Mem(0x80000,0x8000000)");
  c->bios_gr_mode=0x12; //0x12=640x480x4 0x6A=800x600x4
  KernelCfgOptions(c);
  if (c->opts[CFG_DETERMINISTIC])
    StreamPrint("#define SYS_DETERMINISTIC");
  return c;
}
 
U0 KernelCfgDel(CKernelCfg *c)
{
  DocDel(c->add_dev);
  Free(c->acct_name);
  Free(c->dsk_cache_size_exp);
  Free(c);
}
