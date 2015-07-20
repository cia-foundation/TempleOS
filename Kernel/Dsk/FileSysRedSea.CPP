// See $LK,"RedSea File System",A="FI:::/Doc/RedSea.TXT"$

U0 RedSeaFreeFreeLst(CDrv *dv)
{
  CFreeLst *tempf,*tempf1;
  Bool unlock;
  try {
    unlock=DrvLock(dv);
    if (tempf=dv->next_free) {
      while (tempf!=&dv->next_free) {
	tempf1=tempf->next;
	Free(tempf);
	tempf=tempf1;
      }
    }
    dv->next_free=NULL;
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
}

U0 RedSeaFreeLstBuild(CDrv *dv)
{
  Bool unlock;
  CFreeLst *tempf;
  I64 i,first=dv->data_area,max_blk=dv->size+dv->base_offset;
  try {
    unlock=DrvLock(dv);
    if (dv->next_free)
      RedSeaFreeFreeLst(dv);
    QueInit(&dv->next_free);
    while (first<max_blk) {
      i=0;  //count free clusters
      while (first+i<max_blk) {
	DrvMapBlkSet(dv,first+i);
	if (Bt(dv->cur_map_blk,(first+i-dv->data_area)&(BLK_SIZE<<3-1)))
	  break;
	else
	  i++;
      }
      if (i) {
	tempf=AMAlloc(sizeof(CFreeLst));
	tempf->size=i;
	tempf->start=first;
	QueIns(tempf,dv->last_free);
      }
      first+=i+1;
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
}

U0 RedSeaDrvInit(CDrv *dv)
{
  CRedSeaBoot br;
  Bool unlock;
  try {
    unlock=DrvLock(dv);
    RBlks(dv,&br,dv->base_offset,1);
    if (br.signature!=MBR_PT_REDSEA || br.signature2!=0xAA55)
      throw('Drv');
    dv->type=DVT_REDSEA;
    RedSeaFreeFreeLst(dv);
    dv->spc=1;
    dv->size=br.sects;
    dv->data_area=dv->base_offset+br.bitmap_sects;
    dv->root_cluster=br.root_cluster;
    dv->map1=dv->map2=dv->base_offset+1;
    DrvMapBlkAlloc(dv);
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
}

Bool RedSeaDrvValidate(U8 drv_let)
{
  CDrv *dv;
  CRedSeaBoot br;
  Bool result=FALSE;
  try {
    dv=Let2Drv(drv_let);
    if (dv->type==DVT_REDSEA && RBlks(dv,&br,dv->base_offset,1) &&
	  br.signature==MBR_PT_REDSEA && br.signature2==0xAA55 &&
	  br.sects==dv->size)
      result=TRUE;
  } catch
    Fs->catch_except=TRUE;
  return result;
}

U0 RedSeaFmt(U8 drv_let,Bool quick=TRUE)
{
  U8 *root_dir;
  CDirEntry *d_native;
  CRedSeaBoot *br=CAlloc(BLK_SIZE);
  CDrv *dv=Let2Drv(drv_let);
  I64 i,n;
  try {
    DrvLock(dv);
//	DrvTypeSet(drv_let,DVT_REDSEA);
    DrvTypeSet(drv_let,DVT_FAT32);
    dv->type=DVT_REDSEA;
    br->signature=MBR_PT_REDSEA;
    br->signature2=0xAA55;
    br->base_offset=dv->base_offset; //For CD/DVD image copy.
    br->sects=dv->size;
    n=(br->sects+BLK_SIZE<<3-1)/BLK_SIZE<<3;
    br->bitmap_sects=n;
    br->unique_id=GetTSC^Now()(U64);
    br->root_cluster=0;

    if (quick)
      i=n+1;
    else
      i=dv->size;
    WZeroBlks(dv,dv->base_offset,i);

    WBlks(dv,br,dv->base_offset,1);
    RedSeaDrvInit(dv);
    ClustersAlloc(dv,0,1,FALSE);	//Alloc #1
    br->root_cluster=ClustersAlloc(dv,0,1,FALSE);
    WBlks(dv,br,dv->base_offset,1);
    root_dir=CAlloc(BLK_SIZE);

    d_native=root_dir-offset(CDirEntry.start);

    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    *d_native->name='.';
    d_native->cluster=br->root_cluster;
    d_native->size=BLK_SIZE;
    d_native->datetime=Now;

    d_native(U8 *)+=CDIR_SIZE;

    *d_native->name='.';
    d_native->name[1]='.';
    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    d_native->cluster=br->root_cluster;
    d_native->datetime=Now;

    WBlks(dv,root_dir,br->root_cluster,1);
    RedSeaDrvInit(dv);
    DrvUnlock(dv);
  } catch
    DrvUnlock(dv);
  Free(br);
  Free(root_dir);
}

Bool RedSeaFileFind(CDrv *dv,I64 cur_dir_cluster,U8 *name,
	CDirEntry *_result,I64 fuf_flags=0)
{//$LK,"FUF_JUST_DIRS",A="MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES",A="MN:FUF_JUST_FILES"$
  CDirEntry *buf,*buf2,*ptr;
  U8 dname[CDIR_FILENAME_LEN];
  I64 ch;
  Bool result=FALSE,unlock;
  if (fuf_flags&~FUG_FILE_FIND)
    throw('FUF');
  MemSet(_result,0,sizeof(CDirEntry));
  DrvChk(dv);
  if (dv->type!=DVT_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else if (!CFileNameTo(dname,name))
    PrintErr("Invalid FileName.\n");
  else
    try {
      unlock=DrvLock(dv);
      buf2=MAlloc(BLK_SIZE);
      RBlks(dv,buf2,cur_dir_cluster,1);

      ptr=buf2(U8 *)-offset(CDirEntry.start);
      buf=MAlloc(ptr->size);
      RBlks(dv,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
      Free(buf2);

      ptr=buf(U8 *)-offset(CDirEntry.start);
      *ptr->name='.';
      ptr->name[1]=0;
      while (TRUE) {
	if (!(ch=*ptr->name))
	  break;
	else if (!(ptr->attr & RS_ATTR_DELETED) &&
	      !(fuf_flags&FUF_JUST_DIRS && !(ptr->attr & RS_ATTR_DIR)) &&
	      !(fuf_flags&FUF_JUST_FILES && ptr->attr & RS_ATTR_DIR) &&
	      !StrCmp(dname,ptr->name)) {
	  MemCpy(&_result->attr,&ptr->attr,CDIR_SIZE);
	  result=TRUE;
	  goto rsff_done;
	}
	ptr(U8 *)+=CDIR_SIZE;
      }
rsff_done:
      Free(buf);
      if (unlock)
	DrvUnlock(dv);
    } catch
      if (unlock)
	DrvUnlock(dv);
  return result;
}

U8 *RedSeaFileRead(CDrv *dv,U8 *cur_dir,U8 *filename,I64 *_size,I64 *_attr)
{
  U8 *buf=NULL;
  CDirEntry de;
  I64 c,blk_cnt,cur_dir_cluster;
  DrvChk(dv);
  *_size=0;
  *_attr=0;
  if (dv->type!=DVT_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else
    try {
      DrvLock(dv);
      cur_dir_cluster=Name2DirCluster(dv,cur_dir);
      if (RedSeaFileFind(dv,cur_dir_cluster,filename,&de,FUF_JUST_FILES)) {
	blk_cnt=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	buf=MAlloc(blk_cnt<<BLK_SIZE_BITS+1);
	c=de.cluster;
	c=RBlks(dv,buf,c,blk_cnt);
	buf[de.size]=0; //Terminate
	*_size=de.size;
	*_attr=FileAttr(de.name,de.attr);
      }
      DrvUnlock(dv);
    } catch
      DrvUnlock(dv);
  return buf;
}

Bool RedSeaCd(U8 *name,I64 cur_dir_cluster)
{
  CDirEntry de;
  if (Fs->cur_dv->type!=DVT_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else if (RedSeaFileFind(Fs->cur_dv,cur_dir_cluster,name,&de,FUF_JUST_DIRS))
    return TRUE;
  else {
    "%s ",name;
    PrintErr("File not found.\n");
  }
  return FALSE;
}

U0 RedSeaFreeClusters(CDrv *dv,I64 c,I64 cnt)
{
  CFreeLst *tempf;
  Bool found=FALSE,unlock,unlock_break;
  DrvChk(dv);
  if (!c) return;
  if (dv->type!=DVT_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else
    try {
      unlock_break=BreakLock;
      unlock=DrvLock(dv);
      if (!dv->next_free)
	RedSeaFreeLstBuild(dv);
      tempf=dv->next_free;
      while (!found && tempf!=&dv->next_free) {
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
	QueIns(tempf,dv->last_free);
      }
      while (cnt-->0) {
	DrvMapBlkSet(dv,c);
	LBtr(dv->cur_map_blk,(c-dv->data_area)&(BLK_SIZE<<3-1));
	LBts(&dv->map_blk_dirty,0);
	c++;
      }
      DrvMapBlkClean(dv);

      if (unlock)
	DrvUnlock(dv);
      if (unlock_break)
	BreakUnlock;
    } catch {
      if (unlock)
	DrvUnlock(dv);
      if (unlock_break)
	BreakUnlock;
    }
}

I64 RedSeaAllocClusters(CDrv *dv,I64 cnt)
{
  CFreeLst *tempf,*best_free=NULL;
  I64 i,first,best_size=MAX_I64;
  Bool unlock,unlock_break;
  if (cnt<=0)
    throw('Drv');
  try {
    unlock_break=BreakLock;
    unlock=DrvLock(dv);
    if (!dv->next_free)
      RedSeaFreeLstBuild(dv);
    tempf=dv->next_free;
    while (tempf!=&dv->next_free) {
      if (tempf->size>=cnt && tempf->size<best_size) {
	best_free=tempf;
	best_size=tempf->size;
	if (tempf->size==cnt)
	  break;
      }
      tempf=tempf->next;
    }
    if (!best_free)
      throw('Drv');
    first=best_free->start;
    for (i=0;i<cnt;i++) {
      DrvMapBlkSet(dv,first+i);
      LBts(dv->cur_map_blk,(first+i-dv->data_area)&(BLK_SIZE<<3-1));
      LBts(&dv->map_blk_dirty,0);
    }
    DrvMapBlkClean(dv);
    if (best_free->size-=cnt)
      best_free->start+=cnt;
    else {
      QueRem(best_free);
      Free(best_free);
    }
    if (unlock)
      DrvUnlock(dv);
    if (unlock_break)
      BreakUnlock;
  } catch {
    if (unlock)
      DrvUnlock(dv);
    if (unlock_break)
      BreakUnlock;
  }
  return first;
}

Bool RedSeaDirNew(CDrv *dv,U8 *cur_dir,CDirEntry *de,Bool free_old_chain)
{
  CDirEntry *buf,*buf2,*ptr,de2;
  CRedSeaBoot *br;
  I64 c,ch,i=1,j=0,n=BLK_SIZE/CDIR_SIZE,dir_size,cur_dir_cluster;
  Bool written=FALSE,unlock,unlock_break;
  U8 *temp,*parent_dir;
  try {
    unlock_break=BreakLock;
    de->attr|=RS_ATTR_CONTIGUOUS;
    unlock=DrvLock(dv);
    cur_dir_cluster=Name2DirCluster(dv,cur_dir);
    buf2=MAlloc(BLK_SIZE);
    RBlks(dv,buf2,cur_dir_cluster,1);

    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    RBlks(dv,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);

    dir_size=ptr->size;
    ptr=buf(U8 *)-offset(CDirEntry.start)+CDIR_SIZE;
    Free(buf2);
    while (TRUE) {
      if (!(ch=*ptr->name)) {
	if (!written)
	  MemCpy(&ptr->start,&de->start,CDIR_SIZE);
	if ((i+1)*CDIR_SIZE+j<<BLK_SIZE_BITS<dir_size)
	  WBlks(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_cluster+j,1);
	else {
	  buf2=CAlloc(dir_size+BLK_SIZE);
	  MemCpy(buf2,buf,dir_size);
	  RedSeaFreeClusters(dv,cur_dir_cluster,dir_size>>BLK_SIZE_BITS);
	  dir_size+=BLK_SIZE;
	  c=ClustersAlloc(dv,0,dir_size>>BLK_SIZE_BITS,TRUE);
	  Free(buf);
	  buf=buf2;
	  ptr=buf(U8 *)-offset(CDirEntry.start);
	  ptr->size=dir_size;
	  ptr->cluster=c;
	  WBlks(dv,buf,c,dir_size>>BLK_SIZE_BITS);
	  if (cur_dir_cluster==dv->root_cluster) {
	    br=CAlloc(BLK_SIZE);
	    RBlks(dv,br,dv->base_offset,1);
	    br->root_cluster=c;
	    WBlks(dv,br,dv->base_offset,1);
	    Free(br);
	    dv->root_cluster=c;
	  } else {
	    temp=StrNew(cur_dir);
	    parent_dir=StrNew(cur_dir);
	    StrLastRem(parent_dir,"/",temp);
	    if (!*parent_dir) {
	      Free(parent_dir);
	      parent_dir=StrNew("/");
	    }
	    if (RedSeaFileFind(dv,Name2DirCluster(dv,parent_dir),
		  temp,&de2,FUF_JUST_DIRS)) {
	      de2.cluster=c;
	      de2.size=dir_size;
	      RedSeaDirNew(dv,parent_dir,&de2,FALSE);
	    } else
	      throw('Drv');
	    Free(temp);
	    Free(parent_dir);
	  }
	}
	break;
      } else if (ptr->attr & RS_ATTR_DELETED) {
	if (!written) {
	  MemCpy(&ptr->start,&de->start,CDIR_SIZE);
	  WBlks(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_cluster+j,1);
	  written=TRUE;
	}
      } else {
	if (!StrCmp(de->name,ptr->name)) {
	  if (free_old_chain)
	    RedSeaFreeClusters(dv,ptr->cluster,
		  (ptr->size+BLK_SIZE-1)>>BLK_SIZE_BITS);
	  if (!written)
	    MemCpy(&ptr->start,&de->start,CDIR_SIZE);
	  else
	    ptr->attr|=RS_ATTR_DELETED;
	  WBlks(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_cluster+j,1);
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
      DrvUnlock(dv);
    if (unlock_break)
      BreakUnlock;
  } catch {
    if (unlock)
      DrvUnlock(dv);
    if (unlock_break)
      BreakUnlock;
  }
  return FALSE;
}

I64 RedSeaFilesDel(CDrv *dv,U8 *cur_dir,U8 *files_find_mask,I64 fuf_flags,
		     Bool del_dir,Bool print_msg)
{
  CDirEntry *buf,*buf2,*ptr;
  I64 i=0,result=0,ch,j=0,n=BLK_SIZE/CDIR_SIZE,cur_dir_cluster;
  Bool unlock_break;
  try {
    unlock_break=BreakLock;
    DrvLock(dv);
    cur_dir_cluster=Name2DirCluster(dv,cur_dir);
    buf2=MAlloc(BLK_SIZE);
    RBlks(dv,buf2,cur_dir_cluster,1);

    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    RBlks(dv,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
    Free(buf2);

    ptr=buf(U8 *)-offset(CDirEntry.start);
    *ptr->name='.';
    ptr->name[1]=0;
    while (TRUE) {
      if (!(ch=*ptr->name))
	break;
      else if (!(ptr->attr & RS_ATTR_DELETED) && ch!='.' && (del_dir ||
	    !(ptr->attr & RS_ATTR_DIR)) &&
	    FilesFindMatch(ptr->name,files_find_mask,fuf_flags)) {
	if (!(ptr->attr & RS_ATTR_DIR)) result++;
	if (print_msg)
	  "Del %s\n",ptr->name;
	ptr->attr|=RS_ATTR_DELETED;
	WBlks(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_cluster+j,1);
	RedSeaFreeClusters(dv,ptr->cluster,
	      (ptr->size+BLK_SIZE-1)>>BLK_SIZE_BITS);
      }
      ptr(U8 *)+=CDIR_SIZE;
      if (++i>=n) {
	j++;
	i=0;
      }
    }
    Free(buf);
    DrvUnlock(dv);
    if (unlock_break)
      BreakUnlock;
  } catch {
    DrvUnlock(dv);
    if (unlock_break)
      BreakUnlock;
  }
  return result;
}

I64 RedSeaFileWrite(CDrv *dv,U8 *cur_dir,U8 *name,U8 *buf,I64 size,
	CDate cdt,I64 attr)
{
  CDirEntry de;
  I64 c=0,blk_cnt;
  MemSet(&de,0,sizeof(CDirEntry));
  if (size<0) size=0;
  if (dv->type!=DVT_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else if (!CFileNameTo(de.name,name))
    PrintErr("Invalid FileName.\n");
  else {
    RedSeaFilesDel(dv,cur_dir,de.name,0,FALSE,FALSE);
    de.size=size;
    if (blk_cnt=(size+BLK_SIZE-1)>>BLK_SIZE_BITS)
      c=ClustersAlloc(dv,0,blk_cnt,TRUE); //Always contiguous
    else
      c=INVALID_CLUSTER;
    de.cluster=c;
    de.attr=attr|RS_ATTR_CONTIGUOUS;
    de.datetime=cdt;
    if (blk_cnt)
      WBlks(dv,buf,c,blk_cnt);
    RedSeaDirNew(dv,cur_dir,&de,TRUE);
  }
  return c;
}

CDirEntry *RedSeaFilesFind(U8 *files_find_mask,I64 fuf_flags,
	CDirEntry *parent=NULL)
{
  CDrv *dv=Fs->cur_dv;
  CDirEntry *buf,*buf2,*ptr,*result=NULL,*tempde;
  I64 ch,cur_dir_cluster;
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');
  try {
    DrvLock(dv);
    cur_dir_cluster=Name2DirCluster(dv,Fs->cur_dir);
    buf2=MAlloc(BLK_SIZE);
    RBlks(dv,buf2,cur_dir_cluster,1);

    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    RBlks(dv,buf,cur_dir_cluster,ptr->size>>BLK_SIZE_BITS);
    Free(buf2);

    ptr=buf(U8 *)-offset(CDirEntry.start);
    *ptr->name='.';
    ptr->name[1]=0;
    ptr(U8 *)+=CDIR_SIZE;
    ptr->cluster=Name2ParentDirCluster(dv,Fs->cur_dir);
    ptr(U8 *)-=CDIR_SIZE;
    while (TRUE) {
      if (!(ch=*ptr->name))
	break;
      else if (!(ptr->attr & RS_ATTR_DELETED)) {
	tempde=CAlloc(sizeof(CDirEntry));
	MemCpy(&tempde->start,&ptr->start,CDIR_SIZE);
	tempde->parent=parent;
	if (Bt(&fuf_flags,FUf_RECURSE) && tempde->attr&RS_ATTR_DIR &&
	      *tempde->name!='.') {
	  tempde->next=result;
	  result=tempde;
	  tempde->full_name=DirNameAbs(tempde->name);
	  DrvUnlock(dv);
	  if (Cd(tempde->name)) {
	    tempde->sub=RedSeaFilesFind(files_find_mask,fuf_flags,tempde);
	    Cd("..");
	  }
	  DrvLock(dv);
	} else if ((tempde->attr&RS_ATTR_DIR ||
	      !Bt(&fuf_flags,FUf_JUST_DIRS)) &&
	      !(Bt(&fuf_flags,FUf_RECURSE) && *tempde->name=='.' &&
	      tempde->attr&RS_ATTR_DIR) &&
	      FilesFindMatch(tempde->name,files_find_mask,fuf_flags)) {
	  tempde->next=result;
	  result=tempde;
	  tempde->full_name=FileNameAbs(tempde->name);
	} else
	  DirEntryDel(tempde);
      }
      ptr(U8 *)+=CDIR_SIZE;
    }
    Free(buf);
    DrvUnlock(dv);
  } catch
    DrvUnlock(dv);
  return result;
}

Bool RedSeaMkDir(CDrv *dv,U8 *cur_dir,U8 *name)
{
  I64 size=BLK_SIZE,c,cur_dir_cluster=Name2DirCluster(dv,cur_dir);
  U8 *buf=CAlloc(size);
  CDirEntry *d_native=buf-offset(CDirEntry.start);
  Bool unlock_break;

  try {
    unlock_break=BreakLock;
    c=FileWrite(name,buf,size,0,RS_ATTR_DIR);
    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    StrCpy(d_native->name,name);
    d_native->cluster=c;
    d_native->size=size;
    d_native->datetime=Now;
    d_native(U8 *)+=CDIR_SIZE;

    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    *d_native->name='.';
    d_native->name[1]='.';
    d_native->name[2]=0;
    d_native->cluster=cur_dir_cluster;
    d_native->size=0;
    d_native->datetime=Now;
    WBlks(dv,buf,c,1);
    Free(buf);
    if (unlock_break)
      BreakUnlock;
  } catch
    if (unlock_break)
      BreakUnlock;
  return TRUE;
}
