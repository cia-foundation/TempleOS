//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13qEDeCC!etEAAqEetCDCA!!GFqG",
		"I \0don't \0know \0 \0 \0 \0 \0 \0 \0 \0what \0 \0to \0 \0pray.\n\0");
	Play("EDeCC!etEAAqEetCDCA!!GFqG");
	Play("eGEetFDAqF!sGF!GF!G!!FG!F!etBGFqGeDE",
		"It's \0a \0blah \0 \0 \0day.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("GEetFDAqF!sGF!GF!G!!FG!F!etBGFqGeDE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
