//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Proverbs,11:25",A="BF:Proverbs,11:25"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("2eG3CDAqEsACACGEGEqDsAAAAetCDD",
	    "Ple\0ase \0te\0ach \0me.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("2eG3CDAqEsACACGEGEqDsAAAAetCDD",
	    "Wa\0ter \0thi\0s \0tree.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("2qGG3eEBqAGeFEsFGFGetDBD",
	    " \0 \0$$RED$$I \0will \0teach \0thee.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("2qGG3eEBqAGeFEsFGFGetDBD",
	    " \0 \0Wat\0er \0that \0tree.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
