U0 BSplineDemo()
{
  CDC *dc=DCAlias;
  I64 a1,a2,i=0,msg_code;
  CD3I32 c[64];

  SettingsPush;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;

  DocClear;
  "Sel ctrl points with left mouse.  Right when finished.\n";
  DCFill;
  do {
    msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_UP+1<<MSG_IP_R_UP);
    if (msg_code==MSG_IP_L_UP) {
      c[i].x=a1; c[i].y=a2; c[i].z=0;
      dc->color=GREEN;
      GrCircle(dc,a1,a2,10);
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
