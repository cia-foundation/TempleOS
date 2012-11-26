class MPCtrl
{
  CTask *win_task;
  I64 i;
  Bool app_done,app_done_ack;
  Bool refresh_start,refresh_done;
} mp;


U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i;
  F64 t0=tNP,t1;

  mp.refresh_done=FALSE;
  mp.refresh_start=TRUE;
  mp.i++;

  for (i=0;i<32;i++) {
    t1=t0+i*2*ã/64;
    dc->color=(mp.i+i)/10&15;
    GrEllipse(dc,GR_WIDTH/2,GR_HEIGHT/2,
      GR_WIDTH/4*(Sin(t1)+1.0),GR_HEIGHT/4*(Sin(t1)+1.0),t1*2.0);
  }
  while (!mp.refresh_done)
    Yield;
}

U0 MPDrawIt()
{
  I64 i;
  F64 t0=tNP,t1;
  CDC *dc=DCAlias;
  dc->win_task=mp.win_task;

  while (!mp.refresh_start && !mp.app_done)
    Yield;
  if (mp.app_done)
    goto mp_done;
  mp.refresh_start=FALSE;
  DCFill;
  for (i=32;i<64;i++) {
    t1=t0+i*2*ã/64;
    dc->color=(mp.i+i)/10&15;
    GrEllipse(dc,GR_WIDTH/2,GR_HEIGHT/2,
      GR_WIDTH/4*(Sin(t1)+1.0),GR_HEIGHT/4*(Sin(t1)+1.0),t1*2.0);
  }
mp_done:
  DCDel(dc);
  mp.refresh_done=TRUE;
}

U0 MPTask(I64)
{
  while (!mp.app_done)
    MPDrawIt;
  mp.app_done_ack=TRUE;
  DCFill;
}


U0 LayersSpawn()
{
  if (mp_cnt<2) throw(EXCEPT_MULTICORE);

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  WinMax;
  DocClear;
  Fs->draw_it=&DrawIt;
  WinBorder;

  mp.i=0;
  mp.app_done=FALSE;
  mp.win_task=Fs;
  MPSpawn(&MPTask);
  GetChar;
  mp.app_done=TRUE;
  while (!mp.app_done_ack)
    Yield;
  SettingsPop;
}

LayersSpawn;
