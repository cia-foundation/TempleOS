U0 BadCodeJump()
{ //$LK,"::/Adam/God/HSNotes.TXT"$
  CDirEntry *tempde1=FilesFind("/*",FUF_JUST_FILES|FUF_RECURSE|FUF_JUST_SRC|FUF_CLUSTER_ORDER),
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
      st=MPrint("FL:%s,%d",tempde->full_name,-num);
      break;
    }
    tempde=tempde->next;
  }
  DirLstDel(tempde1);
  Ed(st);
  Free(st);
}

public U8 *FileNameNumNext(U8 *files_find_mask,U8 *fmt)
{
  CDirEntry *tempde=FilesFind(files_find_mask);
  U8 *result=MPrint(fmt,FileCnt(tempde));
  DirLstDel(tempde);
  return result;
}

U0 DCImgWrite(CDC *dc)
{
  U8 *st=FileNameNumNext("::" TS_BLOG_DIR "/Img*","::" TS_BLOG_DIR "/Img%03d.BMP"),*st2;
  st2=MPrint("http://www.templeos.org/Wb%s",st+2);
  BMPFileWrite(st,dc);
  Free(st);
  "$$HC,\"<img src=\\\"%s\\\" width=\\\"%d\\\" height=\\\"%d\\\" alt=\\\"\\\">\"$$\n",st2,dc->width,dc->height;
  Free(st2);
}

U0 BlogScreenShot()
{
  Bool old_cursor=DocCursor;
  CDC *dc1,*dc2;
  U8 *old_fp_draw_input_ptr=fp_draw_input_ptr;
  fp_draw_input_ptr=NULL;
  WinMgrSync(2);
  dc1=DCCaptureScreen;
  dc2=DCExtract(dc1,0,FONT_HEIGHT,GR_WIDTH-1,GR_HEIGHT-1);
  DCImgWrite(dc2);
  fp_draw_input_ptr=old_fp_draw_input_ptr;
  DCDel(dc1);
  DCDel(dc2);
  DocCursor(old_cursor);
}

U8 *WebBookMarks(U8 *filename="HOME/Archive/Bookmarks.html")
{
  U8 *st;
  CDocEntry *doc_e;
  CDoc *doc=DocNew;
  CLex *lx=LexNew(MPrint("#include \"%s\"",filename));
  while (Lex(lx))
    if (lx->token==TK_STR && *lx->cur_str(U32 *)=='http') {
      DocPrint(doc,"$$MU-UL,\"%Q\",LE=0x%X$$\n",lx->cur_str,lx->cur_str);
      lx->cur_str=NULL;
    }
  LexDel(lx);

  st=StrNew(PopUpMenu(doc));

  doc_e=doc->root.next;
  while (doc_e!=doc) {
    if (doc_e->type_u8==DOCT_MENU_VAL)
      Free(doc_e->left_exp);
    doc_e=doc_e->next;
  }
  DocDel(doc);

  return st;
}

class CWebBookMarkForm
{
  U8	tag[136]	fmtstr "$$DA-P,135,A=\"Tag Text     :%s\"$$\n";
  U8	special[136];
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CWebBibleForm
{
  U8	tag[136]	fmtstr "$$DA-P,135,A=\"Tag Text     :%s\"$$\n";
  U8	special[136]	fmtstr "$$DA-P,135,A=\"Bible Passage:%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CYouTubeForm
{
  U8	tag[136];
  U8	special[136]	fmtstr "$$DA-P,135,A=\"SerialNum:%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min		fmtstr "$$DA,32,A=\"Minutes  :%d\"$$\n";
  I64	sec		fmtstr "$$DA,32,A=\"Seconds  :%d\"$$\n";
};

class CImgurForm
{
  U8	tag[136];
  U8	special[136]	fmtstr "$$DA-P,135,A=\"SerialNum:%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min		fmtstr "$$DA,32,A=\"Width    :%d\"$$\n";
  I64	sec		fmtstr "$$DA,32,A=\"Height   :%d\"$$\n";
};

class CWikipediaForm
{
  U8	tag[136]	fmtstr "$$DA-P,135,A=\"Tag Text:%s\"$$\n";
  U8	special[136]	fmtstr "$$DA-P,135,A=\"Index   :%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandDecForm
{
  U8	tag[136];
  U8	special[136]	fmtstr "$$DA-P,135,A=\"Line (Dec):%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandHexForm
{
  U8	tag[136];
  U8	special[136]	fmtstr "$$DA-P,135,A=\"Line (Hex):%s\"$$\n";
  I64	timestamp;
  I64	cert;
  I64	min;
  I64	sec;
};

class CRandCertForm
{
  U8	tag[136];
  U8	special[136]	fmtstr "$$DA-P,135,A=\"Line (Dec) :%s\"$$\n";
  I64	timestamp;
  I64	cert		fmtstr "$$DA,32,A=\"Certificate:%d\"$$\n";
  I64	min;
  I64	sec;
};

class CRandHexBeaconForm
{
  U8	tag[136];
  U8	special[136]	fmtstr "$$DA-P,135,A=\"Line (Hex):%s\"$$\n";
  I64	timestamp	fmtstr "$$DA,32,A=\"TimeStamp :%d\"$$\n";
  I64	cert;
  I64	min;
  I64	sec;
};

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

    i=A2I64(st,radix);

    if (modulo) {
      DocPrint(doc_out,"$TX,"(Modulo is just remainder.)",HTML="http://en.wikipedia.org/wiki/Modulo_operation"$\n");
      if (radix==16)
	DocPrint(doc_out,"0x%s-->",st);
      DocPrint(doc_out,"(%d mod %d)",i,ST_BIBLE_LINES-(num_lines-1));
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
    DocPrint(doc_out,"-->Line:%d\n\n",start);

    verse=BibleLine2Verse(start);
    DocPrint(doc_out,"$TX,"KingJamesBible",HTML="http://www.templeos.org/files/BIBLE.TXT"$ (%s) God says...\n$$RED$$",verse);
    Free(verse);

    doc_in=DocNew;
    BibleLines(doc_in,start,num_lines);
    while (TRUE) { //Del leading blanks
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
    while (TRUE) { //Del trailing blanks
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

#define NIST_TIME_OFFSET	(5*3600-353)

public U0 SyncNIST()
{
  while (!ScanChar) {
    "%d %d\n",
	ToI64	((Now-A2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET),
	FloorI64((Now-A2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET,60);
    Sleep(100);
  }
}

U0 FillInMyURL()
{
  U8 *tag,*st,*st2;
  CWebBibleForm *url=CAlloc(sizeof(CWebBibleForm));
  url->timestamp=
	FloorI64((Now-A2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET,60);
  switch (PopUpPickLst("NISTBeacon\0WebBible\0YouTube\0Imgur\0Wikipedia\0RandomNumbers.info\0"
	"Random.org\0VirtualNotary\0Nist\0HotBits\0GenerateData\0WebBookMark\0")) {
    case 0:
      if (DocFormDo(url(CRandHexBeaconForm *))) {
	"$$TX,\"National Institute of Standards Public Randomness Beacon\","
	"HTML=\"http://beacon.nist.gov/home\"$$\n\n"
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag=MPrint("http://beacon.nist.gov/rest/record/%d",url->timestamp);
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n\n",tag,tag;
	Free(tag);
	MyBibleLines(url->special,20,16,TRUE,TRUE);
      }
      break;
    case 1:
      if (DocFormDo(url(CWebBibleForm *))) {
	tag=url->special;
	st2=URLPercentSpaces(url->special);
	st=MPrint("http://www.biblegateway.com/passage/?search=%s&version=NIV",st2);
	Free(st2);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
	Free(st);
      }
      break;
    case 2:
      if (DocFormDo(url(CYouTubeForm *))) {
	if (url->min || url->sec)
	  st=MPrint("%s&hl=enUS&start=%d",url->special,url->min*60+url->sec);
	else
	  st=StrNew(url->special);
	"$$HC,\"<object width=\\\"640\\\" height=\\\"520\\\"><param name=\\\"movie\\\" "
	"value=\\\"http://www.youtube.com/v/%s\\\"></param><param "
	"name=\\\"allowscriptaccess\\\" value=\\\"always\\\"></param><embed "
	"src=\\\"http://www.youtube.com/v/%s\\\" "
	"type=\\\"application/x-shockwave-flash\\\" allowscriptaccess=\\\"always\\\" "
	"width=\\\"640\\\" height=\\\"520\\\"></embed></object>\"$$\n",st,st;
	Free(st);
      }
      break;
    case 3:
      if (DocFormDo(url(CImgurForm *))) {
	"$$HC,\"<img src=\\\"http://i.imgur.com/%s\\\" "
	"width=\\\"%d\\\" height=\\\"%d\\\" alt=\\\"\\\">\"$$\n",
	  url->special,url->min,url->sec;
      }
      break;
    case 4:
      if (DocFormDo(url(CWikipediaForm *))) {
	tag=url->special;
	st=MPrint("http://en.wikipedia.org/wiki/%s",url->special);
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
	Free(st);
      }
      break;
    case 5:
      if (DocFormDo(url(CRandDecForm *))) {
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag="http://www.randomnumbers.info";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n\n",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,TRUE);
      }
      break;
    case 6:
      if (DocFormDo(url(CRandDecForm *))) {
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag="http://www.random.org";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n\n",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case 7:
      if (DocFormDo(url(CRandCertForm *))) {
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag="http://virtual-notary.org";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n",tag,tag;
	tag="Here is the certificate";
	st=MPrint("http://www.templeos.org/Certs/virtual-notary-cert-randomnum-%d.p12",url->cert);
	"\n$$TX,\"%Q: %d\",HTML=\"%Q\"$$.\n\n",tag,url->cert,st;
	Free(st);
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case 8:
      if (DocFormDo(url(CRandHexForm *))) {
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag="http://qrng.anu.edu.au/NIST.php";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n\n",tag,tag;
	MyBibleLines(url->special,20,16,TRUE,TRUE);
      }
      break;
    case 9:
      if (DocFormDo(url(CRandHexForm *))) {
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag="https://www.fourmilab.ch/hotbits/secure_generate.html";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n\n",tag,tag;
	MyBibleLines(url->special,20,16,TRUE,TRUE);
      }
      break;
    case 10:
      if (DocFormDo(url(CRandDecForm *))) {
	"$$TX,\"Guidelines for Talking with God\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n\n";
	tag="http://www.generatedata.com";
	"$$TX,\"%Q\",HTML=\"%Q\"$$\n\n",tag,tag;
	MyBibleLines(url->special,20,10,FALSE,FALSE);
      }
      break;
    case 11:
      if (DocFormDo(url(CWebBookMarkForm *))) {
	tag=st=WebBookMarks;
	if (*url->tag)
	  tag=url->tag;
	"$$TX,\"%Q\",HTML=\"%Q\"$$",tag,st;
	Free(st);
      }
      break;
  }
  Free(url);
}
