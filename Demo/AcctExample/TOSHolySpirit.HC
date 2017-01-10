#help_index "TOS"

U8 *URLPercentSpaces(U8 *src)
{
  U8 buf[2048],*dst=buf;
  while (*src) {
    if (*src==CH_SPACE) {
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

U0 TOSBookLines(U8 *book_filename,I64 book_lines,U8 *st,I64 num_lines,
	I64 radix,Bool modulo,Bool plus_one)
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
	    i,book_lines-(num_lines-1));
      start=i%(book_lines-(num_lines-1));
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

    if (StrMatch("NumBible",book_filename)) {
      verse=BibleLine2Verse(start);
      DocPrint(doc_out,"$TX,"KingJamesBible",HTML="http://www.templeos.org/Wb/Home/Sup1/Sup1Blog/NumBible.TXT"$ (%s)\n\nGod says...\n$$RED$$",verse);
      Free(verse);
    }

    doc_in=DocNew;
    BookLines(doc_in,start,num_lines,book_filename);
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

public U0 GodBooksPassage(U8 *files_find_mask,U8 *file_mask="*",I64 len=512,
	I64 rand_i32=MIN_I64,I64 verbosity=GBP_SHORT)
{//Make God pick a book passage. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  CDirEntry *tmpde=FilesFind(files_find_mask,
	FUF_RECURSE|FUF_JUST_FILES|FUF_JUST_TXT),
	*tmpde1,*tmpde2,**tmpde3;
  I64 i=0;
  U8 *st,*st2=NULL;

  tmpde3=&tmpde2;
  tmpde2=NULL;
  while (tmpde) {
    tmpde1=tmpde->next;
    if (WildMatch(tmpde->full_name,file_mask)) {
      *tmpde3=tmpde;
      tmpde3=&tmpde->next;
    } else
      DirEntryDel(tmpde);
    tmpde=tmpde1;
  }
  *tmpde3=NULL;

  if (tmpde=tmpde2) {
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

    tmpde=tmpde2;
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
    DirTreeDel(tmpde2);
  }
}

U8 *FileDblRead(U8 *filename,I64 line)
{
  U8 *res=NULL;
  CDoc *doc=DocRead(filename,DOCF_PLAIN_TEXT_TABS|DOCF_NO_CURSOR);
  if (DocLineNumGoTo(doc,line-1) && doc->cur_entry->type_u8==DOCT_TEXT)
    res=StrNew(doc->cur_entry->tag);
  DocDel(doc);
  return res;
}

U8 *FileDblWrite(U8 *filename,I64 line,U8 *st)
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
  if (!i) i=Round(10`digits);
  if (st=FileDblRead(filename,i*2)) {
    StrCpy(gvf->title,st);
    Free(st);
    if (!(st=FileDblRead(filename,i*2+1))||!*st) {
      Free(st);
      if (DocForm(gvf) && *gvf->serial) {
	if (gvf->min || gvf->sec)
	  st=MStrPrint("%s&hl=enUS&start=%d",
		gvf->serial,gvf->min*60+gvf->sec);
	else
	  st=StrNew(gvf->serial);
	FileDblWrite(filename,i*2+1,st);
      } else
	st=NULL;
    }
    "'%s'-->(Taking first 2 decimal digits) #%02d\n",rnd,i;
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

U0 GodWebIdx(U8 *title,U8 *webpage,U8 *rnd,I64 digits,U8 *filename)
{
  U8 *st1,*st2;
  I64 i=ExtDecFromHex(rnd,digits);
  if (!i) i=Round(10`digits);
  "'%s'-->(Taking first 2 decimal digits) #%02d\n",rnd,i;
  st1=FileDblRead(filename,i*2);
  st2=FileDblRead(filename,i*2+1);
  "God's response:\n";
  "$$TX,\"%s\",HTML=\"%s\"$$\n",title,webpage;
  "$$TX,\"%s\",HTML=\"%s\"$$\n",st1,st2;
  Free(st1);
  Free(st2);
}

class CRandExtDec5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"5 Decimal Digits from Hex:%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CHexWordForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#0:%s\"$$\n";
  U8	rnd1[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#1:%s\"$$\n";
  U8	rnd2[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#2:%s\"$$\n";
  U8	rnd3[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#3:%s\"$$\n";
  U8	rnd4[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#4:%s\"$$\n";
  U8	rnd5[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#5:%s\"$$\n";
  U8	rnd6[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#6:%s\"$$\n";
  U8	rnd7[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#7:%s\"$$\n";
  U8	rnd8[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#8:%s\"$$\n";
  U8	rnd9[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#9:%s\"$$\n";
  U8	rndA[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#A:%s\"$$\n";
  U8	rndB[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#B:%s\"$$\n";
  I64	cert;
};

class CNISTBeaconHexWordForm
{
  I64	timestamp	fmtstr "$$DA,A=\"TimeStamp:%d\"$$\n";
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#0:%s\"$$\n";
  U8	rnd1[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#1:%s\"$$\n";
  U8	rnd2[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#2:%s\"$$\n";
  U8	rnd3[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#3:%s\"$$\n";
  U8	rnd4[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#4:%s\"$$\n";
  U8	rnd5[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#5:%s\"$$\n";
  U8	rnd6[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#6:%s\"$$\n";
  U8	rnd7[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#7:%s\"$$\n";
  U8	rnd8[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#8:%s\"$$\n";
  U8	rnd9[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#9:%s\"$$\n";
  U8	rndA[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#A:%s\"$$\n";
  U8	rndB[512]	fmtstr "$$DA-P,A=\"Hex Word Indices#B:%s\"$$\n";
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
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
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
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
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
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CPaintingsForm
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,"
	"A=\"Painting #00-99 (2 Decimal Digits from Hex):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
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
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert;
};

class CCertRandDec5Form
{
  I64	timestamp;
  U8	rnd0[512]	fmtstr "$$DA-P,A=\"Line (Dec):%s\"$$\n";
  U8	rnd1[512];
  U8	rnd2[512];
  U8	rnd3[512];
  U8	rnd4[512];
  U8	rnd5[512];
  U8	rnd6[512];
  U8	rnd7[512];
  U8	rnd8[512];
  U8	rnd9[512];
  U8	rndA[512];
  U8	rndB[512];
  I64	cert		fmtstr "$$DA,A=\"Certificate:%d\"$$\n";
};

U0 GodWordBatch(CHexWordForm *gm)
{
  I64 i=0;
  CDirEntry *tmpde,*tmpde1;

  FifoU8Flush(god.fifo);
  GodHexIns(gm->rnd0);
  GodHexIns(gm->rnd1);
  GodHexIns(gm->rnd2);
  GodHexIns(gm->rnd3);
  GodHexIns(gm->rnd4);
  GodHexIns(gm->rnd5);
  GodHexIns(gm->rnd6);
  GodHexIns(gm->rnd7);
  GodHexIns(gm->rnd8);
  GodHexIns(gm->rnd9);
  GodHexIns(gm->rndA);
  GodHexIns(gm->rndB);
  tmpde=tmpde1=FilesFind(god.word_file_mask,god.word_fuf_flags);
  while (tmpde) {
    PutFileLink(tmpde->full_name);
    '\n';
    tmpde=tmpde->next;
  }
  DirTreeDel(tmpde1);

  "\n$$RED$$";
  if (*gm->rnd0)
    "%s\n",gm->rnd0;
  if (*gm->rnd1)
    "%s\n",gm->rnd1;
  if (*gm->rnd2)
    "%s\n",gm->rnd2;
  if (*gm->rnd3)
    "%s\n",gm->rnd3;
  if (*gm->rnd4)
    "%s\n",gm->rnd4;
  if (*gm->rnd5)
    "%s\n",gm->rnd5;
  if (*gm->rnd6)
    "%s\n",gm->rnd6;
  if (*gm->rnd7)
    "%s\n",gm->rnd7;
  if (*gm->rnd8)
    "%s\n",gm->rnd8;
  if (*gm->rnd9)
    "%s\n",gm->rnd9;
  if (*gm->rndA)
    "%s\n",gm->rndA;
  if (*gm->rndB)
    "%s\n",gm->rndB;
  '\n';
  while (FifoU8Cnt(god.fifo)>20) {
     "%02d: ",i++;
     GodWord(20,TRUE);
  }
  FifoU8Flush(god.fifo);
  "$$FG$$\n";
}

#define NIST_TIME_OFFSET	(-8015-local_time_offset/CDATE_FREQ)
#define NIST_TIME_TO_SWITCH	3

I64 CDate2Unix(CDate dt)
{
  return ToI64((dt-Str2Date("1/1/1970"))/CDATE_FREQ+NIST_TIME_OFFSET);
}

CDate Unix2CDate(I64 timestamp)
{
  return (timestamp-NIST_TIME_OFFSET)*CDATE_FREQ+Str2Date("1/1/1970");
}

public U0 NISTSync()
{
  I64 i,j;
  CDate dt;
  CDateStruct ds;
  while (!ScanChar) {
    dt=Now;
    Date2Struct(&ds,dt);
    i=CDate2Unix(dt);
    j=FloorI64(i,60);
    "%d %d %02d %02d\n",i,j,ds.sec,(60+ds.sec-(i-j))%60;
    Sleep(100);
  }
}

U0 NISTBeaconURL(I64 timestamp)
{
  U8 *tag;
  CDateStruct ds;
  CDate dt=Unix2CDate(timestamp);
  Date2Struct(&ds,dt+local_time_offset+30*CDATE_FREQ);
  "$$TX,\"NIST Beacon Date:%D Time:%02d:%02d Unix TimeStamp:%u\","
	"HTML=\"https://beacon.nist.gov/home\"$$\n",dt,ds.hour,ds.min,timestamp;
  tag=MStrPrint("https://beacon.nist.gov/rest/record/%d",timestamp);
  "$$TX,\"NIST Beacon Record:%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
  Free(tag);
}

U0 TOSGodDoodle(CNISTBeaconHexWordForm *gm,Bool NIST)
{
  CDC *dc;
  Bool old_silent;
  U8 *elems,*hex=MAlloc(sizeof(CNISTBeaconHexWordForm.rnd0)*12);

  StrPrint(hex,"%s%s",gm->rnd0,gm->rnd1);
  if (NIST) {
    gm->timestamp=FloorI64(CDate2Unix(Now),60);
    if (PopUpForm(gm(CNISTBeaconHexWordForm *)))
      NISTBeaconURL(gm->timestamp);
    else
      goto gd_done;
  }
  CatPrint(hex,"%s%s",gm->rnd2,gm->rnd3);
  if (NIST) {
    gm->timestamp=FloorI64(CDate2Unix(Now),60);
    if (PopUpForm(gm(CNISTBeaconHexWordForm *)))
      NISTBeaconURL(gm->timestamp);
    else
      goto gd_done;
  }
  CatPrint(hex,"%s%s",gm->rnd4,gm->rnd5);
  if (NIST) {
    gm->timestamp=FloorI64(CDate2Unix(Now),60);
    if (PopUpForm(gm(CNISTBeaconHexWordForm *)))
      NISTBeaconURL(gm->timestamp);
    else
      goto gd_done;
  }
  CatPrint(hex,"%s%s",gm->rnd6,gm->rnd7);
  if (NIST) {
    gm->timestamp=FloorI64(CDate2Unix(Now),60);
    if (PopUpForm(gm(CNISTBeaconHexWordForm *)))
      NISTBeaconURL(gm->timestamp);
    else
      goto gd_done;
  }
  CatPrint(hex,"%s%s",gm->rnd8,gm->rnd9);
  if (NIST) {
    gm->timestamp=FloorI64(CDate2Unix(Now),60);
    if (PopUpForm(gm(CNISTBeaconHexWordForm *)))
      NISTBeaconURL(gm->timestamp);
    else
      goto gd_done;
  }
  CatPrint(hex,"%s%s",gm->rndA,gm->rndB);
  if (elems=GodDoodleSprite(hex)) {
    dc=Sprite2DC(elems);
    BlogDCImgWrite(dc);
    DCDel(dc);
    Free(elems);
    old_silent=Silent;
    MakeWebSitePartial;
    Silent(old_silent);
  }
gd_done:
  Free(hex);
}

I64 GodMiscHeader(U8 *type=NULL)
{
  CDateStruct ds;
  I64 res;
  if (type) {
    res=FloorI64(CDate2Unix(Now+5*60*CDATE_FREQ),60);
    Date2Struct(&ds,Unix2CDate(res)+local_time_offset+30*CDATE_FREQ);
    "Response of %s at %02d:%02d Timestamp:%d.\n\n",type,ds.hour,ds.min,res;
    In("%c//Response of %s at %02d:%02d Timestamp:%d.\n\n",
	  CH_ESC,type,ds.hour,ds.min,res);
  }
  "$$TX,\"Guidelines for Talking with God.\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
  return res;
}

U0 NISTGodMisc1()
{
  I64 timestamp,type;
  type=PopUpPickLst("ExtDec5Bible\0LinuxWords\0HappyWords\0"
	"Hex5Bible\0Hex8Books\0Doodle\0Movie\0Painting\0Metallica\0");
  switch (type) {
    case: timestamp=GodMiscHeader("Bible");		break;
    case: timestamp=GodMiscHeader("LinuxWords");	break;
    case: timestamp=GodMiscHeader("HappyWords");	break;
    case: timestamp=GodMiscHeader("Bible");		break;
    case: timestamp=GodMiscHeader("BookPick");	break;
    case: timestamp=GodMiscHeader("Doodle");	break;
    case: timestamp=GodMiscHeader("Movie");		break;
    case: timestamp=GodMiscHeader("Painting");	break;
    case: timestamp=GodMiscHeader("Metallica");	break;
  }
  Once("InStr(\"\\\"Blog(FALSE);\n\\\";"
	"NISTGodMisc2(%d,%d);\");",timestamp,type);
}

U0 NISTGodMisc2(I64 timestamp,I64 type)
{
  CRandExtDec5Form *gm=CAlloc(sizeof(CRandExtDec5Form));
  gm->timestamp=timestamp;
  switch (type) {
    case:
      if (PopUpForm(gm(CRandExtDec5Form *))) {
	NISTBeaconURL(gm->timestamp);
	TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
	      gm->rnd0,20,-5,FALSE,FALSE);
      }
      break;
    case:
      if (PopUpForm(gm(CHexWordForm *))) {
	NISTBeaconURL(gm->timestamp);
	Adam("GodInit(\"~/Sup1/Sup1Words/LinuxDict.TXT*\");");
	GodWordBatch(gm);
      }
      break;
    case:
      if (PopUpForm(gm(CHexWordForm *))) {
	NISTBeaconURL(gm->timestamp);
	Adam("GodInit(\"~/Sup1/Sup1Words/HappyWords.TXT*\");");
	GodWordBatch(gm);
      }
      break;
    case:
      if (PopUpForm(gm(CRandHex5Form *))) {
	"$$TX,\"How this random Bible passage was chosen.\","
	      "HTML=\"http://www.templeos.org/Wb"
	      "/Home/Web/TAD/NISTPassage.html\"$$\n";
	NISTBeaconURL(gm->timestamp);
	TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
	      gm->rnd0,20,16,TRUE,TRUE);
      }
      break;
    case:
      if (PopUpForm(gm(CRandHex8Form *))) {
	"$$TX,\"How this random book pick was chosen.\","
	      "HTML=\"http://www.templeos.org/Wb"
	      "/Home/Web/TAD/NISTPick002.html\"$$\n";
	NISTBeaconURL(gm->timestamp);
	GodBooksPassage("C:/Home/*","C:/Home/Books?/*",,
	      Str2I64(gm->rnd0,16),GBP_MEDIUM);
      }
      break;
    case:
      if (PopUpForm(gm(CHexWordForm *))) {
	NISTBeaconURL(gm->timestamp);
	TOSGodDoodle(gm,TRUE);
      }
      break;
    case:
      if (PopUpForm(gm(CMoviesForm *))) {
	NISTBeaconURL(gm->timestamp);
	GodVideo("Top 100 Movies",
	      "http://www.afi.com/100years/movies10.aspx",
	      gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Movies.DD.Z");
      }
      break;
    case:
      if (PopUpForm(gm(CPaintingsForm *))) {
	NISTBeaconURL(gm->timestamp);
	GodWebIdx("Top 100 Paintings",
	      "http://www.brushwiz.com/most-famous-paintings",
	      gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Paintings.DD.Z");
      }
      break;
    case:
      if (PopUpForm(gm(CMetallicaForm *))) {
	NISTBeaconURL(gm->timestamp);
	GodVideo("Metallica Song",
	      "http://home.hccnet.nl/a.r.adams/lyrics/"
	      "metallica/songindex.html",
	      gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Metallica.DD.Z");
      }
      break;
  }
  Free(gm);
}

U0 GodMisc()
{
  U8 *st,*tag;
  CRandExtDec5Form *gm=CAlloc(sizeof(CRandExtDec5Form));
  I64 i=PopUpPickLst("NISTBeacon\0HotBits\0ANU_NIST\0Random.org\0"
	"RandomNumbers.info\0Password\0GenerateData\0VirtualNotary\0Timer\0");
  if (i>=0) {
    switch (i) {
      case:
	NISTGodMisc1;
	break;
      start:
	case:
	  tag="https://www.fourmilab.ch/hotbits/secure_generate.html";
	  break;
	case:
	  tag="http://qrng.anu.edu.au/NIST.php";
	  break;
      end:
	GodMiscHeader;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	switch (PopUpPickLst("ExtDec5Bible\0LinuxWords\0"
	      "HappyWords\0Hex5Bible\0Hex8Books\0Doodle\0Movie\0Painting\0"
	      "Metallica\0")) {
	  case:
	    if (DocForm(gm(CRandExtDec5Form *)))
	      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		    gm->rnd0,20,-5,FALSE,FALSE);
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *))) {
	      Adam("GodInit(\"~/Sup1/Sup1Words/LinuxDict.TXT*\");");
	      GodWordBatch(gm);
	    }
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *))) {
	      Adam("GodInit(\"~/Sup1/Sup1Words/HappyWords.TXT*\");");
	      GodWordBatch(gm);
	    }
	    break;
	  case:
	    if (DocForm(gm(CRandHex5Form *))) {
	      "$$TX,\"How this random Bible passage was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/Passage.html\"$$\n";
	      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		    gm->rnd0,20,16,TRUE,TRUE);
	    }
	    break;
	  case:
	    if (DocForm(gm(CRandHex8Form *))) {
	      "$$TX,\"How this random book pick was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/NISTPick002.html\"$$\n";
	      GodBooksPassage("C:/Home/*","C:/Home/Books?/*",,
		    Str2I64(gm->rnd0,16),GBP_MEDIUM);
	    }
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *)))
	      TOSGodDoodle(gm,FALSE);
	    break;
	  case:
	    if (DocForm(gm(CMoviesForm *)))
	      GodVideo("Top 100 Movies",
		    "http://www.afi.com/100years/movies10.aspx",
		    gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Movies.DD.Z");
	    break;
	  case:
	    if (DocForm(gm(CPaintingsForm *))) {
	      GodWebIdx("Top 100 Paintings",
		    "http://www.brushwiz.com/most-famous-paintings",
		    gm->rnd0,2,"C:/Home/Sup1/Sup1Blog/Paintings.DD.Z");
	    }
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
      start:
	case:
	  tag="http://www.random.org";
	  break;
	case:
	  tag="http://www.randomnumbers.info";
	  break;
	case:
	  tag="https://passed.pw";
	  break;
	case:
	  tag="http://www.generatedata.com";
	  break;
      end:
	GodMiscHeader;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	switch (PopUpPickLst("ExtDec5Bible\0HexWordBatch\0"
	      "Hex5Bible\0Hex8Books\0Movie\0Metallica\0")) {
	  case:
	    if (DocForm(gm(CRandExtDec5Form *)))
	      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		    gm->rnd0,20,-5,FALSE,FALSE);
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
	      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		    gm->rnd0,20,16,TRUE,TRUE);
	    }
	    break;
	  case:
	    if (DocForm(gm(CRandHex8Form *))) {
	      "$$TX,\"How this random book pick was chosen.\","
		    "HTML=\"http://www.templeos.org/Wb"
		    "/Home/Web/TAD/NISTPick002.html\"$$\n";
	      GodBooksPassage("C:/Home/*","C:/Home/Books?/*",,
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
	GodMiscHeader;
	if (DocForm(gm(CCertRandDec5Form *))) {
	  tag="http://virtual-notary.org";
	  "$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	  tag="Here is the certificate";
	  st=MStrPrint("http://www.templeos.org/Certs/virtual-notary-cert"
		"-randomnum-%d.p12",gm->cert);
	  "\n$$TX,\"%$$Q: %d\",HTML=\"%$$Q\"$$.\n",tag,gm->cert,st;
	  Free(st);
	  TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		gm->rnd0,20,-5,FALSE,FALSE);
	}
	break;
      case:
	GodMiscHeader;
	FifoU8Flush(god.fifo);
	GodBitsIns(GOD_GOOD_BITS,KbdMsEvtTime>>GOD_BAD_BITS);
	GodBooksPassage("C:/Home/*","C:/Home/Books?/*");
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

RegDft("TempleOS/TOSGodOffer","I64 tos_gus_gwen=0;\n");
RegExe("TempleOS/TOSGodOffer");

U0 GodOffer()
{
  switch (PopUpPickLst("MosesComic\0GusGwen\0Fiction\0DyingLight\0")) {
    case:
      "---- Moses Comic ---- "
	    "(Set in the $TX,"Numbers 11",HTML="http://www.biblegateway.com/passage/?search=Numbers%%%%2011&version=NIV"$ "
	    "part of the story.)\n"
	    "Moses says, \"\"\n"
	    "God says, \"\"\n";
      break;
    case:
      "---- Gus and Gwen: A soap opera Episode %d ----\n"
	    "Gus says, \"\"\n"
	    "Gwen says, \"\"\n"
	    "-----------------------------------------\n",tos_gus_gwen++;
      RegWrite("TempleOS/TOSGodOffer","I64 tos_gus_gwen=%d;\n",
	    tos_gus_gwen);
      break;
    case:
      "---- $TX,"Collaborative Fiction",HTML="http://en.wikipedia.org/wiki/Collaborative_fiction"$ ----\n"
	    "Once upon a time ";
      break;
    case:
      "---- The Dying Light: A soap opera ----\n"
	    "Dr. Hans says, \"\"\n"
	    "Nurse Ruth, \"\"\n"
	    "---------------------------------------\n";
      break;
  }
}

if (FileFind("~/Sup1/Sup1Words/LinuxDict.TXT"))
  GodInit("~/Sup1/Sup1Words/LinuxDict.TXT");
//if (FileFind("~/Sup1/Sup1Words/HappyWords.TXT"))
//  GodInit("~/Sup1/Sup1Words/HappyWords.TXT");
