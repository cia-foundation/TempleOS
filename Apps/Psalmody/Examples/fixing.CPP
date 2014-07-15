//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.758;
  try {
  while (!ScanKey) {
	Play("3sDDeBCsAAeB2G3A2F3sCCeDsFFeEsFFEEeDE");
	Play("sDDeBCsAAeB2G3A2F3sCCeDsFFeEsFFEEeDE");
	Play("sGBGB2etG3EGqFED2G3eBADG");
	Play("sGBGB2etG3EGqFED2G3eBADG");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
