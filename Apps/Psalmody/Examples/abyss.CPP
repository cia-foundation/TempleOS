//7 has words
U0 Song()
{
  CTask *task;
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
      Play("3sBBBBqCq.BeAqBh.A",
	    "Can't \0 \0 \0 \0get \0there, \0from \0he\0re.\n\0");
      Play("M2/4");
      Play("eECqAM4/4");
      Play("sBBBBqCq.BeAqBh.A",
	    "Can't \0 \0 \0 \0get \0there, \0from \0he\0re.\n\0");
      Play("M2/4");
      Play("eDDqDM4/4");
      Play("DeEDqGCeF2G3qGsFGFGeDC",
	    "Oh, \0my \0 \0God, \0some\0thing's... \0 \0 \0ver\0 \0y \0 \0wrong!\n\0 \0");
      Play("qDeEDqGCeF2G3qGsFGFGeDC",
	    "Oh, \0my \0 \0God, \0some\0thing's... \0 \0 \0ver\0 \0y \0 \0wrong!\n\0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
