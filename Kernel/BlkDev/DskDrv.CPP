Bool DrvLock(CDrv *dv)
{//Make this task have exclusive access to drv & BlkDev.
  DrvChk(dv);
  BlkDevLock(dv->bd);
  if (!Bt(&dv->locked_flags,DVlf_LOCKED) || dv->owning_task!=Fs) {
    while (LBts(&dv->locked_flags,DVlf_LOCKED))
      Yield;
    dv->owning_task=Fs;
    return TRUE;
  } else
    return FALSE;
}

Bool DrvUnlock(CDrv *dv,Bool rst=FALSE)
{//Release exclusive lock on access to drv & BlkDev.
  DrvChk(dv);
  if (Bt(&dv->locked_flags,DVlf_LOCKED) && dv->owning_task==Fs) {
    BlkDevUnlock(dv->bd,rst);
    dv->owning_task=NULL;
    LBtr(&dv->locked_flags,DVlf_LOCKED);
    Yield; //Prevent deadlock
    return TRUE;
  } else
    return FALSE;
}

U0 DrvsRelease()
{//When task dies, release all owned drvs.
  I64 i;
  CDrv *dv;
  for (i=0;i<NUM_DRVS;i++) {
    dv=&blkdev.drvs[i];
    if (dv->owning_task==Fs && dv->dv_signature==DRV_SIGNATURE_VAL)
      DrvUnlock(dv,TRUE);
  }
}

CDrv *DrvMakeFree(U8 drv_let)
{//Make a slot free for a new drv, like during $LK,"Mount",A="MN:Mount"$().
//!!! drv_let is not a $LK,"remapped",A="MN:MapDrv"$ drv.
  I64 i=ToUpper(drv_let)-'A';
  CDrv *result;
  if (!(0<=i<NUM_DRVS))
    throw('Drv');
  result=&blkdev.drvs[i];
  MemSet(result,0,sizeof(CDrv));
  result->drv_let='A'+i;
  return result;
}

CDrv *DrvNextFree(U8 first_drv_let='C')
{//Locate free slot for new drv, like during $LK,"Mount",A="MN:Mount"$().
//!!! first_drv_let is not a $LK,"remapped",A="MN:MapDrv"$ drv.
  I64 i=ToUpper(first_drv_let)-'A';
  if (!(0<=i<NUM_DRVS))
    throw('Drv');
  do
    if (blkdev.drvs[i].dv_signature!=DRV_SIGNATURE_VAL)
      return DrvMakeFree(i+'A');
  while (++i<NUM_DRVS);
  throw('Drv');
  return NULL; //never gets here
}

U0 DrvFATBlkAlloc(CDrv *dv)
{
  DrvChk(dv);
  Free(dv->cur_fat_blk);
  dv->cur_fat_blk=AMAlloc(BLK_SIZE);
  dv->cur_fat_blk_num=0;
  dv->fat_blk_dirty=0;
  RBlks(dv,dv->cur_fat_blk,dv->fat1,1);
}

U0 DrvFATBlkClean(CDrv *dv,I64 fat_select=3)
{
  if ((dv->type==DVT_FAT32 || dv->type==DVT_REDSEA)&&Bt(&dv->fat_blk_dirty,0)) {
    if (dv->fat1==dv->fat2) {
      WBlks(dv,dv->cur_fat_blk,dv->fat1+dv->cur_fat_blk_num,1);
      LBtr(&dv->fat_blk_dirty,0);
    } else {
      if (fat_select==3 || !fat_select)
	WBlks(dv,dv->cur_fat_blk,dv->fat1+dv->cur_fat_blk_num,1);
      if (fat_select==3 || fat_select==1) {
	WBlks(dv,dv->cur_fat_blk,dv->fat2+dv->cur_fat_blk_num,1);
	LBtr(&dv->fat_blk_dirty,0);
      }
    }
  }
}

U0 DrvFATBlkSet(CDrv *dv,I64 c,I64 fat_select=3)
{
  I64 fat_blk_num;
  if (c==INVALID_CLUSTER)
    throw('Drv');
  switch (dv->type) {
    case DVT_FAT32:
      fat_blk_num=c>>(BLK_SIZE_BITS-2);
      break;
    case DVT_REDSEA:
      fat_blk_num=(c-dv->data_area)>>(BLK_SIZE_BITS+3);
      break;
    default:
      throw('Drv');
  }
  if (fat_blk_num!=dv->cur_fat_blk_num) {
    DrvFATBlkClean(dv,fat_select);
    dv->cur_fat_blk_num=fat_blk_num;
    if (fat_select==3 || !fat_select)
      RBlks(dv,dv->cur_fat_blk,dv->fat1+dv->cur_fat_blk_num,1);
    else
      RBlks(dv,dv->cur_fat_blk,dv->fat2+dv->cur_fat_blk_num,1);
  }
}

U0 DrvChk(CDrv *dv)
{//Check for valid drv. Throw exception.
  if (!dv || dv->dv_signature!=DRV_SIGNATURE_VAL)
    throw('Drv');
}

U8 Drv2Let(CDrv *dv=NULL)
{//Drv ptr to Drv letter.
  if (!dv)
    dv=Fs->cur_dv;
  DrvChk(dv);
  return dv->drv_let;
}

CDrv *Let2Drv(U8 drv_let=0)
{//Drv letter to Drv ptr.
  CDrv *dv;
  if (!drv_let)
    dv=Fs->cur_dv;
  else {
    if (drv_let==':')
      drv_let=blkdev.boot_drv_let;
    drv_let=ToUpper(drv_let);
    if (!('A'<=drv_let<='Z'))
      throw('Drv');
    dv=blkdev.let_to_drv[drv_let-'A'];
  }
  DrvChk(dv);
  return dv;
}

Bool DrvIsWritable(U8 drv_let=0)
{//Is drive writable?
  CBlkDev *bd=Let2BlkDev(drv_let);
  if (bd->flags & BDF_READ_ONLY)
    return FALSE;
  else
    return TRUE;
}

U0 DskCacheInvalidate(CDrv *dv)
{//Needed for removable media. Called by $LK,"ChgDsk",A="MN:ChgDsk"$().
  Bool unlock;
  CBlkDev *bd=dv->bd;
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    BlkDevInit(bd);
    if (bd->flags & BDF_READ_CACHE)
      DskCacheInvalidate2(dv);
    if (dv->type==DVT_ISO9660 && !(bd->flags & BDF_READ_ONLY_OVERRIDE))
      ISODrvInit(dv,(32767/bd->blk_size+1)*bd->blk_size>>BLK_SIZE_BITS);
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
}

U0 ChgDsk(U8 drv_let=0)
{//Change disk. (Needed for removable media.)
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=dv->bd;
  if (!(bd->flags&BDF_INITIALIZED))
    BlkDevInit(bd);
  else if (bd->flags&BDF_REMOVABLE)
    DskCacheInvalidate(dv);
  Drv(drv_let);
  RedSeaFreeFreeLst(dv);
}

Bool MapDrv(U8 drv_let,CDrv *dv)
{//Make drive letter map to another.
  drv_let=ToUpper(drv_let);
  if ('A'<=drv_let<='Z') {
    blkdev.let_to_drv[drv_let-'A']=dv;
    dv->drv_let=drv_let;
    return TRUE;
  } else
    return FALSE;
}

Bool Drv(U8 drv_let)
{//Set drive.  You can set drive with $LK,"Cd",A="MN:Cd"$() as well.
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=dv->bd;
  BlkDevChk(bd);
  if (dv!=Fs->cur_dv) {
    if (bd->flags & BDF_REMOVABLE && !(bd->flags & BDF_INITIALIZED))
      ChgDsk(Drv2Let(dv));
    if (bd->type==BDT_RAM)
      BlkDevInit(bd);
  }
  Fs->cur_dv=dv;
  Free(Fs->cur_dir);
  Fs->cur_dir=StrNew("/");
  switch (dv->type) {
    case DVT_REDSEA:
    case DVT_FAT32:
    case DVT_ISO9660:
      return TRUE;
    default:
      PrintErr("File System Not Supported\n");
      return FALSE;
  }
}

U8 *DrvSerialNum(U8 drv_let=0)
{//20 bytes max.
  CBlkDev *bd=Let2BlkDev(drv_let);
  U16 *st,*result=NULL;
  I64 i;
  if (bd->dev_id_record) {
    st=CAlloc(20+1);
    for (i=0;i<10;i++)
      st[i]=EndianU16(bd->dev_id_record[10+i]);
    result=MStrUtil(st,SUF_REM_LEADING|SUF_REM_TRAILING);
    Free(st);
  }
  return result;
}

U8 *DrvModelNum(U8 drv_let=0)
{//40 bytes max.
  CBlkDev *bd=Let2BlkDev(drv_let);
  U16 *st,*result=NULL;
  I64 i;
  if (bd->dev_id_record) {
    st=CAlloc(40+1);
    for (i=0;i<20;i++)
      st[i]=EndianU16(bd->dev_id_record[27+i]);
    result=MStrUtil(st,SUF_REM_LEADING|SUF_REM_TRAILING);
    Free(st);
  }
  return result;
}

U0 DrvRep()
{//Drive report.
  CDrv *dv;
  CBlkDev *bd;
  I64 i,drv_let;
  U8 *st;
  "\nDefined Drives:\n\n";
  for (i=0,dv=blkdev.drvs;i<NUM_DRVS;i++,dv++) {
    if (dv->dv_signature==DRV_SIGNATURE_VAL) {
      bd=dv->bd;
      drv_let=Drv2Let(dv);
      if (Bt(&dv->type,DVTf_DISABLE))
	"$$RED$$X";
      else if (drv_let==blkdev.boot_drv_let)
	"$$GREEN$$:";
      else
	'' CH_SPACE;
      "$$PURPLE$$%C %-8Z %-9Z %04X %04X %02X$$FG$$\n",
	    drv_let,dv->type.u16[0],"ST_DRV_TYPES",
	    bd->type,"ST_BLKDEV_TYPES",bd->base0,bd->base1,bd->unit;
      if (st=DrvModelNum(drv_let)) {
	"   $$CYAN$$Model# :%s$$FG$$\n",st;
	Free(st);
      }
      if (st=DrvSerialNum(drv_let)) {
	"   $$CYAN$$Serial#:%s$$FG$$\n",st;
	Free(st);
      }
      "   %016X-%016X\n",
	    dv->base_offset,dv->base_offset+dv->size-1;
      dv->base_offset,dv->base_offset+dv->size-1;
    }
  }
}
