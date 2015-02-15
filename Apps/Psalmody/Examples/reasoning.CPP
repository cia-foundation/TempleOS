//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
  while (!ScanKey) {
	Play("3sF2G3F2G3eGDA2GqG3eEDsAAAA2eG3FsFGFG",
		"In \0 \0the \0 \0twi\0light \0zone,\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("F2G3F2G3eGDA2GqG3eEDsAAAA2eG3FsFGFG",
		"keep \0 \0a \0 \0friend\0ly \0tone.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("C2G3C2G3qFG2GG3sGDGDqGsDCDC",
		"Mind \0 \0in \0 \0knots?  \0 \0 \0 \0Skip \0 \0de \0 \0bait.\n\0 \0 \0 \0 \0");
	Play("C2G3C2G3qFG2GG3sGDGDqGsDCDC",
		"Watch \0 \0those \0 \0thoughts.  \0 \0 \0 \0Head \0 \0on \0 \0straight.\n\0 \0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
