//7 no nothing
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
  while (!ScanKey) {
	Play("3eCsAAeCsAAeDABRCqEsBBeEDFE");
	Play("CAqCsDDAAeBRCDsCCBBeEAFsEE");
	Play("G2G3G2G3qGeBBBFsD2G3eDsB2G3B2G3eBAsGF2eG");
	Play("3sG2G3G2G3qGeBBBFsD2G3eDsB2G3B2G3eBAsGF2eG");
  }
  } catch
    PutExcept;
  Snd(0);
}

Song;
