//8 has words
U0 Song()
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo=2.5;
  try {
    while (!ScanKey) {
      Play("qRsR2GRGeGR3qA2etG3EEqEeDCB2G",
	    " \0 \0He \0 \0is \0 \0 \0our \0 \0 \0 \0"
	    "per\0son\0al \0sav\0ior.\n\0");
      Play("qRsRGRGeGR3qA2etG3EEqEeDCB2G",
	    " \0 \0We \0 \0can \0ne\0 \0ver \0 \0 \0 \0"
	    "re\0turn \0the \0fav\0or.\n\0");
      Play("3CBBGsF2G3F2G3qBsBBBB2eG3AetBBGeBF",
	    " \0 \0 \0That's \0 \0O.\0 \0K. \0 \0 \0 \0"
	    " \0 \0 \0 \0works \0 \0 \0for \0me.\n\0");
      Play("CBBGsF2G3F2G3qBsBBBB2eG3AetBBGeBF",
	    " \0 \0 \0That's \0 \0O.\0 \0K. \0 \0 \0 \0"
	    " \0 \0It's \0 \0works \0 \0 \0for \0me.\n\0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
