//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.758;
  try {
  while (!ScanKey) {
	Play("W13sDDeBCsAAeB2G3A2F3sCCeDsFFeEsFFEEeDE");
	Play("sDDeBCsAAeB2G3A2F3sCCeDsFFeEsFFEEeDE");
	Play("sGBGB2etG3EGqF!!ED2G3eBA!DG");
	Play("sGBGB2etG3EGqF!!ED2G3eBA!DG");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
