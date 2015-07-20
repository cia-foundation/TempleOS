//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      Play("3etDGDqGeCGetAE2G3eEFsGBGBqEG",
	    "Thanks, \0 \0 \0God!  \0 \0 \0"
	    "You \0 \0are \0might\0y \0 \0good \0 \0to \0me.\n\0 \0");
      Play("etDGDqGeCGetAE2G3eEFsGBGBqEG",
	    "Thanks, \0 \0 \0God!  \0 \0 \0"
	    "You \0 \0are \0might\0y \0 \0good \0 \0to \0me.\n\0 \0");
      Play("EsBGBGeEC2G3AqF2G3sC2G3C2GqG",
	    " \0 \0 \0 \0 \0 \0 \0"
	    "I \0won't \0tell.\n\0 \0 \0 \0 \0 \0 \0");
      Play("3EsBGBGeEC2G3AqF2G3sC2G3C2GqG",
	    " \0 \0 \0 \0 \0 \0 \0"
	    "I \0won't \0tell.\n\0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
