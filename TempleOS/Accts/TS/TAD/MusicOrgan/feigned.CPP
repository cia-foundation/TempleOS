//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.857;
  try {
  while (!ScanKey) {
	Play("W13eEDEFqDDeEDEDqCC",
		"$$CL$$Sit\0ting \0in \0a \0church\n\0 \0wond\0ering \0to \0my\0self,\n\0 \0");
	Play("eD2G3BCqDsGGGGeD2G3BCqDsGGGG",
		"\"Is \0 \0God \0 \0real?\"\n\0 \0 \0 \0 \0\"At \0 \0the \0 \0wheel?\"\n\0 \0 \0 \0 \0");
	"$$FG,GREEN$$God talking...\n$$FG$$";
	Play("qF!FFetREA!qG!sG!FG!!F!GEGEqC",
		"$$FG,RED$$Oh, \0Hell \0yes!\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("F!FFetREA!qG!sG!FG!!F!GEGEqC",
		"Pass \0the \0test.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
