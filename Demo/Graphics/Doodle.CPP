//This is a drawing program

U0 Doodle()
{
  I64 msg_code,i,x1,y1,x2,y2,a1,a2,color=BLACK;
  CDC *dc=DCAlias;
 
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;

  WinMax;
  WordStat;

  DCFill;
  DocClear;
  do {
    msg_code=GetMsg(&a1,&a2,
      1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+
      1<<MSG_IP_R_UP);
    switch (msg_code) {
      case MSG_IP_R_UP:
	i=PopUpColor;
	if (i>=0) color=i;
	break;
      case MSG_IP_L_DOWN:
	x1=a1; y1=a2;
	x2=a1; y2=a2;
	dc->color=ROP_XOR+color^TRANSPARENT;
	dc->pen_width=1;
	while (msg_code!=MSG_IP_L_UP) {
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&a1,&a2,
	    1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  x2=a1; y2=a2;
	}
	GrLine3(dc,x1,y1,0,x2,y2,0);
	break;
      case MSG_KEY_DOWN:
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || !a1);
  GetMsg(,,1<<MSG_KEY_UP);
  DCFill;
  DCDel(dc);
  SettingsPop;
}

Doodle;  //Execute when #included
