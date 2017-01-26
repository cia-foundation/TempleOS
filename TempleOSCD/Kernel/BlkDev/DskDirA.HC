Bool DirNew(CDrv *dv,U8 *cur_dir,CDirEntry *tmpde,Bool free_old_chain=TRUE)
{//Makes a directory entry in the directory from a $LK,"CDirEntry",A="MN:CDirEntry"$ node.
  switch (dv->fs_type) {
    case FSt_REDSEA:
      return RedSeaDirNew(dv,cur_dir,tmpde,free_old_chain);
    case FSt_FAT32:
      return FAT32DirNew(dv,cur_dir,tmpde,free_old_chain);
    case FSt_ISO9660:
      PrintErr("Not Writable\n");
      return FALSE;
    default:
      PrintErr("File System Not Supported\n");
      return FALSE;
  }
}

U0 DirEntryDel(CDirEntry *tmpde)
{//Free node returned from $LK,"FilesFind",A="MN:FilesFind"$().  Doesn't Free user_data.
//Does not change the directory on disk.
  if (tmpde) {
    Free(tmpde->full_name);
    Free(tmpde);
  }
}

U0 DirEntryDel2(CDirEntry *tmpde)
{//Free node returned from $LK,"FilesFind",A="MN:FilesFind"$().  Frees user_data
//Does not change the directory on disk.
  if (tmpde) {
    Free(tmpde->full_name);
    Free(tmpde->user_data);
    Free(tmpde);
  }
}

U0 DirTreeDel(CDirEntry *tmpde)
{//Free tree returned from $LK,"FilesFind",A="MN:FilesFind"$().  Doesn't Free user_data.
//Does not change the directory on disk.
  CDirEntry *tmpde2;
  while (tmpde) {
    tmpde2=tmpde->next;
    if (tmpde->sub)
      DirTreeDel(tmpde->sub);
    DirEntryDel(tmpde);
    tmpde=tmpde2;
  }
}

U0 DirTreeDel2(CDirEntry *tmpde)
{//Free tree returned from $LK,"FilesFind",A="MN:FilesFind"$().  Frees user_data
//Does not change the directory on disk.
  CDirEntry *tmpde2;
  while (tmpde) {
    tmpde2=tmpde->next;
    if (tmpde->sub)
      DirTreeDel2(tmpde->sub);
    DirEntryDel2(tmpde);
    tmpde=tmpde2;
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

I64 DirEntryCompareClus(CDirEntry *e1,CDirEntry *e2)
{
  return e1->clus-e2->clus;
}

#define SK_NAME		0
#define SK_CLUS	1

U0 DirFilesSort(CDirEntry **_tmpde,I64 key)
{
  I64 i,cnt;
  CDirEntry *tmpde=*_tmpde,*tmpde1,**sort_buf;
  if (tmpde) {
    cnt=LinkedLstCnt(tmpde);
    if (cnt>1) {
      sort_buf=MAlloc(cnt*sizeof(U8 *));
      i=0;
      tmpde1=tmpde;
      while (tmpde1) {
	sort_buf[i++]=tmpde1;
	tmpde1=tmpde1->next;
      }
      switch [key] {
	case SK_NAME:
	  QSortI64(sort_buf,cnt,&DirEntryCompareName);
	  break;
	case SK_CLUS:
	  QSortI64(sort_buf,cnt,&DirEntryCompareClus);
	  break;
      }
      tmpde=sort_buf[0];
      *_tmpde=tmpde;
      for (i=0;i<cnt-1;i++) {
	tmpde1=sort_buf[i];
	tmpde1->next=sort_buf[i+1];
      }
      tmpde1=sort_buf[i];
      tmpde1->next=NULL;
      Free(sort_buf);

      tmpde1=tmpde;
      while (tmpde1) {
	if (tmpde1->sub)
	  DirFilesSort(&tmpde1->sub,key);
	tmpde1=tmpde1->next;
      }
    } else
      if (tmpde->sub)
	DirFilesSort(&tmpde->sub,key);
  }
}

CDirEntry *DirFilesFlatten(CDirEntry *tmpde,CDirEntry **_res,I64 fuf_flags)
{//Returns last node
  CDirEntry *tmpde1;
  Bool del;
  if (tmpde)
    while (TRUE) {
      tmpde1=tmpde->next;
      if (!(tmpde->attr&RS_ATTR_DIR)||!(fuf_flags&FUF_JUST_FILES)) {
	_res=*_res=tmpde;
	del=FALSE;
      } else
	del=TRUE;
      if (tmpde->sub) {
	_res=DirFilesFlatten(tmpde->sub,_res,fuf_flags);
	tmpde->sub=NULL;
      }
      if (del)
	DirEntryDel(tmpde);
      if (tmpde1)
	tmpde=tmpde1;
      else
	break;
    }
  *_res=NULL;
  return _res;
}

U0 PutFileLink(U8 *filename,U8 *full_name=NULL,I64 line=0,Bool plain_text=FALSE)
{//Put $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ file,line link to StdOut, $LK,"DocPut",A="MN:DocPut"$.
  U8 *st;
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
      if (!full_name)
	full_name=st=FileNameAbs(filename);
      else
	st=NULL;
      if (plain_text) {
	if (line)
	  "$$LK,\"%s,%04d\",A=\"PL:%s,%d\"$$",filename,line,full_name,line;
	else
	  "$$LK,\"%s\",A=\"PI:%s\"$$",filename,full_name;
      } else {
	if (line)
	  "$$LK,\"%s,%04d\",A=\"FL:%s,%d\"$$",filename,line,full_name,line;
	else
	  "$$LK,\"%s\",A=\"FI:%s\"$$",filename,full_name;
      }
      Free(st);
    }
  }
}

U0 PutDirLink(U8 *dirname,U8 *full_name=NULL)
{//Put $LK,"DolDoc",A="FI:::/Doc/DolDocOverview.DD"$ dir macro to StdOut, $LK,"DocPut",A="MN:DocPut"$.
  U8 *st;
  if (!dirname) return;
  if (IsRaw)
    "%s",dirname;
  else {
    if (!full_name)
      full_name=st=DirNameAbs(dirname);
    else
      st=NULL;
    "$$MA,T=\"%s\",LM=\"Cd(\\\"%s\\\");Dir;\n\"$$",dirname,full_name;
    Free(st);
  }
}
