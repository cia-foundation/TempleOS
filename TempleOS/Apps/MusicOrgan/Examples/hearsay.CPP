//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qAeCDqEAeCDECDECsCC");
	Play("qAeCDqEAeCDECDECsCC");
	Play("eDAGAqDsECECCBCBeAFsDCeGDE");
	Play("DAGAqDsECECCBCBeAFsDCeGDE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
