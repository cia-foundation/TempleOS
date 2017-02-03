#define BALLS_NUM	7
#define SPRINGS_NUM	3

#define STRETCH 	500.0
#define GRAVITY 	50.0 //not really gravity
#define BALL_RADIUS	5
#define BASE_SIZE	10

CMass balls[BALLS_NUM];
CSpring springs[SPRINGS_NUM];
F64 collision_t;

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,
	cx=task->pix_width>>1,
	cy=task->pix_height>>1;
  Bool snd_on=FALSE;
  dc->color=BLACK;
  GrPrint(dc,0,0,"Protect your base.");
  GrRect(dc,cx-BASE_SIZE,cy-BASE_SIZE,BASE_SIZE*2,BASE_SIZE*2);
  dc->color=CYAN;
  GrRect(dc,cx-BASE_SIZE+2,cy-BASE_SIZE+2,BASE_SIZE*2-4,BASE_SIZE*2-4);
  dc->color=YELLOW;
  GrLine(dc,balls[0].x,balls[0].y,
	ms.pos.x-task->pix_left-task->scroll_x,
	ms.pos.y-task->pix_top-task->scroll_y);
  for (i=0;i<SPRINGS_NUM;i++)
    GrLine(dc,springs[i].end1->x,springs[i].end1->y,
	  springs[i].end2->x,springs[i].end2->y);

  dc->color=LTCYAN;
  GrCircle(dc,balls[0].x,balls[0].y,BALL_RADIUS);
  GrFloodFill(dc,balls[0].x,balls[0].y,TRUE);
  dc->color=BLACK;
  GrCircle(dc,balls[0].x,balls[0].y,BALL_RADIUS);

  for (i=1;i<BALLS_NUM;i++) {
    dc->color=LTPURPLE;
    GrCircle(dc,balls[i].x,balls[i].y,BALL_RADIUS);
    GrFloodFill(dc,balls[i].x,balls[i].y,TRUE);
    if (cx-BASE_SIZE-BALL_RADIUS<=balls[i].x<=cx+BASE_SIZE+BALL_RADIUS &&
	  cy-BASE_SIZE-BALL_RADIUS<=balls[i].y<=cy+BASE_SIZE+BALL_RADIUS)
      snd_on=TRUE;
    dc->color=BLACK;
    GrCircle(dc,balls[i].x,balls[i].y,BALL_RADIUS);
  }
  if (snd_on)
    Snd(74);
  else
    Snd;
}

U0 MyDerivative(CMathODE *ode,F64 t,COrder2D3 *,COrder2D3 *)
{
  I64 i,j;
  F64 d,dd;
  CD3 p,p2;
  CTask *task=ode->win_task;

  D3SubEqu(D3Equ(&p2,
	ms.pos.x-task->pix_left-task->scroll_x,
	ms.pos.y-task->pix_top-task->scroll_y,0),
	&balls[0].state->x);
  D3AddEqu(&balls[0].DstateDt->DxDt,D3MulEqu(&p2,STRETCH));

  D3Equ(&p2,task->pix_width>>1,task->pix_height>>1,0);
  for (i=1;i<BALLS_NUM;i++) {
    D3Sub(&p,&p2,&balls[i].state->x);
    if (d=D3Norm(&p)) {
//Gravity would be /(d*d*d), but that's too exponential.
      D3MulEqu(&p,GRAVITY/d);
      D3AddEqu(&balls[i].DstateDt->DxDt,&p);
    }
  }

  for (i=0;i<BALLS_NUM;i++)
    for (j=i+1;j<BALLS_NUM;j++) {
      D3Sub(&p,&balls[j].state->x,&balls[i].state->x);
      dd=D3NormSqr(&p);
      if (dd<=(2*BALL_RADIUS)*(2*BALL_RADIUS)) {
	if (t-collision_t>0.05) {
	  Noise(50,102,105);
	  collision_t=t;
	}
	d=Sqrt(dd)+0.0001;
	dd=10.0*Sqr(Sqr((2*BALL_RADIUS)*(2*BALL_RADIUS)-dd));
	D3MulEqu(&p,dd/d);
	D3AddEqu(&balls[j].DstateDt->DxDt,&p);
	D3SubEqu(&balls[i].DstateDt->DxDt,&p);
      }
    }

  d=balls[0].state->x;
  if (d-BALL_RADIUS<0)
    balls[0].DstateDt->DxDt+=Sqr(Sqr(Sqr(d-BALL_RADIUS)));
  if (d+BALL_RADIUS>task->pix_width)
    balls[0].DstateDt->DxDt-=Sqr(Sqr(Sqr((d+BALL_RADIUS)-task->pix_width)));

  d=balls[0].state->y;
  if (d-BALL_RADIUS<0)
    balls[0].DstateDt->DyDt+=Sqr(Sqr(Sqr(d-BALL_RADIUS)));
  if (d+BALL_RADIUS>task->pix_height)
    balls[0].DstateDt->DyDt-=Sqr(Sqr(Sqr((d+BALL_RADIUS)-task->pix_height)));
}

U0 Whap()
{
  I64 i;
  CMathODE *ode=ODENew(0,1e-2,ODEF_HAS_MASSES);

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  AutoComplete;
  WinBorder;
  WinMax;

  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	);
  ode->derive=&MyDerivative;
  ode->drag_v2=0.002;
  ode->drag_v3=0.00001;
  ode->acceleration_limit=5e3;
  MemSet(balls,0,BALLS_NUM*sizeof(CMass));
  D3Equ(&balls[0].x,100,100,0);
  for (i=1;i<BALLS_NUM;i++)
    D3Equ(&balls[i].x,
	  RandI16%500+Fs->pix_width>>1,RandI16%500+Fs->pix_height>>1,0);
  balls[0].x=ms.pos.x-Fs->pix_left-Fs->scroll_x;
  balls[0].y=ms.pos.y-Fs->pix_top-Fs->scroll_y;
  for (i=0;i<BALLS_NUM;i++) {
    balls[i].mass=1.0;
    balls[i].drag_profile_factor=1.0;
    QueIns(&balls[i],ode->last_mass);
  }
  balls[2].x=balls[1].x+15;
  balls[2].y=balls[1].y;
  balls[3].x=balls[1].x;
  balls[3].y=balls[1].y+15;
  MemSet(springs,0,SPRINGS_NUM*sizeof(CSpring));
  springs[0].end1=&balls[1];
  springs[0].end2=&balls[2];
  springs[0].rest_len=15;
  springs[0].const=10000;
  QueIns(&springs[0],ode->last_spring);
  springs[1].end1=&balls[1];
  springs[1].end2=&balls[3];
  springs[1].rest_len=15;
  springs[1].const=10000;
  QueIns(&springs[1],ode->last_spring);
  springs[2].end1=&balls[2];
  springs[2].end2=&balls[3];
  springs[2].rest_len=sqrt2*15;
  springs[2].const=10000;
  QueIns(&springs[2],ode->last_spring);

  collision_t=0;
  QueIns(ode,Fs->last_ode);

  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  GetChar;
  SettingsPop;
  QueRem(ode);
  ODEDel(ode);
  MenuPop;
}

Whap;
