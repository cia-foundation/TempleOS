#help_index "God;Graphics/Sprite;Sprites"

U0 GodDoodleDraw(CTask *task,CDC *dc)
{
  GrBlot(dc,0,0,god.doodle_dc);
  if (Blink) {
    if (god.doodle_done) {
      dc->color=RED;
      GrPrint(dc,(task->pix_width-FONT_WIDTH*29)>>1,
	    (task->pix_height-3*FONT_HEIGHT)>>1,
	    "Press <ESC> to insert sprite.");
      GrPrint(dc,(task->pix_width-FONT_WIDTH*39)>>1,
	    (task->pix_height-3*FONT_HEIGHT)>>1+2*FONT_HEIGHT,
	    "Press <SHIFT-ESC> to throw-away sprite.");
    } else {
      dc->color=GREEN;
      GrPrint(dc,(task->pix_width-FONT_WIDTH*25)>>1,
	    (task->pix_height-FONT_HEIGHT)>>1,
	    "Press <SPACE> repeatedly.");
    }
  }
}

U0 GodDoodleSmooth(I64 num)
{
  CDC *dc=DCExt(god.doodle_dc,0,0,
	god.doodle_dc->width-1,god.doodle_dc->height-1);
  I64 i,x,y,x1,y1,c,histogram[16],best,best_cnt,c_old=god.doodle_dc->color;
  for (y=0;y<god.doodle_dc->height;y++)
    for (x=0;x<god.doodle_dc->width;x++) {
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
      god.doodle_dc->color=best;
      GrPlot(god.doodle_dc,x,y);
    }
  god.doodle_dc->color=c_old;
  DCDel(dc);
}

U0 GodDoodleBitsIns(I64 num_bits,I64 n)
{//Insert bits into God doodle bit fifo.
  I64 i;
  for (i=0;i<num_bits;i++) {
    FifoU8Ins(god.doodle_fifo,n&1);
    n>>=1;
  }
}

U0 GodDoodleHexIns(U8 *st)
{//Insert hex record into God doodle bit fifo.
  U8 buf[2];
  if (st) {
    buf[1]=0;
    while (*buf=*st++)
      if (Bt(char_bmp_hex_numeric,*buf))
	GodDoodleBitsIns(4,rev_bits_table[Str2I64(buf,16)]>>4);
  }
}

I64 GodDoodleBits(I64 num_bits)
{
  U8 b;
  I64 res=0;
  while (num_bits) {
    if (FifoU8Rem(god.doodle_fifo,&b)) {
      res=res<<1+b;
      num_bits--;
    } else {
      god.doodle_ch=GetChar(,FALSE);
      if (god.doodle_ch==CH_ESC||god.doodle_ch==CH_SHIFT_ESC)
	throw;
      else if (god.doodle_ch=='\n') {
	DCFill(god.doodle_dc,WHITE);
	FifoU8Flush(god.doodle_fifo);
      } else if ('0'<=god.doodle_ch<='9')
	GodDoodleSmooth(god.doodle_ch-'0');
      else
	GodDoodleBitsIns(GOD_GOOD_BITS,KbdMsEvtTime>>GOD_BAD_BITS);
    }
  }
  return res;
}

public U8 *GodDoodleSprite(U8 *hex=NULL)
{//Make God draw sprite. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  I64 i,j,w,h,x,y,ch;
  U8 *elems;

  if (god.doodle_dc) return NULL;
  god.doodle_done=FALSE;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;

  if (!hex)
    PopUpOk("The $LK+PU,"Holy Spirit",A="FI:::/Adam/God/HSNotes.DD"$ can puppet you.\n\n"
	  "Press $$GREEN$$<SPACE>$$FG$$ until it finishes.");

  god.doodle_ch=0;
  god.doodle_dc=DCNew(Fs->pix_width,Fs->pix_height);
  DCFill(god.doodle_dc,WHITE);
  w=god.doodle_dc->width;
  h=god.doodle_dc->height;

  Fs->draw_it=&GodDoodleDraw;
  FifoU8Flush(god.doodle_fifo);
  GodDoodleHexIns(hex);
  try {
    for (i=0;i<3;i++) {
      god.doodle_dc->color=RED;
      for (j=0;j<29;j++)
	switch [GodDoodleBits(3)] {
	  case 0:
	    GrEllipse3(god.doodle_dc,
		  (w-1)*GodDoodleBits(5)/15.5-w/2,
		  (h-1)*GodDoodleBits(5)/15.5-h/2,0,
		  (w-1)*GodDoodleBits(5)/15.5,(h-1)*GodDoodleBits(5)/15.5);
	    break;
	  case 1:
	    GrCircle3(god.doodle_dc,
		  (w-1)*GodDoodleBits(5)/15.5-w/2,
		  (h-1)*GodDoodleBits(5)/15.5-h/2,0,
		  (w-1)*GodDoodleBits(5)/15.5);
	    break;
	  case 2:
	    GrBorder(god.doodle_dc,
		  (w-1)*GodDoodleBits(5)/15.5-w/2,
		  (h-1)*GodDoodleBits(5)/15.5-h/2,
		  (w-1)*GodDoodleBits(5)/15.5,(h-1)*GodDoodleBits(5)/15.5);
	    break;
	  case 3...7:
	    GrLine3(god.doodle_dc,
		  (w-1)*GodDoodleBits(4)/15,(h-1)*GodDoodleBits(4)/15,0,
		  (w-1)*GodDoodleBits(4)/15,(h-1)*GodDoodleBits(4)/15,0);
	    break;
	}
      for (j=0;j<6;j++) {
	x=(w-1)*GodDoodleBits(5)/31+w/64;
	y=(h-1)*GodDoodleBits(5)/31+h/64;
	switch [GodDoodleBits(2)] {
	  case 0: god.doodle_dc->color=BLACK;	break;
	  case 1: god.doodle_dc->color=DKGRAY;	break;
	  case 2: god.doodle_dc->color=LTGRAY;	break;
	  case 3: god.doodle_dc->color=WHITE;	break;
	}
	GrFloodFill3(god.doodle_dc,x,y,0);
      }
      GodDoodleSmooth(3);
    }
    god.doodle_done=TRUE;
    if (!hex) {
      do ch=GetChar(,FALSE);
      while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
    } else
      ch=CH_ESC;
  } catch {
    Fs->catch_except=TRUE;
    ch=CH_SHIFT_ESC;
  }
  DCFill;
  SettingsPop;
  if (ch==CH_ESC)
    elems=DC2Sprite(god.doodle_dc);
  else
    elems=NULL;
  DCDel(god.doodle_dc);
  god.doodle_dc=NULL;
  return elems;
}

#help_index "God"
public U0 GodDoodle(U8 *hex=NULL)
{//Make God draw sprite, insert in doc. $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.DD"$
  U8 *elems;
  if (elems=GodDoodleSprite(hex)) {
    Sprite(elems);
    Free(elems);
  }
}
