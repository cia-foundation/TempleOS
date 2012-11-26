U0 Convert()
{
  I64 *ptr;

//Normally, allocate large objects on the heap
//because stk size can't grow. 256 bytes, however,
//is not that bad.
  U8 byte_bit_cnts[256];

  I64 i,j,row,col,ch,best_ch,score,best_score,cur_char_image,diff_image;
  CDC *dc=DCAlias;

//Set-up a table telling how many 1 bits in a val.
  MemSet(byte_bit_cnts,0,sizeof(byte_bit_cnts));
  for (i=0;i<256;i++)
    for (j=0;j<7;j++)
      if (Bt(&i,j))
	byte_bit_cnts[i]++;

  for (row=0;row<Fs->win_height;row++) {
    for (col=0;col<Fs->win_width;col++) {


//The size of one character is 8x8 pixels
//which fits in a 64-bit var. We'll
//fetch the pixels from the screen using $LK,"GrPeek","MN:GrPeek"$().
      cur_char_image=0;
      for (i=0;i<FONT_HEIGHT;i++)
	for (j=0;j<FONT_WIDTH;j++)
//$LK,"GrPeek","MN:GrPeek"$() returns color or -1 if covered or off-screen.
	  if (GrPeek(dc,col*FONT_WIDTH+j,row*FONT_HEIGHT+i)!=COLOR_TRANSPARENT)
	    LBts(&cur_char_image,i*8+j);

      best_score=MAX_I64;
      best_ch=0;
      ptr=&gr_font[32*FONT_HEIGHT];
      for (ch=32;ch<127;ch++) {
	diff_image=*ptr++ ^ cur_char_image;
	score=0;
	for (i=0;i<8;i++)
	  score+=byte_bit_cnts[diff_image.u8[i]];
	if (score<best_score) {
	  best_score=score;
	  best_ch=ch;
	}
      }
      if (best_ch=='$$')
	"$$$$";
      else
	'' best_ch;
    }
    NewLine;
  }
  DCDel(dc);
}

public U0 Pic2ASCII()
{
  I64 msg_code,cnt,x1,y1,x2,y2,p1,p2;
  CDC *dc=DCAlias;
  dc->color=ROP_EQU+BLACK;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->win_inhibit=WIF_ALL-WIF_BORDER;

  WinMax;
  DCFill;
  "$$CL$$Draw a picture and hit\na key to convert to ASCII.\n";
  do {
    msg_code=GetMsg(&p1,&p2,
      1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN);
    switch (msg_code) {
      case MSG_IP_L_DOWN:
	x1=p1; y1=p2;
	x2=p1; y2=p2;
	cnt=0;
	while (msg_code!=MSG_IP_L_UP) {
	  dc->pen_width=0.04*ip_speed;
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&p1,&p2,
	    1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	  x1=x2; y1=y2;
	  x2=p1; y2=p2;
	}
	GrLine3(dc,x1,y1,0,x2,y2,0);
	break;
      case MSG_KEY_DOWN:
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || !p1);

//We clear the screen text and turn-off word-wrap
  "$$CL$$$$WW,0$$";

  Convert;
  DCFill;
  DCDel(dc);
  SettingsPop;
}

Pic2ASCII;  //Execute when #included
