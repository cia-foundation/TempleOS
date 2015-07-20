//8 has graphics

#define	NUM		10
#define RAD_PROP	60
#define RAD_PRISM	8
I32 rainbow_10[NUM]={
RED,LTRED,BROWN,YELLOW,GREEN,LTGREEN,BLUE,LTBLUE,PURPLE,LTPURPLE};

#define	X 320
#define Y 35

F64 t0=Beat;
U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 dt=16*Sin(ã*(Beat-t0)/8),é;
  dc->color=WHITE;
  é=ã*dt+0.5;
  GrLine(dc,X+RAD_PRISM*Cos(é)	    ,Y+RAD_PRISM*Sin(é),
	X+RAD_PRISM*Cos(é+2*ã/3),Y+RAD_PRISM*Sin(é+2*ã/3));
  GrLine(dc,X+RAD_PRISM*Cos(é+2*ã/3),Y+RAD_PRISM*Sin(é+2*ã/3),
	X+RAD_PRISM*Cos(é+4*ã/3),Y+RAD_PRISM*Sin(é+4*ã/3));
  GrLine(dc,X+RAD_PRISM*Cos(é+4*ã/3),Y+RAD_PRISM*Sin(é+4*ã/3),
	X+RAD_PRISM*Cos(é),Y+RAD_PRISM*Sin(é));
  for (i=0;i<NUM;i++) {
    é=ã/8*ToF64(i)/NUM*(3+2*Sin(ã*dt/8))/3+ã*dt;
    dc->color=WHITE;
    GrLine(dc,X,Y,X-RAD_PROP*Cos(é),Y-RAD_PROP*Sin(é));
    dc->color=rainbow_10[i];
    GrLine(dc,X,Y,X+RAD_PROP*Cos(é),Y+RAD_PROP*Sin(é));
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    "$$BG+H,BLACK$$$$FD+H,WHITE$$";
    while (!ScanKey) {
      t0=Beat;
      Play("3qAsDGDGqAsCACAeCBsFDFDetCGAsADAD",
	    " \0I\0mag\0 \0ine.\n\0 \0 \0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("qAsDGDGqAsCACAeCBsFDFDetCGAsADAD",
	    " \0 \0Fa\0 \0thom.\n\0 \0 \0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("GFGFEBEBeCB2qG3sFAFAqCEA",
	    "Hi-\0 \0i-\0gh \0as \0 \0your \0 \0thoughts \0"
	    "will \0go.\n\0 \0 \0 \0 \0 \0 \0 \0");
      Play("sGFGFEBEBeCB2qG3sFAFAqCEA",
	    "Deep \0 \0 \0as \0 \0 \0we \0 \0ought \0to \0"
	    "know.\n\0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
