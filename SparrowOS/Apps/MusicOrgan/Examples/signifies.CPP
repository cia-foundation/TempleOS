//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13hDeDA2qG3hDeD2GqF");
	Play("3hDeD2sFG3qAhDeDA2qG");
	Play("3eC!!EsACACeD2G3qA!AFeE2G!3etFCC");
	Play("eC!!EsACACeD2G3qA!AFeE2G!3etFCC");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
