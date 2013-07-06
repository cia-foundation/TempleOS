//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.976;
  music_stacatto_factor= 0.538;
  try {
  while (!ScanKey) {
	Play("3qEDCDeEEqEeDDqD");
	Play("EDCDeEEqEeDDqD");
	Play("eDAsFDFDqFsFBFB2qG3etBBBqFC");
	Play("eDAsFDFDqFsFBFB2qG3etBBBqFC");
  }
  } catch
    CatchAll;
  Snd(0);
}
 
Song;
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
