//9 has graphics

#define RAD	20
#define WIDTH	16

F64 t0=Beat;
U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0,x1,y1,x2,y2,z,dé;
  I64 i;

  DCDepthBufAlloc(dc);
  dc->x=200;
  dc->y=70;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  Mat4x4RotX(dc->r,ã/4);
  Mat4x4RotY(dc->r,Sin(ã*dt/23)/5);

  if (dt<28)
    dé=-2*ã/8+(dt+16)*Sin(-ã*dt)/3000;
  else if (dt<32)
    dé=-2*ã/8-2*ã/8*(dt-28)/4;
  else
    dé=-2*ã/4;

  z=WIDTH*3;
  x1=-12; y1=0;
  x2=x1-RAD/2*Cos(dé);
  y2=y1+RAD*Sin(dé);
  dc->color=BLACK;
  dc->pen_width=14;
  GrLine3(dc,x1,y1,z,x2,y2,z);
  dc->color=YELLOW;
  dc->pen_width=12;
  GrLine3(dc,x1,y1,z,x2,y2,z);


  for (z=0;z<WIDTH*4;z+=WIDTH) {
    x1=-RAD; y1=0;
    for (i=0;i<4;i++) {
      x2=x1-RAD*Cos(dé*i);
      y2=y1+RAD*Sin(dé*i);
      dc->color=BLACK;
      dc->pen_width=10;
      GrLine3(dc,x1,y1,z,x2,y2,z);
      dc->color=YELLOW;
      dc->pen_width=8;
      GrLine3(dc,x1,y1,z,x2,y2,z);
      x1=x2; y1=y2;
    }
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("M4/4");
      Play("3qBsEGeBDe.AsCeA2qF3sBBeACBBsBB",
	    "I \0could \0 \0 \0die.\n\0 \0 \0"
	    " \0 \0 \0 \0 \0Could \0I \0kill?\n\0 \0 \0");
      Play("qBsEGeBDe.AsCeA2qF3sBBeACBBsBB",
	    "I'm \0not \0 \0 \0spry.\n\0 \0 \0 \0 \0 \0"
	    " \0 \0I \0best \0chill.\n\0 \0 \0");
      Play("eDDsGFGFeEBqGM5/4",
	    "If \0 \0God \0 \0 \0 \0 \0 \0asked,\n\0");
      Play("eBGF4A3e.FsEqAAM4/4",
	    " \0\"$$RED$$Can \0 \0 \0you \0 \0Kill?$$FG$$\"\n\0 \0");
      Play("eDDsGFGFeEBqGM5/4",
	    "I \0 \0think, \0 \0 \0 \0 \0 \0\"Yeah.\"\n\0");
      Play("eBGF4A3e.FsEqAA",
	    " \0\"Right \0 \0 \0my \0 \0will.\"\n\0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
