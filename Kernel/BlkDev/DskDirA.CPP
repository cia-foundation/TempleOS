Bool DirNew(CDrv *dv,U8 *cur_dir,CDirEntry *tempde,Bool free_old_chain=TRUE)
{//Makes a directory entry in the directory from a $LK,"CDirEntry",A="MN:CDirEntry"$ node.
  switch (dv->fs_type) {
    case FSt_REDSEA:
      return RedSeaDirNew(dv,cur_dir,tempde,free_old_chain);
    case FSt_FAT32:
      return FAT32DirNew(dv,cur_dir,tempde,free_old_chain);
    case FSt_ISO9660:
      PrintErr("Not Writable\n");
      return FALSE;
    default:
      PrintErr("File System Not Supported\n");
      return FALSE;
  }
}

U0 DirEntryDel(CDirEntry *tempde)
{//Free node returned from $LK,"FilesFind",A="MN:FilesFind"$().  Doesn't Free user_data.
//Does not change the directory on disk.
  if (tempde) {
    Free(tempde->full_name);
    Free(tempde);
  }
}

U0 DirEntryDel2(CDirEntry *tempde)
{//Free node returned from $LK,"FilesFind",A="MN:FilesFind"$().  Frees user_data
//Does not change the directory on disk.
  if (tempde) {
    Free(tempde->full_name);
    Free(tempde->user_data);
    Free(tempde);
  }
}

U0 DirTreeDel(CDirEntry *tempde)
{//Free tree returned from $LK,"FilesFind",A="MN:FilesFind"$().  Doesn't Free user_data.
//Does not change the directory on disk.
  CDirEntry *tempde2;
  while (tempde) {
    tempde2=tempde->next;
    if (tempde->sub)
      DirTreeDel(tempde->sub);
    DirEntryDel(tempde);
    tempde=tempde2;
  }
}

U0 DirTreeDel2(CDirEntry *tempde)
{//Free tree returned from $LK,"FilesFind",A="MN:FilesFind"$().  Frees user_data
//Does not change the directory on disk.
  CDirEntry *tempde2;
  while (tempde) {
    tempde2=tempde->next;
    if (tempde->sub)
      DirTreeDel2(tempde->sub);
    DirEntryDel2(tempde);
    tempde=tempde2;
  }
}

I64 DirEntryCompareName(CDirEntry *e1,CDirEntry *e2)
{
  U8 buf1[CDIR_FILENAME_LEN],buf2[CDIR_FILENAME_LEN],
	buf3[CDIR_FILENAME_LEN],buf4[CDIR_FILENAME_LEN];
  I64 d1=0,d2=0;
  if (e1->attr & RS_ATTR_DIR)
    d1=1;
  if (e2->attr & RS_ATTR_DIR)
    d2=1;
  if (d1!=d2)
    return d2-d1;
  else {
    StrCpy(buf1,e1->name);
    StrCpy(buf2,e2->name);
    FileExtRem(buf1,buf3);
    FileExtRem(buf2,buf4);
    if (d1=StrCmp(buf3,buf4))
      return d1;
    return StrCmp(buf1,buf2);
  }
}

I64 DirEntryCompareCluster(CDirEntry *e1,CDirEntry *e2)
{
  return e1->cluster-e2->cluster;
}

#define SK_NAME		0
#define SK_CLUSTER	1

U0 DirFilesSort(CDirEntry **_tempde,I64 key)
{
  I64 i,cnt;
  CDirEntry *tempde=*_tempde,*tempde1,**sort_buf;
  if (tempde) {
    cnt=LinkedLstCnt(tempde);
    if (cnt>1) {
      sort_buf=MAlloc(cnt*sizeof(U8 *));
      i=0;
      tempde1=tempde;
      while (tempde1) {
	sort_buf[i++]=tempde1;
	tempde1=tempde1->next;
      }
      switch [key] {
	case SK_NAME:
	  QSortI64(sort_buf,cnt,&DirEntryCompareName);
	  break;
	case SK_CLUSTER:
	  QSortI64(sort_buf,cnt,&DirEntryCompareCluster);
	  break;
      }
      tempde=sort_buf[0];
      *_tempde=tempde;
      for (i=0;i<cnt-1;i++) {
	tempde1=sort_buf[i];
	tempde1->next=sort_buf[i+1];
      }
      tempde1=sort_buf[i];
      tempde1->next=NULL;
      Free(sort_buf);

      tempde1=tempde;
      while (tempde1) {
	if (tempde1->sub)
	  DirFilesSort(&tempde1->sub,key);
	tempde1=tempde1->next;
      }
    } else
      if (tempde->sub)
	DirFilesSort(&tempde->sub,key);
  }
}

CDirEntry *DirFilesFlatten(CDirEntry *tempde,CDirEntry **_result,I64 fuf_flags)
{//Returns last node
  CDirEntry *tempde1;
  Bool del;
  if (tempde)
    while (TRUE) {
      tempde1=tempde->next;
      if (!(tempde->attr&RS_ATTR_DIR)||!(fuf_flags&FUF_JUST_FILES)) {
	_result=*_result=tempde;
	del=FALSE;
      } else
	del=TRUE;
      if (tempde->sub) {
	_result=DirFilesFlatten(tempde->sub,_result,fuf_flags);
	tempde->sub=NULL;
      }
      if (del)
	DirEntryDel(tempde);
      if (tempde1)
	tempde=tempde1;
      else
	break;
    }
  *_result=NULL;
  return _result;
}

U0 PutFileLink(U8 *filename,I64 line=0,Bool plain_text=FALSE)
{//Put $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.TXT"$ file,line link to StdOut, $LK,"DocPut",A="MN:DocPut"$.
  U8 *buf;
  if (!filename) return;
  if (IsRaw) {
    if (line)
      "%s,%04d",filename,line;
    else
      "%s",filename;
  } else {
//LK_DOC,LK_DOC_ANCHOR,LK_DOC_FIND,LK_DOC_LINE
    if (filename[0]=='A'&&filename[2]==':') {
      if (line) //See $LK,"SpriteEdText",A="MN:SpriteEdText"$()
	"$$LK,\"%s,%04d\",A=\"AL:%s,%d\"$$",filename+3,line,filename+3,line;
      else
	"$$LK,\"%s\",A=\"AI:%s\"$$",filename+3,filename+3;
    } else {
      buf=FileNameAbs(filename);
      if (plain_text) {
	if (line)
	  "$$LK,\"%s,%04d\",A=\"PL:%s,%d\"$$",filename,line,buf,line;
	else
	  "$$LK,\"%s\",A=\"PI:%s\"$$",filename,buf;
      } else {
	if (line)
	  "$$LK,\"%s,%04d\",A=\"FL:%s,%d\"$$",filename,line,buf,line;
	else
	  "$$LK,\"%s\",A=\"FI:%s\"$$",filename,buf;
      }
      Free(buf);
    }
  }
}

U0 PutDirLink(U8 *dirname)
{//Put $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.TXT"$ dir macro to StdOut, $LK,"DocPut",A="MN:DocPut"$.
  U8 *buf;
  if (!dirname) return;
  if (IsRaw)
    "%s",dirname;
  else {
    buf=DirNameAbs(dirname);
    "$$MA,T=\"%s\",LM=\"Cd(\\\"%s\\\");Dir;\n\"$$",dirname,buf;
    Free(buf);
  }
}
