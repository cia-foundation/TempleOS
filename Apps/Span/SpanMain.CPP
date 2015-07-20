F64 SpanTime()
{
  if (run_button.state)
    return a.elapsed_t+tS-a.start_wall_t;
  else
    return a.elapsed_t;
}

F64 Cost(CMathODE *ode)
{
  MyMass   *tempm;
  MySpring *temps;
  F64 result=0;
  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    result+=tempm->cost;
    tempm=tempm->next;
  }
  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    result+=temps->cost;
    temps=temps->next;
  }
  return result;
}

U0 DrawIt(CTask *task,CDC *dc)
{
  MyMass   *tempm;
  MySpring *temps;

  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    if (!(temps->flags&SSF_INACTIVE)) {
      dc->color=temps->color;
      dc->pen_width=temps->pen_width;
      GrLine3(dc,temps->end1->x,temps->end1->y,0,
	    temps->end2->x,temps->end2->y,0);
    }
    temps=temps->next;
  }

  if (cursor_mass) {
    dc->color=RED;
    dc->pen_width=2;
    GrLine3(dc,ip.pos.x-task->pix_left-task->scroll_x,
	  ip.pos.y-task->pix_top-task->scroll_y,0,
	  cursor_mass->x,cursor_mass->y,0);
  }

  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    if (!(tempm->flags&MSF_INACTIVE)) {
      dc->color=BLACK;
      GrCircle(dc,tempm->x,tempm->y,tempm->radius);
      GrFloodFill(dc,tempm->x,tempm->y,TRUE);
      dc->color=tempm->color;
      GrCircle(dc,tempm->x,tempm->y,tempm->radius);
      GrFloodFill(dc,tempm->x,tempm->y,TRUE);
      dc->color=BLACK;
      GrCircle(dc,tempm->x,tempm->y,tempm->radius);
    }
    tempm=tempm->next;
  }

  dc->color=BLACK;
  GrPrint(dc,90,0,"Cost:%12.2,f",Cost(ode));
  GrPrint(dc,90,FONT_HEIGHT,"Time:%12.2f",SpanTime);
}

MyMass PlaceMass(I64 x, I64 y)
{
  MyMass *tempm=CAlloc(sizeof(MyMass));
  tempm->drag_profile_factor=1.0;
  tempm->x=x;
  tempm->y=y;
  tempm->mass=MASS_MASS;
  tempm->radius=MASS_RADIUS;
  tempm->cost=25.0*COST_SCALE;
  tempm->color=YELLOW;
  QueIns(tempm,ode->last_mass);
  return tempm;
}

U0 NullSpring(MySpring *temps,F64 scale)
{
  F64 d=D3Dist(&temps->end1->x,&temps->end2->x);
  temps->rest_len=d*scale;
  temps->compression_strength=
	temps->base_compression_strength/(temps->rest_len+1.0);
  temps->tensile_strength=temps->base_tensile_strength/(temps->rest_len+1.0);
  temps->const=temps->base_const/(temps->rest_len+1.0);
  temps->cost=temps->base_cost*temps->rest_len;
}

U0 MoveMass(MyMass *tempm,I64 x, I64 y)
{
  MySpring *temps;
  tempm->x=x;
  tempm->y=y;
  tempm->DxDt=0;
  tempm->DyDt=0;
  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    if (temps->end1==tempm || temps->end2==tempm) {
      if (temps->flags&SSF_NO_COMPRESSION)
	NullSpring(temps,WIRE_PERCENT);
      else
	NullSpring(temps,1.0);
    }
    temps=temps->next;
  }
}

U0 DelSpring(MySpring *temps)
{
  QueRem(temps);
  Free(temps);
}

U0 DelMass(MyMass *tempm)
{
  MySpring *temps,*temps1;
  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    temps1=temps->next;
    if (temps->end1==tempm || temps->end2==tempm)
      DelSpring(temps);
    temps=temps1;
  }
  QueRem(tempm);
  Free(tempm);
}

U0 DrawSpring(CDC *dc,MyMass *tempm,I64 x,I64 y)
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
  GrLine3(dc,tempm->x,tempm->y,0,x,y,0);
}

U0 PlaceSpring(MyMass *tempm1,MyMass *tempm2)
{
  MySpring *temps=CAlloc(sizeof(MySpring));
  temps->end1=tempm1;
  temps->end2=tempm2;
  switch (mode_button.state) {
    case MD_CONCRETE:
      temps->base_const	      = 3.00*SPRING_SCALE;
      temps->base_compression_strength=10.00*STRENGTH_SCALE;
      temps->base_tensile_strength    = 0.35*STRENGTH_SCALE;
      temps->base_cost		      = 0.30*COST_SCALE;
      NullSpring(temps,1.0);
      temps->color=LTGRAY;
      temps->pen_width=2;
      break;
    case MD_STEEL:
      temps->base_const	      = 1.00*SPRING_SCALE;
      temps->base_compression_strength= 1.00*STRENGTH_SCALE;
      temps->base_tensile_strength    = 1.00*STRENGTH_SCALE;
      temps->base_cost		      = 1.00*COST_SCALE;
      NullSpring(temps,1.0);
      temps->color=DKGRAY;
      temps->pen_width=2;
      break;
    case MD_WIRE:
      temps->base_const	      = 0.25*SPRING_SCALE;
      temps->base_compression_strength= 0.00;
      temps->base_tensile_strength    = 0.50*STRENGTH_SCALE;
      temps->base_cost		      = 0.10*COST_SCALE;
      NullSpring(temps,WIRE_PERCENT);
      temps->color=RED;
      temps->pen_width=1;
      temps->flags|=SSF_NO_COMPRESSION;
      break;
  }
  QueIns(temps,ode->last_spring);
}

U0 AnimateTask(SpanAnimateStruct *a)
{//Preemption is off by default.
  MySpring *temps,*temps1;
  Bool old_run=FALSE;
  F64 f;
  while (TRUE) {
    temps=ode->next_spring;
    while (temps!=&ode->next_spring) {
      temps1=temps->next;
      f=temps->f;
      if (f>0 &&  f>temps->compression_strength &&
	    !(temps->flags&SSF_NO_COMPRESSION)||
	    f<0 && -f>temps->tensile_strength &&
	    !(temps->flags&SSF_NO_TENSION))
	temps->flags|=SSF_INACTIVE;
      temps=temps1;
    }
    AdjustLoads(ode);
    WinMgrSync; //CMathODE updated once per refresh.
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
  WinMgrSync(2); //Allow stop to reg in animate task.

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
  MyMass   *tempm1=NULL,*tempm2=NULL;
  MySpring *temps;
  CCtrl *bt_run,*bt_mode;
  U8 *src;
  CDC *dc=DCAlias;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BROWN<<4+BLACK;
  WinMax;
  Fs->song_task=Spawn(&SongTask,NULL,"Song",,Fs);

  bt_run =CtrlButtonNew(0,0,		  80,,
	2,"Stopped\0Running\0",run_colors,&run_button);
  bt_mode=CtrlButtonNew(0,3.0*FONT_HEIGHT,80,,
	MD_NUM_MODES,Define("ST_SPAN_MODES"),mode_colors,&mode_button);
  a.saved_ode=NULL;

  //Preempt eliminates a lot of hassles.
  //The concern is about changing
  //que links while the winmgr task
  //is updating the ode or screen.
  Preempt;

  DocCursor;
  WinBorder;
  AutoComplete;
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
	"\nto select materials.\n");

  try {
    while (TRUE) {
      msg_code=GetMsg(&a1,&a2,1<<MSG_IP_L_DOWN|1<<MSG_IP_R_DOWN|
	    1<<MSG_IP_L_UP|1<<MSG_KEY_DOWN|1<<MSG_IP_MOVE);
      DCFill(dc);
      switch (msg_code) {
	case MSG_IP_L_DOWN:
	  cursor_mass=tempm1=tempm2=NULL;
	  switch (mode_button.state) {
	    case MD_MASS:
	      PlaceMass(a1,a2);
	      break;
	    case MD_CONCRETE:
	    case MD_STEEL:
	    case MD_WIRE:
	      tempm1=MassFind(ode,a1,a2);
	      break;
	    case MD_MOVE:
	      if (run_button.state)
		cursor_mass=MassFind(ode,a1,a2);
	      else
		if (tempm1=MassFind(ode,a1,a2))
		  MoveMass(tempm1,a1,a2);
	      break;
	    case MD_DELETE:
	      MassOrSpringFind(ode,&tempm1,&temps,a1,a2);
	      if (tempm1)
		DelMass(tempm1);
	      if (temps)
		DelSpring(temps);
	      break;
	  }
	  break;
	case MSG_IP_L_UP:
	  switch (mode_button.state) {
	    case MD_CONCRETE:
	    case MD_STEEL:
	    case MD_WIRE:
	      if (tempm1 && (tempm2=MassFind(ode,a1,a2)) && tempm1!=tempm2)
		PlaceSpring(tempm1,tempm2);
	      break;
	    case MD_MOVE:
	      if (!run_button.state && tempm1)
		MoveMass(tempm1,a1,a2);
	      break;
	  }
	  cursor_mass=tempm1=tempm2=NULL;
	  break;
	case MSG_IP_MOVE:
	  switch (mode_button.state) {
	    case MD_MOVE:
	      if (!run_button.state && tempm1)
		MoveMass(tempm1,a1,a2);
	      break;
	    case MD_CONCRETE:
	    case MD_STEEL:
	    case MD_WIRE:
	      if (tempm1) {
		DrawSpring(dc,tempm1,a1,a2);
	      }
	      break;
	  }
	  break;
	case MSG_IP_R_DOWN:
	  mode_button.state++;
	  if (mode_button.state>=MD_NUM_MODES)
	    mode_button.state=0;
	  cursor_mass=tempm1=tempm2=NULL;
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
