#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"
public U8 *DBlk(I64 blk,Bool write=FALSE)
{//Dump disk block. Optionally, write.
//If you set write to TRUE, the block will
  //be written when you press <ESC>.
  //See $LK,"::/Demo/Dsk/DskRaw.HC"$.
  U8 *buf=MAlloc(BLK_SIZE);

  BlkRead(Fs->cur_dv,buf,blk,1);
  DocD(buf,BLK_SIZE);
  if (write) {
    "Edit and press <ESC> to write or <SHIFT-ESC>\n";
    if (View) {
      "Write\n";
      BlkWrite(Fs->cur_dv,buf,blk,1);
    }
  }
  return buf;
}

public U8 *DClus(I64 c,Bool write=FALSE,I64 num=0)
{//Dump disk clus. Optionally, write.
//If you set write to TRUE, the clus will
  //be written when you press <ESC>.
  //See $LK,"::/Demo/Dsk/DskRaw.HC"$.
  //Do $LK,"Dir",A="MN:Dir"$("*",TRUE); to get clus numbers of files.
  U8 *buf=MAlloc(Fs->cur_dv->spc<<BLK_SIZE_BITS);
  c=ClusNumNext(Fs->cur_dv,c,num);
  ClusRead(Fs->cur_dv,buf,c,1);
  "Clus:%X\n",c;
  DocD(buf,Fs->cur_dv->spc<<BLK_SIZE_BITS);
  if (write) {
    "Edit and press <ESC> to write or <SHIFT-ESC>\n";
    if (View) {
      "Write\n";
      ClusWrite(Fs->cur_dv,buf,c,1);
    }
  }
  return buf;
}

public U8 *Dump(U8 *filename,Bool write=FALSE)
{//Dump file. Optionally, write.
//If you set write to TRUE, the file will
  //be written when you press <ESC>.
  U8 *buf;
  I64 size;
  if (buf=FileRead(filename,&size)) {
    DocD(buf,size);
    if (write) {
      "Edit and press <ESC> to write or <SHIFT-ESC>\n";
      if (View) {
	"Write\n";
	FileWrite(filename,buf,size);
      }
    }
  }
  return buf;
}

public Bool Copy(U8 *src_files_find_mask,U8 *dst_files_find_mask=".")
{//Copy files.
//If the name ends in ".Z", it will
  //be stored compressed.  If not ".Z"
  //it will be stored uncompressed.
  Bool res=TRUE;
  CDirContext *dirc;
  CDirEntry *tmpde,*tmpde1;
  U8 *st;
  if (!(tmpde1=FilesFind(src_files_find_mask,FUF_CLUS_ORDER)))
    return FALSE;
  if (IsDir(dst_files_find_mask)) {
    if (dirc=DirContextNew(dst_files_find_mask,TRUE)) {
      tmpde=tmpde1;
      while (tmpde) {
	if (!(tmpde->attr & RS_ATTR_DIR)) {
	  st=FileNameAbs(tmpde->name);
	  if (!CopySingle(tmpde->full_name,st))
	    res=FALSE;
	  Free(st);
	}
	tmpde=tmpde->next;
      }
      DirContextDel(dirc);
    }
    DirTreeDel(tmpde1);
    return res;
  } else {
    DirTreeDel(tmpde1);
    return CopySingle(src_files_find_mask,dst_files_find_mask);
  }
}

public Bool Move(U8 *f1,U8 *f2)
{//Move files from one location to another or rename.
  if (Copy(f1,f2)) {
    Del(f1);
    return TRUE;
  }
  return FALSE;
}

I64 CopyTree2(CDirEntry *tmpde,I64 src_dir_len,I64 dst_dir_len,U8 *dst_dir)
{
  U8 *st;
  I64 res=1;
  while (tmpde) {
    st=MAlloc(StrLen(tmpde->full_name)+dst_dir_len+2);
    MemCpy(st,dst_dir,dst_dir_len);
    StrCpy(st+dst_dir_len,tmpde->full_name+src_dir_len);
    if (tmpde->attr & RS_ATTR_DIR) {
      DirMk(st,LinkedLstCnt(tmpde->sub));
      res+=CopyTree2(tmpde->sub,src_dir_len,dst_dir_len,dst_dir);
    } else
      if (CopySingle(tmpde->full_name,st))
	res++;
    Free(st);
    tmpde=tmpde->next;
  }
  return res;
}
public I64 CopyTree(U8 *src_files_find_mask,U8 *dst_files_find_mask,
	Bool no_mask=TRUE)
{//Copy directory tree.
//Returns the count of copied files (not dirs).
  CDirContext *dirc;
  CDirEntry *tmpde=NULL;
  I64 res=0,i1,i2;
  U8 *st1,*st2;

  st1=DirNameAbs(src_files_find_mask);
  st2=DirNameAbs(dst_files_find_mask);
  i1=StrLen(st1);
  if (!StrNCmp(st1,st2,i1) && (st2[i1]=='/' || !st2[i1]) ) {
    Free(st1);
    Free(st2);
    return 0;
  }
  Free(st1);
  Free(st2);
  if (dirc=DirContextNew(src_files_find_mask,TRUE,,no_mask)) {
    tmpde=FilesFind(dirc->mask,FUF_RECURSE);
    st1=DirCur;
    DirContextDel(dirc);
    i1=StrLen(st1);
    if (i1==3) i1--;
    if (dirc=DirContextNew(dst_files_find_mask,TRUE,TRUE)) {
      st2=DirCur;
      i2=StrLen(st2);
      if (i2==3) i2--;
      res=CopyTree2(tmpde,i1,i2,st2);
      DirContextDel(dirc);
      Free(st2);
    }
    DirTreeDel(tmpde);
    Free(st1);
  }
  return res;
}

I64 DelTreeDirs(CDirEntry *tmpde1)
{
  I64 res=0;
  CDirEntry *tmpde2;
  while (tmpde1) {
    tmpde2=tmpde1->next;
    if (tmpde1->attr & RS_ATTR_DIR) {
      if (tmpde1->sub)
	res+=DelTreeDirs(tmpde1->sub);
      res+=Del(tmpde1->full_name,TRUE,TRUE);
    }
    DirEntryDel(tmpde1);
    tmpde1=tmpde2;
  }
  return res;
}
I64 DelTreeFiles(CDirEntry *tmpde1)
{
  I64 res=0;
  CDirEntry *tmpde2;
  while (tmpde1) {
    tmpde2=tmpde1->next;
    if (tmpde1->attr & RS_ATTR_DIR) {
      if (tmpde1->sub)
	res+=DelTreeFiles(tmpde1->sub);
    } else
      res+=Del(tmpde1->full_name,FALSE,TRUE);
    DirEntryDel(tmpde1);
    tmpde1=tmpde2;
  }
  return res;
}
public I64 DelTree(U8 *files_find_mask,U8 *fu_flags=NULL)
{//Delete directory tree.
  I64 res=0,fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (IsDir(files_find_mask)) {
    res=DelTreeDirs(FilesFind(files_find_mask,fuf_flags));
    res+=Del(files_find_mask,TRUE,TRUE);
    res+=Del(files_find_mask,FALSE,TRUE);
  } else
    res=DelTreeFiles(FilesFind(files_find_mask,fuf_flags));
  return res;
}

U0 TouchFile(U8 *filename,U8 *attr,CDate cdt=I64_MIN)
{
  CDrv *dv=Let2Drv(*filename);
  CDirEntry de;
  U8 *cur_dir=StrNew(filename),buf[STR_LEN];
  if (FileFind(filename,&de,FUF_JUST_FILES)) {
    Free(de.full_name);
    if (!StrCmp(attr,"+?"))
      "%-48ts %s\n",filename,StrPrintFlags(buf,Define("ST_FILE_ATTRS"),de.attr);
    else {
      StrFirstRem(cur_dir,":");
      StrLastRem(cur_dir,"/");
      if (!*cur_dir)
	StrCpy(cur_dir,"/");
      ScanFlags(&de.attr,Define("ST_FILE_ATTRS"),attr);
      if (cdt==I64_MIN)
	de.datetime=Now;
      else
	de.datetime=cdt;
      DirNew(dv,cur_dir,&de,FALSE);
    }
  } else
    PrintErr("File not found: \"%s\".\n",filename);
  Free(cur_dir);
}
public U0 Touch(U8 *files_find_mask="*",U8 *attr="+?",
	U8 *fu_flags=NULL,CDate cdt=I64_MIN)
{/*Touch file attributes and DateTime.
Default lists attributes.
attr: "+?" =show current
"+T" =resident
$LK,"RS_ATTR_READ_ONLY",A="MN:RS_ATTR_READ_ONLY"$  $LK,"ST_FILE_ATTRS",A="MN:ST_FILE_ATTRS"$
To Set DateL:
Touch(filename,"",,datetime);
*/
  I64 fuf_flags=0;
  CDirEntry *tmpde,*tmpde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tmpde=tmpde1=FilesFind(files_find_mask,fuf_flags);
  while (tmpde) {
    TouchFile(tmpde->full_name,attr,cdt);
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}
