U0 DrawIt(CTask *task,CDC *dc)
{
  SND_OUT_CONTAINER *buf=_hda->ostr0_buf[0];
  I64 i,x1,y1,x2,y2,
    cy=task->win_pixel_height>>1;

  dc->color=BLUE;
  x2=0;y2=cy;
  for (i=0;i<SND_BUF_LEN;i+=2) {
    x1=i*task->win_pixel_width/SND_BUF_LEN;
    y1=cy-buf[i]*cy/snd_vol/MAX_I32;
    GrLine(dc,x2,y2,x1,y1);
    x2=x1;y2=y1;
  }

  dc->color=GREEN;
  x2=0;y2=cy;
  for (i=1;i<SND_BUF_LEN;i+=2) {
    x1=i*task->win_pixel_width/SND_BUF_LEN;
    y1=cy-buf[i]*cy/snd_vol/MAX_I32;
    GrLine(dc,x2,y2,x1,y1);
    x2=x1; y2=y1;
  }
}

U0 SndScope()
{
  if (snd_dev!=SD_HD_AUDIO) {
    "Only works for HD Audio.\n";
    return;
  }
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  SetWinHorz(1,18);
  SetWinVert(15,TEXT_ROWS-2);
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  GetChar;
  SettingsPop;
}

//This is so I can use this file stand alone
//or as a component of a program.
#if IncludeDepth<2
SndScope;
#endif
