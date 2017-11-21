// See $LK,"RedSea File System",A="FI:::/Doc/RedSea.DD"$

U0 RedSeaFreeFreeLst(CDrv *dv)
{
  CFreeLst *tmpf,*tmpf1;
  Bool unlock;
  try {
    unlock=DrvLock(dv);
    if (tmpf=dv->next_free) {
      while (tmpf!=&dv->next_free) {
	tmpf1=tmpf->next;
	Free(tmpf);
	tmpf=tmpf1;
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
  CFreeLst *tmpf;
  I64 i,first=dv->data_area,max_blk=dv->size+dv->drv_offset;
  try {
    unlock=DrvLock(dv);
    if (dv->next_free)
      RedSeaFreeFreeLst(dv);
    QueInit(&dv->next_free);
    while (first<max_blk) {
      i=0;  //count free clus
      while (first+i<max_blk) {
	DrvFATBlkSet(dv,first+i);
	if (Bt(dv->cur_fat_blk,(first+i-dv->data_area)&(BLK_SIZE<<3-1)))
	  break;
	else
	  i++;
      }
      if (i) {
	tmpf=AMAlloc(sizeof(CFreeLst));
	tmpf->size=i;
	tmpf->start=first;
	QueIns(tmpf,dv->last_free);
      }
      first+=i+1;
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
}

U0 RedSeaInit(CDrv *dv)
{
  CRedSeaBoot br;
  Bool unlock;
  try {
    unlock=DrvLock(dv);
    BlkRead(dv,&br,dv->drv_offset,1);
    if (br.signature!=MBR_PT_REDSEA || br.signature2!=0xAA55)
      throw('Drv');
    dv->fs_type=FSt_REDSEA;
    RedSeaFreeFreeLst(dv);
    dv->spc=1;
    dv->size=br.sects;
    dv->data_area=dv->drv_offset+br.bitmap_sects;
    dv->root_clus=br.root_clus;
    dv->fat1=dv->fat2=dv->drv_offset+1;
    DrvFATBlkAlloc(dv);
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
}

Bool RedSeaValidate(U8 drv_let)
{
  CDrv *dv;
  CRedSeaBoot br;
  if ((dv=Let2Drv(drv_let,FALSE)) && dv->fs_type==FSt_REDSEA &&
	BlkRead(dv,&br,dv->drv_offset,1) && br.signature==MBR_PT_REDSEA &&
	br.signature2==0xAA55)
    return TRUE;
  else
    return FALSE;
}

U0 RedSeaFmt(U8 drv_let,Bool quick=TRUE)
{
  U8 *root_dir;
  CDirEntry *d_native;
  CRedSeaBoot *br=CAlloc(BLK_SIZE);
  CDrv *dv=Let2Drv(drv_let);
  I64 i,n,root_dir_blks;
  try {
    DrvLock(dv);
//	DrvTypeSet(drv_let,FSt_REDSEA);
    DrvTypeSet(drv_let,FSt_FAT32);
    dv->fs_type=FSt_REDSEA;
    br->signature=MBR_PT_REDSEA;
    br->signature2=0xAA55;
    br->drv_offset=dv->drv_offset; //For CD/DVD image copy.
    br->sects=dv->size;
    n=(br->sects+BLK_SIZE<<3-1)/BLK_SIZE<<3;
    br->bitmap_sects=n;
    br->unique_id=GetTSC^Now()(U64);
    br->root_clus=0;

    if (quick)
      i=n+1;
    else
      i=dv->size;
    BlkWriteZero(dv,dv->drv_offset,i);

    BlkWrite(dv,br,dv->drv_offset,1);
    RedSeaInit(dv);
    ClusAlloc(dv,0,1,FALSE);	//Alloc #1

    root_dir_blks=MaxI64(1,dv->bd->init_root_dir_blks);
    br->root_clus=ClusAlloc(dv,0,root_dir_blks,FALSE);
    BlkWrite(dv,br,dv->drv_offset,1);
    root_dir=CAlloc(BLK_SIZE*root_dir_blks);

    d_native=root_dir-offset(CDirEntry.start);

    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    *d_native->name='.';
    d_native->clus=br->root_clus;
    d_native->size=BLK_SIZE*root_dir_blks;
    d_native->datetime=Now;

    d_native(U8 *)+=CDIR_SIZE;

    *d_native->name='.';
    d_native->name[1]='.';
    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    d_native->clus=br->root_clus;
    d_native->datetime=Now;

    BlkWrite(dv,root_dir,br->root_clus,root_dir_blks);
    RedSeaInit(dv);
    DrvUnlock(dv);
  } catch
    DrvUnlock(dv);
  Free(br);
  Free(root_dir);
}

Bool RedSeaFileFind(CDrv *dv,I64 cur_dir_clus,U8 *name,
	CDirEntry *_res,I64 fuf_flags=0)
{//$LK,"FUF_JUST_DIRS",A="MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES",A="MN:FUF_JUST_FILES"$
  CDirEntry *buf,*buf2,*ptr;
  U8 dname[CDIR_FILENAME_LEN];
  I64 ch;
  Bool res=FALSE,unlock;
  if (fuf_flags&~FUG_FILE_FIND)
    throw('FUF');
  MemSet(_res,0,sizeof(CDirEntry));
  DrvChk(dv);
  if (dv->fs_type!=FSt_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else if (!CFileNameTo(dname,name))
    PrintErr("Invalid FileName: \"%s\".\n",name);
  else
    try {
      unlock=DrvLock(dv);
      buf2=MAlloc(BLK_SIZE);
      BlkRead(dv,buf2,cur_dir_clus,1);

      ptr=buf2(U8 *)-offset(CDirEntry.start);
      buf=MAlloc(ptr->size);
      BlkRead(dv,buf,cur_dir_clus,ptr->size>>BLK_SIZE_BITS);
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
	  MemCpy(&_res->attr,&ptr->attr,CDIR_SIZE);
	  res=TRUE;
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
  return res;
}

U8 *RedSeaFileRead(CDrv *dv,U8 *cur_dir,U8 *filename,I64 *_size,I64 *_attr)
{
  U8 *buf=NULL;
  CDirEntry de;
  I64 c,blk_cnt,cur_dir_clus;
  DrvChk(dv);
  *_size=0;
  *_attr=0;
  if (dv->fs_type!=FSt_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else
    try {
      DrvLock(dv);
      cur_dir_clus=Name2DirClus(dv,cur_dir);
      if (RedSeaFileFind(dv,cur_dir_clus,filename,&de,FUF_JUST_FILES)) {
	blk_cnt=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	buf=MAlloc(blk_cnt<<BLK_SIZE_BITS+1);
	c=de.clus;
	c=BlkRead(dv,buf,c,blk_cnt);
	buf[de.size]=0; //Terminate
	*_size=de.size;
	*_attr=FileAttr(de.name,de.attr);
      }
      DrvUnlock(dv);
    } catch
      DrvUnlock(dv);
  return buf;
}

Bool RedSeaCd(U8 *name,I64 cur_dir_clus)
{
  CDirEntry de;
  if (Fs->cur_dv->fs_type!=FSt_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else if (RedSeaFileFind(Fs->cur_dv,cur_dir_clus,name,&de,FUF_JUST_DIRS))
    return TRUE;
  else
    PrintErr("File not found: \"%s\".\n",name);
  return FALSE;
}

U0 RedSeaFreeClus(CDrv *dv,I64 c,I64 cnt)
{
  CFreeLst *tmpf;
  Bool found=FALSE,unlock,unlock_break;
  DrvChk(dv);
  if (!c) return;
  if (dv->fs_type!=FSt_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else
    try {
      unlock_break=BreakLock;
      unlock=DrvLock(dv);
      if (!dv->next_free)
	RedSeaFreeLstBuild(dv);
      tmpf=dv->next_free;
      while (!found && tmpf!=&dv->next_free) {
	if (tmpf->start+tmpf->size==c) {
	  tmpf->size+=cnt;
	  found=TRUE;
	} else if (c+cnt==tmpf->start) {
	  tmpf->size+=cnt;
	  tmpf->start=c;
	  found=TRUE;
	}
	tmpf=tmpf->next;
      }
      if (!found) {
	tmpf=AMAlloc(sizeof(CFreeLst));
	tmpf->size=cnt;
	tmpf->start=c;
	QueIns(tmpf,dv->last_free);
      }
      while (cnt-->0) {
	DrvFATBlkSet(dv,c);
	LBtr(dv->cur_fat_blk,(c-dv->data_area)&(BLK_SIZE<<3-1));
	LBts(&dv->fat_blk_dirty,0);
	c++;
      }
      DrvFATBlkClean(dv);

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

I64 RedSeaAllocClus(CDrv *dv,I64 cnt)
{
  CFreeLst *tmpf,*best_free=NULL;
  I64 i,first,best_size=I64_MAX;
  Bool unlock,unlock_break;
  if (cnt<=0)
    throw('Drv');
  try {
    unlock_break=BreakLock;
    unlock=DrvLock(dv);
    if (!dv->next_free)
      RedSeaFreeLstBuild(dv);
    tmpf=dv->next_free;
    while (tmpf!=&dv->next_free) {
      if (tmpf->size>=cnt && tmpf->size<best_size) {
	best_free=tmpf;
	best_size=tmpf->size;
	if (tmpf->size==cnt)
	  break;
      }
      tmpf=tmpf->next;
    }
    if (!best_free)
      throw('Drv');
    first=best_free->start;
    for (i=0;i<cnt;i++) {
      DrvFATBlkSet(dv,first+i);
      LBts(dv->cur_fat_blk,(first+i-dv->data_area)&(BLK_SIZE<<3-1));
      LBts(&dv->fat_blk_dirty,0);
    }
    DrvFATBlkClean(dv);
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

Bool RedSeaDirNew(CDrv *dv,U8 *cur_dir,CDirEntry *tmpde,Bool free_old_chain)
{
  CDirEntry *buf,*buf2,*ptr,de2;
  CRedSeaBoot *br;
  I64 c,ch,i=1,j=0,n=BLK_SIZE/CDIR_SIZE,dir_size,cur_dir_clus;
  Bool written=FALSE,unlock,unlock_break;
  U8 *tmp,*parent_dir;
  try {
    unlock_break=BreakLock;
    tmpde->attr|=RS_ATTR_CONTIGUOUS;
    unlock=DrvLock(dv);
    cur_dir_clus=Name2DirClus(dv,cur_dir);
    buf2=MAlloc(BLK_SIZE);
    BlkRead(dv,buf2,cur_dir_clus,1);

    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    BlkRead(dv,buf,cur_dir_clus,ptr->size>>BLK_SIZE_BITS);

    dir_size=ptr->size;
    ptr=buf(U8 *)-offset(CDirEntry.start)+CDIR_SIZE;
    Free(buf2);
    while (TRUE) {
      if (!(ch=*ptr->name)) {
	if (!written)
	  MemCpy(&ptr->start,&tmpde->start,CDIR_SIZE);
	if ((i+1)*CDIR_SIZE+j<<BLK_SIZE_BITS<dir_size)
	  BlkWrite(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_clus+j,1);
	else {
	  buf2=CAlloc(dir_size+BLK_SIZE);
	  MemCpy(buf2,buf,dir_size);
	  RedSeaFreeClus(dv,cur_dir_clus,dir_size>>BLK_SIZE_BITS);
	  dir_size+=BLK_SIZE;
	  c=ClusAlloc(dv,0,dir_size>>BLK_SIZE_BITS,TRUE);
	  Free(buf);
	  buf=buf2;
	  ptr=buf(U8 *)-offset(CDirEntry.start);
	  ptr->size=dir_size;
	  ptr->clus=c;
	  BlkWrite(dv,buf,c,dir_size>>BLK_SIZE_BITS);
	  if (cur_dir_clus==dv->root_clus) {
	    br=CAlloc(BLK_SIZE);
	    BlkRead(dv,br,dv->drv_offset,1);
	    br->root_clus=c;
	    BlkWrite(dv,br,dv->drv_offset,1);
	    Free(br);
	    dv->root_clus=c;
	  } else {
	    tmp=StrNew(cur_dir);
	    parent_dir=StrNew(cur_dir);
	    StrLastRem(parent_dir,"/",tmp);
	    if (!*parent_dir) {
	      Free(parent_dir);
	      parent_dir=StrNew("/");
	    }
	    if (RedSeaFileFind(dv,Name2DirClus(dv,parent_dir),
		  tmp,&de2,FUF_JUST_DIRS)) {
	      de2.clus=c;
	      de2.size=dir_size;
	      RedSeaDirNew(dv,parent_dir,&de2,FALSE);
	    } else
	      throw('Drv');
	    Free(tmp);
	    Free(parent_dir);
	  }
	}
	break;
      } else if (ptr->attr & RS_ATTR_DELETED) {
	if (!written) {
	  MemCpy(&ptr->start,&tmpde->start,CDIR_SIZE);
	  BlkWrite(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_clus+j,1);
	  written=TRUE;
	}
      } else {
	if (!StrCmp(tmpde->name,ptr->name)) {
	  if (free_old_chain)
	    RedSeaFreeClus(dv,ptr->clus,
		  (ptr->size+BLK_SIZE-1)>>BLK_SIZE_BITS);
	  if (!written)
	    MemCpy(&ptr->start,&tmpde->start,CDIR_SIZE);
	  else
	    ptr->attr|=RS_ATTR_DELETED;
	  BlkWrite(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_clus+j,1);
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
  I64 i=0,res=0,ch,j=0,n=BLK_SIZE/CDIR_SIZE,cur_dir_clus;
  Bool unlock_break;
  try {
    unlock_break=BreakLock;
    DrvLock(dv);
    cur_dir_clus=Name2DirClus(dv,cur_dir);
    buf2=MAlloc(BLK_SIZE);
    BlkRead(dv,buf2,cur_dir_clus,1);

    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    BlkRead(dv,buf,cur_dir_clus,ptr->size>>BLK_SIZE_BITS);
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
	if (!(ptr->attr & RS_ATTR_DIR)) res++;
	if (print_msg)
	  "Del %s\n",ptr->name;
	ptr->attr|=RS_ATTR_DELETED;
	BlkWrite(dv,buf(U8 *)+j<<BLK_SIZE_BITS,cur_dir_clus+j,1);
	RedSeaFreeClus(dv,ptr->clus,
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
  return res;
}

I64 RedSeaFileWrite(CDrv *dv,U8 *cur_dir,U8 *name,U8 *buf,I64 size,
	CDate cdt,I64 attr)
{
  CDirEntry de;
  I64 c=0,blk_cnt;
  MemSet(&de,0,sizeof(CDirEntry));
  if (size<0) size=0;
  if (dv->fs_type!=FSt_REDSEA)
    PrintErr("Not RedSea Drv\n");
  else if (!CFileNameTo(de.name,name))
    PrintErr("Invalid FileName: \"%s\".\n",name);
  else {
    RedSeaFilesDel(dv,cur_dir,de.name,0,FALSE,FALSE);
    de.size=size;
    if (blk_cnt=(size+BLK_SIZE-1)>>BLK_SIZE_BITS)
      c=ClusAlloc(dv,0,blk_cnt,TRUE); //Always contiguous
    else
      c=INVALID_CLUS;
    de.clus=c;
    de.attr=attr|RS_ATTR_CONTIGUOUS;
    de.datetime=cdt;
    if (blk_cnt)
      BlkWrite(dv,buf,c,blk_cnt);
    RedSeaDirNew(dv,cur_dir,&de,TRUE);
  }
  return c;
}

CDirEntry *RedSeaFilesFind(U8 *files_find_mask,I64 fuf_flags,
	CDirEntry *parent=NULL)
{
  CDrv *dv=Fs->cur_dv;
  CDirEntry *buf,*buf2,*ptr,*res=NULL,*tmpde;
  I64 ch,cur_dir_clus;
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');
  try {
    DrvLock(dv);
    cur_dir_clus=Name2DirClus(dv,Fs->cur_dir);
    buf2=MAlloc(BLK_SIZE);
    BlkRead(dv,buf2,cur_dir_clus,1);

    ptr=buf2(U8 *)-offset(CDirEntry.start);
    buf=MAlloc(ptr->size);
    BlkRead(dv,buf,cur_dir_clus,ptr->size>>BLK_SIZE_BITS);
    Free(buf2);

    ptr=buf(U8 *)-offset(CDirEntry.start);
    *ptr->name='.';
    ptr->name[1]=0;
    ptr(U8 *)+=CDIR_SIZE;
    ptr->clus=Name2ParentDirClus(dv,Fs->cur_dir);
    ptr(U8 *)-=CDIR_SIZE;
    while (TRUE) {
      if (!(ch=*ptr->name))
	break;
      else if (!(ptr->attr & RS_ATTR_DELETED)) {
	tmpde=CAlloc(sizeof(CDirEntry));
	MemCpy(&tmpde->start,&ptr->start,CDIR_SIZE);
	tmpde->parent=parent;
	if (Bt(&fuf_flags,FUf_RECURSE) && tmpde->attr&RS_ATTR_DIR &&
	      *tmpde->name!='.') {
	  tmpde->next=res;
	  res=tmpde;
	  tmpde->full_name=DirNameAbs(tmpde->name);
	  DrvUnlock(dv);
	  if (Cd(tmpde->name)) {
	    tmpde->sub=RedSeaFilesFind(files_find_mask,fuf_flags,tmpde);
	    Cd("..");
	  }
	  DrvLock(dv);
	} else {
	  tmpde->full_name=FileNameAbs(tmpde->name);
	  if ((tmpde->attr&RS_ATTR_DIR ||
		!Bt(&fuf_flags,FUf_JUST_DIRS)) &&
		!(Bt(&fuf_flags,FUf_RECURSE) && *tmpde->name=='.' &&
		tmpde->attr&RS_ATTR_DIR) &&
		FilesFindMatch(tmpde->full_name,files_find_mask,fuf_flags)) {
	    tmpde->next=res;
	    res=tmpde;
	  } else
	    DirEntryDel(tmpde);
	}
      }
      ptr(U8 *)+=CDIR_SIZE;
    }
    Free(buf);
    DrvUnlock(dv);
  } catch
    DrvUnlock(dv);
  return res;
}

Bool RedSeaMkDir(CDrv *dv,U8 *cur_dir,U8 *name,I64 entry_cnt)
{//entry_cnt is for preallocating dir blks.
  I64	c,cur_dir_clus=Name2DirClus(dv,cur_dir),
	size=CeilU64((entry_cnt+3)<<6,BLK_SIZE);
#assert CDIR_SIZE==64
  U8 *buf=CAlloc(size);
  CDirEntry *d_native=buf-offset(CDirEntry.start);
  Bool unlock_break;
  try {
    unlock_break=BreakLock;
    c=FileWrite(name,buf,size,0,RS_ATTR_DIR);
    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    StrCpy(d_native->name,name);
    d_native->clus=c;
    d_native->size=size;
    d_native->datetime=Now;
    d_native(U8 *)+=CDIR_SIZE;

    d_native->attr=RS_ATTR_DIR|RS_ATTR_CONTIGUOUS;
    *d_native->name='.';
    d_native->name[1]='.';
    d_native->name[2]=0;
    d_native->clus=cur_dir_clus;
    d_native->size=0;
    d_native->datetime=Now;
    BlkWrite(dv,buf,c,1);
    Free(buf);
    if (unlock_break)
      BreakUnlock;
  } catch
    if (unlock_break)
      BreakUnlock;
  return TRUE;
}
