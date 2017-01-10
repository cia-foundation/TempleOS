U0 InSetMsDemo()
{
  I64 t,l;
  WinBorder(ON);
  l=Fs->pix_left+Fs->scroll_x;
  t=Fs->pix_top+Fs->scroll_y;
  MsSet(,,,FALSE,FALSE); //Mouse unclicked
  InSetMs(,l-4,t-4);
  InSetMs(20,l+20,t+20,,TRUE);
  MsSet(,,,FALSE,FALSE);
}

InSetMsDemo;
