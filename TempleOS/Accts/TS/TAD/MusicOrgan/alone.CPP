//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("2q.GeGqG3sBBCB2q.GeGqG3sBBCB");
	Play("q.C2eG3CsDC2eGG3qC2G3eCsDC2eGG");
	Play("3sEAEAeCsCReRsDAe.GsBFFFFeEsAGeCsGDeDG");
	Play("sEAEAeCsCReRsDAe.GsBFFFFeEsAGeCsGDeDG");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
