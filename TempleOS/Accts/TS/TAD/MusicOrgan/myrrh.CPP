//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13eFDGsBReCB2sGRGR!3qEetG!AGeG!sF!!GeRsDG");
	Play("eFDGsBReCB2sGRGR!3qEetG!AGeG!sF!!GeRsDG");
	Play("FAFAqG!eEEqBetE!!2GR!3eFsARe.BsCe.RsE");
	Play("FAFAqG!eEEqBetE!!2GR!3eFsARe.BsCe.RsE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
