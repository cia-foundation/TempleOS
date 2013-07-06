//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 1.930;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("M9/8");
	Play("3qAeCBAq.CsAAqAeCBA2q.G3sBC");
	Play("qAeDCAhBeFsEEeDq.CM4/4");
	Play("sAFAFeF2GsG3B2G3BqGeAsAAetEGDE2G3DeGsGG");
	Play("AFAFeF2GsG3B2G3BqGeAsAAetEGDE2G3DeGsGG");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
