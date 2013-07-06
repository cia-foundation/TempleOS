//8 has words

U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3qF2GsGGGG3GFGFqAeD2G3etDBEsFCFC",
		"Love \0 \0 \0 \0 \0 \0ev\0 \0ery \0 \0sec\0ond.\n\0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qF2GsGGGG3GFGFqAeD2G3etDBEsFCFC",
		"World -- \0 \0 \0 \0 \0 \0all \0 \0that \0 \0beck\0ons.\n\0 \0 \0 \0 \0 \0 \0 \0 \0");
	Play("qBGFsEAEAqEFGeFC",
		" \0Life \0is \0too \0 \0 \0 \0prec\0i\0ous.\n\0 \0 \0");
	Play("qBGFsEAEAqEFGeFC",
		" \0Can \0be \0 \0 \0 \0 \0worr\0i\0ous.\n\0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
