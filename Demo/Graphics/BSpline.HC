#define PTS_NUM	16

U0 BSplineDemo()
{
  CDC *dc=DCAlias;
  I64 arg1,arg2,i=0,msg_code;
  CD3I32 c[PTS_NUM];

  SettingsPush;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;

  DocClear;
  "Sel ctrl points with left mouse.  Right when finished.\n";
  DCFill;
  do {
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_R_UP);
    if (msg_code==MSG_MS_L_UP) {
      c[i].x=arg1; c[i].y=arg2; c[i].z=0;
      dc->color=GREEN;
      GrCircle(dc,arg1,arg2,10);
      i++;
    }
  } while (i<PTS_NUM && msg_code!=MSG_MS_R_UP);
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
