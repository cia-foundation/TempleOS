//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13qEsCDCDqCDeEGsEBEBeCBsDADA",
		"Be \0like \0 \0a \0 \0child \0 \0 \0 \0some \0 \0re\0 \0spect.\n\0 \0 \0 \0 \0 \0");
	Play("qEsCDCDqCDeEGsEBEBeCBsDADA",
		"Ma\0tur\0 \0i\0 \0ty \0 \0 \0 \0don't \0 \0ne\0 \0glect.\n\0 \0 \0 \0 \0 \0");
	Play("eBDqF2sG3A2G3AqAAAetDAAsDBDB");
	Play("eBDqF2sG3A2G3AqAAAetDAAsDBDB");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
