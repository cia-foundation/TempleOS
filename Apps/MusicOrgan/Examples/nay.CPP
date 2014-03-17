//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  try {
  "$$WW+H,1$$";
  while (!ScanKey) {
	Play("3qAAsG2G3G2G3qGF2eG3FqEetCBA",
		"Let's \0ask \0Mike\0 \0 \0 \0y;  \0Hates \0 \0every\0thing.\n\0 \0 \0 \0");
	Play("qAAsG2G3G2G3qGF2eG3FqEetCBA",
		"Let's \0ask \0Mike\0 \0 \0 \0y;  \0Hates \0 \0every\0thing.\n\0 \0 \0 \0");
	Play("2eG3GqE2eG3FqF2sG3G2G3GqAetGEBqD",
		"What \0do \0YOU \0 \0 \0know.  \0 \0 \0 \0 \0 \0It's \0 \0kosh\0er!\n\0");
	Play("2eG3GqE2eG3FqF2sG3G2G3GqAetGEBqD",
		"What \0do \0YOU \0 \0 \0know.  \0 \0 \0 \0 \0 \0It's \0 \0kosh\0er!\n\0");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
