#help_index "Doc/Cmd Line (Typically);Cmd Line (Typically)"

Bool Ed(U8 *link_st,I64 ed_flags=0)
{
  U8 *filename,*pattern;
  I64 i,num;
  Bool cont,result=FALSE;
  CDoc *doc;

  switch (i=EdLinkCvt(link_st,&filename,&pattern,&num,ed_flags)) {
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
	if (!(ed_flags&EDF_BAIL) && !FilesFindMatch(FILENAME_MASK_TXT,filename) &&
	    !PopUpCancelOk(ST_WARN_ST "Not Text File\n\n"))
	  cont=FALSE;
	if (cont)
	  result=DocFileEd(i,filename,pattern,&num,ed_flags);
      }
  }
  Free(filename);
  Free(pattern);
  return result;
}

public Bool Plain(U8 *filename)
//Open a document in plain text mode,
//so that dollar signs are not special.
{
  Bool result;
  U8 *st=MSPrintF("PI:%s",filename);
  result=Ed(st);
  Free(st);
  return result;
}

#help_index "Doc"
public Bool DocLinkChk(U8 *link_st)
{
  U8 *filename;
  I64 num;
  Bool result=FALSE;
  if (link_st) {
    switch (EdLinkCvt(link_st,&filename,,&num)) {
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
{
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
public CDoc *SysAuxDocInit()
{
  CDocEntry *doc_e;
  CDoc *aux,*doc=DocCur;

  aux=DocNew;
  aux->flags|=DOCF_AUX_DOC;

  if (doc) {
    DocPutS(aux,"$$CM+H+BY+RX+NC,-7,1$$");
    doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=doc;
    doc_e->tag_cb=&EdFilterCB;
    doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=doc;
    doc_e->tag_cb=&EdOverStrikeCB;
    doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=doc;
    doc_e->tag_cb=&EdDollarCB;
  }

  DocPutS(aux,"$$CM+H+BY+RX+NC,-4,1$$");
  doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
  doc_e->tag_cb=&EdQuotesCB;
  doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
  doc_e->tag_cb=&EdBracesCB;
  doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
  doc_e->tag_cb=&EdSemicolonCB;

  if (doc) {
    DocPutS(aux,"$$CM+H+BY+RX+NC,-18,1$$");
    doc_e=DocPutS(aux,"$$TX+BD+TC,\"     \"$$");
    doc_e->user_data=doc;
    doc_e->tag_cb=&EdMoreCB;
    doc_e=DocPutS(aux,"$$TX+H+BD+TC,\" \"$$");
    doc_e->user_data=doc;
    doc_e->tag_cb=&EdDollarTypeCB;
  }

  if (Fs->user_num)
    DocPrintF(aux,"$$CM+H+TY+LX+NC,5,-1$$$$TX+H+BD,\"#%d\"$$",Fs->user_num);
  DocPutS(aux,"$$CM+H+TY+NC,0,-1$$");
  doc_e=DocPutS(aux,"$$DA+H-T-P+BD+RD+CX+SCX+IV,140,\"%s...\",16$$");
  doc_e->data=&Fs->task_title;
  DocDataFmt(doc_e);
  DocPrintF(aux,"$$CM+H+NC,1,0$$$$TX+H+BD+IV,\"%X\"$$",Fs);
  aux->settings_root.dft_text_attr=Fs->border_attr;
  if (Bt(&Fs->display_flags,DISPLAYf_HAS_CLOSE_WIN))
    DocPutS(aux,"$$TX+H+RX+BD,\"[X]\"$$");
  DocPutS(aux,"$$BK,1$$$$TX+H+LX+BD,\"MENU\"$$$$BK,0$$");
  return aux;
}

public U0 UseTermDoc()
{
  Bool old_preempt=Preempt(OFF);
  CDoc *doc=DocNew;
  doc->start_text_attr=WHITE<<4+BLUE;
  DocQueIns(doc,Fs->last_doc);
  doc->right_click_link=&TermRightClickLink;
  doc->max_entries=4096;
  doc->flags|=DOCF_ATTR_BY_PRT;
  StrCpy(doc->desc,"Term");
  DocQueInsRev(SysAuxDocInit,Fs->next_doc);
  Fs->cur_menu=MenuFile("::/TempleOS/Doc/EdPullDown.TXT");
  WinScrollsInit(Fs);
  Preempt(old_preempt);
  Raw(OFF);
}

#help_index "Doc/Input"
public U8 *DocGetLine(CDoc *doc,CDocEntry *doc_e,I64 *cur_col=NULL,CDocEntry **_do_e_end=NULL)
{
  CDocEntry *doc_e2=doc_e;
  Bool unlock=DocLock(doc);
  U8 *dst,*src,*start,*result;
  I64 i=0;
  if (cur_col)
    *cur_col=-1;
  while (doc_e2!=doc && doc_e2->type_u8!=DOCT_NEW_LINE) {
    if (Bt(doc_tag_types,doc_e2->type_u8)) {
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
    if (Bt(doc_tag_types,doc_e->type_u8)) {
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
public U0 DocDump(CDoc *doc,I64 uS_delay=0)
{
  U8 *st;
  CDocEntry *doc_e,*doc_e2;
  Bool unlock=DocLock(doc);
  doc_e=doc->root.next;
  while (doc_e!=doc) {
    st=DocGetLine(doc,doc_e,NULL,&doc_e2);
    "" st;
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

public CDocEntry *DocPutLine(CDocEntry *doc_e)
{
  U8 ch,*ptr,*ptr2;
  while (doc_e->type_u8!=DOCT_ERROR && doc_e->type_u8!=DOCT_NEW_LINE) {
    if (Bt(doc_tag_types,doc_e->type_u8)) {
      ptr=doc_e->tag;
      do {
	ptr2=ptr;
	while (ch=*ptr)
	  if (ch=='$$')
	    break;
	  else
	    ptr++;
	*ptr=0;
	"" ptr2;
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
  return doc_e->type_u8==DOCT_ERROR ? doc_e:doc_e->next;
}

#help_index "Doc/Input"
public U8 *DocMGetS(I64 flags=0)
//$LK,"Flags","MN:GETSF_SHIFT_ESC_EXIT"$
{
  CDoc *doc;
  CDocEntry *doc_e,*doc_e_end;
  Bool old_preempt=IsPreempt;
  U8 *st,*result;
  I64 ch,sc;
  "$$FU$$";
  do {
    ch=DocGetKey(&sc);
    if (ch==CH_ESC) {
      doc=Fs->next_doc;
      while (doc!=(&Fs->next_doc)(U8 *)-offset(CDoc.next_doc) &&
	  doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL) {
	DocLock(doc);
	if (!(doc->flags&DOCF_DONT_SHOW) &&
	    (!doc->best_d || !(doc->flags&DOCF_AUX_DOC)) &&
	    doc->doc_in_que_signature==DOC_IN_QUE_SIGNATURE_VAL && doc->cur_entry!=doc) {
	  doc_e=DocEntryCopy(doc,doc->cur_entry);
	  if (doc->flags&DOCF_AUX_DOC && DocCur)
	    DocBottom;
	  else
	    DocBottom(doc);
	  DocEntryRun(doc,doc_e,TRUE);
	  DocEntryDel(doc,doc_e);
	  DocUnlock(doc);
	  break;
	}
	DocUnlock(doc);
	doc=doc->next_doc;
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
  doc=DocCur;
  DocLock(doc);
  if (flags&GETSF_WITH_NEW_LINE) {
    doc_e=doc->cur_entry;
    do doc_e=doc_e->last;
    while (doc_e!=doc && doc_e->type_u8!=DOCT_USER_FOREGROUND);
    doc_e=doc_e->next;
    if (result=DocGetLine(doc,doc_e,NULL,&doc_e_end)) {
      while (doc_e_end!=doc && doc_e_end->type_u8==DOCT_NEW_LINE) {
	result=StrAdd(result,"\n");
	if (st=DocGetLine(doc,doc_e_end->next,NULL,&doc_e_end)) {
	  result=StrAdd(result,st);
	  Free(st);
	}
      }
    }
  } else {
    doc_e=doc->cur_entry;
    do doc_e=doc_e->last;
    while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE &&
	doc_e->type_u8!=DOCT_USER_FOREGROUND);
    if (doc_e!=doc && doc_e->type_u8!=DOCT_USER_FOREGROUND) {
      do doc_e=doc_e->last;
      while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE &&
	  doc_e->type_u8!=DOCT_USER_FOREGROUND);
    }
    doc_e=doc_e->next;
    result=DocGetLine(doc,doc_e,NULL);
  }
  DocUnlock(doc);
mgets_done:
  "$$FU$$$$FG$$$$BG$$";
  Preempt(old_preempt);
  return result;
}

#help_index "Char/Input"

public I64 PmtI64(U8 *msg,I64 dft,I64 lo=MIN_I64,I64 hi=MAX_I64)
{
  Bool okay;
  U8 *st;
  I64 result;
  while (TRUE) {
    "" msg,dft;
    st=MGetS;
    if (!*st) {
      Free(st);
      return dft;
    }
    try {
      okay=FALSE;
      result=ExePrintF3("ToI64(%s);",st);
      okay=TRUE;
    } catch
      Fs->catch_except=TRUE;
    Free(st);
    if (okay && lo<=result<=hi)
      return result;
  }
}

public F64 PmtF64(U8 *msg,F64 dft,F64 lo=MIN_F64,F64 hi=MAX_F64)
{
  Bool okay;
  U8 *st;
  F64 result;
  while (TRUE) {
    "" msg,dft;
    st=MGetS;
    if (!*st) {
      Free(st);
      return dft;
    }
    try {
      okay=FALSE;
      result=ExePrintF3("ToF64(%s);",st)(F64);
      okay=TRUE;
    } catch
      Fs->catch_except=TRUE;
    Free(st);
    if (okay && lo<=result<=hi)
      return result;
  }
}

public U8 *PmtStr(U8 *msg=NULL,U8 *dft=NULL)
{
  U8 *st;
  if (msg) {
    if (dft)
      "" msg,dft;
    else
      "" msg;
  }
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

public CDate PmtCDate(U8 *msg,CDate dft,CDate lo=MIN_I64,CDate hi=MAX_I64)
{
  U8 *st;
  CDate result;
  while (TRUE) {
    "" msg,dft,dft;
    st=MGetS;
    if (!*st) {
      Free(st);
      return dft;
    }
    result=A2CDate(st);
    Free(st);
    if (result>=lo && result<=hi)
      return result;
  }
}

#help_index "Doc"

#define RIGHT_INCLUDE	0
#define RIGHT_DEBUG	1
#define RIGHT_COMPILE	2
#define RIGHT_LOAD	3
#define RIGHT_AINCLUDE	4
#define RIGHT_AUTOFILE	5
#define RIGHT_COPY	6
#define RIGHT_MOVE	7
#define RIGHT_DELETE	8
#define RIGHT_TYPE	9
#define RIGHT_ED	10
#define RIGHT_PLAIN	11
#define RIGHT_BINREP	12

I64 PopUpCancelIncludeDelAuto(U8 *header=NULL,U8 *footer=NULL)
{
  I64 i;
  CDoc *doc=DocNew;
  if (header) DocPutS(doc,header);
  DocPrintF(doc,"\n\n"
	"TXT=%s\n"
	"SRC=%s\n"
	"AOT=%s\n"
	"JIT=%s\n"
	"GRA=%s"
	"$$CM+LX,1,3 $$$$BT,\"INCLUDE          JIT\",RIGHT_INCLUDE$$"
	"$$CM+LX,26,0$$$$BT,\"DEBUG            JIT\",RIGHT_DEBUG$$"
	"$$CM+LX,1,3 $$$$BT,\"AOT COMPILE      AOT\",RIGHT_COMPILE$$"
	"$$CM+LX,26,0$$$$BT,\"LOAD             BIN\",RIGHT_LOAD$$"
	"$$CM+LX,1,3 $$$$BT,\"ADAM_INCLUDE     JIT\",RIGHT_AINCLUDE$$"
	"$$CM+LX,26,0$$$$BT,\"AUTOFILE         AUT\",RIGHT_AUTOFILE$$"
	"$$CM+LX,1,3 $$$$BT,\"COPY                \",RIGHT_COPY$$"
	"$$CM+LX,26,0$$$$BT,\"MOVE                \",RIGHT_MOVE$$"
	"$$CM+LX,1,3 $$$$BT,\"DELETE              \",RIGHT_DELETE$$"
	"$$CM+LX,26,0$$$$BT,\"TYPE         TXT;GRA\",RIGHT_TYPE$$"
	"$$CM+LX,1,3 $$$$BT,\"ED               TXT\",RIGHT_ED$$"
	"$$CM+LX,26,0$$$$BT,\"PLAIN ED         TXT\",RIGHT_PLAIN$$"
	"$$CM+LX,1,3 $$$$BT,\"CANCEL              \",DOCM_CANCEL$$"
	"$$CM+LX,26,0$$$$BT,\"BINREP           BIN\",RIGHT_BINREP$$\n",
	  FILENAME_MASK_TXT,
	  FILENAME_MASK_SRC,
	  FILENAME_MASK_AOT,
	  FILENAME_MASK_JIT,
	  FILENAME_MASK_GRA);
  if (footer) DocPutS(doc,footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 EdLeftClickLink(CDoc *doc,CDocEntry *doc_e)
{ //called with doc locked, exit unlocked
  Bool result;
  U8 *st;
  if (st=DocEntryLink(doc,doc_e)) {
    DocUnlock(doc);
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
      if ((i=PopUpCancelIncludeDelAuto(st))>=0) {
	DocBottom(doc);
	switch (i) {
	  case RIGHT_INCLUDE:
	    if (FileExtDot(st) && !FilesFindMatch(FILENAME_MASK_JIT,st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "#include \"%s\";\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    ExeFile(st);
	    result=TRUE;
	    break;
	  case RIGHT_DELETE:
	    result=ToBool(Del(st));
	    break;
	  case RIGHT_TYPE:
	    result=Type(st);
	    break;
	  case RIGHT_ED:
	    "Ed(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    result=Ed(st);
	    break;
	  case RIGHT_PLAIN:
	    "Plain(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    result=Plain(st);
	    break;
	  case RIGHT_AUTOFILE:
	    if (FileExtDot(st) && !FilesFindMatch("*.AUT*",st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .AUT File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "AutoFile(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    AutoStr("%F\n",st);
	    result=TRUE;
	    break;
	  case RIGHT_AINCLUDE:
	    if (FileExtDot(st) && !FilesFindMatch(FILENAME_MASK_JIT,st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Adam(\"#include \\\"%s\\\"\" );\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    AdamFile(st,FALSE);
	    result=TRUE;
	    break;
	  case RIGHT_COMPILE:
	    if (FileExtDot(st) && !FilesFindMatch(FILENAME_MASK_AOT,st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .PRJ File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Cmp(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    result=!ToBool(Cmp(st));
	    break;
	  case RIGHT_LOAD:
	    if (FileExtDot(st) && !FilesFindMatch("*.BIN*",st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .BIN File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "Load(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    result=ToBool(Load(st));
	    break;
	  case RIGHT_BINREP:
	    if (FileExtDot(st) && !FilesFindMatch("*.BIN*",st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .BIN File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "BinRep(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    result=BinRep(st);
	    break;
	  case RIGHT_COPY:
	    StrCpy(fn.name,st);
	    if (DocFormDo(fn.name,"CEdFileName")) {
	      result=ToBool(Copy(st,fn.name));
	    } else
	      send_new_line=TRUE;
	    break;
	  case RIGHT_MOVE:
	    StrCpy(fn.name,st);
	    if (DocFormDo(fn.name,"CEdFileName"))
	      result=Move(st,fn.name);
	    else
	      send_new_line=TRUE;
	    break;
	  case RIGHT_DEBUG:
	    if (FileExtDot(st) && !FilesFindMatch(FILENAME_MASK_JIT,st)) {
	      if (!PopUpCancelOk(ST_WARN_ST "Not .CPP File\n\n")) {
		send_new_line=TRUE;
		break;
	      }
	    }
	    "IDE(\"%s\");\n$$FU$$$$FG$$$$BG$$",st;
	    WinZBufUpdate;
	    IDE(st);
	    result=TRUE;
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
  "$$FU$$$$FG$$$$BG$$";
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
  CDoc *doc=DocCur;
  Bool unlock=DocLock(doc);
  doc_e=DocPrintF(doc,"$$HX-Z,%d,16$$",cnt);
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
  CDoc *doc=DocCur;
  Bool unlock=DocLock(doc);
  doc_e=DocPrintF(doc,"$$HX,%d,16$$",cnt);
  doc_e->data=buf;
  doc->cur_entry=doc_e->next;
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/Output"
Bool AKDDocPutS(U8 *st)
{
  CDoc *doc;
  if (doc=DocCur)
    DocPutSPartial(doc,st);
  return FALSE;
}

KeyDevAdd(&AKDDocPutKey,&AKDDocPutS,0x80000000,TRUE);
fp_mgets=&DocMGetS;
fp_doc_cur=&DocCur;

#help_index "Doc"
