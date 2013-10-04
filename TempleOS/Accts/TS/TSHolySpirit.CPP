#define TS_HS_BAD_BITS		4
#define GODLE_HS_BAD_BITS	4
#define GODLE_HS_GOOD_BITS	24

Bool godle_done;
I64 godle_ch;
CDC *main_dc;
U0 GodleDraw(CTask *task,CDC *dc)
{
  GrBlot(dc,0,0,main_dc);
  if (!(win_updates&7)) {
    dc->color=LTRED;
    if (godle_done)
      GrPrint(dc,(task->win_pixel_width-FONT_WIDTH*4)>>1,(task->win_pixel_height-FONT_HEIGHT)>>1,
	"Done");
  }
}

U0 GodleSmooth(I64 num)
{
  CDC *dc=DCExtract(main_dc,0,0,main_dc->width-1,main_dc->height-1);
  I64 i,x,y,x1,y1,c,histogram[16],best,best_cnt,c_old=main_dc->color;
  for (y=0;y<main_dc->height;y++)
    for (x=0;x<main_dc->width;x++) {
      MemSet(histogram,0,sizeof(histogram));
      for (y1=y-num;y1<=y+num;y1++)
	for (x1=x-num;x1<=x+num;x1++) {
	  c=GrPeek(dc,x1,y1);
	  if (0<=c<=15)
	    histogram[c]++;
	}
      best=BLACK;
      best_cnt=-1;
      for (i=0;i<16;i++)
	if (histogram[i]>best_cnt) {
	  best=i;
	  best_cnt=histogram[i];
	}
      main_dc->color=best;
      GrPlot(main_dc,x,y);
    }
  main_dc->color=c_old;
  DCDel(dc);
}

CFifoU8 *godle_timer_pick_fifo=FifoU8New(32);
U0 GodleTimerPickFlush()
{
  FifoU8Flush(godle_timer_pick_fifo);
}

I64 GodleTimerPickedBits(I64 num_bits)
{
  U8 b;
  I64 i,k,result=0;
  while (num_bits) {
    if (FifoU8Rem(godle_timer_pick_fifo,&b)) {
      result=result<<1+b;
      num_bits--;
    } else {
      godle_ch=GetChar(,FALSE);
      if (godle_ch==CH_ESC||godle_ch==CH_SHIFT_ESC)
	throw;
      else if (godle_ch=='\n') {
	DCFill(main_dc,WHITE);
	FifoU8Flush(godle_timer_pick_fifo);
      } else if ('0'<=godle_ch<='9')
	GodleSmooth(godle_ch-'0');
      else {
	k=KbdMouseEvtTime>>GODLE_HS_BAD_BITS;
	for (i=0;i<GODLE_HS_GOOD_BITS;i++) {
	  FifoU8Ins(godle_timer_pick_fifo,k&1);
	  k>>=1;
	}
      }
    }
  }
  return result;
}

U0 RandQuote()
{ //$LK-A,"::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$
  I64 i,size,l=512;
  U8 *bible_filename=Define("ST_BIBLE_FILE"),
     *buf=FileRead(bible_filename,&size);
  if (buf) {
    i=KbdMouseEvtTime>>TS_HS_BAD_BITS%(size-l);
    buf[i+l]=0;
    "%s",buf+i;
    Free(buf);
  }
}

U0 RandWord()
{
  "%s ",ws_sorted_words[KbdMouseEvtTime>>TS_HS_BAD_BITS%ws_num_words]->str;
}

U0 BadCodeJump()
{ //$LK-A,"::/TempleOS/Apps/HolySpirit/HSNotes.TXT"$
  CDirEntry *root=FilesFind("/TempleOS/*",FUF_JUST_FILES|FUF_RECURSE|FUF_JUST_SRC),
	*tempdee=root;
  I64 cnt=0,num;
  CDoc *doc;
  U8 *st;
  while (tempdee) {
    doc=DocRead(tempdee->full_name);
    tempdee->user_data=doc->root.y;
    cnt+=doc->root.y;
    DocDel(doc);
    tempdee=tempdee->next;
  }
  num=KbdMouseEvtTime>>TS_HS_BAD_BITS%cnt;
  tempdee=root;
  while (tempdee) {
    num-=tempdee->user_data;
    if (num<0) {
      st=MPrint("FL:%s,%d",tempdee->full_name,-num);
      break;
    }
    tempdee=tempdee->next;
  }
  DirLstDel(root);
  Ed(st);
  Free(st);
}

U0 Godle()
{
  I64 i,j,w,h,x,y,ch;
  godle_done=FALSE;
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinMax;
  WinBorder;
  WordStat;

  WinMgrSync;
  godle_ch=0;
  main_dc=DCNew(DCT_COLOR8,Fs->win_pixel_width,Fs->win_pixel_height);
  DCFill(main_dc,WHITE);
  w=main_dc->width;
  h=main_dc->height;

  Fs->draw_it=&GodleDraw;
  GodleTimerPickFlush;
  try {
    for (i=0;i<3;i++) {
      main_dc->color=RED;
      for (j=0;j<24;j++)
	GrLine(main_dc,
	      (w-1)*GodleTimerPickedBits(4)/15,(h-1)*GodleTimerPickedBits(4)/15,
	      (w-1)*GodleTimerPickedBits(4)/15,(h-1)*GodleTimerPickedBits(4)/15);
      for (j=4;j;) {
	x=(w-1)*GodleTimerPickedBits(5)/31+w/64;
	y=(h-1)*GodleTimerPickedBits(5)/31+h/64;
	switch(GrPeek(main_dc,x,y)) {
	  case BLACK:
	    main_dc->color=WHITE;
	    GrFloodFill(main_dc,x,y);
	    j--;
	    break;
	  case WHITE:
	    main_dc->color=BLACK;
	    GrFloodFill(main_dc,x,y);
	    j--;
	    break;
	}
      }
      GodleSmooth(3);
    }
    godle_done=TRUE;
    do ch=GetChar(,FALSE);
    while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  } catch
    Fs->catch_except=TRUE;
  DCFill;
  SettingsPop;
  DCDel(main_dc);
}

class CMyURL
{
  U8	tag[136]	fmtstr "$$DA-P,135,\"Tag Text:%s\"$$\n";
  U8	special[136]	fmtstr "$$DA-P,135,\"Special :%s\"$$\n";
  I64	min		fmtstr "$$DA,32, \"Min     :%d\"$$\n";
  I64	sec		fmtstr "$$DA,32, \"Sec     :%d\"$$\n";
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

U0 FillInMyURL()
{
  CMyURL *url=CAlloc(sizeof(CMyURL));
  U8 *tag,*st,*st2;
  I64 i=PopUpPickLst("Web Bible\0YouTube\0Wikipedia\0Cert\0"
	"RandomNumbers.info\0Random.org\0Virtual Notary\0Nist\0HotBits\0");
  if (i>=0 && DocFormDo(url)) {
    switch (i) {
      case 0:
	tag=url->special;
	st2=URLPercentSpaces(url->special);
	st=MPrint("http://www.biblegateway.com/passage/?search=%s&version=NIV",st2);
	Free(st2);
	if (*url->tag)
	  tag=url->tag;
	"$$TX+HTML,\"%Q\",\"%Q\"$$",tag,st;
	Free(st);
	break;
      case 1:
	tag="YouTube Video";
	if (url->min || url->sec)
	  st=MPrint("http://www.youtube.com/watch?v=%s&t=%dm%ds",url->special,url->min,url->sec);
	else
	  st=MPrint("http://www.youtube.com/watch?v=%s",url->special);
	if (*url->tag)
	  tag=url->tag;
	"$$TX+HTML,\"%Q\",\"%Q\"$$",tag,st;
	Free(st);
	break;
      case 2:
	tag=url->special;
	st=MPrint("http://en.wikipedia.org/wiki/%s",url->special);
	if (*url->tag)
	  tag=url->tag;
	"$$TX+HTML,\"%Q\",\"%Q\"$$",tag,st;
	Free(st);
	break;
      case 3:
	tag="Here is the certificate";
	st=MPrint("http://www.templeos.org/Certs/virtual-notary-cert-randomnum-%s.p12",url->special);
	if (*url->tag)
	  tag=url->tag;
	"$$TX+HTML,\"%Q\",\"%Q\"$$",tag,st;
	Free(st);
	break;
      case 4:
	tag="http://www.randomnumbers.info";
	"$$TX+HTML,\"%Q\",\"%Q\"$$\n",tag,tag;
	if (*url->special) {
	  i=A2I(url->special);
	  "\nLine:%d\nGod says...\n$$FG,RED$$",i;
	  PutBibleLines(i%100000,20);
	  "$$FG$$\n";
	}
	break;
      case 5:
	tag="http://www.random.org";
	"$$TX+HTML,\"%Q\",\"%Q\"$$\n",tag,tag;
	if (*url->special) {
	  i=A2I(url->special);
	  "\nLine:%d\nGod says...\n$$FG,RED$$",i;
	  PutBibleLines(i%100000,20);
	  "$$FG$$\n";
	}
	break;
      case 6:
	tag="http://virtual-notary.org";
	"$$TX+HTML,\"%Q\",\"%Q\"$$\n",tag,tag;
	if (*url->special) {
	  i=A2I(url->special);
	  "\nLine:%d\nGod says...\n$$FG,RED$$",i;
	  PutBibleLines(i%100000,20);
	  "$$FG$$\n";
	}
	break;
      case 7:
	tag="http://qrng.anu.edu.au/NIST.php";
	"$$TX+HTML,\"%Q\",\"%Q\"$$\n",tag,tag;
	if (*url->special) {
	  i=A2I(url->special,16);
	  "\n0x%s-->%d mod 100000-->Line:%05d\n"
	  "God says...\n$$FG,RED$$",url->special,i,i%100000;
	  PutBibleLines(i%100000,20);
	  "$$FG$$\n";
	}
	break;
      case 8:
	tag="https://www.fourmilab.ch/hotbits/secure_generate.html";
	"$$TX+HTML,\"%Q\",\"%Q\"$$\n",tag,tag;
	if (*url->special) {
	  i=A2I(url->special,16);
	  "\n0x%s-->%d mod 100000-->Line:%05d\n"
	  "God says...\n$$FG,RED$$",url->special,i,i%100000;
	  PutBibleLines(i%100000,20);
	  "$$FG$$\n";
	}
	break;
    }
  }
  Free(url);
}
