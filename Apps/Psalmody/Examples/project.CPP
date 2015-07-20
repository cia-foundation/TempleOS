//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("3qGsARARCBCBeACACGCGBAsEA",
	    "I'm \0mak\0 \0ing \0 \0some\0 \0thing \0"
	    " \0 \0 \0 \0 \0for \0 \0God.\n\0 \0 \0 \0 \0");
      Play("qGsARARCBCBeACACGCGBAsEA",
	    "I'm \0mak\0 \0ing \0 \0some\0 \0thing \0"
	    " \0 \0 \0 \0 \0for \0 \0God.\n\0 \0 \0 \0 \0");
      Play("eAsCEqEsDECEqAsDACAeAGFsBDeCB",
	    " \0 \0 \0I \0hope \0 \0He \0 \0 \0likes \0"
	    " \0it.\n\0 \0 \0 \0 \0 \0 \0 \0 \0");
      Play("AsCEqEsDECEqAsDACAeAGFsBDeCB",
	    " \0 \0 \0I \0hope \0 \0He \0 \0 \0likes \0"
	    " \0it.\n\0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
