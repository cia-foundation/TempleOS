I64 FSize(CFile *f)
{//Report size of opened file in bytes.
  if (f)
    return f->de.size;
  else
    return 0;
}

CFile *FOpen(U8 *filename,U8 *flags,I64 cnt=0)
{//Allows flags "r","w","w+". "c" for contiguous.
//(It uses $LK,"StrOcc",A="MN:StrOcc"$() for 'w', 'r', '+', 'c')
  CFile *f=CAlloc(sizeof(CFile));
  CDirContext *dirc;
  U8 *full_name;
  Bool contiguous=StrOcc(flags,'c');

  f->clus=INVALID_CLUS;
  f->fblk_num=0;
  if (cnt>0)
    f->max_blk=cnt-1;
  else
    f->max_blk=I64_MAX;
  f->file_clus_num=INVALID_CLUS;
  full_name=FileNameAbs(filename);
  f->dv=Let2Drv(*full_name);
  if (f->dv->fs_type==FSt_REDSEA)
    contiguous=TRUE;
  if (contiguous) {
    f->flags|=FF_CONTIGUOUS;
    if (f->dv->fs_type!=FSt_REDSEA &&
	  !(FileAttr(filename) & RS_ATTR_CONTIGUOUS))
      throw('File');
  }
  f->clus_buf=CAlloc(f->dv->spc<<BLK_SIZE_BITS);
  if (StrOcc(flags,'w')) {
    f->flags=f->flags|FF_WRITE|FF_NEEDS_WRITE;
    if (StrOcc(flags,'+')) {
      if (FileFind(full_name,&f->de,FUF_JUST_FILES)) {
	Free(full_name);
	if (contiguous)
	  f->max_blk=(FSize(f)+BLK_SIZE-1)>>BLK_SIZE_BITS-1;
	return f;
      }
    } else
      Del(full_name,,,FALSE);
    f->de.full_name=full_name;
    f->flags|=FF_NEW_FILE;
    if (dirc=DirContextNew(full_name)) {
      StrCpy(f->de.name,dirc->mask);
      if (cnt>0) {//We pre-alloc the whole thing.
	f->de.clus=ClusAlloc(f->dv,0,
	      (cnt+f->dv->spc-1)/f->dv->spc,contiguous);
	f->de.size=cnt<<BLK_SIZE_BITS;
	DirNew(dirc->dv,Fs->cur_dir,&f->de,TRUE);
	f->flags&=~FF_NEW_FILE;
      }
      DirContextDel(dirc);
      return f;
    }
  } else {
    if (FileFind(full_name,&f->de,FUF_JUST_FILES)) {
      Free(full_name);
      f->max_blk=(FSize(f)+BLK_SIZE-1)>>BLK_SIZE_BITS-1;
      return f;
    }
  }
  Free(f->clus_buf);
  Free(full_name);
  Free(f);
  return NULL;
}

U0 FClose(CFile *f)
{//Close CFile, updating directory.
  CDirContext *dirc;
  if (f) {
    if (f->flags & FF_BUF_DIRTY) {
      ClusWrite(f->dv,f->clus_buf,f->clus,1);
      f->flags&=~FF_BUF_DIRTY;
    }
    if (f->flags & FF_NEEDS_WRITE) {
      if (dirc=DirContextNew(f->de.full_name)) {
	if (!(f->flags & FF_USE_OLD_DATETIME))
	  f->de.datetime=Now;
	if (f->flags & FF_NEW_FILE)
	  DirNew(dirc->dv,Fs->cur_dir,&f->de,TRUE);
	else
	  DirNew(dirc->dv,Fs->cur_dir,&f->de,FALSE);
	DirContextDel(dirc);
      } else
	throw('File');
    }
    Free(f->clus_buf);
    Free(f->de.full_name);
    Free(f);
  }
}

I64 FSetClus(CFile *f,I64 c,I64 blk,Bool read)
{
  CDrv *dv=f->dv;
  I64 i;
  if (f->clus!=c) {
    if (f->flags & FF_BUF_DIRTY) {
      i=dv->spc;
      if (f->max_blk!=I64_MAX) {
	i=f->max_blk+1-f->file_clus_num*dv->spc;
	if (i>dv->spc)
	  i=dv->spc;
      }
      ClusBlkWrite(dv,f->clus_buf,f->clus,i);
      f->flags=f->flags & ~FF_BUF_DIRTY;
    }
    f->clus=c;
    f->file_clus_num=blk/dv->spc;
    if (read) {
      i=dv->spc;
      if (f->max_blk!=I64_MAX) {
	i=f->max_blk+1-f->file_clus_num*dv->spc;
	if (i>dv->spc)
	  i=dv->spc;
      }
      c=ClusBlkRead(dv,f->clus_buf,c,i);
    }
  }
  return c;
}

Bool FBlkRead(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1)
{//Read [nth,n+cnt) blks of file.
  CDrv *dv=f->dv;
  I64 spc=dv->spc,i,j,c=f->de.clus;
  if (!f || !dv) return FALSE;
  if (blk==FFB_NEXT_BLK)
    blk=f->fblk_num;
  if (blk+cnt-1>f->max_blk)
    return FALSE;
  if (cnt<=0) return TRUE;

  if (f->flags & FF_CONTIGUOUS) {
    BlkRead(dv,buf,Clus2Blk(dv,c)+blk,cnt);
    blk+=cnt;
  } else {
    i=blk/spc;
    if (0<=f->file_clus_num<=i) {
      c=f->clus;
      i-=f->file_clus_num;
    }
    if (i>0)
      c=ClusNumNext(dv,c,i);

    if (i=blk%spc) {
      c=FSetClus(f,c,blk,TRUE);
      if (cnt<spc-i)
	j=cnt;
      else
	j=spc-i;
      MemCpy(buf,f->clus_buf+i<<BLK_SIZE_BITS,j<<BLK_SIZE_BITS);
      buf+=j<<BLK_SIZE_BITS;
      cnt-=j;
      blk+=j;
    }
    while (cnt>=spc) {
      c=FSetClus(f,c,blk,TRUE);
      MemCpy(buf,f->clus_buf,spc<<BLK_SIZE_BITS);
      buf+=spc<<BLK_SIZE_BITS;
      cnt-=spc;
      blk+=spc;
    }
    if (cnt>0) {
      c=FSetClus(f,c,blk,TRUE);
      MemCpy(buf,f->clus_buf,cnt<<BLK_SIZE_BITS);
      buf+=cnt<<BLK_SIZE_BITS;
      blk+=cnt;
    }
  }
  f->fblk_num=blk;
  return TRUE;
}

Bool FBlkWrite(CFile *f,U8 *buf,I64 blk=FFB_NEXT_BLK,I64 cnt=1)
{//Write [nth,n+cnt) blks of file.
  CDrv *dv=f->dv;
  I64 spc=dv->spc,i,j,c=f->de.clus,c1;
  if (!f || !dv) return FALSE;
  if (blk==FFB_NEXT_BLK)
    blk=f->fblk_num;
  if (blk+cnt-1>f->max_blk)
    return FALSE;
  if (!(f->flags & FF_WRITE))
    return FALSE;
  if (cnt<=0) return TRUE;
  if (f->flags & FF_CONTIGUOUS) {
    BlkWrite(dv,buf,Clus2Blk(dv,c)+blk,cnt);
    blk+=cnt;
  } else {
    if (!c) {
      c=ClusAlloc(dv,0,1,FALSE);
      f->file_clus_num=0;
      f->clus=c;
      f->de.clus=c;
      f->flags|=FF_NEEDS_WRITE|FF_NEW_FILE;
    }
    i=blk/spc;
    if (0<=f->file_clus_num<=i) {
      c=f->clus;
      i-=f->file_clus_num;
    }
    while (i>0) {
      c1=c;
      c=ClusNumNext(dv,c1,1);
      if (c==INVALID_CLUS) {
	c=ClusAlloc(dv,c1,i,FALSE);
	if (i>1)
	  c=ClusNumNext(dv,c,i-1);
	break;
      } else
	i--;
    }

    if (i=blk%spc) {
      FSetClus(f,c,blk,TRUE);
      if (cnt<spc-i)
	j=cnt;
      else
	j=spc-i;
      MemCpy(f->clus_buf+BLK_SIZE*i,buf,j<<BLK_SIZE_BITS);
      f->flags|=FF_BUF_DIRTY;
      buf+=j<<BLK_SIZE_BITS;
      cnt-=j;
      blk+=j;
      if (cnt>0) {
	c1=c;
	c=ClusNumNext(dv,c1,1);
	if (c==INVALID_CLUS)
	  c=ClusAlloc(dv,c1,1,FALSE);
      }
    }
    while (cnt>=spc) {
      FSetClus(f,c,blk,FALSE);
      MemCpy(f->clus_buf,buf,spc<<BLK_SIZE_BITS);
      f->flags|=FF_BUF_DIRTY;
      buf+=spc<<BLK_SIZE_BITS;
      cnt-=spc;
      blk+=spc;
      if (cnt>0) {
	c1=c;
	c=ClusNumNext(dv,c1,1);
	if (c==INVALID_CLUS)
	  c=ClusAlloc(dv,c1,1,FALSE);
      }
    }
    if (cnt>0) {
      FSetClus(f,c,blk,TRUE);
      MemCpy(f->clus_buf,buf,cnt<<BLK_SIZE_BITS);
      f->flags|=FF_BUF_DIRTY;
      buf+=cnt<<BLK_SIZE_BITS;
      blk+=cnt;
    }
    if (f->de.size<blk<<BLK_SIZE_BITS)
      f->de.size=blk<<BLK_SIZE_BITS;
  }
  f->fblk_num=blk;
  return TRUE;
}
