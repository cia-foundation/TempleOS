//8 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  task=PopUpViewStr(
    "$LK,"Romans,8:28","BF:Romans,8:28"$\n"
    );
  TaskWait(task);
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  task->win_top=Fs->win_bottom+3;
  task->win_bottom=task->win_top+12;
  TaskDerivedValsUpdate(task);
  while (!ScanKey) {
	Play("W13qEEBeBDsBBB!BqDsBFB!!FD2G3D!2G",
		"Free \0will... \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0 \0fail.\n\0 \0 \0 \0");
	Play("3qEEBeBDsBBB!BqDsBFB!!FD2G3D!2G",
		"God \0still... \0 \0 \0 \0 \0 \0 \0 \0 \0I \0 \0 \0 \0hail.\n\0 \0 \0 \0");
	Play("3qD2G3eA2G3qCetEC!AqBsA!!C!ACqA",
		"All \0things \0work \0for \0good... \0to \0the \0Lord.\n\0 \0 \0 \0 \0 \0 \0");
	Play("D2G3eA2G3qCetEC!AqBsA!!C!ACqA",
		"It's \0all \0good... \0 \0 \0as\0 \0sured.\n\0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
