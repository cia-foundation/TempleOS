U8 *FileRead(U8 *filename,I64 *_size=NULL,I64 *_attr=NULL)
{//Read whole file from disk.
  CHashGeneric *temph;
  U8 *absname,*altname,*curname,*result=NULL;
  I64 i,size=0,attr=0;
  CDirContext *dirc;
  CArcCompress *arc;
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
      if (dirc=DirContextNew(curname)) {
	switch (dirc->dv->type) {
	  case DVT_REDSEA:
	    result=RedSeaFileRead(dirc->dv,Fs->cur_dir,dirc->mask,
		  &size,&attr);
	    break;
	  case DVT_FAT32:
	    result=FAT32FileRead(dirc->dv,Fs->cur_dir,dirc->mask,
		  &size,&attr);
	    break;
	  case DVT_ISO9660:
	    result=ISO1FileRead(dirc->dv,Fs->cur_dir,dirc->mask,
		  &size,&attr);
	    break;
	  default:
	    PrintErr("File System Not Supported\n");
	}
	DirContextDel(dirc);
      }
    }

    //Search parent directories.
    for (i=0;i<2 && !result;i++) {//Try name, then altname
      if (!i)
	curname=absname;
      else
	curname=altname;
      if (dirc=DirContextNew(curname)) {
	while (!result && StrCmp(Fs->cur_dir,"/")) {
	  Cd("..");
	  switch (Fs->cur_dv->type) {
	    case DVT_REDSEA:
	      result=RedSeaFileRead(dirc->dv,Fs->cur_dir,dirc->mask,
		    &size,&attr);
	      break;
	    case DVT_FAT32:
	      result=FAT32FileRead(dirc->dv,Fs->cur_dir,dirc->mask,
		    &size,&attr);
	      break;
	    case DVT_ISO9660:
	      result=ISO1FileRead(dirc->dv,Fs->cur_dir,dirc->mask,
		    &size,&attr);
	      break;
	    default:
	      PrintErr("File System Not Supported\n");
	  }
	}
	DirContextDel(dirc);
      }
    }
    if (!result) {
      "%s ",filename;
      PrintErr("File not found.\n");
    }
    if (result && attr & RS_ATTR_RESIDENT)
      HashGenericAdd(curname,HTT_FILE,
	    AMAllocIdentical(result),size,0,adam_task);
  }
  if (result && attr & RS_ATTR_COMPRESSED) {
    arc=result;
    size=arc->expanded_size;
    result=ExpandBuf(arc);
    Free(arc);
  }
  if (_attr) *_attr=attr;
  if (_size) *_size=size;
  Free(absname);
  Free(altname);
  return result;
}

I64 FileWrite(U8 *filename,U8 *fbuf,
   I64 size,CDate cdt=0,I64 attr=0)
{//Write whole file to disk.
  I64 c=0;
  CHashGeneric *temph;
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
    switch (dirc->dv->type) {
      case DVT_REDSEA:
	c=RedSeaFileWrite(dirc->dv,Fs->cur_dir,dirc->mask,fbuf,
	      size,cdt,attr);
	break;
      case DVT_FAT32:
	c=FAT32FileWrite(dirc->dv,Fs->cur_dir,dirc->mask,fbuf,
	      size,cdt,attr);
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
    DirContextDel(dirc);
  }
  Free(absname);
  return c;
}
