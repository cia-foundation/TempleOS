//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint(
	  "$LK,"Matthew,20:28",A="BF:Matthew,20:28"$\n"
	  "$LK,"Mark,10:45",A="BF:Mark,10:45"$\n"
	  "$LK,"John,13:14",A="BF:John,13:14"$\n"
	  );
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      "$$GREEN$$Jesus talking...\n$$FG$$";
      Play("M6/8");
      Play("3eAACADAADACAR",
	    " \0 \0I \0 \0came \0 \0 \0to \0 \0serve.\n\0 \0 \0");
      Play("AACADAADACAR",
	    " \0 \0I \0 \0give \0 \0 \0my \0 \0word.\n\0 \0 \0");
      Play("FGetFEGABCDAC2qG3sECEC");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
