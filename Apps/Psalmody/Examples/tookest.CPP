//7 has words
U0 Song()
{
  CTask *task;
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 2.480;
  music.stacatto_factor= 0.902;
  try {
    task=PopUpViewPrint("$LK,"Luke,17:36",A="BF:Luke,17:36"$\n");
    TaskWait(task);
    WinHorz(Fs->win_left,Fs->win_right,task);
    WinVert(Fs->win_bottom+3,Fs->win_bottom+15,task);
    while (!ScanKey) {
      Play("3qABeCq.AqCDeEq.A",
	    "Pile \0of \0clo\0ths.\n\0Pile \0of \0clo\0ths.\n\0");
      Play("qDeDq.CqBABAR",
	    "Has \0the \0rap\0ture \0come \0and \0gone?\n\0 \0");
      Play("EeD2sG3DeB2G3qFCeCsC2G3etGB2G3qE",
	    "Dear \0God, \0 \0 \0am \0I \0here?\n\0"
	    " \0 \0 \0 \0 \0 \0 \0 \0");
      Play("EeG2sG3EeCA2qG3CeCsC2G3EEAAeEE",
	    "Dear \0God, \0 \0 \0am \0I \0here?\n\0"
	    " \0 \0 \0 \0 \0 \0 \0 \0 \0 \0");
    }
  } catch
    PutExcept;
  Snd(0);
}

Song;
