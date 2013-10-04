U0 RawAccess(I64 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let);
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  I64 blk,old_offset,old_size;
  U8 *buf=MAlloc(BLK_SIZE);

  PrtRep;
  PressAKey;

  ClassRep(p);
  PressAKey;

  ClassRep(bdev);
  PressAKey;

  //Temporarily change partition range to full device.
  //Normally, you are limited to just your partition.
  //This is obviously $BK,1$dangerous$BK,0$!
  old_offset=p->offset;
  old_size  =p->size;
  p->offset=bdev->min_blk;
  p->size=bdev->max_blk-bdev->min_blk+1;

  try {
    blk=PmtI64("Blk to Read: ",old_offset,bdev->min_blk,bdev->max_blk);
    RBlks(p,buf,blk,1);
    D(buf,BLK_SIZE);
  } catch
    CatchAll;

  p->offset=old_offset;
  p->size  =old_size;

  Free(buf);
}

RawAccess;

//See $LK,"RBlks","MN:RBlks"$(), $LK,"WBlks","MN:WBlks"$(), $LK,"PrtLock","MN:PrtLock"$(), $LK,"PrtUnlock","MN:PrtUnlock"$()
//$LK,"DBlk","MN:DBlk"$(), $LK,"DCluster","MN:DCluster"$(), $LK,"BlkDevLock","MN:BlkDevLock"$() and $LK,"BlkDevUnlock","MN:BlkDevUnlock"$().
