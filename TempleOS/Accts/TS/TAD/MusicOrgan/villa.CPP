//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("4etEEEqBetDDqtDqBetCCqtCqBhB");
	Play("etEEEqBetDDqtDqBetCCqtCqBhB");
	Play("etABCqDetBCCqDetDCBqAhA");
	Play("etABCqDetBCCqDetDCBqAhA");
	Play("qAetCAEsGCGCetBC3G4ADAqAGetDAD");
	Play("qAetCAEsGCGCetBC3G4ADAqAGetDAD");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
