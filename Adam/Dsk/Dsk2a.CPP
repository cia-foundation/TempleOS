#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"

public U8 *DBlk(I64 blk,Bool write=FALSE)
{//Dump disk block. Optionally, write.
//If you set write to TRUE, the block will
//be written when you press <ESC>.
//See $LK,"::/Demo/Dsk/DskRaw.CPP"$.
  U8 *buf=MAlloc(BLK_SIZE);

  RBlks(Fs->cur_prt,buf,blk,1);
  DocD(buf,BLK_SIZE);
  if (write) {
    "Edit and press <ESC> to write or <SHIFT-ESC>\n";
    if (View) {
      "Write\n";
      WBlks(Fs->cur_prt,buf,blk,1);
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
  U8 *buf=MAlloc(Fs->cur_prt->spc<<BLK_SIZE_BITS);
  c=ClusterNumNext(Fs->cur_prt,c,num);
  RClusters(Fs->cur_prt,buf,c,1);
  "Cluster:%X\n",c;
  DocD(buf,Fs->cur_prt->spc<<BLK_SIZE_BITS);
  if (write) {
    "Edit and press <ESC> to write or <SHIFT-ESC>\n";
    if (View) {
      "Write\n";
      WClusters(Fs->cur_prt,buf,c,1);
    }
  }
  return buf;
}

#help_index "File/Program Routines"

U0 FOFlatten(CDirEntry *tempde,CDirEntry **a,I64 *i)
{
  CDirEntry *tempde1;
  while (tempde) {
    tempde1=tempde->next;
    if (tempde->attr&RS_ATTR_DIR) {
      FOFlatten(tempde->sub,a,i);
      DirEntryDel(tempde);
    } else {
      a[*i]=tempde;
      *i=*i+1;
    }
    tempde=tempde1;
  }
}

public I64 FileCnt(CDirEntry *tempde)
{//Cnt of files in $LK,"CDirEntry",A="MN:CDirEntry"$ tree.
  I64 cnt=0;
  while (tempde) {
    if (tempde->attr&RS_ATTR_DIR)
      cnt+=FileCnt(tempde->sub);
    else
      cnt++;
    tempde=tempde->next;
  }
  return cnt;
}

#help_index "File/Cmd Line (Typically);Cmd Line (Typically)"

public Bool Copy(U8 *src_files_find_mask,U8 *dst_files_find_mask=".")
{//Copy files.
//If the name ends in ".Z", it will
//be stored compressed.  If not ".Z"
//it will be stored uncompressed.
  Bool result=TRUE;
  CFileAccess *fa;
  CDirEntry *tempde,*tempde1;
  U8 *st;
  if (!(tempde=FilesFind(src_files_find_mask,FUF_CLUSTER_ORDER)))
    return FALSE;
  if (IsDir(dst_files_find_mask)) {
    if (fa=FileAccessNew(dst_files_find_mask,TRUE)) {
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
      FileAccessDel(fa);
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
public I64 CopyTree(U8 *src_files_find_mask,U8 *dst_files_find_mask,Bool no_mask=TRUE)
{//Copy directory tree.
//Returns the count of copied files (not dirs).
  CFileAccess *fa;
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
  if (fa=FileAccessNew(src_files_find_mask,TRUE,,no_mask)) {
    tempde=SysFilesFind(fa->mask,FUF_RECURSE);
    st1=CurDir;
    FileAccessDel(fa);
    i1=StrLen(st1);
    if (i1==3) i1--;
    if (fa=FileAccessNew(dst_files_find_mask,TRUE,TRUE)) {
      st2=CurDir;
      i2=StrLen(st2);
      if (i2==3) i2--;
      result=CopyTree2(tempde,i1,i2,st2);
      FileAccessDel(fa);
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
      result+=SysDel(tempde1->full_name,TRUE);
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
      result+=SysDel(tempde1->full_name,FALSE);
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
    result+=SysDel(files_find_mask,TRUE);
    result+=SysDel(files_find_mask,FALSE);
  } else
    result=DelTreeFiles(FilesFind(files_find_mask,fuf_flags));
  return result;
}

U0 TouchFile(U8 *filename,U8 *attr,CDate cdt=MIN_I64)
{
  CPrt *p=Drv2Prt(*filename);
  CDirEntry de;
  U8 *cur_dir=StrNew(filename),
     buf[136];
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
      DirNew(p,cur_dir,&de,FALSE);
    }
  } else
    PrintErr("File not found.\n");
}
public U0 Touch(U8 *files_find_mask,U8 *attr="+?",U8 *fu_flags=NULL,CDate cdt=MIN_I64)
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

public I64 FF(U8 *files_find_mask,U8 *fu_flags=NULL)
{//Files find. List files matching mask.
  I64 cnt=0,fuf_flags=0;
  CDirEntry *tempde,*tempde1;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    PutFileLink(tempde->full_name);
    '\n';
    cnt++;
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return cnt;
}

public U0 SetDrvTextAttr(U8 drv_let,U8 attr)
{//Set color of the window border text
//for when this drive is the current drive.
  CPrt *p;
  try {
    p=Drv2Prt(drv_let);
    p->text_attr=attr;
  } catch
    Fs->catch_except=TRUE;
}
I64 RedSeaFSUnusedDrvSpace(U8 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let);
  I64 result=0,i,l;
  U8 *bitmap;
  try {
    l=(p->size+p->spc-1)/p->spc;
    i=((l+7)>>3+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    RBlks(p,bitmap,p->map1,i);
    for (i=0;i<l;i++)
      if (!Bt(bitmap,i))
	result++;
    Free(bitmap);
  } catch
    PrtUnlock(p);
  return result*BLK_SIZE*p->spc;
}
I64 FAT32UnusedDrvSpace(U8 drv_let=0)
{
  CPrt *p=Drv2Prt(drv_let);
  I64 result=0,i,l;
  U32 *bitmap;
  try {
    l=(p->size+p->spc-1)/p->spc;
    i=(l*4+BLK_SIZE-1)>>BLK_SIZE_BITS;
    bitmap=MAlloc(i<<BLK_SIZE_BITS);
    RBlks(p,bitmap,p->map1,i);
    for (i=0;i<l;i++)
      if (!bitmap[i])
	result++;
    Free(bitmap);
  } catch
    PrtUnlock(p);
  return result*BLK_SIZE*p->spc;
}
public I64 UnusedDrvSpace(U8 drv_let=0)
{//Returns unused size in bytes.
  CPrt *p=Drv2Prt(drv_let),*p_old=Fs->cur_prt;
  U8 *old_dir=StrNew(Fs->cur_dir);
  I64 result=0;
  Drv(drv_let);
  switch (p->type) {
    case PT_REDSEA:
      result=RedSeaFSUnusedDrvSpace(drv_let);
      break;
    case PT_FAT32:
      result=FAT32UnusedDrvSpace(drv_let);
      break;
    default:
      PrintErr("File System Not Supported\n");
  }
  Drv(Prt2Drv(p_old));
  Cd(old_dir);
  Free(old_dir);
  return result;
}

#help_index "File/Program Routines"
I64 Size1(CDirEntry *tempde,I64 *_fuf_flags,I64 round_to)
{
  U8 buf[BLK_SIZE];
  I64 result=0,i;
  CPrt *p;
  while (tempde) {
    if ((i=tempde->size) && Bt(_fuf_flags,FUf_EXPAND) &&
       !(tempde->attr&RS_ATTR_DIR) && FileAttr(tempde->name)&RS_ATTR_COMPRESSED) {
      p=Drv2Prt(*tempde->full_name);
      RBlks(p,buf,Cluster2Blk(p,tempde->cluster),1);
      i=(&buf)(CArcCompress *)->expanded_size;
    }
    if (round_to)
      i=CeilU64(tempde->size,round_to);
    if (tempde->attr&RS_ATTR_DIR)
      i+=Size1(tempde->sub,_fuf_flags,round_to);
    tempde->user_data=i;
    result+=i;
    tempde=tempde->next;
  }
  return result;
}
public I64 Size(U8 *files_find_mask="/*",U8 *fu_flags=NULL,I64 round_to=0)
{//Total size of files in mask. "+x" for expanded size.
//Does not include directory size of base directory, but
//does include size of sub directories.
  I64 fuf_flags=0,result=0;
  CDirEntry *tempde1=NULL;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  if (tempde1=FilesFind(files_find_mask,fuf_flags&FUG_FILES_FIND)) {
    fuf_flags&=FUF_EXPAND;
    result=Size1(tempde1,&fuf_flags,round_to);
    DirTreeDel(tempde1);
  }
  return result;
}
