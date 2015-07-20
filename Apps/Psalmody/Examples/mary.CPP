//8 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.976;
  music.stacatto_factor= 0.538;
  try {
    while (!ScanKey) {
      Play("3qEDCDeEEqEeDDqD");
      Play("EDCDeEEqEeDDqD");
      Play("eDAsFDFDqFsFBFB2qG3etBBBqFC");
      Play("eDAsFDFDqFsFBFB2qG3etBBBqFC");
    }
  } catch
    PutExcept;
  Snd(0);
}
 
Song;
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
