U0 PrsDocFlags(CLex *lx,I64 *flags1,I64 *flags2,U32 *type)
{
  I64 i;
  Bool turn_on;
  while (lx->token=='+' || lx->token=='-') {
    turn_on= lx->token=='+';
    if (Lex(lx)==TK_IDENT) {
      i=DefineMatch(lx->ident,"ST_DOC_FLAGS",LEF_EXACT);
      if (i<0)
	*type=DOCT_ERROR;
      else if (i<64) {
	BEqu(flags1,i,turn_on);
	switch (i) {
	  case DOCEf1_SELECTED:
	  case DOCEf1_UNDERLINE:
	  case DOCEf1_INVERT:
	  case DOCEf1_BLINK:
	    BEqu(type,i,turn_on);
	    break;
	}
      } else
	BEqu(flags2,i-64,turn_on);
      Lex(lx);	//skip flag
    }
  }
}

I64 doc_dft_flags1[DOCT_NUM_TYPES]={
//DOCT_TEXT
0,
	
//DOCT_NEW_LINE
0,

//DOCT_SOFT_NEW_LINE
0,

//DOCT_CURSOR
0,

//DOCT_TAB
0,

//DOCT_CLEAR
0,

//DOCT_PAGE_BREAK
0,

//DOCT_DATA
DOCEF1_DATA_IS_PTR | DOCEF1_HAS_TERMINATOR | DOCEF1_AUX_STR | DOCEF1_U64,

//DOCT_PAGE_LEN
0,

//DOCT_LEFT_MARGIN
0,

//DOCT_RIGHT_MARGIN
0,

//DOCT_HEADER
0,

//DOCT_FOOTER
0,

//DOCT_INDENT
0,

//DOCT_FOREGROUND
0,

//DOCT_BACKGROUND
0,

//DOCT_DFT_FOREGROUND
0,

//DOCT_DFT_BACKGROUND
0,

//DOCT_ALT_FOREGROUND
0,

//DOCT_LINK_FOREGROUND
0,

//DOCT_MACRO_FOREGROUND
0,

//DOCT_ANCHOR_FOREGROUND
0,

//DOCT_HIDDEN_FOREGROUND
0,

//DOCT_TREE_FOREGROUND
0,

//DOCT_USER_FOREGROUND
0,

//DOCT_BINARY_FOREGROUND
0,

//DOCT_KEYWORD_FOREGROUND
0,

//DOCT_KEYWORD2_FOREGROUND
0,

//DOCT_COMMENT_FOREGROUND
0,

//DOCT_QUOTE_FOREGROUND
0,

//DOCT_WORD_WRAP
0,

//DOCT_UNDERLINE
0,

//DOCT_INVERT
0,

//DOCT_BLINK
0,

//DOCT_SHIFTED_X
0,

//DOCT_SHIFTED_Y
0,

//DOCT_CURSOR_MOVEMENT
DOCEF1_LEFT_EXP | DOCEF1_RIGHT_EXP,

//DOCT_ANCHOR
DOCEF1_AUX_STR,

//DOCT_LINK
DOCEF1_LINK|DOCEF1_AUX_STR,

//DOCT_BUTTON
DOCEF1_ESC | DOCEF1_LEFT_EXP | DOCEF1_HAS_BORDER,

//DOCT_CHECK_BOX
DOCEF1_CHECK_COLLAPSABLE|DOCEF1_DATA_IS_PTR|DOCEF1_U8,

//DOCT_MACRO
DOCEF1_ESC | DOCEF1_LEFT_MACRO,

//DOCT_MENU_VAL
DOCEF1_ESC | DOCEF1_LEFT_EXP,

//DOCT_HEX_ED
DOCEF1_DATA_IS_PTR | DOCEF1_ZERO_BASED,

//DOCT_HIDE_START
DOCEF1_CHECKED_COLLAPSED|DOCEF1_CHECK_COLLAPSABLE,

//DOCT_HIDE_END
0,

//DOCT_TREE
DOCEF1_CHECKED_COLLAPSED|DOCEF1_CHECK_COLLAPSABLE|DOCEF1_TREE,

//DOCT_SPRITE
DOCEF1_HAS_BIN,

//DOCT_INS_BINARY
DOCEF1_HAS_BIN,

//DOCT_INS_BINARY_TYPE
DOCEF1_HAS_BIN,

//DOCT_INS_BINARY_SIZE
DOCEF1_HAS_BIN,

//DOCT_BPT
0,

//DOCT_SONG
DOCEF1_AUX_STR,

//DOCT_HIGHLIGHT
0,

//DOCT_LOCK_TO_PAGE
0,

//DOCT_LOCK_TO_CENTER
0,

//DOCT_HTML_IMG
0,

//DOCT_HTML_VIDEO
0,

//DOCT_HTML_CODE
0,

//DOCT_ERROR
0};

I64 doc_dft_flags2[DOCT_NUM_TYPES]={
//DOCT_TEXT
0,
	
//DOCT_NEW_LINE
0,

//DOCT_SOFT_NEW_LINE
0,

//DOCT_CURSOR
0,

//DOCT_TAB
0,

//DOCT_CLEAR
0,

//DOCT_PAGE_BREAK
0,

//DOCT_DATA
0,

//DOCT_PAGE_LEN
0,

//DOCT_LEFT_MARGIN
0,

//DOCT_RIGHT_MARGIN
0,

//DOCT_HEADER
0,

//DOCT_FOOTER
0,

//DOCT_INDENT
0,

//DOCT_FOREGROUND
0,

//DOCT_BACKGROUND
0,

//DOCT_DFT_FOREGROUND
0,

//DOCT_DFT_BACKGROUND
0,

//DOCT_ALT_FOREGROUND
0,

//DOCT_LINK_FOREGROUND
0,

//DOCT_MACRO_FOREGROUND
0,

//DOCT_ANCHOR_FOREGROUND
0,

//DOCT_HIDDEN_FOREGROUND
0,

//DOCT_TREE_FOREGROUND
0,

//DOCT_USER_FOREGROUND
0,

//DOCT_BINARY_FOREGROUND
0,

//DOCT_KEYWORD_FOREGROUND
0,

//DOCT_KEYWORD2_FOREGROUND
0,

//DOCT_COMMENT_FOREGROUND
0,

//DOCT_QUOTE_FOREGROUND
0,

//DOCT_WORD_WRAP
0,

//DOCT_UNDERLINE
0,

//DOCT_INVERT
0,

//DOCT_BLINK
0,

//DOCT_SHIFTED_X
0,

//DOCT_SHIFTED_Y
0,

//DOCT_CURSOR_MOVEMENT
0,

//DOCT_ANCHOR
0,

//DOCT_LINK
0,

//DOCT_BUTTON
0,

//DOCT_CHECK_BOX
0,

//DOCT_MACRO
0,

//DOCT_MENU_VAL
0,

//DOCT_HEX_ED
0,

//DOCT_HIDE_START
0,

//DOCT_HIDE_END
0,

//DOCT_TREE
0,

//DOCT_SPRITE
0,

//DOCT_INS_BINARY
0,

//DOCT_INS_BINARY_TYPE
0,

//DOCT_INS_BINARY_SIZE
0,

//DOCT_BPT
0,

//DOCT_SONG
0,

//DOCT_HIGHLIGHT
0,

//DOCT_LOCK_TO_PAGE
0,

//DOCT_LOCK_TO_CENTER
0,

//DOCT_HTML_IMG
0,

//DOCT_HTML_VIDEO
0,

//DOCT_HTML_CODE
0,

//DOCT_ERROR
0};

I64 doc_dft_type_flags[DOCT_NUM_TYPES]={
//DOCT_TEXT
0,

//DOCT_NEW_LINE
0,

//DOCT_SOFT_NEW_LINE
0,

//DOCT_CURSOR
0,

//DOCT_TAB
0,

//DOCT_CLEAR
0,

//DOCT_PAGE_BREAK
0,

//DOCT_DATA
0,

//DOCT_PAGE_LEN
0,

//DOCT_LEFT_MARGIN
0,

//DOCT_RIGHT_MARGIN
0,

//DOCT_HEADER
0,

//DOCT_FOOTER
0,

//DOCT_INDENT
0,

//DOCT_FOREGROUND
0,

//DOCT_BACKGROUND
0,

//DOCT_DFT_FOREGROUND
0,

//DOCT_DFT_BACKGROUND
0,

//DOCT_ALT_FOREGROUND
0,

//DOCT_LINK_FOREGROUND
0,

//DOCT_MACRO_FOREGROUND
0,

//DOCT_ANCHOR_FOREGROUND
0,

//DOCT_HIDDEN_FOREGROUND
0,

//DOCT_TREE_FOREGROUND
0,

//DOCT_USER_FOREGROUND
0,

//DOCT_BINARY_FOREGROUND
0,

//DOCT_KEYWORD_FOREGROUND
0,

//DOCT_KEYWORD2_FOREGROUND
0,

//DOCT_COMMENT_FOREGROUND
0,

//DOCT_QUOTE_FOREGROUND
0,

//DOCT_WORD_WRAP
0,

//DOCT_UNDERLINE
0,

//DOCT_INVERT
0,

//DOCT_BLINK
0,

//DOCT_SHIFTED_X
0,

//DOCT_SHIFTED_Y
0,

//DOCT_CURSOR_MOVEMENT
0,

//DOCT_ANCHOR
0,

//DOCT_LINK
DOCET_UNDERLINE,

//DOCT_BUTTON
0,

//DOCT_CHECK_BOX
0,

//DOCT_MACRO
DOCET_UNDERLINE,

//DOCT_MENU_VAL
DOCET_UNDERLINE,

//DOCT_HEX_ED
0,

//DOCT_HIDE_START
DOCET_UNDERLINE,

//DOCT_HIDE_END
0,

//DOCT_TREE
DOCET_UNDERLINE,

//DOCT_SPRITE
0,

//DOCT_INS_BINARY
0,

//DOCT_INS_BINARY_TYPE
0,

//DOCT_INS_BINARY_SIZE
0,

//DOCT_BPT
0,

//DOCT_SONG
0,

//DOCT_HIGHLIGHT
0,

//DOCT_LOCK_TO_PAGE
0,

//DOCT_LOCK_TO_CENTER
0,

//DOCT_HTML_IMG
0,

//DOCT_HTML_VIDEO
0,

//DOCT_HTML_CODE
0,

//DOCT_ERROR
0
};

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
    i=A2I(st2);
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
	if (Bt(doc_tag_types,doc_e->type_u8) && doc_e->tag && *doc_e->tag)
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
  I64 i,j,
      t1,f1,f2,flags1,flags2,type;
  U8 *buf,*buf2;

  buf=StrNew(DefineSub(doc_e->type_u8,"ST_DOC_CMDS"));
  if (doc_e->type_u8!=DOCT_ERROR) {
    f1=doc_dft_flags1[doc_e->type_u8];
    f2=doc_dft_flags2[doc_e->type_u8];
    t1=doc_e->type_u8|doc_dft_type_flags[doc_e->type_u8];

    flags1=doc_e->flags1&~(DOCG_BL_IV_UL|DOCEF1_SELECTED|DOCEF1_HIGHLIGHT|DOCEF1_WORD_WRAP|DOCEF1_LOCK_TO_PAGE|DOCEF1_LOCK_TO_CENTER|DOCEF1_SKIP|DOCEF1_FILTER_SKIP);
    flags2=doc_e->flags2;
    for (i=0;i<DOCEf1_NUM_FLAGS+DOCEf2_NUM_FLAGS;i++)
      if (i<DOCEf1_NUM_FLAGS) {
	if (Bt(&f1,i)!=Bt(&flags1,i)) {
	  if (Bt(&flags1,i))
	    buf=StrAdd(buf,"+");
	  else
	    buf=StrAdd(buf,"-");
	  buf=StrAdd(buf,DefineSub(i,"ST_DOC_FLAGS"));
	}
      } else {
	if (Bt(&f2,i-64)!=Bt(&flags2,i-64)) {
	  if (Bt(&flags2,i-64))
	    buf=StrAdd(buf,"+");
	  else
	    buf=StrAdd(buf,"-");
	  buf=StrAdd(buf,DefineSub(i,"ST_DOC_FLAGS"));
	}
      }

    type=doc_e->type&~DOCET_SELECTED;
    for (i=DOCEt_BLINK;i<=DOCEt_UNDERLINE;i++)
      if (Bt(&t1,i)!=Bt(&type,i)) {
	if (Bt(&type,i))
	  buf=StrAdd(buf,"+");
	else
	  buf=StrAdd(buf,"-");
	buf=StrAdd(buf,DefineSub(i,"ST_DOC_FLAGS"));
      }
    buf=StrAdd(buf,",");
    nobound_switch (doc_e->type_u8) {
      case DOCT_TEXT:
      case DOCT_LINK:
      case DOCT_ANCHOR:
      case DOCT_MENU_VAL:
      case DOCT_BUTTON:
      case DOCT_CHECK_BOX:
      case DOCT_TREE:
      case DOCT_SPRITE:
      case DOCT_INS_BINARY:
      case DOCT_INS_BINARY_TYPE:
      case DOCT_INS_BINARY_SIZE:
      case DOCT_SONG:
      case DOCT_HTML_IMG:
      case DOCT_HTML_VIDEO:
      case DOCT_HTML_CODE:
	if (i==DOCT_CHECK_BOX)
	  buf2=doc_e->tag+4;
	else if (doc_e->flags1 & DOCEF1_TREE)
	  buf2=doc_e->tag+3;
	else
	  buf2=doc_e->tag;
	buf2=MSPrintF("%s\"%$$Q\",",buf,buf2);
	Free(buf); buf=buf2;
	break;
      case DOCT_DATA:
	buf2=MSPrintF("%s%d,",buf,doc_e->len);
	Free(buf); buf=buf2;
	break;
      case DOCT_HEX_ED:
	buf2=MSPrintF("%s%d,",buf,doc_e->len);
	Free(buf); buf=buf2;
	buf2=MSPrintF("%s%d,",buf,doc_e->hex_ed_width);
	Free(buf); buf=buf2;
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
      case DOCT_ALT_FOREGROUND:
      case DOCT_LINK_FOREGROUND:
      case DOCT_MACRO_FOREGROUND:
      case DOCT_ANCHOR_FOREGROUND:
      case DOCT_HIDDEN_FOREGROUND:
      case DOCT_TREE_FOREGROUND:
      case DOCT_USER_FOREGROUND:
      case DOCT_BINARY_FOREGROUND:
      case DOCT_KEYWORD_FOREGROUND:
      case DOCT_KEYWORD2_FOREGROUND:
      case DOCT_COMMENT_FOREGROUND:
      case DOCT_QUOTE_FOREGROUND:
      case DOCT_WORD_WRAP:
      case DOCT_HIGHLIGHT:
      case DOCT_LOCK_TO_PAGE:
      case DOCT_LOCK_TO_CENTER:
      case DOCT_UNDERLINE:
      case DOCT_INVERT:
      case DOCT_BLINK:
      case DOCT_SHIFTED_X:
      case DOCT_SHIFTED_Y:
	if (doc_e->attr!=DOC_DFT) {
	  buf2=MSPrintF("%s%d,",buf,doc_e->attr);
	  Free(buf); buf=buf2;
	}
      case DOCT_NUM_TYPES-1: //nobound_switch
	break;
    }
    flags1=doc_e->flags1& (DOCEF1_SCROLLING_X|DOCEF1_SHIFTED_X|DOCEF1_SCROLLING_Y|
    DOCEF1_AUX_STR|DOCEF1_LEFT_EXP|DOCEF1_LEFT_MACRO|
    DOCEF1_HAS_BIN|DOCEF1_RIGHT_EXP|DOCEF1_RIGHT_MACRO|
    DOCEF1_WIDTH|DOCEF1_HEIGHT|DOCEF1_DEFINE|DOCEF1_HTML_LINK);
    while (flags1) {
      j=Bsf(flags1);
      Btr(&flags1,j);
      nobound_switch (j) {
	case DOCEf1_SCROLLING_X:
	  buf2=MSPrintF("%s%d",buf,doc_e->scroll_len);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_SHIFTED_X:
	  j=doc_e->type.u16[1]&0x1F;
	  if (j&0x10) j|=0xFFFFFFF0;
	  buf2=MSPrintF("%s%d",buf,j);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_SHIFTED_Y:
	  j=doc_e->type>>21 &0x1F;
	  if (j&0x10) j|=0xFFFFFFF0;
	  buf2=MSPrintF("%s%d",buf,j);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_AUX_STR:
	  buf2=MSPrintF("%s\"%$$Q\"",buf,doc_e->aux_str);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_LEFT_EXP:
	  buf2=MSPrintF("%s%d",buf,doc_e->left_exp);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_LEFT_MACRO:
	  buf2=MSPrintF("%s\"%$$Q\"",buf,doc_e->left_macro);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_RIGHT_EXP:
	  buf2=MSPrintF("%s%d",buf,doc_e->right_exp);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_RIGHT_MACRO:
	  buf2=MSPrintF("%s\"%$$Q\"",buf,doc_e->right_macro);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_HAS_BIN:
	  buf2=MSPrintF("%s%d",buf,doc_e->bin_num);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_WIDTH:
	  buf2=MSPrintF("%s%d",buf,doc_e->width);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_HEIGHT:
	  buf2=MSPrintF("%s%d",buf,doc_e->height);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_DEFINE:
	  buf2=MSPrintF("%s\"%$$Q\"",buf,doc_e->define_str);
	  Free(buf); buf=buf2;
	  break;
	case DOCEf1_HTML_LINK:
	  buf2=MSPrintF("%s\"%$$Q\"",buf,doc_e->html_link);
	  Free(buf); buf=buf2;
	  break;
#assert DOCEf1_HTML_LINK==DOCEf1_NUM_FLAGS-1 //nobound_switch
      }
      buf=StrAdd(buf,",");
    }
    flags2=doc_e->flags2&DOCEF2_BIN_PTR_LINK;
    while (flags2) {
      j=Bsf(flags2);
      Btr(&flags2,j);
      nobound_switch (j) {
	case DOCEf2_BIN_PTR_LINK:
	  buf2=MSPrintF("%s\"%$$Q\"",buf,doc_e->bin_ptr_link);
	  Free(buf); buf=buf2;
	  break;
#assert DOCEf2_BIN_PTR_LINK==DOCEf2_NUM_FLAGS-1 //nobound_switch
      }
      buf=StrAdd(buf,",");
    }
    buf[StrLen(buf)-1]=0;  //Kill last comma
  }
  buf2=StrNew(buf,doc->mem_task); //exact allocation
  Free(buf);
  return buf2;
}


CDocEntry *PrsDollarCmd(CDoc *doc,U8 *st)
{//This uses $LK,"Lex","MN:Lex"$() to parse a string and make Doc entries.
  I64 i,j,flags1,flags2;
  U8 *ptr,*st2;
  CDocEntry *doc_e=NULL;
  CLex *lx=LexNew(st,LF_DONT_FREE_BUF);
  Lex(lx);
  if (lx->token==TK_IDENT) {
    i=DefineMatch(lx->ident,"ST_DOC_CMDS");
    if (i<0)
      i=DOCT_ERROR;
    Lex(lx); //skip cmd code
    doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
    doc_e->type=i;
    doc_e->flags1=doc_dft_flags1[i];
    doc_e->flags2=doc_dft_flags2[i];
    doc_e->type|=doc_dft_type_flags[i];
    PrsDocFlags(lx,&doc_e->flags1,&doc_e->flags2,&doc_e->type);
    if (lx->token==',')
      Lex(lx);
    nobound_switch (i) {
      case DOCT_TEXT:
      case DOCT_LINK:
      case DOCT_ANCHOR:
      case DOCT_MENU_VAL:
      case DOCT_BUTTON:
      case DOCT_CHECK_BOX:
      case DOCT_TREE:
      case DOCT_SPRITE:
      case DOCT_INS_BINARY:
      case DOCT_INS_BINARY_TYPE:
      case DOCT_INS_BINARY_SIZE:
      case DOCT_SONG:
      case DOCT_HTML_IMG:
      case DOCT_HTML_VIDEO:
      case DOCT_HTML_CODE:
	if (lx->token==TK_STR) {
	  st2=LexExtStr(lx);
	  if (i==DOCT_CHECK_BOX) {
	    st=MSPrintF("[X] %s",st2);
	    Free(st2);
	    doc_e->min_col=1;
	  } else if (doc_e->flags1 & DOCEF1_TREE) {
	    st=MSPrintF("+] %s",st2);
	    Free(st2);
	    doc_e->min_col=1;
	  } else
	    st=st2;
	  doc_e->tag=StrNew(st,doc->mem_task);
	  Free(st);
	} else {
	  if (DOCT_SPRITE<=i<=DOCT_INS_BINARY_SIZE ||
	      i==DOCT_SONG)
	    doc_e->tag=StrNew("",doc->mem_task);
	  else {
	    doc_e->tag=NULL;
	    doc_e->type=DOCT_ERROR;
	  }
	}
	break;
      case DOCT_HIDE_START:
	doc_e->tag=StrNew("<-",doc->mem_task);
	doc_e->min_col=1;
	break;
      case DOCT_HIDE_END:
	doc_e->tag=StrNew(">",doc->mem_task);
	break;
      case DOCT_DATA:
	doc_e->flags1|=DOCEF1_AUX_STR; //don't allow switched off
	doc_e->len=Expression(lx);
	break;
      case DOCT_HEX_ED:
	doc_e->len=Expression(lx);
	if (lx->token==',')
	  Lex(lx);
	doc_e->hex_ed_width=Expression(lx);
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
      case DOCT_ALT_FOREGROUND:
      case DOCT_LINK_FOREGROUND:
      case DOCT_MACRO_FOREGROUND:
      case DOCT_ANCHOR_FOREGROUND:
      case DOCT_HIDDEN_FOREGROUND:
      case DOCT_TREE_FOREGROUND:
      case DOCT_USER_FOREGROUND:
      case DOCT_BINARY_FOREGROUND:
      case DOCT_KEYWORD_FOREGROUND:
      case DOCT_KEYWORD2_FOREGROUND:
      case DOCT_COMMENT_FOREGROUND:
      case DOCT_QUOTE_FOREGROUND:
      case DOCT_WORD_WRAP:
      case DOCT_HIGHLIGHT:
      case DOCT_LOCK_TO_PAGE:
      case DOCT_LOCK_TO_CENTER:
      case DOCT_UNDERLINE:
      case DOCT_INVERT:
      case DOCT_BLINK:
      case DOCT_SHIFTED_X:
      case DOCT_SHIFTED_Y:
	if (!lx->token)
	  doc_e->attr=DOC_DFT;
	else
	  doc_e->attr=Expression(lx);
	break;
      case DOCT_ERROR:
	doc_e->flags1=0;
	doc_e->flags2=0;
	doc_e->type=DOCT_ERROR;
#assert DOCT_ERROR==DOCT_NUM_TYPES-1
//	case DOCT_NUM_TYPES-1: //nobound_switch
	break;
    }
 
    flags1=doc_e->flags1& (DOCEF1_SCROLLING_X|DOCEF1_SHIFTED_X|DOCEF1_SCROLLING_Y|
      DOCEF1_AUX_STR|DOCEF1_LEFT_EXP|DOCEF1_LEFT_MACRO|
      DOCEF1_HAS_BIN|DOCEF1_RIGHT_EXP|DOCEF1_RIGHT_MACRO|
      DOCEF1_WIDTH|DOCEF1_HEIGHT|DOCEF1_DEFINE|DOCEF1_HTML_LINK);
    while (flags1) {
      j=Bsf(flags1);
      Btr(&flags1,j);
      while (lx->token==',')
	Lex(lx);
      nobound_switch (j) {  //TODO: Might check for expression errors
	case DOCEf1_SCROLLING_X:
	  doc_e->scroll_len=Expression(lx);
	  break;
	case DOCEf1_SHIFTED_X:
	  doc_e->type|=(Expression(lx) & 0x1F)<<16;
	  break;
	case DOCEf1_SHIFTED_Y:
	  doc_e->type|=(Expression(lx) & 0x1F)<<21;
	  break;
	case DOCEf1_AUX_STR:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->aux_str=StrNew(st2,doc->mem_task);
	    Free(st2);
	    if (i==DOCT_MACRO && !doc_e->tag)
	      doc_e->tag=StrNew(doc_e->aux_str,doc->mem_task);
	    //$AN,"Anchor","DataTagWidth"$
	    if (i==DOCT_DATA) {
	      if (ptr=StrStr(doc_e->aux_str,":"))
		doc_e->min_col=ptr-doc_e->aux_str+1;
	      doc_e->tag=MAlloc(doc_e->len+doc_e->min_col+2,doc->mem_task); //+2 because "_\0"
	    }
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
	case DOCEf1_LEFT_EXP:
	  doc_e->left_exp=Expression(lx);
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
	  doc_e->right_exp=Expression(lx);
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
	    doc_e->bin_num=Expression(lx);
	  break;
	case DOCEf1_WIDTH:
	  doc_e->width=Expression(lx);
	  break;
	case DOCEf1_HEIGHT:
	  doc_e->height=Expression(lx);
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
#assert DOCEf1_HTML_LINK==DOCEf1_NUM_FLAGS-1 //nobound_switch
      }
    }
    flags2=doc_e->flags2&DOCEF2_BIN_PTR_LINK;
    while (flags2) {
      j=Bsf(flags2);
      Btr(&flags2,j);
      while (lx->token==',')
	Lex(lx);
      nobound_switch (j) {  //TODO: Might check for expression errors
	case DOCEf2_BIN_PTR_LINK:
	  if (lx->token==TK_STR) {
	    st2=LexExtStr(lx);
	    doc_e->bin_ptr_link=StrNew(st2,doc->mem_task);
	    Free(st2);
	    if (!DocBinPtrRst(doc,doc_e))
	      doc_e->type=DOCT_ERROR;
	  } else
	    doc_e->type=DOCT_ERROR;
	  break;
#assert DOCEf2_BIN_PTR_LINK==DOCEf2_NUM_FLAGS-1 //nobound_switch
      }
    }
  } else if (!doc_e) {
    doc_e=CAlloc(sizeof(CDocEntry),doc->mem_task);
    doc_e->type=DOCT_ERROR;
  }
  LexDel(lx);
  return doc_e;
}

public U0 DocIns(CDoc *doc,CDocEntry *doc_e)
{
  U8 *dst;
  Bool unlock=DocLock(doc);
  CDocEntry *doc_ce=doc->cur_entry,*doc_ne;

  doc_e->x=doc_ce->x;
  doc_e->y=doc_ce->y;
  doc_e->page_line_num=doc_ce->page_line_num;
  doc_e->settings=NULL;
  if (doc->cur_data_col>0 &&
      doc_ce->type_u8==DOCT_TEXT &&
      !(doc_ce->flags1&(DOCEF1_TAG_CB|DOCEF1_DEFINE|DOCEF1_HTML_LINK)) &&
      !(doc_ce->flags2&DOCEF2_BIN_PTR_LINK) &&
      doc->cur_data_col<doc_ce->max_col) {
    doc_ne=CAlloc(sizeof(CDocEntryBase)+sizeof(U8 *),doc->mem_task);
    doc_ne->type=DOCT_TEXT|doc_ce->type&0xFFFFFF00;
    doc_ne->x=doc_ce->x;
    doc_ne->y=doc_ce->y;
    doc_ne->page_line_num=doc_ce->page_line_num;
    doc_ne->flags1=doc_ce->flags1;
    doc_ne->flags2=doc_ce->flags2;
    dst=doc_ce->tag+doc->cur_data_col;
    doc_ne->tag=StrNew(dst,doc->mem_task);
    *dst=0;
    doc_ce->max_col=doc->cur_data_col;
    QueIns(doc_ne,doc_ce);
    doc->cur_data_col=0;
    doc_ce=doc_ne;
    doc_ce->max_col=StrLen(doc_ce->tag);
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
public CDocEntry *DocPutS(CDoc *doc,U8 *st)
{ //Does not handle partial Doc entries
  //Returns last newly created dollar-sign CDocEntry.
  U8 ch,*ptr=st,*ptr2,*st2,*ptr3,*ptr4,*src,
     *char_bitmap;
  Bool unlock;
  I64 j;
  CDocEntry *doc_e=NULL,*result=NULL,*doc_ce;
  if (!st || !doc || doc->doc_signature!=DOC_SIGNATURE_VAL)
    return NULL;
  unlock=DocLock(doc);
  if (doc->flags & DOCF_PLAIN_TEXT_WITH_TABS)
    char_bitmap=zero_cr_nl_cursor_bitmap;
  else if (doc->flags & DOCF_PLAIN_TEXT)
    char_bitmap=zero_tab_ff_cr_nl_cursor_bitmap;
  else
    char_bitmap=zero_tab_ff_cr_nl_cursor_dollar_bitmap;
  doc->recalc_start=doc; //TODO:This might be optimized
  doc_ce=doc->cur_entry;
  while (*ptr) {
    ptr2=ptr;
    do ch=*ptr++;
    while (!Bt(char_bitmap,ch) || ch==CH_CURSOR && doc->flags&DOCF_NO_CURSOR);
    ptr--;
    if (!ch) {
      if (j=ptr-ptr2) {
	doc_e=CAlloc(sizeof(CDocEntryBase)+sizeof(U8 *),doc->mem_task);
	doc_e->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
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
	DocIns(doc,doc_e);
	if (doc->flags & DOCF_NO_CURSOR)
	  Free(src);
      }
    } else {
      if (j=ptr-ptr2) {
	*ptr=0;
	doc_e=CAlloc(sizeof(CDocEntryBase)+sizeof(U8 *),doc->mem_task);
	doc_e->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
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
	DocIns(doc,doc_e);
	if (doc->flags & DOCF_NO_CURSOR)
	  Free(src);
	*ptr=ch;
      }
      switch (ch) {
	case CH_CURSOR:
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr << 8;
	  DocIns(doc,doc_e);
	  ptr++;
	  break;
	case CH_FORM_FEED:
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr << 8;
	  DocIns(doc,doc_e);
	  ptr++;
	  break;
	case '\t':
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_TAB | doc->settings_root.dft_text_attr << 8;
	  DocIns(doc,doc_e);
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
	      doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr << 8;
	      DocIns(doc,doc_e);
	      ptr2++;
	    }
	    if (ptr==ptr2) {
	      doc_e=CAlloc(sizeof(CDocEntryBase)+sizeof(U8 *),doc->mem_task);
	      doc_e->type=DOCT_TEXT | doc->settings_root.dft_text_attr << 8;
	      doc_e->max_col=1;
	      if (doc->flags & DOCF_DBL_DOLLARS)
		doc_e->tag=StrNew("$$$$",doc->mem_task);
	      else
		doc_e->tag=StrNew("$$",doc->mem_task);
	      DocIns(doc,doc_e);
	    } else {
	      st2=MAlloc(ptr-ptr2+1);
	      ptr3=ptr2;
	      ptr4=st2;
	      while (ch=*ptr3++) {
		if (ch==CH_CURSOR) {
		  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_e->type=DOCT_CURSOR | doc->settings_root.dft_text_attr << 8;
		  DocIns(doc,doc_e);
		} else if (ch==CH_FORM_FEED) {
		  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
		  doc_e->type=DOCT_PAGE_BREAK | doc->settings_root.dft_text_attr << 8;
		  DocIns(doc,doc_e);
		} else
		  *ptr4++=ch;
	      }
	      *ptr4=0;
	      if (doc_e=PrsDollarCmd(doc,st2)) {
		result=doc_e;
		DocIns(doc,doc_e);
	      }
	      Free(st2);
	    }
	    *ptr++='$$';
	  }
	  break;
	default:
	  doc_e=CAlloc(sizeof(CDocEntryBase),doc->mem_task);
	  doc_e->type=DOCT_NEW_LINE | doc->settings_root.dft_text_attr << 8;
	  DocIns(doc,doc_e);
	  if (ch==CH_CARRIAGE_RETURN)
	    while (*ptr==CH_CARRIAGE_RETURN)
	      ptr++;
	  if (*ptr=='\n')
	    ptr++;
	  while (*ptr==CH_CARRIAGE_RETURN)
	    ptr++;
      }
    }
  }
  if (unlock)
    DocUnlock(doc);
  return result;
}


public CDocEntry *DocPrintF(CDoc *doc,U8 *src,...)
{
  U8 *buf=SPrintFJoin(NULL,src,argc,argv);
  CDocEntry *result=DocPutS(doc,buf);
  Free(buf);
  return result;
}

#help_index "Doc"

