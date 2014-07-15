/*Demonstrates symmetry.
Right-click and draw line of symmetry.
If the line of symmetry is a point, symmetry is turned-off.
*/

U0 SymmetryDemo()
{
  I64 msg_code,x1,y1,x2,y2,a1,a2;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;

  WinMax;
  DCFill;
  DocClear;
  dc->flags|=DCF_SYMMETRY;
  GrSetSymmetry(dc,Fs->pix_width/2,0,Fs->pix_width/2,1);
  do {
    msg_code=GetMsg(&a1,&a2,
      1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+
      1<<MSG_IP_R_DOWN);
    switch (msg_code) {
      case MSG_IP_R_DOWN:
	dc->flags&=~DCF_SYMMETRY;
	dc->color=ROP_XOR+RED^TRANSPARENT;
	x1=a1; y1=a2;
	x2=a1; y2=a2;
	while (msg_code!=MSG_IP_R_UP) {
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&a1,&a2,
	    1<<MSG_IP_R_UP+1<<MSG_IP_MOVE);
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  x2=a1; y2=a2;
	}
	GrLine3(dc,x1,y1,0,x2,y2,0);
	if (GrSetSymmetry3(dc,x1,y1,0,x2,y2,0,x2,y2,1))
	  dc->flags|=DCF_SYMMETRY;
	break;
      case MSG_IP_L_DOWN:
	dc->color=ROP_XOR+BLACK^TRANSPARENT;
	x1=a1; y1=a2;
	x2=a1; y2=a2;
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

SymmetryDemo;  //Execute when #included
