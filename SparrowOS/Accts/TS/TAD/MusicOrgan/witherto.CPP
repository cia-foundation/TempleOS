//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  try {
  task=PopUpViewStr(
    "$LK,"Mark,11:12","BF:Mark,11:12"$\n"
    "$LK,"Matthew,3:10","BF:Matthew,3:10"$\n"
    "$LK,"Matthew,25:18","BF:Matthew,25:18"$\n"
    );
  TaskWait(task);
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  task->win_top=Fs->win_bottom+3;
  task->win_bottom=task->win_top+12;
  TaskDerivedValsUpdate(task);
  while (!ScanKey) {
	Play("2qG3sFEFEqDBEetCB2G3qFsBDBD",
		" \0Damn \0 \0 \0 \0 \0 \0fig \0 \0 \0 \0tree.\n\0 \0 \0 \0 \0");
	Play("2qG3sFEFEqDBEetCB2G3qFsBDBD",
		" \0On\0ly \0 \0 \0good \0 \0for \0a \0 \0 \0pee.\n\0 \0 \0 \0 \0");
	Play("qC2G3eFC2G3EqACDA",
		"Has \0no \0fruit.\n\0 \0 \0 \0 \0 \0 \0 \0");
	Play("C2G3eFC2G3EqACDA",
		"Kill \0the \0root.\n\0 \0 \0 \0 \0 \0 \0 \0");
	"$$FG,GREEN$$Be sure to bear 'fruit'.$$FG$$\n\n";
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
