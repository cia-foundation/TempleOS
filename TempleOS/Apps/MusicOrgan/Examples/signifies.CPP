//8 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music_tempo= 2.480;
  music_stacatto_factor= 0.902;
  try {
  task=PopUpViewStr(
    "$LK,"Luke,12:12","BF:Luke,12:12"$\n"
    );
  TaskWait(task);
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
  while (!ScanKey) {
	Play("W13hDeDA2qG3hDeD2GqF",
		"It \0sig\0ni\0fies\n\0a \0big \0sur\0prise.\n\0");
	Play("3hDeD2sFG3qAhDeDA2qG",
		"Why \0 \0im\0pro\0vise?\n\0$$FG,RED$$\"Be\0cause,\"$$FG$$ \0re\0plies.\n\0");
	Play("3eC!!EsACACeD2G3qA!AFeE2G!3etFCC",
		"If \0 \0you \0 \0 \0 \0love \0your \0god,\n\0 \0don't \0bore \0 \0Him!\n\0 \0 \0");
	Play("eC!!EsACACeD2G3qA!AFeE2G!3etFCC",
		"If \0 \0you \0 \0 \0 \0love \0your \0god,\n\0 \0sur\0prise \0 \0Him!\n\0 \0 \0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
