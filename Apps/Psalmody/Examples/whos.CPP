//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Luke,13:25",A="BF:Luke,13:25"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3qGeEFqEBCGqtBD4A",
	    "God, \0let \0me \0in!\n\0 \0 \0 \0 \0 \0 \0");
      Play("3qGeEFqEBCGqtBDF",
	    "God, \0did \0I \0sin?\n\0 \0 \0 \0 \0 \0 \0");
      "$$GREEN$$God talking...\n$$FG$$";
      Play("qE2eG3FqDEeBEqCeCDBC",
	    "$$RED$$Who's \0 \0 \0 \0there?\n\0 \0 \0 \0 \0 \0 \0 \0");
      Play("qBeAsBAqE2eG3FqDEeBEqC",
	    " \0 \0 \0 \0Who's \0 \0 \0 \0where?$$FG$$\n\0 \0 \0 \0");
      Play("eCDBCqBeAB");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
