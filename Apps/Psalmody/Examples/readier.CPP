//9 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.646;
  music.stacatto_factor= 0.989;
  try {
    while (!ScanKey) {
      Play("2q.G3sCCqAeBB2qG3CeAAqB");
      Play("q.CsCCeCCCCBDDDDDqB",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0Go!\n\0");
      Play("e.AsADCeAetDDDeAsEGeBFsFDFDeEsFACGeC",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0Go!\n\0");
      Play("e.AsADCeAetDDDeAsEGeBFsFDFDeEsFACGeC");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
