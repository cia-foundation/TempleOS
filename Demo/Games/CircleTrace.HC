//Practice tracing circles

//Giotto, a famous artist, drew a freehand circle to get a job.

RegDft("TempleOS/CircleTrace","F64 best_score=999;\n");
RegExe("TempleOS/CircleTrace");

I64 cx,cy;
F64 avg_error=0,elapsed_time=0,total_error=0,score=999;

U0 SongTask(I64)
{
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  while (TRUE)
    Play("5hEDC4A5RCDECR");
}

U0 DrawIt(CTask *task,CDC *dc)
{
  GrPrint(dc,0,0,"Error:%6.3f  Time:%6.3f  Score:%6.3f  Best:%8.3f",
	avg_error,elapsed_time,score,best_score);
}

#define CIRCLE_RADIUS	100
#define CIRCUMFERENCE	(2*ã*CIRCLE_RADIUS)

Bool CTPlot(CDC *dc,I64 x,I64 y,I64)
{
  F64 rad=Sqrt(Sqr(x-cx)+Sqr(y-cy));
  GrPlot(dc,x,y);
  total_error+=Abs(rad-CIRCLE_RADIUS);
  return TRUE;
}

U0 CircleTrace()
{
  I64 msg_code,arg1,arg2,x1,y1;
  F64 rad,t0,total_distance;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;

  cx=Fs->pix_width/2;
  cy=Fs->pix_height/2;

  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS-WIF_SELF_BORDER;
  Fs->draw_it=&DrawIt;

  DCFill;
  dc->color=ROP_XOR+BLACK^TRANSPARENT;
  GrCircle(dc,cx,cy,CIRCLE_RADIUS);
  do {
    msg_code=GetMsg(&arg1,&arg2,
	  1<<MSG_KEY_DOWN+1<<MSG_MS_L_DOWN);
    switch (msg_code) {
      case MSG_KEY_UP:
	break;
      case MSG_MS_L_DOWN:
	DCFill;
	dc->color=ROP_XOR+BLACK^TRANSPARENT;
	GrCircle(dc,cx,cy,CIRCLE_RADIUS);
	dc->color=ROP_XOR+RED^TRANSPARENT;
	t0=tS;
	total_distance=0.001;
	total_error=0;
	x1=arg1; y1=arg2;
	do {
	  msg_code=ScanMsg(&arg1,&arg2,
		1<<MSG_MS_L_UP+1<<MSG_MS_MOVE);
	  switch (msg_code) {
	    case MSG_MS_L_UP:
	      break;
	    case MSG_MS_MOVE:
	      if (x1!=arg1 || y1!=arg2) {
		total_distance+=Sqrt(Sqr(x1-arg1)+Sqr(y1-arg2));
		Line(dc,x1,y1,0,arg1,arg2,0,&CTPlot);
		x1=arg1;y1=arg2;
//undo overlapping pixs on segments
		GrPlot(dc,x1,y1);
		rad=Sqrt(Sqr(x1-cx)+Sqr(y1-cy));
		total_error-=Abs(rad-CIRCLE_RADIUS);
	      }
	      break;
	  }
	  elapsed_time=tS-t0;
	  avg_error=total_error/total_distance;
	  score=elapsed_time*avg_error;

	  //$LK,"Sleep",A="MN:Sleep"$()s until the next time the
	  //window mgr task runs.  The
	  //window mgr calls the UpdateWin() routine
	  //and places msgs in the que, so there's
	  //no need to do anything until the window mgr runs.
	  Refresh;

	} while (msg_code!=MSG_MS_L_UP);
	music.mute=TRUE; Sleep(200);
	if (total_distance>0.95*CIRCUMFERENCE) {
	  if (score<best_score) {
	    Snd(86);Sleep(50);Snd;Sleep(50);
	    Snd(86);Sleep(50);Snd;Sleep(50);
	    Snd(86);Sleep(50);
	    best_score=score;
	  } else {
	    Snd(62);Sleep(50);
	  }
	} else {
	  Snd(34);Sleep(1000);
	}
	Snd; Sleep(200); music.mute=FALSE;
	break;
    }
  } while (msg_code!=MSG_KEY_DOWN || arg1!=CH_SHIFT_ESC && arg1!=CH_ESC);
  GetMsg(,,1<<MSG_KEY_UP);
  SettingsPop;
  DCFill;
  DCDel(dc);
  RegWrite("TempleOS/CircleTrace","F64 best_score=%5.4f;\n",best_score);
}

CircleTrace;  //Execute when #included
