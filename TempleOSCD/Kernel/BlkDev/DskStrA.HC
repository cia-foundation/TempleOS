U8 *FileExtDot(U8 *src)
{//Find dot char in name.
  I64 ch;
  while (ch=*src++)
    if (ch=='.' && *src!='/' && *src!='.')
      return src-1;
  return NULL;
}

U8 *FileExtRem(U8 *src,U8 *dst=NULL)
{//Remove filename extension from str.
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
{//Does name end in .Z?
  I64 i=StrLen(filename);
  if (StrOcc(filename,'.')>1 && filename[i-1]=='Z' && filename[i-2]=='.')
    return TRUE;
  else
    return FALSE;
}

Bool IsDotC(U8 *filename)
{//Does name end in .C?
  I64 i=StrLen(filename);
  if (StrOcc(filename,'.')>1 && filename[i-1]=='C' && filename[i-2]=='.')
    return TRUE;
  else
    return FALSE;
}

Bool FilesFindMatch(U8 *_test_name,U8 *files_find_mask,I64 fuf_flags=0)
{//Does filename meet $LK,"Files Find",A="FI:::/Doc/FileUtils.DD"$ mask?
  I64 tn_len=StrLen(_test_name),mask_len=StrLen(files_find_mask);
  U8 *mask1=MAlloc(mask_len+1),*mask2=MAlloc(mask_len+1),
	*ptr,*test_name1,*test_name2;
  Bool res=FALSE;
  StrCpy(mask1,files_find_mask);
  if (StrOcc(_test_name,'/')) {
    test_name1=MAlloc(tn_len+1);
    test_name2=MAlloc(tn_len+1);
    StrCpy(test_name1,_test_name);
    StrLastRem(test_name1,"/",test_name2);
  } else {
    test_name1=NULL;
    test_name2=NULL;
  }
  while (TRUE) {
    StrFirstRem(mask1,";",mask2);
    if (!test_name2 || StrOcc(mask2,'/'))
      ptr=_test_name;
    else
      ptr=test_name2;
    if (*mask2) {
      if (*mask2=='!') {
	if (WildMatch(ptr,mask2+1)) {
	  res=FALSE;
	  break;
	}
      } else {
	if (WildMatch(ptr,mask2)) {
	  if (Bt(&fuf_flags,FUf_JUST_TXT) &&
		!FilesFindMatch(_test_name,FILEMASK_TXT)) {
	    res=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_DD) &&
		!FilesFindMatch(_test_name,FILEMASK_DD)) {
	    res=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_SRC) &&
		!FilesFindMatch(_test_name,FILEMASK_SRC)) {
	    res=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_AOT) &&
		!FilesFindMatch(_test_name,FILEMASK_AOT)) {
	    res=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_JIT) &&
		!FilesFindMatch(_test_name,FILEMASK_JIT)) {
	    res=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_GR) &&
		!FilesFindMatch(_test_name,FILEMASK_GR)) {
	    res=FALSE;
	    break;
	  } else
	    res=TRUE;
	}
      }
    } else
      break;
  }
  Free(test_name1);
  Free(test_name2);
  Free(mask1);
  Free(mask2);
  return res;
}

U8 *DirNameAbs(U8 *_dirname)
{//MAlloc absolute dir string with drv letter.
  I64 maxlen;
  U8 drv[3],*res,*buf,*buf2,*buf3,*buf4,*dirname,*free_dirname;
  if (!Fs->cur_dir || !*Fs->cur_dir)
    return StrNew(_dirname);
  free_dirname=dirname=MStrUtil(_dirname,
	SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  *drv=Drv2Let;
  drv[1]=':';
  drv[2]=0;
  if (*dirname && dirname[1]==':') {
    if (*dirname==':')
      *drv=blkdev.boot_drv_let;
    else if (*dirname=='~')
      *drv=*blkdev.home_dir;
    else
      *drv=*dirname;
    dirname=dirname+2;
    buf=StrNew("/");
  } else
    buf=StrNew(Fs->cur_dir);
  if (*dirname=='/') {
    Free(buf);
    buf=StrNew("/");
    dirname++;
  }
  buf2=StrNew(dirname);
  maxlen=StrLen(buf)+1+StrLen(buf2)+1;
  buf3=MAlloc(maxlen);
  buf4=MAlloc(maxlen);
  StrCpy(buf3,buf);
  while (*buf2) {
    StrFirstRem(buf2,"/",buf4);
    if (!*buf4)
      StrCpy(buf3,"/");
    else if (!StrCmp(buf4,"..")) {
      StrLastRem(buf3,"/");
      if (!*buf3)
	StrCpy(buf3,"/");
    } else if (!StrCmp(buf4,"~")) {
      Free(buf3);
      buf3=MAlloc(StrLen(blkdev.home_dir+2)+1+StrLen(buf2)+1);
      StrCpy(buf3,blkdev.home_dir+2);
      *drv=*blkdev.home_dir;
    } else if (!StrCmp(buf4,"."));
    else if (*buf4) {
      if (StrCmp(buf3,"/"))
	CatPrint(buf3,"/");
      CatPrint(buf3,buf4);
    }
  }
  Free(buf);
  res=MAlloc(StrLen(buf3)+3);
  StrCpy(res,drv);
  StrCpy(res+2,buf3);
  Free(buf2);
  Free(buf3);
  Free(buf4);
  Free(free_dirname);
  return res;
}

U8 *FileNameAbs(U8 *_filename,I64 fuf_flags=0)
{//Absolute filename. Accepts $LK,"FUF_Z_OR_NOT_Z",A="MN:FUF_Z_OR_NOT_Z"$, $LK,"FUF_SCAN_PARENTS",A="MN:FUF_SCAN_PARENTS"$.
  U8 *res,*filename,*buf,*buf_file,*buf_dir,*free_filename,*free_buf;
  CDirEntry de;
  free_filename=filename=MStrUtil(_filename,
	SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  free_buf=buf=StrNew(filename);
  if (*buf && buf[1]==':') {
    buf+=2;
    filename+=2;
  }
  buf_file=MAlloc(StrLen(free_filename)+1);
  StrLastRem(buf,"/",buf_file);
  if (*filename=='/' && !*buf)
    StrCpy(buf,"/");
  buf_dir=DirNameAbs(free_buf);
  Free(free_buf);
  res=MAlloc(StrLen(buf_dir)+1+StrLen(buf_file)+1);
  StrCpy(res,buf_dir);
  if (res[StrLen(res)-1]!='/')
    CatPrint(res,"/");
  CatPrint(res,buf_file);
  Free(buf_file);
  Free(buf_dir);
  Free(free_filename);
  if (fuf_flags && FileFind(res,&de,fuf_flags|FUF_JUST_FILES)) {
    Free(res);
    res=de.full_name;
  }
  return res;
}

U8 *ExtChg(U8 *filename,U8 *extension)
{//Change filename extension.
  U8 *res=MAlloc(StrLen(filename)+1+StrLen(extension)+1);
  StrCpy(res,filename);
  if (FileExtDot(filename))
    FileExtRem(res);
  return CatPrint(res,".%s",extension);
}

U8 *ExtDft(U8 *filename,U8 *extension)
{//Give extension if has none.
  U8 *res=MAlloc(StrLen(filename)+1+StrLen(extension)+1);
  StrCpy(res,filename);
  if (!FileExtDot(filename))
    CatPrint(res,".%s",extension);
  return res;
}

CDirEntry *Cd2DirEntry(CDirEntry *tmpde,U8 *abs_name)
{
  I64 i;
  while (tmpde) {
    i=StrLen(tmpde->full_name);
    if (StrNCmp(tmpde->full_name,abs_name,i)||
	  i && tmpde->full_name[i-1]!='/' && abs_name[i] && abs_name[i]!='/')
      tmpde=tmpde->next;
    else
      if (StrLen(abs_name)==i)
	return tmpde;
      else
	return Cd2DirEntry(tmpde->sub,abs_name);
  }
  return NULL;
}

I64 FileAttr(U8 *name,I64 attr=0)
{
  if (IsDotZ(name))
    attr|=RS_ATTR_COMPRESSED;
  else
    attr&=~RS_ATTR_COMPRESSED;
  if (IsDotC(name))
    attr|=RS_ATTR_CONTIGUOUS;
  return attr;
}

Bool FileNameChk(U8 *filename)
{//Return check for valid filename, not checking existence.
  U8 *ptr=filename;
  if (!filename) return FALSE;
  if (!*ptr) return FALSE;
  if (*ptr=='.') {
    if (!ptr[1]) return TRUE;
    if (ptr[1]=='.' && !ptr[2]) return TRUE;
  }
  if (StrLen(filename)>=CDIR_FILENAME_LEN) return FALSE;
  while (*ptr)
    if (!Bt(char_bmp_filename,*ptr++))
      return FALSE;
  return TRUE;
}

U8 *ToggleZorNotZ(U8 *name)
{//MAlloc name with Z if not Z or vice versa.
  U8 *res;
  if (IsDotZ(name)) {
    res=StrNew(name);
    res[StrLen(name)-2]=0;
  } else
    res=MStrPrint("%s.Z",name);
  return res;
}

U8 *FileNameTmpTxt()
{//Make pretty-safe tmp filename in home dir.
  return MStrPrint("~/SysTmp%X.DD.Z",GetTSC>>8&0xFFFFFFFF);
}

U8 *DirCur(CTask *task=NULL,CTask *mem_task=NULL)
{//MAlloc copy of cur dir with drv letter.
  U8 *st;
  if (!task) task=Fs;
  if (!task->cur_dir)
    return NULL;
  st=MAlloc(StrLen(task->cur_dir)+3,mem_task);
  *st=Drv2Let(task->cur_dv);
  st[1]=':';
  StrCpy(st+2,task->cur_dir);
  return st;
}

U8 *DirFile(U8 *dirname,U8 *name=NULL,U8 *_extension=NULL)
{/*Strips file from dirname, scans for file upward until found or
returns default.

("/Kernel/KHashA.HC.Z",NULL,NULL)	returns "D:/Kernel"
("/Kernel",NULL,"PRJ.Z")		returns "D:/Kernel/Kernel.PRJ.Z"
("/Kernel/BlkDev",NULL,"PRJ.Z")		returns "D:/Kernel/Kernel.PRJ.Z"
("/Apps/Psalmody","Load","HC.Z")	returns "D:/Apps/Psalmody/Load.HC.Z"
*/
  U8 *st=DirNameAbs(dirname),*st2,*st3,*res,*dft=NULL,*ext;
  if (_extension && *_extension) {
    if (*_extension=='.')
      ext=StrNew(_extension);
    else
      ext=MStrPrint(".%s",_extension);
  } else
    ext=StrNew("");
  while (StrOcc(st,'/')&&!IsDir(st))
    StrLastRem(st,"/");
  while (StrOcc(st,'/')) {
    st2=StrNew(st);
    st3=StrNew(st);
    StrLastRem(st2,"/",st3);

    if (name)
      res=MStrPrint("%s/%s%s",st,name,ext);
    else {
      if (*ext)
	res=MStrPrint("%s/%s%s",st,st3,ext);
      else
	res=StrNew(st);
    }
    if (!dft)
      dft=StrNew(res);
    if (!*ext && (!name||!*name) || FileFind(res)) {
      Free(st3);
      Free(st2);
      Free(st);
      Free(dft);
      Free(ext);
      return res;
    }
    Free(st);
    st=st2;
    Free(st3);
  }
  Free(st);
  Free(ext);
  return dft;
}
