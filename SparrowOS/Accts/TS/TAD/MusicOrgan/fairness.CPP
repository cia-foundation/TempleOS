//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  task=PopUpViewStr(
    "$LK-A,"BF:Luke,7:42"$\n"
    "\n"
    "Prodigal Son\n"
    "$LK-A,"BF:Luke,15:21"$\n"
    );
  TaskWait(task);
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  task->win_top=Fs->win_bottom+3;
  task->win_bottom=task->win_top+12;
  TaskDerivedValsUpdate(task);
  music_tempo=2.5;
  try {
  "$$WW+H,1$$";
  while (!ScanKey) {
	Play("3qGGsEAEAqAsF2G3F2G3qCeEFqD",
		"Life, \0life, \0isn't \0 \0 \0 \0fair.  \0Isn't \0 \0 \0 \0fair.\n\0 \0 \0 \0");
	Play("GGsEAEAqAsF2G3F2G3qCeEFqD",
		"Waaa, \0waaa, \0isn't \0 \0 \0 \0fair.  \0Isn't \0 \0 \0 \0fair.\n\0 \0 \0 \0");
	Play("eFCqEBDBE2eG3EsFEFE",
		" \0 \0I \0 \0ne\0ver \0sin.\n\0 \0 \0 \0 \0 \0 \0");
	Play("eFCqEBDBE2eG3EsFEFE",
		" \0 \0I \0 \0ne\0ver \0win.\n\0 \0 \0 \0 \0 \0 \0");
	"$$FG,GREEN$$(A warning on presumption. 'Who loves more man -- forgiven big debt or little?'  See prodigal son.)$$FG$$\n";
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
