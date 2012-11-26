//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.857;
  try {
  while (!ScanKey) {
	Play("W13eEDEFqDDeEDEDqCC");
	Play("eD2G3BCqDsGGGGeD2G3BCqDsGGGG");
	Play("qF!FFetREA!qG!sG!FG!!F!GEGEqC");
	Play("F!FFetREA!qG!sG!FG!!F!GEGEqC");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
