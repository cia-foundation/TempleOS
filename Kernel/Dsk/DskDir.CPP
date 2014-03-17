Bool DirNew(CPrt *p,U8 *cur_dir,CDirEntry *tempde,Bool free_old_chain=TRUE)
{
  switch (p->type) {
    case PT_REDSEA:
      return RedSeaFSDirNew(p,cur_dir,tempde,free_old_chain);
    case PT_FAT32:
      return FAT32DirNew(p,cur_dir,tempde,free_old_chain);
    case PT_ISO9660:
      PrintErr("Not Writable\n");
      return FALSE;
    default:
      PrintErr("File System Not Supported\n");
      return FALSE;
  }
}

U0 DirEntryDel(CDirEntry *tempde)
{ //Doesn't Free user_data
  if (tempde) {
    Free(tempde->full_name);
    Free(tempde);
  }
}

U0 DirEntryDel2(CDirEntry *tempde)
{ //Frees user_data
  if (tempde) {
    Free(tempde->full_name);
    Free(tempde->user_data);
    Free(tempde);
  }
}

U0 DirLstDel(CDirEntry *tempde)
{ //Doesn't Free user_data
  CDirEntry *tempde2;
  while (tempde) {
    tempde2=tempde->next;
    if (tempde->sub)
      DirLstDel(tempde->sub);
    DirEntryDel(tempde);
    tempde=tempde2;
  }
}

U0 DirLstDel2(CDirEntry *tempde)
{ //Frees user_data
  CDirEntry *tempde2;
  while (tempde) {
    tempde2=tempde->next;
    if (tempde->sub)
      DirLstDel2(tempde->sub);
    DirEntryDel2(tempde);
    tempde=tempde2;
  }
}

I64 DirEntryCompareName(CDirEntry *e1,CDirEntry *e2)
{
  U8 buf1[CDIR_FILENAME_LEN],buf2[CDIR_FILENAME_LEN],
     buf3[CDIR_FILENAME_LEN],buf4[CDIR_FILENAME_LEN];
  I64 d1=0,d2=0;
  if (e1->attr & _ATTR_DIR)
    d1=1;
  if (e2->attr & _ATTR_DIR)
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
  I64 i,cnt=0;
  CDirEntry *tempde=*_tempde,*tempde1,**sort_buf;
  if (tempde) {
    tempde1=tempde;
    while (tempde1) {
      cnt++;
      tempde1=tempde1->next;
    }
    if (cnt>1) {
      sort_buf=MAlloc(cnt*sizeof(U8 *));
      i=0;
      tempde1=tempde;
      while (tempde1) {
	sort_buf[i++]=tempde1;
	tempde1=tempde1->next;
      }
      nobound_switch(key) {
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
{ //returns last node
  CDirEntry *tempde1;
  Bool del;
  if (tempde)
    while (TRUE) {
      tempde1=tempde->next;
      if (!(tempde->attr&_ATTR_DIR)||!(fuf_flags&FUF_JUST_FILES)) {
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

CDirEntry *SysFilesFind(U8 *files_find_mask,I64 fuf_flags)
{
  CPrt *p=Fs->cur_prt;
  CDirEntry *result=NULL;
  PrtChk(p);
  switch (p->type) {
    case PT_REDSEA:
      result=RedSeaFSFilesFind(files_find_mask,fuf_flags);
      break;
    case PT_FAT32:
      result=FAT32FilesFind(files_find_mask,fuf_flags);
      break;
    case PT_ISO9660:
      result=ISO1FilesFind(files_find_mask,fuf_flags);
      break;
    default:
      PrintErr("File System Not Supported\n");
      result=NULL;
  }
  if (result) {
    DirFilesSort(&result,SK_NAME);
    if (fuf_flags&(FUF_FLATTEN_TREE|FUF_JUST_FILES))
      DirFilesFlatten(result,&result,fuf_flags);
    if (fuf_flags&FUF_CLUSTER_ORDER)
      DirFilesSort(&result,SK_CLUSTER);
  }
  return result;
}

U0 PutFileLink(U8 *filename,I64 line=0,Bool plain_text=FALSE)
{
  U8 *buf;
  if (!filename) return;
  if (IsRaw) {
    if (line)
      "%s,%04d",filename,line;
    else
      "%s",filename;
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

U0 PutDirLink(U8 *dirname)
{
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

I64 Dir2(U8 *files_find_mask,I64 fuf_flags,Bool full)
{
  CDirEntry *tempde1=NULL,*tempde2;
  U8 *st;
  I64 csize=0xFFFF,c=0xFFFF,result=0;
  tempde1=SysFilesFind(files_find_mask,fuf_flags);
  if (!(st=CurDir))
    PrintErr("Invalid Drive\n");
  else {
    if (tempde1) {
      //Find max columns
      tempde2=tempde1;
      while (tempde2) {
	if (tempde2->size>csize)
	  csize=tempde2->size;
	if (tempde2->cluster>c)
	  c=tempde2->cluster;
	tempde2=tempde2->next;
      }
      csize=Bsr(csize)/4+1;
      c=Bsr(c)/4+1;

      "$$MA,T=\"Directory\",LM=\"PopUpCd;Dir;\n\"$$ of %s\n",st;
      if (full)
	"__DATE__ __TIME__ %*ts %*ts\n",
	csize,"SIZE",c,"CLUSTER";
      else
	"DATE_ TIME_ %*ts\n",csize,"SIZE";
      while (tempde1) {
	tempde2=tempde1->next;
	result++;
	if (full)
	  "%D %T %0*tX %0*tX ",tempde1->datetime,tempde1->datetime,
	    csize,tempde1->size,c,tempde1->cluster;
	else
	  "%02d/%02d %02d:%02d %0*tX ",Month(tempde1->datetime),Day(tempde1->datetime),
	    Hour24(tempde1->datetime),Minute(tempde1->datetime),
	    csize,tempde1->size;
	if (tempde1->attr & _ATTR_DIR)
	  PutDirLink(tempde1->name);
	else
	  PutFileLink(tempde1->name);

	NewLine;
	DirEntryDel(tempde1);
	tempde1=tempde2;
      }
    } else
      "No matching entries\n";
    Free(st);
  }
  return result;
}
