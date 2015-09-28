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
	  DrvFATBlkSet(dv,c);
	  c=dv->cur_fat_blk[c & (BLK_SIZE/4-1)];
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
