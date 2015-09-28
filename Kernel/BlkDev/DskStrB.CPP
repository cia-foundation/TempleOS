Bool CFileNameTo(U8 *dst,U8 *src)
{
  MemSet(dst,0,CDIR_FILENAME_LEN);
  if (!FileNameChk(src))
    return FALSE;
  StrCpy(dst,src);
  return TRUE;
}

I64 Name2DirCluster(CDrv *dv,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_cluster;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    cur_dir_cluster=dv->root_cluster;
    while (*buf && cont) {
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (dv->type) {
	  case DVT_REDSEA:
	    cont=RedSeaFileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_FAT32:
	    cont=FAT32FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_ISO9660:
	    cont=ISO1FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    throw('Drv');
	}
	cur_dir_cluster=de.cluster;
      }
    }
    if (!cont) {
      "%s ",dirname;
      PrintErr("File not found.\n");
      cur_dir_cluster=0;
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  Free(buf);
  Free(buf2);
  return cur_dir_cluster;
}

I64 Name2ParentDirCluster(CDrv *dv,U8 *dirname)
{
  Bool cont=TRUE,unlock;
  I64 cur_dir_cluster,cur_dir_cluster2;
  CDirEntry de;
  U8 *buf=StrNew(dirname),*buf2=StrNew(dirname);
  DrvChk(dv);
  try {
    unlock=DrvLock(dv);
    cur_dir_cluster=cur_dir_cluster2=dv->root_cluster;
    while (*buf && cont) {
      cur_dir_cluster2=cur_dir_cluster;
      StrFirstRem(buf,"/",buf2);
      if (*buf2) {
	switch (dv->type) {
	  case DVT_REDSEA:
	    cont=RedSeaFileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_FAT32:
	    cont=FAT32FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  case DVT_ISO9660:
	    cont=ISO1FileFind(dv,cur_dir_cluster,buf2,&de,FUF_JUST_DIRS);
	    break;
	  default:
	    throw('Drv');
	}
	cur_dir_cluster=de.cluster;
      }
    }
    if (!cont) {
      "%s ",dirname;
      PrintErr("File not found.\n");
      cur_dir_cluster2=0;
    }
    if (unlock)
      DrvUnlock(dv);
  } catch
    if (unlock)
      DrvUnlock(dv);
  Free(buf);
  Free(buf2);
  return cur_dir_cluster2;
}
