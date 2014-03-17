#help_index "God;Graphics/Sprite"
Bool god_doodle_done;
I64 god_doodle_ch;
CDC *god_doodle_dc=NULL;

U0 GodDoodleDraw(CTask *task,CDC *dc)
{
  GrBlot(dc,0,0,god_doodle_dc);
  if (Blink) {
    if (god_doodle_done) {
      dc->color=RED;
      GrPrint(dc,(task->pix_width-FONT_WIDTH*27)>>1,(task->pix_height-FONT_HEIGHT)>>1,
	"Press <ESC> or <SHIFT-ESC>.");
    } else {
      dc->color=GREEN;
      GrPrint(dc,(task->pix_width-FONT_WIDTH*25)>>1,(task->pix_height-FONT_HEIGHT)>>1,
	"Press <SPACE> repeatedly.");
    }
  }
}

U0 GodDoodleSmooth(I64 num)
{
  CDC *dc=DCExtract(god_doodle_dc,0,0,god_doodle_dc->width-1,god_doodle_dc->height-1);
  I64 i,x,y,x1,y1,c,histogram[16],best,best_cnt,c_old=god_doodle_dc->color;
  for (y=0;y<god_doodle_dc->height;y++)
    for (x=0;x<god_doodle_dc->width;x++) {
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
      god_doodle_dc->color=best;
      GrPlot(god_doodle_dc,x,y);
    }
  god_doodle_dc->color=c_old;
  DCDel(dc);
}

CFifoU8 *god_doodle_fifo=FifoU8New(32);
I64 GodDoodleBits(I64 num_bits)
{
  U8 b;
  I64 i,k,result=0;
  while (num_bits) {
    if (FifoU8Rem(god_doodle_fifo,&b)) {
      result=result<<1+b;
      num_bits--;
    } else {
      god_doodle_ch=GetChar(,FALSE);
      if (god_doodle_ch==CH_ESC||god_doodle_ch==CH_SHIFT_ESC)
	throw;
      else if (god_doodle_ch=='\n') {
	DCFill(god_doodle_dc,WHITE);
	FifoU8Flush(god_doodle_fifo);
      } else if ('0'<=god_doodle_ch<='9')
	GodDoodleSmooth(god_doodle_ch-'0');
      else {
	k=KbdMouseEvtTime>>GOD_BAD_BITS;
	for (i=0;i<GOD_GOOD_BITS;i++) {
	  FifoU8Ins(god_doodle_fifo,k&1);
	  k>>=1;
	}
      }
    }
  }
  return result;
}

public U8 *GodDoodleSprite()
{// $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.TXT"$
  I64 i,j,w,h,x,y,ch;
  U8 *elems;

  if (god_doodle_dc) return NULL;
  god_doodle_done=FALSE;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  WinMax;
  WinBorder;
  WordStat;

  PopUpOk("The $LK+PU,"Holy Spirit",A="FI:::/Adam/God/HSNotes.TXT"$ can puppet you.\n\n"
	"Press $$GREEN$$<SPACE>$$FG$$ until it finishes.");

  god_doodle_ch=0;
  god_doodle_dc=DCNew(Fs->pix_width,Fs->pix_height);
  DCFill(god_doodle_dc,WHITE);
  w=god_doodle_dc->width;
  h=god_doodle_dc->height;

  Fs->draw_it=&GodDoodleDraw;
  FifoU8Flush(god_doodle_fifo);
  try {
    for (i=0;i<3;i++) {
      god_doodle_dc->color=RED;
      for (j=0;j<24;j++)
	nobound_switch (GodDoodleBits(3)) {
	  case 0:
	    GrEllipse(god_doodle_dc,
	      (w-1)*GodDoodleBits(5)/15.5-w/2,(h-1)*GodDoodleBits(5)/15.5-h/2,
	      (w-1)*GodDoodleBits(5)/15.5,(h-1)*GodDoodleBits(5)/15.5);
	    break;
	  case 1:
	    GrCircle(god_doodle_dc,
	      (w-1)*GodDoodleBits(5)/15.5-w/2,(h-1)*GodDoodleBits(5)/15.5-h/2,
	      (w-1)*GodDoodleBits(5)/15.5);
	    break;
	  case 2:
	    GrLineRect4(god_doodle_dc,
	      (w-1)*GodDoodleBits(5)/15.5-w/2,(h-1)*GodDoodleBits(5)/15.5-h/2,
	      (w-1)*GodDoodleBits(5)/15.5,(h-1)*GodDoodleBits(5)/15.5);
	    break;
	  case 3...7:
	    GrLine(god_doodle_dc,
	      (w-1)*GodDoodleBits(4)/15,(h-1)*GodDoodleBits(4)/15,
	      (w-1)*GodDoodleBits(4)/15,(h-1)*GodDoodleBits(4)/15);
	    break;
	}
      for (j=4;j;) {
	x=(w-1)*GodDoodleBits(5)/31+w/64;
	y=(h-1)*GodDoodleBits(5)/31+h/64;
	switch(GrPeek(god_doodle_dc,x,y)) {
	  case BLACK:
	    god_doodle_dc->color=WHITE;
	    GrFloodFill(god_doodle_dc,x,y);
	    j--;
	    break;
	  case WHITE:
	    god_doodle_dc->color=BLACK;
	    GrFloodFill(god_doodle_dc,x,y);
	    j--;
	    break;
	}
      }
      GodDoodleSmooth(3);
    }
    god_doodle_done=TRUE;
    do ch=GetChar(,FALSE);
    while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  } catch {
    Fs->catch_except=TRUE;
    ch=CH_SHIFT_ESC;
  }
  DCFill;
  SettingsPop;
  if (ch==CH_ESC)
    elems=DC2Sprite(god_doodle_dc);
  else
    elems=NULL;
  DCDel(god_doodle_dc);
  god_doodle_dc=NULL;
  return elems;
}

#help_index "God"
public U0 GodDoodle()
{// $LK+PU,"Holy Spirit Instructions",A="FI:::/Adam/God/HSNotes.TXT"$
  U8 *elems;
  if (elems=GodDoodleSprite) {
    Sprite(elems);
    Free(elems);
  }
}
