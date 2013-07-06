//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qAsDGDGqAsCACAeCBsFDFDetCGAsADAD",
		" \0I\0mag\0 \0ine.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qAsDGDGqAsCACAeCBsFDFDetCGAsADAD",
		" \0 \0Fa\0 \0thom.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("GFGFEBEBeCB2qG3sFAFAqCEA",
		"Hi-\0 \0i-\0gh \0as \0 \0your \0 \0thoughts \0will \0go.\n\0 \0 \0 \0 \0 \0 \0 \0");
	Play("sGFGFEBEBeCB2qG3sFAFAqCEA",
		"Deep \0 \0 \0as \0 \0 \0we \0 \0ought \0to \0know.\n\0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
