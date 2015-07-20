//9 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 1.710;
  music.stacatto_factor= 0.736;
  try {
    task=PopUpViewPrint("$LK,"Ecclesiastes,5:12",A="BF:Ecclesiastes,5:12"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3sDDDACCADDACCAe.DsCCC2G3FD2G3FD2G3CCAe.D",
	    "I \0 \0 \0 \0got \0 \0 \0paid, \0 \0 \0to\0 \0 \0"
	    "day.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("qD2sG3G2G3GetGDAeF2GqG3etFBAEACECE",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0"
	    "can't \0 \0 \0get \0 \0to \0");
      Play("qD2sG3G2G3GetGDAeF2GqG3etFBAEACECE",
	    "sleep.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
