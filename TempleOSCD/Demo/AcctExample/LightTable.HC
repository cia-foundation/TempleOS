#define PTS_NUM	1024

U0 LightTable()
{
  I64 msg_code,i,x1,y1,x2,y2,arg1,arg2;
  CDC *dc=DCAlias;
  CD3I32 *c=MAlloc(PTS_NUM*sizeof(CD3I32));

  MenuPush(
	"File {"
	"  Restart(,'\n');"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Edit {"
	"  Fill(,CH_SPACE);"
	"}"
	);
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-
	WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU;
  Fs->text_attr=BLACK<<4+WHITE;	//Current $LK,"CTask",A="MN:CTask"$ is Fs segment register.
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  try {
    while (TRUE) {
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+1<<MSG_MS_R_UP);
lt_restart:
      switch (msg_code) {
	case MSG_MS_R_UP:
	  x1=arg1; y1=arg2;
	  x2=arg1; y2=arg2;
	  dc->color=ROP_XOR+LTRED^TRANSPARENT;
	  do {
	    GrLine3(dc,x1,y1,0,x2,y2,0);
	    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
	    GrLine3(dc,x1,y1,0,x2,y2,0);
	    x2=arg1; y2=arg2;
	  } while (msg_code!=MSG_MS_L_UP);
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  break;
	case MSG_MS_L_DOWN:
	  i=0;
	  dc->color=ROP_XOR+WHITE^TRANSPARENT;
	  do {
	    c[i].x=arg1; c[i].y=arg2; c[i].z=0;
	    Gr2BSpline(dc,c,i+1);
	    msg_code=GetMsg(&arg1,&arg2,
		  1<<MSG_KEY_DOWN+1<<MSG_MS_L_UP+1<<MSG_MS_MOVE+1<<MSG_MS_R_UP);
	    Gr2BSpline(dc,c,i+1);
	    if (msg_code==MSG_KEY_DOWN)
	      goto lt_restart;
	    else if (msg_code==MSG_MS_L_UP) {
	      Noise(100,30,50);
	      i++;
	    }
	  } while (i<PTS_NUM-1 && msg_code!=MSG_MS_R_UP);
	  Gr2BSpline(dc,c,i);
	  break;
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case CH_SPACE:
	      GrFloodFill(dc,ms.pos.x-Fs->pix_left,ms.pos.y-Fs->pix_top);
	      break;
	    case '\n':
	      DCFill;
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto lt_done;
	  }
	  break;
      }
    }
lt_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  DCFill;
  Free(c);
  DCDel(dc);
  SettingsPop;
  MenuPop;
}

LightTable;
