#help_index "DolDoc/Editor"

public I64 EdCurU8(CDoc *doc)
{//Return cur U8. See $LK,"EdRenumAsm",A="MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry;
  I64 result=-1;
  if (doc_ce->type_u8==DOCT_TEXT &&
	doc_ce->min_col<=doc->cur_col<doc_ce->max_col)
    result=doc_ce->tag[doc->cur_col];
  else if (doc_ce->type_u8==DOCT_TAB)
    result='\t';
  else if (doc_ce->type_u8==DOCT_NEW_LINE ||
	doc_ce->type_u8==DOCT_SOFT_NEW_LINE)
    result='\n';
  if (unlock)
    DocUnlock(doc);
  return result;
}

public U0 EdCursorLeft(CDoc *doc,I64 sc=MIN_I64)
{//Move cursor left. Might need a call to $LK,"DocRecalc",A="MN:DocRecalc"$().
//See $LK,"EdRenumAsm",A="MN:EdRenumAsm"$ for an example.
  U8 *dst;
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*original_ce=doc_ce,*doc_ne;
  I64 cc=doc->cur_col,y=doc_ce->y;
  if (sc!=MIN_I64) sc=sc.u32[0];
  if (sc>=0 && sc&SCF_CTRL) {
    while (doc_ce->last!=doc && (doc_ce->last->y==y ||
	  doc_ce->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP)))
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
	    (doc_ce->last->type_u8==DOCT_SOFT_NEW_LINE ||
	    doc_ce->last->type_u8==DOCT_INDENT ||
	    doc_ce->last->de_flags&(DOCEF_SKIP|DOCEF_FILTER_SKIP))) {
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
  doc->cur_col=cc;
  doc->cur_entry=doc_ce;
  if (doc_ce!=original_ce)
    DocFormBwd(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 EdCursorRight(CDoc *doc,I64 sc=MIN_I64)
{//Move cursor right. Might need a call to $LK,"DocRecalc",A="MN:DocRecalc"$().
//See $LK,"EdRenumAsm",A="MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  U8 *dst;
  CDocEntry *doc_ce=doc->cur_entry,*original_ce=doc_ce,*doc_ne;
  I64 cc=doc->cur_col,y=doc_ce->y,old_de_flags,old_color;
  if (sc!=MIN_I64) sc=sc.u32[0];
  if (sc>=0 && sc&SCF_CTRL) {
    while (doc_ce!=doc && doc_ce->next->y==y &&
	  doc_ce->next->type_u8!=DOCT_SOFT_NEW_LINE && doc_ce->next!=doc &&
	  (doc_ce->next->type_u8!=DOCT_NEW_LINE || !(doc->flags & DOCF_FORM)) ||
	  doc_ce->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP))
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
      old_de_flags=doc_ce->de_flags;
      old_color=doc_ce->type;
      if (sc>=0)
	BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
      if (IsEditableText(doc_ce) && cc<doc_ce->max_col) {
	dst=doc_ce->tag+cc;
	doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
	*dst=0;
	doc_ne->type=DOCT_TEXT | old_color & -0x100;
	doc_ne->de_flags=old_de_flags|doldoc.dft_de_flags[DOCT_TEXT];
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
	while (doc_ce!=doc && doc_ce->de_flags&(DOCEF_SKIP|DOCEF_FILTER_SKIP)) {
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
  doc->cur_col=cc;
  doc->cur_entry=doc_ce;
  if (doc_ce!=original_ce)
    DocFormFwd(doc);
  if (unlock)
    DocUnlock(doc);
}

public U0 EdLineUp(CDoc *doc,I64 sc=MIN_I64)
{//Move cursor up. Might need a call to $LK,"DocRecalc",A="MN:DocRecalc"$().
//See $LK,"EdRenumAsm",A="MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  U8 *dst;
  I64 y,x;
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;

  if (sc!=MIN_I64) sc=sc.u32[0];
  if (doc_ce->type_u8==DOCT_HEX_ED) {
    doc->cur_col=doc->cur_col-doc_ce->hex_ed_width*3;
    if (doc->cur_col>=0) {
      if (unlock)
	DocUnlock(doc);
      return;
    } else
      doc->cur_col=0;
  }
  x=doc->x; y=doc->y;
  if (IsEditableText(doc_ce)) {
    if (doc_ce->min_col<doc->cur_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      doc_ne->x=doc_ce->x+doc->cur_col;
      doc_ce->max_col=doc->cur_col;
      QueIns(doc_ne,doc_ce);
    } else if (doc->cur_col==doc_ce->min_col && doc_ce->last!=doc)
      doc_ce=doc_ce->last;
  } else if (doc_ce->last!=doc)
    doc_ce=doc_ce->last;
  if (sc>=0)
    BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
  doc->cur_entry=doc_ce;
  DocFormBwd(doc);
  doc_ce=doc->cur_entry;
  while (doc_ce->last!=doc && (doc_ce->y>=y ||
	doc_ce->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP))) {
    doc_ce=doc_ce->last;
    if (sc>=0)
      BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
  }
  y=doc_ce->y;
  doc->y=y;
  while (doc_ce!=doc && (doc_ce->y>=y && doc_ce->x>=x ||
	doc_ce->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP))) {
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
	if (doc->flags & DOCF_FORM)
	  while (doc_ce->next->x==x &&
		(!Bt(doldoc.type_flags_form,doc_ce->type_u8) &&
		!(doc_ce->de_flags&DOCEF_LINK)||
		doc_ce->de_flags&DOCEF_SKIP_IN_FORM))
	    doc_ce=doc_ce->next;
      }
    }
  }
  if (doc_ce->de_flags&DOCEF_TAG) {
    doc->cur_col=x-doc_ce->x;
    if (IsEditableText(doc_ce)) {
      if (doc->cur_col>doc_ce->max_col)
	doc->cur_col=doc_ce->max_col;
    } else if (doc->cur_col>=doc_ce->max_col)
      doc->cur_col=doc_ce->max_col-1;
    if (doc->cur_col<doc_ce->min_col)
      doc->cur_col=doc_ce->min_col;
  } else {
    if (doc_ce->type_u8==DOCT_HEX_ED) {
      doc->cur_col=RoundI64((doc_ce->len-1)*3,doc_ce->hex_ed_width*3);
      if (doc->cur_col<0)
	doc->cur_col=0;
    } else
      doc->cur_col=doc_ce->min_col;
  }
  if (IsEditableText(doc_ce) && doc_ce->x<x) {
    if (doc->cur_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      if (sc>=0) {
	if (sc&SCF_SHIFT)
	  doc_ne->type=doc_ce->type | DOCET_SELECTED;
	else
	  doc_ne->type=doc_ce->type & ~DOCET_SELECTED;
      }
      doc_ne->x=doc_ce->x+doc->cur_col;
      doc_ce->max_col=doc->cur_col;
      QueIns(doc_ne,doc_ce);
      doc_ce=doc_ne;
      doc->cur_col=doc_ce->min_col;
    }
  }
  doc->cur_entry=doc_ce;
  DocFormFwd(doc);
  doc->x=doc->cur_entry->x+doc->cur_col;
  if (unlock)
    DocUnlock(doc);
}

public U0 EdLineDown(CDoc *doc,I64 sc=MIN_I64)
{//Move cursor down. Might need a call to $LK,"DocRecalc",A="MN:DocRecalc"$().
//See $LK,"EdRenumAsm",A="MN:EdRenumAsm"$ for an example.
  Bool unlock=DocLock(doc);
  U8 *dst;
  I64 y,x,old_de_flags=0,old_color;
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne,*doc_ce2;
  if (sc!=MIN_I64) sc=sc.u32[0];
  if (doc_ce->type_u8==DOCT_HEX_ED) {
    doc->cur_col=doc->cur_col+doc_ce->hex_ed_width*3;
    if (doc->cur_col>=doc_ce->len*3) {
      doc->cur_entry=doc_ce=doc_ce->next;
      doc->cur_col=doc_ce->min_col;
      doc->x=doc_ce->x+doc->cur_col;
      doc->y=doc_ce->y;
    }
    if (unlock)
      DocUnlock(doc);
    return;
  }
  x=doc->x; y=doc->y;
  if (IsEditableText(doc_ce)) {
    if (doc->cur_col>doc_ce->min_col && doc->cur_col<doc_ce->max_col-1) {
      dst=doc_ce->tag+doc->cur_col;
      doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
      *dst=0;
      if (sc>=0) {
	if (sc&SCF_SHIFT)
	  doc_ne->type=doc_ce->type | DOCET_SELECTED;
	else
	  doc_ne->type=doc_ce->type & ~DOCET_SELECTED;
      }
      doc_ne->x=doc_ce->x+doc->cur_col;
      doc_ce->max_col=doc->cur_col;
      QueIns(doc_ne,doc_ce);
      doc_ce=doc_ne;
      doc->cur_col=doc_ce->min_col;
    }
  }
  doc_ce2=doc_ce;
  while (doc_ce!=doc && (doc_ce->y<=y ||
	doc_ce->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP)))
    doc_ce=doc_ce->next;
  y=doc_ce->y;
  doc->y=y;
  while (doc_ce!=doc && (doc_ce->y<=y && doc_ce->x<=x ||
	doc_ce->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP))) {
    old_de_flags=doc_ce->de_flags;
    old_color=doc_ce->type;
    doc_ce=doc_ce->next;
  }
  if (doc_ce->last!=doc && (doc_ce->x>x && doc_ce->last->y>=y || doc_ce->y>y)) {
    doc_ce=doc_ce->last;
    doc->cur_entry=doc_ce;
    if (!((doc_ce->type_u8==DOCT_NEW_LINE ||
	  doc_ce->type_u8==DOCT_SOFT_NEW_LINE ||
	  doc_ce->type_u8==DOCT_INDENT) &&
	  (doc_ce->last->type_u8==DOCT_NEW_LINE ||
	  doc_ce->last->type_u8==DOCT_SOFT_NEW_LINE ||
	  doc_ce->last->type_u8==DOCT_INDENT)))
      DocFormBwd(doc);
    doc_ce=doc->cur_entry;
  }
  while (doc_ce2!=doc && (doc_ce2!=doc_ce || IsEditableText(doc_ce))) {
    if ((doc_ce2->y<y || doc_ce2->x<x ||
	  doc_ce2->de_flags & (DOCEF_SKIP|DOCEF_FILTER_SKIP) ||
	  doc_ce2->x==x && !doc_ce2->max_col &&
	  Bt(doldoc.type_flags_nontag_invisible,doc_ce2->type_u8)) && sc>=0)
      BEqu(&doc_ce2->type,DOCEt_SELECTED,sc&SCF_SHIFT);
    if (doc_ce2==doc_ce) break;
    doc_ce2=doc_ce2->next;
  }
  if (doc_ce->de_flags&DOCEF_TAG) {
    doc->cur_col=x-doc_ce->x;
    if (IsEditableText(doc_ce)) {
      if (doc->cur_col>doc_ce->max_col)
	doc->cur_col=doc_ce->max_col;
    } else if (doc->cur_col>=doc_ce->max_col)
      doc->cur_col=doc_ce->max_col-1;
    if (doc->cur_col<doc_ce->min_col)
      doc->cur_col=doc_ce->min_col;
  } else
    doc->cur_col=doc_ce->min_col;
  if (IsEditableText(doc_ce)&&doc_ce->min_col<doc->cur_col<doc_ce->max_col-1) {
    dst=doc_ce->tag+doc->cur_col;
    doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
    *dst=0;
    doc_ne->type=DOCT_TEXT | old_color & -0x100;
    doc_ne->de_flags=old_de_flags|doldoc.dft_de_flags[DOCT_TEXT];
    doc_ce->max_col=doc->cur_col;
    doc_ne->x=doc_ce->x+doc->cur_col;
    QueIns(doc_ne,doc_ce);
    doc_ce=doc_ne;
    doc->cur_col=doc_ce->min_col;
  }
  doc->cur_entry=doc_ce;
  DocFormFwd(doc);
  if (!(doc->flags & DOCF_FORM))
    while (doc_ce!=doc && doc_ce!=doc->cur_entry) {
      if (sc>=0)
	BEqu(&doc_ce->type,DOCEt_SELECTED,sc&SCF_SHIFT);
      doc_ce=doc_ce->next;
    }
  doc->x=doc->cur_entry->x+doc->cur_col;
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
  }
  if (doc_ce->max_col!=0 &&
	(IsEditableText(doc_ce)||doc_ce->type_u8==DOCT_DATA)) {
    if (doc_ce->type_u8==DOCT_DATA && doc_ce->de_flags & DOCEF_HAS_TERMINATOR &&
	  doc->cur_col==doc_ce->max_col-1) {
      if (unlock)
	DocUnlock(doc);
      return;
    }
    if (doc->cur_col<doc_ce->max_col)
      StrCpy(doc_ce->tag+doc->cur_col,doc_ce->tag+doc->cur_col+1);
    if (doc->cur_col>=doc_ce->max_col-1) {
      doc->cur_entry=doc_ce->next;
      doc->cur_col=doc->cur_entry->min_col;
    }
    DocRemSoftNewLines(doc,doc->cur_entry);
    if (unlock)
      DocUnlock(doc);
    return;
  }
  doc->cur_entry=doc_ce->next;
  doc->cur_col=doc->cur_entry->min_col;
  if (!(doc_ce->de_flags&DOCEF_FILTER_SKIP))
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

  if (doc->flags & DOCF_IN_DOLLAR) {
    if (!Bt(chars_bmp_getkey,ch))
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
	doc->flags&=~DOCF_IN_DOLLAR;
	DocPrint(doc,doc->dollar_buf);
	doc->dollar_buf_ptr=0;
	goto ic_done;
      }
    } else
      goto ic_done;
  }
  if (ch=='$$' && !(doc->flags & (DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_WITH_TABS))) {
    doc->flags|=DOCF_IN_DOLLAR;
    doc->dollar_buf_ptr=0;
    doc->dollar_buf[doc->dollar_buf_ptr++]=ch;
    goto ic_done;
  }
  if (ch=='\r') goto ic_done;

ic_cont:
  if ((ch==CH_SPACE || ch=='\n') &&
	!(sc & (SCF_CTRL|SCF_SHIFT)) &&
	doc_ce->de_flags &
	(DOCEF_LINK|DOCEF_TREE|DOCEF_LST|DOCEF_CHECK_COLLAPSABLE|
	DOCEF_LEFT_MACRO|DOCEF_LEFT_EXP|DOCEF_LEFT_CB|DOCEF_LEFT_AUTO |
	DOCEF_RIGHT_MACRO|DOCEF_RIGHT_EXP|DOCEF_RIGHT_CB|DOCEF_RIGHT_AUTO)) {
    doc->cmd_U8=ch;
    DocEntryRun(doc,doc_ce,FALSE);
    DocLock(doc);
    goto ic_done;
  }
  if (doc_ce->type_u8==DOCT_HEX_ED) {
    if (doc_ce->de_flags & DOCEF_DATA_IS_PTR)
      b=doc_ce->data;
    else
      b=&doc_ce->data;
    i=doc->cur_col;
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
    doc->cur_col++;
    goto ic_done;
  }
  if (doc->flags & DOCF_OVERSTRIKE) {
    if (Bt(chars_bmp_displayable,ch)) {
ic_overstrike_char:
      if (IsEditableText(doc_ce)) {
	if (doc->cur_col<doc_ce->max_col) {
	  if (doc_ce->tag[doc->cur_col]) {
	    doc_ce->tag[doc->cur_col++]=ch;
	    goto ic_done;
	  }
	} else {
	  doc_ce=doc_ce->next;
	  doc->cur_entry=doc_ce;
	  doc->cur_col=doc_ce->min_col;
	  goto ic_overstrike_char;
	}
      } else if (doc_ce->type_u8==DOCT_DATA) {
	if (doc_ce->de_flags & DOCEF_HAS_TERMINATOR) {
	  if (doc_ce->tag[doc->cur_col] &&
		doc->cur_col<doc_ce->min_col+doc_ce->len) {
	    doc_ce->tag[doc->cur_col++]=ch;
	    if ( ! doc_ce->tag[doc->cur_col]) {
	      doc_ce->tag[doc->cur_col]='_';
	      doc_ce->tag[doc->cur_col+1]=0;
	    }
	  }
	} else if (doc_ce->tag[doc->cur_col])
	  doc_ce->tag[doc->cur_col++]=ch;
	goto ic_done;
      }
      b=MAlloc(2,doc->mem_task);
      *b=ch;
      b[1]=0;
      doc_ne=DocEntryNew(doc,doc_ce,b);
      doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr<<8;
      doc_ne->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
      QueIns(doc_ne,doc_ce->last);
    } else if (ch=='\n') {
      while (doc->cur_entry->next!=doc && doc->cur_entry->y==y)
	doc->cur_entry=doc->cur_entry->next;
      doc->cur_col=doc->cur_entry->min_col;
    } else if (ch=='\t') {
      if (doc->flags&DOCF_FORM)
	goto ic_form_tab;
    }
    goto ic_done;
  }
  if (ch=='\n') {
    doc_ne=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
    if (sc&SCF_CTRL && !(sc&SCF_SHIFT)) {
      doc_ne->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr<<8;
      doc_ne->de_flags=doldoc.dft_de_flags[DOCT_PAGE_BREAK];
    } else {
      doc_ne->type=DOCT_NEW_LINE | doc->settings_root.dft_text_attr<<8;
      doc_ne->de_flags=doldoc.dft_de_flags[DOCT_NEW_LINE];
    }
    DocInsEntry(doc,doc_ne);
  } else if (ch=='\t') {
    if (doc->flags&DOCF_FORM &&
	  (Bt(doldoc.type_flags_form,doc->cur_entry->type_u8) ||
	  doc->cur_entry->de_flags&DOCEF_LINK) &&
	  !(doc->cur_entry->de_flags&DOCEF_SKIP_IN_FORM)) {
ic_form_tab:
      doc->cur_entry=doc->cur_entry->next;
      doc->cur_col=doc->cur_entry->min_col;
      DocFormFwd(doc);
      goto ic_done;
    } else {
      doc_ne=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
      doc_ne->type=DOCT_TAB | doc->settings_root.dft_text_attr<<8;
      doc_ne->de_flags=doldoc.dft_de_flags[DOCT_TAB];
      DocInsEntry(doc,doc_ne);
    }
  } else {
    if (Bt(chars_bmp_displayable,ch)) {
      if (doc_ce->type_u8==DOCT_DATA) {
	i=doc_ce->len+doc_ce->min_col;
	if (doc_ce->de_flags & DOCEF_HAS_TERMINATOR)
	  i++;
	if (i>doc_ce->max_col) {
	  b=doc_ce->tag;
	  doc_ce->max_col++;
	  for (i=doc_ce->max_col;i>doc->cur_col;i--)
	    b[i]=b[i-1];
	  b[doc->cur_col++]=ch;
	}
      } else if (IsEditableText(doc_ce)) {
	dst=b=MAlloc(doc_ce->max_col+2,doc->mem_task);
	src=doc_ce->tag;
	i=doc->cur_col;
	while (i-->0)
	  *dst++=*src++;
	*dst++=ch;
	while (*dst++=*src++);
	Free(doc_ce->tag);
	doc_ce->tag=b;
	doc_ce->max_col++;
	doc->cur_col++;
      } else {
	b=MAlloc(2,doc->mem_task);
	*b=ch;
	b[1]=0;
	doc_ne=DocEntryNew(doc,doc_ce,b);
	doc_ne->type=DOCT_TEXT | doc->settings_root.dft_text_attr<<8;
	doc_ne->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
	doc_ne->x=doc_ce->x+1;
	QueIns(doc_ne,doc_ce->last);
      }
    }
  }
ic_done:
  DocRemSoftNewLines(doc,doc->cur_entry);
  if (doc->cur_entry->de_flags & DOCEF_UPDATE_DATA &&
	(doc->cur_entry->type_u8==DOCT_DATA ||
	doc->cur_entry->type_u8==DOCT_CHECK_BOX))
    DocDataScan(doc,doc->cur_entry);
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
  doc->cur_col=doc_ce->min_col;
  doc_ce=doc_ce->last;
  while (doc_ce!=doc && doc_ce->y==y) {
    doc_ce2=doc_ce->last;
    if (!(doc_ce->de_flags&DOCEF_FILTER_SKIP))
      DocEntryDel(doc,doc_ce);
    doc_ce=doc_ce2;
  }
}
