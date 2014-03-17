U0 AFSetIPDemo()
{
  I64 t,l;
  WinBorder(ON);
  l=Fs->pix_left+Fs->scroll_x;
  t=Fs->pix_top+Fs->scroll_y;
  SetIP(,,,FALSE,FALSE,FALSE); //Mouse unclicked
  AFSetIP(,l-4,t-4);
  AFSetIP(20,l+20,t+20,,TRUE);
  SetIP(,,,FALSE,FALSE,FALSE);
}

AFSetIPDemo;
