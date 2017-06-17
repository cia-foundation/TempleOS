Bool CopySingleZ(U8 *f1,U8 *f2) //Just one file
{
  U8 *file_buf=NULL;
  I64 size,attr=0,c;
  CDirEntry de;
  if (FileFind(f1,&de,FUF_JUST_FILES)) {
    Free(de.full_name);
    file_buf=FileRead(f1,&size,&attr);
    attr=FileAttr(f2,attr);
    if (file_buf) {
      "Copying %s to %s\n",f1,f2;
      c=FileWrite(f2,file_buf,size,de.datetime,attr);
      Free(file_buf);
      return ToBool(c);
    } else
      PrintErr("File not found: \"%s\".\n",f1);
  } else
    PrintErr("File not found: \"%s\".\n",f1);
  return FALSE;
}

#define COPY_BUF_BLKS	0x80
Bool CopySingle(U8 *f1,U8 *f2) //Just one file
{
  U8 *absf1=FileNameAbs(f1),*absf2=FileNameAbs(f2),*buf;
  I64 cnt,n,size,attr1=FileAttr(f1),attr2=FileAttr(f2),i,j;
  CFile *in_file=NULL,*out_file=NULL;
  if (!StrCmp(absf1,absf2)) {//onto self?
    Free(absf1);
    Free(absf2);
    return FALSE;
  }
  Free(absf1);
  Free(absf2);
  if (attr1!=attr2)
    return CopySingleZ(f1,f2);
  buf=MAlloc(COPY_BUF_BLKS<<BLK_SIZE_BITS);
  if (attr1 & RS_ATTR_CONTIGUOUS)
    in_file=FOpen(f1,"rc");
  else
    in_file=FOpen(f1,"r");
  if (in_file) {
    size=FSize(in_file);
    cnt=(size+BLK_SIZE-1)>>BLK_SIZE_BITS;
    if (attr2 & RS_ATTR_CONTIGUOUS)
      out_file=FOpen(f2,"wc",cnt);
    else
      out_file=FOpen(f2,"w",cnt);
    if (out_file) {
      "Copying %s to %s\n",f1,f2;
      j=size;
      while (cnt>0) {
	if (cnt>COPY_BUF_BLKS) {
	  n=COPY_BUF_BLKS;
	  i=n<<BLK_SIZE_BITS;
	} else {
	  n=cnt;
	  i=j;
	}
	FBlkRead(in_file, buf,FFB_NEXT_BLK,n);
	FBlkWrite(out_file,buf,FFB_NEXT_BLK,n);
	cnt-=n;
	j-=n<<BLK_SIZE_BITS;
      }
      out_file->flags|=FF_USE_OLD_DATETIME;
      out_file->de.datetime=in_file->de.datetime;
      out_file->de.size=size;
      out_file->de.attr=FileAttr(f2,in_file->de.attr);
      FClose(out_file);
      FClose(in_file);
      Free(buf);
      return TRUE;
    } else
      PrintErr("File not found: \"%s\".\n",f2);
    FClose(in_file);
  } else
    PrintErr("File not found: \"%s\".\n",f1);
  Free(buf);
  return FALSE;
}

I64 Del(U8 *files_find_mask,Bool make_mask=FALSE,
	Bool del_dir=FALSE,Bool print_msg=TRUE)
{//Delete files.
  I64 res=0;
  CDirContext *dirc;
  if (dirc=DirContextNew(files_find_mask,make_mask)) {
    switch (dirc->dv->fs_type) {
      case FSt_REDSEA:
	res=RedSeaFilesDel(dirc->dv,Fs->cur_dir,dirc->mask,
	      0,del_dir,print_msg);
	break;
      case FSt_FAT32:
	res=FAT32FilesDel(dirc->dv,Fs->cur_dir,dirc->mask,
	      0,del_dir,print_msg);
	break;
      default:
	PrintErr("File System Not Supported\n");
    }
    DirContextDel(dirc);
  }
  return res;
}
