//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("4qBDBA3eG4BqCeCFqC");
	Play("BDBA3eG4BqCeCFqC");
	Play("eBDFGqD3eG4G3qG4sA3G4eDqBe.DsE");
	Play("eBDFGqD3eG4G3qG4sA3G4eDqBe.DsE");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
