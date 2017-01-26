U0 RawAccess(I64 drv_let=0)
{
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=Let2BlkDev(drv_let);
  I64 blk,old_offset,old_size;
  U8 *buf=MAlloc(BLK_SIZE);

  DrvRep;
  PressAKey;

  ClassRep(dv);
  PressAKey;

  ClassRep(bd);
  PressAKey;

  //Temporarily change partition range to full device.
  //Normally, you are limited to just your partition.
  //This is obviously $BK,1$dangerous$BK,0$!
  old_offset=dv->drv_offset;
  old_size  =dv->size;
  dv->drv_offset=0;
  dv->size=bd->max_blk+1;

  try {
    blk=GetI64("Blk to Read: ",old_offset,0,bd->max_blk);
    BlkRead(dv,buf,blk,1);
    D(buf,BLK_SIZE);
  } catch
    PutExcept;

  dv->drv_offset=old_offset;
  dv->size  =old_size;

  Free(buf);
}

RawAccess;

//See $LK,"BlkRead",A="MN:BlkRead"$(), $LK,"BlkWrite",A="MN:BlkWrite"$(), $LK,"DrvLock",A="MN:DrvLock"$(), $LK,"DrvUnlock",A="MN:DrvUnlock"$()
//$LK,"DBlk",A="MN:DBlk"$(), $LK,"DClus",A="MN:DClus"$(), $LK,"BlkDevLock",A="MN:BlkDevLock"$() and $LK,"BlkDevUnlock",A="MN:BlkDevUnlock"$().
