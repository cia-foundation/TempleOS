Bool CDirNew(CPrt *p,U8 *cur_dir,CDirEntry *tempm,Bool free_old_chain=TRUE)
{
  switch (p->type) {
    case PT_REDSEA:
      return RedSeaFSDirNew(p,cur_dir,tempm,free_old_chain);
    case PT_FAT32:
      return FAT32DirNew(p,cur_dir,tempm,free_old_chain);
    case PT_ISO9660:
      PutDefineErr("ST_NOT_WRITABLE");
      return FALSE;
    default:
      PutDefineErr("ST_UNSUPPORTED_FILE_SYSTEM");
      return FALSE;
  }
}

U0 CDirEntryDel(CDirEntry *tempm)
{ //Doesn't Free user_data
  if (tempm) {
    Free(tempm->full_name);
    Free(tempm);
  }
}

U0 CDirEntryDel2(CDirEntry *tempm)
{ //Frees user_data
  if (tempm) {
    Free(tempm->full_name);
    Free(tempm->user_data);
    Free(tempm);
  }
}

U0 CDirLstDel(CDirEntry *tempm)
{ //Doesn't Free user_data
  CDirEntry *tempm2;
  while (tempm) {
    tempm2=tempm->next;
    if (tempm->sub)
      CDirLstDel(tempm->sub);
    CDirEntryDel(tempm);
    tempm=tempm2;
  }
}

U0 CDirLstDel2(CDirEntry *tempm)
{ //Frees user_data
  CDirEntry *tempm2;
  while (tempm) {
    tempm2=tempm->next;
    if (tempm->sub)
      CDirLstDel2(tempm->sub);
    CDirEntryDel2(tempm);
    tempm=tempm2;
  }
}

I64 CDirEntryCompare(CDirEntry *e1,CDirEntry *e2)
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

U0 CDirFilesSort(CDirEntry **_tempm)
{
  I64 i,cnt=0;
  CDirEntry *tempm=*_tempm,*tempm1,**sort_buf;
  if (tempm) {
    tempm1=tempm;
    while (tempm1) {
      cnt++;
      tempm1=tempm1->next;
    }
    if (cnt>1) {
      sort_buf=MAlloc(cnt*sizeof(U8 *));
      i=0;
      tempm1=tempm;
      while (tempm1) {
	sort_buf[i++]=tempm1;
	tempm1=tempm1->next;
      }
      QSortI64(sort_buf,cnt,&CDirEntryCompare);
      tempm=sort_buf[0];
      *_tempm=tempm;
      for (i=0;i<cnt-1;i++) {
	tempm1=sort_buf[i];
	tempm1->next=sort_buf[i+1];
      }
      tempm1=sort_buf[i];
      tempm1->next=NULL;
      Free(sort_buf);

      tempm1=tempm;
      while (tempm1) {
	if (tempm1->sub)
	  CDirFilesSort(&tempm1->sub);
	tempm1=tempm1->next;
      }
    } else
      if (tempm->sub)
	CDirFilesSort(&tempm->sub);
  }
}

CDirEntry *CDirFilesFlatten(CDirEntry *tempm,CDirEntry **_result,I64 fuf_flags)
{ //returns last node
  CDirEntry *tempm1;
  Bool del;
  if (tempm)
    while (TRUE) {
      tempm1=tempm->next;
      if (!(tempm->attr&_ATTR_DIR)||!(fuf_flags&FUF_JUST_FILES)) {
	_result=*_result=tempm;
	del=FALSE;
      } else
	del=TRUE;
      if (tempm->sub) {
	_result=CDirFilesFlatten(tempm->sub,_result,fuf_flags);
	tempm->sub=NULL;
      }
      if (del)
	CDirEntryDel(tempm);
      if (tempm1)
	tempm=tempm1;
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
      PutDefineErr("ST_UNSUPPORTED_FILE_SYSTEM");
      result=NULL;
  }
  if (result) {
    CDirFilesSort(&result);
    if (fuf_flags&(FUF_FLATTEN_TREE|FUF_JUST_FILES))
      CDirFilesFlatten(result,&result,fuf_flags);
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
      "" filename;
  } else {
    buf=FileNameAbs(filename);
    if (plain_text) {
      if (line)
	"$$LK,\"%s,%04d\",\"PL:%s,%d\"$$",filename,line,buf,line;
      else
	"$$LK,\"%s\",\"PI:%s\"$$",filename,buf;
    } else {
      if (line)
	"$$LK,\"%s,%04d\",\"FL:%s,%d\"$$",filename,line,buf,line;
      else
	"$$LK,\"%s\",\"FI:%s\"$$",filename,buf;
    }
    Free(buf);
  }
}

U0 PutDirLink(U8 *dirname)
{
  U8 *buf;
  if (!dirname) return;
  if (IsRaw)
    "" dirname;
  else {
    buf=DirNameAbs(dirname);
    "$$MA+A,\"%s\",\"Cd(\\\"%s\\\");Dir;\n\"$$",dirname,buf;
    Free(buf);
  }
}

U0 Dir2(U8 *files_find_mask,I64 fuf_flags,Bool full)
{
  CDirEntry *tempm1=NULL,*tempm2;
  U8 *st;
  I64 csize=0xFFFF,esize=0xFFFF,c=0xFFFF;
  tempm1=SysFilesFind(files_find_mask,fuf_flags);

  if (!(st=CurDir))
    PutDefine("ST_INVALID_DRV");
  else {
    if (tempm1) {
      //Find max columns
      tempm2=tempm1;
      while (tempm2) {
	if (tempm2->size>csize)
	  csize=tempm2->size;
	if (tempm2->expanded_size>esize)
	  esize=tempm2->expanded_size;
	if (tempm2->cluster>c)
	  c=tempm2->cluster;
	tempm2=tempm2->next;
      }
      csize=Bsr(csize)/4+1;
      esize=Bsr(esize)/4+1;
      c=Bsr(c)/4+1;

      "$$MA+A,\"Directory\",\"PopUpCd;Dir;\n\"$$ of %s\n",st;
      if (full)
	"__DATE__ __TIME__ %*ts %*ts %*ts\n",
	csize,"SIZE",esize,"UNCMPSIZE",c,"CLUSTER";
      else
	"DATE_ TIME_ %*ts\n",csize,"SIZE";
      while (tempm1) {
	tempm2=tempm1->next;
	if (full)
	  "%D %T %0*tX %0*tX %0*tX ",tempm1->datetime,tempm1->datetime,
	    csize,tempm1->size,esize,tempm1->expanded_size,
	    c, tempm1->cluster;
	else
	  "%h1D %h1T %0*tX ",tempm1->datetime,tempm1->datetime,
	    csize,tempm1->size;
	if (tempm1->attr & _ATTR_DIR)
	  PutDirLink(tempm1->name);
	else
	  PutFileLink(tempm1->name);

	NewLine;
	CDirEntryDel(tempm1);
	tempm1=tempm2;
      }
    } else
      PutDefine("ST_NO_MATCHING_ENTRIES");
    Free(st);
  }
}
