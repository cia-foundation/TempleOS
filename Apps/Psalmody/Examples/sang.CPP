//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3eCGFsDAe.C2sG3eGDqCDeGsGG2qG");
	Play("3eCGFsDAe.C2sG3eGDqCDeGsGG2qG");
	Play("3eGECGCAFCsCBCBe.GsG2qG3B");
	Play("eGECGCAFCsCBCBe.GsG2qG3B");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
