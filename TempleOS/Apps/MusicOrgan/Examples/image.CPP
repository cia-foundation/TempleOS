//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  music_tempo=2.5;
  try {
  task=PopUpViewStr(
    "Asking God for stuff is like knocking on the door\n"
    "of a neighbor in the middle of the night.\n"
    "It's annoying!  We're supposed to love God!\n"
    "$LK-A,"BF:Luke,11:5"$\n"
    );
  TaskWait(task);
  SetWinHorz(Fs->win_left,Fs->win_right,task);
  SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
  while (!ScanKey) {
	Play("3qBBE2GG3FFC",
		"We \0are \0in \0the \0image \0of \0God.\n\0 \0");
	Play("BBE2GG3FFC",
		"Some \0people's \0prayer \0is \0ver\0y \0odd.\n\0 \0");
	Play("eGDqCB2G3eGEqEC2G",
		"They \0don't \0love \0Him.\n\0 \0They \0love \0their \0neigh\0bor.\n\0");
	Play("3eGDqCB2G3eGEqEC2G",
		"Pest\0er\0ing \0Him.\n\0 \0Ask\0ing \0for \0fav\0ors.\n\0");
  }
  } catch
    CatchAll;
  Snd(0);
}

Song;
