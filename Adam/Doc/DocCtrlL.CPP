#help_index "Doc/Misc"

U8 *ctrl_L_footer=
	"\n$$MU-X+Q,\"Abort\",LE=DOCM_CANCEL$$\n"
	"\n\n$$LK+PU,\"Click for Help\",\"FI:::/Doc/Widget.TXT\"$$\n";

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
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n";
  U8	aux[512];
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdFileAnchorLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n";
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Anchor Label  :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdFileFindLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n";
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Str        :%s\"$$\n";
  I64	num		fmtstr "$$DA-TRM,511,A=\"Occurrence Num:%04d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdFileLineLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n";
  U8	aux[512];
  I64	num		fmtstr "$$DA-TRM,511,A=\"Line Num      :%04d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdManPageLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512];
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Label         :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdAddrLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512];
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Addr Exp   :%s\"$$\n";
  I64	num		fmtstr "$$DA-TRM,511,A=\"Bin Size      :%04d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdPlaceAnchor
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512];
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Anchor Label  :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up;
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide		fmtstr "$$CB,\"Hide\"$$\n";
};

class CEdBibleLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512];
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Chapter Verse :%s\"$$\n";
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdDefLink
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512];
  U8	aux[512]	fmtstr "$$DA-P,511,A=\"Word          :%s\"$$\n";
  I64	num		fmtstr "$$DA-TRM,511,A=\"Def Num       :%4d\"$$\n";
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdProject
{
  U8	tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8	file[512]	fmtstr "$$DA-P,511,A=\"File          :%s\"$$\n";
  U8	aux[512];
  I64	num;
  U8	html_link[512]	fmtstr "$$DA-P,511,A=\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

Bool PmtLink(I64 type,U8 **_tag,U8 **_link,Bool *_pop_up,Bool *_quote,U8 **_html_link)
{
  I64 i;
  CEdFileLink *el=CAlloc(sizeof(CEdFileLink));
  Bool result=FALSE;
  if (type>=0) {
    el->num=1;
    switch (type) {
      case LK_FILE:
	if (DocFormDo(el,,,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->file);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("FI:%s",el->file);
	}
	break;
      case LK_PLAIN:
	if (DocFormDo(el,,,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->file);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("PI:%s",el->file);
	}
	break;
      case LK_FILE_ANCHOR:
	if (DocFormDo(el,"CEdFileAnchorLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->aux);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("FA:%s,%s",el->file,el->aux);
	}
	break;
      case LK_FILE_FIND:
	if (DocFormDo(el,"CEdFileFindLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (el->num==1) {
	    if (!*el->tag)
	      *_tag=StrNew(el->aux);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("FF:%s,%s",el->file,el->aux);
	  } else {
	    if (!*el->tag)
	      *_tag=StrNew(el->aux);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("FF:%s,%s:%d",el->file,el->aux,el->num);
	  }
	}
	break;
      case LK_PLAIN_FIND:
	if (DocFormDo(el,"CEdFileFindLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (el->num==1) {
	    if (!*el->tag)
	      *_tag=StrNew(el->aux);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("PF:%s,%s",el->file,el->aux);
	  } else {
	    if (!*el->tag)
	      *_tag=StrNew(el->aux);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("PF:%s,%s:%d",el->file,el->aux,el->num);
	  }
	}
	break;
      case LK_FILE_LINE:
	if (DocFormDo(el,"CEdFileLineLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->file);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("FL:%s,%d",el->file,el->num);
	}
	break;
      case LK_PLAIN_LINE:
	if (DocFormDo(el,"CEdFileLineLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->file);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("PL:%s,%d",el->file,el->num);
	}
	break;
      case LK_MAN_PAGE:
	if (DocFormDo(el,"CEdManPageLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->aux);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("MN:%s",el->aux);
	}
	break;
      case LK_PLACE_ANCHOR:
	if (DocFormDo(el,"CEdPlaceAnchor",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag) {
	    if (el->hide)
	      *_tag=StrNew("");
	    else
	      *_tag=StrNew(el->aux);
	  } else
	    *_tag=StrNew(el->tag);
	  *_link=StrNew(el->aux);
	}
	break;
      case LK_BIBLE_FIND: //TODO
	i=PopUpPickDefineSub("ST_BIBLE_BOOKS");
	if (i>=0) {
	  if (DocFormDo(el,"CEdBibleLink",,,ctrl_L_footer)) {
	    result=TRUE;
	    if (!*el->tag)
	      *_tag=MPrint("%Z,%s",i,"ST_BIBLE_BOOKS",el->aux);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("BF:%Z,%s",i,"ST_BIBLE_BOOKS",el->aux);
	  }
	}
	break;
      case LK_DEF:
	el->num=-1;
	if (DocFormDo(el,"CEdDefLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->aux);
	  else
	    *_tag=StrNew(el->tag);
	  if (el->num<0)
	    *_link=MPrint("DF:%s",el->aux);
	  else
	    *_link=MPrint("DF:%s,%d",el->aux,el->num);

	}
	break;
      case LK_HELP_INDEX:
	if (DocFormDo(el,"CEdManPageLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (!*el->tag)
	    *_tag=StrNew(el->aux);
	  else
	    *_tag=StrNew(el->tag);
	  *_link=MPrint("HI:%s",el->aux);
	}
	break;
      case LK_ADDR:
	el->num=DFT_ADDR_LINK_BIN_SIZE;
	if (DocFormDo(el,"CEdAddrLink",,,ctrl_L_footer)) {
	  result=TRUE;
	  if (el->num==DFT_ADDR_LINK_BIN_SIZE) {
	    if (!*el->tag)
	      *_tag=StrNew(el->aux);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("AD:%s",el->aux);
	  } else {
	    if (!*el->tag)
	      *_tag=MPrint("%s,%d",el->aux,el->num);
	    else
	      *_tag=StrNew(el->tag);
	    *_link=MPrint("AD:%s,%d",el->aux,el->num);
	  }
	}
	break;
    }
  }
  if (*el->html_link)
    *_html_link=StrNew(el->html_link);
  else
    *_html_link=NULL;
  if (el->pop_up)
    *_pop_up=TRUE;
  else
    *_pop_up=FALSE;
  if (el->quote)
    *_quote=TRUE;
  else
    *_quote=FALSE;
  Free(el);
  return result;
}

U0 EdInsLink()
{
  U8 *tag=NULL,*link=NULL,*st=NULL,*html_link=NULL,*pop_up_st;
  Bool pop_up=FALSE,quote=FALSE;
  I64 type=PopUpLinkType(TRUE);
  if (type>=0) {
    if (PmtLink(type,&tag,&link,&pop_up,&quote,&html_link)) {
      if (pop_up)
	pop_up_st="+PU";
      else
	pop_up_st="";
      switch (type) {
	case LK_PLACE_ANCHOR:
	  if (html_link)
	    st=MPrint("$$AN,\"%Q\",A=\"%Q\",HTML=\"%Q\"$$",tag,link,html_link);
	  else
	    st=MPrint("$$AN,\"%Q\",A=\"%Q\"$$",tag,link);
	  break;
	default:
	  if (html_link)
	    st=MPrint("$$LK%s,\"%Q\",A=\"%Q\",HTML=\"%Q\"$$",pop_up_st,tag,link,html_link);
	  else
	    st=MPrint("$$LK%s,\"%Q\",A=\"%Q\"$$",pop_up_st,tag,link);
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
	"$$MU,\"Lock To Page\",LE=DOCT_LOCK_TO_PAGE$$\n"
	"$$MU,\"Lock To Center\",LE=DOCT_LOCK_TO_CENTER$$\n"
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
};

class CEdPageSetting2
{
  U8	val[512]	fmtstr "$$DA-P,511,A=\"Setting Val:%s\"$$\n";
  Bool winrel		fmtstr "$$CB,\"Win Relative\"$$\n";
};

class CEdPageSetting3
{
  Bool lock		fmtstr "$$CB,\"Lock To Page\"$$\n";
};

class CEdPageSetting4
{
  Bool lock		fmtstr "$$CB,\"Lock To Center\"$$\n";
};


U0 EdInsPageSetting()
{
  I64 type=PopUpPageSettingType;
  CEdPageSetting1 *el=CAlloc(sizeof(CEdPageSetting1));
  CEdPageSetting2 *el2=CAlloc(sizeof(CEdPageSetting2));
  CEdPageSetting3 *el3=CAlloc(sizeof(CEdPageSetting3));
  CEdPageSetting4 *el4=CAlloc(sizeof(CEdPageSetting4));
  if (type>=0) {
    if (type==DOCT_RIGHT_MARGIN||type==DOCT_PAGE_LEN) {
      if (DocFormDo(el2)) {
	if (el2->winrel) {
	  if (*el2->val)
	    "$$%Z+WR,%s$$",type,"ST_DOC_CMDS",el2->val;
	  else
	    "$$%Z+WR,0$$",type,"ST_DOC_CMDS";
	} else {
	  if (*el2->val)
	    "$$%Z,%s$$",type,"ST_DOC_CMDS",el2->val;
	  else
	    "$$%Z$$",type,"ST_DOC_CMDS";
	}
      }
    } else if (type==DOCT_LOCK_TO_PAGE) {
      el3->lock=TRUE;
      if (DocFormDo(el3)) {
	if (el3->lock)
	  "$$%Z,1$$",type,"ST_DOC_CMDS",el->val;
	else
	  "$$%Z,0$$",type,"ST_DOC_CMDS";
      }
    } else if (type==DOCT_LOCK_TO_CENTER) {
      el4->lock=TRUE;
      if (DocFormDo(el4)) {
	if (el4->lock)
	  "$$%Z,1$$",type,"ST_DOC_CMDS",el->val;
	else
	  "$$%Z,0$$",type,"ST_DOC_CMDS";
      }
    } else {
      if (DocFormDo(el)) {
	if (*el->val)
	  "$$%Z,%s$$",type,"ST_DOC_CMDS",el->val;
	else
	  "$$%Z$$",type,"ST_DOC_CMDS";
      }
    }
  }
  Free(el4);
  Free(el3);
  Free(el2);
  Free(el);
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
  Bool	left_x	  fmtstr "$$CB,\"Left X\"$$\n";
  Bool	center_x  fmtstr "$$CB,\"Center X\"$$\n";
  Bool	right_x   fmtstr "$$CB,\"Right X\"$$\n";
  Bool	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool	blink	  fmtstr "$$CB,\"Blink\"$$\n";
  Bool	invert	  fmtstr "$$CB,\"Invert\"$$\n";
  Bool	underline fmtstr "$$CB,\"Underline\"$$\n";
  Bool	tree	  fmtstr "$$CB,\"Tree\"$$\n";
  Bool	collapsed fmtstr "$$CB,\"Collapsed\"$$\n";
  U8	scroll_x [512]	  fmtstr "$$DA-P,511,A=\"Scroll X Length Expression:%s\"$$\n";
  U8	shift_x  [512]	  fmtstr "$$DA-P,511,A=\"X Offset Expression       :%s\"$$\n";
  U8	shift_y  [512]	  fmtstr "$$DA-P,511,A=\"Y Offset Expression       :%s\"$$\n";
  U8	define_str[512]	  fmtstr "$$DA-P,511,A=\"Define Str                :%s\"$$\n";
  U8	html_link[512]	  fmtstr "$$DA-P,511,A=\"Html Link                 :%s\"$$\n";
};

U0 EdInsDollarText()
{
  U8 *st,buf[512];
  CEdDollarText *dt=CAlloc(sizeof(CEdDollarText));
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->left_x  )  CatPrint(buf,"+LX");
    if (dt->center_x)  CatPrint(buf,"+CX");
    if (dt->right_x )  CatPrint(buf,"+RX");
    if (dt->margin_rel) CatPrint(buf,"+MRX");
    if (dt->blink)     CatPrint(buf,"+BK");
    if (dt->invert)    CatPrint(buf,"+IV");
    if (dt->underline) CatPrint(buf,"+UL");
    if (dt->tree)      CatPrint(buf,"+TR");
    if (dt->collapsed) CatPrint(buf,"+C");
    st=MPrint("%q",dt->tag);
    "$$TX%s,\"%Q\"",buf,st;
    Free(st);
    if (*dt->shift_x)
      ",SX=%s",dt->shift_x;
    if (*dt->shift_y)
      ",SY=%s",dt->shift_y;
    if (*dt->scroll_x)
      ",SCX=%s",dt->scroll_x;
    if (*dt->define_str) {
      st=MPrint("%q",dt->define_str);
      ",D=\"%Q\"",st;
      Free(st);
    }
    if (*dt->html_link) {
      st=MPrint("%q",dt->html_link);
      ",HTML=\"%Q\"",st;
      Free(st);
    }
    "$$";
  }
  Free(dt);
}

class CEdSong
{
  U8 tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  U8 song[512]	fmtstr "$$DA-P,511,A=\"Song          :%s\"$$\n";
};

U0 EdInsSong()
{
  CEdSong *dt=CAlloc(sizeof(CEdSong));
  if (DocFormDo(dt,,,,ctrl_L_footer))
    "$$SO,\"%Q\",A=\"%s\"$$",dt->tag,dt->song;
  Free(dt);
}

class CEdHighlight
{
  Bool highlight_on	fmtstr "$$CB,\"Highlight On\"$$\n";
};

U0 EdInsHighlight()
{
  CEdHighlight *dt=CAlloc(sizeof(CEdHighlight));
  if (DocFormDo(dt,,,,ctrl_L_footer))
    "$$HL,%d$$",dt->highlight_on;
  Free(dt);
}

class CEdHtmlCode
{
  U8 tag[512]	fmtstr "$$DA-P,511,A=\"Html Code:%s\"$$\n";
};

U0 EdInsHtml()
{
  CEdHtmlCode *dt=CAlloc(sizeof(CEdHtmlCode));
  if (DocFormDo(dt,,,,ctrl_L_footer))
    "$$HC,\"%Q\"$$",dt->tag;
  Free(dt);
}

class CEdMacroMenu
{
  U8 tag[512]		fmtstr "$$DA-P,511,A=\"Tag Text                  :%s\"$$\n";
  U8 left_macro[512]	fmtstr "$$DA-P,511,A=\"Left Click Macro          :%s\"$$\n";
  U8 left_exp[512]	fmtstr "$$DA-P,511,A=\"Left Click Expression     :%s\"$$\n";
  Bool popup		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool left_is_auto	fmtstr "$$CB,\"Left is AutoStr\"$$\n";
  Bool left_x		fmtstr "$$CB,\"Left X \"$$\n";
  Bool center_x		fmtstr "$$CB,\"Center X \"$$\n";
  Bool right_x		fmtstr "$$CB,\"Right X \"$$\n";
  Bool margin_rel	fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool blink		fmtstr "$$CB,\"Blink\"$$\n";
  Bool invert		fmtstr "$$CB,\"Invert\"$$\n";
  Bool underline	fmtstr "$$CB,\"Underline\"$$\n";
  Bool escape		fmtstr "$$CB,\"Escape\"$$\n";
  U8 scroll_x[512]	fmtstr "$$DA-P,511,A=\"Scroll X Length Expression:%s\"$$\n";
  U8 shift_x[512]	fmtstr "$$DA-P,511,A=\"X Offset Expression       :%s\"$$\n";
  U8 shift_y[512]	fmtstr "$$DA-P,511,A=\"Y Offset Expression       :%s\"$$\n";
};

U0 EdInsMacroMenu(Bool is_macro)
{
  U8 *st,buf[512];
  CEdMacroMenu *dt=CAlloc(sizeof(CEdMacroMenu));
  dt->underline=TRUE;
  dt->escape=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->popup)	CatPrint(buf,"+PU-X");
    if (dt->left_x  )	CatPrint(buf,"+LX");
    if (dt->center_x)	CatPrint(buf,"+CX");
    if (dt->right_x )	CatPrint(buf,"+RX");
    if (dt->margin_rel) CatPrint(buf,"+MRX");
    if (dt->blink)	CatPrint(buf,"+BK");
    if (dt->invert)	CatPrint(buf,"+IV");
    if (!dt->underline) CatPrint(buf,"-UL");
    if (!dt->escape)	CatPrint(buf,"-X");
    if (dt->left_is_auto) CatPrint(buf,"+LA");
    if (*dt->tag || is_macro) {
      if (is_macro) {
	if (*dt->tag) {
	  st=MPrint("%q",dt->tag);
	  "$$MA%s,\"%Q\"",buf,st;
	  Free(st);
	} else
	  "$$MA%s",buf;
      } else {
	st=MPrint("%q",dt->tag);
	"$$MU%s,\"%Q\"",buf,st;
	Free(st);
      }
      if (*dt->left_exp)
	",LE=%s",dt->left_exp;
      if (*dt->left_macro) {
	st=MPrint("%q",dt->left_macro);
	",LM=\"%Q\"",st;
	Free(st);
      }
      if (*dt->shift_x)
	",SX=%s",dt->shift_x;
      if (*dt->shift_y)
	",SY=%s",dt->shift_y;
      if (*dt->scroll_x)
	",SCX=%s",dt->scroll_x;
      "$$";
    }
  }
  Free(dt);
}

class CEdButton
{
  U8	tag[512]   fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  Bool	popup	   fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	left_x	   fmtstr "$$CB,\"Left X \"$$\n";
  Bool	center_x   fmtstr "$$CB,\"Center X \"$$\n";
  Bool	right_x	   fmtstr "$$CB,\"Right X \"$$\n";
  Bool	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool	escape	   fmtstr "$$CB,\"Escape\"$$\n";
  U8	left_macro[512]	fmtstr "$$DA-P,511,A=\"Left Click Macro:%s\"$$\n";
  Bool	left_is_auto	fmtstr "$$CB,\"Left is AutoStr\"$$\n";
  U8	left_exp[512]	fmtstr "$$DA-P,511,A=\"Left Click Expression:%s\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
};

U0 EdInsButton()
{
  U8 *st,buf[512];
  CEdButton *dt=CAlloc(sizeof(CEdButton));
  dt->escape=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->popup)    CatPrint(buf,"+PU-X");
    if (dt->left_x  ) CatPrint(buf,"+LX");
    if (dt->center_x) CatPrint(buf,"+CX");
    if (dt->right_x ) CatPrint(buf,"+RX");
    if (dt->margin_rel) CatPrint(buf,"+MRX");
    if (!dt->escape)	CatPrint(buf,"-X");
    if (dt->left_is_auto) CatPrint(buf,"+LA");
    if (*dt->tag) {
      if (dt->quote) {
	st=MPrint("%q",dt->tag);
	"$$$$BT%s,\\\"%Q\\\"",buf,st;
	Free(st);
	if (*dt->left_exp)
	  ",LE=%s",dt->left_exp;
	if (*dt->left_macro) {
	  st=MPrint("%q",dt->left_macro);
	  ",LM=\\\"%Q\\\"",st;
	  Free(st);
	}
	"$$$$";
      } else {
	st=MPrint("%q",dt->tag);
	"$$BT%s,\"%Q\"",buf,st;
	Free(st);
	if (*dt->left_exp)
	  ",LE=%s",dt->left_exp;
	if (*dt->left_macro) {
	  st=MPrint("%q",dt->left_macro);
	  ",LM=\"%Q\"",st;
	  Free(st);
	}
	"$$";
      }
    }
  }
  Free(dt);
}


class CEdCursorMovement
{
  U8 left_exp[512]	 fmtstr "$$DA-P,511,A=\"X Expression (LE):%s\"$$\n";
  U8 right_exp[512]	 fmtstr "$$DA-P,511,A=\"Y Expression (RE):%s\"$$\n";
  Bool	left_x	 fmtstr "$$CB,\"Left   X \"$$\n";
  Bool	center_x fmtstr "$$CB,\"Center X \"$$\n";
  Bool	right_x  fmtstr "$$CB,\"Right  X \"$$\n";
  Bool	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool	top_y	 fmtstr "$$CB,\"Top    Y \"$$\n";
  Bool	center_y fmtstr "$$CB,\"Center Y \"$$\n";
  Bool	bottom_y fmtstr "$$CB,\"Bottom Y \"$$\n";
  Bool	page_rel fmtstr "$$CB,\"Page Rel Y\"$$\n";
  Bool	quote	 fmtstr "$$CB,\"Quote\"$$\n";
};

U0 EdInsCursorMovement()
{
  U8 buf[512];
  CEdCursorMovement *dt=CAlloc(sizeof(CEdCursorMovement));
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->left_x  ) CatPrint(buf,"+LX");
    if (dt->center_x) CatPrint(buf,"+CX");
    if (dt->right_x ) CatPrint(buf,"+RX");
    if (dt->margin_rel) CatPrint(buf,"+MRX");
    if (dt->top_y   ) CatPrint(buf,"+TY");
    if (dt->center_y) CatPrint(buf,"+CY");
    if (dt->bottom_y) CatPrint(buf,"+BY");
    if (dt->page_rel) CatPrint(buf,"+PRY");
    if (!*dt->left_exp)  CatPrint(buf,"-LE");
    if (!*dt->right_exp) CatPrint(buf,"-RE");
    if (dt->quote)
      "$$";
    "$$CM%s",buf;
    if (*dt->left_exp)
      ",LE=%s",dt->left_exp;
    if (*dt->right_exp)
      ",RE=%s",dt->right_exp;
    "$$";
    if (dt->quote)
      "$$";
  }
  Free(dt);
}

class CEdData
{
  U8	fmt_str[512]	fmtstr "$$DA-P,511,A=\"Format Str:%s\"$$\n";
  I64	len	fmtstr "$$DA,512,A=\"Length:%d\"$$\n";
  Bool	term	fmtstr "$$CB,\"Terminator\"$$\n";
  Bool	str	fmtstr "$$CB,\"Str\"$$\n";
  Bool	refresh	fmtstr "$$CB,\"Refresh Data\"$$\n";
  Bool	update	fmtstr "$$CB,\"Update Data\"$$\n";
  Bool	u8	fmtstr "$$CB,\"U8\"$$\n";
  Bool	u16	fmtstr "$$CB,\"U16\"$$\n";
  Bool	u32	fmtstr "$$CB,\"U32\"$$\n";
  Bool	u64	fmtstr "$$CB,\"U64\"$$\n";
};

U0 EdInsData()
{
  U8 *st,buf[512];
  CEdData *dt=CAlloc(sizeof(CEdData));
  dt->term=TRUE;
  dt->len=512;
  dt->u64=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->str)   CatPrint(buf,"-P");
    if (!dt->term)    CatPrint(buf,"-TRM");
    if (dt->refresh)  CatPrint(buf,"+RD");
    if (dt->update)   CatPrint(buf,"+UD");

    if (dt->u8)        CatPrint(buf,"-U64+U8");
    else if (dt->u16)  CatPrint(buf,"-U64+U16");
    else if (dt->u32)  CatPrint(buf,"-U64+U32");

    st=MPrint("%q",dt->fmt_str);
    "$$$$DA%s,%d,A=\\\"%Q\\\"$$$$",buf,dt->len,st;
    Free(st);
  }
  Free(dt);
}

class CEdCheckBox
{
  U8	label[512]	fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  Bool	refresh		fmtstr "$$CB,\"Refresh Data\"$$\n";
};

U0 EdInsCheckBox()
{
  U8 *st,buf[512];
  CEdCheckBox *dt=CAlloc(sizeof(CEdCheckBox));
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->refresh)  CatPrint(buf,"+RD");
    st=MPrint("%q",dt->label);
    "$$$$CB%s,\\\"%Q\\\"$$$$",buf,st;
    Free(st);
  }
  Free(dt);
}

class CEdHexEd
{
  I64 cnt	fmtstr "$$DA,512,A=\"Count:%d\"$$\n";
  I64 cols	fmtstr "$$DA,512,A=\"Columns:%d\"$$\n";
  Bool zero	fmtstr "$$CB,\"Zero Based\"$$\n";
  Bool refresh	fmtstr "$$CB,\"Refresh Data\"$$\n";
};

U0 EdInsHexEd()
{
  U8 buf[512];
  CEdHexEd *dt=CAlloc(sizeof(CEdHexEd));
  dt->cnt=128;
  dt->cols=4;
  dt->zero=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (!dt->zero)  CatPrint(buf,"-Z");
    if (dt->refresh)  CatPrint(buf,"+RD");
    "$$$$HX%s,%d,%d$$$$",buf,dt->cnt,dt->cols;
  }
  Free(dt);
}


class CEdBinary
{
  U8  tag[512]			fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  I64 bin_num			fmtstr "$$DA,512,A=\"Bin Num:%d\"$$\n";
  U8  bin_ptr_link_file[512]	fmtstr "$$DA-P,511,A=\"File:%s\"$$\n";
  I64 bin_ptr_link_bin_num	fmtstr "$$DA,512,A=\"File Bin Num:%d\"$$\n";
  U8  bin_ptr_link_tag[512]	fmtstr "$$DA-P,511,A=\"File Bin Tag:%s\"$$\n";
};

U0 EdInsBinary(I64 bin_num,I64 type)
{
  CEdBinary *dt=CAlloc(sizeof(CEdBinary));
  SPrint(dt->tag,"<%d>",bin_num);
  dt->bin_num=bin_num;
  dt->bin_ptr_link_bin_num=1;
  if (DocFormDo(dt,,,
"Note: Normally, you enter no file,\n"
"just a bin num.  If You enter a\n"
"file, enter a file bin num or a file\n"
"bin tag.\n\n",ctrl_L_footer)) {
    if (*dt->bin_ptr_link_file) {
      if (*dt->bin_ptr_link_tag)
	"$$%Z,\"%Q\",BI=%d,BP=\"%s,%s\"$$",type,"ST_DOC_CMDS",dt->tag,dt->bin_num,
	  dt->bin_ptr_link_file,dt->bin_ptr_link_tag;
      else
	"$$%Z,\"%Q\",BI=%d,BP=\"%s,%d\"$$",type,"ST_DOC_CMDS",dt->tag,dt->bin_num,
	  dt->bin_ptr_link_file,dt->bin_ptr_link_bin_num;
    } else {
      if (DocBinFindNum(DocPut,dt->bin_num))
	"$$%Z,\"%Q\",BI=%d$$",type,"ST_DOC_CMDS",dt->tag,dt->bin_num;
      else
	PopUpOk("Invalid Binary Num");
    }
  }
  Free(dt);
}

class CEdTree
{
  U8	label[512] fmtstr "$$DA-P,511,A=\"Tag Text      :%s\"$$\n";
  I64	indent	   fmtstr "$$DA,511,A=\"Indention:%d\"$$\n";
  Bool	collapsed  fmtstr "$$CB,\"Collapsed\"$$\n";
};

U0 EdInsTree()
{
  U8 *st,*st2,buf[512];
  CEdTree *dt=CAlloc(sizeof(CEdTree));
  dt->collapsed=TRUE;
  dt->indent=2;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (!dt->collapsed)  CatPrint(buf,"-C");
    st=MPrint("%q",dt->label);
    st2=MPrint("$$TR%s,\"%Q\"$$\n$$ID,%d$$*\n$$ID,%d$$",
      buf,st,dt->indent,-dt->indent);
    DocPrintWhole(DocPut,"%s",st2);
    Free(st);
    Free(st2);
  }
  Free(dt);
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

#define PT_SPRITE		0
#define PT_SPRITE_PTR		1
#define PT_DUP_SPRITE		2
#define PT_SPRITE_SIZE		3
#define PT_SPRITE_MACRO		4
#define PT_SPRITE_MENU		5
#define PT_SPRITE_LINK		6

I64 PopUpSpriteType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$PURPLE$$$$TX+CX,\"Graphic Resource Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",A=\"FI:::/Doc/Resource.TXT.Z\"$$\n"
	"\n$$LTBLUE$$$$MU,\"Make Sprite\",LE=PT_SPRITE$$\n"
	"$$MU,\"Pointer to Sprite\",LE=PT_SPRITE_PTR$$\n"
	"$$MU,\"Duplicate Sprite\",LE=PT_DUP_SPRITE$$\n"
	"$$MU,\"Sprite Size (not really needed)\",LE=PT_SPRITE_SIZE$$\n"
	"$$MU,\"Sprite Macro (Icon)\",LE=PT_SPRITE_MACRO$$\n"
	"$$MU,\"Sprite Menu Item\",LE=PT_SPRITE_MENU$$\n"
	"$$MU,\"Sprite Link\",LE=PT_SPRITE_LINK$$\n"
	"\n$$MU,\"Abort\",LE=DOCM_CANCEL$$\n\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdJustSprite
{
  U8 tag[512]	fmtstr "$$DA-P,511,A=\"Tag Text       :%s\"$$\n";
};

class CEdMacroSprite
{
  U8 tag[512]		fmtstr "$$DA-P,511,A=\"Tag Text       :%s\"$$\n";
  U8 exp[512]		fmtstr "$$DA-P,511,A=\"Macro          :%s\"$$\n";
  Bool escape		fmtstr "$$CB,\"Escape\"$$\n";
  Bool popup		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool is_auto	fmtstr "$$CB,\"AutoStr\"$$\n";
};

class CEdMenuSprite
{
  U8 tag[512]		fmtstr "$$DA-P,511,A=\"Tag Text       :%s\"$$\n";
  U8 exp[512]		fmtstr "$$DA-P,511,A=\"Expression     :%s\"$$\n";
  Bool escape		fmtstr "$$CB,\"Escape\"$$\n";
};

U8 *EdSpriteLink(Bool *_pop_up,U8 **_html_link)
{
  U8 *result=NULL,*tag=NULL,*link=NULL;
  Bool quote=FALSE;
  I64 type=PopUpLinkType(FALSE);
  *_html_link=NULL;
  if (type>=0 && PmtLink(type,&tag,&link,_pop_up,&quote,_html_link))
    result=MPrint("\"%Q\",A=\"%Q\"",tag,link);
  Free(tag);
  Free(link);
  return result;
}

U8 *EdSprite(I64 bin_num)
{
  I64 type=PopUpSpriteType;
  Bool pop_up;
  U8 *st,*st1=NULL,*st2=NULL,buf[1024],*html_link=NULL;
  CEdMacroSprite *dt=CAlloc(sizeof(CEdMacroSprite));
  *buf=0;
  switch (type) {
    case PT_SPRITE:
      SPrint(dt->tag,"<%d>",bin_num);
      if (DocFormDo(dt,"CEdJustSprite",,,ctrl_L_footer))
	st1=MPrint("\"%Q\"",dt->tag);
      break;
    case PT_SPRITE_MACRO:
      dt->escape=TRUE;
      if (DocFormDo(dt,,,,ctrl_L_footer)) {
	CatPrint(buf,"+UL");
	if (dt->is_auto)CatPrint(buf,"+LA");
	if (dt->popup)	CatPrint(buf,"+PU");
	if (dt->escape)	CatPrint(buf,"+X");
	st=MPrint("%q",dt->exp);
	st1=MPrint("\"%Q\",LM=\"%Q\"",dt->tag,st);
	Free(st);
      }
      break;
    case PT_SPRITE_MENU:
      dt->escape=TRUE;
      if (DocFormDo(dt,"CEdMenuSprite",,,ctrl_L_footer)) {
	if (dt->escape)	CatPrint(buf,"+X");
	st1=MPrint("\"%Q\",LE=%s",dt->tag,dt->exp);
      }
      break;
    case PT_SPRITE_LINK:
      CatPrint(buf,"+L");
      st1=EdSpriteLink(&pop_up,&html_link);
      if (pop_up)
	CatPrint(buf,"+PU");
      break;
    case PT_SPRITE_PTR:
      EdInsBinary(--bin_num,DOCT_INS_BINARY);
      break;
    case PT_SPRITE_SIZE:
      EdInsBinary(--bin_num,DOCT_INS_BINARY_SIZE);
      break;
    case PT_DUP_SPRITE:
      EdInsBinary(--bin_num,DOCT_SPRITE);
      break;
  }
  if (st1) {
    if (html_link)
      st2=MPrint("$$SP%s,%s,HTML=\"%Q\",BI=%d$$",buf,st1,html_link,bin_num);
    else
      st2=MPrint("$$SP%s,%s,BI=%d$$",buf,st1,bin_num);
    Free(st1);
  }
  Free(dt);
  Free(html_link);
  return st2;
}
