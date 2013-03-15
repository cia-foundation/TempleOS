//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.714;
  try {
  while (!ScanKey) {
	Play("W13sBBBBEBDBeEBqDeBBEDEBqD");
	Play("sFEFEeFEDBqDeEFEBABqD");
	Play("eDGqEeBAABqF!eBD!qE!A");
	Play("eDGqEeBAABqF!eBD!qE!A");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
