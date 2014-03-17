Bool BlkDevLock(CBlkDev *bdev)
{
  BlkDevChk(bdev);
  while (bdev->lock_fwding)
    bdev=bdev->lock_fwding;
  if (!Bt(&bdev->locked_flags,BDlf_LOCKED) || bdev->owning_task!=Fs) {
    while (LBts(&bdev->locked_flags,BDlf_LOCKED))
      Yield;
    bdev->owning_task=Fs;
    return TRUE;
  } else
    return FALSE;
}

Bool PrtLock(CPrt *p)
{
  PrtChk(p);
  BlkDevLock(p->bdev);
  if (!Bt(&p->locked_flags,Plf_LOCKED) || p->owning_task!=Fs) {
    while (LBts(&p->locked_flags,Plf_LOCKED))
      Yield;
    p->owning_task=Fs;
    return TRUE;
  } else
    return FALSE;
}

Bool BlkDevUnlock(CBlkDev *bdev,Bool rst=FALSE)
{
  BlkDevChk(bdev);
  while (bdev->lock_fwding)
    bdev=bdev->lock_fwding;
  if (Bt(&bdev->locked_flags,BDlf_LOCKED) && bdev->owning_task==Fs) {
    if (rst)
      bdev->flags&=~(BDF_INITIALIZED|BDF_INIT_IN_PROGRESS);
    bdev->owning_task=NULL;
    LBtr(&bdev->locked_flags,BDlf_LOCKED);
    Yield; //Prevent deadlock
    return TRUE;
  } else
    return FALSE;
}

Bool PrtUnlock(CPrt *p,Bool rst=FALSE)
{
  PrtChk(p);
  if (Bt(&p->locked_flags,Plf_LOCKED) && p->owning_task==Fs) {
    BlkDevUnlock(p->bdev,rst);
    p->owning_task=NULL;
    LBtr(&p->locked_flags,Plf_LOCKED);
    Yield; //Prevent deadlock
    return TRUE;
  } else
    return FALSE;
}

Bool BlkDevInit(CBlkDev *bdev)
{
  CPrt *p=Drv2Prt(bdev->first_drv_let);
  Bool old_preempt=Preempt(OFF),result=FALSE;

  if (!LBts(&bdev->flags,BDf_INITIALIZED)) {
    bdev->flags|=BDF_INIT_IN_PROGRESS;
    switch(bdev->type) {
      case BDT_RAM:
	if (!bdev->RAM_dsk) {
	  bdev->RAM_dsk=AMAlloc((bdev->max_blk+1)<<BLK_SIZE_BITS);
	  bdev->max_blk=MSize(bdev->RAM_dsk)>>BLK_SIZE_BITS-1;
	}
	p->type=PT_REDSEA;
	p->size=bdev->max_blk+1;
	if (RedSeaFSDrvValidate(bdev->first_drv_let))
	  RedSeaFSPrtInit(p);
	else
	  RedSeaFSFmt(bdev->first_drv_let);
	result=TRUE;
	break;
      case BDT_ATA:
	bdev->max_reads=128;
	bdev->max_writes=1;
	result=ATAInit(bdev);
	break;
      case BDT_ATAPI:
	//0xFFFF*4 is too big for my taste
	bdev->max_reads=0x800*4;
	//max of maybe a quarter of disk cache
	if (bdev->max_reads>sys_dsk_cache_size/BLK_SIZE/4)
	  bdev->max_reads=sys_dsk_cache_size/BLK_SIZE/4 & ~3;
	if (bdev->max_reads<128)
	  bdev->max_reads=128;
	bdev->max_writes=0xFFFF*4;
	if (result=ATAInit(bdev))
	  p->size=bdev->max_blk+1;
	break;
    }
    if (result && bdev->flags & BDF_READ_CACHE)
      DskCacheInvalidate(p);
    bdev->flags&=~BDF_INIT_IN_PROGRESS;
  } else
    result=TRUE;
  Preempt(old_preempt);
  return result;
}

U0 BlkDevsRelease()
{
  I64 i;
  CBlkDev *bdev;
  for (i=0;i<NUM_BLKDEVS;i++) {
    bdev=&sys_blkdevs[i];
    if (bdev->owning_task==Fs && bdev->bd_signature==BD_SIGNATURE_VAL)
      BlkDevUnlock(bdev,TRUE);
  }
}

U0 PrtsRelease()
{
  I64 i;
  CPrt *p;
  for (i=0;i<NUM_PRTS;i++) {
    p=&sys_prts[i];
    if (p->owning_task==Fs && p->pt_signature==PRT_SIGNATURE_VAL)
      PrtUnlock(p,TRUE);
  }
}

CPrt *PrtMakeFree(U8 drv_let)
{//!!! drv is not a remapped prt.
  I64 i=ToUpper(drv_let)-'A';
  CPrt *result;
  if (!(0<=i<NUM_PRTS))
    throw('Prt');
  result=&sys_prts[i];
  MemSet(result,0,sizeof(CPrt));
  result->drv_let='A'+i;
  return result;
}

CPrt *PrtNextFree(U8 first_drv_let='C')
{//!!! first_drv_let is not a remapped prt.
  I64 i=ToUpper(first_drv_let)-'A';
  if (!(0<=i<NUM_PRTS))
    throw('Prt');
  do
    if (sys_prts[i].pt_signature!=PRT_SIGNATURE_VAL)
      return PrtMakeFree(i+'A');
  while (++i<NUM_PRTS);
  throw('Prt');
  return NULL; //never gets here
}

CBlkDev *BlkDevNextFree(U8 first_drv_let,I64 type)
{
  I64 i=0;
  CBlkDev *result;
  do {
    result=&sys_blkdevs[i];
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
	result->blk_size=CD_BLK_SIZE;
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

U0 MapBufAlloc(CPrt *p,I64 num_blks)
{
  PrtChk(p);
  Free(p->cur_map_blk);
  Free(p->buffered_map_dirty_bits);
  p->num_buffered_map_blks=num_blks;
  p->cur_map_blk=AMAlloc(num_blks<<BLK_SIZE_BITS);
  p->buffered_map_dirty_bits=ACAlloc((num_blks+7)>>3);
  p->cur_map_blk_num=0;
  RBlks(p,p->cur_map_blk,p->map1,p->num_buffered_map_blks);
}


U0 CleanMAPBuf(CPrt *p,I64 MAP_select=-1)
{
  I64 i;
  if ((MAP_select==-1 || !MAP_select) && p->map1!=p->map2)
    for (i=0;i<p->num_buffered_map_blks;i++)
      if (Bt(p->buffered_map_dirty_bits,i))
	WBlks(p,p->cur_map_blk(I64)+i<<BLK_SIZE_BITS,p->map1+i+p->cur_map_blk_num,1);
  if (MAP_select==-1 || MAP_select==1 || p->map1==p->map2)
    for (i=0;i<p->num_buffered_map_blks;i++)
      if (LBtr(p->buffered_map_dirty_bits,i))
	WBlks(p,p->cur_map_blk(I64)+i<<BLK_SIZE_BITS,p->map2+i+p->cur_map_blk_num,1);
}

U0 SetFSBuf(CPrt *p,I64 c,I64 map_select=-1)
{
  I64 MAP_blk_num;
  if (c==INVALID_CLUSTER)
    throw('Prt');
  if (p->type==PT_FAT32)
    MAP_blk_num=c>>(BLK_SIZE_BITS-2);
  else
    MAP_blk_num=c>>(BLK_SIZE_BITS+3);
  if (MAP_blk_num!=p->cur_map_blk_num) {
    CleanMAPBuf(p,map_select);
    p->cur_map_blk_num=MAP_blk_num;
    if (map_select==-1 || !map_select)
      RBlks(p,p->cur_map_blk,p->map1+p->cur_map_blk_num,1);
    else
      RBlks(p,p->cur_map_blk,p->map2+p->cur_map_blk_num,1);
  }
}

#define ZERO_BUF_SIZE	2048
U0 WZeroBlks(CPrt *p,I64 blk,I64 cnt)
{
  I64 n;
  U8 *z=CAlloc(ZERO_BUF_SIZE<<BLK_SIZE_BITS);
  Bool show_progress;
  if (cnt>ZERO_BUF_SIZE && p->bdev->type!=BDT_RAM) {
    progress1=0; progress1_max=cnt;
    StrCpy(progress1_desc,"Zeroing");
    show_progress=TRUE;
  } else
    show_progress=FALSE;
  while (cnt>0) {
    n=cnt;
    if (n>ZERO_BUF_SIZE)
      n=ZERO_BUF_SIZE;
    WBlks(p,z,blk,n);
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

//Very dangerous
Bool SetDrvType(U8 drv_let,I64 type=PT_REDSEA)
{
  I64 i,j,ext_base,prt_num,offset,cur_type;
  CMasterBoot mbr;
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  prt_num=bdev->first_drv_let-'A';
  switch (bdev->type) {
    case BDT_ATA:
      offset=0;
      ext_base=INVALID_CLUSTER;
      while (TRUE) {
	ATAReadBlks(bdev,&mbr,offset,1);
	j=-1;
	for (i=0;i<4;i++) {
	  cur_type=mbr.p[i].type;
	  if (cur_type) {
	    if (cur_type==5 || cur_type==15)
	      j=i;
	    else {
	      if (prt_num+'A'==ToUpper(drv_let)) {
		switch (type) {
		  case PT_REDSEA:
		    mbr.p[i].type=MBR_PT_REDSEA;
		    break;
		  case PT_FAT32:
		    mbr.p[i].type=MBR_PT_FAT32a;
		    break;
		  default:
		    Dbg;
		}
		mbr.p[i].active=0x80;
		ATAWriteBlks(bdev,&mbr,offset,1);
		return TRUE;
	      }
	      prt_num++;
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
  }
}

Bool RBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt)
{
  Bool result=TRUE,unlock;
  CBlkDev *bdev=p->bdev;
  if (cnt<=0) return TRUE;
  PrtChk(p);
  try {
    unlock=PrtLock(p);
    BlkDevInit(bdev);
    if (p->offset && blk<p->offset || blk+cnt>p->offset+p->size)
      throw('Prt');
    blk+=bdev->offset;
 
    if (bdev->flags & BDF_READ_CACHE)
      RCache(p,&buf,&blk,&cnt);
    if (cnt>0) {
      switch (bdev->type) {
	case BDT_RAM:
	  MemCpy(buf,bdev->RAM_dsk+blk<<BLK_SIZE_BITS,cnt<<BLK_SIZE_BITS);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  result=ATARBlks(p,buf,blk,cnt);
	  break;
      }
      bdev->last_time=tS;
      if (bdev->flags & BDF_READ_CACHE)
	DskCacheAdd(p,buf,blk,cnt);
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  return result;
}

Bool WBlks(CPrt *p,U8 *buf, I64 blk, I64 cnt)
{
  Bool result=TRUE,unlock;
  CBlkDev *bdev=p->bdev;
  if (cnt<=0) return TRUE;
  PrtChk(p);
  try {
    unlock=PrtLock(p);
    BlkDevInit(bdev);
    if (bdev->flags&BDF_READ_ONLY &&
    !(bdev->flags & BDF_READ_ONLY_OVERRIDE))
      throw('BlkDev');
    if (p->offset && blk<p->offset || blk+cnt>p->offset+p->size)
      throw('Prt');
    blk+=bdev->offset;
    if (cnt>0) {
      switch (bdev->type) {
	case BDT_RAM:
	  MemCpy(bdev->RAM_dsk+blk<<BLK_SIZE_BITS,buf,cnt<<BLK_SIZE_BITS);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  result=ATAWBlks(p,buf,blk,cnt);
	  break;
      }
      bdev->last_time=tS;
      if (bdev->flags & BDF_READ_CACHE)
	DskCacheAdd(p,buf,blk,cnt);
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  return result;
}


U0 DskCacheInvalidate(CPrt *p)
{
  Bool unlock;
  CBlkDev *bdev=p->bdev;
  PrtChk(p);
  try {
    unlock=PrtLock(p);
    BlkDevInit(bdev);
    if (bdev->flags & BDF_READ_CACHE)
      DskCacheInvalidate2(p);
    if (p->type==PT_ISO9660) {
      if (!(bdev->flags & BDF_READ_ONLY_OVERRIDE))
	ISOPrtInit(p,(32767/bdev->blk_size+1)*
	  bdev->blk_size>>BLK_SIZE_BITS);
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
}

U0 ChgDsk(U8 drv_let=0)
{ //This is needed for removable media
  CPrt *p=Drv2Prt(drv_let);
  CBlkDev *bdev=p->bdev;
  Bool old_preempt=Preempt(OFF);
  if (!(bdev->flags&BDF_INITIALIZED))
    BlkDevInit(bdev);
  else if (bdev->flags&BDF_REMOVABLE)
    DskCacheInvalidate(p);
  Drv(drv_let);
  RedSeaFSFreeFreeLst(p);
  Preempt(old_preempt);
}

U8 *DrvSerialNum(U8 drv_let=0)
{ //20 bytes max.
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  U16 *st,*result=NULL;
  I64 i;
  if (bdev->dev_id_record) {
    st=CAlloc(20+1);
    for (i=0;i<10;i++)
      st[i]=EndianU16(bdev->dev_id_record[10+i]);
    result=MStrUtil(st,SUF_REM_LEADING|SUF_REM_TRAILING);
    Free(st);
  }
  return result;
}

U8 *DrvModelNum(U8 drv_let=0)
{ //40 bytes max.
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  U16 *st,*result=NULL;
  I64 i;
  if (bdev->dev_id_record) {
    st=CAlloc(40+1);
    for (i=0;i<20;i++)
      st[i]=EndianU16(bdev->dev_id_record[27+i]);
    result=MStrUtil(st,SUF_REM_LEADING|SUF_REM_TRAILING);
    Free(st);
  }
  return result;
}

U0 PrtRep()
{ //Partition report
  CPrt *p;
  CBlkDev *bdev;
  I64 i,drv_let;
  U8 *st;
  "\nDefined Partitions:\n\n";
  for (i=0,p=sys_prts;i<NUM_PRTS;i++,p++) {
    if (p->pt_signature==PRT_SIGNATURE_VAL) {
      bdev=p->bdev;
      drv_let=Prt2Drv(p);
      if (Bt(&p->type,PTf_DISABLE))
	"$$RED$$X";
      else if (drv_let==*sys_acct)
	"$$GREEN$$:";
      else
	'' CH_SPACE;
      "$$PURPLE$$%C %-8Z %-9Z %04X %04X %02X$$FG$$\n",
	drv_let,p->type.u16[0],"ST_PRT_TYPES",
	bdev->type,"ST_BLKDEV_TYPES",bdev->base0,bdev->base1,bdev->unit;
      if (st=DrvModelNum(drv_let)) {
	"   $$CYAN$$Model# :%s$$FG$$\n",st;
	Free(st);
      }
      if (st=DrvSerialNum(drv_let)) {
	"   $$CYAN$$Serial#:%s$$FG$$\n",st;
	Free(st);
      }
      "   %016X-%016X\n",
	p->offset,p->offset+p->size-1;
	p->offset,p->offset+p->size-1;
    }
  }
}

I64 ClusterNumNext(CPrt *p,I64 c,I64 cnt=1)
{
  Bool unlock;
  PrtChk(p);
  if (cnt<=0) return c;
  try {
    unlock=PrtLock(p);
    switch (p->type) {
      case PT_ISO9660:
      case PT_REDSEA:
	c+=cnt;
	break;
      case PT_FAT32:
	while (cnt-->0 && 0<c<0x0FFFFFF8) {
	  SetFSBuf(p,c);
	  c=p->cur_map_blk[c & (BLK_SIZE/4-1)];
	}
	if (!(0<c<0x0FFFFFF8))
	  c=INVALID_CLUSTER;
	break;
      default:
	Dbg;
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  return c;
}


I64 Cluster2Blk(CPrt *p,I64 c)
{
  PrtChk(p);
  return p->data+c*p->spc;
}

I64 RClustersBlks(CPrt *p,U8 *buf,I64 c,I64 blks)
{ //This accepts blk count, so padding on last cluster is not read.
  I64 i;
  Bool unlock;
  PrtChk(p);
  if (blks<=0) return c;
  try {
    unlock=PrtLock(p);
    switch (p->type) {
      case PT_ISO9660:
      case PT_REDSEA:
	RBlks(p,buf,p->data+c*p->spc,blks);
	c+=(blks+p->spc-1)/p->spc;
	break;
      case PT_FAT32:
	while (blks && 0<c<0x0FFFFFF8) {
	  i=blks;
	  if (i>p->spc)
	    i=p->spc;
	  RBlks(p,buf,p->data+c*p->spc,i);
	  buf+=i<<BLK_SIZE_BITS;
	  c=ClusterNumNext(p,c,1);
	  blks-=i;
	}
	if (blks)
	  Dbg;
	break;
      default:
	Dbg;
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  return c;
}

I64 RClusters(CPrt *p,U8 *buf,I64 c,I64 cnt)
{
  return RClustersBlks(p,buf,c,cnt*p->spc);
}

Bool CFileNameTo(U8 *dst,U8 *src)
{
  MemSet(dst,0,CDIR_FILENAME_LEN);
  if (!FileNameChk(src))
    return FALSE;
  StrCpy(dst,src);
  return TRUE;
}

I64 Name2DirCluster(CPrt *p,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_cluster;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  PrtChk(p);
  try {
    unlock=PrtLock(p);
    cur_dir_cluster=p->root_cluster;
    while (*buf && cont) {
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (p->type) {
	  case PT_REDSEA:
	    cont=RedSeaFSFileFind(p,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case PT_FAT32:
	    cont=FAT32FileFind(p,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case PT_ISO9660:
	    cont=ISO1FileFind(p,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    Dbg;
	}
	cur_dir_cluster=de.cluster;
      }
    }
    if (!cont) {
      "%s ",dirname;
      PrintErr("File not found.\n");
      cur_dir_cluster=0;
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  Free(buf);
  Free(buf2);
  return cur_dir_cluster;
}

I64 Name2ParentDirCluster(CPrt *p,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_cluster,cur_dir_cluster2;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  PrtChk(p);
  try {
    unlock=PrtLock(p);
    cur_dir_cluster=cur_dir_cluster2=p->root_cluster;
    while (*buf && cont) {
      cur_dir_cluster2=cur_dir_cluster;
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (p->type) {
	  case PT_REDSEA:
	    cont=RedSeaFSFileFind(p,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case PT_FAT32:
	    cont=FAT32FileFind(p,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case PT_ISO9660:
	    cont=ISO1FileFind(p,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    Dbg;
	}
	cur_dir_cluster=de.cluster;
      }
    }
    if (!cont) {
      "%s ",dirname;
      PrintErr("File not found.\n");
      cur_dir_cluster2=0;
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  Free(buf);
  Free(buf2);
  return cur_dir_cluster2;
}


I64 WClustersBlks(CPrt *p,U8 *buf,I64 c,I64 blks)
{ //This accepts blk count, so padding on last cluster is not written.
  I64 i;
  Bool unlock;
  PrtChk(p);
  if (blks<=0) return c;
  try {
    unlock=PrtLock(p);
    switch (p->type) {
      case PT_REDSEA:
	WBlks(p,buf,p->data+c*p->spc,blks);
	c=0;
	break;
      case PT_FAT32:
	while (blks) {
	  if (!(0<c<0x0FFFFFF8))
	    Dbg;
	  i=blks;
	  if (i>p->spc)
	    i=p->spc;
	  WBlks(p,buf,p->data+c*p->spc,i);
	  buf+=i<<BLK_SIZE_BITS;
	  c=ClusterNumNext(p,c);
	  blks-=i;
	}
	break;
      default:
	Dbg;
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
  return c;
}

I64 WClusters(CPrt *p,U8 *buf,I64 c,I64 cnt)
{
  return WClustersBlks(p,buf,c,cnt*p->spc);
}

//c=0 means first cluster in chain
I64 ClustersAlloc(CPrt *p,I64 c=0,I64 cnt=1,Bool contiguous=FALSE)
{
  PrtChk(p);
  if (cnt<=0) return c;
  switch (p->type) {
    case PT_REDSEA:
      return RedSeaFSAllocClusters(p,cnt);
    case PT_FAT32:
      if (contiguous) {
	if (c) throw('File');
	return FAT32AllocContiguousClusters(p,cnt);
      } else
	return FAT32AllocClusters(p,c,cnt);
    default:
      Dbg;
  }
}
