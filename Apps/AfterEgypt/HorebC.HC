U0 AnimateTask(I64)
{
  I64 i;
  F64 t;
  Obj *tmpo;
  while (TRUE) {
    t=tS;
    for (i=0;i<O_NUM_OBJS+O_NUM_PEBBLES;i++) {
      tmpo=&objs[i];
      if (tmpo->type==OT_SHEEP ||
	    tmpo->type==OT_GOAT1 || tmpo->type==OT_GOAT2) {
	tmpo->x+=tmpo->dx;
	tmpo->y+=tmpo->dy;
	tmpo->z+=tmpo->dz;
	tmpo->dx=5*Cos(0.5*t+tmpo->é);
	tmpo->dz=5*Sin(0.5*t+tmpo->é);
      }
    }
    Sleep(20);
  }
}

U0 Init()
{
  Obj *tmpo;
  I64 i,j,k,total=0;

  for (i=0;i<OT_NUM_TYPES;i++)
    total+=type_weights[i];

  MemSet(objs,0,sizeof(objs));
  for (i=0;i<O_NUM_OBJS+O_NUM_PEBBLES;i++) {
    tmpo=&objs[i];
    tmpo->num=i;
    tmpo->x=RandU32&0x1FFF-0x1000;
    tmpo->y=0;
    tmpo->z=RandU32&0x1FFF-0x1000;
    tmpo->é=2*ã*Rand;
    tmpo->sym=RandU16&1;

    if (i<O_NUM_OBJS) {
      k=0;
      j=RandU32%total;
      while (TRUE) {
	j-=type_weights[k];
	if (j<0)
	  break;
	else
	  k++;
      }
      tmpo->type=k;
    } else
      tmpo->type=OT_PEBBLE;
  }
  objs[O_BURNING_BUSH].type=OT_BUSH1;

  vx=0; vz=0; é=0;
  horeb_done=FALSE;
}

#define ANIMATE_STEPS	5

U0 AnimateéTask(F64 a)
{
  I64 i;
  for (i=0;i<ANIMATE_STEPS;i++) {
    é+=a/ANIMATE_STEPS;
    Sleep(20);
  }
}

U0 AnimateVTask(F64 v)
{
  I64 i;
  for (i=0;i<ANIMATE_STEPS;i++) {
    vx+=v/ANIMATE_STEPS*Sin(é);
    vz+=v/ANIMATE_STEPS*Cos(é);
    Sleep(20);
  }
}

U0 Horeb()
{
  I64 sc;
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Fwd(,,SC_CURSOR_UP);"
	"  Bwd(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"}"
	);
  SettingsPush(Fs,TSF_SAME_SONG); //See $LK,"SettingsPush",A="MN:SettingsPush"$
  try {
    Fs->text_attr=YELLOW<<4+BLUE;
    Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	  -WIF_SELF_GRAB_SCROLL-WIF_FOCUS_TASK_MENU;
    AutoComplete;
    WinBorder;
    WinMax;
    DocClear;
    "$$BG,LTCYAN$$%h6c",'\n';
    "$$BG,YELLOW$$%h30c",'\n';
    BibleVerse(,"Exodus,3:1",21);
    Init;
    Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
    Fs->draw_it=&DrawHoreb;
    while (!horeb_done)
      switch (GetKey(&sc)) {
	case 0:
	  switch (sc.u8[0]) {
	    case SC_CURSOR_LEFT:
	      Spawn(&AnimateéTask,(-ã/100)(I64),"Animate é",,Fs);
	      break;
	    case SC_CURSOR_RIGHT:
	      Spawn(&AnimateéTask,( ã/100)(I64),"Animate é",,Fs);
	      break;
	    case SC_CURSOR_UP:
	      Spawn(&AnimateVTask,(-40.0 )(I64),"Animate V",,Fs);
	      break;
	    case SC_CURSOR_DOWN:
	      Spawn(&AnimateVTask,( 40.0 )(I64),"Animate V",,Fs);
	      break;
	  }
	  break;
	case '\n':
	  Init;
	  break;
	case CH_SHIFT_ESC:
	case CH_ESC:
	  goto hr_done;
      }
hr_done: //Don't goto out of try
  } catch
    Fs->catch_except=TRUE;
  DocClear;
  SettingsPop(Fs,TSF_SAME_SONG);
  MenuPop;
}
