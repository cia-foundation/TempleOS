//8 has graphics

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 dt=tMBeat%8.0,z,w,x1,y1,z1,x2,y2,z2;
  Bool first=TRUE;
  DCAllocDepthBuf(dc);
  dc->flags|=DCF_TRANSFORMATION;
  dc->x=30*dt+20.0*Sin(dt)+25;
  dc->y=10*dt-15;
  dc->z=GR_Z_ALL;
  GrRotXEqu(dc->r,0.75*dt);
  GrRotYEqu(dc->r,0.45*dt);
  GrRotZEqu(dc->r,0.50*dt);
  GrScaleMatEqu(dc->r,0.5);
  for (i=-50;i<40;i+=2) {
    if (i&2)
      dc->color=LTGRAY;
    else
      dc->color=DKGRAY;
    z=10*Cos(i/10.0*ã/8.0)*Cos(dt);
    w=Tri(100+i,200);
    x1=i-3;y1=0;z1=z;
    if (-30<=i<=40) {
      GrLine3(dc,i,-10*w,z,i-3,0,z);
      GrLine3(dc,i,+10*w,z,i-3,0,z);
    }
    if (!first) {
      dc->color=DKGRAY;
      GrLine3(dc,x1,y1,z1,x2,y2,z2);
    }
    x2=x1;y2=y1;z2=z1;
    first=FALSE;
  }
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
    "$$BG+H,LTCYAN$$";
    while (!ScanKey) {
	Play("W13qE!AA!etF2G3G!qDF!!etFE!D2eG!3G",
		"A\0dam... \0 \0no \0 \0 \0long\0er \0sees \0Your \0face.\n\0 \0 \0");
	Play("qE!AA!etF2G3G!qDF!!etFE!D2eG!3G",
		"Oh! \0 \0 \0What \0 \0a \0wretch\0ed \0bar\0ren \0waste!\n\0 \0 \0");
	Play("qACFeB!CB2GqG3E!D",
		" \0 \0 \0 \0 \0 \0 \0 \0Ex\0iled.\n\0");
	Play("ACFeB!CB2GqG3E!D");
    }
  } catch
    CatchAll;
  SettingsPop;
}

Song;
