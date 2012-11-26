#if snd_dev==SD_HD_AUDIO
#include "::/SparrowOS/Demo/Snd/PolyPhonic.CPP.Z"
#endif

U0 Start()
{
  CTask *task;
  Type("::/SparrowOS/Doc/Splash.TXT.Z");
  Fs->win_left=TEXT_COLS/4-1;
  Fs->win_right=3*TEXT_COLS/4;
  Fs->win_top=TEXT_ROWS/4-3;
  TaskDerivedValsUpdate;
  LBts(&Fs->display_flags,DISPLAYf_WIN_ON_TOP);
  "$$FG,GREEN$$Tip of day$$FG$$\n";
  TipOfDay;
  "$$FG,GREEN$$<SHIFT-ESC>$$FG$$ to exit.\n";
#if snd_dev==SD_HD_AUDIO
  PopUp("#include \"::/SparrowOS/Demo/Snd/SndScope.CPP.Z\";",NULL,&task);
  TaskWait(task);
  WinToTop;
  PolyPhonic;
  Kill(task);
#else
  nounusedwarn task;
  View;
#endif
}

Start;
