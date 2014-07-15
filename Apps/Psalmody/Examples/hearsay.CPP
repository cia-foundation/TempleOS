//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qAeCDqEAeCDECDECsCC",
		"I \0don't \0 \0know.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qAeCDqEAeCDECDECsCC",
		"Let's \0go \0 \0slow.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("eDAGAqDsECECCBCBeAFsDCeGDE",
		"You \0 \0say \0 \0that, \0God?\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("DAGAqDsECECCBCBeAFsDCeGDE",
		"It \0 \0sounds \0 \0odd.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
