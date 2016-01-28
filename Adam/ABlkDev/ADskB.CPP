#help_index "File/Internal"
I64 DirTreeSerializeSize(CDirEntry *tempde)
{
  I64 result=0;
  while (tempde) {
    result+=CDIR_SIZE+1;
    if (tempde->attr & RS_ATTR_DIR)
      result+=DirTreeSerializeSize(tempde->sub);
    tempde=tempde->next;
  }
  return result+1;
}
I64 DirTreeSerializeFill(CDirEntry *tempde,U8 *dst)
{
  I64 result=0,i;
  while (tempde) {
    *dst++=1;
    result++;
    MemCpy(dst,&tempde->start,CDIR_SIZE);
    dst+=CDIR_SIZE;
    result+=CDIR_SIZE;
    if (tempde->attr & RS_ATTR_DIR) {
      i=DirTreeSerializeFill(tempde->sub,dst);
      dst+=i;
      result+=i;
    }
    tempde=tempde->next;
  }
  *dst=0;
  return result+1;
}
public U8 *DirTreeSerialize(CDirEntry *tempde,I64 *_size=NULL)
{//Serialize tree returned from $LK,"FilesFind",A="MN:FilesFind"$() into a one contiguous U8 array.
  I64 size=DirTreeSerializeSize(tempde);
  U8 *buf=MAlloc(size);
  DirTreeSerializeFill(tempde,buf);
  if (_size) *_size=size;
  return buf;
}

U8 *DirTreeUnserialize2(U8 *src,CDirEntry **tempde)
{
  CDirEntry *tempde1;
  if (*src++) {
    tempde1=CAlloc(sizeof(CDirEntry));
    *tempde=tempde1;
    MemCpy(&tempde1->start,src,CDIR_SIZE);
    src+=CDIR_SIZE;
    if (tempde1->attr & RS_ATTR_DIR)
      src=DirTreeUnserialize2(src,&tempde1->sub);
    src=DirTreeUnserialize2(src,&tempde1->next);
  } else
    *tempde=NULL;
  return src;
}
public CDirEntry *DirTreeUnserialize(U8 *src)
{//Unserialize tree to make it like a tree returned from $LK,"FilesFind",A="MN:FilesFind"$().
  CDirEntry *tempde=NULL;
  DirTreeUnserialize2(src,&tempde);
  return tempde;
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

I64 Size1(CDirEntry *tempde,I64 *_fuf_flags,I64 round_to)
{
  U8 buf[BLK_SIZE];
  I64 result=0,i;
  CDrv *dv;
  while (tempde) {
    if ((i=tempde->size) && Bt(_fuf_flags,FUf_EXPAND) &&
	  !(tempde->attr&RS_ATTR_DIR) &&
	  FileAttr(tempde->name)&RS_ATTR_COMPRESSED) {
      dv=Let2Drv(*tempde->full_name);
      RBlks(dv,buf,Cluster2Blk(dv,tempde->cluster),1);
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

public I64 Zip(U8 *files_find_mask="*",U8 *fu_flags=NULL)
{//Compress files by moving to .Z filename.
  U8 *st;
  CDirEntry *tempde,*tempde1;
  I64 result=0,fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    if (!IsDotZ(tempde->full_name)) {
      st=MStrPrint("%s.Z",tempde->full_name);
      result+=Move(tempde->full_name,st);
      Free(st);
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return result;
}

public I64 Unzip(U8 *files_find_mask="*.Z",U8 *fu_flags=NULL)
{//Uncompress files by moving to not .Z filename.
//You don't have to do this for normal operation.
  //It automatically unzips ".Z" files.
  U8 *st;
  CDirEntry *tempde,*tempde1;
  I64 result=0,fuf_flags=0;
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),"+r+f+F+O");
  ScanFlags(&fuf_flags,Define("ST_FILE_UTIL_FLAGS"),fu_flags);
  tempde=tempde1=FilesFind(files_find_mask,fuf_flags);
  while (tempde) {
    if (IsDotZ(tempde->full_name)) {
      st=StrNew(tempde->full_name);
      StrLastRem(st,".");
      result+=Move(tempde->full_name,st);
      Free(st);
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  return result;
}
