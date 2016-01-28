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
  chg_to_buf=MStrUtil(dirname,
	SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
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
      switch (dv->fs_type) {
	case FSt_REDSEA:
	  result=RedSeaCd(buf,cur_dir_cluster);
	  break;
	case FSt_FAT32:
	  result=FAT32Cd(buf,cur_dir_cluster);
	  break;
	case FSt_ISO9660:
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

Bool IsDir(U8 *dir_name)
{//Is a str a valid, existing Dir?
  U8 *mask=MStrPrint("%s/*",dir_name);
  Bool result,old_silent=Silent;
  CDirContext *dirc;
  if (dirc=DirContextNew(mask)) {
    DirContextDel(dirc);
    result=TRUE;
  } else
    result=FALSE;
  Free(mask);
  Silent(old_silent);
  return result;
}

I64 Dir(U8 *files_find_mask,Bool full)
{//List directory.
  CDirEntry *tempde1=NULL,*tempde2;
  U8 *st;
  CDateStruct ds;
  I64 csize=0xFFFF,c=0xFFFF,result=0;
  tempde1=FilesFind(files_find_mask);
  if (!(st=CurDir))
    PrintErr("Invalid Drive\n");
  else {
    if (tempde1) {
//Find max columns
      tempde2=tempde1;
      while (tempde2) {
	if (tempde2->size>csize)
	  csize=tempde2->size;
	if (tempde2->cluster>c)
	  c=tempde2->cluster;
	tempde2=tempde2->next;
      }
      csize=Bsr(csize)/4+1;
      c=Bsr(c)/4+1;

      "$$MA,T=\"Directory\",LM=\"PopUpCd;Dir;\n\"$$ of %s\n",st;
      if (full)
	"__DATE__ __TIME__ %*ts %*ts\n",
	      csize,"SIZE",c,"CLUSTER";
      else
	"DATE_ TIME_ %*ts\n",csize,"SIZE";
      while (tempde1) {
	tempde2=tempde1->next;
	result++;
	if (full)
	  "%D %T %0*tX %0*tX ",tempde1->datetime,tempde1->datetime,
		csize,tempde1->size,c,tempde1->cluster;
	else {
	  Date2Struct(&ds,tempde1->datetime+local_time_offset);
	  "%02d/%02d %02d:%02d %0*tX ",ds.mon,ds.day_of_mon,ds.hour,ds.min,
		csize,tempde1->size;
	}
	if (tempde1->attr & RS_ATTR_DIR)
	  PutDirLink(tempde1->name);
	else
	  PutFileLink(tempde1->name);
	'\n';
	DirEntryDel(tempde1);
	tempde1=tempde2;
      }
    } else
      "No matching entries\n";
    Free(st);
  }
  return result;
}

Bool MkDir(U8 *filename)
{//Make directory. $LK,"Cd",A="MN:Cd"$() can also make directories.
  U8 *name;
  CDirContext *dirc;
  Bool result=FALSE;
  if (FileFind(filename,,FUF_JUST_DIRS))
    return FALSE;
  if (dirc=DirContextNew(filename)) {
    if (*dirc->mask) {
      if (!FileNameChk(dirc->mask))
	PrintErr("Invalid FileName.\n");
      else {
	"Make Directory:%s\n",filename;
	name=MStrUtil(dirc->mask,
	      SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
	switch (dirc->dv->fs_type) {
	  case FSt_REDSEA:
	    result=RedSeaMkDir(dirc->dv,Fs->cur_dir,name);
	    break;
	  case FSt_FAT32:
	    result=FAT32MkDir(dirc->dv,Fs->cur_dir,name);
	    break;
	  case FSt_ISO9660:
	    PrintErr("Not Writable\n");
	    break;
	  default:
	    PrintErr("File System Not Supported\n");
	}
	Free(name);
      }
    }
    DirContextDel(dirc);
  }
  return result;
}
