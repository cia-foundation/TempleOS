Bool BlkDevLock(CBlkDev *bd)
{//Make this task have exclusive access to BlkDev.
  BlkDevChk(bd);
  while (bd->lock_fwding)
    bd=bd->lock_fwding; //If two blkdevs on same controller, use just one lock
  if (!Bt(&bd->locked_flags,BDlf_LOCKED) || bd->owning_task!=Fs) {
    while (LBts(&bd->locked_flags,BDlf_LOCKED))
      Yield;
    bd->owning_task=Fs;
    return TRUE;
  } else
    return FALSE;
}

Bool BlkDevUnlock(CBlkDev *bd,Bool rst=FALSE)
{//Release exclusive lock on access to BlkDev.
  BlkDevChk(bd);
  while (bd->lock_fwding)
    bd=bd->lock_fwding; //If two blkdevs on same controller, use just one lock
  if (Bt(&bd->locked_flags,BDlf_LOCKED) && bd->owning_task==Fs) {
    if (rst)
      bd->flags&=~(BDF_INITIALIZED|BDF_INIT_IN_PROGRESS);
    bd->owning_task=NULL;
    LBtr(&bd->locked_flags,BDlf_LOCKED);
    Yield; //Prevent deadlock
    return TRUE;
  } else
    return FALSE;
}

Bool BlkDevInit(CBlkDev *bd)
{
  CDrv *dv=Let2Drv(bd->first_drv_let);
  Bool result=FALSE;
  if (!LBts(&bd->flags,BDf_INITIALIZED)) {
    bd->flags|=BDF_INIT_IN_PROGRESS;
    switch (bd->type) {
      case BDT_RAM:
	if (!bd->RAM_dsk) {
	  bd->RAM_dsk=AMAlloc((bd->max_blk+1)<<BLK_SIZE_BITS);
	  bd->max_blk=MSize(bd->RAM_dsk)>>BLK_SIZE_BITS-1;
	}
	dv->type=DVT_REDSEA;
	dv->size=bd->max_blk+1;
	if (RedSeaDrvValidate(bd->first_drv_let))
	  RedSeaDrvInit(dv);
	else
	  RedSeaFmt(bd->first_drv_let);
	result=TRUE;
	break;
      case BDT_ATA:
	bd->max_reads=128;
	bd->max_writes=1;
	result=ATAInit(bd);
	break;
      case BDT_ATAPI:
//0xFFFF*4 is too big for my taste
	bd->max_reads=0x800*4;
//max of maybe a quarter of disk cache
	if (bd->max_reads>blkdev.cache_size/BLK_SIZE/4)
	  bd->max_reads=blkdev.cache_size/BLK_SIZE/4 & ~3;
	if (bd->max_reads<128)
	  bd->max_reads=128;
	bd->max_writes=0xFFFF*4;
	if (result=ATAInit(bd))
	  dv->size=bd->max_blk+1;
	break;
    }
    if (result && bd->flags & BDF_READ_CACHE)
      DskCacheInvalidate(dv);
    bd->flags&=~BDF_INIT_IN_PROGRESS;
  } else
    result=TRUE;
  return result;
}

U0 BlkDevsRelease()
{//When task dies, release all owned BlkDevs.
  I64 i;
  CBlkDev *bd;
  for (i=0;i<NUM_BLKDEVS;i++) {
    bd=&blkdev.blkdevs[i];
    if (bd->owning_task==Fs && bd->bd_signature==BD_SIGNATURE_VAL)
      BlkDevUnlock(bd,TRUE);
  }
}

CBlkDev *BlkDevNextFree(U8 first_drv_let,I64 type)
{//Locate free slot for new BlkDev, like during $LK,"Mount",A="MN:Mount"$().
  I64 i=0;
  CBlkDev *result;
  do {
    result=&blkdev.blkdevs[i];
    if (result->bd_signature!=BD_SIGNATURE_VAL) {
      MemSet(result,0,sizeof(CBlkDev));
      result->first_drv_let=first_drv_let;
      result->type=type;
      if (type==BDT_ATAPI)
	result->flags=BDF_REMOVABLE;
      else
	result->flags=0;
      if (type!=BDT_RAM)
	result->flags|=BDF_READ_CACHE;

      if (type==BDT_ATAPI) {
	result->blk_size=DVD_BLK_SIZE;
	result->flags|=BDF_READ_ONLY;
      } else
	result->blk_size=BLK_SIZE;

      result->max_blk=0xEFFFFFFF;
      return result;
    }
  } while (++i<NUM_BLKDEVS);
  throw('BlkDev');
  return NULL; //never gets here
}

U0 BlkDevChk(CBlkDev *bd)
{//Check for valid BlkDev. Throw exception.
  if (!bd || bd->bd_signature!=BD_SIGNATURE_VAL ||
	!(BDT_NULL<bd->type<BDT_NUM_TYPES))
    throw('BlkDev');
}

CBlkDev *Let2BlkDev(U8 drv_let=0)
{//Drv letter to BlkDev ptr.
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=dv->bd;
  BlkDevChk(bd);
  return bd;
}
