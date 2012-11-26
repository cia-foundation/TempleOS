#define STEPS	360

U0 DrawIt(CTask *task,CDC *dc)
{
  F64 x1,x2,y1,y2,radius,peddles,é,t=tMBeat,
      cx=Fs->win_pixel_width>>1,cy=Fs->win_pixel_height>>1;
  Bool first=TRUE;

  radius=100*Sin(2*ã*t)+150;
  peddles=ToI64(t/8%4)+4;

  dc->color=BLACK;
  for (é=0;é<2*ã;é+=2*ã/STEPS) {
    P2R(&x2,&y2,radius*Sin(peddles*é),é+t);
    if (!first) {
      GrLine(dc,x1+cx,cy-y1,x2+cx,cy-y2);
    } else
      first=FALSE;
    x1=x2; y1=y2;
  }

  dc->color=(t)%16;
  for (é=(2*ã/peddles)/8;é<2*ã;é+=2*ã/peddles/2) {
    P2R(&x1,&y1,radius/2*Sin(peddles*é),é+t);
    GrFloodFill(dc,x1+cx,cy-y1);
  }
}

U0 SongTask(I64)
{ //Song by Terry A. Davis
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE) {
    Play("W13eGGsGGeGGGe.GeCDsEDeCBAe.AeG");
    Play("GsGGeGGGe.GeFEsDEeDCBe.AeFD");
    Play("EDFDEDECDCECDCsGGeG");
    Play("GG2hG3sGGeGGG2hG");
  }
}

U0 Rose()
{
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->song_task=Spawn(&SongTask,NULL,"Song",Fs);
  WordStat;
  WinMax;
  DocClear;
  Fs->draw_it=&DrawIt;
  PressAKey;
  SettingsPop;
}

Rose;