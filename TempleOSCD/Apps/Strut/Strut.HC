$BG,0$
$SP,"<1>",BI=1$

$SP,"<2>",BI=2$


$SP,"<3>",BI=3$

$BG$
#define STARS_NUM	8192
I64 stars_x[STARS_NUM],stars_y[STARS_NUM];

#define RADIUS	7

class MyMass:CMass
{
  U8 *img;
};

class MySpring:CSpring
{
  I64 type,action_key;
};

//Main Modses
#define MMD_EDIT	0
#define MMD_PLAY	1
#define MMD_MODES_NUM	2
CColorROPU32 main_mode_colors[MMD_MODES_NUM]={LTRED,LTGREEN};
DefineLstLoad("ST_MAIN_MODES","Edit\0Play\0");
CCtrlBttnState main_mode_bttn;

//Edit Modes
#define EMD_MASS	0
#define EMD_SPRING	1
#define EMD_CONNECTOR	2
#define EMD_THRUSTER	3
#define EMD_MOVE	4
#define EMD_MODES_NUM	5
CColorROPU32 edit_mode_colors[EMD_MODES_NUM]={LTGRAY,LTCYAN,CYAN,YELLOW,LTBLUE};
DefineLstLoad("ST_EDIT_MODES","Mass\0Spring\0Connector\0Thruster\0Move\0");
CCtrlBttnState edit_mode_bttn;

CTask *task;
F64 zoom;
I64 next_action_key,action_scan_codes[10];
CMathODE *ode=NULL;

U0 S2W(F64 sx,F64 sy,F64 *_wx,F64 *_wy)
{
  sx-=task->pix_left+task->scroll_x;
  sy-=task->pix_top +task->scroll_y;
  *_wx=sx/zoom;
  *_wy=sy/zoom;
}

U0 W2S(F64 wx,F64 wy,F64 *_sx,F64 *_sy)
{
  *_sx=wx*zoom; *_sy=wy*zoom;
}

#define ZOOM_STEPS	20

U0 Zoom(F64 d)
{
  F64 sx,sy,wx,wy;
  I64 i,x=ms.pos.x,y=ms.pos.y;
  d=Exp(Ln(d)/ZOOM_STEPS);
  for (i=0;i<ZOOM_STEPS;i++) {
    S2W(x,y,&wx,&wy);
    zoom=Clamp(zoom*d,0.02,50);
    W2S(wx,wy,&sx,&sy);
    task->scroll_x=ms.pos.x-sx-task->pix_left;
    task->scroll_y=ms.pos.y-sy-task->pix_top;
    Sleep(10);
  }
}

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 é,d,x1,y1,x2,y2;
  MyMass   *tmpm;
  MySpring *tmps;

  tmpm=ode->next_mass;
  if (tmpm!=&ode->next_mass) {
    task->scroll_x=-tmpm->x*zoom+task->pix_width >>1;
    task->scroll_y=-tmpm->y*zoom+task->pix_height>>1;
  }

  dc->flags|=DCF_TRANSFORMATION;
  Mat4x4Scale(dc->r,zoom);

  switch (main_mode_bttn.state) {
    case MMD_EDIT:
      task->text_attr=DKGRAY<<4+WHITE;
      dc->color=BLACK;
      break;
    case MMD_PLAY:
      task->text_attr=BLACK<<4+WHITE;
      dc->color=WHITE;
      for (i=0;i<STARS_NUM;i++)
	GrPlot3(dc,stars_x[i],stars_y[i],0);
      break;
  }

  if (main_mode_bttn.state==MMD_EDIT) {
    if (edit_mode_bttn.state==EMD_CONNECTOR) {
      dc->color=CYAN;
      S2W(FONT_WIDTH*11,FONT_HEIGHT*7,&x1,&y1);
      GrPutChar3(dc,x1,y1,0,next_action_key);
    } else if (edit_mode_bttn.state==EMD_THRUSTER) {
      dc->color=YELLOW;
      S2W(FONT_WIDTH*11,FONT_HEIGHT*7,&x1,&y1);
      GrPutChar3(dc,x1,y1,0,next_action_key);
    }
  }

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    if (tmps->type==EMD_SPRING) {
      dc->color=LTCYAN;
      GrLine3(dc,tmps->end1->x,tmps->end1->y,0,
	    tmps->end2->x,tmps->end2->y,0);
    } else if (tmps->type==EMD_CONNECTOR) {
      dc->color=CYAN;
      GrLine3(dc,tmps->end1->x,tmps->end1->y,0,
	    tmps->end2->x,tmps->end2->y,0);
    }
    tmps=tmps->next;
  }

  dc->color=LTGRAY;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    Sprite3(dc,tmpm->x,tmpm->y,0,tmpm->img);
    tmpm=tmpm->next;
  }

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    x1=tmps->end1->x; y1=tmps->end1->y;
    x2=tmps->end2->x; y2=tmps->end2->y;
    if (tmps->type==EMD_THRUSTER) {
      é=Arg(x2-x1,y2-y1);
      if (Bt(kbd.down_bitmap,action_scan_codes[tmps->action_key-'0'])) {
	dc->flags|=DCF_SYMMETRY;
	DCSymmetry3Set(dc,x1,y1,256,x1,y1,0,x1-256*Cos(é),y1-256*Sin(é),0);
	for (i=0;i<8;i++) {
	  d=20*Rand+6;
	  if (d<10)
	    dc->color=BLUE;
	  else if (d<16)
	    dc->color=LTBLUE;
	  else
	    dc->color=YELLOW;
	  GrLine3(dc,x1-3*Cos(é),y1-3*Sin(é),0,
		x1-d*Cos(é)+0.5*d*Sin(é),
		y1-d*Sin(é)-0.5*d*Cos(é),0);
	  GrLine3(dc,x1-2*d*Cos(é),y1-2*d*Sin(é),0,
		x1-d*Cos(é)+0.5*d*Sin(é),
		y1-d*Sin(é)-0.5*d*Cos(é),0);
	}
	dc->flags&=~DCF_SYMMETRY;
      }
      Sprite3ZB(dc,x1,y1,0,$IB,"<3>",BI=3$,é);
      if (zoom>0.5) {
	dc->color=YELLOW;
	GrPutChar3(dc,(x1*7+x2)/8,(y1*7+y2)/8,0,tmps->action_key);
      }
    } else if (tmps->type==EMD_CONNECTOR) {
      if (zoom>0.5) {
	dc->color=CYAN;
	GrPutChar3(dc,(x1*7+x2)/8,(y1*7+y2)/8,0,tmps->action_key);
      }
    }
    tmps=tmps->next;
  }
}

U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{
//The forces due to springs and drag are
  //automatically handled by the
  //ode code.  We can add new forces
  //here.
  F64 d,dd;
  CD3 p;
  MyMass *tmpm1,*tmpm2;
  MySpring *tmps;

  tmpm1=ode->next_mass;
  while (tmpm1!=&ode->next_mass) {
    tmpm2=tmpm1->next;
    while (tmpm2!=&ode->next_mass) {
      D3Sub(&p,&tmpm2->state->x,&tmpm1->state->x);
      dd=D3NormSqr(&p);
      if (dd<=Sqr(2*RADIUS)) {
	d=Sqrt(dd)+0.0001;
	dd=10.0*Sqr(Sqr(Sqr(2*RADIUS)-dd));
	D3MulEqu(&p,dd/d);
	D3AddEqu(&tmpm2->DstateDt->DxDt,&p);
	D3SubEqu(&tmpm1->DstateDt->DxDt,&p);
      }
      tmpm2=tmpm2->next;
    }
    tmpm1=tmpm1->next;
  }

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    if (main_mode_bttn.state==MMD_PLAY && tmps->type==EMD_THRUSTER &&
	  Bt(kbd.down_bitmap,action_scan_codes[tmps->action_key-'0'])) {
      D3Sub(&p,&tmps->end2->state->x,&tmps->end1->state->x);
      D3Unit(&p);
      D3MulEqu(&p,2000);
      D3AddEqu(&tmps->end1->DstateDt->DxDt,&p);
    }
    tmps=tmps->next;
  }
}

MyMass *PlaceMass(I64 x, I64 y)
{
  MyMass *tmpm=CAlloc(sizeof(MyMass));
  tmpm->mass=1.0;
  tmpm->drag_profile_factor=100.0;
  tmpm->x=x;
  tmpm->y=y;
  QueIns(tmpm,ode->last_mass);
  return tmpm;
}

MySpring *PlaceSpring(MyMass *tmpm1,MyMass *tmpm2,I64 type)
{
  MySpring *tmps=CAlloc(sizeof(MySpring));
  F64 d=D3Dist(&tmpm1->x,&tmpm2->x);
  tmps->end1=tmpm1;
  tmps->end2=tmpm2;
  tmps->rest_len=d;
  tmps->type=type;
  if (type==EMD_THRUSTER)
    tmps->const=0;
  else
    tmps->const=2500000/Sqr(d);
  tmps->action_key=next_action_key;
  QueIns(tmps,ode->last_spring);
  return tmps;
}

U0 CenterMasses()
{
  CD3 p;
  MyMass *tmpm1,*tmpm2;
  tmpm1=ode->next_mass;
  if (tmpm1!=&ode->next_mass) {
    D3Copy(&p,&tmpm1->x);
    tmpm2=ode->next_mass;
    while (tmpm2!=&ode->next_mass) {
      D3SubEqu(&tmpm2->x,&p);
      tmpm2=tmpm2->next;
    }
  }
}

U0 NullSprings()
{
  MySpring *tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    tmps->rest_len=D3Dist(&tmps->end1->x,&tmps->end2->x);
    tmps=tmps->next;
  }
}

U0 BreakConnectors()
{
  MySpring *tmps=ode->next_spring,*tmps1;
  while (tmps!=&ode->next_spring) {
    tmps1=tmps->next;
    if (tmps->type==EMD_CONNECTOR &&
	  Bt(kbd.down_bitmap,action_scan_codes[tmps->action_key-'0'])) {
      QueRem(tmps);
      Free(tmps);
    }
    tmps=tmps1;
  }
}

U0 Init()
{
  I64 i;
  task=Fs;
  for (i=0;i<=9;i++)
    action_scan_codes[i]=Char2ScanCode('0'+i);
  for (i=0;i<STARS_NUM;i++) {
    stars_x[i]=RandU32%8192-4096;
    stars_y[i]=RandU32%8192-4096;
  }
  next_action_key='1';
  zoom=1.0;
  ode=ODENew(0,1e-4,ODEF_HAS_MASSES);
  ode->derive=&MyDerivative;
  ode->acceleration_limit=5e3;
  ode->drag_v2=0.000002;
  ode->drag_v3=0.0000001;
  QueIns(ode,Fs->last_ode);
}

U0 CleanUp()
{
  if (ode) {
    QueRem(ode);
    QueDel(&ode->next_mass,TRUE);
    QueDel(&ode->next_spring,TRUE);
    ODEDel(ode);
    ode=NULL;
  }
}

U0 PlayShip()
{
  I64 arg1,arg2;
  F64 last_noise=0;
  Bool okay;
  ODEPause(ode,OFF);
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Edit {"
	"  EditShip(,CH_SPACE);"
	"}"
	"Play {"
	"  Center(,'c');"
	"  Damp(,'d');"
	"  Action0(,'0');"
	"  Action1(,'1');"
	"  Action2(,'2');"
	"  Action3(,'3');"
	"  Action4(,'4');"
	"  Action5(,'5');"
	"  Action6(,'6');"
	"  Action7(,'7');"
	"  Action8(,'8');"
	"  Action9(,'9');"
	"}"
	"View {"
	"  ZoomIn(,'z');"
	"  ZoomOut(,'Z');"
	"}"
	);
  DocClear;
  try {
    while (main_mode_bttn.state==MMD_PLAY) {
      BreakConnectors;
      switch (ScanMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP)) {
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case '0'...'9':
	      if (tS>last_noise+0.25) {
		Noise(250,18,46);
		last_noise=tS;
	      }
	      break;
	    case 'z':
	      Spawn(&Zoom,2.0(I64));
	      break;
	    case 'Z':
	      Spawn(&Zoom,0.5(I64));
	      break;
	    case 'c':
	      CenterMasses;
	      break;
	    case 'd':
	      ode->drag_v2=0.002;
	      ode->drag_v3=0.0001;
	      break;
	    case CH_SPACE:
	      if (++main_mode_bttn.state==MMD_MODES_NUM)
		main_mode_bttn.state=0;
	      GetMsg(,,1<<MSG_KEY_UP);
	      goto ps_done;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      throw;
	  }
	  break;
	case MSG_KEY_UP:
	  switch (arg1) {
	    case 'd':
	      ode->drag_v2=0.000002;
	      ode->drag_v3=0.0000001;
	      break;
	  }
	  break;
      }
      Refresh;
    }
ps_done: //Don't goto out of try
    okay=TRUE;
  } catch {
    Fs->catch_except=TRUE;
    okay=FALSE;
  }
  MenuPop;
  if (!okay)
    throw;
}

U0 EditShip()
{
  I64 arg1,arg2;
  F64 wx,wy;
  Bool okay;
  MyMass *tmpm1=NULL,*tmpm2=NULL;
  CCtrl *bt_edit_mode=CtrlBttnNew(0,5*FONT_HEIGHT+4,80,,EMD_MODES_NUM,
	Define("ST_EDIT_MODES"),edit_mode_colors,&edit_mode_bttn);
  ODEPause(ode);
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  PlayShip(,CH_SPACE);"
	"  Center(,'c');"
	"  Damp(,'d');"
	"}"
	"Edit {"
	"  Move(,'v');"
	"  Mass(,'m');"
	"  Spring(,'s');"
	"  Connector(,'n');"
	"  Thruster(,'t');"
	"  Restart(,'\n');"
	"  Action0(,'0');"
	"  Action1(,'1');"
	"  Action2(,'2');"
	"  Action3(,'3');"
	"  Action4(,'4');"
	"  Action5(,'5');"
	"  Action6(,'6');"
	"  Action7(,'7');"
	"  Action8(,'8');"
	"  Action9(,'9');"
	"}"
	"View {"
	"  ZoomIn(,'z');"
	"  ZoomOut(,'Z');"
	"}"
	);
  DocClear;
  try {
    while (main_mode_bttn.state==MMD_EDIT) {
      switch (ScanMsg(&arg1,&arg2,
	    1<<MSG_MS_L_DOWN|1<<MSG_MS_L_UP|1<<MSG_MS_R_DOWN|
	    1<<MSG_MS_MOVE|1<<MSG_KEY_DOWN|1<<MSG_KEY_UP)) {
	case MSG_MS_L_DOWN:
	  switch (edit_mode_bttn.state) {
	    case EMD_MASS:
	      S2W(ms.pos.x,ms.pos.y,&wx,&wy);
	      tmpm1=PlaceMass(wx,wy);
	      if (ode->next_mass==tmpm1)
		tmpm1->img=$IB,"<1>",BI=1$;
	      else
		tmpm1->img=$IB,"<2>",BI=2$;
	      tmpm1=NULL;
	      break;
	    case EMD_SPRING:
	    case EMD_CONNECTOR:
	    case EMD_THRUSTER:
	    case EMD_MOVE:
	      S2W(ms.pos.x,ms.pos.y,&wx,&wy);
	      tmpm1=MassFind(ode,wx,wy);
	      tmpm2=NULL;
	      break;
	  }
	  break;
	case MSG_MS_L_UP:
	  switch (edit_mode_bttn.state) {
	    case EMD_MASS:
	      break;
	    case EMD_SPRING:
	    case EMD_CONNECTOR:
	    case EMD_THRUSTER:
	      S2W(ms.pos.x,ms.pos.y,&wx,&wy);
	      if (tmpm1 && (tmpm2=MassFind(ode,wx,wy)) && tmpm1!=tmpm2)
		PlaceSpring(tmpm1,tmpm2,edit_mode_bttn.state);
	      tmpm1=tmpm2=NULL;
	      break;
	    case EMD_MOVE:
	      S2W(ms.pos.x,ms.pos.y,&wx,&wy);
	      if (tmpm1) {
		tmpm1->x=wx;
		tmpm1->y=wy;
		tmpm1->z=0;
		NullSprings;
	      }
	      tmpm1=tmpm2=NULL;
	      break;
	  }
	  break;
	case MSG_MS_MOVE:
	  switch (edit_mode_bttn.state) {
	    case EMD_MOVE:
	      S2W(ms.pos.x,ms.pos.y,&wx,&wy);
	      if (tmpm1) {
		tmpm1->x=wx;
		tmpm1->y=wy;
		tmpm1->z=0;
		NullSprings;
	      }
	      break;
	  }
	  break;
	case MSG_MS_R_DOWN:
	  if (++edit_mode_bttn.state==EMD_MODES_NUM)
	    edit_mode_bttn.state=0;
	  break;
	case MSG_MS_R_UP:
	  break;
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case '\n':
	      CleanUp;
	      Init;
	      break;
	    case '0'...'9':
	      next_action_key=arg1;
	      break;
	    case 'c':
	      CenterMasses;
	      break;
	    case 'd':
	      ODEPause(ode,OFF);
	      ode->drag_v2=0.002;
	      ode->drag_v3=0.0001;
	      break;
	    case 'v':
	      edit_mode_bttn.state=EMD_MOVE;
	      break;
	    case 'm':
	      edit_mode_bttn.state=EMD_MASS;
	      break;
	    case 'n':
	      edit_mode_bttn.state=EMD_CONNECTOR;
	      break;
	    case 's':
	      edit_mode_bttn.state=EMD_SPRING;
	      break;
	    case 't':
	      edit_mode_bttn.state=EMD_THRUSTER;
	      break;
	    case 'z':
	      Spawn(&Zoom,2.0(I64));
	      break;
	    case 'Z':
	      Spawn(&Zoom,0.5(I64));
	      break;
	    case CH_SPACE:
	      if (++main_mode_bttn.state==MMD_MODES_NUM)
		main_mode_bttn.state=0;
	      GetMsg(,,1<<MSG_KEY_UP);
	      goto es_done;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      throw;
	  }
	  break;
	case MSG_KEY_UP:
	  switch (arg1) {
	    case 'd':
	      ODEPause(ode);
	      ode->drag_v2=0.000002;
	      ode->drag_v3=0.0000001;
	      break;
	  }
	  break;
      }
      Refresh;
    }
es_done: //Don't goto out of try
    okay=TRUE;
  } catch {
    Fs->catch_except=TRUE;
    okay=FALSE;
  }
  MenuPop;
  CtrlBttnDel(bt_edit_mode);
  if (!okay)
    throw;
}

U0 Strut()
{
  CCtrl *bt_main_mode;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit|=WIF_SELF_MS_L|WIF_SELF_MS_R|WIG_DBL_CLICK;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  "\n$$WW,1$$$$PURPLE$$$$TX+CX,\"Build a ship.\"$$$$FG$$\n\n"
	"Sel mass mode.\tLeft-click to place masses.\n"
	"Sel spring mode.\tLeft-drag to make members.\n"
	"Sel thruster mode.\tPress a digit, 0-9.  Drag to make thruster.\n"
	"Sel connector mode.\tPress a digit, 0-9.  "
	"Drag to make breakable connector.\n\n"
	"Press $$GREEN$$<SPACE>$$FG$$ to run the game.  Press digits to operate "
	"thrusters and break connectors.\n\n"
	"Press $$GREEN$$<z>$$FG$$ or $$GREEN$$<SHIFT-Z>$$FG$$ to zoom in/out.\n\n";

  PressAKey;
  bt_main_mode=CtrlBttnNew(
	(GR_WIDTH-4*FONT_WIDTH-16)>>1,1*FONT_HEIGHT+4,80,,MMD_MODES_NUM,
	Define("ST_MAIN_MODES"),main_mode_colors,&main_mode_bttn);
  DocClear;
  Init;
  Fs->draw_it=&DrawIt;
  try {
    while (TRUE) {
      EditShip;
      PlayShip;
    }
  } catch {
    CleanUp;
    Fs->catch_except=TRUE;
  }
  SettingsPop;
  CtrlBttnDel(bt_main_mode);
}
             øÿÿÿøÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             øÿÿÿøÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ‚     ùÿÿÿöÿÿÿ      ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿ                                                                                ÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   