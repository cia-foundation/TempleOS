U0 Start()
{
  Type("::/TempleOS/Doc/Splash.TXT");
  SetWinHorz(TEXT_COLS/4-1,3*TEXT_COLS/4);
  SetWinVert(6,Fs->win_bottom);
  LBts(&Fs->display_flags,DISPLAYf_WIN_ON_TOP);
  "$$FG,GREEN$$Tip of day$$FG$$\n";
  TipOfDay;
  "$$FG,GREEN$$<SHIFT-ESC>$$FG$$ to exit.\n";
  View;
}

Start;
