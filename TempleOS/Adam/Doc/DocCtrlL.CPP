U8 *ctrl_L_footer=
	"\n$$MU,\"Abort\",DOCM_CANCEL$$\n"
	"\n\n$$LK+PU,\"Click for Help\",\"FI:::/TempleOS/Doc/Widget.TXT\"$$\n";

#define DFT_ADDR_LINK_BIN_SIZE	64

I64 PopUpLinkType(Bool include_anchor)
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$FG,PURPLE$$$$TX+CX,\"Link Type Menu\"$$\n"
	"\n$$FG,LTBLUE$$$$MU,\"To file\",LK_FILE$$\n"
	"$$MU,\"To anchor in file\",LK_FILE_ANCHOR$$\n"
	"$$MU,\"To str in file\",LK_FILE_FIND$$\n"
	"$$MU,\"To line in file\",LK_FILE_LINE$$\n"
	"$$MU,\"To man page\",LK_MAN_PAGE$$\n"
	"$$MU,\"To plain-text file\",LK_PLAIN$$\n"
	"$$MU,\"To str in plain-text file\",LK_PLAIN_FIND$$\n"
	"$$MU,\"To line in plain-text file\",LK_PLAIN_LINE$$\n"
	"$$MU,\"To Bible chapter line and verse\",LK_BIBLE_FIND$$\n"
	"$$MU,\"To Dictionary Definition\",LK_DEF$$\n"
	"$$MU,\"To Help Index\",LK_HELP_INDEX$$\n"
	"$$MU,\"To Addr\",LK_ADDR$$\n");
  if (include_anchor)
    DocPrint(doc,"$$MU,\"Place Anchor\",LK_PLACE_ANCHOR$$\n");
  DocPrint(doc,"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdFileLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136]	fmtstr "$$DA-P,135,\"File          :%s\"$$\n";
  U8	aux[136];
  I64	num;
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdFileAnchorLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136]	fmtstr "$$DA-P,135,\"File          :%s\"$$\n";
  U8	aux[136]	fmtstr "$$DA-P,135,\"Anchor Label  :%s\"$$\n";
  I64	num;
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdFileFindLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136]	fmtstr "$$DA-P,135,\"File          :%s\"$$\n";
  U8	aux[136]	fmtstr "$$DA-P,135,\"Str        :%s\"$$\n";
  I64	num		fmtstr "$$DA-T,135,\"Occurrence Num:%04d\"$$\n";
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdFileLineLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136]	fmtstr "$$DA-P,135,\"File          :%s\"$$\n";
  U8	aux[136];
  I64	num		fmtstr "$$DA-T,135,\"Line Num      :%04d\"$$\n";
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdManPageLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136];
  U8	aux[136]	fmtstr "$$DA-P,135,\"Label         :%s\"$$\n";
  I64	num;
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdAddrLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136];
  U8	aux[136]	fmtstr "$$DA-P,135,\"Addr Exp   :%s\"$$\n";
  I64	num		fmtstr "$$DA-T,135,\"Bin Size      :%04d\"$$\n";
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdPlaceAnchor
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136];
  U8	aux[136]	fmtstr "$$DA-P,135,\"Anchor Label  :%s\"$$\n";
  I64	num;
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up;
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide		fmtstr "$$CB,\"Hide\"$$\n";
};

class CEdBibleLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136];
  U8	aux[136]	fmtstr "$$DA-P,135,\"Chapter Verse :%s\"$$\n";
  I64	num;
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdDefLink
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136];
  U8	aux[136]	fmtstr "$$DA-P,135,\"Word          :%s\"$$\n";
  I64	num		fmtstr "$$DA-T,135,\"Def Num       :%4d\"$$\n";
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
  Bool	pop_up		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
  Bool	hide;
};

class CEdProject
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8	file[136]	fmtstr "$$DA-P,135,\"File          :%s\"$$\n";
  U8	aux[136];
  I64	num;
  U8	html_link[136]	fmtstr "$$DA-P,135,\"Html Link     :%s\"$$\n";
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
	    st=MPrint("$$AN+HTML,\"%Q\",\"%Q\",\"%Q\"$$",tag,link,html_link);
	  else
	    st=MPrint("$$AN,\"%Q\",\"%Q\"$$",tag,link);
	  break;
	default:
	  if (html_link)
	    st=MPrint("$$LK%s+HTML,\"%Q\",\"%Q\",\"%Q\"$$",pop_up_st,tag,link,html_link);
	  else
	    st=MPrint("$$LK%s,\"%Q\",\"%Q\"$$",pop_up_st,tag,link);
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
  DocPrint(doc,"$$FG,PURPLE$$$$TX+CX,\"Color Type Menu\"$$\n"
	"\n$$FG,LTBLUE$$$$MU,\"Foreground\",DOCT_FOREGROUND$$\n"
	"$$MU,\"Background\",DOCT_BACKGROUND$$\n"
	"$$MU,\"Default Foreground\",DOCT_DFT_FOREGROUND$$\n"
	"$$MU,\"Default Background\",DOCT_DFT_BACKGROUND$$\n"
	"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

I64 PopUpPageSettingType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$FG,PURPLE$$$$TX+CX,\"Page Setting Menu\"$$\n"
	"\n$$FG,LTBLUE$$$$MU,\"Page Length\",DOCT_PAGE_LEN$$\n"
	"$$MU,\"Page Header\",DOCT_HEADER$$\n"
	"$$MU,\"Page Footer\",DOCT_FOOTER$$\n"
	"$$MU,\"Left Margin\",DOCT_LEFT_MARGIN$$\n"
	"$$MU,\"Right Margin\",DOCT_RIGHT_MARGIN$$\n"
	"$$MU,\"Lock To Page\",DOCT_LOCK_TO_PAGE$$\n"
	"$$MU,\"Lock To Center\",DOCT_LOCK_TO_CENTER$$\n"
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
  U8	val[136]	fmtstr "$$DA-P,135,\"Setting Val:%s\"$$\n";
};

class CEdPageSetting2
{
  U8	val[136]	fmtstr "$$DA-P,135,\"Setting Val:%s\"$$\n";
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
  DocPrint(doc,"$$FG,PURPLE$$$$TX+CX,\"Text Widget Menu\"$$\n"
	"\n$$FG,LTBLUE$$$$MU,\"Link\",DOCT_LINK$$\n"
	"$$MU,\"Text\",DOCT_TEXT$$\n"
	"$$MU,\"Tree Branch\",DOCT_TREE$$\n"
	"$$MU,\"Color\",DOCT_FOREGROUND$$\n"
	"$$MU,\"Page Settings\",DOCT_PAGE_LEN$$\n"
	"$$MU,\"Cursor Movement\",DOCT_CURSOR_MOVEMENT$$\n"
	"$$MU,\"Macro\",DOCT_MACRO$$\n"
	"$$MU,\"Button\",DOCT_BUTTON$$\n"
	"$$MU,\"Check Box\",DOCT_CHECK_BOX$$\n"
	"$$MU,\"Menu Val\",DOCT_MENU_VAL$$\n"
	"$$MU,\"Data\",DOCT_DATA$$\n"
	"$$MU,\"Hex Edit\",DOCT_HEX_ED$$\n"
	"$$MU,\"Syntax Highlighting\",DOCT_HIGHLIGHT$$\n"
	"$$MU,\"HTML\",DOCT_HTML_CODE$$\n"
	"$$MU,\"Song\",DOCT_SONG$$\n"
	"%s",ctrl_L_footer);
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdDollarText
{
  U8	tag[136]	  fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  Bool	left_x	  fmtstr "$$CB,\"Left X\"$$\n";
  Bool	center_x  fmtstr "$$CB,\"Center X\"$$\n";
  Bool	right_x   fmtstr "$$CB,\"Right X\"$$\n";
  Bool	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool	blink	  fmtstr "$$CB,\"Blink\"$$\n";
  Bool	invert	  fmtstr "$$CB,\"Invert\"$$\n";
  Bool	underline fmtstr "$$CB,\"Underline\"$$\n";
  Bool	tree	  fmtstr "$$CB,\"Tree\"$$\n";
  Bool	collapsed fmtstr "$$CB,\"Collapsed\"$$\n";
  Bool	alias	  fmtstr "$$CB,\"Alias\"$$\n";
  U8	scroll_x [136]	  fmtstr "$$DA-P,135,\"Scroll X Length Expression:%s\"$$\n";
  U8	shift_x  [136]	  fmtstr "$$DA-P,135,\"X Offset Expression       :%s\"$$\n";
  U8	shift_y  [136]	  fmtstr "$$DA-P,135,\"Y Offset Expression       :%s\"$$\n";
  U8	define_str[136]	  fmtstr "$$DA-P,135,\"Define Str                :%s\"$$\n";
  U8	html_link[136]	  fmtstr "$$DA-P,135,\"Html Link                 :%s\"$$\n";
};

U0 EdInsDollarText()
{
  U8 *st,buf[136];
  CEdDollarText *dt=CAlloc(sizeof(CEdDollarText));
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->left_x  )  StrCat(buf,"+LX");
    if (dt->center_x)  StrCat(buf,"+CX");
    if (dt->right_x )  StrCat(buf,"+RX");
    if (dt->margin_rel) StrCat(buf,"+MRX");
    if (dt->blink)     StrCat(buf,"+BK");
    if (dt->invert)    StrCat(buf,"+IV");
    if (dt->underline) StrCat(buf,"+UL");
    if (dt->tree)      StrCat(buf,"+TR");
    if (dt->collapsed) StrCat(buf,"+C");
    if (dt->alias)     StrCat(buf,"+AL");
    if (*dt->scroll_x) StrCat(buf,"+SCX");
    if (*dt->shift_x)  StrCat(buf,"+SX");
    if (*dt->shift_y)  StrCat(buf,"+SY");
    if (*dt->define_str) StrCat(buf,"+D");
    if (*dt->html_link) StrCat(buf,"+HTML");
    st=MPrint("%q",dt->tag);
    "$$TX%s,\"%Q\"",buf,st;
    Free(st);
    if (*dt->shift_x)
      ",%s",dt->shift_x;
    if (*dt->shift_y)
      ",%s",dt->shift_y;
    if (*dt->scroll_x)
      ",%s",dt->scroll_x;
    if (*dt->define_str) {
      st=MPrint("%q",dt->define_str);
      ",\"%Q\"",st;
      Free(st);
    }
    if (*dt->html_link) {
      st=MPrint("%q",dt->html_link);
      ",\"%Q\"",st;
      Free(st);
    }
    "$$";
  }
  Free(dt);
}

class CEdSong
{
  U8 tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  U8 song[512]	fmtstr "$$DA-P,511,\"Song          :%s\"$$\n";
};

U0 EdInsSong()
{
  CEdSong *dt=CAlloc(sizeof(CEdSong));
  if (DocFormDo(dt,,,,ctrl_L_footer))
    "$$SO,\"%Q\",\"%s\"$$",dt->tag,dt->song;
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

#define HT_CODE		0
#define HT_IMAGE	1
#define HT_VIDEO	2

I64 PopUpHtmlType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$FG,LTBLUE$$$$MU,\"Html Code\",HT_CODE$$\n"
	"$$MU,\"Html Image\",HT_IMAGE$$\n"
	"$$MU,\"Html Video\",HT_VIDEO$$\n"
	"\n$$MU,\"Abort\",DOCM_CANCEL$$");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdHtmlCode
{
  U8 tag[512]	fmtstr "$$DA-P,511,\"Html Code:%s\"$$\n";
};

class CEdHtmlURL
{
  U8 tag[512]	fmtstr "$$DA-P,511,\"URL:%s\"$$\n";
};

U0 EdInsHtml()
{
  CEdHtmlCode *dt=CAlloc(sizeof(CEdHtmlCode));
  switch (PopUpHtmlType) {
    case HT_CODE:
      if (DocFormDo(dt,,,,ctrl_L_footer))
	"$$HC,\"%Q\"$$",dt->tag;
      break;
    case HT_IMAGE:
      if (DocFormDo(dt,"CEdHtmlURL",,,ctrl_L_footer))
	"$$HI,\"%Q\"$$",dt->tag;
      break;
    case HT_VIDEO:
      if (DocFormDo(dt,"CEdHtmlURL",,,ctrl_L_footer))
	"$$HV,\"%Q\"$$",dt->tag;
      break;
  }
  Free(dt);
}

class CEdMacroMenu
{
  U8 tag[136]		fmtstr "$$DA-P,135,\"Tag Text                  :%s\"$$\n";
  U8 left_macro[512]	fmtstr "$$DA-P,511,\"Left Click Macro          :%s\"$$\n";
  U8 left_exp[136]	fmtstr "$$DA-P,135,\"Left Click Expression     :%s\"$$\n";
  Bool popup		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool left_is_auto	fmtstr "$$CB,\"Left is AutoStr\"$$\n";
  Bool left_x		fmtstr "$$CB,\"Left X \"$$\n";
  Bool center_x	fmtstr "$$CB,\"Center X \"$$\n";
  Bool right_x	fmtstr "$$CB,\"Right X \"$$\n";
  Bool margin_rel	fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool blink		fmtstr "$$CB,\"Blink\"$$\n";
  Bool invert		fmtstr "$$CB,\"Invert\"$$\n";
  Bool underline	fmtstr "$$CB,\"Underline\"$$\n";
  Bool escape		fmtstr "$$CB,\"Escape\"$$\n";
  U8 scroll_x[136]	fmtstr "$$DA-P,135,\"Scroll X Length Expression:%s\"$$\n";
  U8 shift_x[136]	fmtstr "$$DA-P,135,\"X Offset Expression       :%s\"$$\n";
  U8 shift_y[136]	fmtstr "$$DA-P,135,\"Y Offset Expression       :%s\"$$\n";
};

U0 EdInsMacroMenu(Bool is_macro)
{
  U8 *st,buf[136];
  CEdMacroMenu *dt=CAlloc(sizeof(CEdMacroMenu));
  dt->underline=TRUE;
  dt->escape=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (is_macro && *dt->tag) StrCat(buf,"+A ");
    if (dt->popup)	StrCat(buf,"+PU-X");
    if (dt->left_x  )	StrCat(buf,"+LX");
    if (dt->center_x)	StrCat(buf,"+CX");
    if (dt->right_x )	StrCat(buf,"+RX");
    if (dt->margin_rel) StrCat(buf,"+MRX");
    if (dt->blink)	StrCat(buf,"+BK");
    if (dt->invert)	StrCat(buf,"+IV");
    if (!dt->underline) StrCat(buf,"-UL");
    if (!dt->escape)	StrCat(buf,"-X");
    if (*dt->scroll_x)	StrCat(buf,"+SCX");
    if (*dt->shift_x)	StrCat(buf,"+SX");
    if (*dt->shift_y)	StrCat(buf,"+SY");
    if (is_macro) {
      if (*dt->left_exp)	StrCat(buf,"+LE");
      if (!*dt->left_macro) StrCat(buf,"-LM");
    } else {
      if (!*dt->left_exp)	StrCat(buf,"-LE");
      if (*dt->left_macro) StrCat(buf,"+LM");
    }
    if (dt->left_is_auto) StrCat(buf,"+LA");
    if (*dt->tag || is_macro) {
      if (is_macro) {
	if (*dt->tag) {
	  st=MPrint("%q",dt->tag);
	  "$$MA%s,\"%Q\"",buf,st;
	  Free(st);
	} else
	  "$$MA%s,",buf;
      } else {
	st=MPrint("%q",dt->tag);
	"$$MU%s,\"%Q\"",buf,st;
	Free(st);
      }
      if (*dt->left_exp)
	",%s",dt->left_exp;
      if (*dt->left_macro) {
	st=MPrint("%q",dt->left_macro);
	",\"%Q\"",st;
	Free(st);
      }
      if (*dt->shift_x)
	",%s",dt->shift_x;
      if (*dt->shift_y)
	",%s",dt->shift_y;
      if (*dt->scroll_x)
	",%s",dt->scroll_x;
      "$$";
    }
  }
  Free(dt);
}

class CEdButton
{
  U8	tag[136]	   fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  Bool	popup	   fmtstr "$$CB,\"PopUp\"$$\n";
  Bool	left_x	   fmtstr "$$CB,\"Left X \"$$\n";
  Bool	center_x   fmtstr "$$CB,\"Center X \"$$\n";
  Bool	right_x	   fmtstr "$$CB,\"Right X \"$$\n";
  Bool	margin_rel fmtstr "$$CB,\"Margin Rel X\"$$\n";
  Bool	escape	   fmtstr "$$CB,\"Escape\"$$\n";
  U8	left_macro  [136]	fmtstr "$$DA-P,135,\"Left Click Macro:%s\"$$\n";
  Bool	left_is_auto	fmtstr "$$CB,\"Left is AutoStr\"$$\n";
  U8	left_exp    [136]	fmtstr "$$DA-P,135,\"Left Click Expression:%s\"$$\n";
  Bool	quote		fmtstr "$$CB,\"Quote\"$$\n";
};

U0 EdInsButton()
{
  U8 *st,buf[136];
  CEdButton *dt=CAlloc(sizeof(CEdButton));
  dt->escape=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->popup)    StrCat(buf,"+PU-X");
    if (dt->left_x  ) StrCat(buf,"+LX");
    if (dt->center_x) StrCat(buf,"+CX");
    if (dt->right_x ) StrCat(buf,"+RX");
    if (dt->margin_rel) StrCat(buf,"+MRX");
    if (!dt->escape)	StrCat(buf,"-X");
    if (!*dt->left_exp)	StrCat(buf,"-LE");
    if (*dt->left_macro) StrCat(buf,"+LM");
    if (dt->left_is_auto) StrCat(buf,"+LA");
    if (*dt->tag) {
      if (dt->quote) {
	st=MPrint("%q",dt->tag);
	"$$$$BT%s,\\\"%Q\\\"",buf,st;
	Free(st);
	if (*dt->left_exp)
	  ",%s",dt->left_exp;
	if (*dt->left_macro) {
	  st=MPrint("%q",dt->left_macro);
	  ",\\\"%Q\\\"",st;
	  Free(st);
	}
	"$$$$";
      } else {
	st=MPrint("%q",dt->tag);
	"$$BT%s,\"%Q\"",buf,st;
	Free(st);
	if (*dt->left_exp)
	  ",%s",dt->left_exp;
	if (*dt->left_macro) {
	  st=MPrint("%q",dt->left_macro);
	  ",\"%Q\"",st;
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
  U8 left_exp[136]	 fmtstr "$$DA-P,135,\"X Expression:%s\"$$\n";
  U8 right_exp[136]	 fmtstr "$$DA-P,135,\"Y Expression:%s\"$$\n";
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
  U8 buf[136];
  CEdCursorMovement *dt=CAlloc(sizeof(CEdCursorMovement));
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->left_x  ) StrCat(buf,"+LX");
    if (dt->center_x) StrCat(buf,"+CX");
    if (dt->right_x ) StrCat(buf,"+RX");
    if (dt->margin_rel) StrCat(buf,"+MRX");
    if (dt->top_y   ) StrCat(buf,"+TY");
    if (dt->center_y) StrCat(buf,"+CY");
    if (dt->bottom_y) StrCat(buf,"+BY");
    if (dt->page_rel) StrCat(buf,"+PRY");
    if (!*dt->left_exp)  StrCat(buf,"-LE");
    if (!*dt->right_exp) StrCat(buf,"-RE");
    if (dt->quote)
      "$$";
    "$$CM%s",buf;
    if (*dt->left_exp)
      ",%s",dt->left_exp;
    if (*dt->right_exp)
      ",%s",dt->right_exp;
    "$$";
    if (dt->quote)
      "$$";
  }
  Free(dt);
}

class CEdData
{
  U8	fmt_str [136]	fmtstr "$$DA-P,135,\"Format Str:%s\"$$\n";
  I64 len		fmtstr "$$DA,256,\"Length:%d\"$$\n";
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
  U8 *st,buf[136];
  CEdData *dt=CAlloc(sizeof(CEdData));
  dt->term=TRUE;
  dt->len=256;
  dt->u64=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->str)   StrCat(buf,"-P");
    if (!dt->term)    StrCat(buf,"-T");
    if (dt->refresh)  StrCat(buf,"+RD");
    if (dt->update)   StrCat(buf,"+UD");

    if (dt->u8)        StrCat(buf,"-U64+U8");
    else if (dt->u16)  StrCat(buf,"-U64+U16");
    else if (dt->u32)  StrCat(buf,"-U64+U32");

    st=MPrint("%q",dt->fmt_str);
    "$$$$DA%s,%d,\\\"%Q\\\"$$$$",buf,dt->len,st;
    Free(st);
  }
  Free(dt);
}

class CEdCheckBox
{
  U8	 label[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  Bool refresh	fmtstr "$$CB,\"Refresh Data\"$$\n";
};

U0 EdInsCheckBox()
{
  U8 *st,buf[136];
  CEdCheckBox *dt=CAlloc(sizeof(CEdCheckBox));
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (dt->refresh)  StrCat(buf,"+RD");
    st=MPrint("%q",dt->label);
    "$$$$CB%s,\\\"%Q\\\"$$$$",buf,st;
    Free(st);
  }
  Free(dt);
}

class CEdHexEd
{
  I64 cnt	fmtstr "$$DA,256,\"Count:%d\"$$\n";
  I64 cols	fmtstr "$$DA,256,\"Columns:%d\"$$\n";
  Bool zero	fmtstr "$$CB,\"Zero Based\"$$\n";
  Bool refresh fmtstr "$$CB,\"Refresh Data\"$$\n";
};

U0 EdInsHexEd()
{
  U8 buf[136];
  CEdHexEd *dt=CAlloc(sizeof(CEdHexEd));
  dt->cnt=128;
  dt->cols=4;
  dt->zero=TRUE;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (!dt->zero)  StrCat(buf,"-Z");
    if (dt->refresh)  StrCat(buf,"+RD");
    "$$$$HX%s,%d,%d$$$$",buf,dt->cnt,dt->cols;
  }
  Free(dt);
}


class CEdBinary
{
  U8  tag[136]	fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  I64 bin_num fmtstr "$$DA,256,\"Bin Num:%d\"$$\n";
  U8  bin_ptr_link_file[136] fmtstr "$$DA-P,135,\"File:%s\"$$\n";
  I64 bin_ptr_link_bin_num fmtstr "$$DA,256,\"File Bin Num:%d\"$$\n";
  U8  bin_ptr_link_tag[136] fmtstr "$$DA-P,135,\"File Bin Tag:%s\"$$\n";
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
	"$$%Z+BP,\"%Q\",%d,\"%s,%s\"$$",type,"ST_DOC_CMDS",dt->tag,dt->bin_num,
	  dt->bin_ptr_link_file,dt->bin_ptr_link_tag;
      else
	"$$%Z+BP,\"%Q\",%d,\"%s,%d\"$$",type,"ST_DOC_CMDS",dt->tag,dt->bin_num,
	  dt->bin_ptr_link_file,dt->bin_ptr_link_bin_num;
    } else {
      if (DocBinFindNum(DocPut,dt->bin_num))
	"$$%Z,\"%Q\",%d$$",type,"ST_DOC_CMDS",dt->tag,dt->bin_num;
      else
	PopUpOk("Invalid Binary Num");
    }
  }
  Free(dt);
}

class CEdTree
{
  U8	label[136] fmtstr "$$DA-P,135,\"Tag Text      :%s\"$$\n";
  I64	indent	   fmtstr "$$DA,135,\"Indention:%d\"$$\n";
  Bool	collapsed  fmtstr "$$CB,\"Collapsed\"$$\n";
};

U0 EdInsTree()
{
  U8 *st,*st2,buf[136];
  CEdTree *dt=CAlloc(sizeof(CEdTree));
  dt->collapsed=TRUE;
  dt->indent=2;
  if (DocFormDo(dt,,,,ctrl_L_footer)) {
    *buf=0;
    if (!dt->collapsed)  StrCat(buf,"-C");
    st=MPrint("%q",dt->label);
    st2=MPrint("$$TR%s,\"%Q\"$$\n$$ID,%d$$*\n$$ID,%d$$",
      buf,st,dt->indent,-dt->indent);
    DocInsSafe(DocPut,st2);
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
#define PT_SPRITE_TYPE		7

I64 PopUpSpriteType()
{
  I64 i;
  CDoc *doc=DocNew;
  DocPrint(doc,"$$FG,PURPLE$$$$TX+CX,\"Graphic Resource Menu\"$$\n"
	"$$LK+PU+CX,\"Click for Help\",\"FI:::/TempleOS/Doc/Resource.TXT.Z\"$$\n"
	"\n$$FG,LTBLUE$$$$MU,\"Make Sprite\",PT_SPRITE$$\n"
	"$$MU,\"Pointer to Sprite\",PT_SPRITE_PTR$$\n"
	"$$MU,\"Duplicate Sprite\",PT_DUP_SPRITE$$\n"
	"$$MU,\"Sprite Size (not really needed)\",PT_SPRITE_SIZE$$\n"
	"$$MU,\"Sprite Macro (Icon)\",PT_SPRITE_MACRO$$\n"
	"$$MU,\"Sprite Menu Item\",PT_SPRITE_MENU$$\n"
	"$$MU,\"Sprite Link\",PT_SPRITE_LINK$$\n"
	"$$MU,\"Binary Data Type (not needed)\",PT_SPRITE_TYPE$$\n"
	"\n$$MU,\"Abort\",DOCM_CANCEL$$\n\n");
  i=PopUpMenu(doc);
  DocDel(doc);
  return i;
}

class CEdJustSprite
{
  U8 tag[136]	fmtstr "$$DA-P,135,\"Tag Text       :%s\"$$\n";
};

class CEdMacroSprite
{
  U8 tag[136]		fmtstr "$$DA-P,135,\"Tag Text       :%s\"$$\n";
  U8 exp[512]		fmtstr "$$DA-P,511,\"Macro          :%s\"$$\n";
  Bool escape		fmtstr "$$CB,\"Escape\"$$\n";
  Bool popup		fmtstr "$$CB,\"PopUp\"$$\n";
  Bool is_auto	fmtstr "$$CB,\"AutoStr\"$$\n";
};

class CEdMenuSprite
{
  U8 tag[136]		fmtstr "$$DA-P,135,\"Tag Text       :%s\"$$\n";
  U8 exp[512]		fmtstr "$$DA-P,511,\"Expression     :%s\"$$\n";
  Bool escape		fmtstr "$$CB,\"Escape\"$$\n";
};

U8 *EdSpriteLink(Bool *_pop_up,U8 **_html_link)
{
  U8 *result=NULL,*tag=NULL,*link=NULL;
  Bool quote=FALSE;
  I64 type=PopUpLinkType(FALSE);
  *_html_link=NULL;
  if (type>=0 && PmtLink(type,&tag,&link,_pop_up,&quote,_html_link))
    result=MPrint("\"%Q\",\"%Q\"",tag,link);
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
	StrCat(buf,"+LM+UL");
	if (dt->is_auto)StrCat(buf,"+LA");
	if (dt->popup)	StrCat(buf,"+PU");
	if (dt->escape)	StrCat(buf,"+X");
	st=MPrint("%q",dt->exp);
	st1=MPrint("\"%Q\",\"%Q\"",dt->tag,st);
	Free(st);
      }
      break;
    case PT_SPRITE_MENU:
      dt->escape=TRUE;
      if (DocFormDo(dt,"CEdMenuSprite",,,ctrl_L_footer)) {
	StrCat(buf,"+LE");
	if (dt->escape)	StrCat(buf,"+X");
	st1=MPrint("\"%Q\",%s",dt->tag,dt->exp);
      }
      break;
    case PT_SPRITE_LINK:
      StrCat(buf,"+L+A");
      st1=EdSpriteLink(&pop_up,&html_link);
      if (pop_up)
	StrCat(buf,"+PU");
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
    case PT_SPRITE_TYPE:
      EdInsBinary(--bin_num,DOCT_INS_BINARY_TYPE);
      break;
  }
  if (st1) {
    if (html_link) {
      StrCat(buf,"+HTML");
      st2=MPrint("$$SP%s,%s,\"%Q\",%d$$",buf,st1,html_link,bin_num);
    } else
      st2=MPrint("$$SP%s,%s,%d$$",buf,st1,bin_num);
    Free(st1);
  }
  Free(dt);
  Free(html_link);
  return st2;
}
