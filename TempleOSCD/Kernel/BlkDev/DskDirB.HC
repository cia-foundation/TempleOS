U0 HomeSet(U8 *dirname)
{//Change home directory.
  dirname=DirNameAbs(dirname);
  Free(blkdev.home_dir);
  blkdev.home_dir=AStrNew(dirname);
  Free(dirname);
}

Bool Cd(U8 *dirname=NULL,Bool make_dirs=FALSE)
{//Change directory. Optionally, make directories, too.
  I64 maxlen,cur_dir_clus=0;
  U8 *chg_to_buf,*new_cur_dir,*buf;
  CDrv *dv;
  Bool res=TRUE;
  if (!dirname)
    dirname="~";
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
  while (*chg_to_buf && res) {
    StrFirstRem(chg_to_buf,"/",buf);
    if (!*buf)
      StrCpy(new_cur_dir,"/");
    else if (!StrCmp(buf,"..")) {
      StrLastRem(new_cur_dir,"/");
      if (!*new_cur_dir)
	StrCpy(new_cur_dir,"/");
    } else if (!StrCmp(buf,"~")) {
      Free(new_cur_dir);
      new_cur_dir=MAlloc(StrLen(blkdev.home_dir+2)+1+StrLen(chg_to_buf)+1);
      StrCpy(new_cur_dir,blkdev.home_dir+2);
      if (Fs->cur_dv!=Let2Drv('~') && !Drv('~'))
	return FALSE;
    } else if (StrCmp(buf,".") && *buf) {
      dv=Fs->cur_dv;
      cur_dir_clus=Name2DirClus(dv,new_cur_dir);
      switch (dv->fs_type) {
	case FSt_REDSEA:
	  res=RedSeaCd(buf,cur_dir_clus);
	  break;
	case FSt_FAT32:
	  res=FAT32Cd(buf,cur_dir_clus);
	  break;
	default:
	  PrintErr("File System Not Supported\n");
	  res=FALSE;
      }
      if (!res && make_dirs) {
	Free(Fs->cur_dir);
	Fs->cur_dir=StrNew(new_cur_dir);
	res=DirMk(buf);
      }
      if (res) {
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
  return res;
}

Bool IsDir(U8 *dir_name)
{//Is a str a valid, existing Dir?
  U8 *mask=MStrPrint("%s/*",dir_name);
  Bool res,old_silent=Silent;
  CDirContext *dirc;
  if (dirc=DirContextNew(mask)) {
    DirContextDel(dirc);
    res=TRUE;
  } else
    res=FALSE;
  Free(mask);
  Silent(old_silent);
  return res;
}

I64 Dir(U8 *files_find_mask,Bool full)
{//List directory.
  CDirEntry *tmpde1=NULL,*tmpde2;
  U8 *st;
  CDateStruct ds;
  I64 csize=0xFFFF,c=0xFFFF,res=0;
  tmpde1=FilesFind(files_find_mask);
  if (!(st=DirCur))
    PrintErr("Invalid Drive\n");
  else {
    if (tmpde1) {
      Free(st);
      st=MAllocIdent(tmpde1->full_name);
      StrLastRem(st,"/");
      if (!st[2])
	StrCpy(st+2,"/");
//Find max columns
      tmpde2=tmpde1;
      while (tmpde2) {
	if (tmpde2->size>csize)
	  csize=tmpde2->size;
	if (tmpde2->clus>c)
	  c=tmpde2->clus;
	tmpde2=tmpde2->next;
      }
      csize=Bsr(csize)/4+1;
      c=Bsr(c)/4+1;

      "$$MA,T=\"Directory\",LM=\"PopUpCd;Dir;\n\"$$ of %s\n",st;
      if (full)
	"__DATE__ __TIME__ %*ts %*ts\n",
	      csize,"SIZE",c,"BLK";
      else
	"DATE_ TIME_ %*ts\n",csize,"SIZE";
      while (tmpde1) {
	tmpde2=tmpde1->next;
	res++;
	if (full)
	  "%D %T %0*tX %0*tX ",tmpde1->datetime,tmpde1->datetime,
		csize,tmpde1->size,c,tmpde1->clus;
	else {
	  Date2Struct(&ds,tmpde1->datetime+local_time_offset);
	  "%02d/%02d %02d:%02d %0*tX ",ds.mon,ds.day_of_mon,ds.hour,ds.min,
		csize,tmpde1->size;
	}
	if (tmpde1->attr & RS_ATTR_DIR)
	  PutDirLink(tmpde1->name,tmpde1->full_name);
	else
	  PutFileLink(tmpde1->name,tmpde1->full_name);
	'\n';
	DirEntryDel(tmpde1);
	tmpde1=tmpde2;
      }
    } else
      "No matching entries\n";
    Free(st);
  }
  return res;
}

Bool DirMk(U8 *filename,I64 entry_cnt=0)
{//Make directory. $LK,"Cd",A="MN:Cd"$() can also make directories.
//entry_cnt is for preallocating dir blks, leave it zero if you like.
  U8 *name;
  CDirContext *dirc;
  Bool res=FALSE;
  if (FileFind(filename,,FUF_JUST_DIRS))
    return FALSE;
  if (dirc=DirContextNew(filename)) {
    if (*dirc->mask) {
      if (!FileNameChk(dirc->mask))
	PrintErr("Invalid FileName: \"%s\".\n",dirc->mask);
      else {
	"Make Directory:%s\n",filename;
	name=MStrUtil(dirc->mask,
	      SUF_REM_LEADING|SUF_REM_TRAILING|SUF_REM_CTRL_CHARS);
	switch (dirc->dv->fs_type) {
	  case FSt_REDSEA:
	    res=RedSeaMkDir(dirc->dv,Fs->cur_dir,name,entry_cnt);
	    break;
	  case FSt_FAT32:
	    res=FAT32MkDir(dirc->dv,Fs->cur_dir,name,entry_cnt);
	    break;
	  default:
	    PrintErr("File System Not Supported\n");
	}
	Free(name);
      }
    }
    DirContextDel(dirc);
  }
  return res;
}
