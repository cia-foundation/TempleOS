U0 AFSetIPDemo()
{
  I64 t,l;
  WinBorder(ON);
  l=Fs->win_pixel_left+Fs->win_scroll_x;
  t=Fs->win_pixel_top+Fs->win_scroll_y;
  SetIP(,,,FALSE,FALSE,FALSE); //Mouse unclicked
  AFSetIP(,l-4,t-4);
  AFSetIP(20,l+20,t+20,,TRUE);
  SetIP(,,,FALSE,FALSE,FALSE);
}

AFSetIPDemo;
