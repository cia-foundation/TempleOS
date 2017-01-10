#help_index "DolDoc/Output;StdOut/DolDoc"

CDocEntry *DocPutS(CDoc *doc,U8 *st)
{//Don't use this.  Use $LK-UL,"DocPrint",A="MN:DocPrint"$().
//Does not handle partial Doc entries.
  //Returns last newly created dollar-sign CDocEntry or NULL.
  U8 *ptr=st,*ptr2,*st2,*ptr3,*ptr4,*src,
	*char_bmp;
  Bool unlock;
  I64 ch,j;
  CDocEntry *doc_e=NULL,*res=NULL,*doc_ce;
  if (!st || !doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return NULL;
  unlock=DocLock(doc);
  if (doc->flags & DOCF_PLAIN_TEXT_TABS)
    char_bmp=char_bmp_zero_cr_nl_cursor;
  else if (doc->flags & DOCF_PLAIN_TEXT)
    char_bmp=char_bmp_zero_tab_cr_nl_cursor;
  else
    char_bmp=char_bmp_zero_tab_cr_nl_cursor_dollar;
  doc_ce=doc->cur_entry;
  while (*ptr) {
    ptr2=ptr;
    do ch=*ptr++;
    while (!Bt(char_bmp,ch) || ch==CH_CURSOR && doc->flags&DOCF_NO_CURSOR);
    ptr--;
    if (!ch) {
      if (j=ptr-ptr2) {
	doc_e=DocEntryNewBase(doc,
	      DOCT_TEXT|doc->settings_head.dft_text_attr<<8);
	if (doc->flags & DOCF_NO_CURSOR) {
	  src=MAlloc(j+1);
	  MemCpy(src,ptr2,j+1);
	  StrUtil(src,SUF_REM_CTRL_CHARS);
	  j=StrLen(src);
	} else
	  src=ptr2;
	doc_e->tag=MAlloc(j+1,doc->mem_task);
	MemCpy(doc_e->tag,src,j+1);
	doc_e->max_col=j;
	DocInsEntry(doc,doc_e);
	if (doc->flags & DOCF_NO_CURSOR)
	  Free(src);
      }
    } else {
      if (j=ptr-ptr2) {
	*ptr=0;
	doc_e=DocEntryNewBase(doc,
	      DOCT_TEXT|doc->settings_head.dft_text_attr<<8);
	if (doc->flags & DOCF_NO_CURSOR) {
	  src=MAlloc(j+1);
	  MemCpy(src,ptr2,j+1);
	  ptr3=src;
	  ptr4=src;
	  while (*ptr3)
	    if (*ptr3!=CH_CURSOR)
	      *ptr4++=*ptr3++;
	    else
	      ptr3++;
	  *ptr4=0;
	  j=ptr4-src;
	} else
	  src=ptr2;
	doc_e->tag=MAlloc(j+1,doc->mem_task);
	MemCpy(doc_e->tag,src,j+1);
	doc_e->max_col=j;
	DocInsEntry(doc,doc_e);
	if (doc->flags & DOCF_NO_CURSOR)
	  Free(src);
	*ptr=ch;
      }
      switch (ch) {
	case CH_CURSOR:
	  doc_e=DocEntryNewBase(doc,
		DOCT_CURSOR|doc->settings_head.dft_text_attr<<8);
	  DocInsEntry(doc,doc_e);
	  ptr++;
	  break;
	case '\t':
	  doc_e=DocEntryNewBase(doc,
		DOCT_TAB|doc->settings_head.dft_text_attr<<8);
	  DocInsEntry(doc,doc_e);
	  ptr++;
	  break;
	case '$$':
	  ptr++; //skip first dollar
	  ptr2=ptr;
	  while (*ptr && *ptr!='$$')
	    ptr++;
	  if (*ptr) {
	    *ptr=0; //zero second dollar
	    if (ptr-1==ptr2 && *ptr2==CH_CURSOR) {
	      doc_e=DocEntryNewBase(doc,
		    DOCT_CURSOR|doc->settings_head.dft_text_attr<<8);
	      DocInsEntry(doc,doc_e);
	      ptr2++;
	    }
	    if (ptr==ptr2) {
	      doc_e=DocEntryNewBase(doc,
		    DOCT_TEXT|doc->settings_head.dft_text_attr<<8);
	      doc_e->max_col=1;
	      if (doc->flags & DOCF_DBL_DOLLARS)
		doc_e->tag=StrNew("$$$$",doc->mem_task);
	      else
		doc_e->tag=StrNew("$$",doc->mem_task);
	      DocInsEntry(doc,doc_e);
	    } else {
	      st2=MAlloc(ptr-ptr2+1);
	      ptr3=ptr2;
	      ptr4=st2;
	      while (ch=*ptr3++) {
		if (ch==CH_CURSOR) {
		  doc_e=DocEntryNewBase(doc,
			DOCT_CURSOR|doc->settings_head.dft_text_attr<<8);
		  DocInsEntry(doc,doc_e);
		} else
		  *ptr4++=ch;
	      }
	      *ptr4=0;
	      if (doc_e=PrsDollarCmd(doc,st2)) {
		res=doc_e;
		DocInsEntry(doc,doc_e);
	      }
	      Free(st2);
	    }
	    *ptr++='$$';
	  }
	  break;
	default:
	  doc_e=DocEntryNewBase(doc,
		DOCT_NEW_LINE|doc->settings_head.dft_text_attr<<8);
	  DocInsEntry(doc,doc_e);
	  if (ch=='\r')
	    while (*ptr=='\r')
	      ptr++;
	  if (*ptr=='\n')
	    ptr++;
	  while (*ptr=='\r')
	    ptr++;
      }
    }
  }
  if (unlock)
    DocUnlock(doc);
  return res;
}

public CDocEntry *DocPrint(CDoc *doc=NULL,U8 *fmt,...)
{//You must not print partial doc cmds.
//Returns last newly created dollar-sign CDocEntry or NULL.
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  CDocEntry *res=DocPutS(doc,buf);
  Free(buf);
  return res;
}

public U0 DocPrintPartial(CDoc *doc=NULL,U8 *fmt,...)
{//Lets you print half a doc cmd, if you like.
  U8 *buf,*st,*src,*dst,*ptr,*ptr2;
  Bool unlock;
  CDocEntry *doc_ce,*doc_ne;
  I64 ch,i,j;
  if (!doc && !(doc=DocPut))
    return;
  buf=StrPrintJoin(NULL,fmt,argc,argv);
  ptr=buf;
  if (doc->user_put_s && (*doc->user_put_s)(doc,doc->user_put_data,buf)) {
    Free(buf);
    return;
  }
  unlock=DocLock(doc);
  if (doc->cur_entry->type_u8==DOCT_DATA)
    while (ch=*ptr++)
      DocPutKey(doc,ch,0);
  else
    while (ch=*ptr) {
      if (!Bt(char_bmp_safe_dollar,ch) ||
	    doc->flags & (DOCF_OVERSTRIKE|DOCF_IN_DOLLAR)) {
	DocPutKey(doc,ch,0);
	ptr++;
      } else {
	ptr2=ptr++;
	while (TRUE) {
	  ch=*ptr++;
	  if (!Bt(char_bmp_safe_dollar,ch))
	    break;
	}
	ptr--;
	*ptr=0;
	doc_ce=doc->cur_entry;
	j=ptr-ptr2;
	if (IsEditableText(doc_ce)) {
	  dst=st=MAlloc(doc_ce->max_col+j+1,doc->mem_task);
	  src=doc_ce->tag;
	  i=doc->cur_col;
	  doc->cur_col+=j;
	  doc_ce->max_col+=j;
	  while (i-->0)
	    *dst++=*src++;
	  while (j-->0)
	    *dst++=*ptr2++;
	  while (*dst++=*src++);
	  Free(doc_ce->tag);
	  doc_ce->tag=st;
	} else {
	  doc_ne=DocEntryNewTag(doc,doc_ce,ptr2);
	  doc_ne->type=DOCT_TEXT|doc->settings_head.dft_text_attr<<8;
	  doc_ne->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
	  QueIns(doc_ne,doc_ce->last);
	  doc->cur_entry=doc_ne;
	  doc->cur_col=StrLen(ptr2);
	}
	*ptr=ch;
	DocRemSoftNewLines(doc,doc->cur_entry);
      }
    }
  if (unlock)
    DocUnlock(doc);
  if (!(doc->flags&DOCF_DONT_SWAP_OUT))
    Yield;
  Free(buf);
}

Bool KDDocPutS(U8 *st)
{
  CDoc *doc;
  if (doc=DocPut)
    DocPrintPartial(doc,"%s",st);
  return FALSE;
}

public U0 DocPrintAtomic(CDoc *doc=NULL,U8 *fmt,...)
{//Prints multiple whole cmds all-at-once. Might need this when printing trees.
  U8 *buf;
  Bool unlock;
  I64 old_flags;
  if (!doc && !(doc=DocPut))
    return;
  buf=StrPrintJoin(NULL,fmt,argc,argv);
  unlock=DocLock(doc);
  old_flags=doc->flags;
  doc->flags|=DOCF_NO_CURSOR;
  DocPrint(doc,"%s",buf);
  DocRecalc(doc);
  doc->flags=old_flags;
  if (unlock)
    DocUnlock(doc);
  Free(buf);
}

U0 DocDump(CDoc *doc,I64 uS_delay=0)
{
  U8 *st;
  CDocEntry *doc_e,*doc_e2;
  Bool unlock=DocLock(doc);
  doc_e=doc->head.next;
  while (doc_e!=doc) {
    st=DocScanLine(doc,doc_e,NULL,&doc_e2);
    "%s",st;
    Free(st);
    doc_e=doc_e2;
    if (doc_e->type_u8==DOCT_NEW_LINE) {
      '\n';
      Busy(uS_delay);
      doc_e=doc_e->next;
    }
  }
  if (unlock)
    DocUnlock(doc);
}

public CDocEntry *DocPutLine(CDoc *doc=NULL,CDocEntry *doc_e)
{//Send line from other doc to StdOut $LK,"DocPut",A="MN:DocPut"$.
  I64 ch;
  U8 *ptr,*ptr2;
  Bool unlock;
  if (!doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return NULL;
  unlock=DocLock(doc);
  while (doc_e!=doc && doc_e->type_u8!=DOCT_NEW_LINE) {
    if (doc_e->de_flags&DOCEF_TAG) {
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
  '\n';
  if (doc_e!=doc)
    doc_e=doc_e->next;
  if (unlock)
    DocUnlock(doc);
  return doc_e;
}

#help_index "Debugging/Dump;DolDoc/Cmd Line (Typically);"\
	"Cmd Line (Typically);DolDoc/Output;StdOut/DolDoc"
public U0 DocDm(U8 *buf,I64 cnt=0x80)
{//Dump live chunk of mem showing addresses. Can be edited.
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
{//Dump live chunk of mem showing offsets. Can be edited.
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
