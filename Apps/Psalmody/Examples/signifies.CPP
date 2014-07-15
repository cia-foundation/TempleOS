//8 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Luke,12:12",A="BF:Luke,12:12"$\n");
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3hDeDA2qG3hDeD2GqF",
	    "It \0sig\0ni\0fies\n\0a \0big \0sur\0prise.\n\0");
      Play("3hDeD2sFG3qAhDeDA2qG",
	    "Why \0 \0im\0pro\0vise?\n\0$$RED$$\"Be\0cause,\"$$FG$$ \0re\0plies.\n\0");
      Play("3eCEsACACeD2G3qAAFeE2G3etFCC",
	    "If \0 \0you \0 \0 \0 \0love \0your \0god,\n\0 \0don't \0bore \0 \0Him!\n\0 \0 \0");
      Play("eCEsACACeD2G3qAAFeE2G3etFCC",
	    "If \0 \0you \0 \0 \0 \0love \0your \0god,\n\0 \0sur\0prise \0 \0Him!\n\0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
