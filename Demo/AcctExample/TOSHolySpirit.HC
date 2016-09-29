#help_index "TOS"

U8 *URLPercentSpaces(U8 *src)
{
  U8 buf[2048],*dst=buf;
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

U8 *URLPlusSpaces(U8 *src)
{
  U8 buf[2048],*dst=buf;
  while (*src) {
    if (*src==CH_SPACE) {
      *dst++='+';
      src++;
    } else
      *dst++=*src++;
  }
  *dst=0;
  return StrNew(buf);
}

I64 ExtDecFromHex(U8 *st,I64 digits)
{
  I64 res=0,ch;
  while (digits) {
    if (!(ch=*st++))
      break;
    if ('0'<=ch<='9') {
      res=10*res+ch-'0';
      digits--;
    }
  }
  return res;
}

U0 TOSBibleLines(U8 *st,I64 num_lines,I64 radix,Bool modulo,Bool plus_one)
{//Radix<0 means extract N dec digits from hex.
  CDoc *doc_out,*doc_in;
  CDocEntry *doc_e;
  I64 i,start;
  U8 *verse;
  if (*st && (doc_out=DocPut)) {
    if (radix<0)
      i=ExtDecFromHex(st,-radix);
    else
      i=Str2I64(st,radix);
    if (radix<0)
      DocPrint(doc_out,"'%s'-->(Taking first %d decimal digits)",st,-radix);
    else if (radix==16)
      DocPrint(doc_out,"0x%s-->",st);
    if (modulo) {
      DocPrint(doc_out,"(%d remainder when divided by %d)",
	    i,ST_BIBLE_LINES-(num_lines-1));
      start=i%(ST_BIBLE_LINES-(num_lines-1));
    } else {
      if (radix>=0 || plus_one)
	DocPrint(doc_out,"%d",i);
      start=i;
    }
    if (plus_one) {
      start++;
      DocPrint(doc_out,"+1");
    }
    DocPrint(doc_out,"-->Line:%d\n",start);

    verse=BibleLine2Verse(start);
    DocPrint(doc_out,"$TX,"KingJamesBible",HTML="http://www.templeos.org/Wb/Home/Sup1/Sup1Blog/NumBible.TXT"$ (%s)\n\nGod says...\n$$RED$$",verse);
    Free(verse);

    doc_in=DocNew;
    BibleLines(doc_in,start,num_lines,"::/Home/Sup1/Sup1Blog/NumBible.TXT");
    while (TRUE) {//Del leading blanks
      doc_e=doc_in->head.next;
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

#define GBP_SHORT	0
#define GBP_MEDIUM	1
#define GBP_LONG	2

public U0 GodBooksPassage(U8 *files_find_mask,I64 len=512,
	I64 rand_i32=MIN_I64,I64 verbosity=GBP_SHORT)
{//Make God pick a book passage. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  CDirEntry *tmpde=FilesFind(files_find_mask,
	FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT),
	*tmpde1=tmpde;
  I64 i=0;
  U8 *st,*st2=NULL;
  while (tmpde) {
    st=FileRead(tmpde->full_name);
    tmpde->user_data=StrLen(st);
    if (verbosity==GBP_LONG)
      "%08X-",i;
    if (tmpde->user_data>=len)
      i+=tmpde->user_data+1-len;
    if (verbosity==GBP_LONG)
      "%08X:$$LK,\"%s\",A=\"FI:%s\"$$\n",
	    i-1,tmpde->full_name+2,tmpde->full_name;
    Free(st);
    tmpde=tmpde->next;
  }
  if (rand_i32<0)
    rand_i32=GodBits(32);
  if (verbosity==GBP_LONG)
    '\n';
  if (verbosity>=GBP_MEDIUM)
    "%08X*%08X",rand_i32,i;
  i*=rand_i32;
  if (verbosity>=GBP_MEDIUM)
    "=%08X.%08X\n",i.u32[1],i.u32[0];
  i=i.u32[1];
  tmpde=tmpde1;
  while (tmpde) {
    if (!st2 && tmpde->user_data>=len) {
      i-=tmpde->user_data+1-len;
      if (i<0) {
	st=FileRead(tmpde->full_name);
	st2=st+(tmpde->user_data+1-len)+i;
	st2[len]=0;
	"%s\nGod says...\n$$RED$$%s$$FG$$\n",tmpde->full_name,st2;
	Free(st);
      }
    }
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);
}

U8 *GodVideoRead(U8 *filename,I64 line)
{
  U8 *res=NULL;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (DocLineNumGoTo(doc,line-1) && doc->cur_entry->type_u8==DOCT_TEXT)
    res=StrNew(doc->cur_entry->tag);
  DocDel(doc);
  return res;
}

U8 *GodVideoWrite(U8 *filename,I64 line,U8 *st)
{
  U8 *res=NULL;
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
  return res;
}

class CGodVideoForm
{
  U8	title[STR_LEN]	fmtstr "$$DA-P,A=\"Title:%s\"$$\n";
  U8	serial[STR_LEN] fmtstr "$$DA-P-TRM,LEN=11,A=\"SerialNum:%11s\"$$\n";
  I64	min		fmtstr "$$DA,A=\"Minutes:%d\"$$\n";
  I64	sec		fmtstr "$$DA,A=\"Seconds:%d\"$$\n";
};

U0 GodVideo(U8 *title,U8 *webpage,U8 *rnd,I64 digits,U8 *filename)
{
  U8 *st,*st2;
  CGodVideoForm *gvf=CAlloc(sizeof(CGodVideoForm));
  I64 i=ExtDecFromHex(rnd,digits);
  if (st=GodVideoRead(filename,i*2)) {
    StrCpy(gvf->title,st);
    Free(st);
    if (!(st=GodVideoRead(filename,i*2+1))||!*st) {
      Free(st);
      if (DocForm(gvf) && *gvf->serial) {
	if (gvf->min || gvf->sec)
	  st=MStrPrint("%s&hl=enUS&start=%d",
		gvf->serial,gvf->min*60+gvf->sec);
	else
	  st=StrNew(gvf->serial);
	GodVideoWrite(filename,i*2+1,st);
      } else
	st=NULL;
    }
    if (StrIMatch("Metallica",filename)) {
      st2=URLPlusSpaces(gvf->title+3);
      "God's response:\n"
	    "$$TX,\"%s\",HTML=\"%s\"$$\n"
	    "$$TX,\"%s (Lyrics)\","
	    "HTML=\"http://lmgtfy.com/?q=Metallica+lyrics+%s\"$$\n",
	    title,webpage,gvf->title,st2;
      Free(st2);
    } else
      "God's response:\n"
	    "$$TX,\"%s\",HTML=\"%s\"$$\n"
	    "#%s\n",title,webpage,gvf->title;
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
  Free(gvf);
}

class CRandExtDec5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"5 Decimal Digits from Hex:%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CNISTBeaconRandExtDec5Form
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"5 Decimal Digits from Hex:%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CHexWordForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#0:%s\"$$\n";
  U8	rnd1[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#1:%s\"$$\n";
  U8	rnd2[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#2:%s\"$$\n";
  U8	rnd3[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#3:%s\"$$\n";
  I64	cert;
};

class CNISTBeaconHexWordForm
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#0:%s\"$$\n";
  U8	rnd1[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#1:%s\"$$\n";
  U8	rnd2[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#2:%s\"$$\n";
  U8	rnd3[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#3:%s\"$$\n";
  I64	cert;
};

class CRandHex5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P-TRM,LEN=5,"
	"A=\"Line (5-Digit Hex):%5s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CNISTBeaconRandHex5Form
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P-TRM,LEN=5,"
	"A=\"Line (5-Digit Hex):%5s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CRandHex8Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P-TRM,LEN=8,"
	"A=\"Line (8-Digit Hex):%8s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CNISTBeaconRandHex8Form
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P-TRM,LEN=8,"
	"A=\"Line (8-Digit Hex):%8s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CMoviesForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Movie #1-100 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CNISTMoviesForm
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Movie #1-100 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CMetallicaForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Song #1-99 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CNISTMetallicaForm
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Song #1-99 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert;
};

class CCertRandDec5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Line (Dec):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  I64	cert		fmtstr "$$DA,A=\"Certificate:%d\"$$\n";
};

U0 GodWordBatch(CHexWordForm *url)
{
  I64 i=0;
  CDirEntry *tmpde,*tmpde1;

  FifoU8Flush(god.fifo);
  GodHexIns(url->rnd0);
  GodHexIns(url->rnd1);
  GodHexIns(url->rnd2);
  GodHexIns(url->rnd3);
  tmpde=tmpde1=FilesFind(god.word_file_mask,god.word_fuf_flags);
  while (tmpde) {
    PutFileLink(tmpde->full_name);
    '\n';
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);

  "\n$$RED$$";
  if (*url->rnd0)
    "%s\n",url->rnd0;
  if (*url->rnd1)
    "%s\n",url->rnd1;
  if (*url->rnd2)
    "%s\n",url->rnd2;
  if (*url->rnd3)
    "%s\n",url->rnd3;
  '\n';
  while (FifoU8Cnt(god.fifo)>20) {
     GodBits(3);
     "%02d: ",i++;
     GodWord(TRUE);
  }
  FifoU8Flush(god.fifo);
  "$$FG$$\n";
}

#define NIST_TIME_OFFSET	(-7998-local_time_offset/CDATE_FREQ)
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

U0 NISTBeaconURL(I64 timestamp)
{
  U8 *tag;
  CDateStruct ds;
  CDate dt=(timestamp-NIST_TIME_OFFSET)*CDATE_FREQ+Str2Date("1/1/1970");
  Date2Struct(&ds,dt+local_time_offset+30*CDATE_FREQ);
  "$$TX,\"NIST Beacon Date:%D Time:%02d:%02d Unix TimeStamp:%u\","
	"HTML=\"https://beacon.nist.gov/home\"$$\n",dt,ds.hour,ds.min,timestamp;
  tag=MStrPrint("https:/beacon.nist.gov/rest/record/%d",timestamp);
  "$$TX,\"NIST Beacon Record:%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
  Free(tag);
}


U0 GodMisc()
{
  U8 *st,*tag;
  CRandExtDec5Form *gm=CAlloc(sizeof(CRandExtDec5Form));
  I64 i=PopUpPickLst("NISTBeacon\0HotBits\0Random.org\0RandomNumbers.info\0"
	"ANU_NIST\0Password\0GenerateData\0VirtualNotary\0Timer\0");
  if (i>=0) {
    "$$TX,\"Guidelines for Talking with God.\","
	  "HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
    switch (i) {
      case:
	gm->timestamp=FloorI64((Now-Str2Date("1/1/1970"))/CDATE_FREQ+
	      NIST_TIME_OFFSET-NIST_TIME_TO_SWITCH,60);
	switch (PopUpPickLst("ExtDec5Bible\0HexWordBatch\0Hex5Bible\0"
	      "Hex8Books\0Movie\0Metallica\0")) {
	  case:
	    if (DocForm(gm(CNISTBeaconRandExtDec5Form *))) {
	      NISTBeaconURL(gm->timestamp);
	      TOSBibleLines(gm->rnd0,20,-5,FALSE,FALSE);
	    }
	    break;
	  case:
	    if (DocForm(gm(CNISTBeaconHexWordForm *))) {
	      NISTBeaconURL(gm->timestamp);
	      GodWordBatch(gm);
	    }
	    break;
	  case:
	    if (DocForm(gm(CNISTBeaconRandHex5Form *))) {
	      "$$TX,\"How this random Bible passage was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/NISTPassage.html\"$$\n";
	      NISTBeaconURL(gm->timestamp);
	      TOSBibleLines(gm->rnd0,20,16,TRUE,TRUE);
	    }
	    break;
	  case:
	    if (DocForm(gm(CNISTBeaconRandHex8Form *))) {
	      "$$TX,\"How this random book pick was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/NISTPick001.html\"$$\n";
	      NISTBeaconURL(gm->timestamp);
	      GodBooksPassage("C:/Home/Sup1/Sup1Texts/*",,
		    Str2I64(gm->rnd0,16),GBP_MEDIUM);
	    }
	    break;
	  case:
	    if (DocForm(gm(CNISTMoviesForm *))) {
	      NISTBeaconURL(gm->timestamp);
	      GodVideo("Top 100 Movies",
		    "http://www.afi.com/100years/movies10.aspx",
		    gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Movies.DD.Z");
	    }
	    break;
	  case:
	    if (DocForm(gm(CNISTMetallicaForm *))) {
	      NISTBeaconURL(gm->timestamp);
	      GodVideo("Metallica Song",
		    "http://home.hccnet.nl/a.r.adams/lyrics/"
		    "metallica/songindex.html",
		    gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Metallica.DD.Z");
	    }
	    break;
	}
	break;
      start:
	case:
	  tag="https://www.fourmilab.ch/hotbits/secure_generate.html";
	  break;
	case:
	  tag="http://www.random.org";
	  break;
	case:
	  tag="http://www.randomnumbers.info";
	  break;
	case:
	  tag="http://qrng.anu.edu.au/NIST.php";
	  break;
	case:
	  tag="https://passed.pw";
	  break;
	case:
	  tag="http://www.generatedata.com";
	  break;
      end:
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	switch (PopUpPickLst("ExtDec5Bible\0HexWordBatch\0Hex5Bible\0"
	      "Hex8Books\0Movie\0Metallica\0")) {
	  case:
	    if (DocForm(gm(CRandExtDec5Form *)))
	      TOSBibleLines(gm->rnd0,20,-5,FALSE,FALSE);
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *)))
	      GodWordBatch(gm);
	    break;
	  case:
	    if (DocForm(gm(CRandHex5Form *))) {
	      "$$TX,\"How this random Bible passage was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/Passage.html\"$$\n";
	      TOSBibleLines(gm->rnd0,20,16,TRUE,TRUE);
	    }
	    break;
	  case:
	    if (DocForm(gm(CRandHex8Form *))) {
	      "$$TX,\"How this random book pick was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/Pick001.html\"$$\n";
	      GodBooksPassage("C:/Home/Sup1/Sup1Texts/*",,
		    Str2I64(gm->rnd0,16),GBP_MEDIUM);
	    }
	    break;
	  case:
	    if (DocForm(gm(CMoviesForm *)))
	      GodVideo("Top 100 Movies",
		    "http://www.afi.com/100years/movies10.aspx",
		    gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Movies.DD.Z");
	    break;
	  case:
	    if (DocForm(gm(CMetallicaForm *)))
	      GodVideo("Metallica Song",
		    "http://home.hccnet.nl/a.r.adams/lyrics/"
		    "metallica/songindex.html",
		    gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Metallica.DD.Z");
	    break;
	}
	break;
      case:
	if (DocForm(gm(CCertRandDec5Form *))) {
	  tag="http://virtual-notary.org";
	  "$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	  tag="Here is the certificate";
	  st=MStrPrint("http://www.templeos.org/Certs/virtual-notary-cert"
		"-randomnum-%d.p12",gm->cert);
	  "\n$$TX,\"%$$Q: %d\",HTML=\"%$$Q\"$$.\n",tag,gm->cert,st;
	  Free(st);
	  TOSBibleLines(gm->rnd0,20,-5,FALSE,FALSE);
	}
	break;
      case:
	FifoU8Flush(god.fifo);
	GodBitsIns(GOD_GOOD_BITS,KbdMouseEvtTime>>GOD_BAD_BITS);
	GodBooksPassage("C:/Home/Sup1/Sup1Texts/*");
	break;
    }
  }
  Free(gm);
}

U8 *YouTubeGet(U8 *filename="C:/Home/Sup1/Sup1Blog/YouTube.DD.Z",U8 *needle)
{
  I64 i=0,line=MIN_I64;
  CDoc	*doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR),
	*doc2=DocNew;
  U8 *title,*code,*res;
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

U0 YouTubeAdd(U8 *filename="C:/Home/Sup1/Sup1Blog/YouTube.DD.Z",
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

class CWebBibleForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P,A=\"Bible Passage:%s\"$$\n";
  I64	min;
  I64	sec;
};

class CYouTubeForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Title:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P-TRM,LEN=11,A=\"SerialNum:%11s\"$$\n";
  I64	min		fmtstr "$$DA,A=\"Minutes:%d\"$$\n";
  I64	sec		fmtstr "$$DA,A=\"Seconds:%d\"$$\n";
};

class CImgurForm
{
  U8	tag[STR_LEN];
  U8	special[STR_LEN] fmtstr "$$DA-P,A=\"SerialNum:%s\"$$\n";
  I64	min		fmtstr "$$DA,A=\"Width    :%d\"$$\n";
  I64	sec		fmtstr "$$DA,A=\"Height   :%d\"$$\n";
};

class CWikipediaForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN] fmtstr "$$DA-P,A=\"Index   :%s\"$$\n";
  I64	min;
  I64	sec;
};

class CWebBookMarkForm
{
  U8	tag[STR_LEN]	fmtstr "$$DA-P,A=\"Tag Text:%s\"$$\n";
  U8	special[STR_LEN];
  I64	min;
  I64	sec;
};

U0 TOSInsMisc()
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

U0 GodCodeJmp()
{//$LK,"::/Adam/God/HSNotes.DD"$
  CDirEntry *tmpde1=FilesFind("/*",
	FUF_JUST_FILES|FUF_RECURSE|FUF_JUST_SRC|FUF_CLUSTER_ORDER),
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
  GodBitsIns(GOD_GOOD_BITS,KbdMouseEvtTime>>GOD_BAD_BITS);
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

if (FileFind("~/Sup1/Sup1Words/LinuxDict.TXT"))
  GodInit("~/Sup1/Sup1Words/LinuxDict.TXT");
//if (FileFind("~/Sup1/Sup1Words/HappyWords.TXT"))
//  GodInit("~/Sup1/Sup1Words/HappyWords.TXT");
