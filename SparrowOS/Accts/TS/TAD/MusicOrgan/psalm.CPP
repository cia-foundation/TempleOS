//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 3.360;
  music_stacatto_factor= 0.846;
  try {
  while (!ScanKey) {
	Play("M5/4");
	Play("W12qGGG3qtCBA2qGGG3qtCetBBqtA");
	Play("qEeCCqDqtCBAM4/4");
	Play("qD!eA!2G3DG2G3DF!FAC!!2qG3eAE");
	Play("qD!eA!2G3DG2G3DF!FAC!!2qG3eAE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
