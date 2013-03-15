//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.976;
  music_stacatto_factor= 0.538;
  try {
  while (!ScanKey) {
	Play("W13qEDCDeEEqEeDDqD");
	Play("EDCDeEEqEeDDqD");
	Play("eD!!AsFDF!DqFsFBFB2qG3etBB!BqFC");
	Play("eD!!AsFDF!DqFsFBFB2qG3etBB!BqFC");
  }
  } catch
    CatchAll;
  Snd(0);
}
 
Song;
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
