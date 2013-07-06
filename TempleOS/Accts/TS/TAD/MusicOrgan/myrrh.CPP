//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3eFDGsBReCB2sGRGR3qEetGAGeGsFGeRsDG");
	Play("eFDGsBReCB2sGRGR3qEetGAGeGsFGeRsDG");
	Play("FAFAqGeEEqBetE2GR3eFsARe.BsCe.RsE");
	Play("FAFAqGeEEqBetE2GR3eFsARe.BsCe.RsE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
