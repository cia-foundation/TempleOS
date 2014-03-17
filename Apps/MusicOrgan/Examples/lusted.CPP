//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qCeBGqFD2G3eBsBGe.AsEeAG");
	Play("qCeBGqFD2G3eBsBGe.AsEeAG");
	Play("CCDqBsCRDBDB2eG3FCFqBsCCDD");
	Play("qCeDqBsCRDBDB2eG3FCFqBsCCDD");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
