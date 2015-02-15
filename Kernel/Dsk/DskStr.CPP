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
  if (i>6 && filename[i-1]=='Z' && filename[i-2]=='.' && filename[i-6]=='.')
    return TRUE;
  else
    return FALSE;
}

Bool IsDotC(U8 *filename)
{//Does name end in .C?
  I64 i=StrLen(filename);
  if (i>6 && filename[i-1]=='C' && filename[i-2]=='.' && filename[i-6]=='.')
    return TRUE;
  else
    return FALSE;
}

Bool FilesFindMatch(U8 *test_filename,U8 *files_find_mask,I64 fuf_flags=0)
{//Does filename meet $LK,"Files Find",A="FI:::/Doc/FileUtils.TXT"$ mask?
  U8 *st1=StrNew(files_find_mask),
     *st2=MAlloc(StrLen(files_find_mask)+1);
  Bool result=FALSE;
  while (TRUE) {
    StrFirstRem(st1,";",st2);
    if (*st2) {
      if (*st2=='~') {
	if (WildMatch(test_filename,st2+1)) {
	  result=FALSE;
	  break;
	}
      } else {
	if (WildMatch(test_filename,st2)) {
	  if (Bt(&fuf_flags,FUf_JUST_TXT) && !FilesFindMatch(test_filename,FILEMASK_TXT)) {
	    result=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_SRC) && !FilesFindMatch(test_filename,FILEMASK_SRC)) {
	    result=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_AOT) && !FilesFindMatch(test_filename,FILEMASK_AOT)) {
	    result=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_JIT) && !FilesFindMatch(test_filename,FILEMASK_JIT)) {
	    result=FALSE;
	    break;
	  } else if (Bt(&fuf_flags,FUf_JUST_GRA) && !FilesFindMatch(test_filename,FILEMASK_GRA)) {
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

U0 DrvChk(CDrv *dv)
{//Check for valid drv. Throw exception.
  if (!dv || dv->dv_signature!=DRV_SIGNATURE_VAL)
    throw('Drv');
}

U8 Drv2Let(CDrv *dv=NULL)
{//Drv ptr to Drv letter.
  if (!dv)
    dv=Fs->cur_dv;
  DrvChk(dv);
  return dv->drv_let;
}

CDrv *Let2Drv(U8 drv_let=0)
{//Drv letter to Drv ptr.
  CDrv *dv;
  if (!drv_let)
    dv=Fs->cur_dv;
  else {
    if (drv_let==':')
      drv_let=dsk.boot_drv_let;
    drv_let=ToUpper(drv_let);
    if (!('A'<=drv_let<='Z'))
      throw('Drv');
    dv=dsk.let_to_drv[drv_let-'A'];
  }
  DrvChk(dv);
  return dv;
}

U0 BlkDevChk(CBlkDev *bd)
{//Check for valid BlkDev. Throw exception.
  if (!bd || bd->bd_signature!=BD_SIGNATURE_VAL ||
      !(BDT_NULL<bd->type<BDT_NUM_TYPES))
    throw('BlkDev');
}

CBlkDev *Let2BlkDev(U8 drv_let=0)
{//Drv letter to BlkDev ptr.
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=dv->bd;
  BlkDevChk(bd);
  return bd;
}

Bool IsDrvWritable(U8 drv_let=0)
{//Is drive writable?
  CBlkDev *bd=Let2BlkDev(drv_let);
  if (bd->flags & BDF_READ_ONLY)
    return FALSE;
  else
    return TRUE;
}

U8 *CurDir(CTask *task=NULL,CTask *mem_task=NULL)
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

U8 *DirNameAbs(U8 *_dirname)
{//MAlloc absolute dir string with drv letter.
  I64 maxlen;
  U8 drv[3],*result,*buf,*buf2,*buf3,*buf4,*dirname,*free_dirname;
  if (!Fs->cur_dir || !*Fs->cur_dir)
    return StrNew(_dirname);
  free_dirname=dirname=MStrUtil(_dirname,
	SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  *drv=Drv2Let;
  drv[1]=':';
  drv[2]=0;
  if (*dirname && dirname[1]==':') {
    if (*dirname==':')
      *drv=dsk.boot_drv_let;
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
    } else if (!StrCmp(buf4,".")) {
      ;
    } else if (*buf4) {
      if (StrCmp(buf3,"/"))
	CatPrint(buf3,"/");
      CatPrint(buf3,buf4);
    }
  }
  Free(buf);
  result=MAlloc(StrLen(buf3)+3);
  StrCpy(result,drv);
  StrCpy(result+2,buf3);
  Free(buf2);
  Free(buf3);
  Free(buf4);
  Free(free_dirname);
  return result;
}

U8 *FileNameAbs(U8 *_filename,I64 fuf_flags=0)
{//Absolute filename. Accepts $LK,"FUF_Z_OR_NOT_Z",A="MN:FUF_Z_OR_NOT_Z"$, $LK,"FUF_SCAN_PARENTS",A="MN:FUF_SCAN_PARENTS"$.
  U8 *result,*filename,*buf,*buf_file,*buf_dir,*free_filename,*free_buf;
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
  result=MAlloc(StrLen(buf_dir)+1+StrLen(buf_file)+1);
  StrCpy(result,buf_dir);
  if (result[StrLen(result)-1]!='/')
    CatPrint(result,"/");
  CatPrint(result,buf_file);
  Free(buf_file);
  Free(buf_dir);
  Free(free_filename);
  if (fuf_flags&&FileFind(result,&de,fuf_flags|FUF_JUST_FILES)) {
    Free(result);
    result=de.full_name;
  }
  return result;
}

U8 *ChgExt(U8 *filename,U8 *extension)
{//Change filename extension.
  U8 *result=MAlloc(StrLen(filename)+1+StrLen(extension)+1);
  StrCpy(result,filename);
  if (FileExtDot(filename))
    FileExtRem(result);
  return CatPrint(result,".%s",extension);
}

U8 *DftExt(U8 *filename,U8 *extension)
{//Give extension if has none.
  U8 *result=MAlloc(StrLen(filename)+1+StrLen(extension)+1);
  StrCpy(result,filename);
  if (!FileExtDot(filename))
    CatPrint(result,".%s",extension);
  return result;
}

CDirEntry *Cd2DirEntry(CDirEntry *tempde,U8 *abs_name)
{
  I64 i;
  while (tempde) {
    i=StrLen(tempde->full_name);
    if (StrNCmp(tempde->full_name,abs_name,i)||
      i && tempde->full_name[i-1]!='/' && abs_name[i] && abs_name[i]!='/')
      tempde=tempde->next;
    else
      if (StrLen(abs_name)==i)
	return tempde;
      else
	return Cd2DirEntry(tempde->sub,abs_name);
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
    if (!Bt(chars_bitmap_filename,*ptr++))
      return FALSE;
  return TRUE;
}

U8 *DirFile(U8 *dirname,U8 *name=NULL,U8 *_extension=NULL)
{/*Strips file from dirname, scans for file upward until found or
returns default.

("/Kernel/KHashA.CPP.Z",NULL,NULL)	returns "D:/Kernel"
("/Kernel",NULL,"PRJ.Z")		returns "D:/Kernel/Kernel.PRJ.Z"
("/Kernel/Dsk",NULL,"PRJ.Z")		returns "D:/Kernel/Kernel.PRJ.Z"
("/Apps/Psalmody","Load","CPP.Z")	returns "D:/Apps/Psalmody/Load.CPP.Z"
*/
  U8 *st=DirNameAbs(dirname),*st2,*st3,*result,*dft=NULL,*ext;
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
      result=MStrPrint("%s/%s%s",st,name,ext);
    else {
      if (*ext)
	result=MStrPrint("%s/%s%s",st,st3,ext);
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

U8 *ToggleZorNotZ(U8 *name)
{//MAlloc name with Z if not Z or vice versa.
  U8 *result;
  if (IsDotZ(name)) {
    result=StrNew(name);
    result[StrLen(name)-2]=0;
  } else
    result=MStrPrint("%s.Z",name);
  return result;
}

U8 *FileNameTempTxt()
{//Make pretty-safe temp filename in home dir.
  return MStrPrint("::/Home/SysTemp%X.TXT.Z",GetTSC>>8&0xFFFFFFFF);
}
