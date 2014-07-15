//9 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3sCGCGetADD2sG3G2G3GFAFADDDDqGsB2G3B2G3qG");
	Play("sCGCGetADD2sG3G2G3GFAFADDDDqGsB2G3B2G3qG");
	Play("eG2G3qEeAsA2G3qAsEAEADEDEqFeA2G");
	Play("3G2G3qEeAsA2G3qAsEAEADEDEqFeA2G");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
