#help_index "DolDoc/Link"

/* See $LK,"TempleOS Link Types",A="MN:ST_LINK_TYPES"$.
"filename"
"FI:filename"
"FA:haystack_filename,needle_anchor_str"
"FF:haystack_filename,needle_str"
"FF:haystack_filename,needle_str:occurnum"
"FL:filename,linenum"
"MN:SymName"
"PI:filename"
"PF:haystack_filename,needle_str"
"PF:haystack_filename,needle_str:occurnum"
"PL:filename,linenum"
"BF:haystack_bible_book,needle_str"
"DN:word"
"DN:word,defnum"
"HI:index"
"AD:code_address_number"

To edit a doc structure already in memory. See $LK,"SpriteEdText",A="MN:SpriteEdText"$().
"AI:doc_address"
"AA:haystack_doc_address,needle_anchor_str"
"AF:haystack_doc_address,needle_str"
"AF:haystack_doc_address,needle_str:occurnum"
"AL:doc_address,linenum"
*/

#define LK_FILE 	0
#define LK_FILE_ANCHOR	1
#define LK_FILE_FIND	2
#define LK_FILE_LINE	3
#define LK_MAN_PAGE	4
#define LK_PLAIN	5
#define LK_PLAIN_FIND	6
#define LK_PLAIN_LINE	7
#define LK_BIBLE_FIND	8
#define LK_DEF		9
#define LK_HELP_INDEX	10
#define LK_ADDR		11
#define LK_DOC		12 //See $LK,"SpriteEdText",A="MN:SpriteEdText"$()
#define LK_DOC_ANCHOR	13
#define LK_DOC_FIND	14
#define LK_DOC_LINE	15
#define LK_PLACE_ANCHOR	16

public U8 *DocEntryLink(CDoc *doc,CDocEntry *doc_e)
{//MAlloc new str, either tag or aux_str if link.
  if (doc_e->de_flags&DOCEF_LINK) {
    if (doc_e->de_flags & DOCEF_AUX_STR)
      return StrNew(doc_e->aux_str,doc->mem_task);
    else if (doc_e->de_flags & DOCEF_TAG)
      return StrNew(doc_e->tag,doc->mem_task);
  }
  return NULL;
}

Bool DocFileEd(I64 _type,U8 *filename,
	U8 *needle_str,I64 *_num,I64 edf_dof_flags)
{
  I64 type=_type,flags=0,old_border_src=Fs->border_src;
  CDocEntry *doc_e;
  CDoc *doc;
  Bool old_silent=Bt(&Fs->display_flags,DISPLAYf_SILENT),
	res=FALSE,other_found=FALSE;
  U8 *st1,*st2;
  try {
    switch (type) {
      case LK_PLAIN:
	flags=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;
      case LK_DOC:
	type=LK_FILE;
	break;
      case LK_DOC_ANCHOR:
	type=LK_FILE_ANCHOR;
	break;
      case LK_PLAIN_FIND:
	flags=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;
      case LK_DOC_FIND:
	type=LK_FILE_FIND;
	break;
      case LK_PLAIN_LINE:
	flags=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;
      case LK_DOC_LINE:
	type=LK_FILE_LINE;
	break;
      case LK_BIBLE_FIND:
	flags=DOCF_PLAIN_TEXT|DOCF_NO_CURSOR;
	break;
    }

    flags|=DOCF_ALLOW_UNDO;

    if (LK_DOC<=_type<=LK_DOC_LINE) {
      doc=Str2I64(filename);//See $LK,"SpriteEdText",A="MN:SpriteEdText"$()
      res=TRUE;
    } else {
      st1=StrNew(filename);
      st2=StrNew(filename);
      StrLastRem(st1,"/",st2); //st2 is name without dir
      if (!FileNameChk(st2))
	doc=NULL;
      else {
	Silent;
	if (Bt(&edf_dof_flags,EDf_BAIL)) //if bail, scan parents
	  res=FileFind(filename,,
		FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	else if (!(res=FileFind(filename,,FUF_JUST_FILES)))
	  other_found=FileFind(filename,,
		FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	doc=DocRead(filename,flags);
	doc->desc='Edit';
	Silent(old_silent);
	Fs->border_src=BDS_ED_FILENAME_DRV;
      }
      Free(st1);
      Free(st2);
    }
    if (!doc||doc->doc_signature!=DOC_SIGNATURE_VAL)
      res=FALSE;
    else {
      if (Bt(&edf_dof_flags,EDf_COLLAPSE))
	DocCollapse(TRUE,doc);
      else if (Bt(&edf_dof_flags,EDf_UNCOLLAPSE))
	DocCollapse(FALSE,doc);

      if (res || other_found)
	switch (type) {
	  case LK_FILE_LINE:
	    res=DocGoToLine(doc,*_num);
	    break;
	  case LK_FILE_ANCHOR:
	    res=DocAnchorFind(doc,needle_str);
	    break;
	  case LK_FILE_FIND:
	    res=DocFind(doc,,needle_str,*_num);
	    break;
	  case LK_BIBLE_FIND:
	    res=DocFind(doc,*_num,needle_str);
	    break;
	  default:
	    DocCenter(doc);
	}
      *_num=doc->cur_entry->y+1;

      if (edf_dof_flags&EDF_WAS_WRITE)
	res=FALSE;
      if (!(edf_dof_flags&EDF_BAIL)) {
	if (*doc->filename.name)
	  doc->filename.dirc=DirContextNew(doc->filename.name);
	else
	  doc->filename.dirc=NULL;
	if (DocEd(doc,edf_dof_flags|DOF_DONT_HOME)) {
	  DocLock(doc);
	  doc_e=doc->cur_entry;
	  if (doc_e!=doc)
	    DocEntryRun(doc,doc_e,TRUE);
	  DocUnlock(doc);
	  if (!(LK_DOC<=_type<=LK_DOC_LINE)) {
	    DocWrite(doc);
	    if (edf_dof_flags&EDF_WAS_WRITE)
	      res=TRUE;
	  }
	}
	DirContextDel(doc->filename.dirc);
      }
      if (!(LK_DOC<=_type<=LK_DOC_LINE))
	DocDel(doc);
    }
  } catch {
    Silent(old_silent);
    res=FALSE;
  }
  Fs->border_src=old_border_src;
  return res;
}

#define DFT_ADDR_LINK_BIN_SIZE	64

public I64 EdLinkCvt(U8 *link_st,U8 **_filename=NULL,U8 **_needle_str=NULL,
	I64 *_num=NULL,I64 edf_dof_flags=0)
{//$LK,"Editor Link",A="MN:LK_FILE"$--> filename, needle_str and line number.
  U8 *st,*ptr,*src,*filename=NULL,*needle_str=NULL,*filename2;
  I64 res,i,num=1;
  CHashSrcSym *tmph;
  if (!link_st||!*link_st) {
    if (edf_dof_flags&EDF_BAIL)
      return -1;
    link_st=blkdev.tmp_filename;
  }
  st=StrNew(link_st);
  res=LK_FILE;
  if (StrLen(st)>3 && st[2]==':') {
    st[2]=0;
    filename2=st+3;
    switch (res=DefineMatch(st,"ST_LINK_TYPES",LMF_IGNORE_CASE)) {
      case LK_MAN_PAGE:
	if (tmph=HashFind(filename2,Fs->hash_table,HTG_SRC_SYM))
	  res=EdLinkCvt(tmph->src_link,&filename,
		&needle_str,&num,edf_dof_flags);
	else
	  res=-1;
	goto lc_done;
      case LK_ADDR:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  i=Str2I64(ptr+1);
	} else
	  i=DFT_ADDR_LINK_BIN_SIZE;
	if (ptr=SrcEdLink(ExePrint("%s;",filename2),i)) {
	  res=EdLinkCvt(ptr,&filename,&needle_str,&num,edf_dof_flags);
	  Free(ptr);
	} else
	  res=-1;
	goto lc_done;
      case LK_DEF:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  i=Str2I64(ptr+1);
	} else
	  i=-1;
	filename=StrNew(filename2);
	num=i;
	goto lc_done;
      case LK_HELP_INDEX:
	filename=StrNew(filename2);
	goto lc_done;
      case LK_BIBLE_FIND:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  src=ptr+1;
	  while (*src) { //We do not allow ending verse
	    if (*src=='-')
	      *src=0;
	    src++;
	  }
	  needle_str=StrNew(ptr+1);
	}
	i=DefineMatch(filename2,"ST_BIBLE_BOOKS",LMF_IGNORE_CASE);
	if (i<0)
	  res=-1;
	else {
	  num=Str2I64(DefineSub(i,"ST_BIBLE_BOOK_LINES"));
	  filename2=BIBLE_FILENAME;
	}
	break;
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_DOC_LINE:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  num=Str2I64(ptr+1);
	}
	break;
      case LK_FILE_ANCHOR:
      case LK_DOC_ANCHOR:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  needle_str=StrNew(ptr+1);
	}
	break;
      case LK_FILE_FIND:
      case LK_PLAIN_FIND:
      case LK_DOC_FIND:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  needle_str=StrNew(ptr+1);
	  if (ptr=StrLastOcc(needle_str,":")) {
	    *ptr=0;
	    num=Str2I64(ptr+1);
	  }
	}
	break;
    }
  } else
    filename2=st;
  if (res>=0) {
    if (LK_DOC<=res<=LK_DOC_LINE)
      filename=StrNew(filename2); //Holds document address as number.
    else
      filename=FileNameAbs(filename2,FUF_Z_OR_NOT_Z);
  }
lc_done:
  Free(st);
  if (_filename)
    *_filename=filename;
  else
    Free(filename);
  if (_needle_str)
    *_needle_str=needle_str;
  else
    Free(needle_str);
  if (_num)
    *_num=num;
  return res;
}

public Bool DocLinkChk(CDoc *doc,U8 *link_st)
{//Check for bad $LK,"Editor Link",A="MN:LK_FILE"$.
  U8 *filename,*st;
  Bool res=FALSE;
  CDirContext *dirc;
  if (link_st) {
    st=FileNameAbs(doc->filename.name);
    dirc=DirContextNew(st);
    Free(st);
    switch (EdLinkCvt(link_st,&filename)) {
      case -1:
	break;
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_FILE:
//We don't check line number
	res=FileFind(filename,,
	      FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	break;
      case LK_BIBLE_FIND:
	st=StrNew(link_st+3);
	if (StrOcc(st,','))
	  StrLastRem(st,",");
	if (DefineMatch(st,"ST_BIBLE_BOOKS",LMF_IGNORE_CASE)>=0)
	  res=TRUE;
	Free(st);
	break;
      default://TODO: Need to validate HI: and DN:
	if (Ed(link_st,EDF_BAIL))
	  res=TRUE;
    }
    Free(filename);
    DirContextDel(dirc);
  }
  return res;
}

public U8 *DocLinkFile(U8 *link_st,CTask *mem_task=NULL)
{//Return the file for an $LK,"Editor Link Types",A="MN:LK_FILE"$.
  U8 *filename=NULL,*st,*res=NULL;
  if (link_st) {
    switch (EdLinkCvt(link_st,&filename)) {
      case LK_FILE:
      case LK_FILE_ANCHOR:
      case LK_FILE_FIND:
      case LK_FILE_LINE:
      case LK_PLAIN:
      case LK_PLAIN_FIND:
      case LK_PLAIN_LINE:
	st=FileNameAbs(filename,FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	res=StrNew(st);
	Free(st);
	break;
      case LK_BIBLE_FIND:
	res=StrNew(BIBLE_FILENAME,mem_task);
	break;
    }
    Free(filename);
  }
  return res;
}
