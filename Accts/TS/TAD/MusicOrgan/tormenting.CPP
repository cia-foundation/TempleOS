//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3eCBBCBCCEEAGGAAsEAAE",
		"I'm \0 \0 \0in \0 \0trou\0ble.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("eCBBCBCCEEAGGAAsEAAE",
		"Bursted \0 \0 \0my \0 \0bub\0ble.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	"$$GREEN$$God talking...\n$$FG$$";
	Play("2qG3eDDsCECE2qGG3eFAetEEEsCAAC",
		" \0$$RED$$Yep, \0 \0you're \0 \0 \0 \0screwed.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("2qG3eDDsCECE2qGG3eFAetEEEsCAAC",
		" \0Re\0 \0pent, \0 \0 \0 \0dude.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
