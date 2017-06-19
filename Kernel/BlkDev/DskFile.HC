U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL)
{//Read whole file from disk.
  CHashGeneric *tmph;
  U8 *absname,*altname,*curname,*res=NULL;
  I64 i,size=0,attr=0;
  CDirContext *dirc;
  CArcCompress *arc;
  absname=FileNameAbs(filename);
  altname=ToggleZorNotZ(absname);
  if ((tmph=HashFind(absname,adam_task->hash_table,HTT_FILE))||
	(tmph=HashFind(altname,adam_task->hash_table,HTT_FILE))) {
    size=tmph->user_data1;
    res=MAlloc(size+1);
    MemCpy(res,tmph->user_data0,size);
    res[size]=0; //Terminate
    attr=FileAttr(tmph->str,attr);
  } else {
    for (i=0;i<2 && !res;i++) {//Try name, then altname
      if (!i)
	curname=absname;
      else
	curname=altname;
      if (dirc=DirContextNew(curname)) {
	switch (dirc->dv->fs_type) {
	  case FSt_REDSEA:
	    res=RedSeaFileRead(dirc->dv,Fs->cur_dir,dirc->mask,&size,&attr);
	    break;
	  case FSt_FAT32:
	    res=FAT32FileRead(dirc->dv,Fs->cur_dir,dirc->mask,&size,&attr);
	    break;
	  default:
	    PrintErr("File System Not Supported\n");
	}
	DirContextDel(dirc);
      }
    }

    //Search parent directories.
    for (i=0;i<2 && !res;i++) {//Try name, then altname
      if (!i)
	curname=absname;
      else
	curname=altname;
      if (dirc=DirContextNew(curname)) {
	while (!res && StrCmp(Fs->cur_dir,"/")) {
	  Cd("..");
	  switch (Fs->cur_dv->fs_type) {
	    case FSt_REDSEA:
	      res=RedSeaFileRead(dirc->dv,Fs->cur_dir,dirc->mask,&size,&attr);
	      break;
	    case FSt_FAT32:
	      res=FAT32FileRead(dirc->dv,Fs->cur_dir,dirc->mask,&size,&attr);
	      break;
	    default:
	      PrintErr("File System Not Supported\n");
	  }
	}
	DirContextDel(dirc);
      }
    }
    if (!res)
      PrintErr("File not found: \"%s\".\n",filename);
    if (res && attr & RS_ATTR_RESIDENT)
      HashGenericAdd(curname,HTT_FILE,AMAllocIdent(res),size,0,adam_task);
  }
  if (res && attr & RS_ATTR_COMPRESSED) {
    arc=res;
    size=arc->expanded_size;
    res=ExpandBuf(arc);
    Free(arc);
  }
  if (_attr) *_attr=attr;
  if (_size) *_size=size;
  Free(absname);
  Free(altname);
  return res;
}

I64 FileWrite(U8 *filename,U8 *fbuf,I64 size,CDate cdt=0,I64 attr=0)
{//Write whole file to disk.
  I64 c=0;
  CHashGeneric *tmph;
  CDirContext *dirc;
  U8 *fbuf2,*absname=FileNameAbs(filename);
  if (dirc=DirContextNew(filename,FALSE,TRUE)) {
    attr=FileAttr(dirc->mask,attr);
    if (attr&RS_ATTR_COMPRESSED) {
      fbuf=CompressBuf(fbuf,size);
      size=fbuf(CArcCompress *)->compressed_size;
      fbuf2=fbuf;
    } else
      fbuf2=NULL;
    if (!cdt) cdt=Now;
    switch (dirc->dv->fs_type) {
      case FSt_REDSEA:
	c=RedSeaFileWrite(dirc->dv,Fs->cur_dir,dirc->mask,fbuf,size,cdt,attr);
	break;
      case FSt_FAT32:
	c=FAT32FileWrite(dirc->dv,Fs->cur_dir,dirc->mask,fbuf,size,cdt,attr);
	break;
      default:
	PrintErr("File System Not Supported\n");
    }
    if (tmph=HashFind(absname,adam_task->hash_table,HTT_FILE)) {
      if (attr & RS_ATTR_RESIDENT) {
	Free(tmph->user_data0);
	tmph->user_data0=AMAllocIdent(fbuf);
	tmph->user_data1=size;
      } else
	HashRemDel(tmph,adam_task->hash_table);
    } else if (attr & RS_ATTR_RESIDENT)
      HashGenericAdd(absname,HTT_FILE,AMAllocIdent(fbuf),size,0,adam_task);
    Free(fbuf2);
    DirContextDel(dirc);
  }
  Free(absname);
  return c;
}
