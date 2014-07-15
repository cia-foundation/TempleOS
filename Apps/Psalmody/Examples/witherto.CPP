//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  music.tempo=2.5;
  try {
    task=PopUpViewPrint(
	  "$LK,"Mark,11:12",A="BF:Mark,11:12"$\n"
	  "$LK,"Matthew,3:10",A="BF:Matthew,3:10"$\n"
	  "$LK,"Matthew,25:18",A="BF:Matthew,25:18"$\n"
	  );
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("2qG3sFEFEqDBEetCB2G3qFsBDBD",
	    " \0Damn \0 \0 \0 \0 \0 \0fig \0 \0 \0 \0tree.\n\0 \0 \0 \0 \0");
      Play("2qG3sFEFEqDBEetCB2G3qFsBDBD",
	    " \0On\0ly \0 \0 \0good \0 \0for \0a \0 \0 \0pee.\n\0 \0 \0 \0 \0");
      Play("qC2G3eFC2G3EqACDA",
	    "Has \0no \0fruit.\n\0 \0 \0 \0 \0 \0 \0 \0");
      Play("C2G3eFC2G3EqACDA",
	    "Kill \0the \0root.\n\0 \0 \0 \0 \0 \0 \0 \0");
      "$$GREEN$$Be sure to bear 'fruit'.$$FG$$\n\n";
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
