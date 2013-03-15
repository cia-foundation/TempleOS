U0 DrawIt(CTask *,CDC *dc)
{
  I64 i,j;
  U16 *ptr=gr_win_z_buf;
  dc->color=BLACK;
  GrPutS(dc,0,0,"Move the windows around and watch.");
  for (i=0;i<TEXT_ROWS;i++)
    for (j=0;j<TEXT_COLS;j++) {
      dc->color=*ptr++&15;
      GrPlot(dc,j,i+FONT_HEIGHT);
    }
}

U0 ScrollBars()
{
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  GetChar(NULL,FALSE);
  SettingsPop;
}

ScrollBars;
