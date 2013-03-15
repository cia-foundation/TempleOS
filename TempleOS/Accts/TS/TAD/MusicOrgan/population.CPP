//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.780;
  try {
  while (!ScanKey) {
	Play("W13eCDBCA2FFF3C2F3E2F3A2F3sEEeD");
	Play("F2FFF3qCeF2F3F2FFF3qCeF2F");
	Play("!3qG!sEAEAqB!etDC!F2qG3D!sGDGDetCRC");
	Play("!qG!sEAEAqB!etDC!F2qG3D!sGDGDetCRC");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
