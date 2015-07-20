U0 BlkDevLockFwdingSet(CBlkDev *bd)
{  //If two blkdevs on same controller, use just one lock
  CBlkDev *bd1;
  I64 i;
  switch (bd->type) {
    case BDT_RAM:
      break;
    case BDT_ATA:
    case BDT_ATAPI:
      for (i=0;i<NUM_BLKDEVS;i++) {
	bd1=&dsk.blkdevs[i];
	if (bd1->bd_signature==BD_SIGNATURE_VAL && bd!=bd1 &&
	      (bd1->type==BDT_ATAPI || bd1->type==BDT_ATA) &&
	      bd1->base0==bd->base0) {
	  bd->lock_fwding=bd1;
	  break;
	}
      }
      break;
  }
}

I64 BlkDevAdd(CBlkDev *bd,Bool one_drv,Bool make_free)
{
  I64 i,j,ext_base,offset,result=0;
  CDrv *dv;
  CRedSeaBoot br;
  CMasterBoot mbr;

  bd->bd_signature=BD_SIGNATURE_VAL;
  if (make_free)
    dv=DrvMakeFree(bd->first_drv_let+result);
  else
    dv=DrvNextFree(bd->first_drv_let+result);
  dv->bd=bd;
  dv->size=bd->max_blk+1;
  switch (bd->type) {
    case BDT_RAM:
      dv->dv_signature=DRV_SIGNATURE_VAL;
      dv->type=DVT_REDSEA;
//This is to force creation of a RAM
      //drive during boot, so it is probably
      //MAlloced to the same addr and can
      //be assumed to be already formatted.
      //If this line is removed, RAM Drives
      //will be alloced on a just-in-time
      //basis.
      if (BlkDevInit(bd))
	result++;
      else
	dv->dv_signature=0;
      break;
    case BDT_ATA:
      dv->dv_signature=DRV_SIGNATURE_VAL; //Temporarily validate
      if (!BlkDevInit(bd))
	dv->dv_signature=0; //Revoke validation
      else {
	dv->dv_signature=0; //Revoke validation
	if (one_drv) {
	  dv->dv_signature=DRV_SIGNATURE_VAL;
	  result++;
	  dv->type=DVT_REDSEA;
	  dv->size=bd->max_blk+1;
//The following read is a test read.
	  //if it hangs, the drive is not supported.
	  ATAReadBlks(bd,&mbr,0,1);
	  break;
	}
	offset=0;
	ext_base=INVALID_CLUSTER;
	while (TRUE) {
	  ATAReadBlks(bd,&mbr,offset,1);
	  if (mbr.signature!=0xAA55)
	    break;
	  j=-1;
	  for (i=0;i<4;i++) {
	    if (mbr.p[i].type) {
	      if (make_free)
		dv=DrvMakeFree(bd->first_drv_let+result);
	      else
		dv=DrvNextFree(bd->first_drv_let+result);
	      dv->bd=bd;
	      dv->base_offset=mbr.p[i].offset+offset;
	      dv->size	=mbr.p[i].size;
	      switch (mbr.p[i].type) {
		case MBR_PT_REDSEA:
		  dv->dv_signature=DRV_SIGNATURE_VAL;
		  result++;
		  dv->type=DVT_REDSEA;
		  RedSeaDrvInit(dv);
		  break;
		case MBR_PT_FAT32a:
		case MBR_PT_FAT32b:
		case MBR_PT_FAT32c:
		case MBR_PT_FAT32d:
		case MBR_PT_FAT32e:
		case MBR_PT_FAT32f:
		  ATAReadBlks(bd,&br,dv->base_offset,1);
		  dv->dv_signature=DRV_SIGNATURE_VAL;
		  result++;
		  if (br.signature==MBR_PT_REDSEA) {
		    dv->type=DVT_REDSEA;
		    RedSeaDrvInit(dv);
		  } else {
		    dv->type=DVT_FAT32;
		    FAT32DrvInit(dv);
		  }
		  break;
		case MBR_PT_NTFS:
		  dv->dv_signature=DRV_SIGNATURE_VAL;
		  result++;
		  dv->type=DVT_NTFS;
		  break;
		case 5:
		case 15:
		  j=i;
		  break;
		default:
		  dv->dv_signature=DRV_SIGNATURE_VAL;
		  result++;
		  dv->type=DVT_UNKNOWN;
	      }
	    }
	  }
	  if (j<0)
	    break;
	  if (!mbr.p[j].offset)
	    break;
	  if (ext_base==INVALID_CLUSTER) {
	    offset=mbr.p[j].offset;
	    ext_base=offset;
	  } else
	    offset=mbr.p[j].offset+ext_base;
	}
      }
      break;
    case BDT_ATAPI:
      dv->dv_signature=DRV_SIGNATURE_VAL;
      result++;
      dv->type=DVT_ISO9660; //Start with this
      dv->size =0;
      break;
  }
  if (result)
    BlkDevLockFwdingSet(bd);
  else
    MemSet(bd,0,sizeof(CBlkDev));
  return result;
}

Bool DrvEnable(U8 drv_let,Bool val)
{//Can unmount or remount, but not mount the first time.
  CDrv *dv;
  Bool result=FALSE;
  try {
    dv=Let2Drv(drv_let);
    result=!LBEqu(&dv->type,DVTf_DISABLE,!val);
  } catch
    Fs->catch_except=TRUE;
  return result;
}

CATARep *ATARepFind(CATARep *haystack_root,I64 needle_num)
{
  while (haystack_root) {
    if (haystack_root->num==needle_num)
      return haystack_root;
    haystack_root=haystack_root->next;
  }
  return NULL;
}

U0 Mount(Bool repartition=FALSE)
{//Mount a drive.
  CBlkDev *bd;
  I64 first_drv_num,type,unit,num_hints;
  U8 blks_buf[STR_LEN],addr_buf[STR_LEN],base0_buf[STR_LEN],base1_buf[STR_LEN];
  Bool add;
  CATARep *root,*tempha;

  do {
    DrvRep;
    "\n****** Mount Drives ******\n"
	  "The first drive letter is the letter\n"
	  "of the first partition.  If adding\n"
	  "a hard drive, you should pick 'C'.\n"
	  "If more than one partition is present\n"
	  "they will be lettered in sequence.\n"
	  "\nFirst Drive Letter ($$PURPLE$$<ENTER>$$FG$$ to exit):";
    first_drv_num=ToUpper(GetChar)-'A';
    '\n';
    if (0<=first_drv_num<NUM_DRVS) {
      "\t$$PURPLE$$1$$FG$$=RAM\n\t$$PURPLE$$2$$FG$$=ATA(Harddisk)\n"
	    "\t$$PURPLE$$3$$FG$$=ATAPI(CD-ROM)\nType:";
      type=GetChar-'0';
      '\n';

      unit=0;
      if (type==BDT_ATA || type==BDT_ATAPI) {
	num_hints=ATARep(,,&root);
	do {
	  if (num_hints)
	    "Enter dev number or\nport with $$PURPLE$$0x$$FG$$ prefix.\n"
		  "I/O Port Base0:\n";
	  else
	    "Include $$PURPLE$$0x$$FG$$ prefix.\nI/O Port Base0:\n";
	  GetS(base0_buf,STR_LEN);
	} while (!Str2I64(base0_buf));
	if (1<=Str2I64(base0_buf)<=num_hints) {
	  tempha=ATARepFind(root,Str2I64(base0_buf));
	  StrPrint(base0_buf,"0x%X",tempha->base0);
	  StrPrint(base1_buf,"0x%X",tempha->base1);
	  unit=tempha->unit;
	} else {
	  if (type==BDT_ATAPI)
	    StrCpy(base1_buf,"0");
	  else
	    do {
	      "I/O Port Base1:\n";
	      GetS(base1_buf,STR_LEN);
	    } while (!Str2I64(base1_buf));
	  do {
	    "\t$$PURPLE$$0$$FG$$=Master\n\t$$PURPLE$$1$$FG$$=Slave\nUnit:";
	    unit=GetChar-'0';
	  } while (!(0<=unit<=1));
	  '\n';
	}
	LinkedLstDel(root);
      }
      if (type==BDT_RAM) {
	"Addr of RAM disk ($$PURPLE$$<ENTER>$$FG$$ to MAlloc):";
	GetS(addr_buf,STR_LEN);
	"Blks of 512 bytes:";
	GetS(blks_buf,STR_LEN);
      }

      if (BDT_NULL<type<BDT_NUM_TYPES && 0<=unit<=1) {
	bd=BlkDevNextFree(first_drv_num+'A',type);
	bd->unit=unit;
	if (type==BDT_RAM) {
	  if (!*addr_buf) StrCpy(addr_buf,"0");
	  bd->RAM_dsk=Str2I64(addr_buf);
	  bd->max_blk=Str2I64(blks_buf)-1;
	}
	if (type==BDT_ATA ||
	      type==BDT_ATAPI) {
	  bd->base0=Str2I64(base0_buf);
	  bd->base1=Str2I64(base1_buf);
	}
	add=TRUE;
	if (type==BDT_ATA && repartition) {
	  "\nReformat WHOLE drive!";
	  if (YorN) {
	    if (!BlkDevAdd(bd,TRUE,TRUE))
	      "\n\n$$BK,1$$$$RED$$Operation Failed$$BK,0$$$$FG$$\n\n";
	    else
	      CallExtStr("PrtDsk",bd->first_drv_let);
	    add=FALSE;
	  }
	}
	if (add && !BlkDevAdd(bd,FALSE,TRUE))
	  "\n\n$$BK,1$$$$RED$$Operation Failed$$BK,0$$$$FG$$\n\n";
      } else
	"\n\n$$BK,1$$$$RED$$Operation Failed$$BK,0$$$$FG$$\n\n";
    }
  } while (0<=first_drv_num<NUM_DRVS);
}

I64 SysGetI64()
{
  U8 st[STR_LEN];
  GetS(st,STR_LEN,FALSE);
  return Str2I64(st,16);
}

Bool GetBaseUnit(CBlkDev *bd)
{
  I64 ch;
  Bool probe;
  "            TempleOS\n\n\n";
  #exe {
    if (kernel_cfg->opts[CFG_DONT_PROBE])
      StreamPrint("probe=FALSE;");
    else
      StreamPrint("probe=TRUE;");
  };
  if (!probe || !BootCdProbeAll(bd)) {
    "\nDon't worry.  This is not a product\n"
	  "registration.	TempleOS just needs the\n"
	  "I/O port numbers for the CD-ROM.\n"
	  "\nRetry the ports above or check Windows\n"
	  "system information under I/O ports for\n"
	  "'IDE', 'ATA' or 'SATA'.\n"
	  "In Linux, use 'lspci -v' for ports.\n"
	  "\n\nEnter 4-digit hex I/O Port number.\n"
	  "CD-ROM I/O Port Base0: 0x";
    bd->base0=SysGetI64;
    bd->base1=0;
    bd->unit =0;
    if (bd->base0) {
      "\nUnit (0 or 1): ";
      do ch=GetChar(,FALSE);
      while (!('0'<=ch<='1'));
      '' ch;
      bd->unit=ch-'0';
      dsk.cdrom_boot_is_good=BootCdProbe(bd);
      return TRUE;
    } else {
      dsk.cdrom_boot_is_good=FALSE;
      return FALSE;
    }
  }
  return FALSE;
}

U0 BlkDevsInitAll()
{
  CBlkDev *bd;
  I64 i;
  dsk.blkdevs=CAlloc(sizeof(CBlkDev)*NUM_BLKDEVS);
  dsk.drvs=CAlloc(sizeof(CDrv)*NUM_DRVS);
  for (i=0;i<NUM_DRVS;i++)
    dsk.let_to_drv[i]=&dsk.drvs[i];
  text.raw_col=0;
  #exe {
    if (kernel_cfg->opts[CFG_AUTO_MOUNT_IDE])
      StreamPrint("AutoMountIDE;");
    StreamPrint("#exe {OptOff(OPTf_WARN_PAREN);}");
    StreamDoc(kernel_cfg->add_dev);
    StreamPrint("#exe {OptOn(OPTf_WARN_PAREN);}");
  };
  DrvRep;
}
