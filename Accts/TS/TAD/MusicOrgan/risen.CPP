//9 has graphics

/*
This is one of the oldest songs.  I picked
the random name "risen" and said to God
"Oh, you're ambitious," thinking it was
an epic name.  He laughed and gave an
epic song!
*/

F64 t0=Beat;

#define WING	6
#define BODY	2

U0 Bird(CDC *dc,F64 wing_é)
{
  CD3I32 p[3];
  dc->color=WHITE;

  p[0].x=0; p[0].y=0; p[0].z=-BODY;
  p[1].x=0; p[1].y=0; p[1].z=BODY;
  p[2].x=-WING*Cos(wing_é); p[2].y=-WING*Sin(wing_é); p[2].z=0;
  GrFillPoly3(dc,3,p);

  p[0].x=0; p[0].y=0; p[0].z=-BODY;
  p[1].x=0; p[1].y=0; p[1].z=BODY;
  p[2].x=WING*Cos(wing_é); p[2].y=-WING*Sin(wing_é); p[2].z=0;
  GrFillPoly3(dc,3,p);
 
  dc->color=ROPF_DITHER+WHITE<<16+LTGRAY;
  GrLine3(dc,0,0,-BODY,0,0,BODY);
}

#define NUM_BIRDS	7
#define RANGE		5

F64 bx[NUM_BIRDS],by[NUM_BIRDS];

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 é,dt=Beat-t0;
  DCAllocDepthBuf(dc);
  dc->x=325;
  dc->y=95;
  dc->z=GR_Z_ALL;
  dc->flags|=DCF_TRANSFORMATION;
  for (i=0;i<NUM_BIRDS;i++) {
    bx[i]=Clamp(bx[i]+0.25*SignI64(RandI16),-RANGE,RANGE);
    by[i]=Clamp(by[i]+0.25*SignI64(RandI16),-RANGE,RANGE);
    é=ã/2*i/NUM_BIRDS+0.2*ã*dt+ã/2;
    GrIdentEqu(dc->r);
    GrRotXEqu(dc->r,ã/2);
    GrRotZEqu(dc->r,é);
    GrSetTranslation(dc->r,50*Cos(é)+bx[i],50*Sin(é)+by[i],30*i/NUM_BIRDS+4*dt);
    GrRotXEqu(dc->r,0.35*ã);
    Bird(dc,Sin(2*ã*dt+i*2*ã/NUM_BIRDS));
  }
}

U0 Song()
{
  I64 i;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=LTCYAN<<4+BLACK;
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  for (i=0;i<NUM_BIRDS;i++) {
    bx[i]=RANGE*RandI16/RANGE;
    by[i]=RANGE*RandI16/RANGE;
  }
  try {
    "$$BG+H,LTCYAN$$";
    while (!ScanKey) {
	t0=Beat;
	Play("3eDEqFFetEEFqDeCDDEetCGF");
	Play("eDEqFFetEEFqDeCDDEetCGF");
	Play("eDCqDEeAAetEEFEDGBDCqF");
	Play("eDCqDEeAAetEEFEDGBDCqF");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
