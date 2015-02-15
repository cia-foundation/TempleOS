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

Bool BlkDevInit(CBlkDev *bd)
{
  CDrv *dv=Let2Drv(bd->first_drv_let);
  Bool old_preempt=Preempt(OFF),result=FALSE;

  if (!LBts(&bd->flags,BDf_INITIALIZED)) {
    bd->flags|=BDF_INIT_IN_PROGRESS;
    switch(bd->type) {
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
	if (bd->max_reads>dsk.cache_size/BLK_SIZE/4)
	  bd->max_reads=dsk.cache_size/BLK_SIZE/4 & ~3;
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
  Preempt(old_preempt);
  return result;
}

U0 BlkDevsRelease()
{//When task dies, release all owned BlkDevs.
  I64 i;
  CBlkDev *bd;
  for (i=0;i<NUM_BLKDEVS;i++) {
    bd=&dsk.blkdevs[i];
    if (bd->owning_task==Fs && bd->bd_signature==BD_SIGNATURE_VAL)
      BlkDevUnlock(bd,TRUE);
  }
}

U0 DrvsRelease()
{//When task dies, release all owned drvs.
  I64 i;
  CDrv *dv;
  for (i=0;i<NUM_DRVS;i++) {
    dv=&dsk.drvs[i];
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
  result=&dsk.drvs[i];
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
    if (dsk.drvs[i].dv_signature!=DRV_SIGNATURE_VAL)
      return DrvMakeFree(i+'A');
  while (++i<NUM_DRVS);
  throw('Drv');
  return NULL; //never gets here
}

CBlkDev *BlkDevNextFree(U8 first_drv_let,I64 type)
{//Locate free slot for new BlkDev, like during $LK,"Mount",A="MN:Mount"$().
  I64 i=0;
  CBlkDev *result;
  do {
    result=&dsk.blkdevs[i];
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

U0 DrvMapBlkAlloc(CDrv *dv)
{
  DrvChk(dv);
  Free(dv->cur_map_blk);
  dv->cur_map_blk=AMAlloc(BLK_SIZE);
  dv->cur_map_blk_num=0;
  dv->map_blk_dirty=0;
  RBlks(dv,dv->cur_map_blk,dv->map1,1);
}

U0 DrvMapBlkClean(CDrv *dv,I64 map_select=3)
{
  if (dv->type==DVT_FAT32 || dv->type==DVT_REDSEA) {
    if ((map_select==3 || !map_select) && dv->map1!=dv->map2 &&
	Bt(&dv->map_blk_dirty,0))
      WBlks(dv,dv->cur_map_blk(I64),dv->map1+dv->cur_map_blk_num,1);
    if (map_select==3 || map_select==1 || dv->map1==dv->map2 &&
	LBtr(&dv->map_blk_dirty,0))
      WBlks(dv,dv->cur_map_blk(I64),dv->map2+dv->cur_map_blk_num,1);
  }
}

U0 DrvMapBlkSet(CDrv *dv,I64 c,I64 map_select=3)
{
  I64 map_blk_num;
  if (c==INVALID_CLUSTER)
    throw('Drv');
  switch (dv->type) {
    case DVT_FAT32:
      map_blk_num=c>>(BLK_SIZE_BITS-2);
      break;
    case DVT_REDSEA:
      map_blk_num=(c-dv->data_area)>>(BLK_SIZE_BITS+3);
      break;
    default:
      throw('Drv');
  }
  if (map_blk_num!=dv->cur_map_blk_num) {
    DrvMapBlkClean(dv,map_select);
    dv->cur_map_blk_num=map_blk_num;
    if (map_select==3 || !map_select)
      RBlks(dv,dv->cur_map_blk,dv->map1+dv->cur_map_blk_num,1);
    else
      RBlks(dv,dv->cur_map_blk,dv->map2+dv->cur_map_blk_num,1);
  }
}

#define ZERO_BUF_SIZE	2048
U0 WZeroBlks(CDrv *dv,I64 blk,I64 cnt)
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
    WBlks(dv,z,blk,n);
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

Bool DrvTypeSet(U8 drv_let,I64 type=DVT_REDSEA)
{//Very dangerous
  I64 i,j,ext_base,drv_num,offset,cur_type;
  CMasterBoot mbr;
  CBlkDev *bd=Let2BlkDev(drv_let);
  drv_num=bd->first_drv_let-'A';
  switch (bd->type) {
    case BDT_ATA:
      offset=0;
      ext_base=INVALID_CLUSTER;
      while (TRUE) {
	ATAReadBlks(bd,&mbr,offset,1);
	j=-1;
	for (i=0;i<4;i++) {
	  cur_type=mbr.p[i].type;
	  if (cur_type) {
	    if (cur_type==5 || cur_type==15)
	      j=i;
	    else {
	      if (drv_num+'A'==ToUpper(drv_let)) {
		switch (type) {
		  case DVT_REDSEA:
		    mbr.p[i].type=MBR_PT_REDSEA;
		    break;
		  case DVT_FAT32:
		    mbr.p[i].type=MBR_PT_FAT32a;
		    break;
		  default:
		    throw('Drv');
		}
		mbr.p[i].active=0x80;
		ATAWriteBlks(bd,&mbr,offset,1);
		return TRUE;
	      }
	      drv_num++;
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

Bool RBlks(CDrv *dv,U8 *buf, I64 blk, I64 cnt)
{//Read blk cnt from Drv to buf.
  Bool result=TRUE,unlock;
  CBlkDev *bd=dv->bd;
  if (cnt<=0) return TRUE;
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    BlkDevInit(bd);
    if (dv->base_offset && blk<dv->base_offset || blk+cnt>dv->base_offset+dv->size)
      throw('Drv');
 
    if (bd->flags & BDF_READ_CACHE)
      RCache(dv,&buf,&blk,&cnt);
    if (cnt>0) {
      switch (bd->type) {
	case BDT_RAM:
	  MemCpy(buf,bd->RAM_dsk+blk<<BLK_SIZE_BITS,cnt<<BLK_SIZE_BITS);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  result=ATARBlks(dv,buf,blk,cnt);
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
  return result;
}

Bool WBlks(CDrv *dv,U8 *buf, I64 blk, I64 cnt)
{//Write blk cnt from buf to Drv.
  Bool result=TRUE,unlock;
  CBlkDev *bd=dv->bd;
  if (cnt<=0) return TRUE;
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    BlkDevInit(bd);
    if (bd->flags&BDF_READ_ONLY && !(bd->flags & BDF_READ_ONLY_OVERRIDE))
      throw('BlkDev');
    if (dv->base_offset && blk<dv->base_offset || blk+cnt>dv->base_offset+dv->size)
      throw('Drv');
    if (cnt>0) {
      switch (bd->type) {
	case BDT_RAM:
	  MemCpy(bd->RAM_dsk+blk<<BLK_SIZE_BITS,buf,cnt<<BLK_SIZE_BITS);
	  break;
	case BDT_ATA:
	case BDT_ATAPI:
	  result=ATAWBlks(dv,buf,blk,cnt);
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
  return result;
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
  Bool old_preempt=Preempt(OFF);
  if (!(bd->flags&BDF_INITIALIZED))
    BlkDevInit(bd);
  else if (bd->flags&BDF_REMOVABLE)
    DskCacheInvalidate(dv);
  Drv(drv_let);
  RedSeaFreeFreeLst(dv);
  Preempt(old_preempt);
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
  for (i=0,dv=dsk.drvs;i<NUM_DRVS;i++,dv++) {
    if (dv->dv_signature==DRV_SIGNATURE_VAL) {
      bd=dv->bd;
      drv_let=Drv2Let(dv);
      if (Bt(&dv->type,DVTf_DISABLE))
	"$$RED$$X";
      else if (drv_let==dsk.boot_drv_let)
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

I64 ClusterNumNext(CDrv *dv,I64 c,I64 cnt=1)
{//Return next cnt'th cluster in chain.
  Bool unlock;
  DrvChk(dv);
  if (cnt<=0) return c;
  try {
    unlock=DrvLock(dv);
    switch (dv->type) {
      case DVT_ISO9660:
      case DVT_REDSEA:
	c+=cnt;
	break;
      case DVT_FAT32:
	while (cnt-->0 && 0<c<0x0FFFFFF8) {
	  DrvMapBlkSet(dv,c);
	  c=dv->cur_map_blk[c & (BLK_SIZE/4-1)];
	}
	if (!(0<c<0x0FFFFFF8))
	  c=INVALID_CLUSTER;
	break;
      default:
	throw('Drv');
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  return c;
}

I64 Cluster2Blk(CDrv *dv,I64 c)
{//Drv cluster num to blk num.
  DrvChk(dv);
  switch (dv->type) {
    case DVT_REDSEA:
      return c;
    case DVT_ISO9660:
    case DVT_FAT32:
      return dv->data_area+c*dv->spc;
    default:
      throw('Drv');
  }
}

I64 RClustersBlks(CDrv *dv,U8 *buf,I64 c,I64 blks)
{//Accepts blk count, so padding on last cluster is not read.
  I64 i;
  Bool unlock;
  DrvChk(dv);
  if (blks<=0) return c;
  try {
    unlock=DrvLock(dv);
    switch (dv->type) {
      case DVT_ISO9660:
	RBlks(dv,buf,dv->data_area+c*dv->spc,blks);
	c+=(blks+dv->spc-1)/dv->spc;
	break;
      case DVT_REDSEA:
	RBlks(dv,buf,c,blks);
	c+=blks;
	break;
      case DVT_FAT32:
	while (blks && 0<c<0x0FFFFFF8) {
	  i=blks;
	  if (i>dv->spc)
	    i=dv->spc;
	  RBlks(dv,buf,dv->data_area+c*dv->spc,i);
	  buf+=i<<BLK_SIZE_BITS;
	  c=ClusterNumNext(dv,c,1);
	  blks-=i;
	}
	if (blks)
	  throw('Drv');
	break;
      default:
	throw('Drv');
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  return c;
}

I64 RClusters(CDrv *dv,U8 *buf,I64 c,I64 cnt)
{//Read cluster cnt from drv to buf.
  return RClustersBlks(dv,buf,c,cnt*dv->spc);
}

Bool CFileNameTo(U8 *dst,U8 *src)
{
  MemSet(dst,0,CDIR_FILENAME_LEN);
  if (!FileNameChk(src))
    return FALSE;
  StrCpy(dst,src);
  return TRUE;
}

I64 Name2DirCluster(CDrv *dv,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_cluster;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    cur_dir_cluster=dv->root_cluster;
    while (*buf && cont) {
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (dv->type) {
	  case DVT_REDSEA:
	    cont=RedSeaFileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_FAT32:
	    cont=FAT32FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_ISO9660:
	    cont=ISO1FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    throw('Drv');
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
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  Free(buf);
  Free(buf2);
  return cur_dir_cluster;
}

I64 Name2ParentDirCluster(CDrv *dv,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_cluster,cur_dir_cluster2;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    cur_dir_cluster=cur_dir_cluster2=dv->root_cluster;
    while (*buf && cont) {
      cur_dir_cluster2=cur_dir_cluster;
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (dv->type) {
	  case DVT_REDSEA:
	    cont=RedSeaFileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_FAT32:
	    cont=FAT32FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_ISO9660:
	    cont=ISO1FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    throw('Drv');
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
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  Free(buf);
  Free(buf2);
  return cur_dir_cluster2;
}

I64 WClustersBlks(CDrv *dv,U8 *buf,I64 c,I64 blks)
{//Accepts blk count, so padding on last cluster is not written.
  I64 i;
  Bool unlock;
  DrvChk(dv);
  if (blks<=0) return c;
  try {
    unlock=DrvLock(dv);
    switch (dv->type) {
      case DVT_REDSEA:
	WBlks(dv,buf,c,blks);
	c=0;
	break;
      case DVT_FAT32:
	while (blks) {
	  if (!(0<c<0x0FFFFFF8))
	    throw('Drv');
	  i=blks;
	  if (i>dv->spc)
	    i=dv->spc;
	  WBlks(dv,buf,dv->data_area+c*dv->spc,i);
	  buf+=i<<BLK_SIZE_BITS;
	  c=ClusterNumNext(dv,c);
	  blks-=i;
	}
	break;
      default:
	throw('Drv');
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  return c;
}

I64 WClusters(CDrv *dv,U8 *buf,I64 c,I64 cnt)
{//Write cluster cnt from buf to drv.
  return WClustersBlks(dv,buf,c,cnt*dv->spc);
}

I64 ClustersAlloc(CDrv *dv,I64 c=0,I64 cnt=1,Bool contiguous=FALSE)
{//Alloc cluster cnt into chain.
//c=0 means first cluster in chain
  DrvChk(dv);
  if (cnt<=0) return c;
  switch (dv->type) {
    case DVT_REDSEA:
      return RedSeaAllocClusters(dv,cnt);
    case DVT_FAT32:
      if (contiguous) {
	if (c) throw('File');
	return FAT32AllocContiguousClusters(dv,cnt);
      } else
	return FAT32AllocClusters(dv,c,cnt);
    default:
      throw('Drv');
  }
}
