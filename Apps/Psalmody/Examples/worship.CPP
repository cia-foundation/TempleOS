//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qEDeCCetEAAqEetCDCAGFqG",
		"I \0don't \0know \0 \0 \0 \0 \0 \0 \0 \0what \0 \0to \0 \0pray.\n\0");
	Play("EDeCCetEAAqEetCDCAGFqG");
	Play("eGEetFDAqFsGFGFGFGFetBGFqGeDE",
		"It's \0a \0blah \0 \0 \0day.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("GEetFDAqFsGFGFGFGFetBGFqGeDE");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
