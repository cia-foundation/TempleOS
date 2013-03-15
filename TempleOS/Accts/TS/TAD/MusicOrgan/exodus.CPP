//0 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("W13e.C2sG3qEe.FsB!eCCsFGeBA!AetAR!!EeE!sCB");
	Play("e.C2sG3qEe.FsB!eCCsFGeBA!AetAR!!EeE!sCB");
	Play("eG!E2qG3sCD!CDe.EsG!!qCsD!CDC!!e.AsReG!E");
	Play("G!E2qG3sCD!CDe.EsG!!qCsD!CDC!!e.AsReG!E");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
