//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qGsARARCBCBeACACGCGBAsEA");
	Play("qGsARARCBCBeACACGCGBAsEA");
	Play("eAsCEqEsDECEqAsDACAeAGFsBDeCB");
	Play("AsCEqEsDECEqAsDACAeAGFsBDeCB");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
