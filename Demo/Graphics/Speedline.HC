//This is a drawing program

public U0 SpeedLineDemo()
{
  I64 msg_code,cnt,x1,y1,x2,y2,a1,a2,color=BLACK;
  CDC *dc=DCAlias;
  dc->color=color;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;

  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  DCFill;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+1<<MSG_IP_R_UP);
    switch (msg_code) {
      case MSG_IP_R_UP:
	color=PopUpColor;
	if (color>=0)
	  dc->color=color;
	break;
      case MSG_IP_L_DOWN:
	x1=a1; y1=a2;
	x2=a1; y2=a2;
	cnt=0;
	while (msg_code!=MSG_IP_L_UP) {
	  dc->pen_width=0.04*ip.speed;
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&a1,&a2,
		1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	  x1=x2; y1=y2;
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

SpeedLineDemo;	//Execute when #included
