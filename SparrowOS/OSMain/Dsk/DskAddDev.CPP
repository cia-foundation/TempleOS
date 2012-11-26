U0 SetBlkDevLockFwding(CBlkDev *bdev)
{
  CBlkDev *bd1;
  I64 i;
  switch (bdev->type) {
    case BDT_RAM:
      break;
    case BDT_ATA:
    case BDT_ATAPI:
      for (i=0;i<num_blkdevs;i++) {
	bd1=&sys_blkdevs[i];
	if (bdev!=bd1 && (bd1->type==BDT_ATAPI || bd1->type==BDT_ATA) &&
	    bd1->base0==bdev->base0) {
	  bdev->lock_fwding=bd1;
	  break;
	}
      }
      break;
    case BDT_NON_NATIVE_FILE:
      bdev->lock_fwding=Drv2BlkDev(bdev->non_native_drv);
      break;
  }
}

I64 BlkDevAdd(CBlkDev *bdev,Bool one_prt)
{
  I64 i,j,ext_base,num_prt,offset;
  CPrt *p;
  CBootStruct br;
  CMasterBoot mbr;

  bdev->bd_signature=BD_SIGNATURE_VAL;
  num_prt=bdev->drv_let_base-'A';
  p=&sys_prts[num_prt];
  MemSet(p,0,sizeof(CPrt));
  p->drv_let=num_prt+'A';
  p->pt_signature=PRT_SIGNATURE_VAL;
  p->bdev=bdev;
  p->size=bdev->max_blk+1;
  switch (bdev->type) {
    case BDT_RAM:
      p->type=PT_NATIVE;
      p->rblks=&RAMRBlks;
      p->wblks=&RAMWBlks;
      num_prt++;

//This is to force creation of a RAM
//drive during boot, so it is probably
//MAlloced to the same addr and can
//be assumed to be already formatted.
//If this line is removed, RAM Drives
//will be allocated on a just-in-time
//basis.
      BlkDevInit(bdev);

      break;
    case BDT_ATA:
      BlkDevInit(bdev);
      if (one_prt) {
	p->type=PT_NATIVE;
	p->rblks=&ATARBlks;
	p->wblks=&ATAWBlks;
	p->size=bdev->max_blk+1;
	num_prt++;
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
	    p=&sys_prts[num_prt];
	    MemSet(p,0,sizeof(CPrt));
	    p->drv_let=num_prt+'A';
	    p->pt_signature=PRT_SIGNATURE_VAL;
	    p->bdev=bdev;
	    p->rblks=&ATARBlks;
	    p->wblks=&ATAWBlks;
	    p->offset=mbr.p[i].offset+offset;
	    p->size  =mbr.p[i].size;
	    p->type=PT_NULL;
	    switch (mbr.p[i].type) {
	      case MBR_PT_NATIVE:
		p->type=PT_NATIVE;
		NativeFSPrtInit(p);
		num_prt++;
		break;
	      case MBR_PT_FAT32a:
	      case MBR_PT_FAT32b:
	      case MBR_PT_FAT32c:
	      case MBR_PT_FAT32d:
	      case MBR_PT_FAT32e:
	      case MBR_PT_FAT32f:
		ATAReadBlks(bdev,&br,p->offset,1);
		if (br.signature==MBR_PT_NATIVE) {
		  p->type=PT_NATIVE;
		  NativeFSPrtInit(p);
		} else {
		  p->type=PT_FAT32;
		  FAT32PrtInit(p);
		}
		num_prt++;
		break;
	      case MBR_PT_NTFS:
		p->type=PT_NTFS;
		num_prt++;
		break;
	      case 5:
	      case 15:
		j=i;
		break;
	      default:
		p->type=PT_UNKNOWN;
		num_prt++;
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
      break;
    case BDT_ATAPI:
      p->type=PT_ISO9660; //Start with this
      p->rblks=&ATARBlks;
      p->wblks=&ATAWBlks;
      p->size =0;
      num_prt++;
      break;
    case BDT_NON_NATIVE_FILE:
      p->type=PT_NATIVE;
      p->offset=bdev->non_native_offset;
      p->rblks=&NonNativeFileRBlks;
      p->wblks=&NonNativeFileWBlks;
      if (bdev->flags & BDF_FMT)
	p->size=bdev->max_blk-bdev->min_blk+1;
      num_prt++;
      break;
  }
  SetBlkDevLockFwding(bdev);
  return num_prt-(bdev->drv_let_base-'A');
}


Bool DrvEnable(U8 drv_let,Bool val)
{//This can be used to unmount a drive or remount,
//but not mount the first time.
  CPrt *p;
  Bool result=FALSE;
  try {
    p=Drv2Prt(drv_let);
    result=!LBEqu(&p->type,PTf_DISABLE,!val);
  } catch {
    Fs->ignore_except=TRUE;
  }
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
  I64 drv_let_base,drv_let,type,k,num_hints;
  U8 buf[32],buf2[128],buf3[128];
  Bool add;
  CATARep *root,*tempha;

  do {
    PrtRep;
    PutDefine("ST_MOUNT_BASE_LET");
    drv_let_base=ToUpper(GetChar)-'A';
    NewLine;
    if (0<=drv_let_base<NUM_PRTS) {
      PutDefine("ST_MOUNT_DRV_TYPE");
      type=GetChar-'0';
      NewLine;

      k=0;

      if (type==BDT_ATA || type==BDT_ATAPI) {
	num_hints=CallExtStr("ATARep",TRUE,&root);
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
	} else {
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
	PutDefine("ST_INCLUDE_0X");
	PutDefine("ST_MOUNT_NON_NATIVE_BLK");
	GetS(buf2,128);
      }
      if (type==BDT_RAM) {
	PutDefine("ST_MOUNT_BLKS");
	GetS(buf,32);
      }

      if (BDT_NULL<type<=BDT_NON_NATIVE_FILE &&
	  0<=k<=1) {
	bdev=&sys_blkdevs[num_blkdevs++];
	bdev->drv_let_base=drv_let_base+'A';
	bdev->unit=k;
	bdev->type=type;

	if (type==BDT_ATAPI)
	  bdev->flags=BDF_REMOVABLE;
	else
	  bdev->flags=0;
	if (type!=BDT_RAM)
	  bdev->flags|=BDF_READ_CACHE;
 
	if (type==BDT_ATAPI) {
	  bdev->blk_size=CD_BLK_SIZE;
	  bdev->flags|=BDF_READ_ONLY;
	} else
	  bdev->blk_size=BLK_SIZE;

	if (type==BDT_RAM)
	  bdev->max_blk=A2I(buf)-1;
	else
	  bdev->max_blk=0xEFFFFFFF;

	if (type==BDT_NON_NATIVE_FILE) {
	  bdev->non_native_drv=drv_let;
	  bdev->non_native_offset=A2I(buf2);
	} else {
	  bdev->non_native_drv=0;
	  bdev->non_native_offset=0;
	}

	if (type==BDT_ATA ||
	    type==BDT_ATAPI) {
	  bdev->base0=A2I(buf2);
	  bdev->base1=A2I(buf3);
	}

	add=TRUE;
	if (type==BDT_ATA && repartition) {
	  PutDefine("ST_MOUNT_FMT_WHOLE_DRV");
	  if (YorN) {
	    if (!BlkDevAdd(bdev,TRUE))
	      PutDefine("ST_FAILED");
	    else
	      CallExtStr("PrtDsk",bdev->drv_let_base);
	    add=FALSE;
	  }
	}
	if (add && !BlkDevAdd(bdev,FALSE))
	  PutDefine("ST_FAILED");
      } else
	PutDefine("ST_FAILED");
    }
  } while (0<=drv_let_base<NUM_PRTS);
}

I64 SysPmtI64()
{
  U8 st[136];
  StrCpy(st,"0x");
  GetS(st+2,134,FALSE);
  return A2I(st);
}

Bool PmtBaseUnit(CBlkDev *bdev)
{
  I64 ch;
  Bool probe;
  "            SparrowOS\n\n\n";
#exe {
  if (osmain_cfg->opts[CFG_DONT_PROBE])
    StreamPutS("probe=FALSE;");
  else
    StreamPutS("probe=TRUE;");
};
  if (!probe || !BootCdProbeAll(bdev)) {
	"\nThis is not a product registration.  SparrowOS\n"
	"needs the I/O port numbers for the CD-ROM.\n"
	"Retry the ports above or check Windows\n"
	"system information under I/O ports for\n"
	"'IDE', 'ATA' or 'SATA'.\n\n"
	"\nEnter 4-digit hex I/O Port number.\n"
	"CD-ROM I/O Port Base0: 0x";
    bdev->base0=SysPmtI64;
    bdev->base1=0;

    "\nUnit (0 or 1): ";
    do ch=GetChar(NULL,FALSE);
    while (!('0'<=ch<='1'));
    '' ch;
    bdev->unit=ch-'0';
    return TRUE;
  }
  return FALSE;
}

U0 BlkDevsInitAll()
{
  CBlkDev *bdev;
  I64 i;
  num_blkdevs=0;
  sys_blkdevs=ACAlloc(sizeof(CBlkDev)*NUM_BLKDEVS);
  sys_prts=ACAlloc(sizeof(CPrt)*NUM_PRTS);
  for (i=0;i<NUM_PRTS;i++)
    drv_to_prt[i]=&sys_prts[i];
  sys_raw_dc_col=0;
#exe {
   StreamPutS("#exe {OptOff(OPTf_WARN_PAREN);}");
   StreamDoc(osmain_cfg->add_dev);
   StreamPutS("#exe {OptOn(OPTf_WARN_PAREN);}");
};
  PrtRep;
}

