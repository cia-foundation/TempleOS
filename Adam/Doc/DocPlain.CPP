#help_index "Doc"

I64 PrsDocFlagSingle(CLex *lx,I64 *_flags1,U32 *_type,Bool turn_on)
{
  I64 result=-1;
  CHashGeneric *temph;
  if (lx->token==TK_IDENT && (temph=HashFind(lx->cur_str,doc_hash,DHT_DOC_FLAG))) {
    result=temph->user_data0;
    if (result<64) {
      BEqu(_flags1,result,turn_on);
      switch (result) {
	case DOCEf1_SELECTED:
	case DOCEf1_UNDERLINE:
	case DOCEf1_INVERT:
	case DOCEf1_BLINK:
	  BEqu(_type,result,turn_on);
	  break;
      }
    }
    Lex(lx);	//skip flag
  }
  return result;
}

I64 PrsDocFlags(CLex *lx,I64 *_flags1,U32 *_type)
{
  I64 result=-1;
  Bool turn_on;
  while (TRUE) {
    if (lx->token=='+')
      turn_on=TRUE;
    else if (lx->token=='-')
      turn_on=FALSE;
    else
      break;
    Lex(lx);
    result=PrsDocFlagSingle(lx,_flags1,_type,turn_on);
  }
  return result;
}

I64 DocBinPtrRst(CDoc *doc,CDocEntry *doc_e)
{
  U8 *st,*st2;
  CDoc *doc2;
  CDocBin *tempb,*tempb2;
  I64 i,bin_num=0;
  if (doc_e->flags1&DOCEF1_HAS_BIN && doc_e->bin_ptr_link && StrLen(doc_e->bin_ptr_link)) {
    bin_num=doc_e->bin_num;
    st=StrNew(doc_e->bin_ptr_link);
    st2=StrNew(st);
    StrLastRem(st,",",st2);
    i=A2I64(st2);
    if (i>0||*st2) {
      doc2=DocRead(st);
      if (i>0  && (tempb2=DocBinFindNum(doc2,i)) ||
	  i==0 && (tempb2=DocBinFindTag(doc2,st2))) {
	i=1;
	if (bin_num>0) {
	  if (tempb=DocBinFindNum(doc,bin_num)) {
	    i=tempb->use_cnt;
	    DocBinDel(doc,tempb);
	  }
	} else
	  bin_num=doc->cur_bin_num++;
	tempb=MAllocIdentical(tempb2,doc->mem_task);
	tempb->use_cnt=i;
	tempb->data=MAllocIdentical(tempb2->data,doc->mem_task);
	tempb->num=bin_num;
	doc_e->bin_data=tempb;
	if (doc_e->flags1&DOCEF1_TAG && doc_e->tag && *doc_e->tag)
	  tempb->tag=StrNew(doc_e->tag,doc->mem_task);
	else
	  tempb->tag=NULL;
	QueIns(tempb,doc->bin_root.last);
      } else
	bin_num=0;
      DocDel(doc2);
    } else
      bin_num=0;
    Free(st2);
    Free(st);
    doc_e->bin_num=bin_num;
  }
  return bin_num;
}

U8 *Doc2PlainText(CDoc *doc,CDocEntry *doc_e)
{ //TODO: break strs
  I64 i,j,attr=doc_e->attr,
	t1,f1,flags1,type;
  U8 *buf,*buf2;

  if (doc_e->type_u8==DOCT_FOREGROUND && doc->flags&DOCF_COLOR_NAMES && 0<=attr<NUM_COLORS) {
    buf=StrNew(DefineSub(attr,"ST_COLORS"));
    attr=DOC_DFT;
  } else
    buf=StrNew(DefineSub(doc_e->type_u8,"ST_DOC_CMDS"));
  if (doc_e->type_u8!=DOCT_ERROR) {
    f1=doc_dft_flags1[doc_e->type_u8];
    t1=doc_e->type_u8|doc_dft_type_flags[doc_e->type_u8];

    flags1=doc_e->flags1&~(DOCG_BL_IV_UL|DOCEF1_SELECTED|DOCEF1_HIGHLIGHT|DOCEF1_WORD_WRAP|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER|DOCEF1_SKIP|DOCEF1_FILTER_SKIP);
    for (i=0;i<DOCEf1_NUM_FLAGS;i++)
      if (Bt(&f1,i)!=Bt(&flags1,i)) {
	if (Bt(&flags1,i)) {
	  if (!(1<<i&DOCEG1_HAS_ARG)) {
	    buf2=MPrint("%s+%Z",buf,i,"ST_DOC_FLAGS");
	    Free(buf); buf=buf2;
	  }
	} else {
	  buf2=MPrint("%s-%Z",buf,i,"ST_DOC_FLAGS");
	  Free(buf); buf=buf2;
	}
      }
    type=doc_e->type&~DOCET_SELECTED;
    for (i=DOCEt_BLINK;i<=DOCEt_UNDERLINE;i++)
      if (Bt(&t1,i)!=Bt(&type,i)) {
	if (Bt(&type,i))
	  buf2=MPrint("%s+%Z",buf,i,"ST_DOC_FLAGS");
	else
	  buf2=MPrint("%s-%Z",buf,i,"ST_DOC_FLAGS");
	Free(buf); buf=buf2;
      }
    buf2=MPrint("%s,",buf);
    Free(buf); buf=buf2;
    nobound_switch (doc_e->type_u8) {
      sub_switch_start:
	case DOCT_DATA:
	  buf2=MPrint("%s%d,",buf,doc_e->len);
	  break;
	case DOCT_HEX_ED:
	  buf2=MPrint("%s%d,",buf,doc_e->len);
	  Free(buf); buf=buf2;
	  buf2=MPrint("%s%d,",buf,doc_e->hex_ed_width);
	  break;
      sub_switch_end:
	Free(buf); buf=buf2;
	break;

      case DOCT_FOREGROUND:
      case DOCT_BACKGROUND:
      case DOCT_DFT_FOREGROUND:
      case DOCT_DFT_BACKGROUND:
	if (doc->flags&DOCF_COLOR_NAMES && 0<=attr<NUM_COLORS) {
	  buf2=MPrint("%s%Z,",buf,doc_e->attr,"ST_COLORS");
	  Free(buf); buf=buf2;
	  break;
	}
      case DOCT_PAGE_LEN:
      case DOCT_LEFT_MARGIN:
      case DOCT_RIGHT_MARGIN:
      case DOCT_HEADER:
      case DOCT_FOOTER:
      case DOCT_INDENT:
      case DOCT_WORD_WRAP:
      case DOCT_HIGHLIGHT:
      case DOCT_LOCK_TO_PAGE:
      case DOCT_LOCK_TO_CENTER:
      case DOCT_UNDERLINE:
      case DOCT_INVERT:
      case DOCT_BLINK:
      case DOCT_SHIFTED_X:
      case DOCT_SHIFTED_Y:
	if (attr!=DOC_DFT) {
	  buf2=MPrint("%s%d,",buf,doc_e->attr);
	  Free(buf); buf=buf2;
	}
      case DOCT_NUM_TYPES-1: //nobound_switch
	break;
    }
    flags1=doc_e->flags1 & DOCEG1_HAS_ARG;
    while (flags1) {
      j=Bsf(flags1);
      Btr(&flags1,j);
      nobound_switch (j) {
	case DOCEf1_TAG:
	  if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_MACRO &&
	      (doc_e->flags1&DOCEF1_LEFT_MACRO && !StrCmp(doc_e->tag,doc_e->left_macro) ||
	       doc_e->flags1&DOCEF1_RIGHT_MACRO && !StrCmp(doc_e->tag,doc_e->right_macro))) {
	    buf2=buf;
	    buf=NULL;
	  } else {
	    if (doc_e->type_u8==DOCT_CHECK_BOX) {
	      if (StrLen(doc_e->tag)>=4)
		buf2=doc_e->tag+4;
	      else
		buf2="";
	    } else if (doc_e->flags1 & DOCEF1_TREE) {
	      if (StrLen(doc_e->tag)>=3)
		buf2=doc_e->tag+3;
	      else
		buf2="";
	    } else
	      buf2=doc_e->tag;
	    if (Bt(&doc_dft_flags1[doc_e->type_u8],DOCEf1_TAG))
	      buf2=MPrint("%s\"%$$Q\",",buf,buf2);
	    else
	      buf2=MPrint("%sT=\"%$$Q\",",buf,buf2);
	  }
	  break;
	case DOCEf1_SCROLLING_X:
	  buf2=MPrint("%sSCX=%d,",buf,doc_e->scroll_len);
	  break;
	case DOCEf1_SHIFTED_X:
	  j=doc_e->type.u16[1]&0x1F;
	  if (j&0x10) j|=0xFFFFFFF0;
	  buf2=MPrint("%sSX=%d,",buf,j);
	  break;
	case DOCEf1_SHIFTED_Y:
	  j=doc_e->type>>21 &0x1F;
	  if (j&0x10) j|=0xFFFFFFF0;
	  buf2=MPrint("%sSY=%d,",buf,j);
	  break;
	case DOCEf1_AUX_STR:
	  buf2=MPrint("%sA=\"%$$Q\",",buf,doc_e->aux_str);
	  break;
	case DOCEf1_LEFT_EXP:
	  buf2=MPrint("%sLE=%d,",buf,doc_e->left_exp);
	  break;
	case DOCEf1_LEFT_MACRO:
	  buf2=MPrint("%sLM=\"%$$Q\",",buf,doc_e->left_macro);
	  break;
	case DOCEf1_RIGHT_EXP:
	  buf2=MPrint("%sRE=%d,",buf,doc_e->right_exp);
	  break;
	case DOCEf1_RIGHT_MACRO:
	  buf2=MPrint("%sRM=\"%$$Q\",",buf,doc_e->right_macro);
	  break;
	case DOCEf1_HAS_BIN:
	  buf2=MPrint("%sBI=%d,",buf,doc_e->bin_num);
	  break;
	case DOCEf1_WIDTH:
	  buf2=MPrint("%sWD=%d,",buf,doc_e->width);
	  break;
	case DOCEf1_HEIGHT:
	  buf2=MPrint("%sHT=%d,",buf,doc_e->height);
	  break;
	case DOCEf1_DEFINE:
	  buf2=MPrint("%sD=\"%$$Q\",",buf,doc_e->define_str);
	  break;
	case DOCEf1_HTML_LINK:
	  buf2=MPrint("%sHTML=\"%$$Q\",",buf,doc_e->html_link);
	  break;
	case DOCEf1_BIN_PTR_LINK:
	  buf2=MPrint("%sBP=\"%$$Q\",",buf,doc_e->bin_ptr_link);
	  break;
#assert DOCEf1_BIN_PTR_LINK==DOCEf1_NUM_FLAGS-1 //nobound_switch
      }
      Free(buf); buf=buf2;
    }
    buf[StrLen(buf)-1]=0;  //Kill last comma
  }
  buf2=StrNew(buf,doc->mem_task); //exact allocation
  Free(buf);
  return buf2;
}


CDocEntry *PrsDollarCmd(CDoc *doc,U8 *st)
{//This uses $LK-UL,"Lex",A="MN:Lex"$() to parse a string and make Doc entries.
  I64 i,j,flags1,processed_flags,attr=DOC_DFT;
  U8 *ptr,*st2;
  CDocEntry *doc_e=NULL;
  CHashGeneric *temph;
  CLex *lx=LexNew(st,LF_DONT_FREE_BUF);
  CHashTable *old_hash_table_lst=lx->htc.hash_table_lst;
  lx->htc.hash_table_lst=NULL;
  if (Lex(lx)==TK_IDENT) {
    if (temph=HashFind(lx->cur_str,doc_hash,DHT_DOC_CMD|DHT_COLOR)) {
      if (temph->type&DHT_DOC_CMD)
	i=temph->user_data0;
      else { //DHT_COLOR
	i=DOCT_FOREGROUND;
	attr=temph->user_data0;
      }
    } else
      i=DOCT_ERROR;
    Lex(lx); //skip cmd code
    doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
    doc_e->type=i;
    doc_e->flags1=doc_dft_flags1[i];
    doc_e->type|=doc_dft_type_flags[i];
    j=PrsDocFlags(lx,&doc_e->flags1,&doc_e->type);
    lx->htc.hash_table_lst=old_hash_table_lst;
    nobound_switch (i) {
      case DOCT_DATA:
	while (lx->token==',')
	  Lex(lx);
	doc_e->len=LexExpression(lx);
	break;
      case DOCT_HEX_ED:
	while (lx->token==',')
	  Lex(lx);
	doc_e->len=LexExpression(lx);
	while (lx->token==',')
	  Lex(lx);
	doc_e->hex_ed_width=LexExpression(lx);
	break;
      case DOCT_PAGE_LEN:
      case DOCT_LEFT_MARGIN:
      case DOCT_RIGHT_MARGIN:
      case DOCT_HEADER:
      case DOCT_FOOTER:
      case DOCT_INDENT:
      case DOCT_FOREGROUND:
      case DOCT_BACKGROUND:
      case DOCT_DFT_FOREGROUND:
      case DOCT_DFT_BACKGROUND:
      case DOCT_WORD_WRAP:
      case DOCT_HIGHLIGHT:
      case DOCT_LOCK_TO_PAGE:
      case DOCT_LOCK_TO_CENTER:
      case DOCT_UNDERLINE:
      case DOCT_INVERT:
      case DOCT_BLINK:
      case DOCT_SHIFTED_X:
      case DOCT_SHIFTED_Y:
	while (lx->token==',')
	  Lex(lx);
	if (lx->token)
	  doc_e->attr=LexExpression(lx);
	else
	  doc_e->attr=attr;
	break;
      case DOCT_ERROR:
	doc_e->flags1=0;
	doc_e->type=DOCT_ERROR;
#assert DOCT_ERROR==DOCT_NUM_TYPES-1
	//	case DOCT_NUM_TYPES-1: //nobound_switch
	break;
    }

    processed_flags=0;
    while (TRUE) {
      lx->htc.hash_table_lst=NULL;
      while (lx->token==',')
	Lex(lx);
      lx->htc.hash_table_lst=old_hash_table_lst;
      j=PrsDocFlagSingle(lx,&doc_e->flags1,&doc_e->type,TRUE);
      if (!(flags1=~processed_flags & doc_e->flags1 & DOCEG1_HAS_ARG))
	break;
      if (lx->token=='=')
	Lex(lx);
      else
	j=Bsf(flags1);
      if (j<0 || Bts(&processed_flags,j)) {
	doc_e->flags1=0;
	doc_e->type=DOCT_ERROR;
	break;
      }
      nobound_switch (j) {  //TODO: Might check for expression errors
	case DOCEf1_TAG:
	  if (!doc_e->tag) {	//If a $$MA,LM=""$$  Tag is filled when the LM is
				//processed. The dft_flag1.tag calls this after.
	    if (lx->token==TK_STR) {
	      st2=LexExtStr(lx);
	      if (i==DOCT_CHECK_BOX) {
		st=MPrint("[X] %s",st2);
		Free(st2);
		doc_e->min_col=1;
	      } else if (doc_e->flags1 & DOCEF1_TREE) {
		st=MPrint("+] %s",st2);
		Free(st2);
		doc_e->min_col=1;
	      } else
		st=st2;
	      doc_e->tag=StrNew(st,doc->mem_task);
	      Free(st);
	    } else {
	      doc_e->tag=NULL;
	      doc_e->type=DOCT_ERROR;
	    }
	  }
	  break;
	case DOCEf1_SCROLLING_X:
	  doc_e->scroll_len=LexExpression(lx);
	  break;
	case DOCEf1_SHIFTED_X:
	  doc_e->type|=(LexExpression(lx) & 0x1F)<<16;
	  break;
	case DOCEf1_SHIFTED_Y:
	  doc_e->type|=(LexExpression(lx) & 0x1F)<<21;
	  break;
	case DOCEf1_AUX_STR:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->aux_str=StrNew(st2,doc->mem_task);
	    Free(st2);
//$AN,"Anchor",A="DataTagWidth"$
	    if (i==DOCT_DATA) {
	      if (ptr=StrMatch(":",doc_e->aux_str))
		doc_e->min_col=ptr-doc_e->aux_str+1;
	      doc_e->tag=MAlloc(doc_e->len+doc_e->min_col+2,doc->mem_task); //+2 because "_\0"
	    }
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
	case DOCEf1_LEFT_EXP:
	  doc_e->left_exp=LexExpression(lx);
	  break;
	case DOCEf1_LEFT_MACRO:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->left_macro=StrNew(st2,doc->mem_task);
	    Free(st2);
	    if (i==DOCT_MACRO && !doc_e->tag)
	      doc_e->tag=StrNew(doc_e->left_macro,doc->mem_task);
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
	case DOCEf1_RIGHT_EXP:
	  doc_e->right_exp=LexExpression(lx);
	  break;
	case DOCEf1_RIGHT_MACRO:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->right_macro=StrNew(st2,doc->mem_task);
	    Free(st2);
	    if (i==DOCT_MACRO && !doc_e->tag)
	      doc_e->tag=StrNew(doc_e->right_macro,doc->mem_task);
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
	case DOCEf1_HAS_BIN:
	  if (!lx->token)
	    doc_e->bin_num=0;
	  else
	    doc_e->bin_num=LexExpression(lx);
	  break;
	case DOCEf1_WIDTH:
	  doc_e->width=LexExpression(lx);
	  break;
	case DOCEf1_HEIGHT:
	  doc_e->height=LexExpression(lx);
	  break;
	case DOCEf1_DEFINE:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->define_str=StrNew(st2,doc->mem_task);
	    Free(st2);
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
	case DOCEf1_HTML_LINK:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->html_link=StrNew(st2,doc->mem_task);
	    Free(st2);
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
	case DOCEf1_BIN_PTR_LINK:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->bin_ptr_link=StrNew(st2,doc->mem_task);
	    Free(st2);
	    if (!DocBinPtrRst(doc,doc_e))
	      doc_e->type=DOCT_ERROR;
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
#assert DOCEf1_BIN_PTR_LINK==DOCEf1_NUM_FLAGS-1 //nobound_switch
      }
    }
  } else if (!doc_e) {
    doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
    doc_e->type=DOCT_ERROR;
  }
  LexDel(lx);
  return doc_e;
}

public U0 DocInsEntry(CDoc *doc,CDocEntry *doc_e)
{
  U8 *dst;
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;

  doc_e->x=doc_ce->x;
  doc_e->y=doc_ce->y;
  doc_e->page_line_num=doc_ce->page_line_num;
  MemCpy(&doc_e->settings,&doc_ce->settings,sizeof(CDocSettings));
  if (doc->cur_data_col>0 &&
      doc_ce->type_u8==DOCT_TEXT &&
      !(doc_ce->flags1&(DOCEF1_TAG_CB|DOCEF1_DEFINE|
	DOCEF1_HTML_LINK|DOCEF1_BIN_PTR_LINK)) &&
      doc->cur_data_col<doc_ce->max_col) {
    dst=doc_ce->tag+doc->cur_data_col;
    doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
    *dst=0;
    doc_ne->type=DOCT_TEXT|doc_ce->type&0xFFFFFF00;
    doc_ce->max_col=doc->cur_data_col;
    QueIns(doc_ne,doc_ce);
    doc->cur_data_col=0;
    doc_ce=doc_ne;
  }
  if (doc_ce->type_u8==DOCT_TEXT && doc->cur_data_col>=doc_ce->max_col) {
    QueIns(doc_e,doc_ce);
    doc->cur_entry=doc_e->next;
  } else {
    QueIns(doc_e,doc_ce->last);
    doc->cur_entry=doc_ce;
  }
  doc->cur_data_col=doc->cur_entry->min_col;
  DocRemSoftNewLines(doc,doc->cur_entry);
  if (unlock)
    DocUnlock(doc);
}


#help_index "Doc/Output"
CDocEntry *DocPutS(CDoc *doc,U8 *st)
{ //Don't use this.  Use $LK-UL,"DocPrint",A="MN:DocPrint"$().
  //Does not handle partial Doc entries
  //Returns last newly created dollar-sign CDocEntry.
  U8 *ptr=st,*ptr2,*st2,*ptr3,*ptr4,*src,
     *char_bitmap;
  Bool unlock;
  I64 ch,j;
  CDocEntry *doc_e=NULL,*result=NULL,*doc_ce;
  if (!st || !doc && !(doc=DocPut) || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return NULL;
  unlock=DocLock(doc);
  if (doc->flags & DOCF_PLAIN_TEXT_WITH_TABS)
    char_bitmap=chars_bitmap_zero_cr_nl_cursor;
  else if (doc->flags & DOCF_PLAIN_TEXT)
    char_bitmap=chars_bitmap_zero_tab_ff_cr_nl_cursor;
  else
    char_bitmap=chars_bitmap_zero_tab_ff_cr_nl_cursor_dollar;
  doc->recalc_start=doc; //TODO:This might be optimized
  doc_ce=doc->cur_entry;
  while (*ptr) {
    ptr2=ptr;
    do ch=*ptr++;
    while (!Bt(char_bitmap,ch) || ch==CH_CURSOR && doc->flags&DOCF_NO_CURSOR);
    ptr--;
    if (!ch) {
      if (j=ptr-ptr2) {
	doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	doc_e->type=DOCT_TEXT | doc->settings_root.dft_text_attr<<8;
	doc_e->flags1=doc_dft_flags1[DOCT_TEXT];
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
	doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	doc_e->type=DOCT_TEXT | doc->settings_root.dft_text_attr<<8;
	doc_e->flags1=doc_dft_flags1[DOCT_TEXT];
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
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr<<8;
	  doc_e->flags1=doc_dft_flags1[DOCT_CURSOR];
	  DocInsEntry(doc,doc_e);
	  ptr++;
	  break;
	case CH_FORM_FEED:
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr<<8;
	  doc_e->flags1=doc_dft_flags1[DOCT_PAGE_BREAK];
	  DocInsEntry(doc,doc_e);
	  ptr++;
	  break;
	case '\t':
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_TAB | doc->settings_root.dft_text_attr<<8;
	  doc_e->flags1=doc_dft_flags1[DOCT_TAB];
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
	      doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	      doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr<<8;
	      doc_e->flags1=doc_dft_flags1[DOCT_CURSOR];
	      DocInsEntry(doc,doc_e);
	      ptr2++;
	    }
	    if (ptr==ptr2) {
	      doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	      doc_e->type=DOCT_TEXT | doc->settings_root.dft_text_attr<<8;
	      doc_e->max_col=1;
	      if (doc->flags & DOCF_DBL_DOLLARS)
		doc_e->tag=StrNew("$$$$",doc->mem_task);
	      else
		doc_e->tag=StrNew("$$",doc->mem_task);
	      doc_e->flags1=doc_dft_flags1[DOCT_TEXT];
	      DocInsEntry(doc,doc_e);
	    } else {
	      st2=MAlloc(ptr-ptr2+1);
	      ptr3=ptr2;
	      ptr4=st2;
	      while (ch=*ptr3++) {
		if (ch==CH_CURSOR) {
		  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr<<8;
		  doc_e->flags1=doc_dft_flags1[DOCT_CURSOR];
		  DocInsEntry(doc,doc_e);
		} else if (ch==CH_FORM_FEED) {
		  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_e->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr<<8;
		  doc_e->flags1=doc_dft_flags1[DOCT_PAGE_BREAK];
		  DocInsEntry(doc,doc_e);
		} else
		  *ptr4++=ch;
	      }
	      *ptr4=0;
	      if (doc_e=PrsDollarCmd(doc,st2)) {
		result=doc_e;
		DocInsEntry(doc,doc_e);
	      }
	      Free(st2);
	    }
	    *ptr++='$$';
	  }
	  break;
	default:
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_NEW_LINE | doc->settings_root.dft_text_attr<<8;
	  doc_e->flags1=doc_dft_flags1[DOCT_NEW_LINE];
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
  return result;
}

public CDocEntry *DocPrint(CDoc *doc,U8 *fmt,...)
{//You must not print partial doc cmds.
  U8 *buf=SPrintJoin(NULL,fmt,argc,argv);
  CDocEntry *result=DocPutS(doc,buf);
  Free(buf);
  return result;
}
