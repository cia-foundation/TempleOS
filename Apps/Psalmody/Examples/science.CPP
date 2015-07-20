//7 has graphics

F64 t0=Beat;

U0 MyCircle3(CDC *dc,I64 x,I64 y,I64 z,I64 r)
{
  F64 é,dé=1.0/r;
  for (é=0;é<2*ã;é+=dé)
    GrPlot3(dc,x+r*Cos(é),y+r*Sin(é),z);
}

U0 MyCylinder(CDC *dc,I64 x,I64 y,I64 z1,I64 z2,I64 r)
{
  I64 z;
  for (z=z1;z<z2;z++)
    MyCircle3(dc,x,y,z,r);
}

U0 DrawIt(CTask *,CDC *dc)
{
  F64 dt=Beat-t0,r1=12,r2=10,w1=25,w2=20,w3=18;

  DCDepthBufAlloc(dc);
  dc->x=300;
  dc->y=35;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  Mat4x4RotX(dc->r,Sin(ã*dt/4)/8);
  Mat4x4RotY(dc->r,ã*dt/16);

  w1+=3*Sin(ã*dt/3);

  dc->color=BROWN;
  MyCylinder(dc,0,0,-w3,w3,r2);

  dc->color=YELLOW;
  MyCylinder(dc,0,0,-w1-w2/2+0,-w1-w2/2+1,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,-w1-w2/2+1,-w1-w2/2+2,r1);
  dc->color=LTGRAY;
  MyCylinder(dc,0,0,-w1-w2/2+2,-w1+w2/2-2,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,-w1+w2/2-2,-w1+w2/2-1,r1);
  dc->color=YELLOW;
  MyCylinder(dc,0,0,-w1+w2/2-1,-w1+w2/2,r1);

  dc->color=BLACK;
  GrLine3(dc,r1,0,-w1-w2/2+1, r1,0,-w1-w2/2+w2-2);

  dc->color=YELLOW;
  MyCylinder(dc,0,0,w1-w2/2+0,w1-w2/2+1,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,w1-w2/2+1,w1-w2/2+2,r1);
  dc->color=LTGRAY;
  MyCylinder(dc,0,0,w1-w2/2+2,w1+w2/2-2,r1);
  dc->color=BLACK;
  MyCylinder(dc,0,0,w1+w2/2-2,w1+w2/2-1,r1);
  dc->color=YELLOW;
  MyCylinder(dc,0,0,w1+w2/2-1,w1+w2/2,r1);

  dc->color=BLACK;
  GrLine3(dc,r1,0,w1-w2/2+1, r1,0,w1-w2/2+w2-2);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.824;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("2eG3sEAeADDG2qG3sFGeCGBsDE2eG3EB",
	    "Was \0 \0Ga\0li\0le\0o \0 \0wrong?\n\0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("2G3sEAeADsDDeG2qG3sFGeCGBsDE2eG3EB",
	    "His \0 \0brain \0 \0was \0might\0 \0y \0strong.\n\0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("etEDE4A3GFeEGqDsDGeAsAGAGqAeEC",
	    "Did \0you \0 \0real\0ly \0want \0to \0 \0win?\n\0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("etEDE4A3GFeEGqDsDGeAsAGAGqAeEC",
	    "We \0have \0 \0us, \0a \0real \0Huck \0 \0Finn.\n\0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
