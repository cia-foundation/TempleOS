//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qBAeEEqAeAEDB2qG3F",
		"$$FG,RED$$You \0need \0help!\n\0 \0 \0 \0Po\0-o\0-o\0r \0dear.\n\0");
	Play("BAeEEqAeAEDB2qG3F",
		"You \0need \0help!\n\0 \0 \0 \0Po\0-o\0-o\0r \0dear.\n$$FG$$\0");
	Play("4eFGqDetFEDeDEetADEC3G4GqD3G",
		"Pat\0ron\0ize?  \0No, \0 \0thank \0you!\n\0 \0Too \0 \0man\0y \0strings \0a\0ttach\0ed.\n\0");
	Play("4eFGqDetFEDeDEetADEC3G4GqD3G",
		"Pat\0ron\0ize?  \0No, \0 \0thank \0you!\n\0 \0Too \0 \0man\0y \0strings \0a\0ttach\0ed.\n\0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
