//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("2q.GeGqG3sBBCB2q.GeGqG3sBBCB",
		"I'm \0a\0lone.\n\0$$RED$$No, \0you're \0 \0not.\n$$FG$$\0I'm \0a\0lone.\n\0$$RED$$No, \0you're \0 \0not.\n\0");
	Play("q.C2eG3CsDC2eGG3qC2G3eCsDC2eGG",
		"You \0are \0not \0a\0lone.\n\0 \0 \0Def\0inately, \0not \0a\0lone.\n\0 \0 \0");
	Play("3sEAEAeCsCReRsDAe.GsBFFFFeEsAGeCsGDeDG",
		"If \0 \0you \0 \0on\0ly \0 \0 \0 \0 \0knew.\n\0 \0His \0 \0 \0 \0Spir\0it \0 \0 \0is \0 \0with \0you.\n\0");
	Play("sEAEAeCsCReRsDAe.GsBFFFFeEsAGeCsGDeDG",
		"If \0 \0you \0 \0on\0ly \0 \0 \0 \0 \0knew.\n\0 \0His \0 \0 \0 \0Spir\0it \0 \0 \0is \0 \0with \0you.\n$$FG$$\0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
