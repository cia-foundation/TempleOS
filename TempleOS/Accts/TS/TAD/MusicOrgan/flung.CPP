//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.714;
  try {
  while (!ScanKey) {
	Play("3sBBBBEBDBeEBqDeBBEDEBqD");
	Play("sFEFEeFEDBqDeEFEBABqD");
	Play("eDGqEeBAABqFeBDqEA");
	Play("eDGqEeBAABqFeBDqEA");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
