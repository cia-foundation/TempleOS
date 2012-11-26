//This is run in a #exe{}.

U8 *osmain_cfg_options="ProtectLow\0Deterministic\0MemInit\0HeapInit\0VarInit\0NoMP\0TextMode\0GrMode\0DontProbe\0Help\0";

#define CFG_PROTECT_LOW		0
#define CFG_DETERMINISTIC	1
#define CFG_MEM_INIT		2
#define CFG_HEAP_INIT		3
#define CFG_VAR_INIT		4
#define CFG_NO_MP		5
#define CFG_TEXT_MODE		6
#define CFG_GR_MODE		7
#define CFG_DONT_PROBE		8
#define CFG_NUM_OPTIONS		9

#define CFG_HELP		9


class COSMainCfg
{
  I64 bios_gr_mode;
  U8 dft_drv,*acct_name;
  U8 *dsk_cache_size_exp;
  CDoc *add_dev;

  Bool opts[CFG_NUM_OPTIONS];
  U8 mem_init_val,heap_init_val,var_init_val;
  Bool cdrom_ports_pmt;
};

CDoc *OSMainCfgAddDev(COSMainCfg *c)
{
  I64 num_hints,drv_let_base,type,drv_let,k,n,num=0;
  U8 buf[256],buf2[128],buf3[128];
  CATARep *root=NULL,*tempha;
  CDoc *doc=DocNew;

  PutDefine("ST_MOUNT_INTRO");
  do {
    PutDefine("ST_MOUNT_BASE_LET");
    drv_let_base=ToUpper(GetChar)-'A';
    NewLine;
    if (0<=drv_let_base<NUM_PRTS) {
      PutDefine("ST_MOUNT_DRV_TYPE");
      type=GetChar-'0';
      NewLine;

      k=0;
      if (type==BDT_RAM) {
	PutDefine("ST_MOUNT_BLKS");
	GetS(buf,256);
      } else if (type==BDT_ATA ||
	  type==BDT_ATAPI) {
	num_hints=ATARep(TRUE,&root);
	if (type==BDT_ATAPI) {
	  PutDefine("ST_MOUNT_BOOTED_CDROM");
	  if (num_hints)
	    PutDefine("ST_MOUNT_BASE0_HINT");
	  else
	    PutDefine("ST_MOUNT_BASE0");
	  GetS(buf2,128);
	} else
	  do {
	    if (num_hints)
	      PutDefine("ST_MOUNT_BASE0_HINT");
	    else
	      PutDefine("ST_MOUNT_BASE0");
	    GetS(buf2,128);
	  } while (!A2I(buf2));
	if (1<=A2I(buf2)<=num_hints) {
	  tempha=ATARepFind(root,A2I(buf2));
	  SPrintF(buf2,"0x%X",tempha->base0);
	  SPrintF(buf3,"0x%X",tempha->base1);
	  k=tempha->unit;
	} else if (type!=BDT_ATAPI || *buf2) {
	  if (type==BDT_ATAPI)
	    StrCpy(buf3,"0");
	  else
	    do {
	      PutDefine("ST_MOUNT_BASE1");
	      GetS(buf3,128);
	    } while (!A2I(buf3));
	  do {
	    PutDefine("ST_MOUNT_UNIT");
	    k=GetChar-'0';
	  } while (!(0<=k<=1));
	  NewLine;
	}
	LinkedListDel(root);
      }
      if (type==BDT_NON_NATIVE_FILE) {
	PutDefine("ST_MOUNT_NON_NATIVE_LET");
	drv_let=GetChar;
	NewLine;
	PutDefine("ST_MOUNT_NON_NATIVE_BLK");
	GetS(buf2,128);
      }

      if (BDT_NULL<type<=BDT_NON_NATIVE_FILE &&
	  0<=k<=1) {
	DocPrintF(doc,"bdev=&sys_blkdevs[num_blkdevs++];");
	DocPrintF(doc,"bdev->drv_let_base=%d;",
	    drv_let_base+'A');
	DocPrintF(doc,"bdev->unit=%d;",k);
	DocPrintF(doc,"bdev->type=%d;",type);

	if (type==BDT_ATAPI)
	  DocPrintF(doc,"bdev->flags=BDF_REMOVABLE;");
	else
	  DocPrintF(doc,"bdev->flags=0;");
	if (type!=BDT_RAM)
	  DocPrintF(doc,"bdev->flags|=BDF_READ_CACHE;");

	if (type==BDT_ATAPI) {
	  DocPrintF(doc,"bdev->blk_size=CD_BLK_SIZE;");
	  DocPrintF(doc,"bdev->flags|=BDF_READ_ONLY;");
	} else
	  DocPrintF(doc,"bdev->blk_size=BLK_SIZE;");

	if (type==BDT_RAM)
	  DocPrintF(doc,"bdev->max_blk=(%s)-1;",buf);
	else
	  DocPrintF(doc,"bdev->max_blk=0xEFFFFFFF;");

	if (type==BDT_NON_NATIVE_FILE) {
	  DocPrintF(doc,"bdev->non_native_drv=%d;",drv_let);
	  n=A2I(buf2);
	  DocPrintF(doc,"bdev->non_native_offset=%d;",n);
	} else
	  DocPutS(doc,"bdev->non_native_drv=0;"
		    "bdev->non_native_offset=0;");

	if (type==BDT_ATA || type==BDT_ATAPI) {
	  if (type==BDT_ATAPI && !*buf2) {
	    c->cdrom_ports_pmt=TRUE;
	    DocPrintF(doc,"boot_drv_pmt=PmtBaseUnit(bdev);");
	  } else
	    DocPrintF(doc,"bdev->base0=%s;bdev->base1=%s;",
		buf2,buf3);
	}

	DocPrintF(doc,"BlkDevAdd(bdev,FALSE);");
	num++;
      } else
	PutDefine("ST_FAILED");
    }
  } while (!num || 0<=drv_let_base<NUM_PRTS);

  //See $LK,"InsBootDstDrv","FF:::/SparrowOS/Adam/Boot/InsBoot.CPP.Z,InsBootDstDrv"$
  if (!(k=NamedFramePtr("InsBootDstDrv")))
    k=Fs->cur_prt->drv_let;

  DocPrintF(doc,"dft_drv=\'%C\';",k);
  c->dft_drv=k;

  return doc;
}

U0 OSMainCfgOptions(COSMainCfg *c)
{
  I64 i;
  U8 *st=NULL;
  Bool state;
  do {
    Free(st);
    for (i=0;i<CFG_NUM_OPTIONS;i++)
      "$$FG,PURPLE$$%13tz$$FG$$:%Z\n",i,osmain_cfg_options,c->opts[i],"ST_OFF_ON";
    "\nType '$$FG,PURPLE$$Help$$FG$$' for help.\n";
    st=PmtStr("Option ($$FG,PURPLE$$<ENTER>$$FG$$ when done):","");
    i=ListEntryMatch(st,osmain_cfg_options,LEF_IGNORE_CASE);
    if (i==CFG_HELP)
"\n"
"$$FG,PURPLE$$ProtectLow$$FG$$ Marks memory below 0x%0X as not present.\n"
"$$FG,PURPLE$$Deterministic$$FG$$ Tries to keep deterministic.\n"
"$$FG,PURPLE$$MemInit$$FG$$ Initializes memory above 0x100000 to a val at boot.\n"
"$$FG,PURPLE$$HeapInit$$FG$$ Initializes MAlloc() memory to a val.\n"
"$$FG,PURPLE$$VarInit$$FG$$ Initializes glbl var memory to a val.\n"
"$$FG,PURPLE$$NoMP$$FG$$ No multicore.\n"
"$$FG,PURPLE$$TextMode$$FG$$ Text Mode (requires hard reboot).\n"
"$$FG,PURPLE$$GrMode$$FG$$ Graphics Mode (requires hard reboot).\n"
"$$FG,PURPLE$$DontProbe$$FG$$ Just prompt CD-ROM ports, don't probe.\n"
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
      }
    }
  } while (*st);
  Free(st);
}

COSMainCfg *OSMainCfgNew()
{
  COSMainCfg *c=CAlloc(sizeof(COSMainCfg));

  c->add_dev=OSMainCfgAddDev(c);
  c->acct_name=PmtStr("Start-up Acct ($$FG,PURPLE$$<ENTER>$$FG$$ will prompt):");
  c->dsk_cache_size_exp=PmtStr("Disk Cache Size in Bytes, gets rounded-up funny, ($$FG,PURPLE$$<ENTER>$$FG$$ will use default of $$FG,PURPLE$$%s$$FG$$ Bytes):",
			       "Scale2Mem(0x80000,0x8000000)");
  c->bios_gr_mode=0x12; //0x12=640x480x4 0x6A=800x600x4
  OSMainCfgOptions(c);
  if (c->opts[CFG_DETERMINISTIC])
    StreamPutS("#define SYS_DETERMINISTIC");
  return c;
}
 
U0 OSMainCfgDel(COSMainCfg *c)
{
  DocDel(c->add_dev);
  Free(c->acct_name);
  Free(c->dsk_cache_size_exp);
  Free(c);
}
