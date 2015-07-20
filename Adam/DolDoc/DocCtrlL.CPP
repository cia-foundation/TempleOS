#help_index "DolDoc/Misc"

U8 *ctrl_L_footer=
	"\n$$MU-X+Q,\"Abort\",LE=DOCM_CANCEL$$\n"
	"\n\n$$LK+PU,\"Click for Help\",A=\"FI:::/Doc/Widget.TXT\"$$\n";

#define DFT_ADDR_LINK_BIN_SIZE	64

I64 PopUpLinkType(Bool include_anchor)
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Link Type Menu\"$$\n"
	"\n$$LTBLUE$$$$MU,\"To file\",LE=LK_FILE$$\n"
	"$$MU,\"To anchor in file\",LE=LK_FILE_ANCHOR$$\n"
	"$$MU,\"To str in file\",LE=LK_FILE_FIND$$\n"
	"$$MU,\"To line in file\",LE=LK_FILE_LINE$$\n"
	"$$MU,\"To man page\",LE=LK_MAN_PAGE$$\n"
	"$$MU,\"To plain-text file\",LE=LK_PLAIN$$\n"
	"$$MU,\"To str in plain-text file\",LE=LK_PLAIN_FIND$$\n"
	"$$MU,\"To line in plain-text file\",LE=LK_PLAIN_LINE$$\n"
	"$$MU,\"To Bible chapter line and verse\",LE=LK_BIBLE_FIND$$\n"
	"$$MU,\"To Dictionary Definition\",LE=LK_DEF$$\n"
	"$$MU,\"To Help Index\",LE=LK_HELP_INDEX$$\n"
	"$$MU,\"To Addr\",LE=LK_ADDR$$\n");
  if (include_anchor)
    DocPrint(doc,"$$MU,\"Place Anchor\",LE=LK_PLACE_ANCHOR$$\n");
  DocPrint(doc,"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdFileLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n",
	aux[512];
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdFileAnchorLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n",
	aux[512]	fmtstr "$$DA-P,511,A=\"Anchor Label  :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdFileFindLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n",
	aux[512]	fmtstr "$$DA-P,511,A=\"Str        :%s\"$$\n";
  I64	num		fmtstr "$$DA-TRM,511,A=\"Occurrence Num:%04d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdFileLineLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n",
	aux[512];
  I64	num		fmtstr "$$DA-TRM,511,A=\"Line Num      :%04d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdManPageLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512],
	aux[512]	fmtstr "$$DA-P,511,A=\"Label         :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdAddrLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512],
	aux[512]	fmtstr "$$DA-P,511,A=\"Addr Exp   :%s\"$$\n";
  I64	num		fmtstr "$$DA-TRM,511,A=\"Bin Size      :%04d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdPlaceAnchor
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512],
	aux[512]	fmtstr "$$DA-P,511,A=\"Anchor Label  :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up,
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide		fmtstr "$$CB,\"Hide\"$$\n";
};

class CEdBibleLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512],
	aux[512]	fmtstr "$$DA-P,511,A=\"Chapter Verse :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdDefLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512],
	aux[512]	fmtstr "$$DA-P,511,A=\"Word          :%s\"$$\n";
  I64	num		fmtstr "$$DA-TRM,511,A=\"Def Num       :%4d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

class CEdProject
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n",
	aux[512];
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n",
	quote		fmtstr "$$CB,\"Quote\"$$\n",
	hide;
};

Bool GetLink(I64 type,U8 **_tag,U8 **_link,Bool *_pop_up,
	Bool *_quote,U8 **_html_link)
{
  I64 i;
  CEdFileLink *e=CAlloc(sizeof(CEdFileLink));
  Bool result=FALSE;
  if (type>=0) {
    e->num=1;
    switch (type) {
      case LK_FILE:
	if (DocForm(e,,,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->file);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("FI:%s",e->file);
	}
	break;
      case LK_PLAIN:
	if (DocForm(e,,,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->file);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("PI:%s",e->file);
	}
	break;
      case LK_FILE_ANCHOR:
	if (DocForm(e,"CEdFileAnchorLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->aux);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("FA:%s,%s",e->file,e->aux);
	}
	break;
      case LK_FILE_FIND:
	if (DocForm(e,"CEdFileFindLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (e->num==1) {
	    if (!*e->tag)
	      *_tag=StrNew(e->aux);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("FF:%s,%s",e->file,e->aux);
	  } else {
	    if (!*e->tag)
	      *_tag=StrNew(e->aux);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("FF:%s,%s:%d",e->file,e->aux,e->num);
	  }
	}
	break;
      case LK_PLAIN_FIND:
	if (DocForm(e,"CEdFileFindLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (e->num==1) {
	    if (!*e->tag)
	      *_tag=StrNew(e->aux);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("PF:%s,%s",e->file,e->aux);
	  } else {
	    if (!*e->tag)
	      *_tag=StrNew(e->aux);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("PF:%s,%s:%d",e->file,e->aux,e->num);
	  }
	}
	break;
      case LK_FILE_LINE:
	if (DocForm(e,"CEdFileLineLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->file);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("FL:%s,%d",e->file,e->num);
	}
	break;
      case LK_PLAIN_LINE:
	if (DocForm(e,"CEdFileLineLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->file);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("PL:%s,%d",e->file,e->num);
	}
	break;
      case LK_MAN_PAGE:
	if (DocForm(e,"CEdManPageLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->aux);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("MN:%s",e->aux);
	}
	break;
      case LK_PLACE_ANCHOR:
	if (DocForm(e,"CEdPlaceAnchor",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag) {
	    if (e->hide)
	      *_tag=StrNew("");
	    else
	      *_tag=StrNew(e->aux);
	  } else
	    *_tag=StrNew(e->tag);
	  *_link=StrNew(e->aux);
	}
	break;
      case LK_BIBLE_FIND: //TODO
	i=PopUpPickDefineSub("ST_BIBLE_BOOKS");
	if (i>=0) {
	  if (DocForm(e,"CEdBibleLink",,,ctrl_L_footer)) {
	    result=TRUE;
	    if (!*e->tag)
	      *_tag=MStrPrint("%Z,%s",i,"ST_BIBLE_BOOKS",e->aux);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("BF:%Z,%s",i,"ST_BIBLE_BOOKS",e->aux);
	  }
	}
	break;
      case LK_DEF:
	e->num=-1;
	if (DocForm(e,"CEdDefLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->aux);
	  else
	    *_tag=StrNew(e->tag);
	  if (e->num<0)
	    *_link=MStrPrint("DF:%s",e->aux);
	  else
	    *_link=MStrPrint("DF:%s,%d",e->aux,e->num);

	}
	break;
      case LK_HELP_INDEX:
	if (DocForm(e,"CEdManPageLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*e->tag)
	    *_tag=StrNew(e->aux);
	  else
	    *_tag=StrNew(e->tag);
	  *_link=MStrPrint("HI:%s",e->aux);
	}
	break;
      case LK_ADDR:
	e->num=DFT_ADDR_LINK_BIN_SIZE;
	if (DocForm(e,"CEdAddrLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (e->num==DFT_ADDR_LINK_BIN_SIZE) {
	    if (!*e->tag)
	      *_tag=StrNew(e->aux);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("AD:%s",e->aux);
	  } else {
	    if (!*e->tag)
	      *_tag=MStrPrint("%s,%d",e->aux,e->num);
	    else
	      *_tag=StrNew(e->tag);
	    *_link=MStrPrint("AD:%s,%d",e->aux,e->num);
	  }
	}
	break;
    }
  }
  if (*e->html_link)
    *_html_link=StrNew(e->html_link);
  else
    *_html_link=NULL;
  if (e->pop_up)
    *_pop_up=TRUE;
  else
    *_pop_up=FALSE;
  if (e->quote)
    *_quote=TRUE;
  else
    *_quote=FALSE;
  Free(e);
  return result;
}

U0 EdInsLink()
{
  U8 *tag=NULL,*link=NULL,*st=NULL,*html_link=NULL,*pop_up_st;
  Bool pop_up=FALSE,quote=FALSE;
  I64 type=PopUpLinkType(TRUE);
  if (type>=0) {
    if (GetLink(type,&tag,&link,&pop_up,&quote,&html_link)) {
      if (pop_up)
	pop_up_st="+PU";
      else
	pop_up_st="";
      switch (type) {
	case LK_PLACE_ANCHOR:
	  if (html_link)
	    st=MStrPrint("$$AN,\"%Q\",A=\"%Q\",HTML=\"%Q\"$$",tag,link,html_link);
	  else
	    st=MStrPrint("$$AN,\"%Q\",A=\"%Q\"$$",tag,link);
	  break;
	default:
	  if (html_link)
	    st=MStrPrint("$$LK%s,\"%Q\",A=\"%Q\",HTML=\"%Q\"$$",
		  pop_up_st,tag,link,html_link);
	  else
	    st=MStrPrint("$$LK%s,\"%Q\",A=\"%Q\"$$",pop_up_st,tag,link);
      }
    }
  }
  if (st) {
    if (quote)
      "%Q",st;
    else
      "%s",st;
  }
  Free(tag);
  Free(link);
  Free(html_link);
  Free(st);
}

I64 PopUpColorType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Color Type Menu\"$$\n"
	"\n$$LTBLUE$$$$MU,\"Foreground\",LE=DOCT_FOREGROUND$$\n"
	"$$MU,\"Background\",LE=DOCT_BACKGROUND$$\n"
	"$$MU,\"Default Foreground\",LE=DOCT_DFT_FOREGROUND$$\n"
	"$$MU,\"Default Background\",LE=DOCT_DFT_BACKGROUND$$\n"
	"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 PopUpPageSettingType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Page Setting Menu\"$$\n"
	"\n$$LTBLUE$$$$MU,\"Page Length\",LE=DOCT_PAGE_LEN$$\n"
	"$$MU,\"Page Header\",LE=DOCT_HEADER$$\n"
	"$$MU,\"Page Footer\",LE=DOCT_FOOTER$$\n"
	"$$MU,\"Left Margin\",LE=DOCT_LEFT_MARGIN$$\n"
	"$$MU,\"Right Margin\",LE=DOCT_RIGHT_MARGIN$$\n"
	"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

U0 EdInsColor()
{
  I64 type=PopUpColorType,col=DOC_DFT;
  if (type>=0) {
    col=PopUpColor(,FALSE);
    if (col==DOC_DFT)
      "$$%Z$$",type,"ST_DOC_CMDS";
    else if (col>=0)
      "$$%Z,%d$$",type,"ST_DOC_CMDS",col;
  }
}

class CEdPageSetting1
{
  U8	val[512]	fmtstr "$$DA-P,511,A=\"Setting Val:%s\"$$\n";
  Bool	winrel;
};

class CEdPageSetting2
{
  U8	val[512]	fmtstr "$$DA-P,511,A=\"Setting Val:%s\"$$\n";
  Bool	winrel		fmtstr "$$CB,\"Win Relative\"$$\n";
};

U0 EdInsPageSetting()
{
  I64 type=PopUpPageSettingType;
  CEdPageSetting1 *e=CAlloc(sizeof(CEdPageSetting1));
  if (type>=0) {
    if (type==DOCT_RIGHT_MARGIN||type==DOCT_PAGE_LEN) {
      if (DocForm(e(CEdPageSetting2 *))) {
	if (e->winrel) {
	  if (*e->val)
	    "$$%Z+WR,%s$$",type,"ST_DOC_CMDS",e->val;
	  else
	    "$$%Z+WR,0$$",type,"ST_DOC_CMDS";
	} else {
	  if (*e->val)
	    "$$%Z,%s$$",type,"ST_DOC_CMDS",e->val;
	  else
	    "$$%Z$$",type,"ST_DOC_CMDS";
	}
      }
    } else {
      if (DocForm(e)) {
	if (*e->val)
	  "$$%Z,%s$$",type,"ST_DOC_CMDS",e->val;
	else
	  "$$%Z$$",type,"ST_DOC_CMDS";
      }
    }
  }
  Free(e);
}

I64 PopUpDollarType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Text Widget Menu\"$$\n"
	"\n$$LTBLUE$$$$MU,\"Link\",LE=DOCT_LINK$$\n"
	"$$MU,\"Text\",LE=DOCT_TEXT$$\n"
	"$$MU,\"Tree Branch\",LE=DOCT_TREE$$\n"
	"$$MU,\"Color\",LE=DOCT_FOREGROUND$$\n"
	"$$MU,\"Page Settings\",LE=DOCT_PAGE_LEN$$\n"
	"$$MU,\"Cursor Movement\",LE=DOCT_CURSOR_MOVEMENT$$\n"
	"$$MU,\"Macro\",LE=DOCT_MACRO$$\n"
	"$$MU,\"Button\",LE=DOCT_BUTTON$$\n"
	"$$MU,\"Check Box\",LE=DOCT_CHECK_BOX$$\n"
	"$$MU,\"List\",LE=DOCT_LST$$\n"
	"$$MU,\"Menu Val\",LE=DOCT_MENU_VAL$$\n"
	"$$MU,\"Data\",LE=DOCT_DATA$$\n"
	"$$MU,\"Hex Edit\",LE=DOCT_HEX_ED$$\n"
	"$$MU,\"Syntax Highlighting\",LE=DOCT_HIGHLIGHT$$\n"
	"$$MU,\"HTML\",LE=DOCT_HTML_CODE$$\n"
	"$$MU,\"Song\",LE=DOCT_SONG$$\n"
	"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdDollarText
{
  U8	tag[512]	  fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  Bool	left_x	  fmtstr "$$CB,\"Left X\"$$\n",
	center_x  fmtstr "$$CB,\"Center X\"$$\n",
	right_x   fmtstr "$$CB,\"Right X\"$$\n",
	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n",
	blink	  fmtstr "$$CB,\"Blink\"$$\n",
	invert	  fmtstr "$$CB,\"Invert\"$$\n",
	underline fmtstr "$$CB,\"Underline\"$$\n",
	tree	  fmtstr "$$CB,\"Tree\"$$\n",
	collapsed fmtstr "$$CB,\"Collapsed\"$$\n";
  U8	scroll_x [512]
	fmtstr "$$DA-P,511,A=\"Scroll X Length Expression:%s\"$$\n",
	shift_x  [512]
	fmtstr "$$DA-P,511,A=\"X Offset Expression       :%s\"$$\n",
	shift_y  [512]
	fmtstr "$$DA-P,511,A=\"Y Offset Expression       :%s\"$$\n",
	define_str[512]
	fmtstr "$$DA-P,511,A=\"Define Str                :%s\"$$\n",
	html_link[512]
	fmtstr "$$DA-P,511,A=\"Html Link                 :%s\"$$\n";
};

U0 EdInsDollarText()
{
  U8 *st,buf[512];
  CEdDollarText *e=CAlloc(sizeof(CEdDollarText));
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->left_x  )  CatPrint(buf,"+LX");
    if (e->center_x)  CatPrint(buf,"+CX");
    if (e->right_x )  CatPrint(buf,"+RX");
    if (e->margin_rel) CatPrint(buf,"+MRX");
    if (e->blink)     CatPrint(buf,"+BK");
    if (e->invert)    CatPrint(buf,"+IV");
    if (e->underline) CatPrint(buf,"+UL");
    if (e->tree)      CatPrint(buf,"+TR");
    if (e->collapsed) CatPrint(buf,"+C");
    st=MStrPrint("%q",e->tag);
    "$$TX%s,\"%Q\"",buf,st;
    Free(st);
    if (*e->shift_x)
      ",SX=%s",e->shift_x;
    if (*e->shift_y)
      ",SY=%s",e->shift_y;
    if (*e->scroll_x)
      ",SCX=%s",e->scroll_x;
    if (*e->define_str) {
      st=MStrPrint("%q",e->define_str);
      ",D=\"%Q\"",st;
      Free(st);
    }
    if (*e->html_link) {
      st=MStrPrint("%q",e->html_link);
      ",HTML=\"%Q\"",st;
      Free(st);
    }
    "$$";
  }
  Free(e);
}

class CEdSong
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n",
	song[512]	fmtstr "$$DA-P,511,A=\"Song          :%s\"$$\n";
};

U0 EdInsSong()
{
  CEdSong *e=CAlloc(sizeof(CEdSong));
  if (DocForm(e,,,,ctrl_L_footer))
    "$$SO,\"%Q\",A=\"%s\"$$",e->tag,e->song;
  Free(e);
}

class CEdHighlight
{
  Bool	highlight_on	fmtstr "$$CB,\"Highlight On\"$$\n";
};

U0 EdInsHighlight()
{
  CEdHighlight *e=CAlloc(sizeof(CEdHighlight));
  if (DocForm(e,,,,ctrl_L_footer))
    "$$HL,%d$$",e->highlight_on;
  Free(e);
}

class CEdHtmlCode
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Html Code:%s\"$$\n";
};

U0 EdInsHtml()
{
  CEdHtmlCode *e=CAlloc(sizeof(CEdHtmlCode));
  if (DocForm(e,,,,ctrl_L_footer))
    "$$HC,\"%Q\"$$",e->tag;
  Free(e);
}

class CEdMacroMenu
{
  U8	tag[512]
	fmtstr "$$DA-P,511,A=\"Tag Text                  :%s\"$$\n",
	left_macro[512]
	fmtstr "$$DA-P,511,A=\"Left Click Macro          :%s\"$$\n",
	left_exp[512]
	fmtstr "$$DA-P,511,A=\"Left Click Expression     :%s\"$$\n";
  Bool	popup		fmtstr "$$CB,\"PopUp\"$$\n",
	left_is_auto	fmtstr "$$CB,\"Left is AutoStr\"$$\n",
	left_x		fmtstr "$$CB,\"Left X \"$$\n",
	center_x	fmtstr "$$CB,\"Center X \"$$\n",
	right_x		fmtstr "$$CB,\"Right X \"$$\n",
	margin_rel	fmtstr "$$CB,\"Margin Rel X\"$$\n",
	blink		fmtstr "$$CB,\"Blink\"$$\n",
	invert		fmtstr "$$CB,\"Invert\"$$\n",
	underline	fmtstr "$$CB,\"Underline\"$$\n",
	escape		fmtstr "$$CB,\"Escape\"$$\n";
  U8	scroll_x[512]
	fmtstr "$$DA-P,511,A=\"Scroll X Length Expression:%s\"$$\n",
	shift_x[512]
	fmtstr "$$DA-P,511,A=\"X Offset Expression       :%s\"$$\n",
	shift_y[512]
	fmtstr "$$DA-P,511,A=\"Y Offset Expression       :%s\"$$\n";
};

U0 EdInsMacroMenu(Bool is_macro)
{
  U8 *st,buf[512];
  CEdMacroMenu *e=CAlloc(sizeof(CEdMacroMenu));
  e->underline=TRUE;
  e->escape=TRUE;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->popup)	CatPrint(buf,"+PU-X");
    if (e->left_x  )	CatPrint(buf,"+LX");
    if (e->center_x)	CatPrint(buf,"+CX");
    if (e->right_x )	CatPrint(buf,"+RX");
    if (e->margin_rel) CatPrint(buf,"+MRX");
    if (e->blink)	CatPrint(buf,"+BK");
    if (e->invert)	CatPrint(buf,"+IV");
    if (!e->underline) CatPrint(buf,"-UL");
    if (!e->escape)	CatPrint(buf,"-X");
    if (e->left_is_auto) CatPrint(buf,"+LA");
    if (*e->tag || is_macro) {
      if (is_macro) {
	if (*e->tag) {
	  st=MStrPrint("%q",e->tag);
	  "$$MA%s,\"%Q\"",buf,st;
	  Free(st);
	} else
	  "$$MA%s",buf;
      } else {
	st=MStrPrint("%q",e->tag);
	"$$MU%s,\"%Q\"",buf,st;
	Free(st);
      }
      if (*e->left_exp)
	",LE=%s",e->left_exp;
      if (*e->left_macro) {
	st=MStrPrint("%q",e->left_macro);
	",LM=\"%Q\"",st;
	Free(st);
      }
      if (*e->shift_x)
	",SX=%s",e->shift_x;
      if (*e->shift_y)
	",SY=%s",e->shift_y;
      if (*e->scroll_x)
	",SCX=%s",e->scroll_x;
      "$$";
    }
  }
  Free(e);
}

class CEdButton
{
  U8	tag[512]   fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  Bool	popup	   fmtstr "$$CB,\"PopUp\"$$\n",
	left_x	   fmtstr "$$CB,\"Left X \"$$\n",
	center_x   fmtstr "$$CB,\"Center X \"$$\n",
	right_x	   fmtstr "$$CB,\"Right X \"$$\n",
	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n",
	escape	   fmtstr "$$CB,\"Escape\"$$\n";
  U8	left_macro[512]	fmtstr "$$DA-P,511,A=\"Left Click Macro:%s\"$$\n";
  Bool	left_is_auto	fmtstr "$$CB,\"Left is AutoStr\"$$\n";
  U8	left_exp[512]	fmtstr "$$DA-P,511,A=\"Left Click Expression:%s\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
};

U0 EdInsButton()
{
  U8 *st,buf[512];
  CEdButton *e=CAlloc(sizeof(CEdButton));
  e->escape=TRUE;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->popup)    CatPrint(buf,"+PU-X");
    if (e->left_x  ) CatPrint(buf,"+LX");
    if (e->center_x) CatPrint(buf,"+CX");
    if (e->right_x ) CatPrint(buf,"+RX");
    if (e->margin_rel) CatPrint(buf,"+MRX");
    if (!e->escape)	CatPrint(buf,"-X");
    if (e->left_is_auto) CatPrint(buf,"+LA");
    if (*e->tag) {
      if (e->quote) {
	st=MStrPrint("%q",e->tag);
	"$$$$BT%s,\\\"%Q\\\"",buf,st;
	Free(st);
	if (*e->left_exp)
	  ",LE=%s",e->left_exp;
	if (*e->left_macro) {
	  st=MStrPrint("%q",e->left_macro);
	  ",LM=\\\"%Q\\\"",st;
	  Free(st);
	}
	"$$$$";
      } else {
	st=MStrPrint("%q",e->tag);
	"$$BT%s,\"%Q\"",buf,st;
	Free(st);
	if (*e->left_exp)
	  ",LE=%s",e->left_exp;
	if (*e->left_macro) {
	  st=MStrPrint("%q",e->left_macro);
	  ",LM=\"%Q\"",st;
	  Free(st);
	}
	"$$";
      }
    }
  }
  Free(e);
}

class CEdCursorMovement
{
  U8	left_exp[512]	 fmtstr "$$DA-P,511,A=\"X Expression (LE):%s\"$$\n",
	right_exp[512]	 fmtstr "$$DA-P,511,A=\"Y Expression (RE):%s\"$$\n";
  Bool	left_x	 fmtstr "$$CB,\"Left   X \"$$\n",
	center_x fmtstr "$$CB,\"Center X \"$$\n",
	right_x  fmtstr "$$CB,\"Right  X \"$$\n",
	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n",
	top_y	 fmtstr "$$CB,\"Top    Y \"$$\n",
	center_y fmtstr "$$CB,\"Center Y \"$$\n",
	bottom_y fmtstr "$$CB,\"Bottom Y \"$$\n",
	page_rel fmtstr "$$CB,\"Page Rel Y\"$$\n",
	quote	 fmtstr "$$CB,\"Quote\"$$\n";
};

U0 EdInsCursorMovement()
{
  U8 buf[512];
  CEdCursorMovement *e=CAlloc(sizeof(CEdCursorMovement));
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->left_x  ) CatPrint(buf,"+LX");
    if (e->center_x) CatPrint(buf,"+CX");
    if (e->right_x ) CatPrint(buf,"+RX");
    if (e->margin_rel) CatPrint(buf,"+MRX");
    if (e->top_y   ) CatPrint(buf,"+TY");
    if (e->center_y) CatPrint(buf,"+CY");
    if (e->bottom_y) CatPrint(buf,"+BY");
    if (e->page_rel) CatPrint(buf,"+PRY");
    if (!*e->left_exp)	CatPrint(buf,"-LE");
    if (!*e->right_exp) CatPrint(buf,"-RE");
    if (e->quote)
      "$$";
    "$$CM%s",buf;
    if (*e->left_exp)
      ",LE=%s",e->left_exp;
    if (*e->right_exp)
      ",RE=%s",e->right_exp;
    "$$";
    if (e->quote)
      "$$";
  }
  Free(e);
}

class CEdData
{
  U8	fmt_str[512]	fmtstr "$$DA-P,511,A=\"Format Str:%s\"$$\n";
  I64	len	fmtstr "$$DA,512,A=\"Length:%d\"$$\n";
  Bool	term	fmtstr "$$CB,\"Terminator\"$$\n",
	str	fmtstr "$$CB,\"Str\"$$\n",
	refresh	fmtstr "$$CB,\"Refresh Data\"$$\n",
	update	fmtstr "$$CB,\"Update Data\"$$\n";
  I64	type	fmtstr "$$LS,D=\"ST_RAW_TYPES\"$$\n";
};

U0 EdInsData()
{
  U8 *st,buf[512],raw_type[16];
  CEdData *e=CAlloc(sizeof(CEdData));
  e->term=TRUE;
  e->len=512;
  e->type=RT_I64;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->type==RT_I64)
      *raw_type=0;
    else
      StrPrint(raw_type,",RT=%Z",e->type,"ST_RAW_TYPES");
    if (e->str)	CatPrint(buf,"-P");
    if (!e->term)	CatPrint(buf,"-TRM");
    if (e->refresh)	CatPrint(buf,"+RD");
    if (e->update)	CatPrint(buf,"+UD");

    st=MStrPrint("%q",e->fmt_str);
    "$$$$DA%s,%d%s,A=\\\"%Q\\\"$$$$",buf,e->len,raw_type,st;
    Free(st);
  }
  Free(e);
}

class CEdCheckBox
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  Bool	refresh		fmtstr "$$CB,\"Refresh Data\"$$\n";
  I64	type		fmtstr "$$LS,D=\"ST_RAW_TYPES\"$$\n";
};

U0 EdInsCheckBox()
{
  U8 *st,buf[512],raw_type[16];
  CEdCheckBox *e=CAlloc(sizeof(CEdCheckBox));
  e->type=RT_I8;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->type==RT_I8)
      *raw_type=0;
    else
      StrPrint(raw_type,",RT=%Z",e->type,"ST_RAW_TYPES");
    if (e->refresh)  CatPrint(buf,"+RD");
    st=MStrPrint("%q",e->tag);
    "$$$$CB%s%s,\\\"%Q\\\"$$$$",buf,raw_type,st;
    Free(st);
  }
  Free(e);
}

class CEdLst
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Dft Select    :%s\"$$\n",
	define_str[512]	fmtstr "$$DA-P,511,A=\"Define Str    :%s\"$$\n";
  Bool	refresh		fmtstr "$$CB,\"Refresh Data\"$$\n";
  I64	type		fmtstr "$$LS,D=\"ST_RAW_TYPES\"$$\n";
};

U0 EdInsLst()
{
  U8 *st1,*st2,buf[512],raw_type[16];
  CEdLst *e=CAlloc(sizeof(CEdLst));
  e->type=RT_I64;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (e->type==RT_I64)
      *raw_type=0;
    else
      StrPrint(raw_type,",RT=%Z",e->type,"ST_RAW_TYPES");
    if (e->refresh)  CatPrint(buf,"+RD");
    if (!*e->tag)
      st1=NULL;
    else
      st1=MStrPrint("%q",e->tag);
    st2=MStrPrint("%q",e->define_str);
    if (st1)
      "$$$$LS%s%s,\\\"%Q\\\",D=\\\"%Q\\\"$$$$",buf,raw_type,st1,st2;
    else
      "$$$$LS%s%s,D=\\\"%Q\\\"$$$$",buf,raw_type,st2;
    Free(st1);
    Free(st2);
  }
  Free(e);
}

class CEdHexEd
{
  I64	cnt	fmtstr "$$DA,512,A=\"Count:%d\"$$\n",
	cols	fmtstr "$$DA,512,A=\"Columns:%d\"$$\n";
  Bool	zero	fmtstr "$$CB,\"Zero Based\"$$\n",
	refresh	fmtstr "$$CB,\"Refresh Data\"$$\n";
};

U0 EdInsHexEd()
{
  U8 buf[512];
  CEdHexEd *e=CAlloc(sizeof(CEdHexEd));
  e->cnt=128;
  e->cols=4;
  e->zero=TRUE;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (!e->zero)  CatPrint(buf,"-Z");
    if (e->refresh)  CatPrint(buf,"+RD");
    "$$$$HX%s,%d,%d$$$$",buf,e->cnt,e->cols;
  }
  Free(e);
}

class CEdBinary
{
  U8	tag[512]		fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  I64	bin_num			fmtstr "$$DA,512,A=\"Bin Num:%d\"$$\n";
  U8	bin_ptr_link_file[512]	fmtstr "$$DA-P,511,A=\"File:%s\"$$\n";
  I64	bin_ptr_link_bin_num	fmtstr "$$DA,512,A=\"File Bin Num:%d\"$$\n";
  U8	bin_ptr_link_tag[512]	fmtstr "$$DA-P,511,A=\"File Bin Tag:%s\"$$\n";
};

U0 EdInsBinary(I64 bin_num,I64 type)
{
  CEdBinary *e=CAlloc(sizeof(CEdBinary));
  StrPrint(e->tag,"<%d>",bin_num);
  e->bin_num=bin_num;
  e->bin_ptr_link_bin_num=1;
  if (DocForm(e,,,
	"Note: Normally, you enter no file,\n"
	"just a bin num.  If You enter a\n"
	"file, enter a file bin num or a file\n"
	"bin tag.\n\n",ctrl_L_footer)) {
    if (*e->bin_ptr_link_file) {
      if (*e->bin_ptr_link_tag)
	"$$%Z,\"%Q\",BI=%d,BP=\"%s,%s\"$$",type,"ST_DOC_CMDS",e->tag,e->bin_num,
	      e->bin_ptr_link_file,e->bin_ptr_link_tag;
      else
	"$$%Z,\"%Q\",BI=%d,BP=\"%s,%d\"$$",type,"ST_DOC_CMDS",e->tag,e->bin_num,
	      e->bin_ptr_link_file,e->bin_ptr_link_bin_num;
    } else {
      if (DocBinFindNum(DocPut,e->bin_num))
	"$$%Z,\"%Q\",BI=%d$$",type,"ST_DOC_CMDS",e->tag,e->bin_num;
      else
	PopUpOk("Invalid Binary Num");
    }
  }
  Free(e);
}

class CEdTree
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  I64	indent		fmtstr "$$DA,511,A=\"Indention:%d\"$$\n";
  Bool	collapsed	fmtstr "$$CB,\"Collapsed\"$$\n";
};

U0 EdInsTree()
{
  U8 *st1,*st2,buf[512];
  CEdTree *e=CAlloc(sizeof(CEdTree));
  e->collapsed=TRUE;
  e->indent=2;
  if (DocForm(e,,,,ctrl_L_footer)) {
    *buf=0;
    if (!e->collapsed)	CatPrint(buf,"-C");
    st1=MStrPrint("%q",e->tag);
    st2=MStrPrint("$$TR%s,\"%Q\"$$\n$$ID,%d$$*\n$$ID,%d$$",
	  buf,st1,e->indent,-e->indent);
    DocPrintAtomic(DocPut,"%s",st2);
    Free(st1);
    Free(st2);
  }
  Free(e);
}

U0 EdInsDollarCmd()
{
  I64 type=PopUpDollarType;
  switch (type) {
    case DOCT_FOREGROUND:
      EdInsColor;
      break;
    case DOCT_PAGE_LEN:
      EdInsPageSetting;
      break;
    case DOCT_LINK:
      EdInsLink;
      break;
    case DOCT_TEXT:
      EdInsDollarText;
      break;
    case DOCT_TREE:
      EdInsTree;
      break;
    case DOCT_MACRO:
      EdInsMacroMenu(TRUE);
      break;
    case DOCT_MENU_VAL:
      EdInsMacroMenu(FALSE);
      break;
    case DOCT_CURSOR_MOVEMENT:
      EdInsCursorMovement;
      break;
    case DOCT_BUTTON:
      EdInsButton;
      break;
    case DOCT_DATA:
      EdInsData;
      break;
    case DOCT_CHECK_BOX:
      EdInsCheckBox;
      break;
    case DOCT_LST:
      EdInsLst;
      break;
    case DOCT_HEX_ED:
      EdInsHexEd;
      break;
    case DOCT_SONG:
      EdInsSong;
      break;
    case DOCT_HIGHLIGHT:
      EdInsHighlight;
      break;
    case DOCT_HTML_CODE:
      EdInsHtml;
  }
}

#define EST_SPRITE		0
#define EST_SPRITE_PTR		1
#define EST_DUP_SPRITE		2
#define EST_SPRITE_SIZE		3
#define EST_SPRITE_MACRO		4
#define EST_SPRITE_MENU		5
#define EST_SPRITE_LINK		6

I64 PopUpSpriteType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Graphic Resource Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/Resource.TXT.Z\"$$\n"
	"\n$$LTBLUE$$$$MU,\"Make Sprite\",LE=EST_SPRITE$$\n"
	"$$MU,\"Pointer to Sprite\",LE=EST_SPRITE_PTR$$\n"
	"$$MU,\"Duplicate Sprite\",LE=EST_DUP_SPRITE$$\n"
	"$$MU,\"Sprite Size (not really needed)\",LE=EST_SPRITE_SIZE$$\n"
	"$$MU,\"Sprite Macro (Icon)\",LE=EST_SPRITE_MACRO$$\n"
	"$$MU,\"Sprite Menu Item\",LE=EST_SPRITE_MENU$$\n"
	"$$MU,\"Sprite Link\",LE=EST_SPRITE_LINK$$\n"
	"\n$$MU,\"Abort\",LE=DOCM_CANCEL$$\n\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdJustSprite
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text       :%s\"$$\n";
};

class CEdMacroSprite
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text       :%s\"$$\n",
	exp[512]	fmtstr "$$DA-P,511,A=\"Macro          :%s\"$$\n";
  Bool	escape		fmtstr "$$CB,\"Escape\"$$\n",
	popup		fmtstr "$$CB,\"PopUp\"$$\n",
	is_auto		fmtstr "$$CB,\"AutoStr\"$$\n";
};

class CEdMenuSprite
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text       :%s\"$$\n",
	exp[512]	fmtstr "$$DA-P,511,A=\"Expression     :%s\"$$\n";
  Bool	escape		fmtstr "$$CB,\"Escape\"$$\n";
};

U8 *EdSpriteLink(Bool *_pop_up,U8 **_html_link)
{
  U8 *result=NULL,*tag=NULL,*link=NULL;
  Bool quote=FALSE;
  I64 type=PopUpLinkType(FALSE);
  *_html_link=NULL;
  if (type>=0 && GetLink(type,&tag,&link,_pop_up,&quote,_html_link))
    result=MStrPrint("\"%Q\",A=\"%Q\"",tag,link);
  Free(tag);
  Free(link);
  return result;
}

U8 *EdSprite(I64 bin_num)
{
  I64 type=PopUpSpriteType;
  Bool pop_up;
  U8 *st,*st1=NULL,*st2=NULL,buf[1024],*html_link=NULL;
  CEdMacroSprite *e=CAlloc(sizeof(CEdMacroSprite));
  *buf=0;
  switch (type) {
    case EST_SPRITE:
      StrPrint(e->tag,"<%d>",bin_num);
      if (DocForm(e,"CEdJustSprite",,,ctrl_L_footer))
	st1=MStrPrint("\"%Q\"",e->tag);
      break;
    case EST_SPRITE_MACRO:
      e->escape=TRUE;
      if (DocForm(e,,,,ctrl_L_footer)) {
	CatPrint(buf,"+UL");
	if (e->is_auto)CatPrint(buf,"+LA");
	if (e->popup)	CatPrint(buf,"+PU");
	if (e->escape)	CatPrint(buf,"+X");
	st=MStrPrint("%q",e->exp);
	st1=MStrPrint("\"%Q\",LM=\"%Q\"",e->tag,st);
	Free(st);
      }
      break;
    case EST_SPRITE_MENU:
      e->escape=TRUE;
      if (DocForm(e,"CEdMenuSprite",,,ctrl_L_footer)) {
	if (e->escape)	CatPrint(buf,"+X");
	st1=MStrPrint("\"%Q\",LE=%s",e->tag,e->exp);
      }
      break;
    case EST_SPRITE_LINK:
      CatPrint(buf,"+L");
      st1=EdSpriteLink(&pop_up,&html_link);
      if (pop_up)
	CatPrint(buf,"+PU");
      break;
    case EST_SPRITE_PTR:
      EdInsBinary(--bin_num,DOCT_INS_BINARY);
      break;
    case EST_SPRITE_SIZE:
      EdInsBinary(--bin_num,DOCT_INS_BINARY_SIZE);
      break;
    case EST_DUP_SPRITE:
      EdInsBinary(--bin_num,DOCT_SPRITE);
      break;
  }
  if (st1) {
    if (html_link)
      st2=MStrPrint("$$SP%s,%s,HTML=\"%Q\",BI=%d$$",buf,st1,html_link,bin_num);
    else
      st2=MStrPrint("$$SP%s,%s,BI=%d$$",buf,st1,bin_num);
    Free(st1);
  }
  Free(e);
  Free(html_link);
  return st2;
}
