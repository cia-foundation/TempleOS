Bool MapDrv(U8 drv_let,CDrv *dv)
{//Make drive letter map to another.
  drv_let=ToUpper(drv_let);
  if ('A'<=drv_let<='Z') {
    dsk.let_to_drv[drv_let-'A']=dv;
    dv->drv_let=drv_let;
    return TRUE;
  } else
    return FALSE;
}

Bool Drv(U8 drv_let)
{//Set drive.  You can set drive with $LK,"Cd",A="MN:Cd"$() as well.
  CDrv *dv=Let2Drv(drv_let);
  CBlkDev *bd=dv->bd;
  BlkDevChk(bd);
  if (dv!=Fs->cur_dv) {
    if (bd->flags & BDF_REMOVABLE && !(bd->flags & BDF_INITIALIZED))
      ChgDsk(Drv2Let(dv));
    if (bd->type==BDT_RAM)
      BlkDevInit(bd);
  }
  Fs->cur_dv=dv;
  Free(Fs->cur_dir);
  Fs->cur_dir=StrNew("/");
  switch (dv->type) {
    case DVT_REDSEA:
    case DVT_FAT32:
    case DVT_ISO9660:
      return TRUE;
    default:
      PrintErr("File System Not Supported\n");
      return FALSE;
  }
}

Bool Cd(U8 *dirname=NULL,Bool make_dirs=FALSE)
{//Change directory. Optionally, make directories, too.
  I64 maxlen,cur_dir_cluster=0;
  U8 *chg_to_buf,*new_cur_dir,*buf;
  CDrv *dv;
  Bool result=TRUE;
  if (!dirname)
    dirname="::/Home";
  else if (!*dirname)
    return TRUE;
  if (dirname[1]==':') {
    if (*dirname==':') {
      if (Fs->cur_dv!=Let2Drv(':') && !Drv(*dirname))
	return FALSE;
    } else if (Fs->cur_dv!=Let2Drv(*dirname) && !Drv(*dirname))
      return FALSE;
    dirname+=2;
  }
  if (*dirname=='/' || !*dirname || !Fs->cur_dir) {
    Free(Fs->cur_dir);
    Fs->cur_dir=StrNew("/");
    if (*dirname=='/')
      dirname++;
  }
  chg_to_buf=MStrUtil(dirname,SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  maxlen=StrLen(Fs->cur_dir)+1+StrLen(chg_to_buf)+1;
  new_cur_dir=MAlloc(maxlen);
  buf=MAlloc(maxlen);
  StrCpy(new_cur_dir,Fs->cur_dir);
  while (*chg_to_buf && result) {
    StrFirstRem(chg_to_buf,"/",buf);
    if (!*buf)
      StrCpy(new_cur_dir,"/");
    else if (!StrCmp(buf,"..")) {
      StrLastRem(new_cur_dir,"/");
      if (!*new_cur_dir)
	StrCpy(new_cur_dir,"/");
    } else if (StrCmp(buf,".") && *buf) {
      dv=Fs->cur_dv;
      cur_dir_cluster=Name2DirCluster(dv,new_cur_dir);
      switch (dv->type) {
	case DVT_REDSEA:
	  result=RedSeaCd(buf,cur_dir_cluster);
	  break;
	case DVT_FAT32:
	  result=FAT32Cd(buf,cur_dir_cluster);
	  break;
	case DVT_ISO9660:
	  result=ISO1Cd(buf,cur_dir_cluster);
	  break;
	default:
	  PrintErr("File System Not Supported\n");
	  result=FALSE;
      }
      if (!result && make_dirs) {
	Free(Fs->cur_dir);
	Fs->cur_dir=StrNew(new_cur_dir);
	result=MkDir(buf);
      }
      if (result) {
	if (StrCmp(new_cur_dir,"/"))
	  CatPrint(new_cur_dir,"/");
	CatPrint(new_cur_dir,buf);
      }
    }
  }
  Free(Fs->cur_dir);
  Fs->cur_dir=StrNew(new_cur_dir);
  Free(buf);
  Free(chg_to_buf);
  Free(new_cur_dir);
  return result;
}

U0 FileAccessDel(CFileAccess *fa)
{//Change back to old cur_dir and drv.
  CBlkDev *bd;
  if (!fa) return;
  bd=fa->old_dv->bd;
  if (!(bd->flags & BDF_INIT_IN_PROGRESS)) {
    if (fa->old_dir) {
      Drv(Drv2Let(fa->old_dv));
      Cd(fa->old_dir);
    }
  } else {
    Fs->cur_dv=fa->old_dv;
    Free(Fs->cur_dir);
    Fs->cur_dir=StrNew("/");
  }
  Free(fa->old_dir);
  Free(fa->mask);
  Free(fa);
}

CFileAccess *FileAccessNew(U8 *_mask,Bool make_mask=FALSE,Bool make_dirs=FALSE,Bool no_mask=FALSE)
{//Save cur_dir and drv. Change to new dir.
  Bool valid=TRUE,old_silent;
  U8 *buf,*mask,*temp_mask;
  CFileAccess *fa=CAlloc(sizeof(CFileAccess));
  mask=MStrUtil(_mask,SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
  temp_mask=mask;
  fa->old_dir=StrNew(Fs->cur_dir);
  fa->old_dv=Fs->cur_dv;
  if (*mask && mask[1]==':') {
    if (Fs->cur_dv!=Let2Drv(*mask) && !Drv(*mask))
      valid=FALSE;
    mask+=2;
  }
  fa->dv=Fs->cur_dv;
  DrvChk(fa->dv);
  buf=MAlloc(StrLen(mask)+2);
  StrCpy(buf,mask);
  fa->mask=MAlloc(StrLen(mask)+2);
  if (no_mask)
    *fa->mask=0;
  else
    StrLastRem(buf,"/",fa->mask);
  if (*mask=='/' && !*buf)
    StrCpy(buf,"/");
  //If began with Dir, change to Dir.
  if (*buf && !Cd(buf,make_dirs))
    valid=FALSE;
  if (valid && make_mask) {
    if (!*fa->mask) {
      Free(fa->mask);
      fa->mask=StrNew("*");
    } else {
      if (!make_dirs || FileNameChk(fa->mask)) {
	old_silent=Silent;
	//Try mask to see if Dir. If Dir, change to dir and set to "*".
	if (Cd(fa->mask,make_dirs)) {
	  Free(fa->mask);
	  fa->mask=StrNew("*");
	}
	Silent(old_silent);
      }
    }
  }
  Free(buf);
  Free(temp_mask);
  if (!valid) {
    FileAccessDel(fa);
    fa=NULL;
  }
  return fa;
}

CDirEntry *FilesFind(U8 *files_find_mask,I64 fuf_flags=0)
{/* See $LK,"::/Doc/FileUtils.TXT"$.

Find files and make a directory tree in memory.

When done, you free with $LK,"DirEntryDel",A="MN:DirEntryDel"$(),$LK,"DirEntryDel2",A="MN:DirEntryDel2"$(),
$LK,"DirTreeDel",A="MN:DirTreeDel"$() or $LK,"DirTreeDel2",A="MN:DirTreeDel2"$().

*/
  CDirEntry *result;
  CFileAccess *fa;
  if (fuf_flags&~FUG_FILES_FIND)
    throw('FUF');
  if (fuf_flags&FUF_SINGLE) {
    result=MAlloc(sizeof(CDirEntry));
    if (!FileFind(files_find_mask,result)) {
      Free(result);
      return NULL;
    }
  } else if (fa=FileAccessNew(files_find_mask,TRUE)) {
    result=SysFilesFind(fa->mask,fuf_flags);
    FileAccessDel(fa);
  } else
    return NULL;
  return result;
}

Bool IsDir(U8 *dir_name)
{//Is a str a valid, existing Dir?
  U8 *mask=MStrPrint("%s/*",dir_name);
  Bool result,old_silent=Silent;
  CFileAccess *fa;
  if (fa=FileAccessNew(mask)) {
    FileAccessDel(fa);
    result=TRUE;
  } else
    result=FALSE;
  Free(mask);
  Silent(old_silent);
  return result;
}

I64 Dir(U8 *files_find_mask="*",Bool full=FALSE)
{//List directory.
  I64 result;
  CFileAccess *fa;
  if (fa=FileAccessNew(files_find_mask,TRUE)) {
    result=Dir2(fa->mask,0,full);
    FileAccessDel(fa);
    return result;
  } else
    return 0;
}

I64 Del(U8 *files_find_mask,Bool print_msg=TRUE)
{//Delete files.
  I64 result=0;
  CFileAccess *fa;
  if (fa=FileAccessNew(files_find_mask)) {
    switch (fa->dv->type) {
      case DVT_REDSEA:
	result=RedSeaFilesDel(fa->dv,Fs->cur_dir,fa->mask,0,FALSE,print_msg);
	break;
      case DVT_FAT32:
	result=FAT32FilesDel(fa->dv,Fs->cur_dir,fa->mask,0,FALSE,print_msg);
	break;
      case DVT_ISO9660:
	PrintErr("Not Writable\n");
	break;
      default:
	PrintErr("File System Not Supported\n");
    }
    FileAccessDel(fa);
  }
  return result;
}

U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL)
{//Read whole file from disk.
  CHashGeneric *temph;
  U8 *absname,*altname,*curname,*result=NULL;
  I64 i,size=0,attr=0;
  CFileAccess *fa;
  CArcCompress *ac;
  absname=FileNameAbs(filename);
  altname=ToggleZorNotZ(absname);
  if ((temph=HashFind(absname,adam_task->hash_table,HTT_FILE))||
       (temph=HashFind(altname,adam_task->hash_table,HTT_FILE))) {
    size=temph->user_data1;
    result=MAlloc(size+1);
    MemCpy(result,temph->user_data0,size);
    result[size]=0; //Terminate
    attr=FileAttr(temph->str,attr);
  } else {
    for (i=0;i<2 && !result;i++) {//Try name, then altname
      if (!i)
	curname=absname;
      else
	curname=altname;
      if (fa=FileAccessNew(curname)) {
	switch (fa->dv->type) {
	  case DVT_REDSEA:
	    result=RedSeaFileRead(fa->dv,Fs->cur_dir,fa->mask,&size,&attr);
	    break;
	  case DVT_FAT32:
	    result=FAT32FileRead(fa->dv,Fs->cur_dir,fa->mask,&size,&attr);
	    break;
	  case DVT_ISO9660:
	    result=ISO1FileRead(fa->dv,Fs->cur_dir,fa->mask,&size,&attr);
	    break;
	  default:
	    PrintErr("File System Not Supported\n");
	}
	FileAccessDel(fa);
      }
    }

    //Search parent directories.
    for (i=0;i<2 && !result;i++) {//Try name, then altname
      if (!i)
	curname=absname;
      else
	curname=altname;
      if (fa=FileAccessNew(curname)) {
	while (!result && StrCmp(Fs->cur_dir,"/")) {
	  Cd("..");
	  switch (Fs->cur_dv->type) {
	    case DVT_REDSEA:
	      result=RedSeaFileRead(fa->dv,Fs->cur_dir,fa->mask,&size,&attr);
	      break;
	    case DVT_FAT32:
	      result=FAT32FileRead(fa->dv,Fs->cur_dir,fa->mask,&size,&attr);
	      break;
	    case DVT_ISO9660:
	      result=ISO1FileRead(fa->dv,Fs->cur_dir,fa->mask,&size,&attr);
	      break;
	    default:
	      PrintErr("File System Not Supported\n");
	  }
	}
	FileAccessDel(fa);
      }
    }
    if (!result) {
      "%s ",filename;
      PrintErr("File not found.\n");
    }
    if (result && attr & RS_ATTR_RESIDENT)
      HashGenericAdd(curname,HTT_FILE,AMAllocIdentical(result),size,0,adam_task);
  }
  if (result && attr & RS_ATTR_COMPRESSED) {
    ac=result;
    size=ac->expanded_size;
    result=ExpandBuf(ac);
    Free(ac);
  }
  if (_attr) *_attr=attr;
  if (_size) *_size=size;
  Free(absname);
  Free(altname);
  return result;
}

Bool FileFind(U8 *filename,CDirEntry *_de=NULL,I64 fuf_flags=0)
{//$LK,"FUF_JUST_DIRS",A="MN:FUF_JUST_DIRS"$, $LK,"FUF_JUST_FILES",A="MN:FUF_JUST_FILES"$, $LK,"FUF_Z_OR_NOT_Z",A="MN:FUF_Z_OR_NOT_Z"$, $LK,"FUF_SCAN_PARENTS",A="MN:FUF_SCAN_PARENTS"$
//If you pass _de, you must Free(_de->full_name);
  I64 i,j,cur_dir_cluster;
  U8 *altname,*curname,*full_name=NULL;
  CDirEntry de;
  CFileAccess *fa;
  Bool result=FALSE,old_silent;
  if (fuf_flags&~FUG_FILE_FIND)
    throw('FUF');
  if (!filename) return FALSE;
  altname=ToggleZorNotZ(filename);
  if (fuf_flags&FUF_Z_OR_NOT_Z)
    j=2;
  else
    j=1;
  for (i=0;i<j && !result;i++) {
    if (!i)
      curname=filename;
    else
      curname=altname;
    old_silent=Silent;
    if (!(fa=FileAccessNew(curname)))
      Silent(old_silent);
    else {
      Silent(old_silent);
      cur_dir_cluster=Name2DirCluster(fa->dv,Fs->cur_dir);
      switch (fa->dv->type) {
	case DVT_REDSEA:
	  result=RedSeaFileFind(fa->dv,cur_dir_cluster,fa->mask,&de,fuf_flags);
	  break;
	case DVT_FAT32:
	  result=FAT32FileFind(fa->dv,cur_dir_cluster,fa->mask,&de,fuf_flags);
	  break;
	case DVT_ISO9660:
	  result=ISO1FileFind(fa->dv,cur_dir_cluster,fa->mask,&de,fuf_flags);
	  break;
	default:
	  PrintErr("File System Not Supported\n");
      }
      if (result && _de) {
	if (StrCmp(Fs->cur_dir,"/"))
	  full_name=MStrPrint("%C:%s/%s",Drv2Let(Fs->cur_dv),Fs->cur_dir,de.name);
	else
	  full_name=MStrPrint("%C:/%s",Drv2Let(Fs->cur_dv),de.name);
      }
      FileAccessDel(fa);
    }
  }
  for (i=0;i<j && !result && fuf_flags&FUF_SCAN_PARENTS;i++) {
    if (!i)
      curname=filename;
    else
      curname=altname;
    old_silent=Silent;
    if (!(fa=FileAccessNew(curname)))
      Silent(old_silent);
    else {
      Silent(old_silent);
      cur_dir_cluster=Name2DirCluster(fa->dv,Fs->cur_dir);
      while (!result && StrCmp(Fs->cur_dir,"/")) {
	Cd("..");
	cur_dir_cluster=Name2DirCluster(fa->dv,Fs->cur_dir);
	switch (fa->dv->type) {
	   case DVT_REDSEA:
	     result=RedSeaFileFind(fa->dv,cur_dir_cluster,fa->mask,&de,fuf_flags);
	     break;
	   case DVT_FAT32:
	     result=FAT32FileFind(fa->dv,cur_dir_cluster,fa->mask,&de,fuf_flags);
	     break;
	   case DVT_ISO9660:
	     result=ISO1FileFind(fa->dv,cur_dir_cluster,fa->mask,&de,fuf_flags);
	     break;
	   default:
	     PrintErr("File System Not Supported\n");
	}
      }
      if (result && _de) {
	if (StrCmp(Fs->cur_dir,"/"))
	  full_name=MStrPrint("%C:%s/%s",Drv2Let(Fs->cur_dv),Fs->cur_dir,de.name);
	else
	  full_name=MStrPrint("%C:/%s",Drv2Let(Fs->cur_dv),de.name);
      }
      FileAccessDel(fa);
    }
  }
  if (_de) {
    if (result) {
      MemCpy(_de,&de,sizeof(CDirEntry));
      _de->full_name=full_name;
    } else
      MemSet(_de,0,sizeof(CDirEntry));
  }
  Free(altname);
  return result;
}

I64 FileWrite(U8 *filename,U8 *fbuf,
   I64 size,CDate cdt=0,I64 attr=0)
{//Write whole file to disk.
  I64 c=0;
  CHashGeneric *temph;
  CFileAccess *fa;
  U8 *fbuf2,*absname=FileNameAbs(filename);
  if (fa=FileAccessNew(filename,FALSE,TRUE)) {
    attr=FileAttr(fa->mask,attr);
    if (attr&RS_ATTR_COMPRESSED) {
      fbuf=CompressBuf(fbuf,size);
      size=fbuf(CArcCompress *)->compressed_size;
      fbuf2=fbuf;
    } else
      fbuf2=NULL;
    if (!cdt) cdt=Now;
    switch (fa->dv->type) {
      case DVT_REDSEA:
	c=RedSeaFileWrite(fa->dv,Fs->cur_dir,fa->mask,fbuf,size,cdt,attr);
	break;
      case DVT_FAT32:
	c=FAT32FileWrite(fa->dv,Fs->cur_dir,fa->mask,fbuf,size,cdt,attr);
	break;
      case DVT_ISO9660:
	PrintErr("Not Writable\n");
	break;
      default:
	PrintErr("File System Not Supported\n");
    }
    if (temph=HashFind(absname,adam_task->hash_table,HTT_FILE)) {
      if (attr & RS_ATTR_RESIDENT) {
	Free(temph->user_data0);
	temph->user_data0=AMAllocIdentical(fbuf);
	temph->user_data1=size;
      } else
	HashRemDel(temph,adam_task->hash_table);
    } else if (attr & RS_ATTR_RESIDENT)
      HashGenericAdd(absname,HTT_FILE,AMAllocIdentical(fbuf),size,0,adam_task);
    Free(fbuf2);
    FileAccessDel(fa);
  }
  Free(absname);
  return c;
}
