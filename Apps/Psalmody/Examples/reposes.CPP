//9 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 1.820;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3eC2G3DDEDqCeEEGFGFqE",
		"Sit\0ting \0 \0 \0on \0the \0fence,\n\0is \0 \0 \0 \0no \0de\0fence.\n\0");
	Play("sFFeGFEFEqDsCCeDCBqBB",
		"Is \0this \0seat \0 \0 \0tak\0 \0en?\n\0An\0 \0y\0one \0op\0 \0pose?\n\0");
	Play("etB2GG3qE2eG3FEEetFBAqFDetFCC",
		"Take \0 \0a \0step.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("B2GG3qE2eG3FEEetFBAqFDetFCC",
		"Take \0 \0the \0step,\n\0 \0 \0 \0 \0 \0 \0 \0not \0back.\n\0 \0 \0 \0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
