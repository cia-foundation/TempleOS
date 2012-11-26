//This is demonstrates symmetry.
//Right-click and draw line of symmetry.
//If the line of symmetry is a point, symmetry is turned-off.

U0 SymmetryDemo()
{
  I64 msg_code,x1,y1,x2,y2,p1,p2;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->win_inhibit=WIF_ALL-WIF_BORDER;

  WinMax;
  DCFill;
  "$$CL$$";
  dc->flags|=DCF_SYMMETRY;
  GrSetSymmetry(dc,Fs->win_pixel_width/2,0,Fs->win_pixel_width/2,1);
  do {
    msg_code=GetMsg(&p1,&p2,
      1<<MSG_KEY_DOWN+1<<MSG_IP_L_DOWN+
      1<<MSG_IP_R_DOWN);
    switch (msg_code) {
      case MSG_IP_R_DOWN:
	dc->flags&=~DCF_SYMMETRY;
	dc->color=ROP_XOR+RED^COLOR_TRANSPARENT;
	x1=p1; y1=p2;
	x2=p1; y2=p2;
	while (msg_code!=MSG_IP_R_UP) {
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&p1,&p2,
	    1<<MSG_IP_R_UP+1<<MSG_IP_MOVE);
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  x2=p1; y2=p2;
	}
	GrLine3(dc,x1,y1,0,x2,y2,0);
	if (GrSetSymmetry3(dc,x1,y1,0,x2,y2,0,x2,y2,1))
	  dc->flags|=DCF_SYMMETRY;
	break;
      case MSG_IP_L_DOWN:
	dc->color=ROP_XOR+BLACK^COLOR_TRANSPARENT;
	x1=p1; y1=p2;
	x2=p1; y2=p2;
	while (msg_code!=MSG_IP_L_UP) {
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  msg_code=GetMsg(&p1,&p2,
	    1<<MSG_IP_L_UP+1<<MSG_IP_MOVE);
	  GrLine3(dc,x1,y1,0,x2,y2,0);
	  x2=p1; y2=p2;
	}
	GrLine3(dc,x1,y1,0,x2,y2,0);
	break;
      case MSG_KEY_DOWN:
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || !p1);
  DCFill;
  DCDel(dc);
  SettingsPop;
}

SymmetryDemo;  //Execute when #included
