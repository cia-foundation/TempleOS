//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.846;
  try {
  while (!ScanKey) {
	Play("3sEAEAqEeFsGAqFsFGeE2qG3sFEFEqA");
	Play("sEAEAqEeFsGAqFsFGeE2qG3sFEFEqA");
	Play("EeGDqBeCE2qG3etDGCeEAFA");
	Play("qEeGDqBeCE2qG3etDGCeEAFA");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
