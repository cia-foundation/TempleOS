#help_index "DolDoc"

U0 EdUndoFilter(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->head.next;
  while (doc_ce!=doc) {
    doc_ce->de_flags&=~DOCEF_FILTER_SKIP;
    doc_ce=doc_ce->next;
  }
  doc->find_replace->filter_lines=0;
  if (unlock)
    DocUnlock(doc);
}

public I64 DocGetKey(I64 *_sc=NULL)
{//Called by $LK,"View",A="MN:View"$. You probably don't need this.
  I64 ch,sc;
  Bool cont,old_key_desc;
  do {
    old_key_desc=LBtr(&Fs->win_inhibit,WIf_SELF_KEY_DESC);
    do ch=GetKey(&sc,TRUE);
    while (sc&SCF_KEY_DESC);
    LBEqu(&Fs->win_inhibit,WIf_SELF_KEY_DESC,old_key_desc);

    DocRecalc(DocPut);
    cont=FALSE;
    if ((ch==CH_ESC || ch==CH_SHIFT_ESC) && //Check for exit filter mode
	  DocPut && DocPut->find_replace->filter_lines) {
      EdUndoFilter(DocPut);
      cont=TRUE;
    }
  } while (cont);
  if (_sc) *_sc=sc;
  return ch;
}

#help_index "DolDoc/Input;StdIn/DolDoc"
public U8 *DocScanLine(CDoc *doc,CDocEntry *doc_e,
	I64 *cur_col=NULL,CDocEntry **_do_e_end=NULL)
{//Start at entry in doc,make and return one line as str.
  CDocEntry *doc_e2=doc_e;
  Bool unlock=DocLock(doc);
  U8 *dst,*src,*start,*res;
  I64 i=0;
  if (cur_col)
    *cur_col=-1;
  while (doc_e2!=doc && doc_e2->type_u8!=DOCT_NEW_LINE) {
    if (doc_e2->de_flags&DOCEF_TAG && doc_e2->tag) {
      src=doc_e2->tag;
      i+=StrLen(src);
    } else if (doc_e2->type_u8==DOCT_TAB)
      i++;
    else if (doc_e2->type_u8==DOCT_SHIFTED_X ||
	  doc_e2->type_u8==DOCT_SHIFTED_Y) {
      if (doc_e2->attr<0)
	i++;
      i+=6; //$$SY,3$$
    }
    doc_e2=doc_e2->next;
  }
  res=MAlloc(i+1);
  dst=res;
  while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE) {
    start=dst;
    if (doc_e->de_flags&DOCEF_TAG && doc_e->tag) {
      src=doc_e->tag;
      while (*src)
	*dst++=*src++;
    } else if (doc_e->type_u8==DOCT_TAB)
      *dst++='\t';
    else if (doc_e->type_u8==DOCT_SHIFTED_Y) {
      *dst(U32 *)++='$$SY,';
      if (doc_e->attr<0)
	*dst++='-';
      *dst++='0'+AbsI64(doc_e->attr); //Supposedly -7 to 7 (single digit)
      *dst++='$$';
    } else if (doc_e->type_u8==DOCT_SHIFTED_X) {
      *dst(U32 *)++='$$SX,';
      if (doc_e->attr<0)
	*dst++='-';
      *dst++='0'+AbsI64(doc_e->attr); //Supposedly -7 to 7 (single digit)
      *dst++='$$';
    }
    if (doc_e==doc->cur_entry && cur_col)
      *cur_col=start-res+doc->cur_col;
    doc_e=doc_e->next;
  }
  if (_do_e_end) *_do_e_end=doc_e;
  *dst=0;
  if (doc_e==doc->cur_entry && cur_col && !doc->cur_col)
    *cur_col=dst-res;
  if (unlock)
    DocUnlock(doc);
  return res;
}

U8 *DocGetStr2(I64 flags=0)
{//$LK,"Flags",A="MN:GSF_SHIFT_ESC_EXIT"$
  CDoc *doc;
  CDocEntry *doc_e,*doc_e_end;
  U8 *st,*st2,*res;
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
      if (flags&GSF_WITH_NEW_LINE)
	break;
    } if (ch==CH_SHIFT_ESC) {
      if (flags&GSF_SHIFT_ESC_EXIT)
	Exit;
      else {
	res=StrNew("");
	goto gs_done;
      }
    }
//<CTRL-SHIFT-ENTER> is a blank line without entry
  } while (flags&GSF_WITH_NEW_LINE || ch!='\n' ||
	sc&SCF_CTRL && sc&SCF_SHIFT);

  doc=DocPut;
  DocLock(doc);
  if (flags&GSF_WITH_NEW_LINE) {
    doc_e=doc->cur_entry;
    do doc_e=doc_e->last;
    while (doc_e!=doc && doc_e->type_u8!=DOCT_PMT);
    doc_e=doc_e->next;
    if (res=DocScanLine(doc,doc_e,NULL,&doc_e_end)) {
      while (doc_e_end!=doc && doc_e_end->type_u8==DOCT_NEW_LINE) {
	st2=MStrPrint("%s\n",res);
	Free(res);
	res=st2;
	if (st=DocScanLine(doc,doc_e_end->next,NULL,&doc_e_end)) {
	  st2=MStrPrint("%s%s",res,st);
	  Free(st);
	  Free(res);
	  res=st2;
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
    res=DocScanLine(doc,doc_e,NULL);
  }
  DocUnlock(doc);
gs_done:
  "$$PT$$$$FG$$$$BG$$";
  return res;
}

#help_index "Keyboard Devices;Char/Input;StdIn"
public I64 GetI64(U8 *msg=NULL,I64 dft=0,I64 lo=I64_MIN,I64 hi=I64_MAX)
{//Prompt user for I64 expression.
  Bool okay;
  U8 *st;
  I64 res;
  while (TRUE) {
    if (msg)
      "" msg,dft;
    st=GetStr;
    if (!*st) {
      Free(st);
      return dft;
    }
    try {
      res=ExePrint2("ToI64(%s);",st);
      okay=TRUE;
    } catch {
      Fs->catch_except=TRUE;
      okay=FALSE;
    }
    Free(st);
    if (okay && lo<=res<=hi)
      return res;
  }
}

public F64 GetF64(U8 *msg=NULL,F64 dft=0,F64 lo=F64_MIN,F64 hi=F64_MAX)
{//Prompt user for F64 expression.
  Bool okay;
  U8 *st;
  F64 res;
  while (TRUE) {
    if (msg)
      "" msg,dft;
    st=GetStr;
    if (!*st) {
      Free(st);
      return dft;
    }
    try {
      res=ExePrint2("ToF64(%s);",st)(F64);
      okay=TRUE;
    } catch {
      Fs->catch_except=TRUE;
      okay=FALSE;
    }
    Free(st);
    if (okay && lo<=res<=hi)
      return res;
  }
}

public CDate GetDate(U8 *msg=NULL,CDate dft=I64_MIN,
	CDate lo=I64_MIN,CDate hi=I64_MAX)
{//Prompt user for date expression. (Use $LK,"Now",A="MN:Now"$() if you want current time.)
  U8 *st;
  CDate res;
  if (dft==I64_MIN) dft=Now;
  while (TRUE) {
    if (msg)
      "" msg,dft,dft;
    st=GetStr;
    if (!*st) {
      Free(st);
      return dft;
    }
    res=Str2Date(st);
    Free(st);
    if (res>=lo && res<=hi)
      return res;
  }
}
