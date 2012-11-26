//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13qEDeCC!etEAAqEetCDCA!!GFqG");
	Play("EDeCC!etEAAqEetCDCA!!GFqG");
	Play("eGEetFDAqF!sGF!GF!G!!FG!F!etBGFqGeDE");
	Play("GEetFDAqF!sGF!GF!G!!FG!F!etBGFqGeDE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
