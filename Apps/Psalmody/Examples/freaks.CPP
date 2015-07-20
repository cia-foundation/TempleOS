//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 1.930;
  music.stacatto_factor= 0.604;
  try {
    while (!ScanKey) {
      Play("3sEqAsDeCqAsEqAsDeCqAsDeEqEsE",
	    "Jes\0us \0kind \0of \0freaks,\n\0"
	    "Jes\0us \0kind \0of \0freaks,\n\0are \0dif\0ferent \0 \0");
      Play("eDqCsDeCqBsBeBqAetBEDqBeCB",
	    "from \0 \0oth\0er\n\0 \0 \0"
	    "peo\0ple.\n\0 \0 \0 \0 \0They \0don't \0");
      Play("2qGeG3FsCDCDqFetBACBEDqBeCB",
	    "care,\n\0for \0God \0 \0is \0"
	    "their \0 \0joy!\n\0 \0 \0 \0 \0 \0 \0 \0They \0don't \0");
      Play("2qGeG3FsCDCDqFetBAC",
	    "care,\n\0for \0God \0 \0is \0"
	    "their \0 \0joy!\n\0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
