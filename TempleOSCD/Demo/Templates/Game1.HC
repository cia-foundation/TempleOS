U0 DrawIt(CTask *task,CDC *dc)
{
}

U0 AnimateTask(I64)
{
  while (TRUE) {
    Sleep(10);
  }
}

U0 Init()
{
}

U0 CleanUp()
{
}

U0 SongTask(I64)
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("4AB5C");
  }
}

U0 MyGame()
{
  I64 sc;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=YELLOW<<4+BLUE;
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Up(,,SC_CURSOR_UP);"
	"  Down(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"}"
	);

  Init;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE)
      switch (GetKey(&sc)) {
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_LEFT:
	      break;
	    case SC_CURSOR_RIGHT:
	      break;
	    case SC_CURSOR_UP:
	      break;
	    case SC_CURSOR_DOWN:
	      break;
	  }
	  break;
	case '\n':
	  CleanUp;
	  Init;
	  break;
	case CH_SHIFT_ESC:
	case CH_ESC:
	  goto mg_done;
      }
mg_done: //Don't goto out of try
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
}

MyGame;
