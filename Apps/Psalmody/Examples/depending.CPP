//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  music.tempo=2.5;
  try {
    task=PopUpViewPrint("$LK,"BF:Luke,12:16"$\n");
    TaskWait(task);
    SetWinHorz(Fs->win_left,Fs->win_right,task);
    SetWinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3etGREqDDBRGCB",
	    "It \0 \0de\0pends \0 \0on \0 \0the \0wea\0ther.\n\0");
      Play("etGREqDDBRGCB",
	    "If \0 \0we \0reach \0 \0the \0 \0end \0ev\0er.\n\0");
      Play("2sG3E2G3EqGsGBGBqDsFCFCqBRsCBCB",
	    "Hand \0 \0of \0 \0God, \0 \0just \0 \0might, \0 \0in\0 \0ter\0 \0cedes.\n\0 \0 \0 \0 \0 \0");
      Play("2G3E2G3EqGsGBGBqDsFCFCqBRsCBCB",
	    "If \0 \0it's \0 \0not \0 \0deemed \0 \0right, \0 \0won't \0 \0suc\0 \0ceed.\n\0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
