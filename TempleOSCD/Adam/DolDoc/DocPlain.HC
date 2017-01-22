#help_index "DolDoc"

I64 PrsDocFlagSingle(CCmpCtrl *cc,I64 *_de_flags,U32 *_type,Bool turn_on)
{
  I64 res=-1;
  CHashGeneric *tmph;
  if (cc->token==TK_IDENT &&
	(tmph=HashFind(cc->cur_str,doldoc.hash,DHT_DOC_FLAG))) {
    res=tmph->user_data0;
    if (res<64) {
      BEqu(_de_flags,res,turn_on);
      switch (res) {
	case DOCEf_BLINK:
	case DOCEf_INVERT:
	case DOCEf_UNDERLINE:
	case DOCEf_SEL:
	  BEqu(_type,res,turn_on);
	  break;
      }
    }
    Lex(cc);	//skip flag
  }
  return res;
}

I64 PrsDocFlags(CCmpCtrl *cc,I64 *_de_flags,U32 *_type)
{
  I64 res=-1;
  Bool turn_on;
  while (TRUE) {
    if (cc->token=='+')
      turn_on=TRUE;
    else if (cc->token=='-')
      turn_on=FALSE;
    else
      break;
    Lex(cc);
    res=PrsDocFlagSingle(cc,_de_flags,_type,turn_on);
  }
  return res;
}

U8 *Doc2PlainText(CDoc *doc,CDocEntry *doc_e)
{//TODO: break strs
  I64 i,j,attr=doc_e->attr,
	t1,f1,de_flags,type;
  U8 *buf,*buf2;

  if (doc_e->type_u8==DOCT_FOREGROUND &&
	doc->flags&DOCF_COLOR_NAMES && 0<=attr<COLORS_NUM) {
    buf=StrNew(DefineSub(attr,"ST_COLORS"));
    attr=DOC_DFT;
  } else
    buf=StrNew(DefineSub(doc_e->type_u8,"ST_DOC_CMDS"));
  if (doc_e->type_u8!=DOCT_ERROR) {
    f1=doldoc.dft_de_flags[doc_e->type_u8];
    t1=doc_e->type_u8|doldoc.dft_type_flags[doc_e->type_u8];

    de_flags=doc_e->de_flags&~(DOCG_BL_IV_UL|DOCEF_SEL|
	  DOCEF_HIGHLIGHT|DOCEF_WORD_WRAP|DOCEF_SKIP|DOCEF_FILTER_SKIP);
    for (i=0;i<DOCEf_FLAGS_NUM;i++)
      if (Bt(&f1,i)!=Bt(&de_flags,i)) {
	if (Bt(&de_flags,i)) {
	  if (!(1<<i&DOCEG_HAS_ARG)) {
	    buf2=MStrPrint("%s+%Z",buf,i,"ST_DOC_FLAGS");
	    Free(buf); buf=buf2;
	  }
	} else {
	  buf2=MStrPrint("%s-%Z",buf,i,"ST_DOC_FLAGS");
	  Free(buf); buf=buf2;
	}
      }
    type=doc_e->type&~DOCET_SEL;
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
    switch [doc_e->type_u8] {
      case DOCT_HEX_ED:
	buf2=MStrPrint("%s%d,",buf,doc_e->len);
	Free(buf); buf=buf2;
	buf2=MStrPrint("%s%d,",buf,doc_e->hex_ed_width);
	Free(buf); buf=buf2;
	break;
      case DOCT_FOREGROUND:
      case DOCT_BACKGROUND:
      case DOCT_DFT_FOREGROUND:
      case DOCT_DFT_BACKGROUND:
	if (doc->flags&DOCF_COLOR_NAMES && 0<=attr<COLORS_NUM) {
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
      case DOCT_BLINK:
      case DOCT_INVERT:
      case DOCT_UNDERLINE:
      case DOCT_SHIFTED_X:
      case DOCT_SHIFTED_Y:
	if (attr!=DOC_DFT) {
	  buf2=MStrPrint("%s%d,",buf,doc_e->attr);
	  Free(buf); buf=buf2;
	}
      case DOCT_TYPES_NUM-1: //nobound switch
	break;
    }
    de_flags=doc_e->de_flags & DOCEG_HAS_ARG;
    while (de_flags) {
      j=Bsf(de_flags);
      Btr(&de_flags,j);
      switch [j] {
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
	case DOCEf_LEN:
	  buf2=MStrPrint("%sLEN=%d,",buf,doc_e->len);
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
	case DOCEf_FLAGS_NUM-1: //nobound switch
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
  CHashGeneric *tmph;
  CCmpCtrl *cc=CmpCtrlNew(st,CCF_DONT_FREE_BUF);
  CHashTable *old_hash_table_lst=cc->htc.hash_table_lst;
  try {
    cc->htc.hash_table_lst=NULL;
    if (Lex(cc)==TK_IDENT) {
      if (tmph=HashFind(cc->cur_str,doldoc.hash,DHT_DOC_CMD|DHT_COLOR)) {
	if (tmph->type&DHT_DOC_CMD)
	  i=tmph->user_data0;
	else {//DHT_COLOR
	  i=DOCT_FOREGROUND;
	  attr=tmph->user_data0;
	}
      } else
	goto pd_err;
      Lex(cc); //skip cmd code
      doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
      doc_e->type=i;
      doc_e->de_flags=doldoc.dft_de_flags[i];
      doc_e->type|=doldoc.dft_type_flags[i];
      doc_e->raw_type=RT_I64;
      doc_e->len=DOCE_LEN_DFT;
      j=PrsDocFlags(cc,&doc_e->de_flags,&doc_e->type);
      cc->htc.hash_table_lst=old_hash_table_lst;
      switch [i] {
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
	case DOCT_BLINK:
	case DOCT_INVERT:
	case DOCT_UNDERLINE:
	case DOCT_SHIFTED_X:
	case DOCT_SHIFTED_Y:
	  while (cc->token==',')
	    Lex(cc);
	  if (cc->token)
	    doc_e->attr=LexExpressionI64(cc);
	  else
	    doc_e->attr=attr;
	  break;
#assert DOCT_ERROR==DOCT_TYPES_NUM-1
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
	if (!(de_flags=~processed_flags & doc_e->de_flags & DOCEG_HAS_ARG))
	  break;
	if (cc->token=='=')
	  Lex(cc);
	else
	  j=Bsf(de_flags);
	if (j<0 || Bts(&processed_flags,j))
	  goto pd_err;
	switch [j] {//TODO: Might check for expression errors
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
	  case DOCEf_LEN:
	    if (cc->token) {
	      doc_e->len=LexExpression(cc);
	      doc_e->de_flags&=~DOCEF_DFT_LEN;
	    } else
	      goto pd_err;
	    break;
	  case DOCEf_AUX_STR:
	    if (cc->token==TK_STR) {
	      st2=LexExtStr(cc);
	      doc_e->aux_str=StrNew(st2,doc->mem_task);
	      Free(st2);
//$AN,"Anchor",A="DataTagWidth"$
	      if (i==DOCT_DATA) { //See $LK,"DocForm",A="MN:DocForm"$()
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
	      doc_e->de_flags&=~DOCEF_DFT_RAW_TYPE;
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
	  case DOCEf_FLAGS_NUM-1: //nobound switch
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
    if (doc_e->de_flags&DOCEF_LST && (doc_e->de_flags&DOCEF_REMALLOC_DATA ||
	  !(doc_e->de_flags&DOCEF_DEREF_DATA))) {
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

U0 DocEntryToggle(CDoc *doc)
{
  Bool unlock=DocLock(doc),old_color_names;
  CDocEntry *doc_ce=doc->cur_entry,*cl1,*doc_ce2;
  U8 ch,*st,*st2;
  I64 i,j,k;
  if (doc_ce!=doc &&
	!(doc->flags&(DOCF_PLAIN_TEXT|DOCF_PLAIN_TEXT_TABS))) {
    if (doc_ce->type_u8==DOCT_TEXT && !(doc_ce->de_flags &
	  ~(DOCEF_TAG|DOCG_BL_IV_UL|DOCEF_WORD_WRAP|DOCEF_HIGHLIGHT|
	  DOCEF_SKIP|DOCEF_FILTER_SKIP)) &&
	  !(doc_ce->type&DOCG_BL_IV_UL)) {
      doc_ce2=doc_ce->last;
      for (k=0;k<20;k++) {
	if (doc_ce2!=doc) {
	  cl1=doc_ce2->last;
	  if (doc_ce2->type_u8==DOCT_TEXT &&
		doc_ce->de_flags==doc_ce2->de_flags &&
		doc_ce->type==doc_ce2->type) {
	    i=StrLen(doc_ce2->tag);
	    j=StrLen(doc_ce->tag);
	    st=MAlloc(i+j+1,doc->mem_task);
	    MemCpy(st,doc_ce2->tag,i);
	    MemCpy(st+i,doc_ce->tag,j+1);
	    Free(doc_ce->tag);
	    doc_ce->tag=st;
	    doc_ce->max_col=i+j;
	    doc->cur_col+=i;
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
	  if (doc_ce2->type_u8==DOCT_TEXT &&
		doc_ce->de_flags==doc_ce2->de_flags &&
		doc_ce->type==doc_ce2->type) {
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
      i=doc->cur_col;
      while (i>doc_ce->min_col && doc_ce->tag[i]!='$$')
	i--;
      j=doc->cur_col+1;
      while (j<doc_ce->max_col && doc_ce->tag[j]!='$$')
	j++;
      if (i<j-1 && doc_ce->min_col<=i<j<doc_ce->max_col &&
	    doc_ce->tag[i]=='$$' && doc_ce->tag[j]=='$$') {
	ch=doc_ce->tag[j+1];
	doc_ce->tag[j+1]=0;
	st=StrNew(doc_ce->tag+i);
	doc_ce->tag[j+1]=ch;
	StrCpy(doc_ce->tag+i,doc_ce->tag+j+1);
	doc->cur_col=i;
	st2=MStrPrint("%q",st);
	if (doc_ce=DocPrint(doc,st2)) {
	  doc->cur_entry=doc_ce;
	  doc->cur_col=doc_ce->min_col;
	}
	Free(st);
	Free(st2);
      }
    } else {
      old_color_names=LBts(&doc->flags,DOCf_COLOR_NAMES);
      st=Doc2PlainText(doc,doc_ce);
      LBEqu(&doc->flags,DOCf_COLOR_NAMES,old_color_names);
      DocEntryDel(doc,doc_ce);
      DocPrint(doc,"$$$$%$$Q$$$$",st);
    }
    DocRecalc(doc);
  }
  if (unlock)
    DocUnlock(doc);
}

U0 DocFlagsToggle(CDoc *doc,I64 tog_flags)
{
  Bool	unlock=DocLock(doc);
  I64 size,flags=doc->flags^tog_flags;
  U8 *st;
  CDocUndo *u_next,*u_last;

  doc->flags=doc->flags&~DOCF_NO_CURSOR|DOCF_COLOR_NAMES;
  st=DocSave(doc,&size);

  u_next=doc->undo_head.next;
  u_last=doc->undo_head.last;
  doc->undo_head.next=doc->undo_head.last=&doc->undo_head;

  DocRst(doc,TRUE);
  doc->undo_head.next=u_next;
  doc->undo_head.last=u_last;
  DocUndoCntSet(doc);
  doc->flags=flags&~(DOCF_NO_CURSOR|DOCG_BL_IV_UL|DOCF_WORD_WRAP);
  DocLoad(doc,st,size);
  doc->flags|=flags&DOCF_NO_CURSOR;
  DocCenter(doc);
  if (unlock)
    DocUnlock(doc);
  Free(st);
}
