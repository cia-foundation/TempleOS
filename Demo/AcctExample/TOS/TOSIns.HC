#help_index "Misc/Blog"

U8 *YouTubeGet(U8 *filename="~/Sup1/Sup1Blog/YouTube.DD.Z",U8 *needle)
{
  I64 i=0,line=I64_MIN;
  CDoc	*doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR),
	*doc2=DocNew;
  U8 *title,*code,*res;
  while (DocFind(doc,line,needle)) {
    if (!(doc->cur_entry->y&1)) {
      title=doc->cur_entry->tag;
      DocGoToLine(doc,doc->cur_entry->y+2);
      if (doc->cur_entry->type_u8==DOCT_TEXT) {
	code=doc->cur_entry->tag;
	DocPrint(doc2,"$$MU-UL,\"%s\",LE=0x%X$$\n",title,code);
	i++;
      }
    }
    line=doc->cur_entry->y+2;
  }
  switch (i) {
    case 0:
      res=NULL;
      break;
    case 1:
      res=StrNew(code);
      break;
    default:
      res=PopUpMenu(doc2);
      if (res==DOCM_CANCEL)
	res=NULL;
      else
	res=StrNew(res);
  }
  DocDel(doc);
  DocDel(doc2);
  return res;
}

U0 YouTubeAdd(U8 *filename="~/Sup1/Sup1Blog/YouTube.DD.Z",
	U8 *title,U8 *code)
{
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (!DocFind(doc,,title)) {
    DocBottom(doc);
    DocPrint(doc,"%s\n%s\n",title,code);
    DocTop(doc);
    DocWrite(doc);
    Sort(filename,,2);
  }
  DocDel(doc);
}

U8 *WebBookMarks(U8 *filename="~/Sup1/Sup1Blog/Bookmarks.html")
{
  U8 *st;
  CDocEntry *doc_e;
  CDoc *doc=DocNew;
  CCmpCtrl *cc=CmpCtrlNew(MStrPrint("#include \"%s\"",filename));
  while (Lex(cc))
    if (cc->token==TK_STR && *cc->cur_str(U32 *)=='http') {
      DocPrint(doc,"$$MU-UL,\"%$$Q\",LE=0x%X$$\n",cc->cur_str,cc->cur_str);
      cc->cur_str=NULL;
    }
  CmpCtrlDel(cc);

  if ((st=PopUpMenu(doc))!=DOCM_CANCEL)
    st=StrNew(st);
  else
    st=NULL;

  doc_e=doc->head.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_MENU_VAL)
      Free(doc_e->left_exp);
    doc_e=doc_e->next;
  }
  DocDel(doc);

  return st;
}

U0 TOSIns()
{
  CWebBibleForm *url=CAlloc(sizeof(CWebBibleForm));
  U8 *tag,*st,*st2;

  switch (PopUpPickLst("BiblePassage\0BibleVerse\0"
	"YouTube\0Imgur\0Wikipedia\0WebBookMark\0")) {
    case:
      if (DocForm(url(CWebBibleForm *))) {
	tag=url->special;
	st2=URLPercentSpaces(url->special);
	st=MStrPrint("http://www.biblegateway.com/passage/"
	      "?search=%s&version=NIV",st2);
	Free(st2);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$",tag,st;
	Free(st);
      }
      break;
    case:
      if (DocForm(url(CWebBibleForm *))) {
	tag=url->special;
	st2=URLPercentSpaces(url->special);
	st=MStrPrint(
	      "http://www.biblegateway.com/verse/en/%s",st2);
	Free(st2);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$",tag,st;
	Free(st);
      }
      break;
    case:
      if (DocForm(url(CYouTubeForm *))) {
	if (*url->special==CH_SPACE)
	  st2=YouTubeGet(,url->tag);
	else
	  st2=StrNew(url->special);
	if (st2 && *st2!=CH_SPACE) {
	  if (url->min || url->sec)
	    st=MStrPrint("%s&hl=enUS&start=%d",st2,url->min*60+url->sec);
	  else
	    st=StrNew(st2);
	  if (*url->special!=CH_SPACE && *url->tag)
	    YouTubeAdd(,url->tag,st);
	  "$$HC,\"<object width=\\\"640\\\" "
		"height=\\\"520\\\"><param name=\\\"movie\\\" "
		"value=\\\"http://www.youtube.com/v/%s\\\"></param><param "
		"name=\\\"allowscriptaccess\\\" "
		"value=\\\"always\\\"></param><embed "
		"src=\\\"http://www.youtube.com/v/%s\\\" "
		"type=\\\"application/x-shockwave-flash\\\" "
		"allowscriptaccess=\\\"always\\\" "
		"width=\\\"640\\\" height=\\\"520\\\">"
		"</embed></object>\"$$\n",st,st;
	  Free(st);
	  Free(st2);
	}
      }
      break;
    case:
      if (DocForm(url(CImgurForm *))) {
	"$$HC,\"<center><img src=\\\"http://i.imgur.com/%s\\\" "
	      "width=\\\"%d\\\" height=\\\"%d\\\" "
	      "alt=\\\"\\\"></center>\"$$\n",
	      url->special,url->min,url->sec;
      }
      break;
    case:
      if (DocForm(url(CWikipediaForm *))) {
	tag=url->special;
	st=MStrPrint("http://en.wikipedia.org/wiki/%s",url->special);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$",tag,st;
	Free(st);
      }
      break;
    case:
      if (DocForm(url(CWebBookMarkForm *))) {
	tag=st=WebBookMarks;
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$",tag,st;
	Free(st);
      }
      break;
  }
  Free(url);
}

#help_index "Misc/TOS/God;God/TOS"

public U0 GodCodeJmp()
{//Jump to rand code in OS. See $LK,"::/Adam/God/HSNotes.DD"$.
  CDirEntry *tmpde1=FilesFind("/*",
	FUF_JUST_FILES|FUF_RECURSE|FUF_JUST_SRC|FUF_CLUS_ORDER),
	*tmpde=tmpde1;
  I64 cnt=0,num;
  CDoc *doc;
  U8 *st;
  while (tmpde) {
    doc=DocRead(tmpde->full_name);
    tmpde->user_data=doc->head.y;
    cnt+=doc->head.y;
    DocDel(doc);
    tmpde=tmpde->next;
  }

  FifoU8Flush(god.fifo);
  GodBitsIns(GOD_GOOD_BITS,KbdMsEvtTime>>GOD_BAD_BITS);
  num=GodBits(GOD_GOOD_BITS)%cnt;

  tmpde=tmpde1;
  while (tmpde) {
    num-=tmpde->user_data;
    if (num<0) {
      st=MStrPrint("FL:%s,%d",tmpde->full_name,-num);
      break;
    }
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
  Ed(st);
  Free(st);
}

U0 GodOffer()
{
  switch (PopUpPickLst("MosesComic\0Fiction\0")) {
    case:
      "---- Moses Comic ---- "
	    "(Set in the $TX,"Numbers 11",HTML="http://www.biblegateway.com/passage/?search=Numbers%%%%2011&version=NIV"$ "
	    "part of the story.)\n"
	    "Moses says, \"\"\n"
	    "God says, \"\"\n";
      break;
    case:
      "---- $TX,"Collaborative Fiction",HTML="http://en.wikipedia.org/wiki/Collaborative_fiction"$ ----\n"
	    "Once upon a time ";
      break;
  }
}
