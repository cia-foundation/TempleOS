//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  try {
  while (!ScanKey) {
    Play("3sEAEAeFFsA2G3A2G3FGFGqCCBA",
	"All \0 \0 \0 \0hell \0 \0breaks \0 \0 \0 \0loose.\n\0 \0 \0 \0I \0will \0get \0through.\n\0");
    Play("sEAEAeFFsA2G3A2G3FGFGqCCBA",
	"All \0 \0 \0 \0hell \0 \0breaks \0 \0 \0 \0loose.\n\0 \0 \0 \0I \0will \0get \0through.\n\0");
    Play("ACeCCsBBBBqEsCCCCqAE");
    Play("ACeCCsBBBBqEsCCCCqAE");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
