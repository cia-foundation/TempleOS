#if snd_dev==SD_HD_AUDIO
#include "::/TempleOS/Demo/Snd/PolyPhonic"
#endif

U0 Start()
{
  CTask *task;
  Type("::/TempleOS/Doc/Splash.TXT");
  SetWinHorz(TEXT_COLS/4-1,3*TEXT_COLS/4);
  SetWinVert(TEXT_ROWS/4-3,Fs->win_bottom);
  LBts(&Fs->display_flags,DISPLAYf_WIN_ON_TOP);
  "$$FG,GREEN$$Tip of day$$FG$$\n";
  TipOfDay;
  "$$FG,GREEN$$<SHIFT-ESC>$$FG$$ to exit.\n";
#if snd_dev==SD_HD_AUDIO
  PopUp("#include \"::/TempleOS/Demo/Snd/SndScope\";",NULL,&task);
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
