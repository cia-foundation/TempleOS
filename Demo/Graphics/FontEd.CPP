/*After making a font...

You can save it as a binary file with:
	FileWrite("filename.BIN.Z",text.font,256*FONT_HEIGHT);

You can load it with:
	U64 *my_font=FileRead("filename.BIN.Z");
	text.aux_font=my_font;

<CTRL-ALT-f> will toggle main font and aux_font.

If you want to change the system font permanently,
save to a file with this font editor program
and cut and paste the code into $LK,"::/Kernel/FontStd.CPP"$.
You will need to recompile Kernel by calling $LK,"MakeOSDrvBootInstall",A="MN:MakeOSDrvBootInstall"$().

See $LK,"::/Demo/ExtendedChars.CPP"$, $LK,"::/Demo/Games/CharDemo.CPP"$,
$LK,"::/Demo/Graphics/CharAnimation.CPP"$ and $LK,"::/Demo/ScreenCodes.CPP"$.
*/

#define BLOW_UP_CHAR_X	(18*FONT_WIDTH)
#define BLOW_UP_CHAR_Y	(4*FONT_HEIGHT)

U8 cur_ch;

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,k,c;
  TextPrint(task,0,0,BLUE<<4+YELLOW,"Press <CTRL-ALT-f> to Toggle Aux Font.");
  k=0;
  for (i=0;i<16;i++)
    for (j=0;j<16;j++) {
      if (k==cur_ch) {
	if (Blink)
	  c=(BLUE<<4+YELLOW)<<8 + k++;
	else
	  c=(YELLOW<<4+BLUE)<<8 + k++;
      } else
	c=(BLUE<<4+WHITE)<<8 + k++;
      TextChar(task,,j,i+2,c);
    }

  k=0;
  for (i=0;i<FONT_HEIGHT;i++)
    for (j=0;j<FONT_WIDTH;j++) {
      if (Bt(&text.font[cur_ch],k++))
	dc->color=YELLOW;
      else
	dc->color=BLUE;
      GrRect(dc,BLOW_UP_CHAR_X+j*FONT_WIDTH,
	    BLOW_UP_CHAR_Y+i*FONT_HEIGHT,
	    FONT_WIDTH,FONT_HEIGHT);
    }
}

U0 FESave(Bool pmt)
{
  U8 old_draw_it=Fs->draw_it;
  CDoc *doc=DocNew;
  I64 i;
  for (i=0;i<256;i++) {
    DocPrint(doc,"0x%016X,",text.font[i]);
    if (Bt(chars_bmp_safe_dollar,i))
      DocPrint(doc,"//%c",i);
    else if (i=='$$')
      DocPrint(doc,"//$$$$",i);
    DocPrint(doc,"\n");
  }
  Fs->draw_it=NULL;
  DocWrite(doc,pmt);
  Fs->draw_it=old_draw_it;
  DocDel(doc);
}

U0 FontEd()
{
  I64 msg_code,a1,a2,k;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  MenuPush(
	"File {"
	"  SaveAs(,CH_CTRLA);"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}");
  AutoComplete;
  DocCursor;
  DocClear;
  Fs->win_inhibit|=WIG_DBL_CLICK;
  cur_ch=0;
  try {
    Fs->draw_it=&DrawIt;
    while (TRUE) {
      switch (msg_code=GetMsg(&a1,&a2,
	    1<<MSG_KEY_DOWN|1<<MSG_IP_L_DOWN|1<<MSG_IP_R_DOWN|1<<MSG_IP_MOVE)) {
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case 0:
	      switch (a2.u8[0]) {
		case SC_CURSOR_LEFT:
		  cur_ch--;
		  break;
		case SC_CURSOR_RIGHT:
		  cur_ch++;
		  break;
		case SC_CURSOR_UP:
		  cur_ch-=16;
		  break;
		case SC_CURSOR_DOWN:
		  cur_ch+=16;
		  break;

	      }
	      break;
	      goto fe_done;
	    case CH_CTRLA:
	      FESave(TRUE);
	      break;
	    case CH_ESC:
	      FESave(FALSE);
	    case CH_SHIFT_ESC:
	      goto fe_done;
	    default:
	      cur_ch=a1;
	  }
	  break;
	case MSG_IP_L_DOWN:
	case MSG_IP_R_DOWN:
	  if (0<=a1<FONT_WIDTH*16 && FONT_HEIGHT*2<=a2<FONT_HEIGHT*16) {
	    cur_ch=(a2/FONT_HEIGHT-2)*16+a1/FONT_WIDTH;
	    break;
	  } //fall through
	case MSG_IP_MOVE:
	  k=((a2-BLOW_UP_CHAR_Y)/FONT_HEIGHT)*FONT_WIDTH+
		(a1-BLOW_UP_CHAR_X)/FONT_WIDTH;
	  if (0<=k<FONT_WIDTH*FONT_HEIGHT) {
	    if (ip.lb||msg_code==MSG_IP_L_DOWN)
	      Bts(&text.font[cur_ch],k);
	    if (ip.rb||msg_code==MSG_IP_R_DOWN)
	      Btr(&text.font[cur_ch],k);
	  }
	  break;
      }
    }
fe_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  MenuPop;
  SettingsPop;
}

FontEd;
