U0 Date2ISO1(CISO1Date *dst,CDate cdt)
{
  CDateStruct ds;
  Date2Struct(&ds,cdt);
  dst->year=ds.year-ISO1_BASE_YEAR;
  dst->mon=ds.mon;
  dst->day=ds.day_of_mon;
  dst->hour=ds.hour;
  dst->min=ds.min;
  dst->sec=ds.sec;
  dst->sec100=ds.sec100;
}

CDate ISO1DateStruct2CDate(CISO1Date *dt)
{
  CDateStruct ds;
  MemSet(&ds,0,sizeof(CDateStruct));
  ds.day_of_mon=dt->day;
  ds.mon=dt->mon;
  ds.year=dt->year+ISO1_BASE_YEAR;
  ds.sec100=dt->sec100;
  ds.sec=dt->sec;
  ds.min=dt->min;
  ds.hour=dt->hour;
  return Struct2Date(&ds);
}

Bool ISO1FromName(U8 *dst,U8 *src)
{
  I64 i,j,n;
  MemSet(dst,0,CDIR_FILENAME_LEN);
  n=*src++;
  if (n==1 && !*src) {
    *dst='.';
  } else if (n==1 && *src==1) {
    *dst='.';
    dst[1]='.';
  } else {
    n>>=1;
    j=0;
    for (i=0;i<n;i++) {
      src++;
      if (*src==';')
	break;
      if (Bt(char_bmp_filename,*src)) {
	if (j>=CDIR_FILENAME_LEN-1)
	  return FALSE;
	dst[j++]=*src++;
      } else
	return FALSE;
    }
  }
  return FileNameChk(dst);
}

Bool ISO1CDirFill(CDirEntry *tmpde,CISO1DirEntry *de)
{
  Bool res;
  MemSet(tmpde,0,sizeof(CDirEntry));
  res=ISO1FromName(tmpde->name,&de->name_len);
  tmpde->clus=de->loc.little;
  tmpde->size=de->size.little;
  tmpde->attr=FileAttr(tmpde->name);
  if (de->flags & ISO1_ATTR_DIR)
    tmpde->attr|=RS_ATTR_DIR;
  tmpde->datetime=ISO1DateStruct2CDate(&de->date);
  return res;
}

Bool ISO1FileFind(CDrv *dv,I64 cur_dir_clus,U8 *name,
	CDirEntry *_res,I64 fuf_flags=0)
{//$LK,"FUF_JUST_DIRS",A="MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES",A="MN:FUF_JUST_FILES"$
  CISO1DirEntry *isoptr,*buf;
  U8 dname[CDIR_FILENAME_LEN];
  Bool res=FALSE,unlock;
  I64 i;
  if (fuf_flags&~FUG_FILE_FIND)
    throw('FUF');
  DrvChk(dv);
  if (dv->fs_type!=FSt_ISO9660)
    PrintErr("Not ISO9660 Drv\n");
  else
    try {
      unlock=DrvLock(dv);
      isoptr=MAlloc(dv->spc<<BLK_SIZE_BITS);
      ClusRead(dv,isoptr,cur_dir_clus,1);
      if (isoptr->name_len==1 && !isoptr->name) {//curdir
	i=(isoptr->size.little+dv->spc<<BLK_SIZE_BITS-1)/dv->spc<<BLK_SIZE_BITS;
	buf=MAlloc(dv->spc<<BLK_SIZE_BITS*i);
	ClusRead(dv,buf,cur_dir_clus,i);
	Free(isoptr);
      } else {
	buf=isoptr;
	i=1;
      }
      i*=dv->spc<<BLK_SIZE_BITS;
      isoptr=buf;
      while (i>0) {
	if (!isoptr->len) {
	  isoptr(U8 *)++;
	  i--;
	} else {
	  ISO1FromName(dname,&isoptr->name_len);
	  if (*dname) {
	    if (!StrCmp(name,dname)) {
	      res=ISO1CDirFill(_res,isoptr);
	      if (res &&
		    !(fuf_flags&FUF_JUST_DIRS &&
		    !(_res->attr & RS_ATTR_DIR)) &&
		    !(fuf_flags&FUF_JUST_FILES && _res->attr & RS_ATTR_DIR) )
		goto iff_done;
	      else
		res=FALSE;
	    }
	  }
	  i-=isoptr->len;
	  isoptr(U8 *)+=isoptr->len;
	}
      }
iff_done:
      Free(buf);
      if (unlock)
	DrvUnlock(dv);
    } catch
      if (unlock)
	DrvUnlock(dv);
  return res;
}

U8 *ISO1FileRead(CDrv *dv,U8 *cur_dir,U8 *filename,I64 *_size,I64 *_attr)
{
  U8 *buf=NULL;
  CDirEntry de;
  I64 c,blk_cnt,cur_dir_clus;
  DrvChk(dv);
  *_size=0;
  *_attr=0;
  if (dv->fs_type!=FSt_ISO9660)
    PrintErr("Not ISO9660 Drv\n");
  else
    try {
      DrvLock(dv);
      cur_dir_clus=Name2DirClus(dv,cur_dir);
      if (ISO1FileFind(dv,cur_dir_clus,filename,&de,FUF_JUST_FILES)) {
	blk_cnt=(de.size+BLK_SIZE-1)>>BLK_SIZE_BITS;
	buf=MAlloc(blk_cnt<<BLK_SIZE_BITS+1);
	c=de.clus;
	c=ClusBlkRead(dv,buf,c,blk_cnt);
	buf[de.size]=0; //Terminate
	*_size=de.size;
	*_attr=FileAttr(de.name,de.attr);
      }
      DrvUnlock(dv);
    } catch
      DrvUnlock(dv);
  return buf;
}

Bool ISO1Cd(U8 *name,I64 cur_dir_clus)
{
  CDirEntry de;
  if (Fs->cur_dv->fs_type!=FSt_ISO9660)
    PrintErr("Not ISO9660 Drv\n");
  else if (ISO1FileFind(Fs->cur_dv,cur_dir_clus,name,&de,FUF_JUST_DIRS))
    return TRUE;
  else {
    "%s ",name;
    PrintErr("File not found.\n");
  }
  return FALSE;
}

CDirEntry *ISO1FilesFind(U8 *files_find_mask,
	I64 fuf_flags,CDirEntry *parent=NULL)
{
  CDrv *dv=Fs->cur_dv;
  CISO1DirEntry *buf,*buf2,*isoptr;
  I64 i,cur_dir_clus=Name2DirClus(dv,Fs->cur_dir);
  CDirEntry *res=NULL,*tmpde;
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');
  isoptr=MAlloc(dv->spc<<BLK_SIZE_BITS);
  ClusRead(dv,isoptr,cur_dir_clus,1);
  if (isoptr->name_len==1 && !isoptr->name) {//curdir
    i=(isoptr->size.little+dv->spc<<BLK_SIZE_BITS-1)/dv->spc<<BLK_SIZE_BITS;
    buf=MAlloc(dv->spc<<BLK_SIZE_BITS*i);
    ClusRead(dv,buf,cur_dir_clus,i);
    Free(isoptr);
  } else {
    buf=isoptr;
    i=1;
  }
  buf2=buf;
  i*=dv->spc<<BLK_SIZE_BITS;
  while (i>0) {
    if (!buf->len) {
      buf(U8 *)++;
      i--;
    } else {
      tmpde=MAlloc(sizeof(CDirEntry));
      if (ISO1CDirFill(tmpde,buf)) {
	tmpde->parent=parent;
	if (Bt(&fuf_flags,FUf_RECURSE) && tmpde->attr&RS_ATTR_DIR &&
	      *tmpde->name!='.') {
	  tmpde->next=res;
	  res=tmpde;
	  tmpde->full_name=DirNameAbs(tmpde->name);
	  if (Cd(tmpde->name)) {
	    tmpde->sub=ISO1FilesFind(files_find_mask,fuf_flags,tmpde);
	    Cd("..");
	  }
	} else {
	  tmpde->full_name=FileNameAbs(tmpde->name);
	  if ((tmpde->attr & RS_ATTR_DIR ||
		!Bt(&fuf_flags,FUf_JUST_DIRS)) &&
		!(Bt(&fuf_flags,FUf_RECURSE) &&
		*tmpde->name=='.' && tmpde->attr&RS_ATTR_DIR) &&
		FilesFindMatch(tmpde->full_name,files_find_mask,fuf_flags)) {
	    tmpde->next=res;
	    res=tmpde;
	  } else
	    DirEntryDel(tmpde);
	}
      } else
	DirEntryDel(tmpde);
      i-=buf->len;
      buf(U8 *)+=buf->len;
    }
  }
  Free(buf2);
  return res;
}
