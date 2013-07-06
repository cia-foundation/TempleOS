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
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
  while (!ScanKey) {
	Play("2eG3CDAqEsACACGEGEqDsAAAAetCDD",
		"Ple\0ase \0te\0ach \0me.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2eG3CDAqEsACACGEGEqDsAAAAetCDD",
		"Wa\0ter \0thi\0s \0tree.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2qGG3eEBqAGeFEsFGFGetDBD",
		" \0 \0$$FG,RED$$I \0will \0teach \0thee.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2qGG3eEBqAGeFEsFGFGetDBD",
		" \0 \0Wat\0er \0that \0tree.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
