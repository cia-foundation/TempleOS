U8 *FileExtDot(U8 *src)
{
  U8 ch;
  while (ch=*src++)
    if (ch=='.' && *src!='/' && *src!='.')
      return src-1;
  return NULL;
}

U8 *FileExtRem(U8 *src,U8 *dst=NULL)
{
  U8 *ptr;
  if (ptr=FileExtDot(src)) {
    if (dst)
      StrCpy(dst,ptr+1);
    *ptr=0;
  } else if (dst)
    *dst=0;
  return dst;
}

Bool IsDotZ(U8 *filename)
{
  I64 i=StrLen(filename);
  if (i>6 && filename[i-1]=='Z' && filename[i-2]=='.' && filename[i-6]=='.')
    return TRUE;
  else
    return FALSE;
}

Bool IsDotC(U8 *filename)
{
  I64 i=StrLen(filename);
  if (i>6 && filename[i-1]=='C' && filename[i-2]=='.' && filename[i-6]=='.')
    return TRUE;
  else
    return FALSE;
}

Bool IsTextFile(U8 *filename)
{
  return FilesFindMatch(TEXT_FILE_MASK,filename);
}

Bool IsSrcFile(U8 *filename)
{
  return FilesFindMatch(SRC_FILE_MASK,filename);
}

Bool FilesFindMatch(U8 *files_find_mask,U8 *filename,I64 fuf_flags=0)
{
  U8 *st1=StrNew(files_find_mask),
     *st2=MAlloc(StrLen(files_find_mask)+1);
  Bool result=FALSE;
  while (TRUE) {
    StrFirstRem(st1,";",st2);
    if (*st2) {
      if (*st2=='~') {
	if (WildCardMatch(st2+1,filename)) {
	  result=FALSE;
	  break;
	}
      } else {
	if (WildCardMatch(st2,filename)) {
	  if (Bt(&fuf_flags,FUf_JUST_TEXT_FILES) && !IsTextFile(filename)) {
	    result=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_SRC_FILES) && !IsSrcFile(filename)) {
	    result=FALSE;
	    break;
	  } else
	    result=TRUE;
	}
      }
    } else
      break;
  }
  Free(st1);
  Free(st2);
  return result;
}

U0 PrtChk(CPrt *p)
{
  if (!p || p->pt_signature!=PRT_SIGNATURE_VAL)
    throw(EXCEPT_PRT,4);
}

U8 Prt2Drv(CPrt *p=NULL)
{
  if (!p)
    p=Fs->cur_prt;
  PrtChk(p);
  return p->drv_let;
}

CPrt *Drv2Prt(U8 drv_let=0)
{
  CPrt *p;
  if (!drv_let)
    p=Fs->cur_prt;
  else if (drv_let==':')
    p=dft_prt;
  else
    p=drv_to_prt[ToUpper(drv_let)-'A'];
  PrtChk(p);
  return p;
}

U0 BlkDevChk(CBlkDev *bdev)
{
  if (!bdev || bdev->bd_signature!=BD_SIGNATURE_VAL)
    throw(EXCEPT_BLKDEV,1);
  if (bdev->type<=BDT_NULL || bdev->type>BDT_NON_NATIVE_FILE)
    throw(EXCEPT_BLKDEV,2);
}

CBlkDev *Drv2BlkDev(U8 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let);
  CBlkDev *bdev=p->bdev;
  BlkDevChk(bdev);
  return bdev;
}

CBlkDev *Drv2BlkDevFwded(U8 drv_let=0)
{
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  while (bdev && bdev->non_native_drv)
    bdev=Drv2BlkDev(bdev->non_native_drv);
  return bdev;
}

Bool IsDrvWritable(U8 drv_let=0)
{
  CBlkDev *bdev=Drv2BlkDev(drv_let);
  if (bdev->flags & BDF_READ_ONLY)
    return FALSE;
  else
    return TRUE;
}

U8 *CurDir(CTask *task=NULL,CTask *mem_task=NULL)
{
  U8 *st;
  if (!task) task=Fs;
  if (!task->cur_dir)
    return NULL;
  st=MAlloc(StrLen(task->cur_dir)+3,mem_task);
  *st=Prt2Drv;
  st[1]=':';
  StrCpy(st+2,task->cur_dir);
  return st;
}

U8 *DirNameAbs(U8 *_dirname)
{
  I64 maxlen;
  U8 drv[3],*cur_dir,*buf2,*buf3,*buf,*dirname,*temp_name;
  if (!Fs->cur_dir || !*Fs->cur_dir)
    return StrNew(_dirname);
  dirname=MStrUtil(_dirname,
    SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  temp_name=dirname;
  *drv=Prt2Drv;
  drv[1]=':';
  drv[2]=0;
  if (*dirname && dirname[1]==':') {
    if (*dirname==':')
      *drv=dft_drv;
    else
      *drv=*dirname;
    dirname=dirname+2;
    cur_dir=StrNew("/");
  } else
    cur_dir=StrNew(Fs->cur_dir);
  if (*dirname=='/') {
    Free(cur_dir);
    cur_dir=StrNew("/");
    dirname++;
  }

  buf2=StrNew(dirname);
  maxlen=StrLen(cur_dir)+1+StrLen(buf2)+1;
  buf3=MAlloc(maxlen);
  buf =MAlloc(maxlen);
  StrCpy(buf3,cur_dir);
  while (*buf2) {
    StrFirstRem(buf2,"/",buf);
    if (!*buf)
      StrCpy(buf3,"/");
    else if (!StrCmp(buf,"..")) {
      StrLastRem(buf3,"/");
      if (!*buf3)
	StrCpy(buf3,"/");
    } else if (!StrCmp(buf,".")) {
      ;
    } else if (*buf) {
      if (!StrCmp(buf,"HOME")) {
	Free(buf3);
	buf3=MAlloc(maxlen+StrLen(sys_acct));
	StrCpy(buf3,sys_acct+2);
	*drv=*sys_acct;
      } else {
	if (StrCmp(buf3,"/"))
	  StrCat(buf3,"/");
	StrCat(buf3,buf);
      }
    }
  }
  Free(cur_dir);
  cur_dir=MAlloc(StrLen(buf3)+3);
  StrCpy(cur_dir,drv);
  StrCpy(cur_dir+2,buf3);
  Free(buf);
  Free(buf2);
  Free(buf3);
  Free(temp_name);
  return cur_dir;
}

U8 *FileNameAbs(U8 *_filename)
{
  U8 *buf,*buf2,*buf3,*filename,*temp_name,*temp_buf;
  filename=MStrUtil(_filename,
    SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  temp_name=filename;
  buf=StrNew(filename);
  temp_buf=buf;
  if (*buf && buf[1]==':') {
    buf+=2;
    filename+=2;
  }
  buf2=MAlloc(StrLen(temp_name)+1);
  StrLastRem(buf,"/",buf2);
  if (*filename=='/' && !*buf)
    StrCpy(buf,"/");
  buf3=DirNameAbs(temp_buf);
  Free(temp_buf);
  buf=MAlloc(StrLen(buf3)+1+StrLen(buf2)+1);
  StrCpy(buf,buf3);
  if (buf[StrLen(buf)-1]!='/')
    StrCat(buf,"/");
  StrCat(buf,buf2);
  Free(buf2);
  Free(buf3);
  Free(temp_name);
  return buf;
}

U8 *ChgExt(U8 *filename,U8 *extension)
{//Change file name extension
  U8 *result=MAlloc(StrLen(filename)+1+StrLen(extension)+1);
  StrCpy(result,filename);
  if (FileExtDot(filename))
    FileExtRem(result);
  StrCat(result,".");
  StrCat(result,extension);
  return result;
}

U8 *DftExt(U8 *filename,U8 *extension)
{//Give extension if has none.
  U8 *result=MAlloc(StrLen(filename)+1+StrLen(extension)+1);
  StrCpy(result,filename);
  if (!FileExtDot(filename)) {
    StrCat(result,".");
    StrCat(result,extension);
  }
  return result;
}

CDirEntry *CdDir(CDirEntry *tempm,U8 *abs_name)
{
  I64 i;
  while (tempm) {
    i=StrLen(tempm->full_name);
    if (StrNCmp(tempm->full_name,abs_name,i))
      tempm=tempm->next;
    else
      if (StrLen(abs_name)==i)
	return tempm;
      else
	return CdDir(tempm->sub,abs_name);
  }
  return NULL;
}

I64 FileAttr(U8 *name,I64 attr=0)
{
  if (IsDotZ(name))
    attr|=_ATTR_COMPRESSED;
  else
    attr&=~_ATTR_COMPRESSED;
  if (IsDotC(name))
    attr|=_ATTR_CONTIGUOUS;
  return attr;
}

Bool FileNameChk(U8 *filename)
{
  U8 *ptr=filename;
  if (!filename) return FALSE;
  if (!*ptr) return FALSE;
  if (*ptr=='.') {
    if (!ptr[1]) return TRUE;
    if (ptr[1]=='.' && !ptr[2]) return TRUE;
  }
  if (StrLen(filename)>=CDIR_FILENAME_LEN) return FALSE;
  while (*ptr)
    if (!Bt(filename_chars_bitmap,*ptr++))
      return FALSE;
  return TRUE;
}

U8 *DirFile(U8 *dirname,U8 *name=NULL,U8 *_extension=NULL)
{/*
Strips file from dirname, scans for file upward until found or returns default.

("/SparrowOS/OSMain/Hash1a.CPP.Z",NULL,NULL)	returns "D:/SparrowOS/OSMain"
("/SparrowOS/OSMain",NULL,"PRJ.Z")		returns "D:/SparrowOS/OSMain/OSMain.PRJ.Z"
("/SparrowOS/OSMain/Dsk",NULL,"PRJ.Z")		returns "D:/SparrowOS/OSMain/OSMain.PRJ.Z"
("/SparrowOS/Apps/MusicOrgan","Load","CPP.Z")	returns "D:/SparrowOS/Apps/MusicOrgan/Load.CPP.Z"
*/
  U8 *st=DirNameAbs(dirname),*st2,*st3,*result,*dft=NULL,*ext;
  if (_extension && *_extension) {
    if (*_extension=='.')
      ext=StrNew(_extension);
    else
      ext=MSPrintF(".%s",_extension);
  } else
    ext=StrNew("");
  while (StrOcc(st,'/')&&!IsDir(st))
    StrLastRem(st,"/");
  while (StrOcc(st,'/')) {
    st2=StrNew(st);
    st3=StrNew(st);
    StrLastRem(st2,"/",st3);
 
    if (name)
      result=MSPrintF("%s/%s%s",st,name,ext);
    else {
      if (*ext)
	result=MSPrintF("%s/%s%s",st,st3,ext);
      else
	result=StrNew(st);
    }
    if (!dft)
      dft=StrNew(result);
    if (!*ext&&(!name||!*name)||FileFind(result)) {
      Free(st3);
      Free(st2);
      Free(st);
      Free(dft);
      Free(ext);
      return result;
    }
    Free(st);
    st=st2;
    Free(st3);
  }
  Free(st);
  Free(ext);
  return dft;
}

U8 *ZorNotZ(U8 *name)
{ //Compressed file name unless only uncompressed is available
  I64 i;
  U8 *result=StrNew(name);
  if (!*result||FileFind(result)||!IsDotZ(name))
    return result;
  i=StrLen(name)-2;
  result[i]=0;
  if (FileFind(result))
    return result;
  result[i]='.';
  return result;
}
