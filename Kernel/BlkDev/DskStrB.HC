Bool CFileNameTo(U8 *dst,U8 *src)
{
  MemSet(dst,0,CDIR_FILENAME_LEN);
  if (!FileNameChk(src))
    return FALSE;
  StrCpy(dst,src);
  return TRUE;
}

I64 Name2DirClus(CDrv *dv,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_clus;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    cur_dir_clus=dv->root_clus;
    while (*buf && cont) {
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (dv->fs_type) {
	  case FSt_REDSEA:
	    cont=RedSeaFileFind(dv,cur_dir_clus,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case FSt_FAT32:
	    cont=FAT32FileFind(dv,cur_dir_clus,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    throw('Drv');
	}
	cur_dir_clus=de.clus;
      }
    }
    if (!cont) {
      PrintErr("File not found: \"%s\".\n",dirname);
      cur_dir_clus=0;
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  Free(buf);
  Free(buf2);
  return cur_dir_clus;
}

I64 Name2ParentDirClus(CDrv *dv,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_clus,cur_dir_clus2;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    cur_dir_clus=cur_dir_clus2=dv->root_clus;
    while (*buf && cont) {
      cur_dir_clus2=cur_dir_clus;
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (dv->fs_type) {
	  case FSt_REDSEA:
	    cont=RedSeaFileFind(dv,cur_dir_clus,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case FSt_FAT32:
	    cont=FAT32FileFind(dv,cur_dir_clus,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    throw('Drv');
	}
	cur_dir_clus=de.clus;
      }
    }
    if (!cont) {
      PrintErr("File not found: \"%s\".\n",dirname);
      cur_dir_clus2=0;
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  Free(buf);
  Free(buf2);
  return cur_dir_clus2;
}
