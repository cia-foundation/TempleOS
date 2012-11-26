//7 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13qA!!sDGD!!GqAsCA!CAeCBsFDF!D!!etCGAsAD!A!!D",
		" \0I\0mag\0 \0ine.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qA!!sDGD!!GqAsCA!CAeCBsFDF!D!!etCGAsAD!A!!D",
		" \0 \0Fa\0 \0thom.\n\0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("!!GF!G!!FE!!BE!BeCB2qG3sFAFAqCEA",
		"Hi-\0 \0i-\0gh \0as \0 \0your \0 \0thoughts \0will \0go.\n\0 \0 \0 \0 \0 \0 \0 \0");
	Play("!!sGF!G!!FE!!BE!BeCB2qG3sFAFAqCEA",
		"Stu-\0 \0u-\0ck \0on \0 \0this \0 \0Earth \0be\0low.\n\0 \0 \0 \0 \0 \0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
