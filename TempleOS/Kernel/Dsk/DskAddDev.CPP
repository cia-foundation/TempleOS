U0 SetBlkDevLockFwding(CBlkDev *bdev)
{
  CBlkDev *bdev1;
  I64 i;
  switch (bdev->type) {
    case BDT_RAM:
      break;
    case BDT_ATA:
    case BDT_ATAPI:
      for (i=0;i<NUM_BLKDEVS;i++) {
	bdev1=&sys_blkdevs[i];
	if (bdev1->bd_signature==BD_SIGNATURE_VAL && bdev!=bdev1 &&
	    (bdev1->type==BDT_ATAPI || bdev1->type==BDT_ATA) &&
	    bdev1->base0==bdev->base0) {
	  bdev->lock_fwding=bdev1;
	  break;
	}
      }
      break;
  }
}

I64 BlkDevAdd(CBlkDev *bdev,Bool one_prt,Bool make_free)
{
  I64 i,j,ext_base,offset,result=0;
  CPrt *p;
  CBootStruct br;
  CMasterBoot mbr;

  bdev->bd_signature=BD_SIGNATURE_VAL;
  if (make_free)
    p=PrtMakeFree(bdev->first_drv_let+result);
  else
    p=PrtNextFree(bdev->first_drv_let+result);
  p->bdev=bdev;
  p->size=bdev->max_blk+1;
  switch (bdev->type) {
    case BDT_RAM:
      p->pt_signature=PRT_SIGNATURE_VAL;
      p->type=PT_REDSEA;
      p->rblks=&RAMRBlks;
      p->wblks=&RAMWBlks;
//This is to force creation of a RAM
//drive during boot, so it is probably
//MAlloced to the same addr and can
//be assumed to be already formatted.
//If this line is removed, RAM Drives
//will be allocated on a just-in-time
//basis.
      if (BlkDevInit(bdev))
	result++;
      else
	p->pt_signature=0;
      break;
    case BDT_ATA:
      p->pt_signature=PRT_SIGNATURE_VAL; //Temporarily validate
      if (!BlkDevInit(bdev))
	p->pt_signature=0; //Revoke validation
      else {
	p->pt_signature=0; //Revoke validation
	if (one_prt) {
	  p->pt_signature=PRT_SIGNATURE_VAL;
	  result++;
	  p->type=PT_REDSEA;
	  p->rblks=&ATARBlks;
	  p->wblks=&ATAWBlks;
	  p->size=bdev->max_blk+1;
	  //The following read is a test read.
	  //if it hangs, the drive is not supported.
	  ATAReadBlks(bdev,&mbr,0,1);
	  break;
	}
	offset=0;
	ext_base=INVALID_CLUSTER;
	while (TRUE) {
	  ATAReadBlks(bdev,&mbr,offset,1);
	  if (mbr.signature!=0xAA55)
	    break;
	  j=-1;
	  for (i=0;i<4;i++) {
	    if (mbr.p[i].type) {
	      if (make_free)
		p=PrtMakeFree(bdev->first_drv_let+result);
	      else
		p=PrtNextFree(bdev->first_drv_let+result);
	      p->bdev=bdev;
	      p->rblks=&ATARBlks;
	      p->wblks=&ATAWBlks;
	      p->offset=mbr.p[i].offset+offset;
	      p->size  =mbr.p[i].size;
	      switch (mbr.p[i].type) {
		case MBR_PT_REDSEA:
		  p->pt_signature=PRT_SIGNATURE_VAL;
		  result++;
		  p->type=PT_REDSEA;
		  RedSeaFSPrtInit(p);
		  break;
		case MBR_PT_FAT32a:
		case MBR_PT_FAT32b:
		case MBR_PT_FAT32c:
		case MBR_PT_FAT32d:
		case MBR_PT_FAT32e:
		case MBR_PT_FAT32f:
		  ATAReadBlks(bdev,&br,p->offset,1);
		  p->pt_signature=PRT_SIGNATURE_VAL;
		  result++;
		  if (br.signature==MBR_PT_REDSEA) {
		    p->type=PT_REDSEA;
		    RedSeaFSPrtInit(p);
		  } else {
		    p->type=PT_FAT32;
		    FAT32PrtInit(p);
		  }
		  break;
		case MBR_PT_NTFS:
		  p->pt_signature=PRT_SIGNATURE_VAL;
		  result++;
		  p->type=PT_NTFS;
		  break;
		case 5:
		case 15:
		  j=i;
		  break;
		default:
		  p->pt_signature=PRT_SIGNATURE_VAL;
		  result++;
		  p->type=PT_UNKNOWN;
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
      p->pt_signature=PRT_SIGNATURE_VAL;
      result++;
      p->type=PT_ISO9660; //Start with this
      p->rblks=&ATARBlks;
      p->wblks=&ATAWBlks;
      p->size =0;
      break;
  }
  if (result)
    SetBlkDevLockFwding(bdev);
  else
    MemSet(bdev,0,sizeof(CBlkDev));
  return result;
}


Bool DrvEnable(U8 drv_let,Bool val)
{//This can be used to unmount a drive or remount,
//but not mount the first time.
  CPrt *p;
  Bool result=FALSE;
  try {
    p=Drv2Prt(drv_let);
    result=!LBEqu(&p->type,PTf_DISABLE,!val);
  } catch
    Fs->catch_except=TRUE;
  return result;
}

CATARep *ATARepFind(CATARep *root,I64 num)
{
  CATARep *tempha=root;
  while (tempha) {
    if (tempha->num==num)
      return tempha;
    tempha=tempha->next;
  }
  return tempha;
}

U0 Mount(Bool repartition=FALSE)
{
  CBlkDev *bdev;
  I64 first_prt_num,type,unit,num_hints;
  U8 blks_buf[128],addr_buf[128],base0_buf[128],base1_buf[128];
  Bool add;
  CATARep *root,*tempha;

  do {
    PrtRep;
    PutDefine("ST_MOUNT_FIRST_DRV_LET");
    first_prt_num=ToUpper(GetChar)-'A';
    NewLine;
    if (0<=first_prt_num<NUM_PRTS) {
      PutDefine("ST_MOUNT_DRV_TYPE");
      type=GetChar-'0';
      NewLine;

      unit=0;
      if (type==BDT_ATA || type==BDT_ATAPI) {
	num_hints=ATARep(,,&root);
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
	} else {
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
      if (type==BDT_RAM) {
	PutDefine("ST_MOUNT_ADDR");
	GetS(addr_buf,128);
	PutDefine("ST_MOUNT_BLKS");
	GetS(blks_buf,128);
      }

      if (BDT_NULL<type<BDT_NUM_TYPES && 0<=unit<=1) {
	bdev=BlkDevNextFree(first_prt_num+'A',type);
	bdev->unit=unit;
	if (type==BDT_RAM) {
	  if (!*addr_buf) StrCpy(addr_buf,"0");
	  bdev->RAM_dsk=A2I(addr_buf);
	  bdev->max_blk=A2I(blks_buf)-1;
	}
	if (type==BDT_ATA ||
	    type==BDT_ATAPI) {
	  bdev->base0=A2I(base0_buf);
	  bdev->base1=A2I(base1_buf);
	}
	add=TRUE;
	if (type==BDT_ATA && repartition) {
	  PutDefine("ST_MOUNT_FMT_WHOLE_DRV");
	  if (YorN) {
	    if (!BlkDevAdd(bdev,TRUE,TRUE))
	      PutDefine("ST_FAILED");
	    else
	      CallExtStr("PrtDsk",bdev->first_drv_let);
	    add=FALSE;
	  }
	}
	if (add && !BlkDevAdd(bdev,FALSE,TRUE))
	  PutDefine("ST_FAILED");
      } else
	PutDefine("ST_FAILED");
    }
  } while (0<=first_prt_num<NUM_PRTS);
}

I64 SysPmtI64()
{
  U8 st[136];
  StrCpy(st,"0x");
  GetS(st+2,134,FALSE);
  if (!st[2])
    return 0;
  else
    return A2I(st);
}

Bool PmtBaseUnit(CBlkDev *bdev)
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
  if (!probe || !BootCdProbeAll(bdev)) {
    PutDefine("ST_CDROM_PORTS_PMT1");
    bdev->base0=SysPmtI64;
    bdev->base1=0;
    bdev->unit =0;
    if (bdev->base0) {
      "\nUnit (0 or 1): ";
      do ch=GetChar(,FALSE);
      while (!('0'<=ch<='1'));
      '' ch;
      bdev->unit=ch-'0';
      cdrom_boot_is_good=BootCdProbe(bdev);
      return TRUE;
    } else {
      cdrom_boot_is_good=FALSE;
      return FALSE;
    }
  }
  return FALSE;
}

U0 BlkDevsInitAll()
{
  CBlkDev *bdev;
  I64 i;
  sys_blkdevs=CAlloc(sizeof(CBlkDev)*NUM_BLKDEVS);
  sys_prts=CAlloc(sizeof(CPrt)*NUM_PRTS);
  for (i=0;i<NUM_PRTS;i++)
    drv_to_prt[i]=&sys_prts[i];
  sys_raw_col=0;
#exe {
  if (kernel_cfg->opts[CFG_AUTO_MOUNT_IDE])
    StreamPrint("AutoMountIDE;");
  StreamPrint("#exe {OptOff(OPTf_WARN_PAREN);}");
  StreamDoc(kernel_cfg->add_dev);
  StreamPrint("#exe {OptOn(OPTf_WARN_PAREN);}");
};
  PrtRep;
}

