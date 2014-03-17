//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.714;
  try {
  while (!ScanKey) {
	Play("3sBBBBEBDBeEBqDeBBEDEBqD",
		" \0 \0 \0 \0Had \0 \0a \0 \0pass\0ing \0thought.\n\0 \0 \0Not \0a \0thought \0I \0ought.\n\0");
	Play("sFEFEeFEDBqDeEFEBABqD",
		"Can't \0 \0 \0 \0clear \0my \0head \0of \0thought.\n\0In \0this \0trap \0my \0mind \0is \0caught.\n\0");
	Play("eDGqEeBAABqFeBDqEA");
	Play("eDGqEeBAABqFeBDqEA");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
