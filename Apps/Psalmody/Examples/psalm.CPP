//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 3.360;
  music.stacatto_factor= 0.846;
  try {
    while (!ScanKey) {
      Play("M5/4");
      Play("2qGGG3qtCBA2qGGG3qtCetBBqtA",
	    "Hymn's \0not \0long.\n\0 \0 \0 \0"
	    "Nor \0this \0song.\n\0 \0 \0 \0 \0");
      Play("qEeCCqDqtCBAM4/4",
	    "For\0 \0 \0give!\n\0 \0 \0 \0");
      Play("qDeA2G3DG2G3DFFAC2qG3eAE",
	    "That's \0 \0 \0all \0 \0I \0 \0say.\n\0 \0 \0 \0 \0 \0 \0");
      Play("qDeA2G3DG2G3DFFAC2qG3eAE",
	    "This \0 \0 \0I \0 \0 \0 \0pray.\n\0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
