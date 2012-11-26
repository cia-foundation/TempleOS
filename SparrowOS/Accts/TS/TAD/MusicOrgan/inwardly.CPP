//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W12q.GeG3qB2GG3A2GeFF");
	Play("q.GeG3qB2GG3A2GeFF");
	Play("3CD!qGB!AF!C!FeDB");
	Play("CD!qGB!AF!C!FeDB");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
