//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      Play("2eG3DGDqGetA2G3B2qG3GCE",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0It's \0ko\0sher.\n\0");
      Play("2eG3DGDqGetA2G3B2qG3GCE",
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0It's \0ko\0sher.\n\0");
      Play("C2G3sGFGFDCDCetE2G3CGFAqDeFC");
      Play("qC2G3sGFGFDCDCetE2G3CGFAqDeFC");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
