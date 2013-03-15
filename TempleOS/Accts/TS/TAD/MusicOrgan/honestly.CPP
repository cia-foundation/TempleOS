//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.758;
  try {
  while (!ScanKey) {
	Play("W13qBeABqA2G3AB2hG");
	Play("3sCCCCqBsAAAA2qG3sCCCCqE2GR");
	Play("!!3sBDB!!DA2G!3A2G!!3etAA!EsGCGCetFFEqDFeE2G");
	Play("!!3sBDB!!DA2G!3A2G!!3etAA!EsGCGCetFFEqDFeE2G");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
