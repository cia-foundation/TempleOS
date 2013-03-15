//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.846;
  try {
  while (!ScanKey) {
	Play("W13sEAE!AqE!!eFsG!A!qFsF!G!!eE2qG!3sFEFEqA");
	Play("sEAE!AqE!!eFsG!A!qFsF!G!!eE2qG!3sFEFEqA");
	Play("EeGDqBeCE!2qG3etDG!!C!!eEAFA");
	Play("qEeGDqBeCE!2qG3etDG!!C!!eEAFA");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
