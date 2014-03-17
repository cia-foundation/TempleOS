#help_index "Doc"

U0 DocInit()
{ //$LK-UL,"CMD Defines",A="MN:DOCT_TEXT"$
  CLex *lx;
  I64 i;
  CHashGeneric *temph;

  DefineLstLoad("ST_DOC_CMDS",
	"TX\0CR\0SR\0CU\0TB\0CL\0PB\0DA\0PL\0LM\0RM\0HD\0FO\0ID\0"
	"FG\0BG\0FD\0BD\0PT\0WW\0UL\0IV\0BK\0SX\0SY\0CM\0AN\0LK\0BT\0"
	"CB\0MA\0MU\0HX\0TR\0SP\0IB\0IS\0SO\0HL\0LP\0CC\0HC\0ER\0");
  DefineLstLoad("ST_DOC_FLAGS",
	"T\0A\0L\0"
	"LC\0LE\0LM\0LA\0RC\0RE\0RM\0RA\0"
	"TC\0BI\0X\0Q\0LX\0CX\0RX\0TY\0CY\0BY\0SX\0SY\0SCX\0"
	"HL\0LP\0CC\0WW\0BK\0IV\0SEL\0UL\0"
	"FST\0B\0S\0BD\0C\0CA\0RD\0P\0TRM\0Z\0H\0TR\0SK\0PU\0PRY\0MRX\0"
	"SIF\0WD\0HT\0HS\0FS\0NC\0DD\0UD\0"
	"U8\0U16\0U32\0U64\0"
	"D\0WR\0HTML\0BP\0"
	);
  DefineLstLoad("ST_LINK_TYPES","FI\0FA\0FF\0FL\0MN\0PI\0PF\0PL\0BF\0DF\0HI\0AD\0");

  doc_hash=HashTableNew(512);
  HashDefineLstAdd("ST_DOC_CMDS",DHT_DOC_CMD,doc_hash);
  HashDefineLstAdd("ST_DOC_FLAGS",DHT_DOC_FLAG,doc_hash);

  MemSet(doc_dft_flags1,	0,sizeof(doc_dft_flags1));
  MemSet(doc_dft_type_flags,	0,sizeof(doc_dft_type_flags));
  lx=LexNew("TX+T;DA+T+P+TRM+U64;CM+LE+RE;AN+T;LK+T+L+UL;MA+T+X+UL;MU+T+X+UL;"
	"BT+T+X+B;CB+T+CA+P+U8;HX+P+Z;TR+T+C+CA+TR+UL;SP+T;IB+T;IS+T;"
	"SO+T;HC+T;",LF_DONT_FREE_BUF);
  lx->htc.hash_table_lst=NULL;
  Lex(lx);
  while (lx->token==TK_IDENT) {
    if (temph=HashFind(lx->cur_str,doc_hash,DHT_DOC_CMD)) {
      i=temph->user_data0;
      Lex(lx);
      PrsDocFlags(lx,&doc_dft_flags1[i],&doc_dft_type_flags[i]);
    }
    if (lx->token==';')
      Lex(lx);
  }
  LexDel(lx);

  for (i=0;i<DOCT_NUM_TYPES;i++)
    doc_dft_flags1[i]&=~DOCG_BL_IV_UL;

  doc_type_flags_form[0]=0;
  for (i=0;i<DOCT_NUM_TYPES;i++)
    if (Bt(&doc_dft_flags1[i],DOCEf1_TAG) && i!=DOCT_TEXT || i==DOCT_HEX_ED)
      Bts(doc_type_flags_form,i);

  //Don't forget $LK,"doc_type_flags_src_code",A="FF:::/Compiler/Lex.CPP,doc_type_flags_src_code"$
  doc_type_flags_nontag_invisible[0]=
	1<<DOCT_PAGE_LEN|
	1<<DOCT_LEFT_MARGIN|
	1<<DOCT_RIGHT_MARGIN|
	1<<DOCT_HEADER|
	1<<DOCT_FOOTER|
	1<<DOCT_INDENT|
	1<<DOCT_FOREGROUND|
	1<<DOCT_BACKGROUND|
	1<<DOCT_DFT_FOREGROUND|
	1<<DOCT_DFT_BACKGROUND|
	1<<DOCT_PMT|
	1<<DOCT_WORD_WRAP|
	1<<DOCT_UNDERLINE|
	1<<DOCT_INVERT|
	1<<DOCT_BLINK|
	1<<DOCT_SHIFTED_X|
	1<<DOCT_SHIFTED_Y|
	1<<DOCT_SPRITE|
	1<<DOCT_HIGHLIGHT|
	1<<DOCT_LOCK_TO_PAGE|
	1<<DOCT_LOCK_TO_CENTER;

  doc_type_flags_data[0]=1<<DOCT_DATA|1<<DOCT_HEX_ED|1<<DOCT_CHECK_BOX;

  Bts(doc_clean_scancodes,SC_CURSOR_RIGHT);
  Bts(doc_clean_scancodes,SC_CURSOR_LEFT);
  Bts(doc_clean_scancodes,SC_CURSOR_UP);
  Bts(doc_clean_scancodes,SC_CURSOR_DOWN);
  Bts(doc_clean_scancodes,SC_PAGE_UP);
  Bts(doc_clean_scancodes,SC_PAGE_DOWN);
  Bts(doc_clean_scancodes,SC_ALT);
  Bts(doc_clean_scancodes,SC_CTRL);
  Bts(doc_clean_scancodes,SC_SHIFT);
  Bts(doc_clean_scancodes,SC_CAPS);
  Bts(doc_clean_scancodes,SC_NUM);
  Bts(doc_clean_scancodes,SC_SCROLL);
  Bts(doc_clean_scancodes,SC_HOME);
  Bts(doc_clean_scancodes,SC_END);
  Bts(doc_clean_scancodes,SC_GUI);
  Bts(doc_clean_scancodes,SC_BACKSPACE); //Handled specially

  HashDefineLstAdd("ST_COLORS",DHT_COLOR,doc_hash);
}

DocInit;
