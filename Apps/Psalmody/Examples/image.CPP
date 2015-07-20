//7 has graphics

#define WIDTH	17.0
#define X	290
#define Y	25

F64 t0=Beat;
U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0,d,y,x1,y1,x2,y2;
  I64 i,j;

  dc->color=BLACK;
  dc->pen_width=7;
  for (i=0;i<4;i++) {
    y=0;
    d=dt%4;
    if (d<1) {
      d+=0.25*i;
      if (d<1.0)
	y=Tri(d,1)`2.0;
    }
    x1=X+(i-2.5)*WIDTH;
    y1=50-20*y;
    GrLine3(dc,x1,y1,0,(X*3+x1)/4,Y,0);
  }

  x1=X+1.3*WIDTH;
  y1=37;
  GrLine3(dc,x1,y1,0,(X*3+x1)/4,Y,0);

  for (i=0;i<4;i++) {
    y=0;
    d=dt%4;
    if (d<1) {
      d+=0.25*i;
      if (d<1.0)
	y=Tri(d,1)`2.0;
    }
    x1=X+(i-2.5)*WIDTH;
    y1=50-20*y;
    dc->color=YELLOW;
    dc->pen_width=5;
    GrLine3(dc,x1,y1,0,(X*3+x1)/4,Y,0);

    dc->color=BLACK;
    for (j=0;j<4;j++) {
      x2=(x1*(4-j)+(X*3+x1)/4*j)/4;
      y2=(y1*(4-j)+Y*j)/4;
      GrLine(dc,x2-2,y2,x2,y2-1);
      GrLine(dc,x2,y2-1,x2+1,y2);
    }
  }

  x1=X+1.3*WIDTH;
  y1=37;
  dc->color=YELLOW;
  dc->pen_width=5;
  GrLine3(dc,x1,y1,0,(X*3+x1)/4,Y,0);
  dc->color=BLACK;
  x2=0.6*x1+0.4*(X*3+x1)/4;
  y2=0.6*y1+0.4*Y;
  GrLine(dc,x2,y2-1,x2-1,y2+1);
  x2=0.2*x1+0.8*(X*3+x1)/4;
  y2=0.2*y1+0.8*Y;
  GrLine(dc,x2-1,y2,x2-1,y2+2);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    task=PopUpViewPrint(
	  "Asking God for stuff is like knocking on the door\n"
	  "of a neighbor in the middle of the night.\n"
	  "It's annoying!  We're supposed to love God!\n"
	  "$LK,"BF:Luke,11:5"$\n"
	  );
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      t0=Beat;
      Play("3qBBE2GG3FFC",
	    "We \0are \0in \0the \0image \0of \0God.\n\0 \0");
      Play("BBE2GG3FFC",
	    "Some \0people's \0prayer \0is \0ver\0y \0odd.\n\0 \0");
      Play("eGDqCB2G3eGEqEC2G",
	    "They \0don't \0love \0Him.\n\0 \0"
	    "They \0love \0their \0neigh\0bor.\n\0");
      Play("3eGDqCB2G3eGEqEC2G",
	    "Pest\0er\0ing \0Him.\n\0 \0Ask\0ing \0for \0fav\0ors.\n\0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
