//Conway's Game of Life

CDC *dc[2]; //dbl buf
I64 cur_dc,;
Bool restart;

U0 DrawIt(CTask *,CDC *dc2)
{
  dc[cur_dc]->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc2,0,0,dc[cur_dc]);
}

U0 AnimateTask(I64)
{
  I64 x,y,x1,y1,cnt,next_dc;
  while (TRUE) {
    next_dc=cur_dc^1;
start_over:
    restart=FALSE;
    DCClear(dc[next_dc]);
    for (y=1;y<Fs->parent_task->pix_height-1;y++) {
      for (x=1;x<Fs->parent_task->pix_width-1;x++) {
	cnt=0;
	for (y1=y-1;y1<=y+1;y1++)
	  for (x1=x-1;x1<=x+1;x1++)
	    if (GrPeek(dc[cur_dc],x1,y1)==GREEN)
	      cnt++;
	if (restart) goto start_over;
	if (GrPeek(dc[cur_dc],x,y)==GREEN) {
	  if (cnt==3) {
	    dc[next_dc]->color=GREEN;
	    GrPlot(dc[next_dc],x,y);
	  }
	} else {
//	  if (cnt==3) {//Use this for the classic rules
	  if (cnt==2) {
	    dc[next_dc]->color=GREEN;
	    GrPlot(dc[next_dc],x,y);
	  }
	}
      }
      Yield;
    }
    Sleep(50);
    cur_dc=next_dc;
  }
}

public U0 Life()
{
  I64 msg_code,cnt,x1,y1,x2,y2,arg1,arg2;

  dc[0]=DCNew(GR_WIDTH,GR_HEIGHT);
  dc[1]=DCNew(GR_WIDTH,GR_HEIGHT);
  cur_dc=0;
  restart=FALSE;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocClear;
  DCFill;

  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  Fs->draw_it=&DrawIt;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;
  do {
    msg_code=GetMsg(&arg1,&arg2,1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN);
    switch (msg_code) {
      case MSG_MS_L_DOWN:
	x1=arg1; y1=arg2;
	x2=arg1; y2=arg2;
	cnt=0;
	while (msg_code!=MSG_MS_L_UP) {
	  restart=TRUE;
	  dc[cur_dc]->color=GREEN;
	  dc[cur_dc]->thick=0.04*ms.speed;
	  GrLine3(dc[cur_dc],x1,y1,0,x2,y2,0);
	  restart=TRUE;
	  msg_code=GetMsg(&arg1,&arg2,1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
	  x1=x2; y1=y2;
	  x2=arg1; y2=arg2;
	}
	GrLine3(dc[cur_dc],x1,y1,0,x2,y2,0);
	break;
      case MSG_KEY_DOWN:
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || !arg1);
  GetMsg(,,1<<MSG_KEY_UP);
  SettingsPop;
  DCFill;
  DCDel(dc[0]);
  DCDel(dc[1]);
}

Life;
