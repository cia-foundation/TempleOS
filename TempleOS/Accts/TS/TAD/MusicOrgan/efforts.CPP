//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  try {
  while (!ScanKey) {
	Play("2qG3sDADAeF2G3etFDCBA2G3qAetGFC2eG3E",
		" \0Let's \0 \0try.  \0 \0 \0 \0O\0oh\0 \0h.  \0 \0 \0 \0Didn't \0 \0 \0work.\n\0 \0");
	Play("2qG3sDADAeF2G3etFDCBA2G3qAetGFC2eG3E",
		" \0Let's \0 \0try.  \0 \0 \0 \0O\0oh\0 \0h.  \0 \0 \0 \0Didn't \0 \0 \0work.\n\0 \0");
	Play("sA2G3A2G3eBCsE2G3E2G3qGsFEFEDBDBeAG2etG3B2G");
	Play("3sA2G3A2G3eBCsE2G3E2G3qGsFEFEDBDBeAG2etG3B2G");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
