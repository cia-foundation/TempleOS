U0 ATABlkSelect(CBlkDev *bdev,I64 blk,I64 cnt)
{
  if (bdev->type!=BDT_ATAPI && bdev->base1)
    OutU8(bdev->base1+ATAR1_CTRL,0x8);
  if (bdev->flags & BDF_EXT_SIZE) {
    OutU8(bdev->base0+ATAR0_NSECT,cnt.u8[1]);
    OutU8(bdev->base0+ATAR0_SECT,blk.u8[3]);
    OutU8(bdev->base0+ATAR0_LCYL,blk.u8[4]);
    OutU8(bdev->base0+ATAR0_HCYL,blk.u8[5]);
    OutU8(bdev->base0+ATAR0_NSECT,cnt);
    OutU8(bdev->base0+ATAR0_SECT,blk);
    OutU8(bdev->base0+ATAR0_LCYL,blk.u8[1]);
    OutU8(bdev->base0+ATAR0_HCYL,blk.u8[2]);
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  } else {
    OutU8(bdev->base0+ATAR0_NSECT,cnt);
    OutU8(bdev->base0+ATAR0_SECT,blk);
    OutU8(bdev->base0+ATAR0_LCYL,blk.u8[1]);
    OutU8(bdev->base0+ATAR0_HCYL,blk.u8[2]);
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4|blk.u8[3]);
  }
}

Bool ATAWaitNotBUSY(CBlkDev *bdev,F64 timeout)
{
  while (TRUE) {
    if (timeout && tS>timeout)
      return FALSE;
    if (InU8(bdev->base0+ATAR0_STAT)&ATAS_BSY)
      Yield;
    else
      return TRUE;
  }
}

Bool ATAWaitDRQ(CBlkDev *bdev,F64 timeout)
{
  while (TRUE) {
    if (timeout && tS>timeout)
      return FALSE;
    if (!(InU8(bdev->base0+ATAR0_STAT)&ATAS_DRQ))
      Yield;
    else
      return TRUE;
  }
}

Bool ATANop(CBlkDev *bdev,F64 timeout)
{
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_FEAT,0);
  OutU8(bdev->base0+ATAR0_CMD,ATA_NOP);
  return ATAWaitNotBUSY(bdev,timeout);
}

U0 ATACmd(CBlkDev *bdev,U8 cmd)
{
  OutU8(bdev->base0+ATAR0_FEAT,0);
  OutU8(bdev->base0+ATAR0_CMD,cmd);
  bdev->last_time=tS;
  PortNop;
}

Bool ATAGetResult(CBlkDev *bdev,U8 *buf,I64 cnt,I64 _avail,Bool one_read,F64 timeout=0)
{
  I64 avail,overflow;
  bdev->flags&=~BDF_LAST_WAS_WRITE;
  MemSet(buf,0,cnt);
  while (cnt>0) {
    if (!ATAWaitDRQ(bdev,timeout))
      return FALSE;
    if (_avail)
      avail=_avail;
    else
      avail=InU8(bdev->base0+ATAR0_HCYL)<<8+InU8(bdev->base0+ATAR0_LCYL);
    if (avail) {
      if (avail>cnt) {
	overflow=avail-cnt;
	avail=cnt;
      } else
	overflow=0;
      if (avail&2)
	RepInU16(buf,avail>>1,bdev->base0+ATAR0_DATA);
      else
	RepInU32(buf,avail>>2,bdev->base0+ATAR0_DATA);
      cnt-=avail;
      buf+=avail;
      while (overflow>0) {
	if (timeout && tS>timeout)
	  return FALSE;
	InU16(bdev->base0+ATAR0_DATA);
	overflow-=2;
      }
      if (one_read)
	break;
    } else
      Yield;
  }
  return ATAWaitNotBUSY(bdev,timeout);
}

Bool ATAPIWritePktWord(CBlkDev *bdev,U16 w,F64 timeout=0)
{
  if (!ATAWaitDRQ(bdev,timeout))
    return FALSE;
  OutU16(bdev->base0+ATAR0_DATA,EndianU16(w));
  bdev->last_time=tS;
  return TRUE;
}

Bool ATAPISetMaxSpeed(CBlkDev *bdev)
{
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_LCYL,0);
  OutU8(bdev->base0+ATAR0_HCYL,0);
  ATACmd(bdev,ATA_PACKET);
  ATAPIWritePktWord(bdev,0xBB00); //Set speed
  ATAPIWritePktWord(bdev,0xFFFF); //read at max
  ATAPIWritePktWord(bdev,0xFFFF); //write at max
  ATAPIWritePktWord(bdev,0);
  ATAPIWritePktWord(bdev,0);
  ATAPIWritePktWord(bdev,0);
  return ATAWaitNotBUSY(bdev,0);
}


Bool ATAPISeek(CBlkDev *bdev,I64 native_blk)
{
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_LCYL,0);
  OutU8(bdev->base0+ATAR0_HCYL,0);
  ATACmd(bdev,ATA_PACKET);
  ATAPIWritePktWord(bdev,0x2B00); //Seek
  ATAPIWritePktWord(bdev,native_blk>>16);
  ATAPIWritePktWord(bdev,native_blk);
  ATAPIWritePktWord(bdev,0);
  ATAPIWritePktWord(bdev,0);
  ATAPIWritePktWord(bdev,0x0000);
  return ATAWaitNotBUSY(bdev,0);
}

Bool ATAPIStartStop(CBlkDev *bdev,Bool start,F64 timeout)
{
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  ATACmd(bdev,ATA_PACKET);

  //Start/Stop
  if (!ATAPIWritePktWord(bdev,0x1B00,timeout))		return FALSE;
  if (!ATAPIWritePktWord(bdev,0x0000,timeout))		return FALSE;
  if (start) {
    if (!ATAPIWritePktWord(bdev,0x0100,timeout))	return FALSE;
  } else {
    if (!ATAPIWritePktWord(bdev,0x0000,timeout))	return FALSE;
  }
  if (!ATAPIWritePktWord(bdev,0x0000,timeout))		return FALSE;
  if (!ATAPIWritePktWord(bdev,0x0000,timeout))		return FALSE;
  if (!ATAPIWritePktWord(bdev,0x0000,timeout))		return FALSE;
  return ATAWaitNotBUSY(bdev,timeout);
}

I64 ATAGetDevId(CBlkDev *bdev,U16 **_id_record,F64 timeout)
{
  U16 *id_record;
  if (_id_record) *_id_record=NULL;
  if (bdev->type!=BDT_ATAPI && bdev->base1)
    OutU8(bdev->base1+ATAR1_CTRL,0x8);
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  ATACmd(bdev,ATA_ID_DEV);
  if (!ATAWaitNotBUSY(bdev,timeout))
    return BDT_NULL;
  if (InU8(bdev->base0+ATAR0_STAT)&ATAS_ERR)
    return BDT_ATAPI;
  id_record=ACAlloc(512);
  if (!ATAGetResult(bdev,id_record,512,512,FALSE,timeout)) {
    Free(id_record);
    return BDT_NULL;
  }
  if (_id_record) *_id_record=id_record;
  return BDT_ATA;
}

I64 ATAReadNativeMax(CBlkDev *bdev,F64 timeout)
{ //returns zero on err
  I64 result=0;
  Bool okay=TRUE;

  if (bdev->type==BDT_ATAPI) {
    if (bdev->flags & BDF_EXT_SIZE)
      OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
    else
      OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
    ATACmd(bdev,ATA_DEV_RST);
    if (!ATAWaitNotBUSY(bdev,0))
      okay=FALSE;
  } else {
    while (InU8(bdev->base0+ATAR0_STAT) & ATAS_BSY) {
      if (timeout && tS>timeout)
	return FALSE;
      if (bdev->flags&BDF_LAST_WAS_WRITE)
	OutU16(bdev->base0+ATAR0_DATA,0);
      else
	InU16(bdev->base0+ATAR0_DATA);
      Yield;
    }

    Free(bdev->dev_id_record);
    if (ATAGetDevId(bdev,&bdev->dev_id_record,timeout)==BDT_NULL)
      okay=FALSE;
    else
      BEqu(&bdev->flags,BDf_EXT_SIZE,Bt(&bdev->dev_id_record[86],10));
  }
  if (okay) {
    if (bdev->flags & BDF_EXT_SIZE && bdev->base1) {
      OutU8(bdev->base1+ATAR1_CTRL,0x8);
      OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
      ATACmd(bdev,ATA_READ_NATIVE_MAX_EXT);
      if (ATAWaitNotBUSY(bdev,timeout)) {
	result.u8[0]=InU8(bdev->base0+ATAR0_SECT);
	result.u8[1]=InU8(bdev->base0+ATAR0_LCYL);
	result.u8[2]=InU8(bdev->base0+ATAR0_HCYL);

	OutU8(bdev->base1+ATAR1_CTRL,0x80);
	result.u8[3]=InU8(bdev->base0+ATAR0_SECT);
	result.u8[4]=InU8(bdev->base0+ATAR0_LCYL);
	result.u8[5]=InU8(bdev->base0+ATAR0_HCYL);
 
	if (result>>24==result&0xFFFFFF) {//Kludge to make qemu-kvm work
	  bdev->flags&=~BDF_EXT_SIZE;
	  result&=0xFFFFFF;
	}
      }
    } else {
      if (bdev->type!=BDT_ATAPI && bdev->base1)
	OutU8(bdev->base1+ATAR1_CTRL,0x8);
      OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
      ATACmd(bdev,ATA_READ_NATIVE_MAX);
      if (ATAWaitNotBUSY(bdev,timeout)) {
	result.u8[0]=InU8(bdev->base0+ATAR0_SECT);
	result.u8[1]=InU8(bdev->base0+ATAR0_LCYL);
	result.u8[2]=InU8(bdev->base0+ATAR0_HCYL);
	result.u8[3]=InU8(bdev->base0+ATAR0_SELECT) & 0xF;
      }
    }
  }
  return bdev->max_blk=result;
}

I64 ATAPIReadCapacity(CBlkDev *bdev,I64 *_blk_size=NULL)
{ //Supposedly this can return a result +/- 75 sects.
//Error might just be for music.
  Bool unlock=BlkDevLock(bdev);
  U32 buf[2];
  if (ATAWaitNotBUSY(bdev,0)) {
    if (bdev->flags & BDF_EXT_SIZE)
      OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
    else
      OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
    OutU8(bdev->base0+ATAR0_LCYL,8);
    OutU8(bdev->base0+ATAR0_HCYL,0);
    ATACmd(bdev,ATA_PACKET);
    ATAPIWritePktWord(bdev,0x2500);  //Read Capacity
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    if (!ATAGetResult(bdev,buf,8,0,TRUE))
      buf[0]=buf[1]=0;
  } else
    buf[0]=buf[1]=0;

  if (unlock) BlkDevUnlock(bdev);
  if (_blk_size) *_blk_size=EndianU32(buf[1]);
  return EndianU32(buf[0]);
}

CATAPITrack *ATAPIReadTrackInfo(CBlkDev *bdev,I64 blk)
{
  CATAPITrack *result=CAlloc(sizeof(CATAPITrack));
  Bool unlock=BlkDevLock(bdev);
  if (ATAWaitNotBUSY(bdev,0)) {
    if (bdev->flags & BDF_EXT_SIZE)
      OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
    else
      OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
    OutU8(bdev->base0+ATAR0_LCYL,sizeof(CATAPITrack)&0xFF);
    OutU8(bdev->base0+ATAR0_HCYL,sizeof(CATAPITrack)>>8);
    ATACmd(bdev,ATA_PACKET);
    ATAPIWritePktWord(bdev,0x5200);  //Read Track Info
    ATAPIWritePktWord(bdev,blk.u16[1]);
    ATAPIWritePktWord(bdev,blk.u16[0]);
    ATAPIWritePktWord(bdev,(sizeof(CATAPITrack)&0xFF00)>>8);
    ATAPIWritePktWord(bdev,(sizeof(CATAPITrack)&0x00FF)<<8);
    ATAPIWritePktWord(bdev,0x0000);
    if (!ATAGetResult(bdev,result,sizeof(CATAPITrack),0,TRUE)) {
      Free(result);
      result=NULL;
    }
  } else {
    Free(result);
    result=NULL;
  }
  if (unlock) BlkDevUnlock(bdev);
  return result;
}

Bool ATAInit(CBlkDev *bdev)
{
  Bool unlock=BlkDevLock(bdev),okay=FALSE;

  if (bdev->type==BDT_ATAPI)
    bdev->flags&=~BDF_EXT_SIZE;
  else
    bdev->flags|=BDF_EXT_SIZE;

  if (ATAReadNativeMax(bdev,tS+0.1)) {

    ATABlkSelect(bdev,bdev->max_blk,0);
    if (bdev->flags&BDF_EXT_SIZE)
      ATACmd(bdev,ATA_SET_MAX_EXT);
    else
      ATACmd(bdev,ATA_SET_MAX);
    if (ATAWaitNotBUSY(bdev,0)) {
      okay=TRUE;
      if (bdev->type==BDT_ATAPI) {
	if (ATAPIStartStop(bdev,TRUE,0)) {
	  if(!ATAPISetMaxSpeed(bdev))
	    okay=FALSE;
	} else
	  okay=FALSE;
      }
    }
  }
  if (unlock) BlkDevUnlock(bdev);
  return okay;
}

Bool ATAPIWaitReady(CBlkDev *bdev,F64 timeout)
{
  while (TRUE) {
    if (!ATAWaitNotBUSY(bdev,timeout) ||
	!ATANop(bdev,timeout) ||
	!ATAPIStartStop(bdev,TRUE,timeout))
      return FALSE;
    if (InU8(bdev->base0+ATAR0_STAT) & ATAS_DRDY &&
	!InU8(bdev->base0+ATAR0_FEAT));
      return TRUE;
    if (timeout && tS>timeout)
      return FALSE;
    ATAInit(bdev);
    Yield;
  }
}

U0 ATAReadBlks(CBlkDev *bdev,U8 *buf, I64 blk, I64 cnt)
{
  I64 retries=3;
  Bool unlock=BlkDevLock(bdev);

retry:
  ATABlkSelect(bdev,blk,cnt);
  if (bdev->flags & BDF_EXT_SIZE)
    ATACmd(bdev,ATA_READ_MULTI_EXT);
  else
    ATACmd(bdev,ATA_READ_MULTI);
  if (!ATAGetResult(bdev,buf,cnt*bdev->blk_size,BLK_SIZE,FALSE,tS+1.0)) {
    if (retries--) {
      ATAWaitNotBUSY(bdev,0);
      goto retry;
    } else
      throw(EXCEPT_BLKDEV,6);
  }

  sys_dsk_reads+=(cnt*bdev->blk_size)>>BLK_SIZE_BITS;
  if (unlock) BlkDevUnlock(bdev);
}

I64 ATAProbe(I64 base0,I64 base1,I64 unit)
{
  CBlkDev bdev;
  MemSet(&bdev,0,sizeof(CBlkDev));
  bdev.type=BDT_ATAPI;
  bdev.base0=base0;
  bdev.base1=base1;
  bdev.unit=unit;
  bdev.blk_size=CD_BLK_SIZE;
  return ATAGetDevId(&bdev,NULL,tS+0.1);
}

Bool ATAPIReadBlks2(CBlkDev *bdev,U8 *buf, I64 native_blk, I64 cnt,Bool lock,F64 timeout)
{
  Bool result,unlock;
  if (cnt<=0)
    return FALSE;
  if (lock)
    unlock=BlkDevLock(bdev);
  if (!ATAPIWaitReady(bdev,timeout))			goto arb_false_done;
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_LCYL,bdev->blk_size);
  OutU8(bdev->base0+ATAR0_HCYL,bdev->blk_size.u8[1]);
  ATACmd(bdev,ATA_PACKET);
  if (!ATAPIWritePktWord(bdev,0xA800,timeout))		goto arb_false_done;
  if (!ATAPIWritePktWord(bdev,native_blk.u16[1],timeout))	goto arb_false_done;
  if (!ATAPIWritePktWord(bdev,native_blk,timeout))		goto arb_false_done;
  if (!ATAPIWritePktWord(bdev,cnt.u16[1],timeout))	goto arb_false_done;
  if (!ATAPIWritePktWord(bdev,cnt,timeout))		goto arb_false_done;
  if (!ATAPIWritePktWord(bdev,0x0000,timeout))		goto arb_false_done;
  if (!ATAGetResult(bdev,buf,cnt*bdev->blk_size,0,FALSE,timeout)) {
arb_false_done:
    result=FALSE;
  } else {
    sys_dsk_reads+=(cnt*bdev->blk_size)>>BLK_SIZE_BITS;
    result=TRUE;
  }
//  ATAPIStartStop(bdev,FALSE,0);
  if (lock && unlock) BlkDevUnlock(bdev);
  return result;
}

U0 ATAPIReadBlks(CBlkDev *bdev,U8 *buf, I64 blk, I64 cnt)
{
  CPrt *p=Drv2Prt(bdev->first_drv_let);
  I64 retry,spc=bdev->blk_size>>BLK_SIZE_BITS,n,blk2,
    l2=bdev->max_reads<<1+spc<<1;
  U8 *cd_buf=MAlloc(l2<<BLK_SIZE_BITS);
  if (cnt>0) {
    if (blk<=bdev->max_reads)
      blk2=0;
    else
      blk2=FloorU64(blk-bdev->max_reads,spc);
    if (blk2+l2>p->size)
      l2=p->size-blk2;
    n=(l2+spc-1)/spc;

    retry=4;
    while (--retry)
      if (ATAPIReadBlks2(bdev,cd_buf,blk2/spc,n,TRUE,
	//n is 0x800 if max_reads.  Up to 8 additional seconds
	tS+7.0+0.004*n))
	break;
    if (!retry)
      ATAPIReadBlks2(bdev,cd_buf,blk2/spc,n,TRUE,0);
    if (bdev->flags & BDF_READ_CACHE)
      DskCacheAdd(p,cd_buf,blk2,n*spc);
    MemCpy(buf,cd_buf+(blk-blk2)<<BLK_SIZE_BITS,cnt<<BLK_SIZE_BITS);
  }
  Free(cd_buf);
}

U0 ATARBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt)
{
  I64 n;
  CBlkDev *bdev=p->bdev;
  BlkDevChk(bdev);
  while (cnt>0) {
    n=cnt;
    if (n>bdev->max_reads)
      n=bdev->max_reads;
    if (bdev->type==BDT_ATAPI)
      ATAPIReadBlks(bdev,buf,blk,n);
    else
      ATAReadBlks(bdev,buf,blk,n);
    buf+=n<<BLK_SIZE_BITS;
    blk+=n;
    cnt-=n;
  }
}

//This is for low level disk access
//Call WBlks() instead
U0 ATAWriteBlks(CBlkDev *bdev,U8 *buf, I64 blk, I64 cnt)
{
  I64 i,U32s_avail,sects_avail,retries=3;
  F64 timeout;
  Bool unlock=BlkDevLock(bdev);
retry:
  ATABlkSelect(bdev,blk,cnt);
  if (bdev->flags&BDF_EXT_SIZE)
    ATACmd(bdev,ATA_WRITE_MULTI_EXT);
  else
    ATACmd(bdev,ATA_WRITE_MULTI);
  bdev->flags|=BDF_LAST_WAS_WRITE;
  while (cnt>0) {
    timeout=tS+1.0;
    while (TRUE) {
      i=InU8(bdev->base0+ATAR0_STAT);
      if (!(i & ATAS_DRDY)||!(i & ATAS_DRQ)) {
	Yield;
      } else
	break;
      if (/* i&ATAS_ERR||*/ tS>timeout) {
	if (retries--) {
	  ATAWaitNotBUSY(bdev,0);
	  goto retry;
	} else
	  throw(EXCEPT_BLKDEV,7);
      }
    }
    sects_avail=1;
    U32s_avail=sects_avail<<BLK_SIZE_BITS>>2;
    RepOutU32(buf,U32s_avail,bdev->base0+ATAR0_DATA);
    buf+=U32s_avail<<2;
    cnt-=sects_avail;
    retries=3;
  }
  ATAWaitNotBUSY(bdev,0);
  if (unlock) BlkDevUnlock(bdev);
}

Bool ATAPISync(CBlkDev *bdev)
{
  Bool okay=TRUE;
  if (!ATAWaitNotBUSY(bdev,0))
    okay=FALSE;
  else {
    if (bdev->flags & BDF_EXT_SIZE)
      OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
    else
      OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
    OutU8(bdev->base0+ATAR0_LCYL,0);
    OutU8(bdev->base0+ATAR0_HCYL,0);
    ATACmd(bdev,ATA_PACKET);
    ATAPIWritePktWord(bdev,0x3500);  //Sync
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    ATAPIWritePktWord(bdev,0x0000);
    if (!ATAWaitNotBUSY(bdev,0))
      okay=FALSE;
  }
  return okay;
}

U0 ATAPIClose(CBlkDev *bdev,I64 close_field=0x200,I64 track=0)
{
//0x200 CD/DVD part 1
//0x300    DVD part 2
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_LCYL,0);
  OutU8(bdev->base0+ATAR0_HCYL,0);
  ATACmd(bdev,ATA_PACKET);
  ATAPIWritePktWord(bdev,0x5B00);  //Close
  ATAPIWritePktWord(bdev,close_field);
  ATAPIWritePktWord(bdev,track);
  ATAPIWritePktWord(bdev,0x0000);
  ATAPIWritePktWord(bdev,0x0000);
  ATAPIWritePktWord(bdev,0x0000);
  ATAWaitNotBUSY(bdev,0);
}

U0 ATAPIWriteBlks(CBlkDev *bdev,U8 *buf, I64 native_blk, I64 cnt)
{
  I64 U32s_avail;
  U8 *buf2;
  ATAWaitNotBUSY(bdev,0);
  ATAPISeek(bdev,native_blk);

  OutU8(bdev->base0+ATAR0_FEAT,0);
  OutU8(bdev->base0+ATAR0_LCYL,bdev->blk_size);
  OutU8(bdev->base0+ATAR0_HCYL,bdev->blk_size.u8[1]);
  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_CMD,ATA_PACKET);
  ATAPIWritePktWord(bdev,0x0400);  //FMT
  ATAPIWritePktWord(bdev,native_blk.u16[1]);
  ATAPIWritePktWord(bdev,native_blk);
  ATAPIWritePktWord(bdev,cnt.u16[1]);
  ATAPIWritePktWord(bdev,cnt);
  ATAPIWritePktWord(bdev,0x0000);
  bdev->flags|=BDF_LAST_WAS_WRITE;
  ATAWaitNotBUSY(bdev,0);

  ATAPISeek(bdev,native_blk);

  if (bdev->flags & BDF_EXT_SIZE)
    OutU8(bdev->base0+ATAR0_SELECT,0xEF|bdev->unit<<4);
  else
    OutU8(bdev->base0+ATAR0_SELECT,0xE0|bdev->unit<<4);
  OutU8(bdev->base0+ATAR0_LCYL,bdev->blk_size);
  OutU8(bdev->base0+ATAR0_HCYL,bdev->blk_size.u8[1]);
  ATACmd(bdev,ATA_PACKET);
  ATAPIWritePktWord(bdev,0xAA00);  //Write
  ATAPIWritePktWord(bdev,native_blk.u16[1]);
  ATAPIWritePktWord(bdev,native_blk);
  ATAPIWritePktWord(bdev,cnt.u16[1]);
  ATAPIWritePktWord(bdev,cnt);
  ATAPIWritePktWord(bdev,0x0000);
  buf2=buf+bdev->blk_size*cnt;
  while (buf<buf2) {
    ATAWaitDRQ(bdev,0);
    U32s_avail=(InU8(bdev->base0+ATAR0_HCYL)<<8+InU8(bdev->base0+ATAR0_LCYL))>>2;
    if (buf+U32s_avail<<2>buf2)
      U32s_avail=(buf2-buf)>>2;
    if (U32s_avail) {
      RepOutU32(buf,U32s_avail,bdev->base0+ATAR0_DATA);
      buf+=U32s_avail<<2;
      sys_dsk_writes+=U32s_avail>>(BLK_SIZE_BITS-2);
    }
  }
  ATAWaitNotBUSY(bdev,0);
}

U0 ATAWBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt)
{
  I64 n,spc;
  CBlkDev *bdev=p->bdev;
  Bool unlock;
  BlkDevChk(bdev);
  spc=bdev->blk_size>>BLK_SIZE_BITS;
  if (bdev->type==BDT_ATAPI) {
    unlock=BlkDevLock(bdev);
    ATAPIWaitReady(bdev,0);
  }
  while (cnt>0) {
    n=cnt;
    if (n>bdev->max_writes)
      n=bdev->max_writes;
    if (bdev->type==BDT_ATAPI)
      ATAPIWriteBlks(bdev,buf,blk/spc,(n+spc-1)/spc);
    else
      ATAWriteBlks(bdev,buf,blk,n);
    buf+=n<<BLK_SIZE_BITS;
    blk+=n;
    cnt-=n;
    sys_dsk_writes+=n;
  }
  if (bdev->type==BDT_ATAPI) {
    ATAPISync(bdev);
//    ATAPIStartStop(bdev,FALSE,0);
    if (unlock) BlkDevUnlock(bdev);
  }
}

Bool BootCdProbe(CBlkDev *bdev)
{
  CKernel *img=CAlloc(CD_BLK_SIZE);
  I64 i;
  Bool result=FALSE;
  "Port: %04X,%04X Unit: %02X  ",bdev->base0,bdev->base1,bdev->unit;
  if (ATAProbe(bdev->base0,bdev->base1,bdev->unit)==BDT_ATAPI) {
    " ATAPI";
    if (ATAPIStartStop(bdev,TRUE,tS+5.0)) {
      " Started";
      for (i=0;i<2;i++) { //Retry
	if (ATAPIReadBlks2(bdev,img,sys_boot_code.u16[1],1,FALSE,tS+7.0)) {
	  if (img->compile_time==sys_compile_time) {
	    " Found";
	    return TRUE;
	  } else
	    " Read";
	} else
	  " NoRead";
      }
    }
  }
  " Nope\n";
  Free(img);
  return result;
}

Bool BootCdProbeAll(CBlkDev *bdev)
{
  I64 i,j,k;
  U32 d1,d2;

  bdev->base1=0;
  for (k=0;k<256;k++) {
    i=-1;
    while (TRUE) {
      j=PCIClassFind(0x010100+k,++i);
      if (j<0)
	break;
      "Subcode:0x%X Bus:0x%X Dev:0x%X Fun:0x%X\n",k,j.u8[2],j.u8[1],j.u8[0];
      d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x10);
      d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x14);
      if (d1&1 && d2&1) {
	if (bdev->base0=d1&-8) {
	  bdev->unit=0;
	  if (BootCdProbe(bdev))
	    return TRUE;
	  bdev->unit=1;
	  if (BootCdProbe(bdev))
	    return TRUE;
	}
      }
      d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x18);
      d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x1C);
      if (d1&1 && d2&1) {
	if (bdev->base0=d1&-8) {
	  bdev->unit=0;
	  if (BootCdProbe(bdev))
	    return TRUE;
	  bdev->unit=1;
	  if (BootCdProbe(bdev))
	    return TRUE;
	}
      }
    }
  }

  d1=0x1F0;
  d2=0x3F6;
  if (bdev->base0=d1&-8) {
    bdev->unit=0;
    if (BootCdProbe(bdev))
      return TRUE;
    bdev->unit=1;
    if (BootCdProbe(bdev))
      return TRUE;
  }

  d1=0x170;
  d2=0x376;
  if (bdev->base0=d1&-8) {
    bdev->unit=0;
    if (BootCdProbe(bdev))
      return TRUE;
    bdev->unit=1;
    if (BootCdProbe(bdev))
      return TRUE;
  }
}

U0 ATARepEntry(I64 base0,I64 base1,I64 unit,I64 irq,U8 *msg,CATARep **_root,I64 *num_hints)
{
  I64 type;
  base0&=-8;
  base1&=-4;
  CATARep *tempha;
  if (type=ATAProbe(base0,base1,unit)) {
    *num_hints+=1;
    "\n\n$$FG,PURPLE$$ $$BT-LE+LM+X,\"%d\",\"%d\\n\"$$$$FG$$$$LM,4$$",*num_hints,*num_hints;
    if (type==BDT_ATA)
      "$$FG,RED$$ATA   ";
    else
      "$$FG,RED$$ATAPI ";
    "$$FG,LTBLUE$$%s$$FG$$\nBase0:0x%04X    Base1:0x%04X\nIrq:0x%02X Unit:%d$$LM,0$$\n",msg,base0,base1,irq,unit;
    if (_root) {
      tempha=CAlloc(sizeof(CATARep));
      tempha->next=*_root;
      *_root=tempha;
      tempha->num=*num_hints;
      tempha->type=type;
      tempha->base0=base0;
      tempha->base1=base1;
      tempha->unit=unit;
    }
  }
}

Bool ATARepExitAllApplications()
{
  PutDefine("ST_GOING_TO_PROBE_HARDWARE");
  PutDefine("ST_EXIT_ALL_APPLICATIONS");
  PutDefine("ST_PROBE_OR_SKIP");
  if (ToUpper(GetChar(,FALSE))=='S')
    return TRUE;
  else
    return FALSE;
}

public I64 ATARep(Bool pmt=TRUE,Bool just_ide=FALSE,CATARep **_root=NULL)
{
  U32 d1,d2;
  U8 d3;
  I64 i,j,k,cnt=0,unlock_flags=0,num_hints=0;
#assert NUM_BLKDEVS<=64
  Bool old_preempt;

  if (_root) *_root=NULL;

  if (pmt && ATARepExitAllApplications)
    return 0;

  old_preempt=Preempt(OFF);
  for (i=0;i<NUM_BLKDEVS;i++)
    if (sys_blkdevs[i].bd_signature==BD_SIGNATURE_VAL)
      BEqu(&unlock_flags,i,
	    BlkDevLock(&sys_blkdevs[i]));

  if (!just_ide)
    for (k=0;k<256;k++) {
      i=-1;
      while (TRUE) {
	j=PCIClassFind(0x010100+k,++i);
	if (j<0)
	  break;

	"\nSubcode:0x%X Bus:0x%X Dev:0x%X Fun:0x%X\n",k,j.u8[2],j.u8[1],j.u8[0];
	cnt++;

	d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x10);
	d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x14);
	d3=PCIReadU8(j.u8[2],j.u8[1],j.u8[0],0x3C);
	if (d1&1 && d2&1) {
	  ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);
	} else {
	  d1=0x1F0;
	  d2=0x3F6;
	  d3=14;
	  ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);
	}
	d1=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x18);
	d2=PCIReadU32(j.u8[2],j.u8[1],j.u8[0],0x1C);
	if (d1&1 && d2&1) {
	  ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
	} else {
	  d1=0x170;
	  d2=0x376;
	  d3=15;
	  ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
	  ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
	}
      }
    }
  if (!cnt) {
    d1=0x1F0;
    d2=0x3F6;
    d3=14;
    ATARepEntry(d1,d2,0,d3,"Primary IDE",_root,&num_hints);
    ATARepEntry(d1,d2,1,d3,"Primary IDE",_root,&num_hints);

    d1=0x170;
    d2=0x376;
    d3=15;
    ATARepEntry(d1,d2,0,d3,"Secondary IDE",_root,&num_hints);
    ATARepEntry(d1,d2,1,d3,"Secondary IDE",_root,&num_hints);
  }
  NewLine(2);
  for (i=0;i<NUM_BLKDEVS;i++)
    if (Bt(&unlock_flags,i))
      BlkDevUnlock(&sys_blkdevs[i]);
  Preempt(old_preempt);
  return num_hints;
}

CATARep *ATAIDDrvs(CATARep *root,U8 _sys_acct_drv,CATARep **_ata_drv,CATARep **_atapi_drv)
{
  CATARep *result=NULL,*tempha=root,*ata_drv=NULL,*atapi_drv=NULL;
  CBlkDev *bdev;
  Bool was_silent=Silent(ON);
  try
    bdev=Drv2BlkDev(_sys_acct_drv);
  catch {
    Fs->catch_except=TRUE;
    bdev=NULL;
  }
  Silent(was_silent);
  while (tempha) {
    if (!result && bdev && bdev->type==tempha->type) {
      if (bdev->type==BDT_ATAPI &&
	  bdev->base0==tempha->base0 && bdev->unit==tempha->unit)
	result=atapi_drv=tempha;
      else if (bdev->type==BDT_ATA && bdev->base0==tempha->base0 &&
	  bdev->base1==tempha->base1 && bdev->unit==tempha->unit)
	result=ata_drv=tempha;
    }
    if (!result || result->type!=tempha->type) {
      if (tempha->type==BDT_ATA) {
	if (!ata_drv || tempha->unit<ata_drv->unit ||
	  tempha->unit==ata_drv->unit && tempha->num<ata_drv->num)
	  ata_drv=tempha;
      } else if (tempha->type==BDT_ATAPI) {
	if (!atapi_drv || tempha->unit<atapi_drv->unit ||
	  tempha->unit==atapi_drv->unit && tempha->num<atapi_drv->num)
	  atapi_drv=tempha;
      }
    }
    tempha=tempha->next;
  }
  if (_ata_drv)   *_ata_drv  =ata_drv;
  if (_atapi_drv) *_atapi_drv=atapi_drv;
  return result;
}

CBlkDev *ATAMount(U8 first_drv_let,I64 type,I64 base0,I64 base1,I64 unit)
{
  CBlkDev *result;
  if (0<=first_drv_let-'A'<NUM_PRTS && (type==BDT_ATA || type==BDT_ATAPI) &&
      0<=unit<=1) {
    result=BlkDevNextFree(first_drv_let,type);
    result->unit=unit;
    result->base0=base0;
    result->base1=base1;
    if (BlkDevAdd(result,FALSE,FALSE))
      return result;
  }
  return NULL;
}

I64 AutoMountIDE()
{
  I64 result=0;
  CATARep *root=NULL,*ata_drv=NULL,*atapi_drv=NULL,*tempha;
  ATARep(FALSE,TRUE,&root);
  ATAIDDrvs(root,*sys_acct,&ata_drv,&atapi_drv);
  if (ata_drv && ATAMount(sys_first_hd_drv_let,BDT_ATA,
	ata_drv->base0,ata_drv->base1,ata_drv->unit))
    result++;
  if (atapi_drv && ATAMount(sys_first_cd_drv_let,BDT_ATAPI,
      atapi_drv->base0,atapi_drv->base1,atapi_drv->unit))
    result++;
  tempha=root;
  while (tempha) {
    if (tempha!=ata_drv && tempha!=atapi_drv) {
      if (tempha->type==BDT_ATA && ATAMount(sys_first_hd_drv_let,BDT_ATA,
	  tempha->base0,tempha->base1,tempha->unit))
	result++;
      else if (tempha->type==BDT_ATAPI && ATAMount(sys_first_cd_drv_let,BDT_ATAPI,
	  tempha->base0,tempha->base1,tempha->unit))
	result++;
    }
    tempha=tempha->next;
  }
  LinkedLstDel(root);
  sys_auto_mount_ide_cnt=result;
  return result;
}
