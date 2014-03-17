U0 BSplineDemo()
{
  CDC *dc=DCAlias;
  I64 p1,p2,i=0,msg_code;
  CD3I32 c[64];

  SettingsPush;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_FOCUS-WIF_BORDER;

  DocClear;
  "Select ctrl points with left mouse.  Right when finished.\n";
  DCFill;
  do {
    msg_code=GetMsg(&p1,&p2,1<<MSG_IP_L_UP+1<<MSG_IP_R_UP);
    if (msg_code==MSG_IP_L_UP) {
      c[i].x=p1; c[i].y=p2; c[i].z=0;
      dc->color=GREEN;
      GrCircle(dc,p1,p2,10);
      i++;
    }
  } while (msg_code!=MSG_IP_R_UP);
  dc->color=RED;
  Gr2BSpline(dc,c,i,TRUE);
  dc->color=BLUE;
  Gr3BSpline(dc,c,i,TRUE);
  DCDel(dc);
  SettingsPop;

  PressAKey;
  DCFill;
}

BSplineDemo;
