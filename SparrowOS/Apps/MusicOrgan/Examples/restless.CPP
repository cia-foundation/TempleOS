//9 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 1.710;
  music_stacatto_factor= 0.736;
  try {
  while (!ScanKey) {
	Play("W13sDDDACCADDACCAe.DsCCC2G3FD2G3FD2G3CCAe.D");
	Play("qD2sG3G2G3G!etGDAeF2GqG3etF!BA!!EACE!CE");
	Play("qD2sG3G2G3G!etGDAeF2GqG3etF!BA!!EACE!CE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
