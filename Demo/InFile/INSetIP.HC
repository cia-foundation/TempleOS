U0 INSetIPDemo()
{
  I64 t,l;
  WinBorder(ON);
  l=Fs->pix_left+Fs->scroll_x;
  t=Fs->pix_top+Fs->scroll_y;
  IPSet(,,,FALSE,FALSE); //Mouse unclicked
  InSetIP(,l-4,t-4);
  InSetIP(20,l+20,t+20,,TRUE);
  IPSet(,,,FALSE,FALSE);
}

INSetIPDemo;
