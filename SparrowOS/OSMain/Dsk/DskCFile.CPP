U0 FAllocFile(CFile *f,I64 cnt,Bool contiguous)
{
  CPrt *fp=f->p;
  CFileAccess *fa=FileAccessNew(f->de.full_name);
  f->de.cluster=ClusersAlloc(fp,0,(cnt+fp->spc-1)/fp->spc,contiguous);
  f->de.size=cnt<<BLK_SIZE_BITS;
  f->de.expanded_size=f->de.size;
  if (fa) {
    CDirNew(fa->p,Fs->cur_dir,&f->de,TRUE);
    FileAccessDel(fa);
  }
  f->flags&=~FF_NEW_FILE;
}

I64 FSize(CFile *f)
{
  if (f)
    return f->de.size;
  else
    return 0;
}

U0 FSync(CFile *f)
{
  if (f->flags & FF_BUF_DIRTY) {
    WClusters(f->p,f->cluster_buf,f->cluster,1);
    f->flags&=~FF_BUF_DIRTY;
  }
}


CFile *FOpen(U8 *filename,U8 *flags,I64 cnt=0)
{  //allows flags "r","w","w+" and "c" (contiguous)
  CFile *f=CAlloc(sizeof(CFile));
  CFileAccess *fa;
  U8 *full_name,*full_name2;
  Bool contiguous=StrOcc(flags,'c');

  f->cluster=INVALID_CLUSTER;
  f->fblk_num=0;
  if (cnt>0)
    f->max_blk=cnt-1;
  else
    f->max_blk=MAX_I64;
  f->file_cluster_num=INVALID_CLUSTER;
  full_name2=FileNameAbs(filename);
  full_name=AStrNew(full_name2);
  Free(full_name2);
  f->p=Drv2Prt(*full_name);
  if (f->p->type==PT_NATIVE)
    contiguous=TRUE;
  if (contiguous) {
    f->flags|=FF_CONTIGUOUS;
    if (f->p->type!=PT_NATIVE) {
      if (!(FileAttr(filename) & _ATTR_CONTIGUOUS))
	throw(EXCEPT_FILE,2);
    }
  }
  f->cluster_buf=CAlloc(f->p->spc<<BLK_SIZE_BITS);
  if (StrOcc(flags,'w')) {
    f->flags=f->flags|FF_WRITE|FF_NEEDS_WRITE;
    if (StrOcc(flags,'+')) {
      if (FileFind(full_name,&f->de,FALSE,TRUE)) {
	f->de.full_name=full_name;
	if (contiguous)
	  f->max_blk=(FSize(f)+BLK_SIZE-1)>>BLK_SIZE_BITS-1;
	return f;
      }
    } else
      Del(full_name,FALSE);
    f->de.full_name=full_name;
    f->flags|=FF_NEW_FILE;
    fa=FileAccessNew(full_name);
    if (fa) {
      StrCpy(f->de.name,fa->mask);
      FileAccessDel(fa);
      if (cnt>0)
	FAllocFile(f,cnt,contiguous);
      if (!contiguous || cnt>0)
	return f;
    }
  } else {
    if (FileFind(full_name,&f->de,FALSE,TRUE)) {
      f->de.full_name=full_name;
      f->max_blk=(FSize(f)+BLK_SIZE-1)>>BLK_SIZE_BITS-1;
      return f;
    }
  }
  Free(f->cluster_buf);
  Free(full_name);
  Free(f);
  return NULL;
}

U0 FClose(CFile *f)
{
  CFileAccess *fa;
  if (f) {
    FSync(f);
    if (f->flags & FF_NEEDS_WRITE) {
      fa=FileAccessNew(f->de.full_name);
      if (fa) {
	if (!(f->flags & FF_USE_OLD_DATETIME))
	  f->de.datetime=GetCurTimeCDate;
	if (f->flags & FF_NEW_FILE)
	  CDirNew(fa->p,Fs->cur_dir,&f->de,TRUE);
	else
	  CDirNew(fa->p,Fs->cur_dir,&f->de,FALSE);
	FileAccessDel(fa);
      } else
	throw(EXCEPT_FILE,1);
    }
    Free(f->cluster_buf);
    Free(f->de.full_name);
    Free(f);
  }
}

I64 FSetCluster(CFile *f,I64 c,I64 blk,Bool read)
{
  CPrt *p=f->p;
  I64 i;
  if (f->cluster!=c) {
    if (f->flags & FF_BUF_DIRTY) {
      i=p->spc;
      if (f->max_blk!=MAX_I64) {
	i=f->max_blk+1-f->file_cluster_num*p->spc;
	if (i>p->spc)
	  i=p->spc;
      }
      WClustersBlks(p,f->cluster_buf,f->cluster,i);
      f->flags=f->flags & ~FF_BUF_DIRTY;
    }
    f->cluster=c;
    f->file_cluster_num=blk/p->spc;
    if (read) {
      i=p->spc;
      if (f->max_blk!=MAX_I64) {
	i=f->max_blk+1-f->file_cluster_num*p->spc;
	if (i>p->spc)
	  i=p->spc;
      }
      c=RClustersBlks(p,f->cluster_buf,c,i);
    }
  }
  return c;
}

Bool FRBlks(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1)
{
  CPrt *p=f->p;
  I64 spc=p->spc,i,j,c=f->de.cluster;
  if (!f || !p) return FALSE;
  if (blk==FFB_NEXT_BLK)
    blk=f->fblk_num;
  if (blk+cnt-1>f->max_blk)
    return FALSE;
  if (cnt<=0) return TRUE;

  if (f->flags & FF_CONTIGUOUS) {
    RBlks(p,buf,Cluster2Blk(p,c)+blk,cnt);
    blk+=cnt;
  } else {
    i=blk/spc;
    if (0<=f->file_cluster_num<=i) {
      c=f->cluster;
      i-=f->file_cluster_num;
    }
    if (i>0)
      c=ClusterNumNext(p,c,i);

    if (i=blk%spc) {
      c=FSetCluster(f,c,blk,TRUE);
      j= cnt<spc-i ? cnt:spc-i;
      MemCpy(buf,f->cluster_buf+i<<BLK_SIZE_BITS,j<<BLK_SIZE_BITS);
      buf+=j<<BLK_SIZE_BITS;
      cnt-=j;
      blk+=j;
    }
    while (cnt>=spc) {
      c=FSetCluster(f,c,blk,TRUE);
      MemCpy(buf,f->cluster_buf,spc<<BLK_SIZE_BITS);
      buf+=spc<<BLK_SIZE_BITS;
      cnt-=spc;
      blk+=spc;
    }
    if (cnt>0) {
      c=FSetCluster(f,c,blk,TRUE);
      MemCpy(buf,f->cluster_buf,cnt<<BLK_SIZE_BITS);
      buf+=cnt<<BLK_SIZE_BITS;
      blk+=cnt;
    }
  }
  f->fblk_num=blk;
  return TRUE;
}

Bool FWBlks(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1)
{
  CPrt *p=f->p;
  I64 spc=p->spc,i,j,c=f->de.cluster,c1;
  if (!f || !p) return FALSE;
  if (blk==FFB_NEXT_BLK)
    blk=f->fblk_num;
  if (blk+cnt-1>f->max_blk)
    return FALSE;
  if (!(f->flags & FF_WRITE))
    return FALSE;
  if (cnt<=0) return TRUE;
  if (f->flags & FF_CONTIGUOUS) {
    WBlks(p,buf,Cluster2Blk(p,c)+blk,cnt);
    blk+=cnt;
  } else {
    if (!c) {
      c=ClusersAlloc(p,0,1,FALSE);
      f->file_cluster_num=0;
      f->cluster=c;
      f->de.cluster=c;
      f->flags|=FF_NEEDS_WRITE|FF_NEW_FILE;
    }
    i=blk/spc;
    if (0<=f->file_cluster_num<=i) {
      c=f->cluster;
      i-=f->file_cluster_num;
    }
    while (i>0) {
      c1=c;
      c=ClusterNumNext(p,c1,1);
      if (c==INVALID_CLUSTER) {
	c=ClusersAlloc(p,c1,i,FALSE);
	if (i>1)
	  c=ClusterNumNext(p,c,i-1);
	break;
      } else
	i--;
    }

    if (i=blk%spc) {
      FSetCluster(f,c,blk,TRUE);
      j=cnt<spc-i ? cnt:spc-i;
      MemCpy(f->cluster_buf+BLK_SIZE*i,buf,j<<BLK_SIZE_BITS);
      f->flags|=FF_BUF_DIRTY;
      buf+=j<<BLK_SIZE_BITS;
      cnt-=j;
      blk+=j;
      if (cnt>0) {
	c1=c;
	c=ClusterNumNext(p,c1,1);
	if (c==INVALID_CLUSTER)
	  c=ClusersAlloc(p,c1,1,FALSE);
      }
    }
    while (cnt>=spc) {
      FSetCluster(f,c,blk,FALSE);
      MemCpy(f->cluster_buf,buf,spc<<BLK_SIZE_BITS);
      f->flags|=FF_BUF_DIRTY;
      buf+=spc<<BLK_SIZE_BITS;
      cnt-=spc;
      blk+=spc;
      if (cnt>0) {
	c1=c;
	c=ClusterNumNext(p,c1,1);
	if (c==INVALID_CLUSTER)
	  c=ClusersAlloc(p,c1,1,FALSE);
      }
    }
    if (cnt>0) {
      FSetCluster(f,c,blk,TRUE);
      MemCpy(f->cluster_buf,buf,cnt<<BLK_SIZE_BITS);
      f->flags|=FF_BUF_DIRTY;
      buf+=cnt<<BLK_SIZE_BITS;
      blk+=cnt;
    }
    if (f->de.size<blk<<BLK_SIZE_BITS)
      f->de.size=blk<<BLK_SIZE_BITS;
    if (f->de.expanded_size<blk<<BLK_SIZE_BITS)
      f->de.expanded_size=blk<<BLK_SIZE_BITS;
  }
  f->fblk_num=blk;
  return TRUE;
}
