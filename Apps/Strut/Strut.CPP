$BG,0$
$SP,"<1>",BI=1$

$SP,"<2>",BI=2$


$SP,"<3>",BI=3$

$BG$
#define NUM_STARS	8192
I64 stars_x[NUM_STARS],stars_y[NUM_STARS];

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
#define MMD_NUM_MODES	2
CColorROPU32 main_mode_colors[MMD_NUM_MODES]={LTRED,LTGREEN};
DefineLstLoad("ST_MAIN_MODES","Edit\0Play\0");
CCtrlButtonState main_mode_button;

//Edit Modes
#define EMD_MASS	0
#define EMD_SPRING	1
#define EMD_CONNECTOR	2
#define EMD_THRUSTER	3
#define EMD_MOVE	4
#define EMD_NUM_MODES	5
CColorROPU32 edit_mode_colors[EMD_NUM_MODES]={LTGRAY,LTCYAN,CYAN,YELLOW,LTBLUE};
DefineLstLoad("ST_EDIT_MODES","Mass\0Spring\0Connector\0Thruster\0Move\0");
CCtrlButtonState edit_mode_button;

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
  I64 i,x=ip.pos.x,y=ip.pos.y;
  d=Exp(Ln(d)/ZOOM_STEPS);
  for (i=0;i<ZOOM_STEPS;i++) {
    S2W(x,y,&wx,&wy);
    zoom=Clamp(zoom*d,0.02,50);
    W2S(wx,wy,&sx,&sy);
    task->scroll_x=ip.pos.x-sx-task->pix_left;
    task->scroll_y=ip.pos.y-sy-task->pix_top;
    Sleep(10);
  }
}

U0 DrawIt(CTask *,CDC *dc)
{
  I64 i;
  F64 é,d,x1,y1,x2,y2;
  MyMass   *tempm;
  MySpring *temps;

  tempm=ode->next_mass;
  if (tempm!=&ode->next_mass) {
    task->scroll_x=-tempm->x*zoom+task->pix_width >>1;
    task->scroll_y=-tempm->y*zoom+task->pix_height>>1;
  }

  dc->flags|=DCF_TRANSFORMATION;
  Mat4x4Scale(dc->r,zoom);

  switch (main_mode_button.state) {
    case MMD_EDIT:
      task->text_attr=DKGRAY<<4+WHITE;
      dc->color=BLACK;
      break;
    case MMD_PLAY:
      task->text_attr=BLACK<<4+WHITE;
      dc->color=WHITE;
      for (i=0;i<NUM_STARS;i++)
	GrPlot3(dc,stars_x[i],stars_y[i],0);
      break;
  }

  if (main_mode_button.state==MMD_EDIT) {
    if (edit_mode_button.state==EMD_CONNECTOR) {
      dc->color=CYAN;
      S2W(FONT_WIDTH*11,FONT_HEIGHT*7,&x1,&y1);
      GrPutChar3(dc,x1,y1,0,next_action_key);
    } else if (edit_mode_button.state==EMD_THRUSTER) {
      dc->color=YELLOW;
      S2W(FONT_WIDTH*11,FONT_HEIGHT*7,&x1,&y1);
      GrPutChar3(dc,x1,y1,0,next_action_key);
    }
  }

  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    if (temps->type==EMD_SPRING) {
      dc->color=LTCYAN;
      GrLine3(dc,temps->end1->x,temps->end1->y,0,
	    temps->end2->x,temps->end2->y,0);
    } else if (temps->type==EMD_CONNECTOR) {
      dc->color=CYAN;
      GrLine3(dc,temps->end1->x,temps->end1->y,0,
	    temps->end2->x,temps->end2->y,0);
    }
    temps=temps->next;
  }

  dc->color=LTGRAY;
  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    Sprite3(dc,tempm->x,tempm->y,0,tempm->img);
    tempm=tempm->next;
  }

  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    x1=temps->end1->x; y1=temps->end1->y;
    x2=temps->end2->x; y2=temps->end2->y;
    if (temps->type==EMD_THRUSTER) {
      é=Arg(x2-x1,y2-y1);
      if (Bt(kbd.down_bitmap,action_scan_codes[temps->action_key-'0'])) {
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
      SpriteZ3B(dc,x1,y1,0,$IB,"<3>",BI=3$,é);
      if (zoom>0.5) {
	dc->color=YELLOW;
	GrPutChar3(dc,(x1*7+x2)/8,(y1*7+y2)/8,0,temps->action_key);
      }
    } else if (temps->type==EMD_CONNECTOR) {
      if (zoom>0.5) {
	dc->color=CYAN;
	GrPutChar3(dc,(x1*7+x2)/8,(y1*7+y2)/8,0,temps->action_key);
      }
    }
    temps=temps->next;
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
  MyMass *tempm1,*tempm2;
  MySpring *temps;

  tempm1=ode->next_mass;
  while (tempm1!=&ode->next_mass) {
    tempm2=tempm1->next;
    while (tempm2!=&ode->next_mass) {
      D3Sub(&tempm2->state->x,&tempm1->state->x,&p);
      dd=D3NormSqr(&p);
      if (dd<=Sqr(2*RADIUS)) {
	d=Sqrt(dd)+0.0001;
	dd=10.0*Sqr(Sqr(Sqr(2*RADIUS)-dd));
	D3MulEqu(&p,dd/d);
	D3AddEqu(&tempm2->DstateDt->DxDt,&p);
	D3SubEqu(&tempm1->DstateDt->DxDt,&p);
      }
      tempm2=tempm2->next;
    }
    tempm1=tempm1->next;
  }

  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    if (main_mode_button.state==MMD_PLAY && temps->type==EMD_THRUSTER &&
	  Bt(kbd.down_bitmap,action_scan_codes[temps->action_key-'0'])) {
      D3Sub(&temps->end2->state->x,&temps->end1->state->x,&p);
      D3Unit(&p);
      D3MulEqu(&p,2000);
      D3AddEqu(&temps->end1->DstateDt->DxDt,&p);
    }
    temps=temps->next;
  }
}

MyMass *PlaceMass(I64 x, I64 y)
{
  MyMass *tempm=CAlloc(sizeof(MyMass));
  tempm->mass=1.0;
  tempm->drag_profile_factor=100.0;
  tempm->x=x;
  tempm->y=y;
  QueIns(tempm,ode->last_mass);
  return tempm;
}

MySpring *PlaceSpring(MyMass *tempm1,MyMass *tempm2,I64 type)
{
  MySpring *temps=CAlloc(sizeof(MySpring));
  F64 d=D3Dist(&tempm1->x,&tempm2->x);
  temps->end1=tempm1;
  temps->end2=tempm2;
  temps->rest_len=d;
  temps->type=type;
  if (type==EMD_THRUSTER)
    temps->const=0;
  else
    temps->const=2500000/Sqr(d);
  temps->action_key=next_action_key;
  QueIns(temps,ode->last_spring);
  return temps;
}

U0 CenterMasses()
{
  CD3 p;
  MyMass *tempm1,*tempm2;
  tempm1=ode->next_mass;
  if (tempm1!=&ode->next_mass) {
    D3Copy(&p,&tempm1->x);
    tempm2=ode->next_mass;
    while (tempm2!=&ode->next_mass) {
      D3SubEqu(&tempm2->x,&p);
      tempm2=tempm2->next;
    }
  }
}

U0 NullSprings()
{
  MySpring *temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    temps->rest_len=D3Dist(&temps->end1->x,&temps->end2->x);
    temps=temps->next;
  }
}

U0 BreakConnectors()
{
  MySpring *temps=ode->next_spring,*temps1;
  while (temps!=&ode->next_spring) {
    temps1=temps->next;
    if (temps->type==EMD_CONNECTOR &&
	  Bt(kbd.down_bitmap,action_scan_codes[temps->action_key-'0'])) {
      QueRem(temps);
      Free(temps);
    }
    temps=temps1;
  }
}

U0 Init()
{
  I64 i;
  task=Fs;
  for (i=0;i<=9;i++)
    action_scan_codes[i]=Char2ScanCode('0'+i);
  for (i=0;i<NUM_STARS;i++) {
    stars_x[i]=RandU32%8192-4096;
    stars_y[i]=RandU32%8192-4096;
  }
  next_action_key='1';
  zoom=1.0;
  ode=ODENew(0,1e-4,ODEF_HAS_MASSES);
  ode->derivative=&MyDerivative;
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
  I64 a1,a2;
  F64 last_noise=0;
  Bool okay;
  ode->flags&=~ODEF_PAUSED;
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
    while (main_mode_button.state==MMD_PLAY) {
      BreakConnectors;
      switch (ScanMsg(&a1,&a2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP)) {
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case '0'...'9':
	      if (tS>last_noise+0.25) {
		Noise(250,40,200);
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
	      if (++main_mode_button.state==MMD_NUM_MODES)
		main_mode_button.state=0;
	      GetMsg(,,1<<MSG_KEY_UP);
	      goto ps_done;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      throw;
	  }
	  break;
	case MSG_KEY_UP:
	  switch (a1) {
	    case 'd':
	      ode->drag_v2=0.000002;
	      ode->drag_v3=0.0000001;
	      break;
	  }
	  break;
      }
      WinMgrSync;
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
  I64 a1,a2;
  F64 wx,wy;
  Bool okay;
  MyMass *tempm1=NULL,*tempm2=NULL;
  CCtrl *bt_edit_mode=CtrlButtonNew(0,5*FONT_HEIGHT+4,80,,EMD_NUM_MODES,
	Define("ST_EDIT_MODES"),edit_mode_colors,&edit_mode_button);
  ode->flags|=ODEF_PAUSED;
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
    while (main_mode_button.state==MMD_EDIT) {
      switch (ScanMsg(&a1,&a2,1<<MSG_IP_L_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_DOWN|
	    1<<MSG_IP_MOVE|1<<MSG_KEY_DOWN|1<<MSG_KEY_UP)) {
	case MSG_IP_L_DOWN:
	  switch (edit_mode_button.state) {
	    case EMD_MASS:
	      S2W(ip.pos.x,ip.pos.y,&wx,&wy);
	      tempm1=PlaceMass(wx,wy);
	      if (ode->next_mass==tempm1)
		tempm1->img=$IB,"<1>",BI=1$;
	      else
		tempm1->img=$IB,"<2>",BI=2$;
	      tempm1=NULL;
	      break;
	    case EMD_SPRING:
	    case EMD_CONNECTOR:
	    case EMD_THRUSTER:
	    case EMD_MOVE:
	      S2W(ip.pos.x,ip.pos.y,&wx,&wy);
	      tempm1=MassFind(ode,wx,wy);
	      tempm2=NULL;
	      break;
	  }
	  break;
	case MSG_IP_L_UP:
	  switch (edit_mode_button.state) {
	    case EMD_MASS:
	      break;
	    case EMD_SPRING:
	    case EMD_CONNECTOR:
	    case EMD_THRUSTER:
	      S2W(ip.pos.x,ip.pos.y,&wx,&wy);
	      if (tempm1 && (tempm2=MassFind(ode,wx,wy)) && tempm1!=tempm2)
		PlaceSpring(tempm1,tempm2,edit_mode_button.state);
	      tempm1=tempm2=NULL;
	      break;
	    case EMD_MOVE:
	      S2W(ip.pos.x,ip.pos.y,&wx,&wy);
	      if (tempm1) {
		tempm1->x=wx;
		tempm1->y=wy;
		tempm1->z=0;
		NullSprings;
	      }
	      tempm1=tempm2=NULL;
	      break;
	  }
	  break;
	case MSG_IP_MOVE:
	  switch (edit_mode_button.state) {
	    case EMD_MOVE:
	      S2W(ip.pos.x,ip.pos.y,&wx,&wy);
	      if (tempm1) {
		tempm1->x=wx;
		tempm1->y=wy;
		tempm1->z=0;
		NullSprings;
	      }
	      break;
	  }
	  break;
	case MSG_IP_R_DOWN:
	  if (++edit_mode_button.state==EMD_NUM_MODES)
	    edit_mode_button.state=0;
	  break;
	case MSG_IP_R_UP:
	  break;
	case MSG_KEY_DOWN:
	  switch (a1) {
	    case '\n':
	      CleanUp;
	      Init;
	      break;
	    case '0'...'9':
	      next_action_key=a1;
	      break;
	    case 'c':
	      CenterMasses;
	      break;
	    case 'd':
	      ode->flags&=~ODEF_PAUSED;
	      ode->drag_v2=0.002;
	      ode->drag_v3=0.0001;
	      break;
	    case 'v':
	      edit_mode_button.state=EMD_MOVE;
	      break;
	    case 'm':
	      edit_mode_button.state=EMD_MASS;
	      break;
	    case 'n':
	      edit_mode_button.state=EMD_CONNECTOR;
	      break;
	    case 's':
	      edit_mode_button.state=EMD_SPRING;
	      break;
	    case 't':
	      edit_mode_button.state=EMD_THRUSTER;
	      break;
	    case 'z':
	      Spawn(&Zoom,2.0(I64));
	      break;
	    case 'Z':
	      Spawn(&Zoom,0.5(I64));
	      break;
	    case CH_SPACE:
	      if (++main_mode_button.state==MMD_NUM_MODES)
		main_mode_button.state=0;
	      GetMsg(,,1<<MSG_KEY_UP);
	      goto es_done;
	    case CH_SHIFT_ESC:
	    case CH_ESC:
	      throw;
	  }
	  break;
	case MSG_KEY_UP:
	  switch (a1) {
	    case 'd':
	      ode->flags|=ODEF_PAUSED;
	      ode->drag_v2=0.000002;
	      ode->drag_v3=0.0000001;
	      break;
	  }
	  break;
      }
      WinMgrSync;
    }
es_done: //Don't goto out of try
    okay=TRUE;
  } catch {
    Fs->catch_except=TRUE;
    okay=FALSE;
  }
  MenuPop;
  CtrlButtonDel(bt_edit_mode);
  if (!okay)
    throw;
}

U0 Strut()
{
  CCtrl *bt_main_mode;
  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->win_inhibit|=WIF_SELF_IP_L|WIF_SELF_IP_R|WIG_DBL_CLICK;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  "\n$$WW,1$$$$PURPLE$$$$TX+CX,\"Build a ship.\"$$$$FG$$\n\n"
	"Select mass mode.\tLeft-click to place masses.\n"
	"Select spring mode.\tLeft-drag to make members.\n"
	"Select thruster mode.\tPress a digit, 0-9.  Drag to make thruster.\n"
	"Select connector mode.\tPress a digit, 0-9.  "
	"Drag to make breakable connector.\n\n"
	"Press $$GREEN$$<SPACE>$$FG$$ to run the game.  Press digits to operate "
	"thrusters and break connectors.\n\n"
	"Press $$GREEN$$<z>$$FG$$ or $$GREEN$$<SHIFT-Z>$$FG$$ to zoom in/out.\n\n";

  PressAKey;
  bt_main_mode=CtrlButtonNew(
	(GR_WIDTH-4*FONT_WIDTH-16)>>1,1*FONT_HEIGHT+4,80,,MMD_NUM_MODES,
	Define("ST_MAIN_MODES"),main_mode_colors,&main_mode_button);
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
  CtrlButtonDel(bt_main_mode);
}
             øÿÿÿøÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ             øÿÿÿøÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        ‚     ùÿÿÿöÿÿÿ      ÿÿÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿ                                                                                ÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿÿÿ   ÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿÿÿÿÿÿ   