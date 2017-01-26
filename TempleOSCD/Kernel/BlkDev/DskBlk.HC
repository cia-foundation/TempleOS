#define ZERO_BUF_SIZE	2048
U0 BlkWriteZero(CDrv *dv,I64 blk,I64 cnt)
{//Fill blk cnt with zeros in Drv.
  I64 n;
  U8 *z=CAlloc(ZERO_BUF_SIZE<<BLK_SIZE_BITS);
  Bool show_progress;
  if (cnt>ZERO_BUF_SIZE && dv->bd->type!=BDT_RAM) {
    progress1=0; progress1_max=cnt;
    StrCpy(progress1_desc,"Zeroing");
    show_progress=TRUE;
  } else
    show_progress=FALSE;
  while (cnt>0) {
    n=cnt;
    if (n>ZERO_BUF_SIZE)
      n=ZERO_BUF_SIZE;
    BlkWrite(dv,z,blk,n);
    blk+=n;
    cnt-=n;
    if (show_progress)
      progress1+=n;
    Yield;  //Prevent locking
  }
  Free(z);
  if (show_progress) {
    *progress1_desc=0;
    progress1=progress1_max=0;
  }
}

Bool BlkRead(CDrv *dv,U8 *buf, I64 blk, I64 cnt)
{//Read blk cnt from Drv to buf.
  Bool res=TRUE,unlock;
  CBlkDev *bd=dv->bd;
  if (cnt<=0) return TRUE;
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    BlkDevInit(bd);
    if (dv->drv_offset && blk<dv->drv_offset ||
	  blk+cnt>dv->drv_offset+dv->size)
      throw('Drv');
    if (bd->flags & BDF_READ_CACHE)
      RCache(dv,&buf,&blk,&cnt);
    if (cnt>0) {
      switch (bd->type) {
	case BDT_RAM:
	  MemCpy(buf,bd->RAM_dsk+blk<<BLK_SIZE_BITS,cnt<<BLK_SIZE_BITS);
	  break;
	case BDT_ISO_FILE_READ:
	case BDT_ISO_FILE_WRITE:
	  FBlkRead(bd->file_dsk,buf,blk,cnt);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  res=ATARBlks(dv,buf,blk,cnt);
	  break;
      }
      bd->last_time=tS;
      if (bd->flags & BDF_READ_CACHE)
	DskCacheAdd(dv,buf,blk,cnt);
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  return res;
}

Bool BlkWrite(CDrv *dv,U8 *buf, I64 blk, I64 cnt)
{//Write blk cnt from buf to Drv.
  Bool res=TRUE,unlock;
  CBlkDev *bd=dv->bd;
  if (cnt<=0) return TRUE;
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    BlkDevInit(bd);
    if (bd->flags&BDF_READ_ONLY && !(bd->flags & BDF_READ_ONLY_OVERRIDE))
      throw('BlkDev');
    if (dv->drv_offset && blk<dv->drv_offset ||
	  blk+cnt>dv->drv_offset+dv->size)
      throw('Drv');
    if (cnt>0) {
      switch (bd->type) {
	case BDT_RAM:
	  MemCpy(bd->RAM_dsk+blk<<BLK_SIZE_BITS,buf,cnt<<BLK_SIZE_BITS);
	  break;
	case BDT_ISO_FILE_READ:
	case BDT_ISO_FILE_WRITE:
	  FBlkWrite(bd->file_dsk,buf,blk,cnt);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  res=ATAWBlks(dv,buf,blk,cnt);
	  break;
      }
      bd->last_time=tS;
      if (bd->flags & BDF_READ_CACHE)
	DskCacheAdd(dv,buf,blk,cnt);
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  return res;
}
