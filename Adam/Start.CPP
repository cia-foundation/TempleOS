U0 Start()
{
  Type("::/Doc/Splash.TXT");
  SetWinHorz(TEXT_COLS/4-1,3*TEXT_COLS/4);
  SetWinVert(6,Fs->win_bottom);
  "$$GREEN$$Tip of day$$FG$$\n";
  TipOfDay;
  "$$GREEN$$<SHIFT-ESC>$$FG$$ to exit.\n";
  View;
}

Start;
