//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.802;
  try {
  while (!ScanKey) {
	DocClear;
	Play("3qDsCCCCeBBCCqDsCCCCeBBCC",
		"I'm \0 \0 \0 \0 \0on \0my \0way.\n\0 \0Got \0a \0 \0 \0 \0bus\0y \0day.\n\0 \0");
	Play("sBBBBeAAsBA2G3AqAsBBBBeAAsBA2G3AqA",
		"Woe, \0 \0 \0 \0I \0for\0got \0 \0to \0 \0pray.\n\0What's \0 \0 \0 \0God \0 \0have \0 \0to \0 \0say?\n\0");
	"$$FG,GREEN$$God talking...\n$$FG$$";
	Play("AeB2G3GGqE2etG3FBeFGqFeEC",
		"$$FG,RED$$Oh, \0 \0 \0hi-\0a \0you!\n\0What\0ch\0u \0been \0 \0do\0ing?\n\0 \0");
	Play("qAeB2G3GGqE2etG3FBeFGqFeEC",
		"Oh, \0 \0 \0hi-\0a \0you!\n\0What\0ch\0u \0been \0 \0do\0ing?$$FG$$\n\0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
