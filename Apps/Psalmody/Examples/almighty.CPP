//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
  while (!ScanKey) {
	Play("2qG3etC2GG3qARR2etG3FBqEeFE",
		"All \0righ\0 \0 \0ty.\n\0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2qG3etC2GG3qARR2etG3FBqEeFE",
		"All \0migh\0 \0 \0ty.\n\0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qFFEEeEBqFCB",
		"Al\0read\0y, \0 \0I've \0 \0seen \0en\0ough.\n\0");
	Play("FFEEeEBqFCB",
		"Oh \0bo\0y, \0 \0was \0 \0that \0rough.\n\0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
