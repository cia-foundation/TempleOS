/* I did not limit myself to real physics.  It's a game.
I suppose the gun shoots plazma blobs that have a delayed
explosion due to a chemical reaction timer.

The caliber of the gun is adjustible.

I did a simple anti-spin controller when you press cursor-down.
It's intentionally not very good.  It needs to take into
account flexing of the struts.  You'll notice when you
try to shoot the laser.

If you are interested in control theory, you might download
SimStructure, a program I wrote.

http://www.templeos.org/files/SimStrSetUp.zip

There's not sound in space and explosions
don't make shock waves, or something like that.
In X-Caliber, ships are destroyed by
causing springs to break due to acceleration
differences on different masses in a ship.

A real game would be dull with guided
missiles that always hit and lasers
that never miss.

Don't be anal.  If you aspire to making
games, do like Hollywood.  They know the
public isn't interested in realism.

*/

// ********************************** Glbls
AcctRegSetDftEntry("TempleOS/XCaliber","I64 best_score=0;\n");
AcctRegExeBranch("TempleOS/XCaliber");

class MyMass:CMass
{
  F64 temperature;
  F64 radius;
  I64 color;
};

class MySpring:CSpring
{
  F64 strength;
  I64 color;
};


#define SPIN_GAIN		0.25
#define MAX_MASSES		8
#define MAX_SPRINGS		16
#define ST_HUMAN1		0
#define ST_ENEMY1		1
#define ST_ENEMY2		2
class Ship
{
  Ship *next,*last;
  I64 type,masses,springs;
  MyMass   p[MAX_MASSES];
  MySpring s[MAX_SPRINGS];
  F64 fire_rate;
  F64 reload_timeout,spacewalk_timeout;
  F64 die_time,die_timeout;
  I64 spacewalk_side;
  F64 laser_temperature;
  Bool lasering,exploding,laser_overheat;
} *human;

F64 human_t_left,human_t_right,human_antispin;

class Bullet
{
  Bullet *next,*last;
  F64 radius;
  F64 fuse_time,die_timeout;
  MyMass p;
  Bool exploding;
};

#define MAX_THRUST	200.0
#define MAX_ANTISPIN	25.0
#define SPACEWALK_TIME	7.5

#define CMD_NULL	0
#define CMD_SPIN_LEFT	1
#define CMD_SPIN_RIGHT	2
#define CMD_THRUST	3
#define CMD_FIRE	4
#define CMD_EXIT	5
Bool game_over,show_level_msg;

#define NUM_STARS	100
I64 stars_x[NUM_STARS],stars_y[NUM_STARS];

Ship ship_root;
Bullet bullet_root;
CMathODE *ode=NULL;
I64 level,score,remaining;

$BG,0$
$SP,"<1>",1$

$SP,"<2>",2$

$SP,"<3>",3$


$SP,"<4>",4$


$SP,"<5>",5$


$SP,"<6>",6$

$BG$
// ********************************** Ship
Bool CheckOverlap()
{
  CD3 p;
  MyMass *tempm,*tempm1;
  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    tempm1=ode->next_mass;
    while (tempm1!=&ode->next_mass) {
      if (tempm!=tempm1) {
	D3Sub(&tempm->x,&tempm1->x,&p);
	if (D3SqrNorm(&p)<=Sqr(tempm->radius+tempm1->radius))
	  return TRUE;
      }
      tempm1=tempm1->next;
    }
    tempm=tempm->next;
  }
  return FALSE;
}

Ship *ShipNew(I64 x,I64 y,I64 type)
{
  I64 i;
  Ship *temps=CAlloc(sizeof(Ship));

  switch (temps->type=type) {
    case ST_HUMAN1:
      temps->fire_rate=25;
      temps->masses=5;
      temps->p[0].x=x;
      temps->p[0].y=y;
      temps->p[1].x=x+3;
      temps->p[1].y=y+10;
      temps->p[2].x=x-3;
      temps->p[2].y=y+10;
      temps->p[3].x=x+20;
      temps->p[3].y=y+20;
      temps->p[4].x=x-20;
      temps->p[4].y=y+20;

      for (i=0;i<temps->masses;i++) {
	temps->p[i].mass=1;
	if (i<3)
	  temps->p[i].radius=2.5;
	else
	  temps->p[i].radius=4;
	temps->p[i].drag_profile_factor=3;
	QueIns(&temps->p[i],ode->last_mass);
      }
      temps->p[3].mass/=10.0;
      temps->p[4].mass/=10.0;

      temps->springs=7;
      temps->s[0].end1=&temps->p[0];
      temps->s[0].end2=&temps->p[1];
      temps->s[1].end1=&temps->p[2];
      temps->s[1].end2=&temps->p[0];
      temps->s[2].end1=&temps->p[1];
      temps->s[2].end2=&temps->p[2];
      temps->s[3].end1=&temps->p[1];
      temps->s[3].end2=&temps->p[3];
      temps->s[4].end1=&temps->p[0];
      temps->s[4].end2=&temps->p[3];
      temps->s[5].end1=&temps->p[2];
      temps->s[5].end2=&temps->p[4];
      temps->s[6].end1=&temps->p[0];
      temps->s[6].end2=&temps->p[4];

      for (i=0;i<temps->springs;i++) {
	temps->s[i].rest_len=D3Dist(&temps->s[i].end1->x,&temps->s[i].end2->x);
	temps->s[i].constant=10000;
	temps->s[i].strength  =30000;
	if (i<=2)
	  temps->s[i].color=LTCYAN;
	else
	  temps->s[i].color=LTGRAY;
	QueIns(&temps->s[i],ode->last_spring);
      }
      remaining=0;

      break;
    case ST_ENEMY1:
      temps->fire_rate=2.5;
      temps->masses=3;
      temps->p[0].x=x;
      temps->p[0].y=y;
      temps->p[1].x=x+15;
      temps->p[1].y=y;
      temps->p[2].x=x;
      temps->p[2].y=y+15;

      for (i=0;i<temps->masses;i++) {
	temps->p[i].mass=1;
	temps->p[i].radius=7;
	temps->p[i].drag_profile_factor=3;
	QueIns(&temps->p[i],ode->last_mass);
      }

      temps->springs=3;
      temps->s[0].end1=&temps->p[0];
      temps->s[0].end2=&temps->p[1];
      temps->s[1].end1=&temps->p[1];
      temps->s[1].end2=&temps->p[2];
      temps->s[2].end1=&temps->p[2];
      temps->s[2].end2=&temps->p[0];

      for (i=0;i<temps->springs;i++) {
	temps->s[i].rest_len=D3Dist(&temps->s[i].end1->x,&temps->s[i].end2->x);
	temps->s[i].constant=10000;
	temps->s[i].strength  =20000;
	temps->s[i].color=BLACK;
	QueIns(&temps->s[i],ode->last_spring);
      }
      remaining++;
      break;
    case ST_ENEMY2:
      temps->fire_rate=5.0;
      temps->masses=5;
      temps->p[0].x=x;
      temps->p[0].y=y;
      temps->p[1].x=x-7;
      temps->p[1].y=y+10;
      temps->p[2].x=x+7;
      temps->p[2].y=y+10;
      temps->p[3].x=x-14;
      temps->p[3].y=y+20;
      temps->p[4].x=x+14;
      temps->p[4].y=y+20;

      for (i=0;i<temps->masses;i++) {
	temps->p[i].mass=1;
	temps->p[i].radius=6;
	temps->p[i].drag_profile_factor=5;
	temps->p[i].color=PURPLE;
	QueIns(&temps->p[i],ode->last_mass);
      }

      temps->springs=7;
      temps->s[0].end1=&temps->p[0];
      temps->s[0].end2=&temps->p[1];
      temps->s[1].end1=&temps->p[0];
      temps->s[1].end2=&temps->p[2];
      temps->s[2].end1=&temps->p[1];
      temps->s[2].end2=&temps->p[2];
      temps->s[3].end1=&temps->p[1];
      temps->s[3].end2=&temps->p[3];
      temps->s[4].end1=&temps->p[2];
      temps->s[4].end2=&temps->p[4];
      temps->s[5].end1=&temps->p[2];
      temps->s[5].end2=&temps->p[3];
      temps->s[6].end1=&temps->p[1];
      temps->s[6].end2=&temps->p[4];

      for (i=0;i<temps->springs;i++) {
	temps->s[i].rest_len=D3Dist(&temps->s[i].end1->x,&temps->s[i].end2->x);
	temps->s[i].constant= 40000;
	temps->s[i].strength  =75000;
	if (i>=3)
	  temps->s[i].color=LTPURPLE;
	else
	  temps->s[i].color=BLACK;
	QueIns(&temps->s[i],ode->last_spring);
      }
      remaining++;
      break;
  }
  QueIns(temps,ship_root.last);
  return temps;
}

U0 ShipDel(Ship *temps)
{
  I64 i;
  if (!temps) return;
  for (i=0;i<temps->masses;i++)
    QueRem(&temps->p[i]);
  for (i=0;i<temps->springs;i++)
    QueRem(&temps->s[i]);
  QueRem(temps);
  Free(temps);
  remaining--;
}

U0 PlaceShip(I64 type)
{
  Ship *ee;
  if (CheckOverlap)
    return;
  while (TRUE) {
    ee=ShipNew(RandU16%(Fs->win_pixel_width-20)+10,RandU16%(Fs->win_pixel_height-20)+10,type);
    if (CheckOverlap)
      ShipDel(ee);
    else
      break;
  }
}

// ********************************** Human Ship

I64 Tweaked()
{
  CD3 p,p1,p2;
  if (human) {
    D3Sub(&human->p[0].x,&human->p[1].x,&p1);
    D3Sub(&human->p[0].x,&human->p[2].x,&p2);
    D3Add(&p1,&p2,&p);
    D3Unit(&p);
    D3Sub(&human->p[0].x,&human->p[3].x,&p1);
    D3Sub(&human->p[0].x,&human->p[4].x,&p2);
    D3Unit(&p1);
    D3Unit(&p2);
    if (!(human->s[3].flags&SSF_INACTIVE) && D3Dot(&p,&p1)>Cos(20*ã/180))
      return 3;
    if (!(human->s[5].flags&SSF_INACTIVE) && D3Dot(&p,&p2)>Cos(20*ã/180))
      return 4;
    return 0;
  }
}

U0 AllDel(CMathODE *ode)
{
  Ship *temps,*temps1;
  QueRem(ode);
  temps=ship_root.next;
  while (temps!=&ship_root) {
    temps1=temps->next;
    ShipDel(temps);
    temps=temps1;
  }
  human=NULL;
  QueDel(&bullet_root,TRUE);
  ODEDel(ode);
}

Bool LaserPlot(CDC *dc,I64 x,I64 y,I64)
{
  I64 c;
  c=GrPeek(dc,x,y);
  if (c!=BLACK && c!=WHITE)
    return FALSE;
  else {
    GrPlot(dc,x,y);
    return TRUE;
  }
}

// **********************************
U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j;
  F64 arg;
  Ship *temps;
  Bullet *tempb;
  CD3 p,p1,p2;
  F64 t_left,t_right,spin,d,x,y;
  MySpring *tempss;
  U8 *img;
  Bool draw_laser_line=FALSE;

  if (ode!=task->last_ode) return;

  dc->color=WHITE;
  GrPrintF(dc,0,0,"Level:%d Score:%d High Score:%d",level,score,best_score);
  if (game_over) {
    if (Blink)
      GrPutS(dc,(task->win_pixel_width-9*FONT_WIDTH)/2,
	  (task->win_pixel_height-FONT_HEIGHT)/2,"Game Over");
  } else if (show_level_msg) {
    if (Blink)
      GrPrintF(dc,(task->win_pixel_width-8*FONT_WIDTH)/2,
	  (task->win_pixel_height-FONT_HEIGHT)/2+50,"Level %d",level);
  }

  for (i=0;i<NUM_STARS;i++)
    GrPlot(dc,stars_x[i],stars_y[i]);

  tempss=ode->next_spring;
  while (tempss!=&ode->next_spring) {
    if (!(tempss->flags&SSF_INACTIVE)) {
      dc->color=tempss->color;
      GrLine(dc,tempss->end1->x,tempss->end1->y,
	  tempss->end2->x,tempss->end2->y);
    }
    tempss=tempss->next;
  }

  temps=ship_root.next;
  while (temps!=&ship_root) {
    if (!temps->exploding)
      switch (temps->type) {
	case ST_HUMAN1:
	  if (temps->spacewalk_side) {
	    t_left=0;
	    t_right=0;
	  } else {
	    if (d=D3Norm(D3Sub(&temps->p[0].x,&temps->p[1].x,&p1))) {
	      D3Sub(&temps->p[0].DxDt,&temps->p[1].DxDt,&p2);
	      D3Cross(&p1,&p2,&p);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_left =Limit(human_t_left+SPIN_GAIN*spin*human_antispin,0,MAX_THRUST);

	    if (d=D3Norm(D3Sub(&temps->p[0].x,&temps->p[2].x,&p1))) {
	      D3Sub(&temps->p[0].DxDt,&temps->p[2].DxDt,&p2);
	      D3Cross(&p1,&p2,&p);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_right=Limit(human_t_right-SPIN_GAIN*spin*human_antispin,0,MAX_THRUST);
	  }


	  D3Sub(&temps->p[1].x,&temps->p[0].x,&p1);
	  D3Sub(&temps->p[2].x,&temps->p[0].x,&p2);
	  D3Unit(D3Add(&p1,&p2,&p));

	  if (!(temps->s[3].flags&SSF_INACTIVE)) {
	    dc->color=YELLOW;
	    D3AddEqu(D3Mul(t_left/25,&p,&p1),&temps->p[3].x);
	    GrLine(dc,temps->p[1].x,temps->p[1].y,p1.x,p1.y);
	    arg=Arg(p.x,p.y);
	    SpritePlotRotZ3b(dc,temps->p[3].x,temps->p[3].y,0,$IB,"<thruster>",1$,arg);
	  }

	  if (!(temps->s[5].flags&SSF_INACTIVE)) {
	    dc->color=YELLOW;
	    D3AddEqu(D3Mul(t_right/25,&p,&p2),&temps->p[4].x);
	    GrLine(dc,temps->p[2].x,temps->p[2].y,p2.x,p2.y);
	    arg=Arg(p.x,p.y);
	    SpritePlotRotZ3b(dc,temps->p[4].x,temps->p[4].y,0,$IB,"<thruster>",1$,arg);
	  }

	  if (tT>temps->reload_timeout)
	    img=$IB,"<gun_ready>",2$;
	  else
	    img=$IB,"<gun_busy>",3$;
	  arg=Arg(p.x,p.y);
	  switch (level) {
	    case 3:
	      if (!(temps->s[3].flags&SSF_INACTIVE))
		SpritePlotRotZ3b(dc,temps->p[3].x,temps->p[3].y,0,img,arg);
	      if (!(temps->s[5].flags&SSF_INACTIVE))
		SpritePlotRotZ3b(dc,temps->p[4].x,temps->p[4].y,0,img,arg);
	    case 2:
	      if (!(temps->s[1].flags&SSF_INACTIVE))
		SpritePlotRotZ3b(dc,temps->p[1].x,temps->p[1].y,0,img,arg);
	      if (!(temps->s[2].flags&SSF_INACTIVE))
		SpritePlotRotZ3b(dc,temps->p[2].x,temps->p[2].y,0,img,arg);
	    case 1:
	      SpritePlotRotZ3b(dc,temps->p[0].x,temps->p[0].y,0,img,arg);
	      break;
	    default:
	      SpritePlotRotZ3b(dc,temps->p[0].x,temps->p[0].y,0,$IB,"<Laser>",6$,arg);
	      if (temps->lasering && !temps->laser_overheat) {
		draw_laser_line=TRUE;
		Snd(1000);
	      }
	  }

	  ctrl_panel.laser_temperature=temps->laser_temperature;

	  if (temps->spacewalk_side) {
	    d=1.0-(temps->spacewalk_timeout-tT)/SPACEWALK_TIME;
	    if (d>1.0) {
	      temps->spacewalk_side=0;
	      ctrl_panel.spacewalk=FALSE;
	    } else {
	      if (d<0.5) {
		d=d*2;
		x=temps->p[0].x*(1.0-d)+temps->p[temps->spacewalk_side].x*(d);
		y=temps->p[0].y*(1.0-d)+temps->p[temps->spacewalk_side].y*(d);
	      } else {
		d=(d-0.5)*2;
		x=temps->p[temps->spacewalk_side].x*(1.0-d)+temps->p[0].x*(d);
		y=temps->p[temps->spacewalk_side].y*(1.0-d)+temps->p[0].y*(d);
	      }
	      SpritePlotRotZ3b(dc,x,y,0,$IB,"<spacewalk>",4$,arg+0.75*Sin(tT*2));
	    }
	  } else {
	    if (ctrl_panel.spacewalk) {
	      if (temps->spacewalk_side=Tweaked)
		temps->spacewalk_timeout=tT+SPACEWALK_TIME;
	      else
		ctrl_panel.spacewalk=FALSE;
	    }
	  }
	  break;
	case ST_ENEMY2:
	  for (i=3;i<temps->masses;i++) {
	    dc->color=temps->p[i].color;
	    GrCircle(dc,temps->p[i].x,temps->p[i].y,temps->p[i].radius);
	    GrFloodFill(dc,temps->p[i].x,temps->p[i].y+2,TRUE);
	    dc->color=WHITE;
	    GrCircle(dc,temps->p[i].x,temps->p[i].y,temps->p[i].radius);
	  }
	  case ST_ENEMY1:
	  D3DivEqu(D3Sub(&temps->p[1].x,&temps->p[0].x,&p1),2.0);
	  D3DivEqu(D3Sub(&temps->p[2].x,&temps->p[0].x,&p2),2.0);
	  D3Unit(D3Add(&p1,&p2,&p));
	  if (tT>temps->reload_timeout)
	    img=$IB,"<gun_ready>",2$;
	  else
	    img=$IB,"<gun_busy>",3$;
	  arg=Arg(p.x,p.y);
	  SpritePlotRotZ3b(dc,temps->p[0].x,temps->p[0].y,0,img,arg);
	  arg=Arg(p1.x,p1.y);
	  SpritePlotRotZ3b(dc,temps->p[0].x+p1.x,temps->p[0].y+p1.y,0,$IB,"<EnemySide>",5$,arg);
	  arg=Arg(p2.x,p2.y);
	  SpritePlotRotZ3b(dc,temps->p[0].x+p2.x,temps->p[0].y+p2.y,0,$IB,"<EnemySide>",5$,arg);
	  for (i=0;i<temps->masses;i++) {
	    dc->color=YELLOW;
	    if (temps->p[i].temperature>=1.0)
	      GrCircle(dc,temps->p[i].x,temps->p[i].y,temps->p[i].temperature);
	  }
	  break;
      }
    else if (temps->die_time<=tT<=temps->die_timeout)
      for (j=0;j<temps->masses;j++) {
	d=(tT-temps->die_time)/(temps->die_timeout-temps->die_time);
	d=7*Sin(ã*d)*(6+j)+1;
	for (i=1;i<d;i++) {
	  if (i&1)
	    dc->color=YELLOW;
	  else
	    dc->color=LTRED;
	  GrCircle(dc,temps->p[j].x,temps->p[j].y,i);
	}
      }
    temps=temps->next;
  }

  tempb=bullet_root.next;
  while (tempb!=&bullet_root) {
    if (tT>tempb->fuse_time) {
      d=(tT-tempb->fuse_time)/(tempb->die_timeout-tempb->fuse_time);
      d=7*Sin(ã*d)*tempb->radius+1;
      for (i=1;i<d;i++) {
	if (i&1)
	  dc->color=YELLOW;
	else
	  dc->color=LTRED;
	GrCircle(dc,tempb->p.x,tempb->p.y,i);
      }
      } else {
      if (tempb->radius<1.0) {
	dc->color=LTGREEN;
	GrPlot(dc,tempb->p.x,tempb->p.y);
      } else {
	dc->color=YELLOW;
	GrCircle(dc,tempb->p.x,tempb->p.y,tempb->radius);
	if (tempb->radius>=2.0)
	  GrFloodFill(dc,tempb->p.x,tempb->p.y,TRUE);
	dc->color=LTGREEN;
	GrCircle(dc,tempb->p.x,tempb->p.y,tempb->radius);
      }
    }
    tempb=tempb->next;
  }

  if (human && draw_laser_line) {
    D3Sub(&human->p[1].x,&human->p[0].x,&p1);
    D3Sub(&human->p[2].x,&human->p[0].x,&p2);
    D3Unit(D3Add(&p1,&p2,&p));
    dc->color=LTBLUE;
    LOSLine(dc,human->p[0].x-10*p.x,human->p[0].y-10*p.y,0,human->p[0].x-800*p.x,human->p[0].y-800*p.y,0,&LaserPlot);
  }
}

U0 Explosion(MyMass *tempm1,F64 r)
{
  MyMass *tempm;
  CD3 p1;
  F64 d;

  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    if (tempm!=tempm1) {
      D3Sub(&tempm->state->x,&tempm1->state->x,&p1);
      d=D3SqrNorm(&p1)-tempm->radius*tempm->radius;
      if (d<100.0*100.0) {
	if (d<1)
	  d=1;
	else
	  d=Sqrt(d);
	d=2e8*r`7/d`6;
	if (d>1e5) d=1e5;
	D3MulEqu(&p1,d);
	D3AddEqu(&tempm->DstateDt->DxDt,&p1);
      }
    }
    tempm=tempm->next;
  }
}

U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{
  I64 i;
  F64 d,dd,dd2,spin,t_left,t_right;
  CTask *task=ode->win_task;
  CD3 p,p1,p2;
  Bullet *tempb;
  Ship *temps;
  MyMass *tempm,*tempm1;

  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    d=tempm->state->x;
    if (d-tempm->radius<0)
      tempm->DstateDt->DxDt+=Sqr(Sqr(Sqr(d-tempm->radius)));
    if (d+tempm->radius>task->win_pixel_width)
      tempm->DstateDt->DxDt-=Sqr(Sqr(Sqr((d+tempm->radius)-task->win_pixel_width)));

    d=tempm->state->y;
    if (d-tempm->radius<0)
      tempm->DstateDt->DyDt+=Sqr(Sqr(Sqr(d-tempm->radius)));
    if (d+tempm->radius>task->win_pixel_height)
      tempm->DstateDt->DyDt-=Sqr(Sqr(Sqr((d+tempm->radius)-task->win_pixel_height)));
    tempm1=ode->next_mass;
    while (tempm1!=&ode->next_mass) {
      if (tempm!=tempm1) {
	D3Sub(&tempm->state->x,&tempm1->state->x,&p);
	dd=D3SqrNorm(&p);
	dd2=Sqr(tempm->radius+tempm1->radius);
	if (dd<=dd2) {
	  d=Sqrt(dd)+0.0001;
	  D3MulEqu(&p,Sqr(Sqr(dd2-dd))/d);
	  D3AddEqu(&tempm ->DstateDt->DxDt,&p);
	  D3SubEqu(&tempm1->DstateDt->DxDt,&p);
	}
      }
      tempm1=tempm1->next;
    }
    tempm=tempm->next;
  }


  temps=ship_root.next;
  while (temps!=&ship_root) {
    if (temps->exploding && temps->die_time<=tT<=temps->die_timeout)
      for (i=0;i<temps->masses;i++)
	Explosion(&temps->p[i],temps->p[i].radius/3.0);
    switch (temps->type) {
      case ST_HUMAN1:
	if (!temps->exploding) {
	  if (temps->spacewalk_side) {
	    t_left=0;
	    t_right=0;
	    d=1.0-(temps->spacewalk_timeout-tT)/SPACEWALK_TIME;
	    if (0.485<d<0.515) {
	      D3Unit(D3Sub(&temps->p[2].state->x,&temps->p[1].state->x,&p));
	      if (temps->spacewalk_side==3) {
		temps->p[3].DstateDt->DxDt-=10*MAX_THRUST*p.x;
		temps->p[3].DstateDt->DyDt-=10*MAX_THRUST*p.y;
		temps->p[1].DstateDt->DxDt+=10*MAX_THRUST*p.x;
		temps->p[1].DstateDt->DyDt+=10*MAX_THRUST*p.y;
	      } else {
		temps->p[4].DstateDt->DxDt+=10*MAX_THRUST*p.x;
		temps->p[4].DstateDt->DyDt+=10*MAX_THRUST*p.y;
		temps->p[2].DstateDt->DxDt-=10*MAX_THRUST*p.x;
		temps->p[2].DstateDt->DyDt-=10*MAX_THRUST*p.y;
	      }
	    }
	  } else {
	    if (d=D3Norm(D3Sub(&temps->p[0].state->x,&temps->p[1].state->x,&p1))) {
	      D3Sub(&temps->p[0].state->DxDt,&temps->p[1].state->DxDt,&p2);
	      D3Cross(&p1,&p2,&p);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_left =Limit(human_t_left+SPIN_GAIN*spin*human_antispin,0,MAX_THRUST);

	    if (d=D3Norm(D3Sub(&temps->p[0].state->x,&temps->p[2].state->x,&p1))) {
	      D3Sub(&temps->p[0].state->DxDt,&temps->p[2].state->DxDt,&p2);
	      D3Cross(&p1,&p2,&p);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_right=Limit(human_t_right-SPIN_GAIN*spin*human_antispin,0,MAX_THRUST);

	    D3Sub(&temps->p[0].state->x,&temps->p[1].state->x,&p1);
	    D3Sub(&temps->p[0].state->x,&temps->p[2].state->x,&p2);
	    D3Unit(D3Add(&p1,&p2,&p));
	    if (!(temps->s[3].flags&SSF_INACTIVE)) {
	      D3Mul(t_left,&p,&p1);
	      D3AddEqu(&temps->p[3].DstateDt->DxDt,&p1);
	    }
	    if (!(temps->s[5].flags&SSF_INACTIVE)) {
	      D3Mul(t_right,&p,&p2);
	      D3AddEqu(&temps->p[4].DstateDt->DxDt,&p2);
	    }
	  }
	}
	break;
    }
    temps=temps->next;
  }

  tempb=bullet_root.next;
  while (tempb!=&bullet_root) {
    if (tempb->fuse_time<=tT<=tempb->die_timeout)
      Explosion(&tempb->p,tempb->radius);
    tempb=tempb->next;
  }
}


U0 CheckDamage()
{
  I64 i,j,k;
  Ship *temps,*temps1;
  MyMass *tempm,*best_mass;
  CD3 p,p1,p2;
  F64 d,best_distance;

  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    tempm->temperature*=0.9;
    tempm=tempm->next;
  }
  if (human) {
    human->laser_temperature*=0.975;

    if (human->laser_overheat) {
      if (human->laser_temperature<LASER_THRESHOLD_TEMP)
	human->laser_overheat=FALSE;
    }
    if (!human->laser_overheat && human->lasering) {
      if (human->laser_temperature>=LASER_MAX_TEMP) {
	human->laser_overheat=TRUE;
	Snd(0);
      } else {
	human->laser_temperature+=1.0;
	D3Sub(&human->p[0].x,&human->p[1].x,&p1);
	D3Sub(&human->p[0].x,&human->p[2].x,&p2);
	D3Unit(D3Add(&p1,&p2,&p));
	p2.x=p.y;
	p2.y=-p.x;
	p2.z=0;
	best_mass=NULL;
	best_distance=MAX_F64;
	tempm=ode->next_mass;
	while (tempm!=&ode->next_mass) {
	  D3Sub(&human->p[0].x,&tempm->x,&p1);
	  if (Abs(D3Dot(&p1,&p2))<tempm->radius &&
	      D3Dot(&p1,&p)<0.0) {
	    d=D3SqrNorm(&p1);
	    if (d<best_distance) {
	      best_distance=d;
	      best_mass=tempm;
	    }
	  }
	  tempm=tempm->next;
	}
	if (best_mass)
	  best_mass->temperature+=1.0;
      }
    }
  }

  temps=ship_root.next;
  while (temps!=&ship_root) {
    temps1=temps->next;
    switch (temps->type) {
      case ST_HUMAN1:
	if (temps->exploding) {
	  if (tT>temps->die_timeout)  {
	    ShipDel(temps);
	    human=NULL;
	  }
	} else
	  for (i=0;i<temps->springs;i++) {
	    if (Abs(temps->s[i].f)>temps->s[i].strength)
	      temps->s[i].flags|=SSF_INACTIVE;
	    if (temps->s[i].flags&SSF_INACTIVE) {
	      if (i<3) {
		if (!temps->exploding) {
		  temps->exploding=TRUE;
		  temps->die_time=tT;
		  temps->die_timeout=tT+0.75;
		  game_over=TRUE;
		  Noise(750,1000,2000);
		  break;
		}
	      }
	    }
	  }
	  break;
      default:
	if (temps->exploding) {
	  if (tT>temps->die_timeout) {
	    ShipDel(temps);
	    score+=level;
	    if (score>best_score)
	      best_score=score;
	  }
	} else {
	  j=0;
	  for (i=0;i<temps->springs;i++) {
	    if (temps->s[i].flags&SSF_INACTIVE)
	      j++;
	    else if (Abs(temps->s[i].f)>temps->s[i].strength) {
	      temps->s[i].flags|=SSF_INACTIVE;
	      j++;
	    }
	  }
	  k=0;
	  for (i=0;i<temps->masses;i++) {
	    if (temps->p[i].temperature>MASS_MAX_TEMP)
	      k++;
	  }
	  if (j>1 || k) {
	    temps->exploding=TRUE;
	    temps->die_time=tT;
	    temps->die_timeout=tT+0.75;
	    Noise(750,1000,3000);
	  }
	}
    }
    temps=temps1;
  }
}

// ********************************** Bullets

U0 FireOneGun(Ship *temps,I64 g,F64 r,F64 fuse_time)
{
  CD3 p,p1,p2;
  Bullet *tempb;

  tempb=CAlloc(sizeof(Bullet));
  D3Copy(&tempb->p.x,&temps->p[g].x);
  tempb->radius=r;
  tempb->fuse_time=tT+1.5*fuse_time;
  tempb->die_timeout=tempb->fuse_time+0.125;
  tempb->p.mass=0.3*r*r*r;
  D3Sub(&temps->p[0].x,&temps->p[1].x,&p1);
  D3Sub(&temps->p[0].x,&temps->p[2].x,&p2);
  D3Add(&p1,&p2,&p);
  D3AddEqu(&tempb->p.x,D3MulEqu(D3Copy(&p1,D3Unit(&p)),r+temps->p[0].radius+5));
  D3AddEqu(D3MulEqu(&p,1000/(r+1)),&temps->p[g].DxDt);
  D3Copy(&tempb->p.DxDt,&p);
  D3MulEqu(&p,tempb->p.mass/temps->p[g].mass/100.0);
  D3SubEqu(&temps->p[g].DxDt,&p);
  QueIns(&tempb->p,ode->last_mass);
  QueIns(tempb,bullet_root.last);
  temps->reload_timeout=tT+r/temps->fire_rate;
  Noise(100,r*200,r*400);
}

U0 HumanFireBegin()
{

  F64 r=3.0*ctrl_panel.bullet_radius/CTRL_PANEL_RANGE+0.5,
     fuse_time=ToF64(ctrl_panel.fuse_time+1)/CTRL_PANEL_RANGE;
  if (human) {
    if (!human->exploding && !human->spacewalk_side && tT>human->reload_timeout)
      switch (level) {
	case 3:
	  if (!(human->s[3].flags&SSF_INACTIVE))
	    FireOneGun(human,3,r,fuse_time);
	  if (!(human->s[5].flags&SSF_INACTIVE))
	    FireOneGun(human,4,r,fuse_time);
	case 2:
	  if (!(human->s[1].flags&SSF_INACTIVE))
	    FireOneGun(human,1,r,fuse_time);
	  if (!(human->s[2].flags&SSF_INACTIVE))
	    FireOneGun(human,2,r,fuse_time);
	case 1:
	  FireOneGun(human,0,r,fuse_time);
	  break;
	default:
	  human->lasering=TRUE;
      }
  }
}

U0 HumanFireEnd()
{
  if (human && !human->exploding)
    switch (level) {
      case 3:
      case 2:
      case 1:
	break;
      default:
	human->lasering=FALSE;
	Snd(0);
    }
}

U0 ExpireShots()
{
  Bullet *tempb=bullet_root.next,*tempb1;
  while (tempb!=&bullet_root) {
    tempb1=tempb->next;
    if (tT>tempb->fuse_time && !tempb->exploding) {
      tempb->exploding=TRUE;
      Noise(50,3000,6000);
    }
    if (tT>tempb->die_timeout) {
      QueRem(tempb);
      QueRem(&tempb->p);
      Free(tempb);
    }
    tempb=tempb1;
  }
}

// ********************************** AI

U0 AI()
{
  CD3 p,p1,p2;
  Ship *temps=ship_root.next;
  if (human && !human->exploding) {
    while (temps!=&ship_root) {
      D3Sub(&temps->p[0].x,&temps->p[1].x,&p1);
      D3Sub(&temps->p[0].x,&temps->p[2].x,&p2);
      D3Add(&p1,&p2,&p);
      D3Sub(&human->p[0].x,&temps->p[0].x,&p1);
      if (D3Dot(D3Unit(&p),D3Unit(&p1))>0.995 &&
	tT>temps->reload_timeout) {
	FireOneGun(temps,0,1.5+.5,.4);
      }
      temps=temps->next;
    }
  }
}

// ********************************** Init


U0 InitLevel()
{
  I64 i;
  human=ShipNew(Fs->win_pixel_width/2,Fs->win_pixel_height/2,ST_HUMAN1);
  for (i=0;i<level+2;i++)
    PlaceShip(ST_ENEMY1);
  PlaceShip(ST_ENEMY2);
  show_level_msg=TRUE;
  ode->flags|=ODEF_PAUSED;
}

U0 Init()
{
  I64 i;
  game_over=FALSE;
  score=0;
  level=1;

  ship_root.next=ship_root.last=&ship_root;
  bullet_root.next=bullet_root.last=&bullet_root;

  for (i=0;i<NUM_STARS;i++) {
    stars_x[i]=RandU16%GR_WIDTH;
    stars_y[i]=RandU16%GR_HEIGHT;
  }

  human_t_left=0;
  human_t_right=0;
  human_antispin=0;

  InitLevel;
}


// ********************************** Main

U0 XCaliber()
{
  I64 ch,msg_code,p1,p2,sc;
  CCtrl *cp=CtrlPanelNew;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  Preempt(OFF);
  MenuPush(
  "File {"
  "  Abort(,CH_SHIFT_ESC);"
  "  Exit(,CH_ESC);"
  "}"
  "Play {"
  "  Restart(,'\n');"
  "  Fire(,CH_SPACE);"
  "  Thrust(,,SC_CURSOR_UP);"
  "  StopSpin(,,SC_CURSOR_DOWN);"
  "  Left(,,SC_CURSOR_LEFT);"
  "  Right(,,SC_CURSOR_RIGHT);"
  "  Spackwalk(,'w');"
  "  LongerFuse(,,SC_CURSOR_RIGHT|SCF_SHIFT);"
  "  ShorterFuse(,,SC_CURSOR_LEFT|SCF_SHIFT);"
  "  LargerShot(,,SC_CURSOR_UP|SCF_SHIFT);"
  "  SmallerShot(,,SC_CURSOR_DOWN|SCF_SHIFT);"
      "}"
      );
  WinMax;
  WordStat;
  WinBorder(ON);
  Fs->win_inhibit=WIF_ALL-WIF_BORDER-WIF_MENU-WIF_CTRLS;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  do {
    ode=ODENew(0,0.01,ODEF_HAS_MASSES);
    ode->derivative=&MyDerivative;
    ode->min_tolerance=1e-9;
    ode->drag_v3=0.00001;
    Init;
    QueIns(ode,Fs->last_ode);
    ch=0;
    do {
      while (!game_over && !show_level_msg &&
	  (msg_code=ScanMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP))) {
	switch (msg_code) {
	  case MSG_KEY_DOWN:
	    ch=p1; sc=p2;
	    switch (ch) {
	      case 0:
		switch (sc.u8[0]) {
		  case SC_CURSOR_RIGHT:
		    if (sc&SCF_SHIFT)
		      ctrl_panel.fuse_time+=2;
		    else
		      human_t_right=MAX_THRUST;
		    break;
		  case SC_CURSOR_LEFT:
		    if (sc&SCF_SHIFT)
		      ctrl_panel.fuse_time-=2;
		    else
		      human_t_left =MAX_THRUST;
		    break;
		  case SC_CURSOR_UP:
		    if (sc&SCF_SHIFT)
		      ctrl_panel.bullet_radius+=2;
		    else {
		      human_t_right=MAX_THRUST;
		      human_t_left =MAX_THRUST;
		    }
		    break;
		  case SC_CURSOR_DOWN:
		    if (sc&SCF_SHIFT)
		      ctrl_panel.bullet_radius-=2;
		    else
		      human_antispin=MAX_ANTISPIN;
		    break;
		}
		break;
	      case CH_SPACE:
		HumanFireBegin;
		break;
	      case 'w':
		ctrl_panel.spacewalk=TRUE;
		break;
	    }
	    break;
	  case MSG_KEY_UP:
	    ch=p1; sc=p2;
	    switch (ch) {
	      case 0:
		switch (sc.u8[0]) {
		  case SC_CURSOR_RIGHT:
		    human_t_right=0;
		    break;
		  case SC_CURSOR_LEFT:
		    human_t_left =0;
		    break;
		  case SC_CURSOR_UP:
		    human_t_right=0;
		    human_t_left =0;
		    break;
		  case SC_CURSOR_DOWN:
		    human_antispin=0;
		    break;
		}
		break;
	      case '\n':
		ch=0;
		break;
	      case CH_SPACE:
		HumanFireEnd;
		break;
	    }
	    break;
	}
      }
      AI;
      ExpireShots;
      CheckDamage;
      WinMgrSync; //msgs are only qued by winmngr
      if (show_level_msg) {
	ch=GetKey(&sc);
	if (ch=='\n')
	  ch=0;
	ode->flags&=~ODEF_PAUSED;
	show_level_msg=FALSE;
      } else if (game_over) {
	ch=ScanChar;
      } else {
	if (!remaining) {
	  level++;
	  ShipDel(human);
	  human=NULL;
	  InitLevel;
	}
      }
    } while (ch!=CH_ESC && ch!='\n' && ch!=CH_SHIFT_ESC);
    AllDel(ode);
  } while (ch!=CH_ESC && ch!=CH_SHIFT_ESC);
  SettingsPop;
  CtrlPanelDel(cp);
  MenuPop;
  AcctRegWriteBranch("TempleOS/XCaliber","I64 best_score=%d;\n",best_score);
}

           ž          ÿÿÿÿ   ÿÿÿÿ                            ÿÿÿÿ   ýÿÿÿ   ÿÿÿÿ                        ÿÿÿÿ         þÿÿÿ   þÿÿÿ                       ‚      	      øÿÿÿýÿÿÿÿÿÿÿÿÿ			ÿÿÿÿÿÿÿÿÿÿ			
	ÿÿÿÿÿÿÿÿ		



	ÿÿÿÿÿÿÿ			



	ÿÿÿÿÿÿÿÿÿÿ			
	ÿÿÿÿÿÿÿÿÿÿÿÿÿ			ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ	ÿÿÿÿÿÿÿ           ‚      	      øÿÿÿýÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ           À      ýÿÿÿüÿÿÿýÿÿÿ   þÿÿÿ   ÿÿÿÿüÿÿÿþÿÿÿ   ýÿÿÿ   þÿÿÿýÿÿÿ    ÿÿÿÿ    ÿÿÿÿ   ýÿÿÿ    ýÿÿÿ              ÿÿÿÿ   ÿÿÿÿ   ýÿÿÿ       üÿÿÿ    ûÿÿÿ              ÿÿÿÿüÿÿÿ    úÿÿÿ           â             ýÿÿÿ         ÷ÿÿÿ    ÿÿÿÿ
   ùÿÿÿ
      
   ýÿÿÿþÿÿÿÿÿÿÿÿÿÿÿýÿÿÿ   ÷ÿÿÿÿÿÿÿþÿÿÿ         úÿÿÿÿÿÿÿ               õÿÿÿ   õÿÿÿúÿÿÿ          þÿÿÿ
       
          ùÿÿÿ   ùÿÿÿ   þÿÿÿ           –      	   úÿÿÿûÿÿÿ    ÿÿÿÿÿÿÿÿ    ÿÿÿÿ       úÿÿÿ          úÿÿÿüÿÿÿ    þÿÿÿ       úÿÿÿ       ýÿÿÿ   ùÿÿÿ   ùÿÿÿ   ùÿÿÿÿÿÿÿùÿÿÿÿÿÿÿýÿÿÿÿÿÿÿ 