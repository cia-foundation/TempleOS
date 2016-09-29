F64 SpanTime()
{
  if (run_button.state)
    return a.elapsed_t+tS-a.start_wall_t;
  else
    return a.elapsed_t;
}

F64 Cost(CMathODE *ode)
{
  MyMass   *tmpm;
  MySpring *tmps;
  F64 res=0;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    res+=tmpm->cost;
    tmpm=tmpm->next;
  }
  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    res+=tmps->cost;
    tmps=tmps->next;
  }
  return res;
}

U0 DrawIt(CTask *task,CDC *dc)
{
  MyMass   *tmpm;
  MySpring *tmps;

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    if (!(tmps->flags&SSF_INACTIVE)) {
      dc->color=tmps->color;
      dc->pen_width=tmps->pen_width;
      GrLine3(dc,tmps->end1->x,tmps->end1->y,0,
	    tmps->end2->x,tmps->end2->y,0);
    }
    tmps=tmps->next;
  }

  if (cursor_mass) {
    dc->color=RED;
    dc->pen_width=2;
    GrLine3(dc,ip.pos.x-task->pix_left-task->scroll_x,
	  ip.pos.y-task->pix_top-task->scroll_y,0,
	  cursor_mass->x,cursor_mass->y,0);
  }

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    if (!(tmpm->flags&MSF_INACTIVE)) {
      dc->color=BLACK;
      GrCircle(dc,tmpm->x,tmpm->y,tmpm->radius);
      GrFloodFill(dc,tmpm->x,tmpm->y,TRUE);
      dc->color=tmpm->color;
      GrCircle(dc,tmpm->x,tmpm->y,tmpm->radius);
      GrFloodFill(dc,tmpm->x,tmpm->y,TRUE);
      dc->color=BLACK;
      GrCircle(dc,tmpm->x,tmpm->y,tmpm->radius);
    }
    tmpm=tmpm->next;
  }

  dc->color=BLACK;
  GrPrint(dc,90,0,"Cost:%12.2,f",Cost(ode));
  GrPrint(dc,90,FONT_HEIGHT,"Time:%12.2f",SpanTime);
}

MyMass *PlaceMass(I64 x, I64 y)
{
  MyMass *tmpm=CAlloc(sizeof(MyMass));
  tmpm->drag_profile_factor=1.0;
  tmpm->x=x;
  tmpm->y=y;
  tmpm->mass=MASS_MASS;
  tmpm->radius=MASS_RADIUS;
  tmpm->cost=25.0*COST_SCALE;
  tmpm->color=YELLOW;
  QueIns(tmpm,ode->last_mass);
  return tmpm;
}

U0 NullSpring(MySpring *tmps,F64 scale)
{
  F64 d=D3Dist(&tmps->end1->x,&tmps->end2->x);
  tmps->rest_len=d*scale;
  tmps->compression_strength=
	tmps->base_compression_strength/(tmps->rest_len+1.0);
  tmps->tensile_strength=tmps->base_tensile_strength/(tmps->rest_len+1.0);
  tmps->const=tmps->base_const/(tmps->rest_len+1.0);
  tmps->cost=tmps->base_cost*tmps->rest_len;
}

U0 MoveMass(MyMass *tmpm,I64 x, I64 y)
{
  MySpring *tmps;
  tmpm->x=x;
  tmpm->y=y;
  tmpm->DxDt=0;
  tmpm->DyDt=0;
  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    if (tmps->end1==tmpm || tmps->end2==tmpm) {
      if (tmps->flags&SSF_NO_COMPRESSION)
	NullSpring(tmps,WIRE_PERCENT);
      else
	NullSpring(tmps,1.0);
    }
    tmps=tmps->next;
  }
}

U0 DelSpring(MySpring *tmps)
{
  QueRem(tmps);
  Free(tmps);
}

U0 DelMass(MyMass *tmpm)
{
  MySpring *tmps,*tmps1;
  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    tmps1=tmps->next;
    if (tmps->end1==tmpm || tmps->end2==tmpm)
      DelSpring(tmps);
    tmps=tmps1;
  }
  QueRem(tmpm);
  Free(tmpm);
}

U0 DrawSpring(CDC *dc,MyMass *tmpm,I64 x,I64 y)
{
  switch (mode_button.state) {
    case MD_CONCRETE:
      dc->color=LTGRAY;
      dc->pen_width=2;
      break;
    case MD_STEEL:
      dc->color=DKGRAY;
      dc->pen_width=2;
      break;
    case MD_WIRE:
      dc->color=RED;
      dc->pen_width=1;
      break;
  }
  GrLine3(dc,tmpm->x,tmpm->y,0,x,y,0);
}

U0 PlaceSpring(MyMass *tmpm1,MyMass *tmpm2)
{
  MySpring *tmps=CAlloc(sizeof(MySpring));
  tmps->end1=tmpm1;
  tmps->end2=tmpm2;
  switch (mode_button.state) {
    case MD_CONCRETE:
      tmps->base_const	      = 3.00*SPRING_SCALE;
      tmps->base_compression_strength=10.00*STRENGTH_SCALE;
      tmps->base_tensile_strength    = 0.35*STRENGTH_SCALE;
      tmps->base_cost		      = 0.30*COST_SCALE;
      NullSpring(tmps,1.0);
      tmps->color=LTGRAY;
      tmps->pen_width=2;
      break;
    case MD_STEEL:
      tmps->base_const	      = 1.00*SPRING_SCALE;
      tmps->base_compression_strength= 1.00*STRENGTH_SCALE;
      tmps->base_tensile_strength    = 1.00*STRENGTH_SCALE;
      tmps->base_cost		      = 1.00*COST_SCALE;
      NullSpring(tmps,1.0);
      tmps->color=DKGRAY;
      tmps->pen_width=2;
      break;
    case MD_WIRE:
      tmps->base_const	      = 0.25*SPRING_SCALE;
      tmps->base_compression_strength= 0.00;
      tmps->base_tensile_strength    = 0.50*STRENGTH_SCALE;
      tmps->base_cost		      = 0.10*COST_SCALE;
      NullSpring(tmps,WIRE_PERCENT);
      tmps->color=RED;
      tmps->pen_width=1;
      tmps->flags|=SSF_NO_COMPRESSION;
      break;
  }
  QueIns(tmps,ode->last_spring);
}

U0 AnimateTask(SpanAnimateStruct *a)
{
  MySpring *tmps,*tmps1;
  Bool old_run=FALSE;
  F64 f;
  while (TRUE) {
    tmps=ode->next_spring;
    while (tmps!=&ode->next_spring) {
      tmps1=tmps->next;
      f=tmps->f;
      if (f>0 &&  f>tmps->compression_strength &&
	    !(tmps->flags&SSF_NO_COMPRESSION)||
	    f<0 && -f>tmps->tensile_strength &&
	    !(tmps->flags&SSF_NO_TENSION))
	tmps->flags|=SSF_INACTIVE;
      tmps=tmps1;
    }
    AdjustLoads(ode);
    Refresh; //CMathODE updated once per refresh.
    if (old_run!=run_button.state) {
      if (run_button.state) {
	if (!a->elapsed_t || !a->saved_ode) {
	  Free(a->saved_ode);
	  a->saved_ode=SpanSave(ode);
	}
	a->start_wall_t=tS;
	ode->flags&=~ODEF_PAUSED;
      } else {
	ode->flags|=ODEF_PAUSED;
	a->elapsed_t+=tS-a->start_wall_t;
      }
      old_run=run_button.state;
    }
  }
}

U0 Init(SpanAnimateStruct *a)
{
  SpanDel(ode);
  ode=SpanNew;

  run_button.state=0;
  Refresh(2); //Allow stop to reg in animate task.

  if (a->saved_ode)
    SpanLoad(ode,a->saved_ode);
  else
    SpanBridge1Init(ode);
  a->elapsed_t=0;
  cursor_mass=NULL;
}

U0 SongTask(I64)
{//Song by Terry A. Davis
  Fs->task_end_cb=&SndTaskEndCB;
  MusicSettingsRst;
  music.tempo= 3.636;
  music.stacatto_factor= 0.902;
  while (TRUE) {
    Play("3q.EeDqED2G3DhE");
    Play("q.EeDqED2G3DhE");
    Play("q.FeEFEqF2G3EhF");
    Play("q.FeEFEqF2G3EhF");
  }
}

U0 Span()
{
  I64 msg_code,a1,a2;
  MyMass   *tmpm1=NULL,*tmpm2=NULL;
  MySpring *tmps;
  CCtrl *bt_run,*bt_mode;
  U8 *src;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BROWN<<4+BLACK;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);

  bt_run =CtrlButtonNew(0,0,		  80,,
	2,"Stopped\0Running\0",run_colors,&run_button);
  bt_mode=CtrlButtonNew(0,3.0*FONT_HEIGHT,80,,
	MD_NUM_MODES,Define("ST_SPAN_MODES"),mode_colors,&mode_button);
  a.saved_ode=NULL;

  Fs->win_inhibit|=WIG_DBL_CLICK;

  MenuPush(
	"File {"
	"  New(,CH_CTRLN);"
	"  Open(,CH_CTRLO);"
	"  SaveAs(,CH_CTRLA);"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  RunStop(,CH_SPACE);"
	"  Mass(,'m');"
	"  Concrete(,'c');"
	"  Steel(,'s');"
	"  Wire(,'w');"
	"  Move(,'v');"
	"  Delete(,'d');"
	"}"
	);

  ode=NULL;
  Init(&a);
  Fs->animate_task=Spawn(&AnimateTask,&a,"Animate",,Fs);
  Fs->draw_it=&DrawIt;

  PopUpOk(
	"Build a bridge to hold-up the\n"
	"red masses.  Test your design\n"
	"by pressing run/stop.\n\n"
	"The lowest cost bridge that\n"
	"stays standing wins.\n\n"
	"For a variation, try without\n"
	"using the center base point.\n"
	"\n"
	"Use\n"
	"\t$$GREEN$$'m'$$FG$$ass\n"
	"\t$$GREEN$$'c'$$FG$$oncrete\n"
	"\t$$GREEN$$'s'$$FG$$teel\n"
	"\t$$GREEN$$'w'$$FG$$ire\n"
	"\nto sel materials.\n");

  try {
    while (TRUE) {
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_DOWN|1<<MSG_IP_R_DOWN|
	    1<<MSG_IP_L_UP|1<<MSG_KEY_DOWN|1<<MSG_IP_MOVE);
      DCFill(dc);
      switch (msg_code) {
	case MSG_IP_L_DOWN:
	  cursor_mass=tmpm1=tmpm2=NULL;
	  switch (mode_button.state) {
	    case MD_MASS:
	      PlaceMass(a1,a2);
	      break;
	    case MD_CONCRETE:
	    case MD_STEEL:
	    case MD_WIRE:
	      tmpm1=MassFind(ode,a1,a2);
	      break;
	    case MD_MOVE:
	      if (run_button.state)
		cursor_mass=MassFind(ode,a1,a2);
	      else
		if (tmpm1=MassFind(ode,a1,a2))
		  MoveMass(tmpm1,a1,a2);
	      break;
	    case MD_DELETE:
	      MassOrSpringFind(ode,&tmpm1,&tmps,a1,a2);
	      if (tmpm1)
		DelMass(tmpm1);
	      if (tmps)
		DelSpring(tmps);
	      break;
	  }
	  break;
	case MSG_IP_L_UP:
	  switch (mode_button.state) {
	    case MD_CONCRETE:
	    case MD_STEEL:
	    case MD_WIRE:
	      if (tmpm1 && (tmpm2=MassFind(ode,a1,a2)) && tmpm1!=tmpm2)
		PlaceSpring(tmpm1,tmpm2);
	      break;
	    case MD_MOVE:
	      if (!run_button.state && tmpm1)
		MoveMass(tmpm1,a1,a2);
	      break;
	  }
	  cursor_mass=tmpm1=tmpm2=NULL;
	  break;
	case MSG_IP_MOVE:
	  switch (mode_button.state) {
	    case MD_MOVE:
	      if (!run_button.state && tmpm1)
		MoveMass(tmpm1,a1,a2);
	      break;
	    case MD_CONCRETE:
	    case MD_STEEL:
	    case MD_WIRE:
	      if (tmpm1) {
		DrawSpring(dc,tmpm1,a1,a2);
	      }
	      break;
	  }
	  break;
	case MSG_IP_R_DOWN:
	  mode_button.state++;
	  if (mode_button.state>=MD_NUM_MODES)
	    mode_button.state=0;
	  cursor_mass=tmpm1=tmpm2=NULL;
	  break;
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case '\n':
	      if (!SpanTime || !a.saved_ode) {
		Free(a.saved_ode);
		a.saved_ode=SpanSave(ode);
	      }
	      Init(&a);
	      break;
	    case CH_CTRLN:
	      Free(a.saved_ode);
	      a.saved_ode=NULL;
	      Init(&a);
	      break;
	    case CH_CTRLO:
	      if (src=SpanRead) {
		Free(a.saved_ode);
		a.saved_ode=src;
		Init(&a);
	      }
	      break;
	    case CH_CTRLA:
	      if (!SpanTime || !a.saved_ode) {
		Free(a.saved_ode);
		a.saved_ode=SpanSave(ode);
	      }
	      Init(&a);
	      SpanWrite(ode);
	      break;
	    case CH_SPACE:
	      run_button.state=!run_button.state;
	      break;
	    case 'c':
	      mode_button.state=MD_CONCRETE;
	      break;
	    case 's':
	      mode_button.state=MD_STEEL;
	      break;
	    case 'w':
	      mode_button.state=MD_WIRE;
	      break;
	    case 'm':
	      mode_button.state=MD_MASS;
	      break;
	    case 'v':
	      mode_button.state=MD_MOVE;
	      break;
	    case 'd':
	      mode_button.state=MD_DELETE;
	      break;
	    case CH_ESC:
	      if (!SpanTime || !a.saved_ode) {
		Free(a.saved_ode);
		a.saved_ode=SpanSave(ode);
	      }
	      Init(&a);
	      SpanWrite(ode);
	    case CH_SHIFT_ESC:
	      goto span_done;
	  }
	  break;
      }
    }
span_done: //Don't goto out of try
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  DocClear;
  SettingsPop;
  CtrlButtonDel(bt_run);
  CtrlButtonDel(bt_mode);
  SpanDel(ode);
  DCFill(dc);
  DCDel(dc);
  MenuPop;
}
