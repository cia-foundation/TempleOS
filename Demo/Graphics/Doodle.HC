//This is a drawing program

U0 Doodle()
{
  I64 msg_code,i,x1,y1,x2,y2,arg1,arg2,color=BLACK;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;

  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  DCFill;
  do {
    msg_code=GetMsg(&arg1,&arg2,
	  1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN+1<<MSG_MS_R_UP);
    switch (msg_code) {
      case MSG_MS_R_UP:
	i=PopUpColor;
	if (i>=0) color=i;
	break;
      case MSG_MS_L_DOWN:
	x1=arg1; y1=arg2;
	x2=arg1; y2=arg2;
	dc->color=ROP_XOR+color^TRANSPARENT;
	dc->thick=7;
	do {
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  x2=arg1; y2=arg2;
	} while (msg_code!=MSG_MS_L_UP);
	GrLine3(dc,x1,y1,0,x2,y2,0);
	break;
      case MSG_KEY_DOWN:
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || !arg1);
  GetMsg(,,1<<MSG_KEY_UP);
  DCFill;
  DCDel(dc);
  SettingsPop;
}

Doodle;  //Execute when #included
