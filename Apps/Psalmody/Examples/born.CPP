//9 has graphics
F64 t0=Beat;
#define HORIZON_Y	50
#define LEFT_EDGE	200

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i;
  F64 dt=Beat-t0,d=Clamp(0.1*dt,0,2),d2;

  dc->color=ROPF_PROBABILITY_DITHER+LTCYAN<<16+CYAN;
  for (i=HORIZON_Y;i>=0;i--) {
    d2=ToF64(HORIZON_Y-i)/HORIZON_Y;
    dc->dither_probability_u16=1<<16*Clamp((4.0-d2)/4.0*d*d-0.15,0,1.0);
    GrLine(dc,LEFT_EDGE,i,task->pix_width-1,i);
  }

  d=(task->pix_width-LEFT_EDGE)/2+LEFT_EDGE;
  dc->color=YELLOW;
  GrCircle(dc,d,HORIZON_Y+50-dt,40);
  GrFloodFill(dc,d,HORIZON_Y+15-dt,TRUE);

  dc->color=GREEN;
  GrRect(dc,200,HORIZON_Y,task->pix_width-200,task->pix_height-HORIZON_Y);
}

U0 Song()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;

  MusicSettingsRst;
  music.tempo= 2.500;
  music.stacatto_factor= 0.900;
  try {
    while (!ScanKey) {
      t0=Beat;
      Play("3qBeB2G3qAeFAsFAFAqC2sG3A2G3ARBRB",
	  "New \0day \0is \0born.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("qBeB2G3qAeFAsFAFAqC2sG3A2G3ARBRB",
	  "Though \0it's \0the \0mourn',\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("eDDRCqG2eGR3qEetEAFqCsDGDG",
	  "cheer-\0up \0 \0the \0heart\n\0 \0 \0 \0 \0 \0 \0 \0 \0as \0 \0you \0");
      Play("eDDRCqG2eGR3qEetEAFqCsDGDG",
	  "hitch-\0up \0 \0the \0cart.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
