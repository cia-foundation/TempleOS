//9 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13sC!G!CG!!etA!DD2sG!3G2G3G!F!A!!FADD!!DD!qGsB2G3B2G3qG");
	Play("sC!G!CG!!etA!DD2sG!3G2G3G!F!A!!FADD!!DD!qGsB2G3B2G3qG");
	Play("eG!2G3qEeAsA2G3qAsEAEAD!E!!DEqFeA!2G");
	Play("3G!2G3qEeAsA2G3qAsEAEAD!E!!DEqFeA!2G");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
