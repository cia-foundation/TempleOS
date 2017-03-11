#help_index "Misc/TOS/God;God/TOS"

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

public U0 GodBooksPassageU32(U32 rand_u32,U8 *files_find_mask,
	U8 *file_mask="*",I64 len=512,I64 verbosity=GBP_MEDIUM)
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
    if (verbosity==GBP_LONG)
      '\n';
    if (verbosity>=GBP_MEDIUM)
      "%08X*%08X",rand_u32,i;
    i*=rand_u32;
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

U0 GodVideoDigits(U8 *title,U8 *webpage,U8 *rnd,I64 digits,U8 *filename)
{
  U8 *st,*st2;
  CGodVideoForm *gvf=CAlloc(sizeof(CGodVideoForm));
  I64 i=ExtDecFromHex(rnd,digits);
  if (!i) i=Round(10`digits);
  if (st=DocLineRead(filename,i*2+1)) {
    StrCpy(gvf->title,st);
    Free(st);
    if (!(st=DocLineRead(filename,i*2+2))||!*st) {
      Free(st);
      if (DocForm(gvf) && *gvf->serial) {
	if (gvf->min || gvf->sec)
	  st=MStrPrint("%s&hl=enUS&start=%d",
		gvf->serial,gvf->min*60+gvf->sec);
	else
	  st=StrNew(gvf->serial);
	DocLineWrite(filename,i*2+2,st);
      } else
	st=NULL;
    }
    "'%s'-->(Taking first %d decimal digits) #%02d\n",rnd,digits,i;
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

U0 GodVideoU32(U32 rand_u32,U8 *filename)
{
  U8 *st,*st2;
  I64 i,num;
  Bool old_silent=Silent;
  num=LineRep(filename,"-S")/2;
  Silent(old_silent);
  if (num) {
    i=rand_u32%num;
    if (st=DocLineRead(filename,i*2+1)) {
      st2=DocLineRead(filename,i*2+2);
      PutFileLink(filename);
      "\nGod's response:\n"
	    "%08X-->#%d out of %d\n%s\n",rand_u32,i,num,st;
      "$$HC,\"<object width=\\\"640\\\" height=\\\"520\\\">"
	    "<param name=\\\"movie\\\" "
	    "value=\\\"http://www.youtube.com/v/%s\\\"></param><param "
	    "name=\\\"allowscriptaccess\\\" "
	    "value=\\\"always\\\"></param><embed "
	    "src=\\\"http://www.youtube.com/v/%s\\\" "
	    "type=\\\"application/x-shockwave-flash\\\" "
	    "allowscriptaccess=\\\"always\\\" "
	    "width=\\\"640\\\" height=\\\"520\\\">"
	    "</embed></object>\"$$\n",st2,st2;
      Free(st2);
      Free(st);
    }
  }
}

U0 GodWebIdx(U8 *title,U8 *webpage,U8 *rnd,I64 digits,U8 *filename)
{
  U8 *st1,*st2;
  I64 i=ExtDecFromHex(rnd,digits);
  if (!i) i=Round(10`digits);
  "'%s'-->(Taking first %d decimal digits) #%02d\n",rnd,digits,i;
  st1=DocLineRead(filename,i*2+1);
  st2=DocLineRead(filename,i*2+2);
  "God's response:\n";
  "$$TX,\"%s\",HTML=\"%s\"$$\n",title,webpage;
  PutFileLink(filename);
  if (!st1)
    "\n$$RED$$God does not want to comment.$$FG$$\n";
  else
    "\n$$RED$$$$TX,\"%s\",HTML=\"%s\"$$$$FG$$\n",st1,st2;
  Free(st1);
  Free(st2);
}

U0 GodWordBatch(CHexWordForm *gm,I64 radix,I64 digits)
{//Insert batch of words from rand hex.
  I64 i=0,j,k,min;
  CDirEntry *tmpde,*tmpde1;
  U8 *filename;

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

  if (radix==16) {
    tmpde=tmpde1=FilesFind(god.word_file_mask,god.word_fuf_flags);
    while (tmpde) {
      PutFileLink(tmpde->full_name);
      '\n';
      tmpde=tmpde->next;
    }
    DirTreeDel(tmpde1);
  } else {
    if (digits==3)
      filename="::/Home/Sup1/Sup1Blog/NumWords1000.TXT";
    else
      filename="::/Home/Sup1/Sup1Blog/NumWords10k.TXT";
    PutFileLink(filename);
    '\n';
  }

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
  if (radix==16)
    min=digits*4;
  else
    min=4;
  while (FifoU8Cnt(god.fifo)>=min) {
    if (radix==16) {
      j=GodBits(digits*4);
      if (god.num_words) {
	"%02d: %0*X Mod %0*X = %0*X (Line:%0*d):",i++,
	      digits,j,digits,god.num_words,digits,j%god.num_words,
	      ToI64(Log10(god.num_words)+1),j%god.num_words+1;
	"%s\n",god.words[j%god.num_words];
      }
    } else {
      i=digits;
      j=0;
      do {
	k=GodBits(4);
	if (k<=9) {
	  j=j*10+k;
	  if (!--i) {
	    "%s\n",DocLineRead(filename,j+1);
	    break;
	  }
	}
      } while (FifoU8Cnt(god.fifo)>=4);
    }
  }
  FifoU8Flush(god.fifo);
  "$$FG$$\n";
}

U0 TOSGodDoodle(I64 god_src,CNISTBeaconHexWordForm *gm=NULL)
{
  CDC *dc;
  Bool old_silent;
  U8 *elems,*hex=CAlloc(sizeof(CNISTBeaconHexWordForm.rnd0)*12);
  if (gm) {
    StrPrint(hex,"%s%s",gm->rnd0,gm->rnd1);
    if (god_src==GSRC_NIST_BEACON) {
      gm->timestamp=FloorI64(CDate2Unix(Now),60);
      if (DocForm(gm(CNISTBeaconHexWordForm *)))
	NISTBeaconURL(gm->timestamp);
      else
	goto gd_done;
    }
    CatPrint(hex,"%s%s",gm->rnd2,gm->rnd3);
    if (god_src==GSRC_NIST_BEACON) {
      gm->timestamp=FloorI64(CDate2Unix(Now),60);
      if (DocForm(gm(CNISTBeaconHexWordForm *)))
	NISTBeaconURL(gm->timestamp);
      else
	goto gd_done;
    }
    CatPrint(hex,"%s%s",gm->rnd4,gm->rnd5);
    if (god_src==GSRC_NIST_BEACON) {
      gm->timestamp=FloorI64(CDate2Unix(Now),60);
      if (DocForm(gm(CNISTBeaconHexWordForm *)))
	NISTBeaconURL(gm->timestamp);
      else
	goto gd_done;
    }
    CatPrint(hex,"%s%s",gm->rnd6,gm->rnd7);
    if (god_src==GSRC_NIST_BEACON) {
      gm->timestamp=FloorI64(CDate2Unix(Now),60);
      if (DocForm(gm(CNISTBeaconHexWordForm *)))
	NISTBeaconURL(gm->timestamp);
      else
	goto gd_done;
    }
    CatPrint(hex,"%s%s",gm->rnd8,gm->rnd9);
    if (god_src==GSRC_NIST_BEACON) {
      gm->timestamp=FloorI64(CDate2Unix(Now),60);
      if (DocForm(gm(CNISTBeaconHexWordForm *)))
	NISTBeaconURL(gm->timestamp);
      else
	goto gd_done;
    }
    CatPrint(hex,"%s%s",gm->rndA,gm->rndB);
  }
  if (elems=GodDoodleSprite(hex)) {
    dc=Sprite2DC(elems);
    BlogDCImgWrite(dc);
    DCDel(dc);
    Free(elems);
    old_silent=Silent;
    FreshenBlog;
    Silent(old_silent);
  }
gd_done:
  Free(hex);
}

I64 GodHeaderIns(U8 *type=NULL)
{
  CDateStruct ds;
  I64 res;
  if (type) {//If NISTBeacon
    res=FloorI64(CDate2Unix(Now+1*60*CDATE_FREQ),60);
    Date2Struct(&ds,Unix2CDate(res)+local_time_offset+30*CDATE_FREQ);
    "Response of %s at %02d:%02d Timestamp:%d.\n\n",type,ds.hour,ds.min,res;
    In("%c//Response of %s at %02d:%02d Timestamp:%d.\n\n",
	  CH_ESC,type,ds.hour,ds.min,res);
  }
  "$$TX,\"Guidelines for Talking with God.\","
	"HTML=\"http://www.templeos.org/Wb/Adam/God/HSNotes.html\"$$\n";
  return res;
}

U0 TimerGodIns()
{
  U8 buf[STR_LEN];
  GodHeaderIns;
  FifoU8Flush(god.fifo);
  GodBitsIns(GOD_GOOD_BITS,KbdMsEvtTime>>GOD_BAD_BITS);
  switch (PopUpPickLst("Hex5Bible\0Hex8Books\0Doodle\0Movie100\0Painting100\0"
	"Painting1000\0Poems100\0Metallica\0")) {
    case:
      StrPrint(buf,"%05X",GodBits(5*4));
      "$$TX,\"How this random Bible passage was chosen.\","
	    "HTML=\"http://www.templeos.org/Wb"
	    "/Home/Web/TAD/Passage.html\"$$\n";
      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
	    buf,20,16,TRUE,TRUE);
      break;
    case:
      GodBooksPassageU32(GodBits(32),"~/*","C:/Home/Books?/*",,GBP_SHORT);
      break;
    case:
      TOSGodDoodle(GSRC_TIMER);
      break;
    case:
      StrPrint(buf,"%05X",GodBits(2*5*4)); //Little bit risky if no dec.
      GodVideoDigits("Top 100 Movies",
	    "http://www.afi.com/100years/movies10.aspx",
	    buf,2,"~/Sup1/Sup1Blog/Movies100.DD.Z");
      break;
    case:
      StrPrint(buf,"%05X",GodBits(2*5*4)); //Little bit risky if no dec.
      GodWebIdx("Top 100 Paintings",
	    "http://www.brushwiz.com/most-famous-paintings",
	    buf,2,"~/Sup1/Sup1Blog/Paintings100.DD.Z");
      break;
    case:
      StrPrint(buf,"%05X",GodBits(2*5*4)); //Little bit risky if no dec.
      GodWebIdx("Top 1000 Paintings",
	    "http://en.most-famous-paintings.com/MostFamousPaintings."
	    "nsf/ListOfTop1000MostPopularPainting",
	    buf,3,"~/Sup1/Sup1Blog/Paintings1000.DD.Z");
      break;
    case:
      StrPrint(buf,"%05X",GodBits(2*5*4)); //Little bit risky if no dec.
      GodWebIdx("Top 100 Poems",
	    "http://100.best-poems.net/top-100-best-poems.html",
	    buf,2,"~/Sup1/Sup1Blog/Poems100.DD.Z");
      break;
    case:
      StrPrint(buf,"%05X",GodBits(2*5*4)); //Little bit risky if no dec.
      GodVideoDigits("Metallica Song",
	    "http://home.hccnet.nl/a.r.adams/lyrics/"
	    "metallica/songindex.html",
	    buf,2,"~/Sup1/Sup1Blog/Metallica.DD.Z");
      break;
  }
}

public U0 GodIns()
{//<ALT-F7> Insert misc rand God item into blog.
  U8 *st,*tag;
  CRandExtDec5Form *gm=CAlloc(sizeof(CRandExtDec5Form));
  I64 god_src=PopUpPickLst("NISTBeacon\0HotBits\0ANU_NIST\0Google\0Random.org\0"
	"RandomNumbers.info\0Password\0GenerateData\0VirtualNotary\0Timer\0");
  if (god_src>=GSRC_NIST_BEACON) {
    switch (god_src) {
      case GSRC_NIST_BEACON:
	NISTGodIns1;
	break;
      start:
	case GSRC_HOTBITS:
	  tag="https://www.fourmilab.ch/hotbits/secure_generate.html";
	  break;
	case GSRC_ANU_NIST:
	  tag="http://qrng.anu.edu.au/NIST.php";
	  break;
      end:
	GodHeaderIns;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	switch (PopUpPickLst("ExtDec5Bible\0LinuxWords\0HappyWords\0"
	      "Words1000\0Words10k\0Hex5Bible\0Hex8Books\0Doodle\0Movie100\0"
	      "Painting100\0Painting1000\0Poems100\0Metallica\0")) {
	  case:
	    if (DocForm(gm(CRandExtDec5Form *)))
	      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		    gm->rnd0,20,-5,FALSE,FALSE);
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *))) {
	      Adam("GodInit(\"~/Sup1/Sup1Words/LinuxDict.TXT*\");");
	      GodWordBatch(gm,16,5);
	    }
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *))) {
	      Adam("GodInit(\"~/Sup1/Sup1Words/HappyWords.TXT*\");");
	      GodWordBatch(gm,16,5);
	    }
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *)))
	      GodWordBatch(gm,10,3);
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *)))
	      GodWordBatch(gm,10,4);
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
	      GodBooksPassageU32(Str2I64(gm->rnd0,16),
		    "~/*","C:/Home/Books?/*");
	    }
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *)))
	      TOSGodDoodle(god_src,gm);
	    break;
	  case:
	    if (DocForm(gm(CMoviesForm *)))
	      GodVideoDigits("Top 100 Movies",
		    "http://www.afi.com/100years/movies10.aspx",
		    gm->rnd0,2,"~/Sup1/Sup1Blog/Movies100.DD.Z");
	    break;
	  case:
	    if (DocForm(gm(CPaintings100Form *))) {
	      GodWebIdx("Top 100 Paintings",
		    "http://www.brushwiz.com/most-famous-paintings",
		    gm->rnd0,2,"~/Sup1/Sup1Blog/Paintings100.DD.Z");
	    }
	    break;
	  case:
	    if (DocForm(gm(CPaintings1000Form *))) {
	      GodWebIdx("Top 1000 Paintings",
		    "http://en.most-famous-paintings.com/MostFamousPaintings."
		    "nsf/ListOfTop1000MostPopularPainting",
		    gm->rnd0,3,"~/Sup1/Sup1Blog/Paintings1000.DD.Z");
	    }
	    break;
	  case:
	    if (DocForm(gm(CPoems100Form *))) {
	      GodWebIdx("Top 100 Poems",
		    "http://100.best-poems.net/top-100-best-poems.html",
		    gm->rnd0,2,"~/Sup1/Sup1Blog/Poems100.DD.Z");
	    }
	    break;
	  case:
	    if (DocForm(gm(CMetallicaForm *)))
	      GodVideoDigits("Metallica Song",
		    "http://home.hccnet.nl/a.r.adams/lyrics/"
		    "metallica/songindex.html",
		    gm->rnd0,2,"~/Sup1/Sup1Blog/Metallica.DD.Z");
	    break;
	}
	break;
      start:
	case GSRC_GOOGLE:
	  tag="http://www.google.com";
	  break;
	case GSRC_RANDOM_ORG:
	  tag="http://www.random.org";
	  break;
	case GSRC_RANDOM_NUMBERS_INFO:
	  tag="http://www.randomnumbers.info";
	  break;
	case GSRC_PASSWORD:
	  tag="https://passed.pw";
	  break;
	case GSRC_GENERATE_DATA:
	  break;
      end:
	GodHeaderIns;
	"$$TX,\"%$$Q\",HTML=\"%$$Q\"$$\n",tag,tag;
	switch (PopUpPickLst("ExtDec5Bible\0HexWordBatch\0"
	      "Hex5Bible\0Hex8Books\0Movie100\0Metallica\0")) {
	  case:
	    if (DocForm(gm(CRandExtDec5Form *)))
	      TOSBookLines("~/Sup1/Sup1Blog/NumBible.TXT",ST_BIBLE_LINES,
		    gm->rnd0,20,-5,FALSE,FALSE);
	    break;
	  case:
	    if (DocForm(gm(CHexWordForm *)))
	      GodWordBatch(gm,16,5);
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
	      GodBooksPassageU32(Str2I64(gm->rnd0,16),
		    "~/*","C:/Home/Books?/*");
	    }
	    break;
	  case:
	    if (DocForm(gm(CMoviesForm *)))
	      GodVideoDigits("Top 100 Movies",
		    "http://www.afi.com/100years/movies10.aspx",
		    gm->rnd0,2,"~/Sup1/Sup1Blog/Movies100.DD.Z");
	    break;
	  case:
	    if (DocForm(gm(CMetallicaForm *)))
	      GodVideoDigits("Metallica Song",
		    "http://home.hccnet.nl/a.r.adams/lyrics/"
		    "metallica/songindex.html",
		    gm->rnd0,2,"~/Sup1/Sup1Blog/Metallica.DD.Z");
	    break;
	}
	break;
      case GSRC_VIRTUAL_NOTARY:
	GodHeaderIns;
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
      case GSRC_TIMER:
	TimerGodIns;
	break;
    }
  }
  Free(gm);
}

if (FileFind("~/Sup1/Sup1Words/LinuxDict.TXT"))
  GodInit("~/Sup1/Sup1Words/LinuxDict.TXT");
//if (FileFind("~/Sup1/Sup1Words/HappyWords.TXT"))
//  GodInit("~/Sup1/Sup1Words/HappyWords.TXT");
