//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.857;
  try {
  while (!ScanKey) {
	Play("3eEDEFqDDeEDEDqCC",
		"$$CL$$Sit\0ting \0in \0a \0church\n\0 \0wond\0ering \0to \0my\0self,\n\0 \0");
	Play("eD2G3BCqDsGGGGeD2G3BCqDsGGGG",
		"\"Is \0 \0God \0 \0real?\"\n\0 \0 \0 \0 \0\"At \0 \0the \0 \0wheel?\"\n\0 \0 \0 \0 \0");
	"$$GREEN$$God talking...\n$$FG$$";
	Play("qFFFetREAqGsGFGFGEGEqC",
		"$$RED$$Oh, \0Hell \0yes!\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("FFFetREAqGsGFGFGEGEqC",
		"Pass \0the \0test.$$FG$$\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
