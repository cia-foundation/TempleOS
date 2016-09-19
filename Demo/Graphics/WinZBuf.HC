U0 DrawIt(CTask *,CDC *dc)
{
  I64 i,j;
  U16 *ptr=gr.win_z_buf;
  dc->color=BLACK;
  GrPrint(dc,0,0,"Move the windows around and watch.");
  for (i=0;i<TEXT_ROWS;i++)
    for (j=0;j<TEXT_COLS;j++) {
      dc->color=*ptr++&15;
      GrPlot(dc,j,i+FONT_HEIGHT);
    }
}

U0 ScrollBars()
{
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  DocClear;
  Fs->draw_it=&DrawIt;
  GetChar(,FALSE);
  SettingsPop;
}

ScrollBars;
