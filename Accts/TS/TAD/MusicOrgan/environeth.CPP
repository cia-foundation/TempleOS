//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	"$$GREEN$$To God...$$FG$$\n";
	Play("M5/4");
	Play("3eEqCeEFEDCqBeEqCeEFEDCqB",
		"Fa\0ther, \0for\0give \0them \0if \0you \0must.\n\0I'd \0like \0to \0see \0them \0turn \0to \0dust.\n\0");
	Play("M4/4");
	Play("sGBGBetAF2G3eEFsEGEGqDsBDBDGFGF2etG3GB");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
