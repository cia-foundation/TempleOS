//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3e.C2sG3qEe.FsBeCCsFGeBAAetAREeEsCB");
	Play("e.C2sG3qEe.FsBeCCsFGeBAAetAREeEsCB");
	Play("eGE2qG3sCDCDe.EsGqCsDCDCe.AsReGE");
	Play("GE2qG3sCDCDe.EsGqCsDCDCe.AsReGE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
