class MyMass:CMass
{
  F64 radius;
};

class MySpring:CSpring
{
};

CMathODE *ode=NULL;

U0 DrawIt(CTask *,CDC *dc)
{
  MyMass   *tmpm;
  MySpring *tmps;

  dc->color=RED;
  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    GrLine(dc,tmps->end1->x,tmps->end1->y,tmps->end2->x,tmps->end2->y);
    tmps=tmps->next;
  }

  dc->color=BLACK;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    GrCircle(dc,tmpm->x,tmpm->y,tmpm->radius);
    tmpm=tmpm->next;
  }
}

U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{//The forces due to springs and drag are
//automatically handled by the ode code.
  //We can add new forces here.
  F64 d,dd;
  CD3 p;
  MyMass *tmpm1,*tmpm2;

  tmpm1=ode->next_mass;
  while (tmpm1!=&ode->next_mass) {
    tmpm2=tmpm1->next;
    while (tmpm2!=&ode->next_mass) {
      D3Sub(&p,&tmpm2->state->x,&tmpm1->state->x);
      dd=D3NormSqr(&p);
      if (dd<=Sqr(tmpm1->radius+tmpm2->radius)) {
	d=Sqrt(dd)+0.0001;
	dd=10.0*Sqr(Sqr(Sqr(tmpm1->radius+tmpm2->radius)-dd));
	D3MulEqu(&p,dd/d);
	D3AddEqu(&tmpm2->DstateDt->DxDt,&p);
	D3SubEqu(&tmpm1->DstateDt->DxDt,&p);
      }
      tmpm2=tmpm2->next;
    }
    tmpm1=tmpm1->next;
  }
}

U0 PlaceMass(I64 x, I64 y)
{
  MyMass *tmpm=CAlloc(sizeof(MyMass));
  tmpm->mass=1.0;
  tmpm->drag_profile_factor=100.0;
  tmpm->x=x;
  tmpm->y=y;
  tmpm->radius=10*(Rand+0.25);
  QueIns(tmpm,ode->last_mass);
}

U0 PlaceSpring(MyMass *tmpm1,MyMass *tmpm2)
{
  MySpring *tmps=CAlloc(sizeof(MySpring));
  tmps->end1=tmpm1;
  tmps->end2=tmpm2;
  tmps->const=10000;
  tmps->rest_len=100;
  QueIns(tmps,ode->last_spring);
}

U0 Init()
{
  ode=ODENew(0,1e-4,ODEF_HAS_MASSES);
  ode->derive=&MyDerivative;
  ode->drag_v2=0.002;
  ode->drag_v3=0.00001;
  ode->acceleration_limit=5e3;

  QueIns(ode,Fs->last_ode);
}

U0 CleanUp()
{
  QueRem(ode);
  QueDel(&ode->next_mass,TRUE);
  QueDel(&ode->next_spring,TRUE);
  ODEDel(ode);
}

U0 MassSpringDemo()
{
  I64 msg_code,arg1,arg2;
  MyMass *tmpm1=NULL,*tmpm2=NULL;

  PopUpOk("Left-Click to place mas\n"
	"Right-Click and drag to\n"
	"connect with spring.\n\n"
	"Springs are 100 pixs long.\n");
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;

  Fs->win_inhibit|=WIG_DBL_CLICK;

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"}"
	);

  Init;
  Fs->draw_it=&DrawIt;

  try {
    while (TRUE) {
      msg_code=GetMsg(&arg1,&arg2,
	    1<<MSG_MS_L_DOWN|1<<MSG_MS_R_DOWN|1<<MSG_MS_R_UP|1<<MSG_KEY_DOWN);
      switch (msg_code) {
	case MSG_MS_L_DOWN:
	  PlaceMass(arg1,arg2);
	  break;
	case MSG_MS_R_DOWN:
	  tmpm1=MassFind(ode,arg1,arg2);
	  tmpm2=NULL;
	  break;
	case MSG_MS_R_UP:
	  if (tmpm1 && (tmpm2=MassFind(ode,arg1,arg2)) && tmpm1!=tmpm2)
	    PlaceSpring(tmpm1,tmpm2);
	  tmpm1=tmpm2=NULL;
	  break;
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case '\n':
	      CleanUp;
	      Init;
	      break;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      goto ms_done;
	  }
	  break;
      }
    }
ms_done: //Don't goto out of try
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
}

MassSpringDemo;
