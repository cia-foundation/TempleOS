#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"
public U8 *DBlk(I64 blk,Bool write=FALSE)
{//Dump disk block. Optionally, write.
//If you set write to TRUE, the block will
  //be written when you press <ESC>.
  //See $LK,"::/Demo/Dsk/DskRaw.CPP"$.
  U8 *buf=MAlloc(BLK_SIZE);

  RBlks(Fs->cur_dv,buf,blk,1);
  DocD(buf,BLK_SIZE);
  if (write) {
    "Edit and press <ESC> to write or <SHIFT-ESC>\n";
    if (View) {
      "Write\n";
      WBlks(Fs->cur_dv,buf,blk,1);
    }
  }
  return buf;
}

public U8 *DCluster(I64 c,Bool write=FALSE,I64 num=0)
{//Dump disk cluster. Optionally, write.
//If you set write to TRUE, the cluster will
  //be written when you press <ESC>.
  //See $LK,"::/Demo/Dsk/DskRaw.CPP"$.
  //Do $LK,"Dir",A="MN:Dir"$("*",TRUE); to get cluster numbers of files.
  U8 *buf=MAlloc(Fs->cur_dv->spc<<BLK_SIZE_BITS);
  c=ClusterNumNext(Fs->cur_dv,c,num);
  RClusters(Fs->cur_dv,buf,c,1);
  "Cluster:%X\n",c;
  DocD(buf,Fs->cur_dv->spc<<BLK_SIZE_BITS);
  if (write) {
    "Edit and press <ESC> to write or <SHIFT-ESC>\n";
    if (View) {
      "Write\n";
      WClusters(Fs->cur_dv,buf,c,1);
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
  Bool result=TRUE;
  CDirContext *dirc;
  CDirEntry *tempde,*tempde1;
  U8 *st;
  if (!(tempde=FilesFind(src_files_find_mask,FUF_CLUSTER_ORDER)))
    return FALSE;
  if (IsDir(dst_files_find_mask)) {
    if (dirc=DirContextNew(dst_files_find_mask,TRUE)) {
      while (tempde) {
	tempde1=tempde->next;
	if (!(tempde->attr & RS_ATTR_DIR)) {
	  st=FileNameAbs(tempde->name);
	  if (!CopySingle(tempde->full_name,st))
	    result=FALSE;
	  Free(st);
	}
	DirEntryDel(tempde);
	tempde=tempde1;
      }
      DirContextDel(dirc);
    }
    return result;
  } else
    return CopySingle(src_files_find_mask,dst_files_find_mask);
}

public Bool Move(U8 *f1,U8 *f2)
{//Move files from one location to another or rename.
  if (Copy(f1,f2)) {
    Del(f1);
    return TRUE;
  }
  return FALSE;
}

I64 CopyTree2(CDirEntry *tempde,I64 src_dir_len,I64 dst_dir_len,U8 *dst_dir)
{
  U8 *st;
  I64 result=1;
  while (tempde) {
    st=MAlloc(StrLen(tempde->full_name)+dst_dir_len+2);
    MemCpy(st,dst_dir,dst_dir_len);
    StrCpy(st+dst_dir_len,tempde->full_name+src_dir_len);
    if (tempde->attr & RS_ATTR_DIR) {
      MkDir(st);
      result+=CopyTree2(tempde->sub,src_dir_len,dst_dir_len,dst_dir);
    } else
      if (CopySingle(tempde->full_name,st))
	result++;
    Free(st);
    tempde=tempde->next;
  }
  return result;
}
public I64 CopyTree(U8 *src_files_find_mask,U8 *dst_files_find_mask,
	Bool no_mask=TRUE)
{//Copy directory tree.
//Returns the count of copied files (not dirs).
  CDirContext *dirc;
  CDirEntry *tempde=NULL;
  I64 result=0,i1,i2;
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
    tempde=FilesFind(dirc->mask,FUF_RECURSE);
    st1=CurDir;
    DirContextDel(dirc);
    i1=StrLen(st1);
    if (i1==3) i1--;
    if (dirc=DirContextNew(dst_files_find_mask,TRUE,TRUE)) {
      st2=CurDir;
      i2=StrLen(st2);
      if (i2==3) i2--;
      result=CopyTree2(tempde,i1,i2,st2);
      DirContextDel(dirc);
      Free(st2);
    }
    DirTreeDel(tempde);
    Free(st1);
  }
  return result;
}

I64 DelTreeDirs(CDirEntry *tempde1)
{
  I64 result=0;
  CDirEntry *tempde2;
  while (tempde1) {
    tempde2=tempde1->next;
    if (tempde1->attr & RS_ATTR_DIR) {
      if (tempde1->sub)
	result+=DelTreeDirs(tempde1->sub);
      result+=Del(tempde1->full_name,TRUE,TRUE);
    }
    DirEntryDel(tempde1);
    tempde1=tempde2;
  }
  return result;
}
I64 DelTreeFiles(CDirEntry *tempde1)
{
  I64 result=0;
  CDirEntry *tempde2;
  while (tempde1) {
    tempde2=tempde1->next;
    if (tempde1->attr & RS_ATTR_DIR) {
      if (tempde1->sub)
	result+=DelTreeFiles(tempde1->sub);
    } else
      result+=Del(tempde1->full_name,FALSE,TRUE);
    DirEntryDel(tempde1);
    tempde1=tempde2;
  }
  return result;
}
public I64 DelTree(U8 *files_find_mask,U8 *fu_flags=NULL)
{//Delete directory tree.
  I64 result=0,fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (IsDir(files_find_mask)) {
    result=DelTreeDirs(FilesFind(files_find_mask,fuf_flags));
    result+=Del(files_find_mask,TRUE,TRUE);
    result+=Del(files_find_mask,FALSE,TRUE);
  } else
    result=DelTreeFiles(FilesFind(files_find_mask,fuf_flags));
  return result;
}

U0 TouchFile(U8 *filename,U8 *attr,CDate cdt=MIN_I64)
{
  CDrv *dv=Let2Drv(*filename);
  CDirEntry de;
  U8 *cur_dir=StrNew(filename),
	buf[STR_LEN];
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
      if (cdt==MIN_I64)
	de.datetime=Now;
      else
	de.datetime=cdt;
      DirNew(dv,cur_dir,&de,FALSE);
    }
  } else
    PrintErr("File not found.\n");
}
public U0 Touch(U8 *files_find_mask="*",U8 *attr="+?",
	U8 *fu_flags=NULL,CDate cdt=MIN_I64)
{/*Touch file attributes and DateTime.
Default lists attributes.
attr: "+?" =show current
"+T" =resident
$LK,"RS_ATTR_READ_ONLY",A="MN:RS_ATTR_READ_ONLY"$  $LK,"ST_FILE_ATTRS",A="MN:ST_FILE_ATTRS"$
To Set DateL:
Touch(filename,"",,datetime);
*/
  I64 fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    TouchFile(tempde->full_name,attr,cdt);
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}
