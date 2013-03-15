//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 1.930;
  music_stacatto_factor= 0.604;
  try {
  while (!ScanKey) {
	Play("W13sEqAsDeCqAsEqAsDeCqAsDeEqEsE",
		"Jes\0us \0kind \0of \0freaks,\n\0Jes\0us \0kind \0of \0freaks,\n\0are \0dif\0ferent \0 \0");
	Play("eDqCsDeCqBsBeBqAetBED!qBeC!B",
		"from \0 \0oth\0er\n\0 \0 \0peo\0ple.\n\0 \0 \0 \0 \0They \0don't \0");
	Play("2qGeG!3FsCDC!!DqFetB!ACBED!qBeC!B",
		"care,\n\0for \0God \0 \0is \0their \0 \0joy!\n\0 \0 \0 \0 \0 \0 \0 \0They \0don't \0");
	Play("2qGeG!3FsCDC!!DqFetB!AC",
		"care,\n\0for \0God \0 \0is \0their \0 \0joy!\n\0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
