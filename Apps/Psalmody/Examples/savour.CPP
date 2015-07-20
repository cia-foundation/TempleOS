//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("3qFCeEAEAqDeCEqBA",
	    "This \0 \0 \0 \0 \0 \0sand\0wich \0is \0bland.\n\0 \0");
      Play("eCCqDeDDqEeAECECEqA",
	    "God \0 \0damn, \0my \0 \0tastes \0 \0 \0 \0"
	    "sure \0 \0are \0grand.\n\0");
      Play("M6/8");
      Play("FEB2G3sAGeCqE",
	    "Ho \0hum.  \0 \0 \0 \0How \0 \0rude!\n\0");
      Play("FEB2G3sAGeCqE",
	    "That \0bum \0 \0 \0 \0likes \0his \0food.\n\0");
      Play("2G3sAGeDqGFEB",
	    " \0 \0Likes \0his \0food!\n\0 \0 \0 \0");
      Play("2G3sAAeEqD2G3sDBDB2eGG");
      Play("3AsEDqAeAsBGEBEB2qG3sDBDB");
      Play("2eGG3AsEDqAeAsBGEBEB");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
