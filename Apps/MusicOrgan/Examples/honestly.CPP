//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.758;
  try {
  while (!ScanKey) {
	Play("3qBeABqA2G3AB2hG");
	Play("3sCCCCqBsAAAA2qG3sCCCCqE2GR");
	Play("3sBDBDA2G3A2G3etAAEsGCGCetFFEqDFeE2G");
	Play("3sBDBDA2G3A2G3etAAEsGCGCetFFEqDFeE2G");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
