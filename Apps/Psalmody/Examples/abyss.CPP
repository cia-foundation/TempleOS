//7 has graphics

F64 t0=Beat;

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,w=task->pix_width,h=task->pix_height;
  F64 dt=Beat-t0,num;

  num=MinI64(Exp(0.9*dt),3000);
  dc->color=LTGRAY;
  for (i=0;i<num;i++) {
    j=RandU32;
    GrPlot(dc,j.u16[0]%w,j.u16[1]%h);
  }

  num=MinI64(Exp(0.6*dt),6000);
  dc->color=DKGRAY;
  for (i=0;i<num;i++) {
    j=RandU32;
    GrPlot(dc,j.u16[0]%w,j.u16[1]%h);
  }

  num=MinI64(Exp(0.3*dt),9000);
  dc->color=BLACK;
  for (i=0;i<num;i++) {
    j=RandU32;
    GrPlot(dc,j.u16[0]%w,j.u16[1]%h);
  }
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.736;
  try {
    task=PopUpViewPrint(
	  "There are myths about the River\n"
	  "Styx separating the land of the\n"
	  "living from dead.  Jesus' told a story\n"
	  "about the land of living separate from\n"
	  "land of dead.  ($LK,"Luke,16:22",A="BF:Luke,16:22"$)  In the Old\n"
	  "Testament, the River Jordan was a special\n"
	  "barrier.  ($LK,"Deuteronomy,4:22",A="BF:Deuteronomy,4:22"$)\n"
	  );
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      t0=Beat;
      Play("3sBBBBqCq.BeAqBh.A",
	    "Can't \0 \0 \0 \0get \0there, \0from \0he\0re.\n\0");
      Play("M2/4");
      Play("eECqAM4/4");
      Play("sBBBBqCq.BeAqBh.A",
	    "Can't \0 \0 \0 \0get \0there, \0from \0he\0re.\n\0");
      Play("M2/4");
      Play("eDDqDM4/4");
      Play("DeEDqGCeF2G3qGsFGFGeDC",
	    "Oh, \0my \0 \0God, \0some\0thing's... \0 \0 \0"
	    "ver\0 \0y \0 \0wrong!\n\0 \0");
      Play("qDeEDqGCeF2G3qGsFGFGeDC",
	    "Oh, \0my \0 \0God, \0some\0thing's... \0 \0 \0"
	    "ver\0 \0y \0 \0wrong!\n\0 \0");
    }
  } catch
    PutExcept;
  SettingsPop;
}

Song;
