//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3sC#C#D#D#C#C#CCC#C#D#D#C#C#CCEEDDC#C#DDEEDDC#C#DD",
		"Still \0 \0 \0 \0there.\n\0 \0 \0 \0Still \0 \0 \0 \0there.\n\0 \0 \0 \0Not \0 \0 \0 \0fair.\n\0 \0 \0 \0Not \0 \0 \0 \0fair.\n\0 \0 \0 \0");
	Play("C#C#D#D#C#C#CCC#C#D#D#C#C#CCEEDDC#C#DDEEDDC#C#DD",
		"Un\0 \0 \0 \0aware!\n\0 \0 \0 \0Un\0 \0 \0 \0aware!\n\0 \0 \0 \0Every\0 \0 \0 \0where!\n\0 \0 \0 \0Every\0 \0 \0 \0where!\n\0 \0 \0 \0");
	Play("eBDetFGAeFEBEetEFEqCeFEDA",
		"$$RED$$Running \0 \0 \0a\0way \0 \0 \0 \0 \0 \0from \0the \0Lord \0won't \0 \0work.\n\0 \0");
	Play("BDetFGAeFEBEetEFEqCeFEDA",
		"Running \0 \0 \0a\0way \0 \0 \0 \0 \0 \0from \0the \0Lord \0won't \0 \0work.$$FG$$\n\0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
