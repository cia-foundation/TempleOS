#help_index "Doc/Edit"
public Bool DocLineNumGoTo(CDoc *doc,I64 line_num) //one based
{
  Bool result=FALSE,unlock;
  if (doc) {
    unlock=DocLock(doc);
    doc->x=0;
    doc->y=line_num-1;
    DocRecalc(doc,FALSE,DOC_RECALC_FIND_CURSOR);
    DocCenter(doc);
    if (doc->cur_entry->y==line_num-1)
      result=TRUE;
    if (unlock)
      DocUnlock(doc);
  }
  return result;
}


public Bool DocPatternFind(CDoc *doc,I64 start_line_num=MIN_I64,
			      U8 *pattern,I64 match=1)
{
  Bool result=FALSE,unlock;
  CDocEntry *doc_e;
  U8 *ptr;
  I64 i;
  if (doc) {
    unlock=DocLock(doc);
    if (start_line_num==MIN_I64) {
      result=TRUE;
      doc_e=doc->root.next;
    } else {
      result=DocLineNumGoTo(doc,start_line_num);
      doc_e=doc->cur_entry;
    }
    if (result) {
      if (pattern) {
	result=FALSE;
	while (doc_e!=doc) {
	  if (Bt(doc_tag_types,doc_e->type_u8) &&
		(ptr=StrIMatch(pattern,doc_e->tag))) { //TODO: handle multientry strings
	    i=ptr-doc_e->tag;
	    if (!--match) {
	      doc->cur_entry=doc_e;
	      if (i<doc_e->min_col) i=doc_e->min_col;
	      if (i>doc_e->max_col) i=doc_e->max_col;
	      doc->cur_data_col=i;
	      result=TRUE;
	      break;
	    }
	  }
	  doc_e=doc_e->next;
	}
      } else
	result=FALSE;
    }
    if (!result) DocBottom(doc);
    DocCenter(doc);
    if (unlock)
      DocUnlock(doc);
  }
  return result;
}

public Bool DocAnchorFind(CDoc *doc,U8 *pattern)
{
  Bool result=FALSE,unlock;
  CDocEntry *doc_e;
  if (doc) {
    unlock=DocLock(doc);
    doc_e=doc->root.next;
    if (pattern)
      while (doc_e!=doc) {
	if (doc_e->type_u8==DOCT_ANCHOR) {
	  if (!StrCmp(pattern,doc_e->aux_str)) {
	    doc->cur_entry=doc_e;
	    doc->cur_data_col=doc_e->min_col;
	    result=TRUE;
	    break;
	  }
	}
	doc_e=doc_e->next;
      }
    if (!result) DocBottom(doc);
    DocCenter(doc);
    if (unlock)
      DocUnlock(doc);
  }
  return result;
}

public I64 EdCurU8(CDoc *doc)
{
//See $LK,"EdRenumAsm","MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry;
  I64 result=-1;
  if (doc_ce->type_u8==DOCT_TEXT && doc_ce->min_col<=doc->cur_data_col<doc_ce->max_col)
    result=doc_ce->tag[doc->cur_data_col];
  else if (doc_ce->type_u8==DOCT_TAB)
    result='\t';
  else if (doc_ce->type_u8==DOCT_NEW_LINE || doc_ce->type_u8==DOCT_SOFT_NEW_LINE)
    result='\n';
  if (unlock)
    DocUnlock(doc);
  return result;
}

public U0 EdCursorLeft(CDoc *doc,I64 sc=MIN_I64)
{//You might need a call to $LK,"DocRecalc","MN:DocRecalc"$().
//See $LK,"EdRenumAsm","MN:EdRenumAsm"$ for an example.
  U8 *dst;
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*original_ce=doc_ce,*doc_ne;
  I64 cc=doc->cur_data_col,y=doc_ce->y;
  if (sc!=MIN_I64) sc=sc.u32[0];
  if (sc>=0 && sc&SCF_CTRL) {
    while (doc_ce->last!=doc && (doc_ce->last->y==y ||
	doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)))
      doc_ce=doc_ce->last;  //TODO: select? recurse?
    cc=doc_ce->min_col;
  } else {
    if (cc>doc_ce->min_col) {
      if (IsEditableText(doc_ce) && cc<doc_ce->max_col) {
	dst=doc_ce->tag+cc;
	doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
	*dst=0;
	doc_ce->max_col=cc;
	QueIns(doc_ne,doc_ce);
      }
      cc--;
      if (IsEditableText(doc_ce) && cc>doc_ce->min_col) {
	dst=doc_ce->tag+cc;
	doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
	*dst=0;
	doc_ce->max_col=cc;
	QueIns(doc_ne,doc_ce);
	doc_ce=doc_ne;
	cc=doc_ce->min_col;
      }
      if (sc>=0)
	BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
    } else {
      cc=doc_ce->min_col;
      while (doc_ce->last!=doc &&
	(doc_ce->last->type_u8==DOCT_SOFT_NEW_LINE || doc_ce->last->type_u8==DOCT_INDENT ||
	doc_ce->last->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
	doc_ce=doc_ce->last;
	if (sc>=0)
	  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
      }
      if (doc_ce->last!=doc) {
	doc_ce=doc_ce->last;
	if (doc_ce->max_col>doc_ce->min_col) {
	  cc=doc_ce->max_col-1;
	  if (IsEditableText(doc_ce) && cc>doc_ce->min_col) {
	    dst=doc_ce->tag+cc;
	    doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
	    *dst=0;
	    doc_ce->max_col=cc;
	    QueIns(doc_ne,doc_ce);
	    doc_ce=doc_ne;
	    cc=doc_ce->min_col;
	  }
	} else
	  cc=doc_ce->max_col;
	if (sc>=0)
	  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
      }
    }
  }
  doc->cur_data_col=cc;
  doc->cur_entry=doc_ce;
  if (doc_ce!=original_ce)
    DocFormBackward(doc);
  doc->recalc_start=doc->cur_entry->last;
  if (unlock)
    DocUnlock(doc);
}

public U0 EdCursorRight(CDoc *doc,I64 sc=MIN_I64)
{//You might need a call to $LK,"DocRecalc","MN:DocRecalc"$().
//See $LK,"EdRenumAsm","MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  U8 *dst;
  CDocEntry *doc_ce=doc->cur_entry,*original_ce=doc_ce,*doc_ne;
  I64 cc=doc->cur_data_col,y=doc_ce->y,old_flags1,old_flags2,old_color;
  doc->recalc_start=doc_ce->last;
  if (sc!=MIN_I64) sc=sc.u32[0];
  if (sc>=0 && sc&SCF_CTRL) {
    while (doc_ce!=doc && doc_ce->next->y==y &&
	   doc_ce->next->type_u8!=DOCT_SOFT_NEW_LINE && doc_ce->next!=doc && (doc_ce->next->type_u8!=DOCT_NEW_LINE || !(doc->flags & DOCF_FORM)) ||
	   doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))
      doc_ce=doc_ce->next;
    if (doc_ce->max_col>doc_ce->min_col)
      cc=doc_ce->max_col-1;
    else
      cc=doc_ce->min_col;
  } else {
    if (cc<doc_ce->max_col) {
      if (IsEditableText(doc_ce) && cc>doc_ce->min_col) {
	dst=doc_ce->tag+cc;
	doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
	*dst=0;
	doc_ce->max_col=cc;
	QueIns(doc_ne,doc_ce);
	doc_ce=doc_ne;
	cc=doc_ce->min_col;
      }
      cc++;
      old_flags1=doc_ce->flags1;
      old_flags2=doc_ce->flags2;
      old_color=doc_ce->type;
      if (sc>=0)
	BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
      if (IsEditableText(doc_ce) && cc<doc_ce->max_col) {
	dst=doc_ce->tag+cc;
	doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
	*dst=0;
	doc_ne->type=DOCT_TEXT | old_color & -0x100;
	doc_ne->flags1=old_flags1;
	doc_ne->flags2=old_flags2;
	doc_ce->max_col=cc;
	QueIns(doc_ne,doc_ce);
	doc_ce=doc_ne;
	cc=doc_ce->min_col;
      } else if (cc>=doc_ce->max_col) {
	doc_ce=doc_ce->next;
	cc=doc_ce->min_col;
      }
    } else {
      if (doc_ce!=doc) {
	if (cc<=doc_ce->min_col && sc>=0)
	  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
	doc_ce=doc_ce->next;
	while (doc_ce!=doc && doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)) {
	  if (sc>=0)
	    BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
	  doc_ce=doc_ce->next;
	}
	cc=doc_ce->min_col;
	if (doc_ce->type_u8==DOCT_SOFT_NEW_LINE) {
	  if (sc>=0)
	    BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
	  doc_ce=doc_ce->next;
	  cc=doc_ce->min_col;
	}
      }
    }
  }
  doc->cur_data_col=cc;
  doc->cur_entry=doc_ce;
  if (doc_ce!=original_ce)
    DocFormFwd(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 EdLineUp(CDoc *doc,I64 sc=MIN_I64)
{//You might need a call to $LK,"DocRecalc","MN:DocRecalc"$().
//See $LK,"EdRenumAsm","MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  U8 *dst;
  I64 y,x;
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;

  if (sc!=MIN_I64) sc=sc.u32[0];
  if (doc_ce->type_u8==DOCT_HEX_ED) {
    doc->cur_data_col=doc->cur_data_col-doc_ce->hex_ed_width*3;
    if (doc->cur_data_col>=0) {
      if (unlock)
	DocUnlock(doc);
      return;
    } else
      doc->cur_data_col=0;
  }
  x=doc->x; y=doc->y;
  if (IsEditableText(doc_ce)) {
    if (doc_ce->min_col<doc->cur_data_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_data_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      doc_ne->x=doc_ce->x+doc->cur_data_col;
      doc_ce->max_col=doc->cur_data_col;
      QueIns(doc_ne,doc_ce);
    } else
      if (doc->cur_data_col==doc_ce->min_col && doc_ce->last!=doc)
	doc_ce=doc_ce->last;
  } else
    if (doc_ce->last!=doc)
      doc_ce=doc_ce->last;
  if (sc>=0)
    BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
  doc->cur_entry=doc_ce;
  DocFormBackward(doc);
  doc_ce=doc->cur_entry;
  while (doc_ce->last!=doc && (doc_ce->y>=y || doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
    doc_ce=doc_ce->last;
    if (sc>=0)
      BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
  }
  y=doc_ce->y;
  doc->y=y;
  while (doc_ce!=doc && (doc_ce->y>=y && doc_ce->x>=x || doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
    if (sc>=0)
      BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
    doc_ce=doc_ce->last;
  }

  if (doc_ce==doc || doc_ce->y<y)
    doc_ce=doc_ce->next;
  else {
    if (!IsEditableText(doc_ce)) {
      if (sc>=0)
	BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
    } else {
      if (doc_ce->next->x==x) {
	doc_ce=doc_ce->next;
	if (doc->flags & DOCF_FORM) {
	  while (doc_ce->next->x==x &&
	    (!Bt(doc_form_types,doc_ce->type_u8) ||
	     doc_ce->flags1&DOCEF1_SKIP_IN_FORM))
	    doc_ce=doc_ce->next;
	}
      }
    }
  }
  if (Bt(doc_tag_types,doc_ce->type_u8)) {
    doc->cur_data_col=x-doc_ce->x;
    if (IsEditableText(doc_ce)) {
      if (doc->cur_data_col>doc_ce->max_col)
	doc->cur_data_col=doc_ce->max_col;
    } else {
      if (doc->cur_data_col>=doc_ce->max_col)
	doc->cur_data_col=doc_ce->max_col-1;
    }
    if (doc->cur_data_col<doc_ce->min_col)
      doc->cur_data_col=doc_ce->min_col;
  } else {
    if (doc_ce->type_u8==DOCT_HEX_ED) {
      doc->cur_data_col=RoundI64((doc_ce->len-1)*3,doc_ce->hex_ed_width*3);
      if (doc->cur_data_col<0)
	doc->cur_data_col=0;
    } else
      doc->cur_data_col=doc_ce->min_col;
  }
  if (IsEditableText(doc_ce) && doc_ce->x<x) {
    if (doc->cur_data_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_data_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      if (sc>=0) {
	if (sc&SCF_SHIFT)
	  doc_ne->type=doc_ce->type | DOCET_SELECTED;
	else
	  doc_ne->type=doc_ce->type & ~DOCET_SELECTED;
      }
      doc_ne->x=doc_ce->x+doc->cur_data_col;
      doc_ce->max_col=doc->cur_data_col;
      QueIns(doc_ne,doc_ce);
      doc_ce=doc_ne;
      doc->cur_data_col=doc_ce->min_col;
    }
  }
  doc->cur_entry=doc_ce;
  DocFormFwd(doc);
  doc->x=doc->cur_entry->x+doc->cur_data_col;
  doc->recalc_start=doc->cur_entry->last;
  if (unlock)
    DocUnlock(doc);
}

public U0 EdLineDown(CDoc *doc,I64 sc=MIN_I64)
{//You might need a call to $LK,"DocRecalc","MN:DocRecalc"$().
//See $LK,"EdRenumAsm","MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  U8 *dst;
  I64 y,x,old_flags1=0,old_flags2=0,old_color;
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne,*doc_ce2;
  if (sc!=MIN_I64) sc=sc.u32[0];
  if (doc_ce->type_u8==DOCT_HEX_ED) {
    doc->cur_data_col=doc->cur_data_col+doc_ce->hex_ed_width*3;
    if (doc->cur_data_col>=doc_ce->len*3) {
      doc->cur_entry=doc_ce=doc_ce->next;
      doc->cur_data_col=doc_ce->min_col;
      doc->x=doc_ce->x+doc->cur_data_col;
      doc->y=doc_ce->y;
    }
    if (unlock)
      DocUnlock(doc);
    return;
  }
  doc->recalc_start=doc_ce->last;
  x=doc->x; y=doc->y;
  if (IsEditableText(doc_ce)) {
    if (doc->cur_data_col>doc_ce->min_col && doc->cur_data_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_data_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      if (sc>=0) {
	if (sc&SCF_SHIFT)
	  doc_ne->type=doc_ce->type | DOCET_SELECTED;
	else
	  doc_ne->type=doc_ce->type & ~DOCET_SELECTED;
      }
      doc_ne->x=doc_ce->x+doc->cur_data_col;
      doc_ce->max_col=doc->cur_data_col;
      QueIns(doc_ne,doc_ce);
      doc_ce=doc_ne;
      doc->cur_data_col=doc_ce->min_col;
    }
  }
  doc_ce2=doc_ce;
  while (doc_ce!=doc && (doc_ce->y<=y || doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)))
    doc_ce=doc_ce->next;
  y=doc_ce->y;
  doc->y=y;
  while (doc_ce!=doc && (doc_ce->y<=y && doc_ce->x<=x || doc_ce->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
    old_flags1=doc_ce->flags1;
    old_flags2=doc_ce->flags2;
    old_color=doc_ce->type;
    doc_ce=doc_ce->next;
  }
  if (doc_ce->last!=doc && (doc_ce->x>x || doc_ce->y>y)) {
    doc_ce=doc_ce->last;
    doc->cur_entry=doc_ce;
    if (!((doc_ce->type_u8==DOCT_NEW_LINE || doc_ce->type_u8==DOCT_SOFT_NEW_LINE || doc_ce->type_u8==DOCT_INDENT) &&
	  (doc_ce->last->type_u8==DOCT_NEW_LINE || doc_ce->last->type_u8==DOCT_SOFT_NEW_LINE || doc_ce->last->type_u8==DOCT_INDENT)))
      DocFormBackward(doc);
    doc_ce=doc->cur_entry;
  }
  while (doc_ce2!=doc && (doc_ce2!=doc_ce || IsEditableText(doc_ce))) {
    if (doc_ce2->y<y || doc_ce2->x<x || doc_ce2->flags1 & (DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP) ||
       doc_ce2->x==x && !doc_ce2->max_col &&
       Bt(doc_nontag_invisible_types,doc_ce2->type_u8)) {
      if (sc>=0)
	BEqu(&doc_ce2->type,DOCEt_SELECTED,sc&SCF_SHIFT);
    }
    if (doc_ce2==doc_ce) break;
    doc_ce2=doc_ce2->next;
  }
  if (Bt(doc_tag_types,doc_ce->type_u8)) {
    doc->cur_data_col=x-doc_ce->x;
    if (IsEditableText(doc_ce)) {
      if (doc->cur_data_col>doc_ce->max_col)
	doc->cur_data_col=doc_ce->max_col;
    } else {
      if (doc->cur_data_col>=doc_ce->max_col)
	doc->cur_data_col=doc_ce->max_col-1;
    }
    if (doc->cur_data_col<doc_ce->min_col)
      doc->cur_data_col=doc_ce->min_col;
  } else
    doc->cur_data_col=doc_ce->min_col;
  if (IsEditableText(doc_ce)) {
    if (doc_ce->min_col<doc->cur_data_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_data_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      doc_ne->type=DOCT_TEXT | old_color & -0x100;
      doc_ne->flags1=old_flags1;
      doc_ne->flags2=old_flags2;
      doc_ce->max_col=doc->cur_data_col;
      doc_ne->x=doc_ce->x+doc->cur_data_col;
      QueIns(doc_ne,doc_ce);
      doc_ce=doc_ne;
      doc->cur_data_col=doc_ce->min_col;
    }
  }
  doc->cur_entry=doc_ce;
  DocFormFwd(doc);
  if (!(doc->flags & DOCF_FORM))
    while (doc_ce!=doc && doc_ce!=doc->cur_entry) {
      if (sc>=0)
	BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
      doc_ce=doc_ce->next;
    }
  doc->x=doc->cur_entry->x+doc->cur_data_col;
  if (unlock)
    DocUnlock(doc);
}

U0 EdDelChar(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry;

  if (doc_ce==doc) {
    if (unlock)
      DocUnlock(doc);
    return;
  } else
    doc->recalc_start=doc_ce->last;
  if (doc_ce->max_col!=0 && (IsEditableText(doc_ce)||doc_ce->type_u8==DOCT_DATA)) {
    if (doc_ce->type_u8==DOCT_DATA) {
      if (doc_ce->flags1 & DOCEF1_HAS_TERMINATOR && doc->cur_data_col==doc_ce->max_col-1) {
	if (unlock)
	  DocUnlock(doc);
	return;
      }
    }
    if (doc->cur_data_col<doc_ce->max_col)
      StrCpy(doc_ce->tag+doc->cur_data_col,doc_ce->tag+doc->cur_data_col+1);
    if (doc->cur_data_col>=doc_ce->max_col-1) {
      doc->cur_entry=doc_ce->next;
      doc->cur_data_col=doc->cur_entry->min_col;
    }
    DocRemSoftNewLines(doc,doc->cur_entry);
    if (unlock)
      DocUnlock(doc);
    return;
  }
  doc->cur_entry=doc_ce->next;
  doc->cur_data_col=doc->cur_entry->min_col;
  if (!(doc_ce->flags1&DOCEF1_FILTER_SKIP))
    DocEntryDel(doc,doc_ce);
  DocRemSoftNewLines(doc,doc->cur_entry);
  if (unlock)
    DocUnlock(doc);
}

U0 ChkDollarBufSize(CDoc *doc)
{
  U8 *b;
  if (doc->dollar_buf_ptr>=doc->dollar_buf_size-2) {
    doc->dollar_buf_size<<=1;
    b=MAlloc(doc->dollar_buf_size,doc->mem_task);
    MemCpy(b,doc->dollar_buf,doc->dollar_buf_ptr);
    Free(doc->dollar_buf);
    doc->dollar_buf=b;
  }
}


U0 EdInsChar(I64 ch,I64 sc,CDoc *doc)
{
  Bool unlock=DocLock(doc);
  U8 *b,*src,*dst;
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;
  I64 i,j,m,y=doc_ce->y;

  doc->recalc_start=doc_ce->last;
  while (doc->recalc_start!=doc && doc->recalc_start->flags1&(DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))
    doc->recalc_start=doc->recalc_start->last;

  if (doc->flags & DOCF_IN_DOLLAR) {
    if (!Bt(getkey_chars_bitmap,ch))
      goto ic_done;
    ChkDollarBufSize(doc);
    doc->dollar_buf[doc->dollar_buf_ptr++]=ch;
    if (ch=='$$') {
      if (doc->dollar_buf_ptr==2) {
	doc->flags&=~DOCF_IN_DOLLAR;
	doc->dollar_buf_ptr=0;
	goto ic_cont;
      } else {
	doc->dollar_buf[doc->dollar_buf_ptr]=0;
	DocPrint(doc,doc->dollar_buf);
	doc->flags&=~DOCF_IN_DOLLAR;
	doc->dollar_buf_ptr=0;
	goto ic_done;
      }
    } else
      goto ic_done;
  }
  if (ch=='$$') {
    if (!(doc->flags & (DOCF_PLAIN_TEXT | DOCF_PLAIN_TEXT_WITH_TABS))) {
      doc->flags|=DOCF_IN_DOLLAR;
      doc->dollar_buf_ptr=0;
      doc->dollar_buf[doc->dollar_buf_ptr++]=ch;
      goto ic_done;
    }
  }
  if (ch=='\r') goto ic_done;

ic_cont:
  if ((ch==CH_SPACE || ch=='\n') &&
      !(sc & (SCF_CTRL|SCF_SHIFT)) &&
       doc_ce->flags1 & (DOCEF1_LINK|DOCEF1_TREE|DOCEF1_CHECK_COLLAPSABLE|
       DOCEF1_LEFT_MACRO|DOCEF1_LEFT_EXP|DOCEF1_LEFT_CB|DOCEF1_LEFT_AUTO |
       DOCEF1_RIGHT_MACRO|DOCEF1_RIGHT_EXP|DOCEF1_RIGHT_CB|DOCEF1_RIGHT_AUTO)) {
    doc->cmd_U8=ch;
    DocEntryRun(doc,doc_ce,FALSE);
    DocLock(doc);
    goto ic_done;
  }
  if (doc_ce->type_u8==DOCT_HEX_ED) {
    if (doc_ce->flags1 & DOCEF1_DATA_IS_PTR)
      b=doc_ce->data;
    else
      b=&doc_ce->data;
    i=doc->cur_data_col;
    j=i%(doc_ce->hex_ed_width*3);
    m=i/(doc_ce->hex_ed_width*3)*doc_ce->hex_ed_width;
    if (j>=doc_ce->hex_ed_width<<1)
      b[j-doc_ce->hex_ed_width<<1+m]=ch;
    else {
      ch=ToUpper(ch)-'0';
      if (ch>9) {
	ch+='0'-'A'+10;
	if (!(10<=ch<=15))
	  goto ic_done;
      }
      m=j>>1+m;
      if (j & 1)
	b[m]=b[m] & 0xF0| ch;
      else
	b[m]=b[m] & 0xF | ch<<4;
    }
    doc->cur_data_col++;
    goto ic_done;
  }
  if (doc->flags & DOCF_OVERSTRIKE) {
    if (Bt(displayable_chars_bitmap,ch)) {
ic_overstrike_char:
      if (IsEditableText(doc_ce)) {
	if (doc->cur_data_col<doc_ce->max_col) {
	  if (doc_ce->tag[doc->cur_data_col]) {
	    doc_ce->tag[doc->cur_data_col++]=ch;
	    goto ic_done;
	  }
	} else {
	  doc_ce=doc_ce->next;
	  doc->cur_entry=doc_ce;
	  doc->cur_data_col=doc_ce->min_col;
	  goto ic_overstrike_char;
	}
      } else if (doc_ce->type_u8==DOCT_DATA) {
	if (doc_ce->flags1 & DOCEF1_HAS_TERMINATOR) {
	  if (doc_ce->tag[doc->cur_data_col] &&
	     doc->cur_data_col<doc_ce->min_col+doc_ce->len) {
	    doc_ce->tag[doc->cur_data_col++]=ch;
	    if ( ! doc_ce->tag[doc->cur_data_col]) {
	      doc_ce->tag[doc->cur_data_col]='_';
	      doc_ce->tag[doc->cur_data_col+1]=0;
	    }
	  }
	} else {
	  if (doc_ce->tag[doc->cur_data_col])
	     doc_ce->tag[doc->cur_data_col++]=ch;
	}
	goto ic_done;
      }
      b=MAlloc(2,doc->mem_task);
      *b=ch;
      b[1]=0;
      doc_ne=DocEntryNew(doc,doc_ce,b);
      doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
      doc_ne->flags1=0;
      doc_ne->flags2=0;
      QueIns(doc_ne,doc_ce->last);
    } else if (ch=='\n') {
      while (doc->cur_entry->next!=doc && doc->cur_entry->y==y)
	doc->cur_entry=doc->cur_entry->next;
      doc->cur_data_col=doc->cur_entry->min_col;
    } else if (ch=='\t') {
      if (doc->flags&DOCF_FORM)
	goto ic_form_tab;
    }
    goto ic_done;
  }
  if (ch=='\n') {
    doc_ne=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
    if (sc&SCF_CTRL && !(sc&SCF_SHIFT))
      doc_ne->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr << 8;
    else
      doc_ne->type=DOCT_NEW_LINE | doc->settings_root.dft_text_attr << 8;
    DocIns(doc,doc_ne);
  } else if (ch=='\t') {
    if (doc->flags&DOCF_FORM &&
       Bt(doc_form_types,doc->cur_entry->type_u8) &&
       !(doc->cur_entry->flags1&DOCEF1_SKIP_IN_FORM)) {
ic_form_tab:
      doc->cur_entry=doc->cur_entry->next;
      doc->cur_data_col=doc->cur_entry->min_col;
      DocFormFwd(doc);
      goto ic_done;
    } else {
      doc_ne=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
      doc_ne->type=DOCT_TAB | doc->settings_root.dft_text_attr << 8;
      DocIns(doc,doc_ne);
    }
  } else {
    if (Bt(displayable_chars_bitmap,ch)) {
      if (doc_ce->type_u8==DOCT_DATA) {
	i=doc_ce->len+doc_ce->min_col;
	if (doc_ce->flags1 & DOCEF1_HAS_TERMINATOR)
	  i++;
	if (i>doc_ce->max_col) {
	  b=doc_ce->tag;
	  doc_ce->max_col++;
	  for (i=doc_ce->max_col;i>doc->cur_data_col;i--)
	    b[i]=b[i-1];
	  b[doc->cur_data_col++]=ch;
	}
      } else if (IsEditableText(doc_ce)) {
	dst=b=MAlloc(doc_ce->max_col+2,doc->mem_task);
	src=doc_ce->tag;
	i=doc->cur_data_col;
	while (i-->0)
	  *dst++=*src++;
	*dst++=ch;
	while (*dst++=*src++);
	Free(doc_ce->tag);
	doc_ce->tag=b;
	doc_ce->max_col++;
	doc->cur_data_col++;
      } else {
	b=MAlloc(2,doc->mem_task);
	*b=ch;
	b[1]=0;
	doc_ne=DocEntryNew(doc,doc_ce,b);
	doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
	doc_ne->flags1=0;
	doc_ne->flags2=0;
	doc_ne->x=doc_ce->x+1;
	QueIns(doc_ne,doc_ce->last);
      }
    }
  }
ic_done:
  DocRemSoftNewLines(doc,doc->cur_entry);
  if (doc->cur_entry->flags1 & DOCEF1_UPDATE_DATA) {
    if (doc->cur_entry->type_u8==DOCT_DATA || doc->cur_entry->type_u8==DOCT_CHECK_BOX)
    DocDataScan(doc->cur_entry);
  }
  if (unlock)
    DocUnlock(doc);
}

U0 EdDelLine(CDoc *doc)
{
  CDocEntry *doc_ce=doc->cur_entry,*doc_ce2;
  I64 y;
  y=doc->y;
  while (doc_ce!=doc && doc_ce->y==y)
    doc_ce=doc_ce->next;
  doc->cur_entry=doc_ce;
  doc->cur_data_col=doc_ce->min_col;
  doc_ce=doc_ce->last;
  while (doc_ce!=doc && doc_ce->y==y) {
    doc_ce2=doc_ce->last;
    if (!(doc_ce->flags1&DOCEF1_FILTER_SKIP))
      DocEntryDel(doc,doc_ce);
    doc_ce=doc_ce2;
  }
  if (doc_ce==doc)
    doc->recalc_start=doc_ce;
  else
    doc->recalc_start=doc_ce->last;
}

public U0 EdFindNext(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  U8 *ptr,*ptr2,*ptr3;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e=doc_ce;
  I64 sf_flags;
  if (doc->find_replace->match_case)
    sf_flags=0;
  else
    sf_flags=SFF_IGNORE_CASE;
  if (doc->find_replace->whole_labels)
    sf_flags|=SFG_WHOLE_LABELS;
  do {
    if (doc_e!=doc) {
      if (Bt(doc_tag_types,doc_e->type_u8)&&
	!(doc_e->flags1&(DOCEG1_DONT_EDIT|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)) &&
	!(doc_e->flags2&DOCEG2_DONT_EDIT)) {
	if (doc_e->type & DOCET_SELECTED || !doc->find_replace->scan_selected_text) {
	  if (doc->find_replace->scan_fwd) {
	    if (doc_e==doc_ce) {
	      ptr=doc_ce->tag+doc->cur_data_col+1;
	      if (ptr-doc_ce->tag>=doc_ce->max_col)
		goto fn_skip;
	      if (ptr-doc_ce->tag<doc_ce->min_col)
		ptr=doc_ce->tag+doc_ce->min_col;
	    } else
	      ptr=doc_e->tag;
	    if (ptr=StrFind(doc->find_replace->find_text,ptr,sf_flags)) {
	      doc->cur_entry=doc_e;
	      doc->cur_data_col=ptr-doc_e->tag;
	      if (doc->cur_data_col>=doc_e->max_col)
		doc->cur_data_col=doc_e->max_col-1;
	      if (doc->cur_data_col<doc_e->min_col)
		doc->cur_data_col=doc_e->min_col;
	      DocCenter(doc);
	      if (unlock)
		DocUnlock(doc);
	      return;
	    }
	  } else {
	    ptr2=NULL;
	    ptr=doc_e->tag+doc_e->min_col;
	    if (doc_e==doc_ce)
	      ptr3=doc_ce->tag+doc->cur_data_col;
	    else
	      ptr3=doc_e->tag+doc_e->max_col;
	    while (ptr=StrFind(doc->find_replace->find_text,ptr,sf_flags)) {
	      if (ptr>=ptr3)
		break;
	      ptr2=ptr++;
	    }
	    if (ptr2 && ptr2<ptr3) {
	      doc->cur_entry=doc_e;
	      doc->cur_data_col=ptr2-doc_e->tag;
	      if (doc->cur_data_col>=doc_e->max_col)
		doc->cur_data_col=doc_e->max_col-1;
	      if (doc->cur_data_col<doc_e->min_col)
		doc->cur_data_col=doc_e->min_col;
	      DocCenter(doc);
	      if (unlock)
		DocUnlock(doc);
	      return;
	    }
	  }
	}
      }
    }
fn_skip:
    if (doc->find_replace->scan_fwd)
      doc_e=doc_e->next;
    else
      doc_e=doc_e->last;
  } while (doc_e!=doc_ce);
  if (unlock)
    DocUnlock(doc);
}

public U0 EdSelectAll(CDoc *doc,Bool select)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_e=doc->root.next;
  while (doc_e!=doc) {
    BEqu(&doc_e->type,DOCEt_SELECTED,select);
    doc_e=doc_e->next;
  }
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/IDE"
public Bool EdFindPaired(CDoc *doc,U8 plus,U8 minus,Bool fwd,Bool abort_on_dbl_colon=FALSE)
{
  Bool unlock=DocLock(doc),result=FALSE;
  U8 ch,*ptr,original_col=doc->cur_data_col;
  I64 levels=0,colons=0;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e=doc_ce,*original_ce=doc_ce;
  if (abort_on_dbl_colon && EdCurU8(doc)==':')
    colons=1;
  else
    colons=0;
  do {
    if (doc_e!=doc) {
      if (Bt(doc_tag_types,doc_e->type_u8)&&!(doc_e->flags1&(DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
	if (fwd) {
	  if (doc_e==doc_ce)
	    ptr=doc_e->tag+doc->cur_data_col+1;
	  else
	    ptr=doc_e->tag;
	  if (ptr-doc_e->tag<doc_e->min_col)
	    ptr=doc_e->tag+doc_e->min_col;
	  if (ptr-doc_e->tag>=doc_e->max_col)
	    goto pa_skip;
	  while (ch=*ptr++)
	    if (abort_on_dbl_colon && ch==':') {
	      if (++colons==2) {
		doc->cur_entry=doc_e;
		doc->cur_data_col=ptr-doc_e->tag-1;
		EdCursorLeft(doc);
		result=FALSE;
		goto pa_done;
	      }
	    } else {
	      colons=0;
	      if (ch==plus)
		levels++;
	      else if (ch==minus) {
		if (!levels--) {
		  doc->cur_entry=doc_e;
		  doc->cur_data_col=ptr-doc_e->tag-1;
		  result=doc->cur_entry!=original_ce || doc->cur_data_col!=original_col;
		  goto pa_done;
		}
	      }
	    }
	} else {
	  if (doc_e==doc_ce) {
	    ptr=doc_e->tag+doc->cur_data_col-1;
	    if (ptr-doc_e->tag>=doc_e->max_col)
	      ptr=doc_e->tag+doc_e->max_col-1;
	  } else
	    ptr=doc_e->tag+doc_e->max_col-1;
	  if (ptr-doc_e->tag<doc_e->min_col)
	    goto pa_skip;
	  while (ptr>=doc_e->tag+doc_e->min_col) {
	    ch=*ptr--;
	    if (abort_on_dbl_colon && ch==':') {
	      if (++colons==2) {
		doc->cur_entry=doc_e;
		doc->cur_data_col=ptr-doc_e->tag+1;
		result=FALSE;
		goto pa_done;
	      }
	    } else {
	      colons=0;
	      if (ch==plus)
		levels++;
	      else if (ch==minus) {
		if (!levels--) {
		  doc->cur_entry=doc_e;
		  doc->cur_data_col=ptr-doc_e->tag+1;
		  result=doc->cur_entry!=original_ce || doc->cur_data_col!=original_col;
		  goto pa_done;
		}
	      }
	    }
	  }
	}
      }
    }
pa_skip:
    if (fwd)
      doc_e=doc_e->next;
    else
      doc_e=doc_e->last;
  } while (doc_e!=doc_ce);
pa_done:
  DocRecalc(doc);
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
  return result;
}

public Bool EdGoToFun(CDoc *doc,Bool fwd,Bool abort_on_dbl_colon)
{
  Bool unlock=DocLock(doc),result=FALSE;
  I64 ch,levels,colons;
  if (fwd) {
    levels=0;
    colons=0;
    while (doc->cur_entry!=doc) {
      ch=EdCurU8(doc);
      if (abort_on_dbl_colon && ch==':') {
	if (++colons==2) {
	  EdCursorLeft(doc);
	  break;
       }
      } else {
	colons=0;
	if (ch=='{')
	  levels++;
	else if (ch=='}' && !levels--)
	  break;
      }
      EdCursorRight(doc);
    }
    DocRecalc(doc);
    if (doc->cur_entry!=doc)
      result=TRUE;
  } else {
    while (EdFindPaired(doc,'}','{',FALSE,abort_on_dbl_colon));
    if (doc->cur_entry!=doc) {
      ch=EdCurU8(doc);
      if (abort_on_dbl_colon && ch==':')
	result=TRUE;
      else {
	if (ch=='{')
	  result=TRUE;
      }
    }
  }
  if (unlock)
    DocUnlock(doc);
  return result;
}

public U0 EdSelectFun(CDoc *doc,Bool abort_on_dbl_colon=FALSE)
{
  Bool unlock=DocLock(doc);
  U8 ch,*ptr;
  I64 levels=0,colons=0;
  CDocEntry *doc_e;
  EdSelectAll(doc,FALSE);
  EdGoToFun(doc,FALSE,abort_on_dbl_colon);
  if (EdCurU8(doc)=='{')
    levels--;
  else if (abort_on_dbl_colon && EdCurU8(doc)==':') {
    EdCursorRight(doc);
    if (EdCurU8(doc)==':')
      EdCursorRight(doc);
  }
  doc_e=doc->cur_entry;
  while (doc_e!=doc) {
    doc_e->type|=DOCET_SELECTED;
    if (doc_e->type_u8==DOCT_TEXT) {
      ptr=doc_e->tag;
      if (doc_e==doc->cur_entry)
	ptr+=doc->cur_data_col;
      while (ch=*ptr++)
	if (abort_on_dbl_colon && ch==':') {
	  if (++colons==2)
	    goto sf_done;
	} else {
	  colons=0;
	  if (ch=='{')
	    levels++;
	  else if (ch=='}' && !levels--)
	    goto sf_done;
	}
    }
    doc_e=doc_e->next;
  }
sf_done:
  DocRecalc(doc);
  if (unlock)
    DocUnlock(doc);
}

#help_index "Doc/Edit"
U0 EdUndoFilter(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->root.next;
  while (doc_ce!=doc) {
    doc_ce->flags1&=~DOCEF1_FILTER_SKIP;
    doc_ce=doc_ce->next;
  }
  doc->find_replace->filter_lines=0;
  if (unlock)
    DocUnlock(doc);
}

I64 EdFindReplace(CDoc *doc)
{
  Bool first=TRUE,found,unlock;
  I64 cmd,i,j,plen,rlen,dlen,result=-1,sf_flags;
  U8 *src,*dst,*dst2;
  CDocEntry *doc_ce=doc->cur_entry,*doc_e;
  if (doc->find_replace->pmt)
    cmd=RSAC_REPLACE;
  else
    cmd=RSAC_ALL;
  if (!doc->find_replace->pmt || DocFormDo(doc->find_replace)) {
    result=0;
    unlock=DocLock(doc);
    if (doc->find_replace->match_case || doc->find_replace->local_var)
      sf_flags=0;
    else
      sf_flags=SFF_IGNORE_CASE;
    if (doc->find_replace->whole_labels || doc->find_replace->local_var)
      sf_flags|=SFG_WHOLE_LABELS;

    if (i=doc->find_replace->filter_lines) {
      doc_ce=doc->root.next;
      while (doc_ce!=doc) {
	if (doc_ce->type_u8==DOCT_TEXT && !(doc_ce->flags1&(DOCEF1_TAG_CB|DOCEF1_DEFINE|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))&&
	  StrFind(doc->find_replace->find_text,doc_ce->tag,sf_flags)) {
	  doc_ce->type|=DOCET_SELECTED;
	  result++;
	} else
	  doc_ce->type&=~DOCET_SELECTED;
	doc_ce=doc_ce->next;
      }
 
      doc_ce=doc->root.next;
      while (doc_ce!=doc) {
	if (!(doc_ce->flags1&(DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
	  found=FALSE;

	  doc_e=doc_ce;
	  while (doc_e!=doc && doc_e->y>doc_ce->y-i) {
	    if (doc_e->type&DOCET_SELECTED) {
	      found=TRUE;
	      break;
	    } else
	      doc_e=doc_e->last;
	  }

	  if (!found) {
	    doc_e=doc_ce;
	    while (doc_e!=doc && doc_e->y<doc_ce->y+i) {
	      if (doc_e->type&DOCET_SELECTED) {
		found=TRUE;
		break;
	      } else
		doc_e=doc_e->next;
	    }
	  }

	  if (!found)
	    doc_ce->flags1|=DOCEF1_FILTER_SKIP;
	}

	doc_ce=doc_ce->next;
      }
      EdSelectAll(doc,FALSE);
      goto fr_unlock_done;
    }

    if (doc->find_replace->local_var) {
      EdSelectFun(doc);
      doc_ce=doc->cur_entry;
    }

    if (!doc->find_replace->replace && !doc->find_replace->local_var) {
      EdFindNext(doc);
      goto fr_unlock_done;
    }
    plen=StrLen(doc->find_replace->find_text);
    if (!plen)
      goto fr_unlock_done;
    rlen=StrLen(doc->find_replace->replace_text);
    doc_e=doc_ce;
    while (doc_e!=doc_ce || first) {
      first=FALSE;
      if (doc_e->type_u8==DOCT_TEXT &&
	  !(doc_e->flags1&(DOCEG1_DONT_EDIT|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP)) &&
	  !(doc_e->flags2&DOCEG2_DONT_EDIT) &&
	   (doc_e->type & DOCET_SELECTED || !doc->find_replace->scan_selected_text&&!doc->find_replace->local_var)) {
	src=doc_e->tag;
	while (src) {
	  src=StrFind(doc->find_replace->find_text,src,sf_flags);
	  if (src) {
	    doc->cur_data_col=src-doc_e->tag;
	    doc->cur_entry=doc_e;
	    if (cmd!=RSAC_ALL)
	      DocCenter(doc);
	    doc_e=doc->cur_entry;
	    src=doc->cur_data_col+doc_e->tag;
	    if (cmd!=RSAC_ALL) {
	      DocUnlock(doc);
	      cmd=PopUpReplaceSkipAllCancel("");
	      DocLock(doc);
	      if (cmd<0)
		goto fr_unlock_done;
	    }
	    if (cmd==RSAC_REPLACE || cmd==RSAC_ALL) {
	      dlen=StrLen(doc_e->tag);
	      doc_e->max_col=dlen+rlen-plen;
	      dst=MAlloc(doc_e->max_col+1,doc->mem_task);
	      dst2=dst;
	      j=src-doc_e->tag;
	      for (i=0;i<j;i++)
		*dst++=doc_e->tag[i];
	      for (i=0;i<rlen;i++)
		*dst++=doc->find_replace->replace_text[i];
	      src=dst;
	      for (i=j+plen;i<=dlen;i++)
		*dst++=doc_e->tag[i];
	      Free(doc_e->tag);
	      doc_e->tag=dst2;
	      doc->cur_data_col=src-doc_e->tag;
	      doc->cur_entry=doc_e;
	      if (cmd!=RSAC_ALL) {
		DocRemSoftNewLines(doc,doc->cur_entry);
		DocRecalc(doc);
	      }
	      doc_e=doc->cur_entry;
	      src=doc->cur_data_col+doc_e->tag;
	      result++;
	    } else
	      src++;
	  }
	}
      }
      if (doc->find_replace->scan_fwd) {
	doc_e=doc_e->next;
	if (doc_e==doc) {
	  if (doc_ce==doc) break;
	  doc_e=doc_e->next;
	}
      } else {
	doc_e=doc_e->last;
	if (doc_e==doc) {
	  if (doc_ce==doc) break;
	  doc_e=doc_e->last;
	}
      }
    }
fr_unlock_done:
    DocRemSoftNewLines(doc,NULL);
    DocRecalc(doc);
    DocCenter(doc);
    if (unlock)
      DocUnlock(doc);
  }
  return result;
}

public I64 EdReplace(CDoc *doc,U8 *find,U8 *replace,
  Bool selected=TRUE,Bool match_case=TRUE,Bool whole_labels=FALSE)
{
  CEdFindText old_find_replace;
  Bool unlock;
  I64 i,result=-1;
  if (!doc) return -1;
  unlock=DocLock(doc);
  MemCpy(&old_find_replace,doc->find_replace,sizeof(CEdFindText));
  MemSet(doc->find_replace,0,sizeof(CEdFindText));
  i=StrLen(find);
  if (i<sizeof(CEdFindText.find_text)) {
    MemCpy(doc->find_replace->find_text,find,i+1);
    i=StrLen(replace);
    if (i<sizeof(CEdFindText.replace_text)) {
      MemCpy(doc->find_replace->replace_text,replace,i+1);
      doc->find_replace->replace=TRUE;
      doc->find_replace->scan_selected_text=selected;
      doc->find_replace->match_case=match_case;
      doc->find_replace->whole_labels=whole_labels;
      doc->find_replace->pmt=FALSE;
      result=EdFindReplace(doc);
    }
  }
  MemCpy(doc->find_replace,&old_find_replace,sizeof(CEdFindText));
  if (unlock)
    DocUnlock(doc);
  return result;
}

class CEdLineGoTo
{
  I64 line fmtstr "$$DA,256,\"Go to Line:%d\"$$";
};

public U0 EdGoToLine(CDoc *doc)
{
  CEdLineGoTo gtl;
  gtl.line=1;
  if (DocFormDo(&gtl))
    DocLineNumGoTo(doc,gtl.line);
}

U0 DocEntryToggle(CDoc *doc)
{
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*cl1,*doc_ce2;
  U8 ch,*st,*st2;
  I64 i,j,k;
  if (doc_ce!=doc && !(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS))) {
    if (doc_ce->type_u8==DOCT_TEXT && !(doc_ce->flags1 & ~(DOCG_BL_IV_UL|DOCEF1_WORD_WRAP|DOCEF1_HIGHLIGHT|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER|DOCEF1_SKIP|DOCEF1_HARD_SKIP|DOCEF1_FILTER_SKIP))) {
      doc_ce2=doc_ce->last;
      for (k=0;k<20;k++) {
	if (doc_ce2!=doc) {
	  cl1=doc_ce2->last;
	  if (doc_ce2->type_u8==DOCT_TEXT && doc_ce->flags1==doc_ce2->flags1 && doc_ce->flags2==doc_ce2->flags2 && doc_ce->type==doc_ce2->type) {
	    i=StrLen(doc_ce2->tag);
	    j=StrLen(doc_ce->tag);
	    st=MAlloc(i+j+1,doc->mem_task);
	    MemCpy(st,doc_ce2->tag,i);
	    MemCpy(st+i,doc_ce->tag,j+1);
	    Free(doc_ce->tag);
	    doc_ce->tag=st;
	    doc_ce->max_col=i+j;
	    doc->cur_data_col+=i;
	    DocEntryDel(doc,doc_ce2);
	  } else if (doc_ce2->type_u8==DOCT_SOFT_NEW_LINE)
	    DocEntryDel(doc,doc_ce2);
	  else
	    break;
	  doc_ce2=cl1;
	} else
	  break;
      }
      doc_ce2=doc_ce->next;
      for (k=0;k<20;k++) {
	if (doc_ce2!=doc) {
	  cl1=doc_ce2->next;
	  if (doc_ce2->type_u8==DOCT_TEXT && doc_ce->flags1==doc_ce2->flags1 && doc_ce->flags2==doc_ce2->flags2 && doc_ce->type==doc_ce2->type) {
	    i=StrLen(doc_ce->tag);
	    j=StrLen(doc_ce2->tag);
	    st=MAlloc(i+j+1,doc->mem_task);
	    MemCpy(st,doc_ce->tag,i);
	    MemCpy(st+i,doc_ce2->tag,j+1);
	    Free(doc_ce->tag);
	    doc_ce->tag=st;
	    doc_ce->max_col=i+j;
	    DocEntryDel(doc,doc_ce2);
	  } else if (doc_ce2->type_u8==DOCT_SOFT_NEW_LINE)
	    DocEntryDel(doc,doc_ce2);
	  else
	    break;
	  doc_ce2=cl1;
	} else
	  break;
      }
      i=doc->cur_data_col;
      while (i>doc_ce->min_col && doc_ce->tag[i]!='$$')
	i--;
      j=doc->cur_data_col+1;
      while (j<doc_ce->max_col && doc_ce->tag[j]!='$$')
	j++;
      if (i<j-1 && doc_ce->min_col<=i<j<doc_ce->max_col && doc_ce->tag[i]=='$$' && doc_ce->tag[j]=='$$') {
	ch=doc_ce->tag[j+1];
	doc_ce->tag[j+1]=0;
	st=StrNew(doc_ce->tag+i);
	doc_ce->tag[j+1]=ch;
	StrCpy(doc_ce->tag+i,doc_ce->tag+j+1);
	doc->cur_data_col=i;
	st2=MPrint("%q",st);
	if (doc_ce=DocPrint(doc,st2)) {
	  doc->cur_entry=doc_ce;
	  doc->cur_data_col=doc_ce->min_col;
	}
	Free(st);
	Free(st2);
      }
    } else {
      st=Doc2PlainText(doc,doc_ce);
      DocEntryDel(doc,doc_ce);
      DocPrint(doc,"$$$$%Q$$$$",st);
    }
    DocRecalc(doc);
  }
  if (unlock)
    DocUnlock(doc);
}

U0 DocFlagsToggle(CDoc *doc,I64 tog_flags)
{
  Bool unlock=DocLock(doc);
  I64 size,flags=doc->flags^tog_flags;
  U8 *st=DocSave(doc,&size);
  CDocUndo *u_next,*u_last;

  u_next=doc->undo_root.next;
  u_last=doc->undo_root.last;
  doc->undo_root.next=doc->undo_root.last=&doc->undo_root;

  DocRst(doc,TRUE);
  doc->undo_root.next=u_next;
  doc->undo_root.last=u_last;
  DocUndoCntSet(doc);
  doc->flags=flags & ~(DOCG_BL_IV_UL|DOCF_WORD_WRAP);
  DocLoad(doc,st,size);
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
  Free(st);
}

#help_index "Doc/Output"
public U0 DocPutKey(CDoc *doc,I64 ch,I64 sc)
{
  I64 i,x,y,sc2,ext_ascii_cnt=0;
  CDoc *m;
  CDocEntry *doc_ce;
  U8 *st,*st2;
  Bool unlock;

  if (!doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return;
  if (doc->user_put_key)
    if ((*doc->user_put_key)(doc,doc->user_put_data,ch,sc))
      return;
  unlock=DocLock(doc);
  if (!Bt(doc_clean_scancodes,sc.u8[0]))
    doc->flags|=DOCF_UNDO_DIRTY;
  DocCaptureUndo(doc);
  if (Bt(getkey_chars_bitmap,ch) &&
      !(sc&(SCF_CTRL|SCF_ALT))) {
    if (sc&SCF_KEY_DESC) {
      st=A2KeyName(ch,FALSE);
      SetKeyDesc("Char/'%s'",st);
      Free(st);
    } else
      EdInsChar(ch,sc,doc);
  } else {
    doc_ce=doc->cur_entry;
    x=doc->x; y=doc->y;
    sc2=sc.u8[0];
    switch (ch) {
      case CH_CTRLA:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Save As");
	else
	  DocWrite(doc,TRUE);
	break;
      case CH_CTRLB:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Win/Toggle Border");
	else
	  WinBorder(Bt(&doc->win_task->display_flags,DISPLAYf_NO_BORDER),doc->win_task);
	break;
      case CH_CTRLC:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Copy to Clipboard");
	else
	  EdCopyToClipboard(doc);
	break;
      case CH_CTRLD:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Cmd/Jump to File Manager");
	else {
	  DocUnlock(doc);
	  FileMgr;
	  DocLock(doc);
	}
	break;
      case CH_CTRLF:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Cmd/Grep Wizard");
	  else
	    GrepWiz;
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Find & Replace");
	  else
	    EdFindReplace(doc);
	}
	break;
      case CH_CTRLG:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/GoTo Line Num");
	else
	  EdGoToLine(doc);
	break;
      case CH_BACKSPACE: //<CTRL-H>
	if (sc&SCF_ALT && !(sc&SCF_CTRL)) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Undo");
	  else
	    DocUndoRestore(doc);
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Char/Back Space");
	  else {
	    DocCaptureUndo(doc);
	    doc_ce=doc->cur_entry;
	    if (doc->cur_data_col<=doc_ce->min_col) {
	      doc_ce=doc->cur_entry=doc_ce->last;
	      if (doc_ce!=doc && doc_ce->type_u8==DOCT_SOFT_NEW_LINE)
		doc_ce=doc->cur_entry=doc_ce->last;
	      if (doc_ce==doc || doc_ce->type_u8==DOCT_PMT) {
		doc_ce=doc->cur_entry=doc_ce->next;
		doc->cur_data_col=doc_ce->min_col;
	      } else {
		doc->cur_data_col=doc_ce->max_col;
		if (doc->cur_data_col>doc_ce->min_col)
		  doc->cur_data_col--;
		EdDelChar(doc);
	      }
	    } else {
	      doc->cur_data_col--;
	      EdDelChar(doc);
	    }
	  }
	}
	break;
      case CH_CTRLI:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Indent Out");
	  else
	    DocPrint(doc,"$$ID,-2$$");
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Indent In");
	  else
	    DocPrint(doc,"$$ID,2$$");
	}
	break;
      case '\n':
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Char/Return");
	  else
	    SetKeyDesc("Char/Page Break");
	} else
	  EdInsChar(ch,sc,doc);
	break;
      case CH_CTRLK:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Blinking Text");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$BK,0$$");
	else
	  DocPrint(doc,"$$BK,1$$");
	break;
      case CH_CTRLL:
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Edit/Managed Code");
	  else
	    SetKeyDesc("Dollar/Wizard");
	} else {
	  if (sc&SCF_SHIFT)
	    EdCodeTools(doc);
	  else {
	    DocUnlock(doc);
	    EdInsDollarCmd;
	    DocLock(doc);
	  }
	}
	break;
      case CH_CTRLM:
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Cmd/Jump to Notes File");
	  else
	    SetKeyDesc("Cmd/Jump to Menu");
	} else {
	  if (sc&SCF_SHIFT)
	    Ed("HOME/PersonalNotes.TXT.Z");
	  else {
	    m=DocRead("HOME/PersonalMenu.TXT.Z");
	    DocMenuDo(m);
	    DocDel(m);
	  }
	}
	break;
      case CH_CTRLO:
	if (sc&SCF_KEY_DESC) {
	  if (sc&SCF_SHIFT)
	    SetKeyDesc("Edit/Collapse");
	  else
	    SetKeyDesc("Edit/Uncolapse");
	} else {
	  if (sc&SCF_SHIFT)
	    DocCollapse(TRUE,doc);
	  else
	    DocCollapse(FALSE,doc);
	}
	break;
      case CH_CTRLP:
	if (doc->flags & (DOCF_SUPERSCRIPT_MODE | DOCF_SUBSCRIPT_MODE)) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Toggle Super or Sub script");
	  else {
	    DocPrint(doc,"$$SY,0$$");
	    doc->flags&=~(DOCF_SUPERSCRIPT_MODE | DOCF_SUBSCRIPT_MODE);
	  }
	} else if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Toggle Subscript");
	  else {
	    DocPrint(doc,"$$SY,3$$");
	    doc->flags|=DOCF_SUBSCRIPT_MODE;
	  }
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Dollar/Toggle Superscript");
	  else {
	    DocPrint(doc,"$$SY,-3$$");
	    doc->flags|=DOCF_SUPERSCRIPT_MODE;
	  }
	}
	break;
      case CH_CTRLQ:
	break;
      case CH_CTRLR:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Edit or Create Graphic");
	else {
	  if (!(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS))) {
	    DocUnlock(doc);
	    if (doc_ce->type_u8==DOCT_SPRITE)
	      EdExistingGraphic(doc,
		  (doc_ce->x-doc->line_start_col+doc_ce->max_col)*FONT_WIDTH,
		  (doc_ce->y-doc->cur_top_line_num)*FONT_HEIGHT);
	    else
	      EdInsGraphic(doc,
		  (doc_ce->x+doc->cur_data_col-doc->line_start_col)*FONT_WIDTH,
		  (doc_ce->y-doc->cur_top_line_num)*FONT_HEIGHT);
	    DocLock(doc);
	  }
	}
	break;
      case CH_CTRLS:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Find Misspelled");
	  else
	    WSFindMisspelled(doc);
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Save");
	  else
	    DocWrite(doc);
	}
	break;
      case CH_CTRLT:
	if (sc&SCF_SHIFT) {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Single Entry Toggle Plain Text");
	  else
	    DocEntryToggle(doc);
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Toggle Plain Text Display");
	  else
	    if (!(doc->flags&DOCF_FORM) || doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS))
	      DocFlagsToggle(doc,DOCF_PLAIN_TEXT);
	}
	break;
      case CH_CTRLU:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Underline");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$UL,0$$");
	else
	  DocPrint(doc,"$$UL,1$$");
	break;
      case CH_CTRLV:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Paste Clipboard");
	else
	  EdPasteClipboard(doc);
	break;
      case CH_CTRLW:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Word Wrap");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$WW,0$$");
	else
	  DocPrint(doc,"$$WW,1$$");
	break;
      case CH_CTRLX:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Cut To Clipboard");
	else
	  EdCutToClipboard(doc);
	break;
      case CH_CTRLY:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Edit/Delete Line");
	else
	  EdDelLine(doc);
	break;
      case CH_CTRLZ:
	if (sc&SCF_KEY_DESC)
	  SetKeyDesc("Dollar/Toggle Inverted Text");
	else if (sc&SCF_SHIFT)
	  DocPrint(doc,"$$IV,0$$");
	else
	  DocPrint(doc,"$$IV,1$$");
	break;
      case '0'...'9':
	if (!(sc&SCF_ALT)) {
	  if (sc&SCF_CTRL) {
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Jump to Word Definition");
	      else
		SetKeyDesc("Edit/Autocomplete Word");
	    } else {
	      if (WordStat(ON)) {
		DocUnlock(doc);
		if (sc&SCF_SHIFT)
		  WSDDef(ch-'0',Fs);
		else
		  WSDFillin(ch-'0');
		DocLock(doc);
	      }
	    }
	  }
	} else {
	  if (sc&SCF_KEY_DESC)
	    SetKeyDesc("Edit/Enter extended-ASCII");
	  else {
	    if (++doc->ext_ascii_cnt==1)
	      doc->ext_ascii_val=ch-'0';
	    else
	      doc->ext_ascii_val=doc->ext_ascii_val*10+ch-'0';
	    if (doc->ext_ascii_cnt==3)
	      EdInsChar(doc->ext_ascii_val.u8[0],sc,doc);
	    else
	      ext_ascii_cnt=doc->ext_ascii_cnt;
	  }
	}
	break;
      case '[':
	if (!(sc&SCF_ALT) && sc&SCF_CTRL) {
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching brace");
	    else
	      EdFindPaired(doc,'}','{',FALSE);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching bracket");
	    else
	      EdFindPaired(doc,']','[',FALSE);
	  }
	}
	break;
      case ']':
	if (!(sc&SCF_ALT) && sc&SCF_CTRL) {
	  if (sc&SCF_SHIFT) {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching brace");
	    else
	      EdFindPaired(doc,'{','}',TRUE);
	  } else {
	    if (sc&SCF_KEY_DESC)
	      SetKeyDesc("Edit/GoTo matching bracket");
	    else
	      EdFindPaired(doc,'[',']',TRUE);
	  }
	}
	break;
      default:
	switch (sc2) {
	  case SC_F1...SC_F10:
	    if (!(sc&SCF_ALT)) {
	      if (sc&SCF_CTRL) {
		if (sc&SCF_KEY_DESC) {
		  if (sc&SCF_SHIFT)
		    SetKeyDesc("Edit/Jump to Src of Sym");
		  else
		    SetKeyDesc("Edit/Autocomplete Sym");
		} else {
		  if (WordStat(ON)) {
		    DocUnlock(doc);
		    if (sc&SCF_SHIFT)
		      WSMan(sc2-SC_F1+1,Fs);
		    else
		      WSFillIn(sc2-SC_F1+1);
		    DocLock(doc);
		  }
		}
	      } else {
		switch (sc2) {
		  case SC_F1:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("About");
		      else
			SetKeyDesc("Help");
		    } else {
		      if (sc&SCF_SHIFT)
			Ed("::/TempleOS/Doc/Splash.TXT.Z");
		      else
			Ed("::/TempleOS/Doc/HelpIndex.TXT.Z");
		    }
		    break;
		  case SC_F2:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Edit/Play Macro");
		      else
			SetKeyDesc("Edit/Macro");
		    } else {
		      DocUnlock(doc);
		      if (sc&SCF_SHIFT) {
			if (TaskValidate(sys_macro_util_task))
			  SendMsg(sys_macro_util_task,MSG_KEY_DOWN_UP,CH_SHIFT_ESC,0);
			SysMacroStripKey(&sys_macro_root,ch,sc);
			PlaySysMacro;
		      } else
			EdMacroUtil;
		      DocLock(doc);
		    }
		    break;
		  case SC_F3:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Edit/Find Last");
		      else
			SetKeyDesc("Edit/Find Next");
		    }else {
		      doc->find_replace->scan_fwd=!(sc&SCF_SHIFT);
		      EdFindNext(doc);
		    }
		    break;
		  case SC_F4:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Misc/Insert Directory Name");
		      else
			SetKeyDesc("Misc/Insert File Name");
		    } else {
		      DocUnlock(doc);
		      if (sc&SCF_SHIFT)
			st=PopUpPickDir;
		      else
			st=PopUpPickFile;
		      DocLock(doc);
		      if (st) {
			DocPutSPartial(doc,st);
			Free(st);
		      }
		    }
		    break;
		  case SC_F5:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Cmd/Adam Include");
		      else
			SetKeyDesc("Cmd/Run (Execute);Dbg/Go");
		    } else {
		      if (IsDbgTask) {
			DocUnlock(doc);
			if (!(sc&SCF_SHIFT))
			  IDEGo;
			DocLock(doc);
		      } else {
			if (st2=DocEntryLink(doc,doc_ce)) {
			  st=DocLinkFile(st2);
			  Free(st2);
			} else {
			  DocWrite(doc);
			  st=StrNew(doc->filename.name);
			}
			if (st2=DirFile(st,"Run","CPP.Z")) {
			  if (FileFind(st2)) {
			    Free(st);
			    st=st2;
			  } else
			    Free(st2);
			}
			if (st) {
			  if (sc&SCF_SHIFT)
			    AdamFile(st);
			  else
			    PopUpFile(st);
			  Free(st);
			}
		      }
		    }
		    break;
		  case SC_F9:
		    if (sc&SCF_KEY_DESC) {
		      if (sc&SCF_SHIFT)
			SetKeyDesc("Dbg/Clear All Break Points");
		      else
			SetKeyDesc("Dbg/Toggle Break Point");
		    } else {
		      if (IsDbgTask) {
			DocUnlock(doc);
			if (sc&SCF_SHIFT)
			  IDEClearAllBpts(doc);
			else
			  IDEToggleBpt(doc);
			DocLock(doc);
		      }
		    }
		    break;
		  case SC_F10:
		    if (sc&SCF_KEY_DESC) {
		      if (!(sc&SCF_SHIFT))
			SetKeyDesc("Dbg/Step Over");
		    } else {
		      if (IsDbgTask) {
			DocUnlock(doc);
			if (!(sc&SCF_SHIFT))
			  IDEStepOver(doc);
			DocLock(doc);
		      }
		    }
		    break;
		}
	      }
	    }
	    break;
	  case SC_F11:
	  case SC_F12:
	    if (!(sc & (SCF_ALT|SCF_CTRL))) {
	      switch (sc2) {
		case SC_F11:
		  if (sc&SCF_KEY_DESC) {
		    if (!(sc&SCF_SHIFT))
		      SetKeyDesc("Dbg/Step Into");
		  } else {
		    if (IsDbgTask) {
		      DocUnlock(doc);
		      if (!(sc&SCF_SHIFT))
			IDEStepInto(doc);
		      DocLock(doc);
		    }
		  }
		  break;
	      }
	    }
	    break;
	  case SC_CURSOR_DOWN:
	  case SC_END:
	    if (sc2==SC_END || sc&SCF_CTRL) {
	      if (sc&SCF_KEY_DESC) {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/GoTo Doc End, Select");
		else
		  SetKeyDesc("Edit/GoTo Doc End");
	      } else {
		while (doc_ce!=doc) {
		  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
		  doc_ce=doc->cur_entry=doc_ce->next;
		}
		doc->cur_data_col=doc_ce->min_col;
		doc->recalc_start=doc;
		DocFormBackward(doc);
	      }
	    } else if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Cursor Down, Select");
	      else
		SetKeyDesc("Edit/Cursor Down");
	    } else
	      EdLineDown(doc,sc);
	    break;
	  case SC_CURSOR_UP:
	  case SC_HOME:
	    if (sc2==SC_HOME || sc&SCF_CTRL) {
	      if (sc&SCF_KEY_DESC) {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/GoTo Top of Doc, Select");
		else
		  SetKeyDesc("Edit/GoTo Top of Doc");
	      } else {
		if (doc_ce==doc) doc_ce=doc_ce->last;
		while (doc_ce!=doc) {
		  BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
		  doc_ce=doc->cur_entry=doc_ce->last;
		}
		doc_ce=doc->cur_entry=doc->root.next;
		doc->cur_data_col=doc_ce->min_col;
		doc->recalc_start=doc;
		DocFormFwd(doc);
	      }
	    } else if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Cursor Up, Select");
	      else
		SetKeyDesc("Edit/Cursor Up");
	    } else
	      EdLineUp(doc,sc);
	    break;
	  case SC_PAGE_DOWN:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Page Down, Select");
	      else
		SetKeyDesc("Edit/Page Down");
	    } else {
	      i=doc_ce->y+doc->win_task->win_height-1;
	      if (doc_ce->type_u8==DOCT_HEX_ED)
		i+=doc->cur_data_col/3/doc_ce->hex_ed_width;
	      while (doc_ce!=doc &&
		  (doc_ce->type_u8!=DOCT_HEX_ED && doc_ce->y<i ||
		   doc_ce->type_u8==DOCT_HEX_ED && doc_ce->y+doc->cur_data_col/3/doc_ce->hex_ed_width<i) &&
		  doc_ce->y!=doc->root.last->y) {
		EdLineDown(doc,sc);
		//paranoid check for stuck on same node
		if (doc->cur_entry==doc_ce && doc_ce->type_u8!=DOCT_HEX_ED)
		  break;
		doc_ce=doc->cur_entry;
	      }
	    }
	    break;
	  case SC_PAGE_UP:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Page Up, Select");
	      else
		SetKeyDesc("Edit/Page Up");
	    }else {
	      i=doc_ce->y-(doc->win_task->win_height-1);
	      if (doc_ce->type_u8==DOCT_HEX_ED)
		i+=doc->cur_data_col/3/doc_ce->hex_ed_width;
	      while (doc_ce->last!=doc &&
		  (doc_ce->type_u8!=DOCT_HEX_ED && doc_ce->y>i ||
		   doc_ce->type_u8==DOCT_HEX_ED && doc_ce->y+doc->cur_data_col/3/doc_ce->hex_ed_width>i) &&
		  doc_ce->y!=doc->root.next->y) {
		EdLineUp(doc,sc);
		//paranoid check for stuck on same node
		if (doc->cur_entry==doc_ce && doc_ce->type_u8!=DOCT_HEX_ED)
		  break;
		doc_ce=doc->cur_entry;
	      }
	    }
	    break;
	  case SC_CURSOR_LEFT:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_CTRL)
		SetKeyDesc("Edit/GoTo Start of Line");
	      else {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/Cursor Left, Select");
		else
		  SetKeyDesc("Edit/Cursor Left");
	      }
	    } else
	      EdCursorLeft(doc,sc);
	    break;
	  case SC_CURSOR_RIGHT:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_CTRL)
		SetKeyDesc("Edit/GoTo End of Line");
	      else {
		if (sc&SCF_SHIFT)
		  SetKeyDesc("Edit/Cursor Right, Select");
		else
		  SetKeyDesc("Edit/Cursor Right");
	      }
	    } else
	      EdCursorRight(doc,sc);
	    break;
	  case SC_DELETE:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Cut To Clipboard");
	      else
		SetKeyDesc("Char/Delete");
	    } else {
	      if (sc&SCF_SHIFT)
		EdCutToClipboard(doc);
	      else
		EdDelChar(doc);
	    }
	    break;
	  case SC_INS:
	    if (sc&SCF_KEY_DESC) {
	      if (sc&SCF_SHIFT)
		SetKeyDesc("Edit/Paste Clipboard");
	      else if (sc&SCF_CTRL)
		SetKeyDesc("Edit/Copy to Clipboard");
	      else
		SetKeyDesc("Edit/Toggle Overstrike");
	    } else {
	      if (sc&SCF_SHIFT)
		EdPasteClipboard(doc);
	      else if (sc&SCF_CTRL)
		EdCopyToClipboard(doc);
	      else
		doc->flags^=DOCF_OVERSTRIKE;
	    }
	    break;
	}  //switch (sc2)
    }
  }
  if (!doc->recalc_start)
    doc->recalc_start=doc->cur_entry->last;
  doc->ext_ascii_cnt=ext_ascii_cnt;
  if (unlock)
    DocUnlock(doc);
  if (!(doc->flags&DOCF_DONT_SWAP_OUT))
    Yield;
}

U0 DocNewLine(CDoc *doc=NULL,I64 cnt=1)
{
  while (cnt-->0)
    DocPutKey(doc,'\n',0);
}

Bool AKDDocPutKey(I64 ch,I64 scancode)
{
  CDoc *doc;
  if (doc=DocPut)
    DocPutKey(doc,ch,scancode);
  return FALSE;
}

public U0 DocPutSPartial(CDoc *doc,U8 *st)
{
  U8 ch,*b,*src,*dst,*ptr=st,*ptr2;
  Bool unlock;
  CDocEntry *doc_ce,*doc_ne;
  I64 i,j;
  if (!st)
    return;
  if (doc->user_put_s)
    if ((*doc->user_put_s)(doc,doc->user_put_data,st))
      return;
  unlock=DocLock(doc);
  if (doc->cur_entry->type_u8==DOCT_DATA)
    while (ch=*ptr++)
      DocPutKey(doc,ch,0);
  else
    while (ch=*ptr) {
      if (!Bt(safe_dollar_chars_bitmap,ch) ||
	  doc->flags & (DOCF_OVERSTRIKE | DOCF_IN_DOLLAR)) {
	DocPutKey(doc,ch,0);
	ptr++;
      } else {
	ptr2=ptr++;
	while (TRUE) {
	  ch=*ptr++;
	  if (!Bt(safe_dollar_chars_bitmap,ch))
	    break;
	}
	ptr--;
	*ptr=0;
	doc_ce=doc->cur_entry;
	doc->recalc_start=doc_ce->last;
	j=ptr-ptr2;
	if (IsEditableText(doc_ce)) {
	  dst=b=MAlloc(doc_ce->max_col+j+1,doc->mem_task);
	  src=doc_ce->tag;
	  i=doc->cur_data_col;
	  doc->cur_data_col+=j;
	  doc_ce->max_col+=j;
	  while (i-->0)
	    *dst++=*src++;
	  while (j-->0)
	    *dst++=*ptr2++;
	  while (*dst++=*src++);
	  Free(doc_ce->tag);
	  doc_ce->tag=b;
	} else {
	  dst=MAlloc(j+1,doc->mem_task);
	  MemCpy(dst,ptr2,j+1);
	  doc_ne=DocEntryNew(doc,doc_ce,dst);
	  doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
	  doc_ne->flags1=0;
	  doc_ne->flags2=0;
	  QueIns(doc_ne,doc_ce->last);
	  doc->cur_entry=doc_ne;
	  doc->cur_data_col=StrLen(ptr2);
	}
	*ptr=ch;
	DocRemSoftNewLines(doc,doc->cur_entry);
      }
    }
  if (unlock)
    DocUnlock(doc);
  if (!(doc->flags&DOCF_DONT_SWAP_OUT))
    Yield;
}

#help_index "Doc"
Bool DocFileEd(I64 type,U8 *filename,
	U8 *pattern,I64 *_num,I64 ed_flags)
{
  I64 flags=0,i,old_border_src=Fs->border_src;
  CDocEntry *doc_e;
  CDoc *doc;
  Bool old_silent=Bt(&Fs->display_flags,DISPLAYf_SILENT),
	result=FALSE;
  try {
    switch (type) {
      case LK_PLAIN:
	type=LK_FILE;
	flags=DOCF_PLAIN_TEXT;
	break;
      case LK_PLAIN_FIND:
	type=LK_FILE_FIND;
	flags=DOCF_PLAIN_TEXT;
	break;
      case LK_PLAIN_LINE:
	type=LK_FILE_LINE;
	flags=DOCF_PLAIN_TEXT;
	break;
      case LK_BIBLE_FIND:
	flags=DOCF_PLAIN_TEXT;
	break;
    }
 
    flags|=DOCF_ALLOW_UNDO;
 
    if (!*filename)
      doc=DocPut;
    else {
      Silent(ON);
      if (Bt(&ed_flags,EDf_BAIL)) //if bail, scan parents
	i=FUF_JUST_FILES|FUF_Z_OR_NOT_Z|FUF_SCAN_PARENTS;
      else
	i=FUF_JUST_FILES;
      result=FileFind(filename,,i);
      doc=DocRead(filename,flags);
      StrCpy(doc->desc,"Edit");
      Silent(old_silent);
      Fs->border_src=BDS_ED_FILENAME_PRT;
    }
 
    if (Bt(&ed_flags,EDf_COLLAPSE))
      DocCollapse(TRUE,doc);
    else if (Bt(&ed_flags,EDf_UNCOLLAPSE))
      DocCollapse(FALSE,doc);

    if (result)
      switch (type) {
	case LK_FILE_LINE:
	  result=DocLineNumGoTo(doc,*_num);
	  break;
	case LK_FILE_ANCHOR:
	  result=DocAnchorFind(doc,pattern);
	  break;
	case LK_FILE_FIND:
	  result=DocPatternFind(doc,,pattern,*_num);
	  break;
	case LK_BIBLE_FIND:
	  result=DocPatternFind(doc,*_num,pattern);
	  break;
	default:
	  DocCenter(doc);
      }
    *_num=doc->cur_entry->y+1;
    if (!(ed_flags&EDF_BAIL) && DocDo(doc,DOF_DONT_HOME)) {
      DocLock(doc);
      doc_e=doc->cur_entry;
      if (doc_e!=doc)
	DocEntryRun(doc,doc_e,TRUE);
      DocUnlock(doc);
      DocWrite(doc);
    }
    if (DocPut!=doc)
      DocDel(doc);
    else
      result=FALSE;
  } catch {
    Silent(old_silent);
    result=FALSE;
  }
  Fs->border_src=old_border_src;
  return result;
}

public I64 EdLinkCvt(U8 *link_st,U8 **_filename=NULL,U8 **_pattern=NULL,
  I64 *_num=NULL,I64 ed_flags=0)
{
  U8 *st,*ptr,*filename=NULL,*pattern=NULL,*filename2;
  I64 result,i,num=1;
  CHashSrcSym *temph;
  if (!link_st||!*link_st) {
    if (ed_flags&EDF_BAIL)
      return -1;
    link_st=sys_temp_filename;
  }
  st=StrNew(link_st);
  result=LK_FILE;
  if (StrLen(st)>3 && st[2]==':') {
    st[2]=0;
    filename2=st+3;
    switch (result=DefineMatch(st,"ST_LINK_TYPES",LMF_IGNORE_CASE)) {
      case LK_MAN_PAGE:
	if (temph=HashFind(filename2,Fs->hash_table,HTG_SRC_SYM)) {
	  if (temph->type&HTT_DEFINE_STR && temph(CHashDefineStr *)->caller) {
	    if (ptr=SrcEdLink(temph(CHashDefineStr *)->caller,DFT_ADDR_LINK_BIN_SIZE)) {
	      result=EdLinkCvt(ptr,&filename,&pattern,&num,ed_flags);
	      Free(ptr);
	    } else
	      result=-1;
	  } else
	    result=EdLinkCvt(temph->src_link,&filename,&pattern,&num,ed_flags);
	} else
	  result=-1;
	goto lc_done;
      case LK_ADDR:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  i=A2I(ptr+1);
	} else
	  i=DFT_ADDR_LINK_BIN_SIZE;
	if (ptr=SrcEdLink(ExePrint("%s;",filename2),i)) {
	  result=EdLinkCvt(ptr,&filename,&pattern,&num,ed_flags);
	  Free(ptr);
	} else
	  result=-1;
	goto lc_done;
      case LK_DEF:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  i=A2I(ptr+1);
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
	  pattern=StrNew(ptr+1);
	}
	i=DefineMatch(filename2,"ST_BIBLE_BOOKS",LMF_IGNORE_CASE);
	if (i<0)
	  result=-1;
	else {
	  num=A2I(DefineSub(i,"ST_BIBLE_BOOK_LINES"));
	  filename2=Define("ST_BIBLE_FILE");
	}
	break;
      case LK_FILE_LINE:
      case LK_PLAIN_LINE:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  num=A2I(ptr+1);
	}
	break;
      case LK_FILE_ANCHOR:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  pattern=StrNew(ptr+1);
	}
	break;
      case LK_FILE_FIND:
      case LK_PLAIN_FIND:
	if (ptr=StrLastOcc(filename2,",")) {
	  *ptr=0;
	  pattern=StrNew(ptr+1);
	  if (ptr=StrLastOcc(pattern,":")) {
	    *ptr=0;
	    num=A2I(ptr+1);
	  }
	}
	break;
    }
  } else
    filename2=st;
  if (result>=0)
    filename=FileNameAbs(filename2,FUF_Z_OR_NOT_Z);
lc_done:
  Free(st);
  if (_filename)
    *_filename=filename;
  else
    Free(filename);
  if (_pattern)
    *_pattern=pattern;
  else
    Free(pattern);
  if (_num)
    *_num=num;
  return result;
}
