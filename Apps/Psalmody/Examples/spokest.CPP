//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qB2e.G3sADCeDsGEeBqGe.DsGeE2G3GsBC",
		"God \0spoke.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0How \0 \0'bout \0that!\n\0 \0 \0");
	Play("qB2e.G3sADCeDsGEeBqGe.DsGeE2G3GsBC",
		"God \0spoke.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0Where's \0 \0He \0at?\n\0 \0 \0");
	"$$GREEN$$God talking...\n$$FG$$";
	Play("eFBBsDAECeCqB2e.G3sAF2G3eFe.GsBeDD",
		" \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0$$RED$$I'm \0 \0right \0here.\n\0 \0 \0 \0");
	Play("FBBsDAECeCqB2e.G3sAF2G3eFe.GsBeDD",
		" \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0You \0 \0can \0hear!$$FG$$\n\0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
