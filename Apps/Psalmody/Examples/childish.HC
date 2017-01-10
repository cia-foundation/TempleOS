//9 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    while (!ScanKey) {
      Play("5qEsCDCDqCDeEGsE4B5E4B5eC4B5sD4A5D4A",
	    "Be \0like \0 \0a \0 \0child \0 \0 \0 \0"
	    "some \0 \0re\0 \0spect.\n\0 \0 \0 \0 \0 \0");
      Play("5qEsCDCDqCDeEGsE4B5E4B5eC4B5sD4A5D4A",
	    "Ma\0tur\0 \0i\0 \0ty \0 \0 \0 \0don't \0 \0"
	    "ne\0 \0glect.\n\0 \0 \0 \0 \0 \0");
      Play("4eB5DqF4sGAGAqAAA5etD4AA5sD4B5D4B");
      Play("4eB5DqF4sGAGAqAAA5etD4AA5sD4B5D4B");
    }
  } catch
    PutExcept;
  Snd;
}

Song;
