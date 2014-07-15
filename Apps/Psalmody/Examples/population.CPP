//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.780;
  try {
  while (!ScanKey) {
	Play("3eCDBCA2FFF3C2F3E2F3A2F3sEEeD");
	Play("F2FFF3qCeF2F3F2FFF3qCeF2F");
	Play("3qGsEAEAqBetDCF2qG3DsGDGDetCRC");
	Play("qGsEAEAqBetDCF2qG3DsGDGDetCRC");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
