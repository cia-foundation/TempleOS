//7 has words

F64 t0=tMBeat;

U0 DrawIt(CTask *task,CDC *dc)
{
  static I64 i=0;
  F64 dt=tMBeat-t0;
  if (dt>16.0 && i++&1) {
    dc->color=WHITE;
    GrRect(dc,0,0,task->win_pixel_width,task->win_pixel_height);
  }
}

U0 Song()
{
  CTask *task;
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->task_end_cb=&SndTaskEndCB;

  task=PopUpViewStr(
      "$LK-A,"BF:Acts,9:3"$\n"
      );
  TaskWait(task);
  task->win_right=Fs->win_right;
  task->win_left=Fs->win_left;
  task->win_top=Fs->win_bottom+3;
  task->win_bottom=task->win_top+12;
  TaskDerivedValsUpdate(task);
  music_tempo=2.5;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  try {
    while (!ScanKey) {
      t0=tMBeat;
      Play("3qEeACA2G3qCEC2G3E",
	  "Now... \0 \0 \0 \0 \0I \0see \0the \0light.\n\0 \0");
      Play("EeACA2G3qCEC2G3E",
	  "Now... \0 \0 \0 \0 \0I \0see \0the \0light.\n\0 \0");
      Play("GCGBEeDFCAqD",
	  "It's \0 \0quite \0 \0bright.\n\0 \0 \0 \0 \0 \0");
      Play("GCGBEeDFCAqD",
	  "It's \0 \0quite \0 \0bright.\n\0 \0 \0 \0 \0 \0");
    }
  } catch
    CatchAll;
  SettingsPop;
}

Song;
