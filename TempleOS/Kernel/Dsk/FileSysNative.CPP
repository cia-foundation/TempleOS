U0 NativeFSFreeFreeLst(CPrt *p)
{
  CFreeLst *tempf,*tempf1;
  Bool unlock;
  try {
    unlock=PrtLock(p);
 
    if (tempf=p->next_free) {
      while (tempf!=&p->next_free) {
	tempf1=tempf->next;
	Free(tempf);
	tempf=tempf1;
      }
    }
    p->next_free=NULL;
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
}

U0 NativeFSFreeLstBuild(CPrt *p)
{
  Bool unlock;
  CFreeLst *tempf;
  I64 i,first=0,
      max_cluster=(p->size+p->offset-p->data)/p->spc;
 
  try {
    unlock=PrtLock(p);
    if (p->next_free)
      NativeFSFreeFreeLst(p);
    p->next_free=&p->next_free;
    p->last_free=&p->next_free;
    while (first<max_cluster) {
      i=0;  //count free clusters
      while (first+i<max_cluster) {
	SetNativeBuf(p,first+i);
	if (Bt(p->cur_map_blk,(first+i)&(BLK_SIZE<<3-1)))
	  break;
	else
	  i++;
      }
      if (i) {
	tempf=AMAlloc(sizeof(CFreeLst));
	tempf->size=i;
	tempf->start=first;
	QueIns(tempf,p->last_free);
      }
      first+=i+1;
    }
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
}

U0 NativeFSPrtInit(CPrt *p)
{
  CBootStruct br;
  Bool unlock;
  try {
    unlock=PrtLock(p);
    RBlks(p,&br,p->offset,1);
    if (br.signature!=MBR_PT_NATIVE ||
    br.signature2!=0xAA55)
      throw(EXCEPT_PRT,7);
    p->type=PT_NATIVE;
    NativeFSFreeFreeLst(p);
    p->spc=br.sects_per_cluster;
    p->size=br.sects;
    p->data=p->offset+1+br.bitmap_sects-p->spc;
    p->root_cluster=br.root_cluster;
    p->map1=p->offset+1;
    p->map2=p->offset+1;
    MapBufAlloc(p,1);
    if (unlock)
      PrtUnlock(p);
  } catch
    if (unlock)
      PrtUnlock(p);
}

Bool NativeFSDrvValidate(U8 drv_let)
{
  CPrt *p;
  CBootStruct br;
  Bool result=FALSE;
  try {
    p=Drv2Prt(drv_let);
    if (p->type==PT_NATIVE &&
	RBlks(p,&br,p->offset,1) &&
	br.signature==MBR_PT_NATIVE &&
	br.signature2==0xAA55 &&
	br.sects==p->size)
      result=TRUE;
  } catch
    Fs->catch_except=TRUE;
  return result;
}

U0 NativeFSFmt(U8 drv_let,Bool quick=TRUE)
{
  U8 *root_dir;
  CDirEntry *d_native;
  CBootStruct *br=CAlloc(BLK_SIZE);
  CPrt *p=Drv2Prt(drv_let);
  I64 i,n;
  try {
    PrtLock(p);
    //	SetDrvType(drv_let,PT_NATIVE);
    SetDrvType(drv_let,PT_FAT32);
    p->type=PT_NATIVE;
    br->U8s_per_sect=BLK_SIZE;
    if	  (p->size<0x20000)
      br->sects_per_cluster=1;
    else if (p->size<0x200000)
      br->sects_per_cluster=2;
    else if (p->size<0x2000000)
      br->sects_per_cluster=4;
    else if (p->size<0x20000000)
      br->sects_per_cluster=8;
    else if (p->size<0x200000000)
      br->sects_per_cluster=16;
    else
      br->sects_per_cluster=32;
 
    br->signature=MBR_PT_NATIVE;
    br->signature2=0xAA55;
    br->sects=p->size;
    n=br->sects_per_cluster*(BLK_SIZE<<3);
    n=(br->sects+n-1)/n;
    br->bitmap_sects=n;
    br->unique_id=GetTimeStamp>>4;
    br->root_cluster=0;
 
    if (quick)
      i=n+1;
    else
      i=p->size;
    WZeroBlks(p,p->offset,i);
 
    WBlks(p,br,p->offset,1);
    NativeFSPrtInit(p);
    ClusersAlloc(p,0,1,FALSE);	//Alloc #1
    br->root_cluster=ClusersAlloc(p,0,1,FALSE);
    WBlks(p,br,p->offset,1);
    root_dir=CAlloc(p->spc<<BLK_SIZE_BITS);
 
    d_native=root_dir-offset(CDirEntry.start);
 
    d_native->attr=_ATTR_DIR|_ATTR_CONTIGUOUS;
    *d_native->name='.';
    d_native->cluster=br->root_cluster;
    d_native->size=p->spc<<BLK_SIZE_BITS;
    d_native->expanded_size=d_native->size;
    d_native->datetime=GetCurTimeCDate;
 
    d_native(U8 *)+=CDIR_SIZE;
 
    *d_native->name='.';
    d_native->name[1]='.';
    d_native->attr=_ATTR_DIR|_ATTR_CONTIGUOUS;
    d_native->cluster=br->root_cluster;
    d_native->datetime=GetCurTimeCDate;
 
    WBlks(p,root_dir,p->data+br->root_cluster*p->spc,p->spc);
    NativeFSPrtInit(p);
    PrtUnlock(p);
  } catch
    PrtUnlock(p);
  Free(br);
  Free(root_dir);
}

Bool NativeFSFileFind(CPrt *p,I64 cur_dir_cluster,U8 *name,CDirEntry *_result,I64 fuf_flags=0)
{ //$LK,"FUF_JUST_DIRS","MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES","MN:FUF_JUST_FILES"$
  CDirEntry *buf,*buf2,*ptr;
  U8 dname[CDIR_FILENAME_LEN];
  I64 ch;
  Bool result=FALSE,unlock;
  if (fuf_flags&~FUG_FILE_FIND)
    throw(EXCEPT_FUF,10);
  MemSet(_result,0,sizeof(CDirEntry));
  PrtChk(p);
  if (p->type!=PT_NATIVE)
    PutDefineErr("ST_NOT_NATIVE");
  else if (!CFileNameTo(dname,name))
    PutDefineErr("ST_INVALID_FILENAME");
  else {
    try {
      unlock=PrtLock(p);
      buf2=MAlloc(p->spc<<BLK_SIZE_BITS);
      RClusters(p,buf2,cur_dir_cluster,1);
 
      ptr=buf2(U8 *)-offset(CDirEntry.start);
      buf=MAlloc(ptr->size);
      RClustersBlks(p,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
      Free(buf2);
 
      ptr=buf(U8 *)-offset(CDirEntry.start);
      *ptr->name='.';
      ptr->name[1]=0;
      while (TRUE) {
	if (!(ch=*ptr->name))
	  break;
	else if (!(ptr->attr & _ATTR_DELETED) &&
	!(fuf_flags&FUF_JUST_DIRS && !(ptr->attr & _ATTR_DIR)) &&
	!(fuf_flags&FUF_JUST_FILES && ptr->attr & _ATTR_DIR) &&
	!StrCmp(dname,ptr->name)) {
	  MemCpy(&_result->attr,&ptr->attr,CDIR_SIZE);
	  result=TRUE;
		goto exit_this;
	}
	ptr(U8 *)+=CDIR_SIZE;
      }
exit_this:
      Free(buf);
      if (unlock)
	PrtUnlock(p);
    } catch
      if (unlock)
	PrtUnlock(p);
  }
  return result;
}



U8 *NativeFSFileRead(CPrt *p,U8 *cur_dir,U8 *filename,I64 *_size,
  I64 *_attr,Bool raw,CArcCompress **_ac2)
{
  U8 *buf=NULL;
  CDirEntry de;
  I64 c,blk_cnt,cur_dir_cluster;
  CArcCompress *ac;
 
  PrtChk(p);
  if (_size) *_size=0;
  if (_ac2)  *_ac2=NULL;
  if (_attr) *_attr=0;
  if (p->type!=PT_NATIVE)
    PutDefineErr("ST_NOT_NATIVE");
  else {
    try {
      PrtLock(p);
      cur_dir_cluster=Name2DirCluster(p,cur_dir);
      if (NativeFSFileFind(p,cur_dir_cluster,filename,&de,FUF_JUST_FILES)) {
	if (_attr) *_attr=de.attr;
	blk_cnt=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	buf=MAlloc(blk_cnt<<BLK_SIZE_BITS+1);
	c=de.cluster;
	c=RClustersBlks(p,buf,c,blk_cnt);
	if (de.xsum) {
	  if (de.xsum^ArcXSum(buf,de.size) !=NATIVE_XSUM)
	    throw(EXCEPT_ARC_XSUM);
	}
	if (FileAttr(de.name) & _ATTR_COMPRESSED && !raw) {
	  ac=buf;
	  if (_attr) *_attr=FileAttr(de.name,*_attr);
	  if (_size) *_size=ac->expanded_size;
	  if (_ac2) {
	    *_ac2=ac;
	    buf=ExpandBuf(ac);
	  } else {
	    buf=ExpandBuf(ac);
	    Free(ac);
	  }
	} else {
	  buf[de.size]=0; //Terminate
	  if (_size) *_size=de.size;
	}
	if (_attr) *_attr=FileAttr(de.name,*_attr);
      }
      PrtUnlock(p);
    } catch
      PrtUnlock(p);
  }
  return buf;
}

Bool NativeFSCd(U8 *name,I64 cur_dir_cluster)
{
  CDirEntry de;
  if (Fs->cur_prt->type!=PT_NATIVE)
    PutDefineErr("ST_NOT_NATIVE");
  else {
    if (NativeFSFileFind(Fs->cur_prt,cur_dir_cluster,
			name,&de,FUF_JUST_DIRS))
      return TRUE;
    else {
      "%s ",name;
      PutDefineErr("ST_FILE_NOT_FOUND");
    }
  }
  return FALSE;
}

U0 NativeFSFreeClusters(CPrt *p,I64 c,I64 cnt)
{
  CFreeLst *tempf;
  Bool found=FALSE,unlock,unlock_break;
  PrtChk(p);
  if (!c) return;
  if (p->type!=PT_NATIVE)
    PutDefineErr("ST_NOT_NATIVE");
  else {
    try {
      unlock_break=BreakLock;
      unlock=PrtLock(p);
      if (!p->next_free)
	NativeFSFreeLstBuild(p);
      tempf=p->next_free;
      while (!found && tempf!=&p->next_free) {
	if (tempf->start+tempf->size==c) {
	  tempf->size+=cnt;
	  found=TRUE;
	} else if (c+cnt==tempf->start) {
	  tempf->size+=cnt;
	  tempf->start=c;
	  found=TRUE;
	}
	tempf=tempf->next;
      }
      if (!found) {
	tempf=AMAlloc(sizeof(CFreeLst));
	tempf->size=cnt;
	tempf->start=c;
	QueIns(tempf,p->last_free);
      }
      while (cnt-->0) {
	SetNativeBuf(p,c);
	LBtr(p->cur_map_blk,c&(BLK_SIZE<<3-1));
	LBts(p->buffered_map_dirty_bits,0);
	c++;
      }
      CleanMAPBuf(p);
 
      if (unlock)
	PrtUnlock(p);
      if (unlock_break)
	BreakUnlock;
    } catch {
      if (unlock)
	PrtUnlock(p);
      if (unlock_break)
	BreakUnlock;
    }
  }
}

I64 NativeFSAllocClusters(CPrt *p,I64 cnt)
{
  CFreeLst *tempf,*best_free=NULL;
  I64 i,first,best_size=MAX_I64;
  Bool unlock,unlock_break;
  if (cnt<=0)
    Dbg;
  try {
    unlock_break=BreakLock;
    unlock=PrtLock(p);
    if (!p->next_free)
      NativeFSFreeLstBuild(p);
    tempf=p->next_free;
    while (tempf!=&p->next_free) {
      if (tempf->size>=cnt && tempf->size<best_size) {
	best_free=tempf;
	best_size=tempf->size;
	if (tempf->size==cnt)
	  break;
      }
      tempf=tempf->next;
    }
    if (!best_free)
      Dbg;
    first=best_free->start;
    for (i=0;i<cnt;i++) {
      SetNativeBuf(p,first+i);
      LBts(p->cur_map_blk,(first+i)&(BLK_SIZE<<3-1));
      LBts(p->buffered_map_dirty_bits,0);
    }
    CleanMAPBuf(p);
    if (best_free->size-=cnt)
      best_free->start+=cnt;
    else {
      QueRem(best_free);
      Free(best_free);
    }
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  } catch {
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  return first;
}


Bool NativeFSDirNew(CPrt *p,U8 *cur_dir,CDirEntry *de,Bool free_old_chain)
{
  CDirEntry *buf,*buf2,*ptr,de2;
  CBootStruct *br;
  I64 c,ch,i=1,j=0,n=BLK_SIZE/CDIR_SIZE,dir_size,cur_dir_cluster;
  Bool written=FALSE,unlock,unlock_break;
  U8 *temp,*parent_dir;
  try {
    unlock_break=BreakLock;
    de->attr|=_ATTR_CONTIGUOUS;
    unlock=PrtLock(p);
    cur_dir_cluster=Name2DirCluster(p,cur_dir);
    buf2=MAlloc(p->spc<<BLK_SIZE_BITS);
    RClusters(p,buf2,cur_dir_cluster,1);
 
    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    RClustersBlks(p,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
 
    dir_size=ptr->size;
    ptr=buf(U8 *)-offset(CDirEntry.start)+CDIR_SIZE;
    Free(buf2);
    while (TRUE) {
      if (!(ch=*ptr->name)) {
	if (!written)
	  MemCpy(&ptr->start,&de->start,CDIR_SIZE);
	if ((i+1)*CDIR_SIZE+j<<BLK_SIZE_BITS<dir_size)
	  WBlks(p,buf(U8 *)+j<<BLK_SIZE_BITS,
	  p->data+cur_dir_cluster*p->spc+j,1);
	else {
	  buf2=CAlloc(dir_size+p->spc<<BLK_SIZE_BITS);
	  MemCpy(buf2,buf,dir_size);
	  NativeFSFreeClusters(p,cur_dir_cluster,
	    (dir_size>>BLK_SIZE_BITS+p->spc-1)/p->spc);
	  dir_size+=p->spc<<BLK_SIZE_BITS;
	  c=ClusersAlloc(p,0,(dir_size>>BLK_SIZE_BITS+p->spc-1)/p->spc,TRUE);
	  Free(buf);
	  buf=buf2;
	  ptr=buf(U8 *)-offset(CDirEntry.start);
	  ptr->size=dir_size;
	  ptr->expanded_size=dir_size;
	  ptr->cluster=c;
	  WClustersBlks(p,buf,c,dir_size>>BLK_SIZE_BITS);
	  if (cur_dir_cluster==p->root_cluster) {
	    br=CAlloc(BLK_SIZE);
	    RBlks(p,br,p->offset,1);
	    br->root_cluster=c;
	    WBlks(p,br,p->offset,1);
	    Free(br);
	    p->root_cluster=c;
	  } else {
	    temp=StrNew(cur_dir);
	    parent_dir=StrNew(cur_dir);
	    StrLastRem(parent_dir,"/",temp);
	    if (!*parent_dir) {
	      Free(parent_dir);
	      parent_dir=StrNew("/");
	    }
	    if (NativeFSFileFind(p,Name2DirCluster(p,parent_dir),
		temp,&de2,FUF_JUST_DIRS)) {
	      de2.cluster=c;
	      de2.size=dir_size;
	      de2.expanded_size=dir_size;
	      NativeFSDirNew(p,parent_dir,&de2,FALSE);
	    } else
	      Dbg;
	    Free(temp);
	    Free(parent_dir);
	  }
	}
	break;
      } else if (ptr->attr & _ATTR_DELETED) {
	if (!written) {
	  MemCpy(&ptr->start,&de->start,CDIR_SIZE);
	  WBlks(p,buf(U8 *)+j<<BLK_SIZE_BITS,
	  p->data+cur_dir_cluster*p->spc+j,1);
	  written=TRUE;
	}
      } else {
	if (!StrCmp(de->name,ptr->name)) {
	  if (free_old_chain)
	    NativeFSFreeClusters(p,ptr->cluster,
	    (ptr->size+p->spc<<BLK_SIZE_BITS-1)/p->spc<<BLK_SIZE_BITS);
	  if (!written)
	    MemCpy(&ptr->start,&de->start,CDIR_SIZE);
	  else
	    ptr->attr|=_ATTR_DELETED;
	    WBlks(p,buf(U8 *)+j<<BLK_SIZE_BITS,
	  p->data+cur_dir_cluster*p->spc+j,1);
	  break;
	}
      }
      ptr(U8 *)+=CDIR_SIZE;
      if (++i>=n) {
	j++;
	i=0;
      }
    }
    Free(buf);
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  } catch {
    if (unlock)
      PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  return FALSE;
}

I64 NativeFSFilesDel(CPrt *p,U8 *cur_dir,U8 *files_find_mask,I64 fuf_flags,
		     Bool del_dir,Bool print_msg)
{
  CDirEntry *buf,*buf2,*ptr;
  I64 i=0,result=0,ch,j=0,n=BLK_SIZE/CDIR_SIZE,cur_dir_cluster;
  Bool unlock_break;
  try {
    unlock_break=BreakLock;
    PrtLock(p);
    cur_dir_cluster=Name2DirCluster(p,cur_dir);
    buf2=MAlloc(p->spc<<BLK_SIZE_BITS);
    RClusters(p,buf2,cur_dir_cluster,1);
 
    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    RClustersBlks(p,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
    Free(buf2);
 
    ptr=buf(U8 *)-offset(CDirEntry.start);
    *ptr->name='.';
    ptr->name[1]=0;
    while (TRUE) {
      if (!(ch=*ptr->name))
	break;
      else if (!(ptr->attr & _ATTR_DELETED) && ch!='.' && (del_dir ||
	  !(ptr->attr & _ATTR_DIR)) &&
	  FilesFindMatch(files_find_mask,ptr->name,fuf_flags)) {
	if (!(ptr->attr & _ATTR_DIR)) result++;
	if (print_msg) {
	  PutDefine("ST_DELETE");
	  "" ptr->name;
	  NewLine;
	}
	ptr->attr|=_ATTR_DELETED;
	WBlks(p,buf(U8 *)+j<<BLK_SIZE_BITS,
	p->data+cur_dir_cluster*p->spc+j,1);
	NativeFSFreeClusters(p,ptr->cluster,
	(ptr->size+p->spc<<BLK_SIZE_BITS-1)/p->spc<<BLK_SIZE_BITS);
      }
      ptr(U8 *)+=CDIR_SIZE;
      if (++i>=n) {
	j++;
	i=0;
      }
    }
    Free(buf);
    PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  } catch {
    PrtUnlock(p);
    if (unlock_break)
      BreakUnlock;
  }
  return result;
}

I64 NativeFSFileWrite(CPrt *p,U8 *cur_dir,U8 *name,U8 *buf,I64 size,
		CDate spdt,I64 attr,Bool raw,
		CArcCompress **_ac2)
{
  CDirEntry de;
  I64 c=0,flags=0,blk_cnt;
  CArcCompress *ac=NULL;
  Bool contiguous;
  MemSet(&de,0,sizeof(CDirEntry));
  if (_ac2) *_ac2=NULL;
  if (size<0) size=0;
  if (!spdt)
    spdt=GetCurTimeCDate;

  if (p->type!=PT_NATIVE)
    PutDefineErr("ST_NOT_NATIVE");
  else if (!CFileNameTo(de.name,name))
    PutDefineErr("ST_INVALID_FILENAME");
  else {
    NativeFSFilesDel(p,cur_dir,de.name,0,FALSE,FALSE);
    attr=FileAttr(name,attr);
    attr|=_ATTR_CONTIGUOUS;
    contiguous=TRUE;
    de.expanded_size=size;
    if (attr&_ATTR_COMPRESSED && !raw) {
      ac=CompressBuf(buf,size,flags);
      size=ac->compressed_size;
      buf=ac;
    }
    de.size=size;
    de.xsum=ArcXSum(buf,size)^NATIVE_XSUM;
    if (blk_cnt=(size+BLK_SIZE-1)>>BLK_SIZE_BITS)
      c=ClusersAlloc(p,0,(blk_cnt+p->spc-1)/p->spc,contiguous);
    else
      c=INVALID_CLUSTER;
    de.cluster=c;
    de.attr=attr;
    de.datetime=spdt;
    if (blk_cnt)
      WClustersBlks(p,buf,c,blk_cnt);
    NativeFSDirNew(p,cur_dir,&de,TRUE);
    if (_ac2)
      *_ac2=ac;
    else
      Free(ac);
  }
  return c;
}

CDirEntry *NativeFSFilesFind(U8 *files_find_mask,I64 fuf_flags,CDirEntry *parent=NULL)
{
  CPrt *p=Fs->cur_prt;
  CDirEntry *buf,*buf2,*ptr,*result=NULL,*tempm;
  I64 ch,cur_dir_cluster;
  if (fuf_flags&~FUG_FILES_FIND)
    throw(EXCEPT_FUF,11);
  try {
    PrtLock(p);
    cur_dir_cluster=Name2DirCluster(p,Fs->cur_dir);
    buf2=MAlloc(p->spc<<BLK_SIZE_BITS);
    RClusters(p,buf2,cur_dir_cluster,1);
 
    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    RClustersBlks(p,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
    Free(buf2);
 
    ptr=buf(U8 *)-offset(CDirEntry.start);
    *ptr->name='.';
    ptr->name[1]=0;
    ptr(U8 *)+=CDIR_SIZE;
    ptr->cluster=Name2ParentDirCluster(p,Fs->cur_dir);
    ptr(U8 *)-=CDIR_SIZE;
    while (TRUE) {
      if (!(ch=*ptr->name))
	break;
      else if (!(ptr->attr & _ATTR_DELETED)) {
	tempm=CAlloc(sizeof(CDirEntry));
	MemCpy(&tempm->start,&ptr->start,CDIR_SIZE);
	tempm->parent=parent;
	if (Bt(&fuf_flags,FUf_RECURSE) && tempm->attr&_ATTR_DIR && *tempm->name!='.') {
	  tempm->next=result;
	  result=tempm;
	  tempm->full_name=DirNameAbs(tempm->name);
	  PrtUnlock(p);
	  if (Cd(tempm->name)) {
	    tempm->sub=NativeFSFilesFind(files_find_mask,fuf_flags,tempm);
	    Cd("..");
	  }
	  PrtLock(p);
	} else if ((tempm->attr&_ATTR_DIR || !Bt(&fuf_flags,FUf_JUST_DIRS)) &&
	!(Bt(&fuf_flags,FUf_RECURSE) && *tempm->name=='.' && tempm->attr&_ATTR_DIR) &&
	  FilesFindMatch(files_find_mask,tempm->name,fuf_flags)) {
	  tempm->next=result;
	  result=tempm;
	  tempm->full_name=FileNameAbs(tempm->name);
	} else
	  CDirEntryDel(tempm);
      }
      ptr(U8 *)+=CDIR_SIZE;
    }
    Free(buf);
    PrtUnlock(p);
  } catch
    PrtUnlock(p);
  return result;
}

Bool NativeFSMkDir(CPrt *p,U8 *cur_dir,U8 *name)
{
  I64 size=p->spc<<BLK_SIZE_BITS,c,
      cur_dir_cluster=Name2DirCluster(p,cur_dir);
  U8 *buf=CAlloc(size);
  CDirEntry *d_native=buf-offset(CDirEntry.start);
  Bool unlock_break;

  try {
    unlock_break=BreakLock;
    c=FileWrite(name,buf,size,0,_ATTR_DIR,FALSE);
    d_native->attr=_ATTR_DIR|_ATTR_CONTIGUOUS;
    StrCpy(d_native->name,name);
    d_native->cluster=c;
    d_native->size=size;
    d_native->expanded_size=size;
    d_native->datetime=GetCurTimeCDate;
    d_native(U8 *)+=CDIR_SIZE;
 
    d_native->attr=_ATTR_DIR|_ATTR_CONTIGUOUS;
    *d_native->name='.';
    d_native->name[1]='.';
    d_native->name[2]=0;
    d_native->cluster=cur_dir_cluster;
    d_native->size=0;
    d_native->expanded_size=0;
    d_native->datetime=GetCurTimeCDate;
    WClusters(p,buf,c,1);
    Free(buf);
    if (unlock_break)
      BreakUnlock;
  } catch
    if (unlock_break)
      BreakUnlock;
  return TRUE;
}

