U0 BadCodeJump()
{//$LK,"::/Adam/God/HSNotes.TXT"$
  CDirEntry *tempde1=FilesFind("/*",
	FUF_JUST_FILES|FUF_RECURSE|FUF_JUST_SRC|FUF_CLUSTER_ORDER),
	*tempde=tempde1;
  I64 cnt=0,num;
  CDoc *doc;
  U8 *st;
  while (tempde) {
    doc=DocRead(tempde->full_name);
    tempde->user_data=doc->root.y;
    cnt+=doc->root.y;
    DocDel(doc);
    tempde=tempde->next;
  }
  num=KbdMouseEvtTime>>GOD_BAD_BITS%cnt;
  tempde=tempde1;
  while (tempde) {
    num-=tempde->user_data;
    if (num<0) {
      st=MStrPrint("FL:%s,%d",tempde->full_name,-num);
      break;
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
  Ed(st);
  Free(st);
}

public U8 *FileNameNumNext(U8 *files_find_mask,U8 *fmt)
{
  CDirEntry *tempde=FilesFind(files_find_mask);
  U8 *result=MStrPrint(fmt,FileCnt(tempde));
  DirTreeDel(tempde);
  return result;
}

U0 DCImgWrite(CDC *dc)
{
  U8 *st1=FileNameNumNext("::/Home/Web/Imgs/Img*",
	"::/Home/Web/Imgs/Img%03d.BMP"),
	*st2=MStrPrint("http://www.templeos.org/Wb%s",st1+2);
  BMPWrite(st1,dc);
  Free(st1);
  "$$HC,\"<img src=\\\"%s\\\" width=\\\"%d\\\" "
	"height=\\\"%d\\\" alt=\\\"\\\">\"$$\n",st2,dc->width,dc->height;
  Free(st2);
}

U0 BlogScreenShot()
{
  Bool old_cursor=DocCursor;
  CDC *dc1,*dc2;
  U8 *old_fp_draw_input_ptr=gr.fp_draw_input_ptr;
  gr.fp_draw_input_ptr=NULL;
  WinMgrSync(2);
  dc1=DCScreenCapture;
  dc2=DCExtract(dc1,0,FONT_HEIGHT,GR_WIDTH-1,GR_HEIGHT-1);
  DCImgWrite(dc2);
  gr.fp_draw_input_ptr=old_fp_draw_input_ptr;
  DCDel(dc1);
  DCDel(dc2);
  DocCursor(old_cursor);
}

U8 *WebBookMarks(U8 *filename="::/Home/Downloads/Blog/Bookmarks.html")
{
  U8 *st;
  CDocEntry *doc_e;
  CDoc *doc=DocNew;
  CCmpCtrl *cc=CmpCtrlNew(MStrPrint("#include \"%s\"",filename));
  while (Lex(cc))
    if (cc->token==TK_STR && *cc->cur_str(U32 *)=='http') {
      DocPrint(doc,"$$MU-UL,\"%Q\",LE=0x%X$$\n",cc->cur_str,cc->cur_str);
      cc->cur_str=NULL;
    }
  CmpCtrlDel(cc);

  if ((st=PopUpMenu(doc))!=DOCM_CANCEL)
    st=StrNew(st);
  else
    st=NULL;

  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_MENU_VAL)
      Free(doc_e->left_exp);
    doc_e=doc_e->next;
  }
  DocDel(doc);

  return st;
}

U8 *YouTubeGet(U8 *filename="C:/Home/Downloads/Blog/YouTube.TXT.Z",U8 *needle)
{
  I64 i=0,line=MIN_I64;
  CDoc	*doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR),
	*doc2=DocNew;
  U8 *title,*code,*result;
  while (DocFind(doc,line,needle)) {
    if (!(doc->cur_entry->y&1)) {
      title=doc->cur_entry->tag;
      DocLineNumGoTo(doc,doc->cur_entry->y+2);
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
      result=NULL;
      break;
    case 1:
      result=StrNew(code);
      break;
    default:
      result=PopUpMenu(doc2);
      if (result==DOCM_CANCEL)
	result=NULL;
      else
	result=StrNew(result);
  }
  DocDel(doc);
  DocDel(doc2);
  return result;
}

U0 YouTubeAdd(U8 *filename="C:/Home/Downloads/Blog/YouTube.TXT.Z",
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

class CTopMoviesForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN];
  I64	timestamp;
  I64	cert		fmtstr "$$DA-TRM,2,A=\"Movie #1-100:%02d\"$$\n";
  I64	min;
  I64	sec;
};

class CMetallicaForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN];
  I64	timestamp;
  I64	cert		fmtstr "$$DA-TRM,2,A=\"Song #1-99:%02d\"$$\n";
  I64	min;
  I64	sec;
};

class CWebBookMarkForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,STR_LEN-1,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN];
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CWebBibleForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,STR_LEN-1,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"Bible Passage:%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CYouTubeForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,STR_LEN-1,A=\"Title:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P-TRM,11,A=\"SerialNum:%11s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min		fmtstr "$$DA,32,A=\"Minutes  :%d\"$$\n";
  I64	sec		fmtstr "$$DA,32,A=\"Seconds  :%d\"$$\n";
};

class CImgurForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"SerialNum:%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min		fmtstr "$$DA,32,A=\"Width    :%d\"$$\n";
  I64	sec		fmtstr "$$DA,32,A=\"Height   :%d\"$$\n";
};

class CWikipediaForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,STR_LEN-1,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"Index   :%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandDecForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"Line (Dec):%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandHexForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"Line (Hex):%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandCertForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN] fmtstr "$$DA-P,STR_LEN-1,A=\"Line (Dec) :%s\"$$\n";
  I64	timestamp;
  I64	cert		fmtstr "$$DA,32,A=\"Certificate:%d\"$$\n";
  I64	min;
  I64	sec;
};

class CRandHexBeacon5Form
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN]
			fmtstr "$$DA-P-TRM,5,A=\"Line (5-Digit Hex):%5s\"$$\n";
  I64	timestamp	fmtstr "$$DA,32,A=\"TimeStamp :%d\"$$\n";
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandHexBeacon8Form
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN]
			fmtstr "$$DA-P-TRM,8,A=\"Line (8-Digit Hex):%8s\"$$\n";
  I64	timestamp	fmtstr "$$DA,32,A=\"TimeStamp :%d\"$$\n";
  I64	cert;
  I64	min;
  I64	sec;
};

U8 *RandVideoRead(U8 *filename,I64 line)
{
  U8 *result=NULL;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (DocLineNumGoTo(doc,line-1) && doc->cur_entry->type_u8==DOCT_TEXT)
    result=StrNew(doc->cur_entry->tag);
  DocDel(doc);
  return result;
}

U8 *RandVideoWrite(U8 *filename,I64 line,U8 *st)
{
  U8 *result=NULL;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (DocLineNumGoTo(doc,line-1)) {
    if (doc->cur_entry->type_u8==DOCT_TEXT) {
      Free(doc->cur_entry->tag);
      doc->cur_entry->tag=StrNew(st);
    } else
      DocPrint(doc,"%s",st);
    DocTop(doc);
    DocWrite(doc);
  }
  DocDel(doc);
  return result;
}

U0 RandVideo(U8 *title,U8 *webpage,CTopMoviesForm *url,U8 *filename)
{
  U8 *st;
  if (st=RandVideoRead(filename,url->cert*2)) {
    StrCpy(url->tag,st);
    Free(st);
    if (!(st=RandVideoRead(filename,url->cert*2+1))||!*st) {
      Free(st);
      if (DocForm(url(CYouTubeForm *)) && *url->special) {
	if (url->min || url->sec)
	  st=MStrPrint("%s&hl=enUS&start=%d",
		url->special,url->min*60+url->sec);
	else
	  st=StrNew(url->special);
	RandVideoWrite(filename,url->cert*2+1,st);
      } else
	st=NULL;
    }
    "God's response:\n"
	  "$$TX,\"%s\",HTML=\"%s\"$$\n"
	  "#%s\n",title,webpage,url->tag;
    if (st) {
      "$$HC,\"<object width=\\\"640\\\" height=\\\"520\\\">"
	    "<param name=\\\"movie\\\" "
	    "value=\\\"http://www.youtube.com/v/%s\\\"></param><param "
	    "name=\\\"allowscriptaccess\\\" "
	    "value=\\\"always\\\"></param><embed "
	    "src=\\\"http://www.youtube.com/v/%s\\\" "
	    "type=\\\"application/x-shockwave-flash\\\" "
	    "allowscriptaccess=\\\"always\\\" "
	    "width=\\\"640\\\" height=\\\"520\\\">"
	    "</embed></object>\"$$\n",st,st;
      Free(st);
    }
  }
}

U8 *URLPercentSpaces(U8 *src)
{
  U8 buf[1024],*dst=buf;
  while (*src) {
    if (*src==CH_SPACE) {
      *dst++='%';
      *dst++='%';
      *dst++='2';
      *dst++='0';
      src++;
    } else
      *dst++=*src++;
  }
  *dst=0;
  return StrNew(buf);
}

U0 MyBibleLines(U8 *st,I64 num_lines,I64 radix,Bool modulo,Bool plus_one)
{
  CDoc *doc_out,*doc_in;
  CDocEntry *doc_e;
  I64 i,start;
  U8 *verse;
  if (*st && (doc_out=DocPut)) {
    DocPrint(doc_out,"$$BG,LTGRAY$$");

    i=Str2I64(st,radix);
    if (modulo) {
      if (radix==16)
	DocPrint(doc_out,"0x%s-->",st);
      DocPrint(doc_out,"(%d remainder when divided by %d)",
	    i,ST_BIBLE_LINES-(num_lines-1));
      start=i%(ST_BIBLE_LINES-(num_lines-1));
    } else {
      if (radix==16)
	DocPrint(doc_out,"0x%s-->",st);
      DocPrint(doc_out,"%d",i);
      start=i;
    }
    if (plus_one) {
      start++;
      DocPrint(doc_out,"+1");
    }
    DocPrint(doc_out,"-->Line:%d\n",start);

    verse=BibleLine2Verse(start);
    DocPrint(doc_out,"$TX,"KingJamesBible",HTML="http://www.templeos.org/files/NumBible.TXT"$ (%s)\n$$BG$$\nGod says...\n$$RED$$",verse);
    Free(verse);

    doc_in=DocNew;
    BibleLines(doc_in,start,num_lines);
    while (TRUE) {//Del leading blanks
      doc_e=doc_in->root.next;
      if (doc_e==doc_in)
	break;
      else {
	if (doc_e->type_u8==DOCT_NEW_LINE)
	  DocEntryDel(doc_in,doc_e);
	else
	  break;
      }
    }
    while (TRUE) {//Del trailing blanks
      doc_e=doc_in->cur_entry->last;
      if (doc_e==doc_in)
	break;
      else {
	if (doc_e->type_u8==DOCT_NEW_LINE)
	  DocEntryDel(doc_in,doc_e);
	else
	  break;
      }
    }
    DocPrint(doc_in,"$$FG$$\n");
    DocInsDoc(doc_out,doc_in);
    DocDel(doc_in);
  }
}

public U0 GodBooksPassage(U8 *files_find_mask,I64 len=512,
	I64 rand_i32=MIN_I64,Bool full=FALSE)
{//Make God pick a book passage. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.TXT"$
  CDirEntry *tempde=FilesFind(files_find_mask,
	FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT),
	*tempde1=tempde;
  I64 i=0;
  U8 *st,*st2=NULL;
  "$$BG,LTGRAY$$";
  while (tempde) {
    st=FileRead(tempde->full_name);
    tempde->user_data=StrLen(st);
    if (full)
      "%08X-",i;
    if (tempde->user_data>=len)
      i+=tempde->user_data+1-len;
    if (full)
      "%08X:$$LK,\"%s\",A=\"FI:%s\"$$\n",
	    i-1,tempde->full_name+2,tempde->full_name;
    Free(st);
    tempde=tempde->next;
  }
  if (rand_i32<0)
    rand_i32=GodBits(32);
  if (full)
    "                                   \n%08X*%08X",rand_i32,i;
  i*=rand_i32;
  if (full)
    "=%08X.%08X\n",i.u32[1],i.u32[0];
  i=i.u32[1];
  tempde=tempde1;
  while (tempde) {
    if (!st2 && tempde->user_data>=len) {
      i-=tempde->user_data+1-len;
      if (i<0) {
	st=FileRead(tempde->full_name);
	st2=st+(tempde->user_data+1-len)+i;
	st2[len]=0;
	"%s\n$$BG$$\nGod says...\n$$RED$$%s$$FG$$\n",tempde->full_name,st2;
	Free(st);
      }
    }
    tempde=tempde->next;
  }
  DirTreeDel(tempde1);
}

#define NIST_TIME_OFFSET	(-7879-local_time_offset/CDATE_FREQ)
#define NIST_TIME_TO_SWITCH	3

public U0 NISTSync()
{
  I64 i,j;
  CDate d;
  CDateStruct ds;
  while (!ScanChar) {
    d=Now;
    Date2Struct(&ds,d);
    i=ToI64((d-Str2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET);
    j=FloorI64(i,60);
    "%d %d %02d %02d\n",i,j,ds.sec,(60+ds.sec-(i-j))%60;
    Sleep(100);
  }
}

U0 InsMyURL()
{
  U8 *tag,*st,*st2;
  CWebBibleForm *url=CAlloc(sizeof(CWebBibleForm));
  url->timestamp=
	FloorI64((Now-Str2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET-
	NIST_TIME_TO_SWITCH,60);
  switch (PopUpPickLst("NISTBeacon5Bible\0NISTBeacon8Passage\0"
	"BibleGatewaySearch\0BibleGatewayVerse\0"
	"YouTube\0Imgur\0Wikipedia\0RandomNumbers.info\0"
	"Random.org\0PassWord\0VirtualNotary\0ANU_NIST\0HotBits\0"
	"GenerateData\0WebBookMark\0Roulette\0Movie\0Metallica\0")) {
    case:
      if (DocForm(url(CRandHexBeacon5Form *))) {
	"$$BG,LTGRAY$$";
//	if (FileFind("::/Home/Web/TAD/NISTPassage.TXT.Z"))
//	  Type("::/Home/Web/TAD/NISTPassage.TXT.Z",0);
	"$$TX,\"Guidelines for Talking with God.\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag=MStrPrint("https:/beacon.nist.gov/rest/record/%d",url->timestamp);
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	Free(tag);
	MyBibleLines(url->special,20,16,TRUE,TRUE);
      }
      break;
    case:
      if (DocForm(url(CRandHexBeacon8Form *))) {
	"$$BG,LTGRAY$$";
//	if (FileFind("::/Home/Web/TAD/NISTPick.TXT.Z"))
//	  Type("::/Home/Web/TAD/NISTPick.TXT.Z",0);
	"$$TX,\"Guidelines for Talking with God.\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag=MStrPrint("https:/beacon.nist.gov/rest/record/%d",url->timestamp);
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	Free(tag);
	GodBooksPassage("C:/Home/Downloads/Text/*",,
	      Str2I64(url->special,16),TRUE);
      }
      break;
    case:
      if (DocForm(url(CWebBibleForm *))) {
	tag=url->special;
	st2=URLPercentSpaces(url->special);
	st=MStrPrint(
	      "http://www.biblegateway.com/passage/?search=%s&version=NIV",st2);
	Free(st2);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
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
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
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
	"$$HC,\"<img src=\\\"http://i.imgur.com/%s\\\" "
	      "width=\\\"%d\\\" height=\\\"%d\\\" alt=\\\"\\\">\"$$\n",
	      url->special,url->min,url->sec;
      }
      break;
    case:
      if (DocForm(url(CWikipediaForm *))) {
	tag=url->special;
	st=MStrPrint("http://en.wikipedia.org/wiki/%s",url->special);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
	Free(st);
      }
      break;
    case:
      if (DocForm(url(CRandDecForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="http://www.randomnumbers.info";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,TRUE);
      }
      break;
    case:
      if (DocForm(url(CRandDecForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="http://www.random.org";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case:
      if (DocForm(url(CRandDecForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="https://passed.pw";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case:
      if (DocForm(url(CRandCertForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="http://virtual-notary.org";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n",tag,tag;
	tag="Here is the certificate";
	st=MStrPrint("http://www.templeos.org/Certs/virtual-notary-cert"
	      "-randomnum-%d.p12",url->cert);
	"\n$$TX,\"%Q: %d\",HTML=\"%Q\"$$.\n$$BG$$",tag,url->cert,st;
	Free(st);
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case:
      if (DocForm(url(CRandHexForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="http://qrng.anu.edu.au/NIST.php";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,16,TRUE,TRUE);
      }
      break;
    case:
      if (DocForm(url(CRandHexForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="https://www.fourmilab.ch/hotbits/secure_generate.html";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,16,TRUE,TRUE);
      }
      break;
    case:
      if (DocForm(url(CRandDecForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="http://www.generatedata.com";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case:
      if (DocForm(url(CWebBookMarkForm *))) {
	tag=st=WebBookMarks;
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
	Free(st);
      }
      break;
    case:
      if (DocForm(url(CRandDecForm *))) {
	"$$BG,LTGRAY$$$$TX,\"Guidelines for Talking with God\","
	      "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
	tag="https://www.dublinbet.com";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n$$BG$$",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case:
      if (DocForm(url(CTopMoviesForm *)))
	RandVideo("Top 100 Movies","http://www.afi.com/100years/movies10.aspx",
	      url,"C:/Home/Downloads/Blog/Movies.TXT.Z");
      break;
    case:
      if (DocForm(url(CMetallicaForm *)))
	RandVideo("Metallica Song",
	      "http://home.hccnet.nl/a.r.adams/lyrics/metallica/songindex.html",
	      url,"C:/Home/Downloads/Blog/Metallica.TXT.Z");
      break;
  }
  Free(url);
}

if (FileFind("::/Home/Downloads/Words/LinuxDict.TXT"))
  GodInit("::/Home/Downloads/Words/LinuxDict.TXT");
