#help_index "Doc/Cmd Line (Typically);Cmd Line (Typically)"

Bool Ed(U8 *link_st,I64 ed_flags=0)
{
  U8 *filename,*needle_str;
  I64 i,num;
  Bool cont,result=FALSE;
  CDoc *doc;

  switch (i=EdLinkCvt(link_st,&filename,&needle_str,&num,ed_flags)) {
    case -1:
      break;
    case LK_DEF:
      doc=DocNew;
      StrCpy(doc->desc,"DictDef");
      WSDDisplayDefs(doc,filename,num);
      goto ej_doc;
    case LK_HELP_INDEX:
      doc=DocNew;
      StrCpy(doc->desc,"HelpIdx");
      DocHelpIndex(doc,filename);
ej_doc:
      if (!(ed_flags&EDF_BAIL)) {
	DocDo(doc);
	DocDel(doc);
      }
      result=TRUE;
      break;
    default:
      if (IsRaw)
	EdLite(filename,num);
      else {
	cont=TRUE;
	if (!(ed_flags&EDF_BAIL) && !FilesFindMatch(filename,FILEMASK_TXT) &&
	    !PopUpCancelOk(ST_WARN_ST "Not Text File\n\n"))
	  cont=FALSE;
	if (cont)
	  result=DocFileEd(i,filename,needle_str,&num,ed_flags);
      }
  }
  Free(filename);
  Free(needle_str);
  return result;
}

public Bool Plain(U8 *filename)
//Open a document in plain text mode,
//so that dollar signs are not special.
{
  Bool result;
  U8 *st=MPrint("PI:%s",filename);
  result=Ed(st);
  Free(st);
  return result;
}

#help_index "Doc"
public Bool DocLinkChk(U8 *link_st)
{
  U8 *filename;
  Bool result=FALSE;
  if (link_st) {
    switch (EdLinkCvt(link_st,&filename)) {
      case -1:
	break;
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
      case LK_FILE:
	//We don't check line number
	result=FileFind(filename,,FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS);
	break;
      case LK_BIBLE_FIND:
	result=TRUE; //We don't check verse string
	break;
      default://TODO: Need to validate HI: and DF:
	if (Ed(link_st,EDF_BAIL))
	  result=TRUE;
    }
    Free(filename);
  }
  return result;
}

public U8 *DocLinkFile(U8 *link_st,CTask *mem_task=NULL)
{//Return the file for an $LK,"Editor Link Types",A="MN:LK_FILE"$.
  U8 *filename=NULL,*st,*result=NULL;
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
	result=StrNew(st);
	Free(st);
	break;
      case LK_BIBLE_FIND:
	result=StrNew(Define("ST_BIBLE_FILE"),mem_task);
	break;
    }
    Free(filename);
  }
  return result;
}

#help_index "Doc/Task"
public CDoc *DocBorderNew(CDoc *pdoc)
{
  CDocEntry *doc_e;
  CDoc *bdoc;

  bdoc=DocNew;
  bdoc->flags|=DOCF_BORDER_DOC;
  if (pdoc) {
    DocPrint(bdoc,"$$CM+H+BY+RX+NC,-7,1$$");
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdFilterCB;
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdOverStrikeCB;
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdDollarCB;
    DocPrint(bdoc,"$$CM+H+BY+RX+NC,-18,1$$");
    doc_e=DocPrint(bdoc,"$$TX+BD+TC,\"     \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdMoreCB;
    doc_e=DocPrint(bdoc,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=pdoc;
    doc_e->tag_cb=&EdDollarTypeCB;
  }
  DocPrint(bdoc,"$$CM+H+TY+NC,0,-1$$");
  doc_e=DocPrint(bdoc,"$$DA+H-TRM-P+BD+RD+CX+IV,140,A=\"%%s...\",SCX=15$$");
  doc_e->data=&Fs->task_title;
  DocDataFmt(doc_e);
  DocPrint(bdoc,"$$CM+H+NC,1,0$$$$TX+H+BD+IV,\"%X\"$$",Fs);
  DocPrint(bdoc,"$$TX+H+RX+BD,\"[X]\"$$");
  DocPrint(bdoc,"$$BK,1$$$$TX+H+LX+BD,\"MENU\"$$$$BK,0$$");
  return bdoc;
}

public U0 DocTermNew()
{
  Bool old_preempt=Preempt(OFF);
  CDoc *pdoc=DocNew;
  pdoc->right_click_link=&TermRightClickLink;
  pdoc->max_entries=4096;
  Fs->border_src=BDS_CUR_PRT;
  StrCpy(pdoc->desc,"Term");
  Fs->put_doc=Fs->display_doc=pdoc;
  Fs->border_doc=DocBorderNew(pdoc);
  Fs->cur_menu=MenuFile("::/Doc/EdPullDown.TXT");
  WinScrollsInit(Fs);
  Preempt(old_preempt);
  Raw(OFF);
}

#help_index "Doc/Input"
public U8 *DocScanLine(CDoc *doc,CDocEntry *doc_e,I64 *cur_col=NULL,CDocEntry **_do_e_end=NULL)
{
  CDocEntry *doc_e2=doc_e;
  Bool unlock=DocLock(doc);
  U8 *dst,*src,*start,*result;
  I64 i=0;
  if (cur_col)
    *cur_col=-1;
  while (doc_e2!=doc && doc_e2->type_u8!=DOCT_NEW_LINE) {
    if (doc_e2->flags1&DOCEF1_TAG) {
      src=doc_e2->tag;
      i+=StrLen(src);
    } else if (doc_e2->type_u8==DOCT_TAB)
      i++;
    doc_e2=doc_e2->next;
  }
  result=MAlloc(i+1);
  dst=result;
  while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE) {
    start=dst;
    if (doc_e->flags1&DOCEF1_TAG) {
      src=doc_e->tag;
      while (*src)
	*dst++=*src++;
    } else if (doc_e->type_u8==DOCT_TAB)
      *dst++='\t';
    if (doc_e==doc->cur_entry && cur_col)
      *cur_col=start-result+doc->cur_data_col;
    doc_e=doc_e->next;
  }
  if (_do_e_end) *_do_e_end=doc_e;
  *dst=0;
  if (doc_e==doc->cur_entry && cur_col && !doc->cur_data_col)
    *cur_col=dst-result;
  if (unlock)
    DocUnlock(doc);
  return result;
}

#help_index "Doc/Output"
U0 DocDump(CDoc *doc,I64 uS_delay=0)
{
  U8 *st;
  CDocEntry *doc_e,*doc_e2;
  Bool unlock=DocLock(doc);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    st=DocScanLine(doc,doc_e,NULL,&doc_e2);
    "%s",st;
    Free(st);
    doc_e=doc_e2;
    if (doc_e->type_u8==DOCT_NEW_LINE) {
      NewLine;
      BusyWait(uS_delay);
      doc_e=doc_e->next;
    }
  }
  if (unlock)
    DocUnlock(doc);
}

public CDocEntry *DocPutLine(CDoc *doc=NULL,CDocEntry *doc_e)
{
  I64 ch;
  U8 *ptr,*ptr2;
  Bool unlock;
  if (!doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return NULL;
  unlock=DocLock(doc);
  while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE) {
    if (doc_e->flags1&DOCEF1_TAG) {
      ptr=doc_e->tag;
      do {
	ptr2=ptr;
	while (ch=*ptr)
	  if (ch=='$$')
	    break;
	  else
	    ptr++;
	*ptr=0;
	"%s",ptr2;
	*ptr=ch;
	if (ch=='$$') {
	  "$$$$";
	  ptr++;
	}
      } while (ch);
    } else if (doc_e->type_u8==DOCT_TAB)
      '\t';
    doc_e=doc_e->next;
  }
  NewLine;
  if (doc_e!=doc)
    doc_e=doc_e->next;
  if (unlock)
    DocUnlock(doc);
  return doc_e;
}

#help_index "Doc/Input"
public U8 *DocMGetS(I64 flags=0)
//$LK,"Flags",A="MN:GETSF_SHIFT_ESC_EXIT"$
{
  CDoc *doc;
  CDocEntry *doc_e,*doc_e_end;
  Bool old_preempt=IsPreempt;
  U8 *st,*st2,*result;
  I64 ch,sc;
  "$$PT$$";
  do {
    ch=DocGetKey(&sc);
    if (ch==CH_ESC) {
      if (doc=DocPut) {
	DocLock(doc);
	if (doc->doc_signature==DOC_SIGNATURE_VAL && doc->cur_entry!=doc) {
	  doc_e=DocEntryCopy(doc,doc->cur_entry);
	  DocBottom(doc);
	  DocEntryRun(doc,doc_e,TRUE);
	  DocEntryDel(doc,doc_e);
	}
	DocUnlock(doc);
      }
      if (flags&GETSF_WITH_NEW_LINE)
	break;
    } if (ch==CH_SHIFT_ESC) {
      if (flags&GETSF_SHIFT_ESC_EXIT)
	Exit;
      else {
	result=StrNew("");
	goto mgets_done;
      }
    }
    //<CTRL-SHIFT-CR> is a blank line without entry
  } while (flags&GETSF_WITH_NEW_LINE || ch!='\n' || sc & SCF_CTRL && sc & SCF_SHIFT);

  Preempt(OFF);
  doc=DocPut;
  DocLock(doc);
  if (flags&GETSF_WITH_NEW_LINE) {
    doc_e=doc->cur_entry;
    do doc_e=doc_e->last;
    while (doc_e!=doc && doc_e->type_u8!=DOCT_PMT);
    doc_e=doc_e->next;
    if (result=DocScanLine(doc,doc_e,NULL,&doc_e_end)) {
      while (doc_e_end!=doc && doc_e_end->type_u8==DOCT_NEW_LINE) {
	st2=MPrint("%s\n",result);
	Free(result);
	result=st2;
	if (st=DocScanLine(doc,doc_e_end->next,NULL,&doc_e_end)) {
	  st2=MPrint("%s%s",result,st);
	  Free(st);
	  Free(result);
	  result=st2;
	}
      }
    }
  } else {
    doc_e=doc->cur_entry;
    do doc_e=doc_e->last;
    while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE &&
	doc_e->type_u8!=DOCT_PMT);
    if (doc_e!=doc && doc_e->type_u8!=DOCT_PMT) {
      do doc_e=doc_e->last;
      while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE &&
	  doc_e->type_u8!=DOCT_PMT);
    }
    doc_e=doc_e->next;
    result=DocScanLine(doc,doc_e,NULL);
  }
  DocUnlock(doc);
mgets_done:
  "$$PT$$$$FG$$$$BG$$";
  Preempt(old_preempt);
  return result;
}

#help_index "Char/Input"

public I64 PmtI64(U8 *msg=NULL,I64 dft=0,I64 lo=MIN_I64,I64 hi=MAX_I64)
{
  Bool okay;
  U8 *st;
  I64 result;
  while (TRUE) {
    if (msg)
      "" msg,dft;
    st=MGetS;
    if (!*st) {
      Free(st);
      return dft;
    }
    try {
      result=ExePrint2("ToI64(%s);",st);
      okay=TRUE;
    } catch {
      Fs->catch_except=TRUE;
      okay=FALSE;
    }
    Free(st);
    if (okay && lo<=result<=hi)
      return result;
  }
}

public F64 PmtF64(U8 *msg=NULL,F64 dft=0,F64 lo=MIN_F64,F64 hi=MAX_F64)
{
  Bool okay;
  U8 *st;
  F64 result;
  while (TRUE) {
    if (msg)
      "" msg,dft;
    st=MGetS;
    if (!*st) {
      Free(st);
      return dft;
    }
    try {
      result=ExePrint2("ToF64(%s);",st)(F64);
      okay=TRUE;
    } catch {
      Fs->catch_except=TRUE;
      okay=FALSE;
    }
    Free(st);
    if (okay && lo<=result<=hi)
      return result;
  }
}

public U8 *PmtStr(U8 *msg=NULL,U8 *dft=NULL)
{
  U8 *st;
  if (msg)
    "" msg,dft;
  st=MGetS;
  if (!*st) {
    Free(st);
    if (dft)
      return StrNew(dft);
    else
      return StrNew("");
  }
  return st;
}

public CDate PmtDate(U8 *msg=NULL,CDate dft=MIN_I64,CDate lo=MIN_I64,CDate hi=MAX_I64)
{
  U8 *st;
  CDate result;
  if (dft==MIN_I64) dft=Now;
  while (TRUE) {
    if (msg)
      "" msg,dft,dft;
    st=MGetS;
    if (!*st) {
      Free(st);
      return dft;
    }
    result=A2Date(st);
    Free(st);
    if (result>=lo && result<=hi)
      return result;
  }
}

#help_index "Doc"

#define RIGHT_INCLUDE	0
#define RIGHT_AINCLUDE	1
#define RIGHT_COMPILE	2
#define RIGHT_LOAD	3
#define RIGHT_AUTOFILE	4
#define RIGHT_BINREP	5
#define RIGHT_COPY	6
#define RIGHT_MOVE	7
#define RIGHT_DELETE	8
#define RIGHT_TYPE	9
#define RIGHT_ED	10
#define RIGHT_PLAIN	11

I64 PopUpTermRight(U8 *header)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPrint(doc,"%s",header);
  DocPrint(doc,"\n\n"
	"TXT=%s\nSRC=%s\nAOT=%s\nJIT=%s\nGRA=%s"
	"$$CM+LX,1,3 $$$$BT,\"INCLUDE          JIT\",LE=RIGHT_INCLUDE$$"
	"$$CM+LX,26,0$$$$BT,\"ADAM_INCLUDE     JIT\",LE=RIGHT_AINCLUDE$$"
	"$$CM+LX,1,3 $$$$BT,\"AOT COMPILE      AOT\",LE=RIGHT_COMPILE$$"
	"$$CM+LX,26,0$$$$BT,\"LOAD             BIN\",LE=RIGHT_LOAD$$"
	"$$CM+LX,1,3 $$$$BT,\"AUTOFILE         AUT\",LE=RIGHT_AUTOFILE$$"
	"$$CM+LX,26,0$$$$BT,\"BINREP           BIN\",LE=RIGHT_BINREP$$"
	"$$CM+LX,1,3 $$$$BT,\"COPY                \",LE=RIGHT_COPY$$"
	"$$CM+LX,26,0$$$$BT,\"MOVE                \",LE=RIGHT_MOVE$$"
	"$$CM+LX,1,3 $$$$BT,\"DELETE              \",LE=RIGHT_DELETE$$"
	"$$CM+LX,26,0$$$$BT,\"TYPE         TXT;GRA\",LE=RIGHT_TYPE$$"
	"$$CM+LX,1,3 $$$$BT,\"ED               TXT\",LE=RIGHT_ED$$"
	"$$CM+LX,26,0$$$$BT,\"PLAIN ED         TXT\",LE=RIGHT_PLAIN$$"
	"$$CM+LX,1,3 $$$$BT,\"CANCEL              \",LE=DOCM_CANCEL$$\n",
	FILEMASK_TXT,FILEMASK_SRC,FILEMASK_AOT,FILEMASK_JIT,FILEMASK_GRA);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 EdLeftClickLink(CDoc *doc,CDocEntry *doc_e)
{ //called with doc locked, exit unlocked
  Bool result;
  U8 *st,*st2;
  if (st=DocEntryLink(doc,doc_e)) {
    DocUnlock(doc);
    if (doc_e->flags1 & DOCEF1_POPUP) {
      st2=MPrint("Ed(\"%Q\");",st);
      result=PopUp(st2);
      Free(st2);
    } else
      result=Ed(st);
    Free(st);
    return result;
  }
}

I64 TermRightClickLink(CDoc *doc,CDocEntry *doc_e)
{ //called with doc locked, exit unlocked
  Bool send_new_line=FALSE,result=FALSE;
  U8 *st,*st2;
  I64 i;
  CEdFileName fn;
  if (st2=DocEntryLink(doc,doc_e)) {
    if (st=DocLinkFile(st2)) {
      DocUnlock(doc);
      if ((i=PopUpTermRight(st))>=0) {
	DocBottom(doc);
	switch (i) {
	  case RIGHT_INCLUDE:
	    if (FileExtDot(st) && !FilesFindMatch(st,FILEMASK_JIT)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "#include \"%s\";\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    ExeFile(st);
	    result=TRUE;
	    break;
	  case RIGHT_AINCLUDE:
	    if (FileExtDot(st) && !FilesFindMatch(st,FILEMASK_JIT)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Adam(\"#include \\\"%s\\\"\" );\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    AdamFile(st,FALSE);
	    result=TRUE;
	    break;
	  case RIGHT_COMPILE:
	    if (FileExtDot(st) && !FilesFindMatch(st,FILEMASK_AOT)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .PRJ File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Cmp(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    result=!ToBool(Cmp(st));
	    break;
	  case RIGHT_LOAD:
	    if (FileExtDot(st) && !FilesFindMatch(st,"*.BIN*")) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .BIN File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Load(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    result=ToBool(Load(st));
	    break;
	  case RIGHT_AUTOFILE:
	    if (FileExtDot(st) && !FilesFindMatch(st,"*.AUT*")) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .AUT File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "AutoFile(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    AutoStr("%F\n",st);
	    result=TRUE;
	    break;
	  case RIGHT_BINREP:
	    if (FileExtDot(st) && !FilesFindMatch(st,"*.BIN*")) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .BIN File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "BinRep(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    result=BinRep(st);
	    break;
	  case RIGHT_COPY:
	    StrCpy(fn.name,st);
	    if (DocFormDo(&fn)) {
	      result=ToBool(Copy(st,fn.name));
	    } else
	      send_new_line=TRUE;
	    break;
	  case RIGHT_MOVE:
	    StrCpy(fn.name,st);
	    if (DocFormDo(&fn))
	      result=Move(st,fn.name);
	    else
	      send_new_line=TRUE;
	    break;
	  case RIGHT_DELETE:
	    result=ToBool(Del(st));
	    break;
	  case RIGHT_TYPE:
	    result=Type(st);
	    break;
	  case RIGHT_ED:
	    "Ed(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    result=Ed(st);
	    break;
	  case RIGHT_PLAIN:
	    "Plain(\"%s\");\n$$PT$$$$FG$$$$BG$$",st;
	    result=Plain(st);
	    break;
	}
      } else
	send_new_line=TRUE;
      Free(st);
    } else
      send_new_line=TRUE;
    Free(st2);
  } else
    send_new_line=TRUE;
  DocBottom(doc);
  "$$PT$$$$FG$$$$BG$$";
  if (send_new_line)
    '\n';
  return result;
}

  #help_index "Debugging/Dump;Doc/Cmd Line (Typically);Cmd Line (Typically);Doc/Output"
public U0 DocDm(U8 *buf,I64 cnt=0x80)
//Dumps a chunk of mem, showing
//addresses.  It is live and can be
//edited.
{
  CDocEntry *doc_e;
  CDoc *doc=DocPut;
  Bool unlock=DocLock(doc);
  doc_e=DocPrint(doc,"$$HX-Z,%d,16$$",cnt);
  doc_e->data=buf;
  doc->cur_entry=doc_e->next;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 DocD(U8 *buf,I64 cnt=0x80)
//Dumps a chunk of mem, showing
//offsets.  It is live and can be
//edited.
{
  CDocEntry *doc_e;
  CDoc *doc=DocPut;
  Bool unlock=DocLock(doc);
  doc_e=DocPrint(doc,"$$HX,%d,16$$",cnt);
  doc_e->data=buf;
  doc->cur_entry=doc_e->next;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/Output"
Bool KDDocPutS(U8 *st)
{
  CDoc *doc;
  if (doc=DocPut)
    DocPrintPartial(doc,"%s",st);
  return FALSE;
}

KeyDevAdd(&KDDocPutKey,&KDDocPutS,0x80000000,TRUE);
fp_mgets=&DocMGetS;
fp_doc_put=&DocPut;
