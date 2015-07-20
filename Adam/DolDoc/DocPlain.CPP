#help_index "DolDoc"

I64 PrsDocFlagSingle(CCmpCtrl *cc,I64 *_de_flags,U32 *_type,Bool turn_on)
{
  I64 result=-1;
  CHashGeneric *temph;
  if (cc->token==TK_IDENT &&
	(temph=HashFind(cc->cur_str,doldoc.hash,DHT_DOC_FLAG))) {
    result=temph->user_data0;
    if (result<64) {
      BEqu(_de_flags,result,turn_on);
      switch (result) {
	case DOCEf_SELECTED:
	case DOCEf_UNDERLINE:
	case DOCEf_INVERT:
	case DOCEf_BLINK:
	  BEqu(_type,result,turn_on);
	  break;
      }
    }
    Lex(cc);	//skip flag
  }
  return result;
}

I64 PrsDocFlags(CCmpCtrl *cc,I64 *_de_flags,U32 *_type)
{
  I64 result=-1;
  Bool turn_on;
  while (TRUE) {
    if (cc->token=='+')
      turn_on=TRUE;
    else if (cc->token=='-')
      turn_on=FALSE;
    else
      break;
    Lex(cc);
    result=PrsDocFlagSingle(cc,_de_flags,_type,turn_on);
  }
  return result;
}

I64 DocBinPtrRst(CDoc *doc,CDocEntry *doc_e)
{
  U8 *st,*st2;
  CDoc *doc2;
  CDocBin *tempb,*tempb2;
  I64 i,bin_num=0;
  if (doc_e->de_flags&DOCEF_HAS_BIN &&
	doc_e->bin_ptr_link && StrLen(doc_e->bin_ptr_link)) {
    bin_num=doc_e->bin_num;
    st=StrNew(doc_e->bin_ptr_link);
    st2=StrNew(st);
    StrLastRem(st,",",st2);
    i=Str2I64(st2);
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
	if (doc_e->de_flags&DOCEF_TAG && doc_e->tag && *doc_e->tag)
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
{//TODO: break strs
  I64 i,j,attr=doc_e->attr,
	t1,f1,de_flags,type;
  U8 *buf,*buf2;

  if (doc_e->type_u8==DOCT_FOREGROUND &&
	doc->flags&DOCF_COLOR_NAMES && 0<=attr<NUM_COLORS) {
    buf=StrNew(DefineSub(attr,"ST_COLORS"));
    attr=DOC_DFT;
  } else
    buf=StrNew(DefineSub(doc_e->type_u8,"ST_DOC_CMDS"));
  if (doc_e->type_u8!=DOCT_ERROR) {
    f1=doldoc.dft_de_flags[doc_e->type_u8];
    t1=doc_e->type_u8|doldoc.dft_type_flags[doc_e->type_u8];

    de_flags=doc_e->de_flags&~(DOCG_BL_IV_UL|DOCEF_SELECTED|
	  DOCEF_HIGHLIGHT|DOCEF_WORD_WRAP|DOCEF_SKIP|DOCEF_FILTER_SKIP);
    for (i=0;i<DOCEf_NUM_FLAGS;i++)
      if (Bt(&f1,i)!=Bt(&de_flags,i)) {
	if (Bt(&de_flags,i)) {
	  if (!(1<<i&DOCEG1_HAS_ARG)) {
	    buf2=MStrPrint("%s+%Z",buf,i,"ST_DOC_FLAGS");
	    Free(buf); buf=buf2;
	  }
	} else {
	  buf2=MStrPrint("%s-%Z",buf,i,"ST_DOC_FLAGS");
	  Free(buf); buf=buf2;
	}
      }
    type=doc_e->type&~DOCET_SELECTED;
    for (i=DOCEt_BLINK;i<=DOCEt_UNDERLINE;i++)
      if (Bt(&t1,i)!=Bt(&type,i)) {
	if (Bt(&type,i))
	  buf2=MStrPrint("%s+%Z",buf,i,"ST_DOC_FLAGS");
	else
	  buf2=MStrPrint("%s-%Z",buf,i,"ST_DOC_FLAGS");
	Free(buf); buf=buf2;
      }
    buf2=MStrPrint("%s,",buf);
    Free(buf); buf=buf2;
    nobound_switch (doc_e->type_u8) {
      sub_switch_start:
	case DOCT_DATA:
	  buf2=MStrPrint("%s%d,",buf,doc_e->len);
	  break;
	case DOCT_HEX_ED:
	  buf2=MStrPrint("%s%d,",buf,doc_e->len);
	  Free(buf); buf=buf2;
	  buf2=MStrPrint("%s%d,",buf,doc_e->hex_ed_width);
	  break;
      sub_switch_end:
	Free(buf); buf=buf2;
	break;

      case DOCT_FOREGROUND:
      case DOCT_BACKGROUND:
      case DOCT_DFT_FOREGROUND:
      case DOCT_DFT_BACKGROUND:
	if (doc->flags&DOCF_COLOR_NAMES && 0<=attr<NUM_COLORS) {
	  buf2=MStrPrint("%s%Z,",buf,doc_e->attr,"ST_COLORS");
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
      case DOCT_UNDERLINE:
      case DOCT_INVERT:
      case DOCT_BLINK:
      case DOCT_SHIFTED_X:
      case DOCT_SHIFTED_Y:
	if (attr!=DOC_DFT) {
	  buf2=MStrPrint("%s%d,",buf,doc_e->attr);
	  Free(buf); buf=buf2;
	}
      case DOCT_NUM_TYPES-1: //nobound_switch
	break;
    }
    de_flags=doc_e->de_flags & DOCEG1_HAS_ARG;
    while (de_flags) {
      j=Bsf(de_flags);
      Btr(&de_flags,j);
      nobound_switch (j) {
	case DOCEf_TAG:
	  if (doc_e->type_u8==DOCT_DATA || doc_e->type_u8==DOCT_MACRO &&
		(doc_e->de_flags&DOCEF_LEFT_MACRO &&
		!StrCmp(doc_e->tag,doc_e->left_macro) ||
		doc_e->de_flags&DOCEF_RIGHT_MACRO &&
		!StrCmp(doc_e->tag,doc_e->right_macro)) ||
		doc_e->de_flags&DOCEF_LST && !StrCmp(doc_e->tag,"[]") &&
		doc_e->de_flags&DOCEF_DEFINE) {
	    buf2=buf;
	    buf=NULL;
	  } else {
	    if (doc_e->type_u8==DOCT_CHECK_BOX) {
	      if (StrLen(doc_e->tag)>=4)
		buf2=doc_e->tag+4;
	      else
		buf2="";
	    } else if (doc_e->de_flags & DOCEF_TREE) {
	      if (StrLen(doc_e->tag)>=3)
		buf2=doc_e->tag+3;
	      else
		buf2="";
	    } else
	      buf2=doc_e->tag;
	    if (Bt(&doldoc.dft_de_flags[doc_e->type_u8],DOCEf_TAG))
	      buf2=MStrPrint("%s\"%$$Q\",",buf,buf2);
	    else
	      buf2=MStrPrint("%sT=\"%$$Q\",",buf,buf2);
	  }
	  break;
	case DOCEf_AUX_STR:
	  buf2=MStrPrint("%sA=\"%$$Q\",",buf,doc_e->aux_str);
	  break;
	case DOCEf_DEFINE:
	  buf2=MStrPrint("%sD=\"%$$Q\",",buf,doc_e->define_str);
	  break;
	case DOCEf_HTML_LINK:
	  buf2=MStrPrint("%sHTML=\"%$$Q\",",buf,doc_e->html_link);
	  break;
	case DOCEf_LEFT_EXP:
	  buf2=MStrPrint("%sLE=%d,",buf,doc_e->left_exp);
	  break;
	case DOCEf_LEFT_MACRO:
	  buf2=MStrPrint("%sLM=\"%$$Q\",",buf,doc_e->left_macro);
	  break;
	case DOCEf_RIGHT_EXP:
	  buf2=MStrPrint("%sRE=%d,",buf,doc_e->right_exp);
	  break;
	case DOCEf_RIGHT_MACRO:
	  buf2=MStrPrint("%sRM=\"%$$Q\",",buf,doc_e->right_macro);
	  break;
	case DOCEf_HAS_BIN:
	  buf2=MStrPrint("%sBI=%d,",buf,doc_e->bin_num);
	  break;
	case DOCEf_BIN_PTR_LINK:
	  buf2=MStrPrint("%sBP=\"%$$Q\",",buf,doc_e->bin_ptr_link);
	  break;
	case DOCEf_RAW_TYPE:
	  if (doc_e->type_u8==DOCT_CHECK_BOX&&doc_e->raw_type!=RT_I8 ||
		doc_e->type_u8!=DOCT_CHECK_BOX&&doc_e->raw_type!=RT_I64)
	    buf2=MStrPrint("%sRT=%Z,",buf,doc_e->raw_type,"ST_RAW_TYPES");
	  break;
	case DOCEf_SHIFTED_X:
	  j=doc_e->type.u16[1]&0x1F;
	  if (j&0x10) j|=0xFFFFFFF0;
	  buf2=MStrPrint("%sSX=%d,",buf,j);
	  break;
	case DOCEf_SHIFTED_Y:
	  j=doc_e->type>>21 &0x1F;
	  if (j&0x10) j|=0xFFFFFFF0;
	  buf2=MStrPrint("%sSY=%d,",buf,j);
	  break;
	case DOCEf_SCROLLING_X:
	  buf2=MStrPrint("%sSCX=%d,",buf,doc_e->scroll_len);
	  break;
	case DOCEf_USER_DATA:
	  buf2=MStrPrint("%sU=0x%X,",buf,doc_e->user_data);
	  break;
	case DOCEf_NUM_FLAGS-1: //nobound_switch
	  break;
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
{//Uses $LK-UL,"Lex",A="MN:Lex"$() to parse a string and make Doc entries.
  I64 i,j,de_flags,processed_flags,attr=DOC_DFT;
  U8 *ptr,*st2;
  CDocEntry *doc_e=NULL;
  CHashGeneric *temph;
  CCmpCtrl *cc=CmpCtrlNew(st,CCF_DONT_FREE_BUF);
  CHashTable *old_hash_table_lst=cc->htc.hash_table_lst;
  try {
    cc->htc.hash_table_lst=NULL;
    if (Lex(cc)==TK_IDENT) {
      if (temph=HashFind(cc->cur_str,doldoc.hash,DHT_DOC_CMD|DHT_COLOR)) {
	if (temph->type&DHT_DOC_CMD)
	  i=temph->user_data0;
	else {//DHT_COLOR
	  i=DOCT_FOREGROUND;
	  attr=temph->user_data0;
	}
      } else
	goto pd_err;
      Lex(cc); //skip cmd code
      doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
      doc_e->type=i;
      doc_e->de_flags=doldoc.dft_de_flags[i];
      doc_e->type|=doldoc.dft_type_flags[i];
      doc_e->raw_type=RT_I64;
      j=PrsDocFlags(cc,&doc_e->de_flags,&doc_e->type);
      cc->htc.hash_table_lst=old_hash_table_lst;
      nobound_switch (i) {
	case DOCT_DATA:
	  while (cc->token==',')
	    Lex(cc);
	  if (cc->token)
	    doc_e->len=LexExpressionI64(cc);
	  else
	    goto pd_err;
	  break;
	case DOCT_CHECK_BOX:
	  doc_e->raw_type=RT_I8;
	  break;
	case DOCT_HEX_ED:
	  while (cc->token==',')
	    Lex(cc);
	  if (cc->token)
	    doc_e->len=LexExpressionI64(cc);
	  else
	    goto pd_err;
	  while (cc->token==',')
	    Lex(cc);
	  if (cc->token)
	    doc_e->hex_ed_width=LexExpressionI64(cc);
	  else
	    goto pd_err;
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
	case DOCT_UNDERLINE:
	case DOCT_INVERT:
	case DOCT_BLINK:
	case DOCT_SHIFTED_X:
	case DOCT_SHIFTED_Y:
	  while (cc->token==',')
	    Lex(cc);
	  if (cc->token)
	    doc_e->attr=LexExpressionI64(cc);
	  else
	    doc_e->attr=attr;
	  break;
#assert DOCT_ERROR==DOCT_NUM_TYPES-1
	case DOCT_ERROR:
	  goto pd_err;
      }

      processed_flags=0;
      while (TRUE) {
	cc->htc.hash_table_lst=NULL;
	while (cc->token==',')
	  Lex(cc);
	cc->htc.hash_table_lst=old_hash_table_lst;
	j=PrsDocFlagSingle(cc,&doc_e->de_flags,&doc_e->type,TRUE);
	if (!(de_flags=~processed_flags & doc_e->de_flags & DOCEG1_HAS_ARG))
	  break;
	if (cc->token=='=')
	  Lex(cc);
	else
	  j=Bsf(de_flags);
	if (j<0 || Bts(&processed_flags,j))
	  goto pd_err;
	nobound_switch (j) {//TODO: Might check for expression errors
	  case DOCEf_TAG:
	    if (!doc_e->tag) {
//If a $$MA,LM=""$$, Tag is filled when the LM is processed.
	      //if doc_e->df_flags&DOCEF_LST,
	      // Tag is filled when the Define is processed.
	      //(The dft_flag1.tag calls this after.)
	      if (cc->token==TK_STR) {
		st2=LexExtStr(cc);
		if (i==DOCT_CHECK_BOX) {
		  st=MStrPrint("[X] %s",st2);
		  Free(st2);
		  doc_e->min_col=1;
		} else if (doc_e->de_flags & DOCEF_LST) {
		  if (*st2!='[') {
		    st=MStrPrint("[%s]",st2);
		    Free(st2);
		  } else
		    st=st2;
		  doc_e->min_col=1;
		} else if (doc_e->de_flags & DOCEF_TREE) {
		  st=MStrPrint("+] %s",st2);
		  Free(st2);
		  doc_e->min_col=1;
		} else
		  st=st2;
		doc_e->tag=StrNew(st,doc->mem_task);
		Free(st);
	      } else
		goto pd_err;
	    }
	    break;
	  case DOCEf_AUX_STR:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->aux_str=StrNew(st2,doc->mem_task);
	      Free(st2);
//$AN,"Anchor",A="DataTagWidth"$
	      if (i==DOCT_DATA) {
		if (ptr=StrMatch(":",doc_e->aux_str))
		  doc_e->min_col=ptr-doc_e->aux_str+1;
		doc_e->tag=MAlloc(doc_e->len+doc_e->min_col+2,
		      doc->mem_task); //+2 because "_\0"
	      }
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_DEFINE:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->define_str=StrNew(st2,doc->mem_task);
	      Free(st2);
	      if (doc_e->de_flags&DOCEF_LST && !doc_e->tag)
		doc_e->tag=StrNew("[]",doc->mem_task);
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_HTML_LINK:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->html_link=StrNew(st2,doc->mem_task);
	      Free(st2);
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_LEFT_EXP:
	    if (cc->token)
	      doc_e->left_exp=LexExpression(cc);
	    else
	      goto pd_err;
	    break;
	  case DOCEf_LEFT_MACRO:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->left_macro=StrNew(st2,doc->mem_task);
	      Free(st2);
	      if (i==DOCT_MACRO && !doc_e->tag)
		doc_e->tag=StrNew(doc_e->left_macro,doc->mem_task);
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_RIGHT_EXP:
	    if (cc->token)
	      doc_e->right_exp=LexExpression(cc);
	    else
	      goto pd_err;
	    break;
	  case DOCEf_RIGHT_MACRO:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->right_macro=StrNew(st2,doc->mem_task);
	      Free(st2);
	      if (i==DOCT_MACRO && !doc_e->tag)
		doc_e->tag=StrNew(doc_e->right_macro,doc->mem_task);
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_HAS_BIN:
	    if (cc->token)
	      doc_e->bin_num=LexExpressionI64(cc);
	    else
	      goto pd_err;
	    break;
	  case DOCEf_BIN_PTR_LINK:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->bin_ptr_link=StrNew(st2,doc->mem_task);
	      Free(st2);
	      if (!DocBinPtrRst(doc,doc_e))
		doc_e->type=DOCT_ERROR;
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_RAW_TYPE:
	    if (cc->token==TK_IDENT) {
	      j=DefineMatch(cc->cur_str,"ST_RAW_TYPES");
	      if (j<0)
		goto pd_err;
	      doc_e->raw_type=j;
	      Lex(cc);
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_SHIFTED_X:
	    if (cc->token)
	      doc_e->type|=(LexExpressionI64(cc) & 0x1F)<<16;
	    else
	      goto pd_err;
	    break;
	  case DOCEf_SHIFTED_Y:
	    if (cc->token)
	      doc_e->type|=(LexExpressionI64(cc) & 0x1F)<<21;
	    else
	      goto pd_err;
	    break;
	  case DOCEf_SCROLLING_X:
	    if (cc->token)
	      doc_e->scroll_len=LexExpressionI64(cc);
	    else
	      goto pd_err;
	    break;
	  case DOCEf_USER_DATA:
	    if (cc->token)
	      doc_e->user_data=LexExpression(cc);
	    else
	      goto pd_err;
	    break;
	  case DOCEf_NUM_FLAGS-1: //nobound_switch
	    break;
	}
      }
    } else {
pd_err:
      if (!doc_e)
	doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
      doc_e->type=DOCT_ERROR;
      doc_e->de_flags=0;
    }
    if (doc_e->de_flags & DOCEF_LST &&
	  !(doc_e->de_flags & DOCEF_DATA_IS_PTR)) {
      DocDataScan(doc,doc_e);
      DocDataFmt(doc,doc_e);
    }
    CmpCtrlDel(cc);
  } catch {
    Fs->catch_except=TRUE;
    if (!doc_e)
      doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
    doc_e->type=DOCT_ERROR;
    doc_e->de_flags=0;
  }
  return doc_e;
}

public U0 DocInsEntry(CDoc *doc,CDocEntry *doc_e)
{//Insert entry into doc, updating its vals.
  U8 *dst;
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;

  doc_e->x=doc_ce->x;
  doc_e->y=doc_ce->y;
  doc_e->page_line_num=doc_ce->page_line_num;
  MemCpy(&doc_e->settings,&doc_ce->settings,sizeof(CDocSettings));
  if (doc->cur_col>0 &&
	doc_ce->type_u8==DOCT_TEXT &&
	!(doc_ce->de_flags&(DOCEF_TAG_CB|DOCEF_DEFINE|
	DOCEF_HTML_LINK|DOCEF_BIN_PTR_LINK)) &&
	doc->cur_col<doc_ce->max_col) {
    dst=doc_ce->tag+doc->cur_col;
    doc_ne=DocEntryNew(doc,doc_ce,StrNew(dst,doc->mem_task));
    *dst=0;
    doc_ne->type=DOCT_TEXT|doc_ce->type&0xFFFFFF00;
    doc_ce->max_col=doc->cur_col;
    QueIns(doc_ne,doc_ce);
    doc->cur_col=0;
    doc_ce=doc_ne;
  }
  if (doc_ce->type_u8==DOCT_TEXT && doc->cur_col>=doc_ce->max_col) {
    QueIns(doc_e,doc_ce);
    doc->cur_entry=doc_e->next;
  } else {
    QueIns(doc_e,doc_ce->last);
    doc->cur_entry=doc_ce;
  }
  doc->cur_col=doc->cur_entry->min_col;
  DocRemSoftNewLines(doc,doc->cur_entry);
  if (unlock)
    DocUnlock(doc);
}

#help_index "DolDoc/Output;StdOut/DolDoc"
CDocEntry *DocPutS(CDoc *doc,U8 *st)
{//Don't use this.  Use $LK-UL,"DocPrint",A="MN:DocPrint"$().
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
    char_bitmap=chars_bmp_zero_cr_nl_cursor;
  else if (doc->flags & DOCF_PLAIN_TEXT)
    char_bitmap=chars_bmp_zero_tab_ff_cr_nl_cursor;
  else
    char_bitmap=chars_bmp_zero_tab_ff_cr_nl_cursor_dollar;
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
	doc_e->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
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
	doc_e->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
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
	  doc_e->de_flags=doldoc.dft_de_flags[DOCT_CURSOR];
	  DocInsEntry(doc,doc_e);
	  ptr++;
	  break;
	case CH_FORM_FEED:
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr<<8;
	  doc_e->de_flags=doldoc.dft_de_flags[DOCT_PAGE_BREAK];
	  DocInsEntry(doc,doc_e);
	  ptr++;
	  break;
	case '\t':
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_TAB | doc->settings_root.dft_text_attr<<8;
	  doc_e->de_flags=doldoc.dft_de_flags[DOCT_TAB];
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
	      doc_e->de_flags=doldoc.dft_de_flags[DOCT_CURSOR];
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
	      doc_e->de_flags=doldoc.dft_de_flags[DOCT_TEXT];
	      DocInsEntry(doc,doc_e);
	    } else {
	      st2=MAlloc(ptr-ptr2+1);
	      ptr3=ptr2;
	      ptr4=st2;
	      while (ch=*ptr3++) {
		if (ch==CH_CURSOR) {
		  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr<<8;
		  doc_e->de_flags=doldoc.dft_de_flags[DOCT_CURSOR];
		  DocInsEntry(doc,doc_e);
		} else if (ch==CH_FORM_FEED) {
		  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_e->type=DOCT_PAGE_BREAK |
			doc->settings_root.dft_text_attr<<8;
		  doc_e->de_flags=doldoc.dft_de_flags[DOCT_PAGE_BREAK];
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
	  doc_e->de_flags=doldoc.dft_de_flags[DOCT_NEW_LINE];
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
  U8 *buf=StrPrintJoin(NULL,fmt,argc,argv);
  CDocEntry *result=DocPutS(doc,buf);
  Free(buf);
  return result;
}
