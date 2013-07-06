//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("M9/8");
	Play("3hDeFhDeFhDC");
	Play("FeDhFeDhFE");
	Play("M4/4");
	Play("qD2G3AeBFFE2qG3DA",
		"Hold \0 \0on!\n\0 \0It's \0gon\0na \0be \0fine.\n\0 \0");
	Play("D2G3AeBFFE2qG3DA",
		"Hold \0 \0on!\n\0 \0It's \0gon\0na \0be \0fine.\n\0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
