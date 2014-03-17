//9 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 1.710;
  music_stacatto_factor= 0.736;
  try {
    task=PopUpViewStr("$LK,"Ecclesiastes,5:12",A="BF:Ecclesiastes,5:12"$\n");
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3sDDDACCADDACCAe.DsCCC2G3FD2G3FD2G3CCAe.D",
	    "I \0 \0 \0 \0got \0 \0 \0paid, \0 \0 \0to\0 \0 \0day.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("qD2sG3G2G3GetGDAeF2GqG3etFBAEACECE",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0can't \0 \0 \0get \0 \0to \0");
      Play("qD2sG3G2G3GetGDAeF2GqG3etFBAEACECE",
	    "sleep.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
