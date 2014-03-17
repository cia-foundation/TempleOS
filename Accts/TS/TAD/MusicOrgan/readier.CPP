//9 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.646;
  music_stacatto_factor= 0.989;
  try {
  while (!ScanKey) {
	Play("2q.G3sCCqAeBB2qG3CeAAqB");
	Play("q.CsCCeCCCCBDDDDDqB");
	Play("e.AsADCeAetDDDeAsEGeBFsFDFDeEsFACGeC");
	Play("e.AsADCeAetDDDeAsEGeBFsFDFDeEsFACGeC");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
