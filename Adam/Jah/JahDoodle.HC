#help_index "Jah;Graphics/Sprite;Sprites"

U0 JahDoodleDraw(CTask *task,CDC *dc)
{
  GrBlot(dc,0,0,jah.doodle_dc);
  if (Blink) {
    if (jah.doodle_done) {
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

U0 JahDoodleSmooth(I64 num)
{
  CDC *dc=DCExt(jah.doodle_dc,0,0,
	jah.doodle_dc->width-1,jah.doodle_dc->height-1);
  I64 i,x,y,x1,y1,c,histogram[16],best,best_cnt,c_old=jah.doodle_dc->color;
  for (y=0;y<jah.doodle_dc->height;y++)
    for (x=0;x<jah.doodle_dc->width;x++) {
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
      jah.doodle_dc->color=best;
      GrPlot(jah.doodle_dc,x,y);
    }
  jah.doodle_dc->color=c_old;
  DCDel(dc);
}

U0 JahDoodleBitsIns(I64 num_bits,I64 n)
{//Insert bits into Jah doodle bit fifo.
  I64 i;
  for (i=0;i<num_bits;i++) {
    FifoU8Ins(jah.doodle_fifo,n&1);
    n>>=1;
  }
}

U0 JahDoodleHexIns(U8 *st)
{//Insert hex record into Jah doodle bit fifo.
  U8 buf[2];
  if (st) {
    buf[1]=0;
    while (*buf=*st++)
      if (Bt(char_bmp_hex_numeric,*buf))
	JahDoodleBitsIns(4,rev_bits_table[Str2I64(buf,16)]>>4);
  }
}

I64 JahDoodleBits(I64 num_bits)
{
  U8 b;
  I64 res=0;
  while (num_bits) {
    if (FifoU8Rem(jah.doodle_fifo,&b)) {
      res=res<<1+b;
      num_bits--;
    } else {
      jah.doodle_ch=GetChar(,FALSE);
      if (jah.doodle_ch==CH_ESC||jah.doodle_ch==CH_SHIFT_ESC)
	throw;
      else if (jah.doodle_ch=='\n') {
	DCFill(jah.doodle_dc,WHITE);
	FifoU8Flush(jah.doodle_fifo);
      } else if ('0'<=jah.doodle_ch<='9')
	JahDoodleSmooth(jah.doodle_ch-'0');
      else
	JahDoodleBitsIns(JAH_GOOD_BITS,KbdMsEvtTime>>JAH_BAD_BITS);
    }
  }
  return res;
}

public U8 *JahDoodleSprite(U8 *hex=NULL)
{//Make Jah draw sprite. $LK+PU,"Jah Spirit Instructions",A="FI:::/Adam/Jah/JSNotes.DD"$
  I64 i,j,w,h,x,y,ch;
  U8 *elems;

  if (jah.doodle_dc) return NULL;
  jah.doodle_done=FALSE;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;

  if (!hex)
    PopUpOk("The $LK+PU,"Jah Spirit",A="FI:::/Adam/Jah/JSNotes.DD"$ can puppet you.\n\n"
	  "Press $$GREEN$$<SPACE>$$FG$$ until it finishes.");

  jah.doodle_ch=0;
  jah.doodle_dc=DCNew(Fs->pix_width,Fs->pix_height);
  DCFill(jah.doodle_dc,WHITE);
  w=jah.doodle_dc->width;
  h=jah.doodle_dc->height;

  Fs->draw_it=&JahDoodleDraw;
  FifoU8Flush(jah.doodle_fifo);
  JahDoodleHexIns(hex);
  try {
    for (i=0;i<3;i++) {
      jah.doodle_dc->color=RED;
      for (j=0;j<29;j++)
	switch [JahDoodleBits(3)] {
	  case 0:
	    GrEllipse3(jah.doodle_dc,
		  (w-1)*JahDoodleBits(5)/15.5-w/2,
		  (h-1)*JahDoodleBits(5)/15.5-h/2,0,
		  (w-1)*JahDoodleBits(5)/15.5,(h-1)*JahDoodleBits(5)/15.5);
	    break;
	  case 1:
	    GrCircle3(jah.doodle_dc,
		  (w-1)*JahDoodleBits(5)/15.5-w/2,
		  (h-1)*JahDoodleBits(5)/15.5-h/2,0,
		  (w-1)*JahDoodleBits(5)/15.5);
	    break;
	  case 2:
	    GrBorder(jah.doodle_dc,
		  (w-1)*JahDoodleBits(5)/15.5-w/2,
		  (h-1)*JahDoodleBits(5)/15.5-h/2,
		  (w-1)*JahDoodleBits(5)/15.5,(h-1)*JahDoodleBits(5)/15.5);
	    break;
	  case 3...7:
	    GrLine3(jah.doodle_dc,
		  (w-1)*JahDoodleBits(4)/15,(h-1)*JahDoodleBits(4)/15,0,
		  (w-1)*JahDoodleBits(4)/15,(h-1)*JahDoodleBits(4)/15,0);
	    break;
	}
      for (j=0;j<6;j++) {
	x=(w-1)*JahDoodleBits(5)/31+w/64;
	y=(h-1)*JahDoodleBits(5)/31+h/64;
	switch [JahDoodleBits(2)] {
	  case 0: jah.doodle_dc->color=BLACK;	break;
	  case 1: jah.doodle_dc->color=DKGRAY;	break;
	  case 2: jah.doodle_dc->color=LTGRAY;	break;
	  case 3: jah.doodle_dc->color=WHITE;	break;
	}
	GrFloodFill3(jah.doodle_dc,x,y,0);
      }
      JahDoodleSmooth(3);
    }
    jah.doodle_done=TRUE;
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
    elems=DC2Sprite(jah.doodle_dc);
  else
    elems=NULL;
  DCDel(jah.doodle_dc);
  jah.doodle_dc=NULL;
  return elems;
}

#help_index "Jah"
public U0 JahDoodle(U8 *hex=NULL)
{//Make Jah draw sprite, insert in doc. $LK+PU,"Jah Spirit Instructions",A="FI:::/Adam/Jah/JSNotes.DD"$
  U8 *elems;
  if (elems=JahDoodleSprite(hex)) {
    Sprite(elems);
    Free(elems);
  }
}
