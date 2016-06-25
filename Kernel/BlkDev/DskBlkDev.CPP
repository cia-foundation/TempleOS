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
  CDirEntry de;
  U8 buf[STR_LEN];
  CDrv *dv=Let2Drv(bd->first_drv_let);
  Bool res=FALSE,invalid=FALSE;
  if (!LBts(&bd->flags,BDf_INITIALIZED)) {
    bd->flags|=BDF_INIT_IN_PROGRESS;
    switch (bd->type) {
      start:
	case BDT_RAM:
	  if (!bd->RAM_dsk) {
	    bd->RAM_dsk=AMAlloc((bd->max_blk+1)<<BLK_SIZE_BITS);
	    bd->max_blk=MSize(bd->RAM_dsk)>>BLK_SIZE_BITS-1;
	  }
	  break;
	case BDT_FILE:
	  if (!bd->file_dsk_name) {
	    StrPrint(buf,"%C:/Drv%C.DRV.C",
		  blkdev.boot_drv_let,bd->first_drv_let);
	    bd->file_dsk_name=AStrNew(buf);
	  }
	  if (FileFind(bd->file_dsk_name,&de,FUF_JUST_FILES))
	    bd->max_blk=de.size>>BLK_SIZE_BITS-1;
	  else {
	    invalid=TRUE;
	    if (bd->max_blk<7)
	      bd->max_blk=7;
	  }
	  bd->file_dsk=FOpen(bd->file_dsk_name,"wc+",bd->max_blk+1);
	  break;
      end:
	dv->fs_type=FSt_REDSEA;
	dv->size=bd->max_blk+1-bd->drv_offset;
	if (!invalid && RedSeaDrvValidate(bd->first_drv_let))
	  RedSeaDrvInit(dv);
	else
	  RedSeaFmt(bd->first_drv_let);
	res=TRUE;
	break;
      case BDT_ATA:
	bd->max_reads=128;
	bd->max_writes=1;
	res=ATAInit(bd);
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
	if (res=ATAInit(bd))
	  dv->size=bd->max_blk+1;
	break;
    }
    if (res && bd->flags & BDF_READ_CACHE)
      DskCacheInvalidate(dv);
    bd->flags&=~BDF_INIT_IN_PROGRESS;
  } else
    res=TRUE;
  return res;
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
  CBlkDev *res;
  if (Let2BlkDevType(first_drv_let)!=type)
    throw('BlkDev');
  do {
    res=&blkdev.blkdevs[i];
    if (res->bd_signature!=BD_SIGNATURE_VAL) {
      MemSet(res,0,sizeof(CBlkDev));
      res->first_drv_let=first_drv_let;
      res->type=type;
      if (type==BDT_ATAPI)
	res->flags=BDF_REMOVABLE;
      else
	res->flags=0;
      if (type!=BDT_RAM)
	res->flags|=BDF_READ_CACHE;

      if (type==BDT_ATAPI) {
	res->blk_size=DVD_BLK_SIZE;
	res->flags|=BDF_READ_ONLY;
      } else
	res->blk_size=BLK_SIZE;

      res->max_blk=0xEFFFFFFF;
      return res;
    }
  } while (++i<NUM_BLKDEVS);
  throw('BlkDev');
  return NULL; //never gets here
}

U0 BlkDevDel(CBlkDev *bd)
{//Delete BlkDev
  FClose(bd->file_dsk);
  Free(bd->file_dsk_name);
  Free(bd->dev_id_record);
  MemSet(bd,0,sizeof(CBlkDev));
}

CBlkDev *BlkDevChk(CBlkDev *bd,Bool except=TRUE)
{//Check for valid BlkDev. Throw exception.
  if (!bd || bd->bd_signature!=BD_SIGNATURE_VAL ||
	!(BDT_NULL<bd->type<BDT_NUM_TYPES)) {
    if (except)
      throw('BlkDev');
    else
      return NULL;
  } else
    return bd;
}

CBlkDev *Let2BlkDev(U8 drv_let=0,Bool except=TRUE)
{//Drv letter to BlkDev ptr.
  CDrv *dv;
  if (dv=Let2Drv(drv_let,except))
    return BlkDevChk(dv->bd,except);
  else
    return NULL;
}
