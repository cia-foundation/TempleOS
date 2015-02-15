//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("M4/4");
	Play("3qBsEGeBDe.AsCeA2qF3sBBeACBBsBB",
		"I \0could \0 \0 \0die.\n\0 \0 \0 \0 \0 \0 \0 \0Could \0I \0kill?\n\0 \0 \0");
	Play("qBsEGeBDe.AsCeA2qF3sBBeACBBsBB",
		"I'm \0not \0 \0 \0spry.\n\0 \0 \0 \0 \0 \0 \0 \0I \0best \0chill.\n\0 \0 \0");
	Play("eDDsGFGFeEBqGM5/4",
		"If \0 \0God \0 \0 \0 \0 \0 \0asked,\n\0");
	Play("eBGF4A3e.FsEqAAM4/4",
		" \0\"$$RED$$Can \0 \0 \0you \0 \0Kill?$$FG$$\"\n\0 \0");
	Play("eDDsGFGFeEBqGM5/4",
		"I \0 \0think, \0 \0 \0 \0 \0 \0\"Yeah.\"\n\0");
	Play("eBGF4A3e.FsEqAA",
		" \0\"Right \0 \0 \0my \0 \0will.\"\n\0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
