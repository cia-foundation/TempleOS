//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("M3/4");
	Play("3eECqAAeECqAA",
		"I \0hate \0you.\n\0 \0I \0hate \0you.\n\0 \0");
	Play("eDCqAAeECqAR",
		" \0 \0 \0 \0I \0hate \0you.\n\0 \0");
	Play("M6/8");
	Play("etABGqCsGEGEetGBDqCsCDCD",
		"$$RED$$Don't \0 \0 \0smile.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("etABGqCsGEGEetGBDqCsCDCD",
		"Don't \0 \0 \0smile.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
