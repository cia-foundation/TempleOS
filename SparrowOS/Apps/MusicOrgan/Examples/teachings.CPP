//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  task=PopUpViewStr(
	"$LK,"Proverbs,11:25","BF:Proverbs,11:25"$\n"
    );

  TaskWait(task);
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  task->win_top=Fs->win_bottom+3;
  task->win_bottom=task->win_top+12;
  TaskDerivedValsUpdate(task);
  while (!ScanKey) {
	Play("W12eG3CD!AqE!!sACACG!EGEqDsAAA!AetCDD",
		"Ple\0ase \0te\0ach \0me.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2eG3CD!AqE!!sACACG!EGEqDsAAA!AetCDD",
		"Wa\0ter \0thi\0s \0tree.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2qG!G3eEBqAGeFE!!sF!GFG!etD!BD",
		" \0 \0$$FG,RED$$I \0will \0teach \0thee.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2qG!G3eEBqAGeFE!!sF!GFG!etD!BD",
		" \0 \0Wat\0er \0that \0tree.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
