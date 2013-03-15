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
	Play("W13qAeCBAq.CsAAqAeCBA2q.G3sBC");
	Play("qAeDCAhBeFsEEeDq.CM4/4");
	Play("sAFAFeF!!2GsG3B2G!!3BqGeA!sAAetEGDE2G!3DeGsGG");
	Play("AFAFeF!!2GsG3B2G!!3BqGeA!sAAetEGDE2G!3DeGsGG");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
