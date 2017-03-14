#define  XMSGF_ANTISPIN		0
#define  XMSGF_SOLAR_STORM	1
RegDft("TempleOS/XCaliber",
	"I64 best_score=0;\n"
	"I64 msg_flags=0;\n"
	);
RegExe("TempleOS/XCaliber");

#define MT_HUMAN_SHIP		0
#define MT_ENEMY_SHIP		1
#define MT_SOLAR_FLARE		2
#define MT_ION			3
#define MT_ANTIMATTER_BALL	4
#define MT_ANTIMATTER_SPLAT	5
#define MT_MISSILE		6

class MyMass:CMass
{
  F64 temperature,radius,die_timeout;
  I64 type;
  Bool no_overlap;
};

class MySpring:CSpring
{
  F64 strength;
  I64 color;
};

#define SPIN_GAIN		0.25
#define MASSES_NUM		8
#define SPRINGS_NUM		16
#define MISSILES_NUM		2
#define ST_HUMAN1		0
#define ST_ENEMY1		1
#define ST_ENEMY2		2
extern class Ship;

#define MISSILE_LEN		5
class Missile
{
  Missile *next,*last;
  F64 tons,fuse_time,die_timeout;
  MyMass p_front,p_back;
  MySpring s[5];
  U8 *img;
  Ship *owner,*target;
  Bool active,launched,exploding;
  U8 label[5];
} missile_head;

class Ship
{
  Ship *next,*last;
  I64 type,masses,springs;
  MyMass   p[MASSES_NUM];
  MySpring s[SPRINGS_NUM];
  F64 fire_rate;
  F64 reload_timeout,spacewalk_timeout;
  F64 die_time,die_timeout;
  I64 spacewalk_side;
  F64 laser_temperature;
  Missile missiles[MISSILES_NUM];
  Bool lasering,exploding,laser_overheat;
} ship_head,*human;

F64 human_t_left,human_t_right,human_antispin;

class Shot
{
  Shot *next,*last;
  F64 radius,fuse_time;
  I64 splats;
  MyMass p;
} shot_head;

F64 t_solar_storm;
Bool alarm;

#define THRUST_MAX	200.0
#define ANTISPIN_MAX	25.0
#define SPACEWALK_TIME	7.5

#define CMD_NULL	0
#define CMD_SPIN_LEFT	1
#define CMD_SPIN_RIGHT	2
#define CMD_THRUST	3
#define CMD_FIRE	4
#define CMD_EXIT	5
Bool game_over,show_level_msg;

#define STARS_NUM	100
I64 stars_x[STARS_NUM],stars_y[STARS_NUM];

CMathODE *ode=NULL;
I64 level,score,remaining;

$BG,0$
$SP,"<1>",BI=1$

$SP,"<2>",BI=2$

$SP,"<3>",BI=3$

$SP,"<4>",BI=4$


$SP,"<5>",BI=5$

$SP,"<6>",BI=6$

$SP,"<7>",BI=7$

$SP,"<8>",BI=8$

$BG$
//********************************** Ship
Bool CheckOverlap()
{
  CD3 p;
  MyMass *tmpm,*tmpm1;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    tmpm1=ode->next_mass;
    while (tmpm1!=&ode->next_mass) {
      if (tmpm!=tmpm1 && !tmpm->no_overlap && !tmpm1->no_overlap) {
	D3Sub(&p,&tmpm->x,&tmpm1->x);
	if (D3NormSqr(&p)<=Sqr(tmpm->radius+tmpm1->radius))
	  return TRUE;
      }
      tmpm1=tmpm1->next;
    }
    tmpm=tmpm->next;
  }
  return FALSE;
}

U0 MissileNew(Ship *tmpsp,I64 n)
{
  I64 i;
  CD3 p,p1,p2;
  Missile *tmpmi=&tmpsp->missiles[n];
  MemSet(tmpmi,0,sizeof(Missile));

  D3Equ(&tmpmi->p_front.x,
	(tmpsp->p[n+1].x+tmpsp->p[n+3].x)/2,
	(tmpsp->p[n+1].y+tmpsp->p[n+3].y)/2,0);
  D3Copy(&tmpmi->p_back.x,&tmpmi->p_front.x);

  if (n&1)
    StrCpy(tmpmi->label,"L");
  else
    StrCpy(tmpmi->label,"R");
  tmpmi->owner=tmpsp;
  tmpmi->tons=0.5;
  tmpmi->p_front.mass=0.1;
  tmpmi->p_front.type=MT_MISSILE;
  tmpmi->p_back.mass =0.1;
  tmpmi->p_back.type =MT_MISSILE;
  tmpmi->p_front.radius=2;
  tmpmi->p_back.radius =2;
  tmpmi->p_front.no_overlap=TRUE;
  tmpmi->p_back.no_overlap =TRUE;
  D3Sub(&p1,&tmpsp->p[0].x,&tmpsp->p[1].x);
  D3Sub(&p2,&tmpsp->p[0].x,&tmpsp->p[2].x);
  D3Unit(D3Add(&p,&p1,&p2));
  D3AddEqu(&tmpmi->p_front.x,D3MulEqu(D3Copy(&p1,&p),MISSILE_LEN/2+1));
  D3SubEqu(&tmpmi->p_back.x ,D3MulEqu(D3Copy(&p1,&p),MISSILE_LEN/2-1));
  D3Copy(&tmpmi->p_front.DxDt,&tmpsp->p[n].DxDt);
  D3Copy(&tmpmi->p_back.DxDt,&tmpsp->p[n].DxDt);
  QueIns(&tmpmi->p_front,ode->last_mass);
  QueIns(&tmpmi->p_back, ode->last_mass);

  tmpmi->s[0].end1=&tmpmi->p_front;
  tmpmi->s[0].end2=&tmpmi->p_back;
  tmpmi->s[1].end1=&tmpmi->p_front;
  tmpmi->s[1].end2=&tmpsp->p[n+1];
  tmpmi->s[2].end1=&tmpmi->p_back;
  tmpmi->s[2].end2=&tmpsp->p[n+1];
  tmpmi->s[3].end1=&tmpmi->p_front;
  tmpmi->s[3].end2=&tmpsp->p[n+3];
  tmpmi->s[4].end1=&tmpmi->p_back;
  tmpmi->s[4].end2=&tmpsp->p[n+3];
 
  for (i=0;i<5;i++) {
    tmpmi->s[i].const=10000;
    tmpmi->s[i].strength  =20000;
    tmpmi->s[i].color=BLACK;
    tmpmi->s[i].rest_len=D3Dist(&tmpmi->s[i].end1->x,&tmpmi->s[i].end2->x);
    QueIns(&tmpmi->s[i],ode->last_spring);
  }
  tmpmi->img=$IB,"<7>",BI=7$;
  tmpmi->active=TRUE;
  QueIns(tmpmi,missile_head.last);
}

Ship *ShipNew(I64 x,I64 y,I64 type)
{
  I64 i;
  Ship *tmpsp=CAlloc(sizeof(Ship));

  switch (tmpsp->type=type) {
    case ST_HUMAN1:
      tmpsp->fire_rate=25;
      tmpsp->masses=5;
      tmpsp->p[0].x=x;
      tmpsp->p[0].y=y;
      tmpsp->p[1].x=x+3;
      tmpsp->p[1].y=y+10;
      tmpsp->p[2].x=x-3;
      tmpsp->p[2].y=y+10;
      tmpsp->p[3].x=x+20;
      tmpsp->p[3].y=y+20;
      tmpsp->p[4].x=x-20;
      tmpsp->p[4].y=y+20;

      for (i=0;i<tmpsp->masses;i++) {
	tmpsp->p[i].mass=1;
	tmpsp->p[i].type=MT_HUMAN_SHIP;
	if (i<3)
	  tmpsp->p[i].radius=2.5;
	else
	  tmpsp->p[i].radius=4;
	tmpsp->p[i].drag_profile_factor=3;
	QueIns(&tmpsp->p[i],ode->last_mass);
      }
      tmpsp->p[3].mass/=10.0;
      tmpsp->p[4].mass/=10.0;

      tmpsp->springs=7;
      tmpsp->s[0].end1=&tmpsp->p[0];
      tmpsp->s[0].end2=&tmpsp->p[1];
      tmpsp->s[1].end1=&tmpsp->p[2];
      tmpsp->s[1].end2=&tmpsp->p[0];
      tmpsp->s[2].end1=&tmpsp->p[1];
      tmpsp->s[2].end2=&tmpsp->p[2];
      tmpsp->s[3].end1=&tmpsp->p[1];
      tmpsp->s[3].end2=&tmpsp->p[3];
      tmpsp->s[4].end1=&tmpsp->p[0];
      tmpsp->s[4].end2=&tmpsp->p[3];
      tmpsp->s[5].end1=&tmpsp->p[2];
      tmpsp->s[5].end2=&tmpsp->p[4];
      tmpsp->s[6].end1=&tmpsp->p[0];
      tmpsp->s[6].end2=&tmpsp->p[4];

      for (i=0;i<tmpsp->springs;i++) {
	tmpsp->s[i].rest_len=
	      D3Dist(&tmpsp->s[i].end1->x,&tmpsp->s[i].end2->x);
	tmpsp->s[i].const=10000;
	tmpsp->s[i].strength  =30000;
	if (i<=2)
	  tmpsp->s[i].color=LTCYAN;
	else
	  tmpsp->s[i].color=LTGRAY;
	QueIns(&tmpsp->s[i],ode->last_spring);
      }
      MissileNew(tmpsp,0);
      MissileNew(tmpsp,1);
      remaining=0;

      break;
    case ST_ENEMY1:
      tmpsp->fire_rate=2.5;
      tmpsp->masses=3;
      tmpsp->p[0].x=x;
      tmpsp->p[0].y=y;
      tmpsp->p[1].x=x+15;
      tmpsp->p[1].y=y;
      tmpsp->p[2].x=x;
      tmpsp->p[2].y=y+15;

      for (i=0;i<tmpsp->masses;i++) {
	tmpsp->p[i].mass=1;
	tmpsp->p[i].type=MT_ENEMY_SHIP;
	tmpsp->p[i].radius=7;
	tmpsp->p[i].drag_profile_factor=3;
	QueIns(&tmpsp->p[i],ode->last_mass);
      }

      tmpsp->springs=3;
      tmpsp->s[0].end1=&tmpsp->p[0];
      tmpsp->s[0].end2=&tmpsp->p[1];
      tmpsp->s[1].end1=&tmpsp->p[1];
      tmpsp->s[1].end2=&tmpsp->p[2];
      tmpsp->s[2].end1=&tmpsp->p[2];
      tmpsp->s[2].end2=&tmpsp->p[0];

      for (i=0;i<tmpsp->springs;i++) {
	tmpsp->s[i].rest_len=
	      D3Dist(&tmpsp->s[i].end1->x,&tmpsp->s[i].end2->x);
	tmpsp->s[i].const=10000;
	tmpsp->s[i].strength  =20000;
	tmpsp->s[i].color=BLACK;
	QueIns(&tmpsp->s[i],ode->last_spring);
      }
      remaining++;
      break;
    case ST_ENEMY2:
      tmpsp->fire_rate=5.0;
      tmpsp->masses=5;
      tmpsp->p[0].x=x;
      tmpsp->p[0].y=y;
      tmpsp->p[1].x=x-7;
      tmpsp->p[1].y=y+10;
      tmpsp->p[2].x=x+7;
      tmpsp->p[2].y=y+10;
      tmpsp->p[3].x=x-14;
      tmpsp->p[3].y=y+20;
      tmpsp->p[4].x=x+14;
      tmpsp->p[4].y=y+20;

      for (i=0;i<tmpsp->masses;i++) {
	tmpsp->p[i].mass=1;
	tmpsp->p[i].type=MT_ENEMY_SHIP;
	tmpsp->p[i].radius=6;
	tmpsp->p[i].drag_profile_factor=5;
	QueIns(&tmpsp->p[i],ode->last_mass);
      }

      tmpsp->springs=7;
      tmpsp->s[0].end1=&tmpsp->p[0];
      tmpsp->s[0].end2=&tmpsp->p[1];
      tmpsp->s[1].end1=&tmpsp->p[0];
      tmpsp->s[1].end2=&tmpsp->p[2];
      tmpsp->s[2].end1=&tmpsp->p[1];
      tmpsp->s[2].end2=&tmpsp->p[2];
      tmpsp->s[3].end1=&tmpsp->p[1];
      tmpsp->s[3].end2=&tmpsp->p[3];
      tmpsp->s[4].end1=&tmpsp->p[2];
      tmpsp->s[4].end2=&tmpsp->p[4];
      tmpsp->s[5].end1=&tmpsp->p[2];
      tmpsp->s[5].end2=&tmpsp->p[3];
      tmpsp->s[6].end1=&tmpsp->p[1];
      tmpsp->s[6].end2=&tmpsp->p[4];

      for (i=0;i<tmpsp->springs;i++) {
	tmpsp->s[i].rest_len=
	      D3Dist(&tmpsp->s[i].end1->x,&tmpsp->s[i].end2->x);
	tmpsp->s[i].const= 40000;
	tmpsp->s[i].strength  =75000;
	if (i>=3)
	  tmpsp->s[i].color=LTPURPLE;
	else
	  tmpsp->s[i].color=BLACK;
	QueIns(&tmpsp->s[i],ode->last_spring);
      }
      remaining++;
      break;
  }
  QueIns(tmpsp,ship_head.last);
  return tmpsp;
}

U0 MissileDel(Missile *tmpmi)
{
  I64 i;
  if (tmpmi->active) {
    QueRem(tmpmi);
    for(i=0;i<5;i++)
      QueRem(&tmpmi->s[i]);
    QueRem(&tmpmi->p_front);
    QueRem(&tmpmi->p_back);
    tmpmi->active=FALSE;
  }
}

U0 ShipDel(Ship *tmpsp)
{
  I64 i;
  if (!tmpsp) return;
  for (i=0;i<tmpsp->masses;i++)
    QueRem(&tmpsp->p[i]);
  for (i=0;i<tmpsp->springs;i++)
    QueRem(&tmpsp->s[i]);
  for (i=0;i<2;i++)
    MissileDel(&tmpsp->missiles[i]);
  QueRem(tmpsp);
  Free(tmpsp);
  remaining--;
}

U0 PlaceShip(I64 type)
{
  Ship *tmpsp;
  if (CheckOverlap)
    return;
  while (TRUE) {
    tmpsp=ShipNew(RandU16%(Fs->pix_width-20)+10,
	  RandU16%(Fs->pix_height-20)+10,type);
    if (CheckOverlap)
      ShipDel(tmpsp);
    else
      break;
  }
}

//********************************** Human Ship

I64 Tweaked()
{
  CD3 p,p1,p2;
  if (human) {
    D3Sub(&p1,&human->p[0].x,&human->p[1].x);
    D3Sub(&p2,&human->p[0].x,&human->p[2].x);
    D3Unit(D3Add(&p,&p1,&p2));
    D3Sub(&p1,&human->p[0].x,&human->p[3].x);
    D3Sub(&p2,&human->p[0].x,&human->p[4].x);
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
  Ship *tmpsp,*tmpsp1;
  QueRem(ode);
  tmpsp=ship_head.next;
  while (tmpsp!=&ship_head) {
    tmpsp1=tmpsp->next;
    ShipDel(tmpsp);
    tmpsp=tmpsp1;
  }
  human=NULL;
  QueDel(&shot_head,TRUE);
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

//**********************************
U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j;
  F64 arg;
  Ship *tmpsp;
  Shot *tmps;
  Missile *tmpmi;
  CD3 p,p1,p2;
  F64 t_left,t_right,spin,d,x,y;
  MySpring *tmpsps;
  MyMass *tmpm;
  U8 *img;
  Bool draw_laser_line=FALSE;

  if (ode!=task->last_ode) return;

  dc->color=WHITE;
  GrPrint(dc,0,0,"Level:%d Score:%d High Score:%d",level,score,best_score);
  if (game_over) {
    if (Blink)
      GrPrint(dc,(task->pix_width-9*FONT_WIDTH)/2,
	    (task->pix_height-FONT_HEIGHT)/2,"Game Over");
  } else if (show_level_msg) {
    if (Blink)
      GrPrint(dc,(task->pix_width-8*FONT_WIDTH)/2,
	    (task->pix_height-FONT_HEIGHT)/2+50,"Level %d",level);
  }

  for (i=0;i<STARS_NUM;i++)
    GrPlot(dc,stars_x[i],stars_y[i]);

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    if (tmpm->type==MT_ANTIMATTER_SPLAT) {
      dc->color=LTGREEN;
      GrPlot(dc,tmpm->x,tmpm->y);
    } else if (tmpm->type==MT_ION) {
      dc->color=YELLOW;
      GrPlot(dc,tmpm->x,tmpm->y);
    }
    tmpm=tmpm->next;
  }

  tmpsps=ode->next_spring;
  while (tmpsps!=&ode->next_spring) {
    if (!(tmpsps->flags&SSF_INACTIVE) && tmpsps->color) {
      dc->color=tmpsps->color;
      GrLine(dc,tmpsps->end1->x,tmpsps->end1->y,
	    tmpsps->end2->x,tmpsps->end2->y);
    }
    tmpsps=tmpsps->next;
  }

  tmpmi=missile_head.next;
  while (tmpmi!=&missile_head) {
    if (tmpmi->active) {
      if (tmpmi->launched && tmpmi->exploding) {
	d=(tS-tmpmi->fuse_time)/(tmpmi->die_timeout-tmpmi->fuse_time);
	d=70*Sin(ã*d)*tmpmi->tons+1;
	for (i=1;i<d;i++) {
	  if (i&1)
	    dc->color=YELLOW;
	  else
	    dc->color=LTRED;
	  GrCircle(dc,tmpmi->p_front.x,tmpmi->p_front.y,i);
	}
      } else
	Sprite3ZB(dc,(tmpmi->p_front.x+tmpmi->p_back.x)/2,
	      (tmpmi->p_front.y+tmpmi->p_back.y)/2,0,tmpmi->img,
	      Arg(tmpmi->p_front.x-tmpmi->p_back.x,
	      tmpmi->p_front.y-tmpmi->p_back.y));
    }
    tmpmi=tmpmi->next;
  }

  tmpsp=ship_head.next;
  while (tmpsp!=&ship_head) {
    if (!tmpsp->exploding) {
      switch (tmpsp->type) {
	case ST_HUMAN1:
	  if (tmpsp->spacewalk_side) {
	    t_left=0;
	    t_right=0;
	  } else {
	    if (d=D3Norm(D3Sub(&p1,&tmpsp->p[0].x,&tmpsp->p[1].x))) {
	      D3Sub(&p2,&tmpsp->p[0].DxDt,&tmpsp->p[1].DxDt);
	      D3Cross(&p,&p1,&p2);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_left =Clamp(human_t_left+SPIN_GAIN*spin*human_antispin,
		  0,THRUST_MAX);

	    if (d=D3Norm(D3Sub(&p1,&tmpsp->p[0].x,&tmpsp->p[2].x))) {
	      D3Sub(&p2,&tmpsp->p[0].DxDt,&tmpsp->p[2].DxDt);
	      D3Cross(&p,&p1,&p2);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_right=Clamp(human_t_right-SPIN_GAIN*spin*human_antispin,
		  0,THRUST_MAX);
	  }

	  D3Sub(&p1,&tmpsp->p[1].x,&tmpsp->p[0].x);
	  D3Sub(&p2,&tmpsp->p[2].x,&tmpsp->p[0].x);
	  D3Unit(D3Add(&p,&p1,&p2));

	  if (!(tmpsp->s[3].flags&SSF_INACTIVE)) {
	    dc->color=YELLOW;
	    D3AddEqu(D3Mul(&p1,t_left/25,&p),&tmpsp->p[3].x);
	    GrLine(dc,tmpsp->p[1].x,tmpsp->p[1].y,p1.x,p1.y);
	    arg=Arg(p.x,p.y);
	    Sprite3ZB(dc,tmpsp->p[3].x,tmpsp->p[3].y,0,$IB,"<thruster>",BI=1$,arg);
	  }

	  if (!(tmpsp->s[5].flags&SSF_INACTIVE)) {
	    dc->color=YELLOW;
	    D3AddEqu(D3Mul(&p2,t_right/25,&p),&tmpsp->p[4].x);
	    GrLine(dc,tmpsp->p[2].x,tmpsp->p[2].y,p2.x,p2.y);
	    arg=Arg(p.x,p.y);
	    Sprite3ZB(dc,tmpsp->p[4].x,tmpsp->p[4].y,0,$IB,"<thruster>",BI=1$,arg);
	  }

	  if (tS>tmpsp->reload_timeout)
	    img=$IB,"<gun_ready>",BI=2$;
	  else
	    img=$IB,"<gun_busy>",BI=3$;
	  arg=Arg(p.x,p.y);
	  switch (level) {
	    case 3:
	      if (!(tmpsp->s[3].flags&SSF_INACTIVE))
		Sprite3ZB(dc,tmpsp->p[3].x,tmpsp->p[3].y,0,img,arg);
	      if (!(tmpsp->s[5].flags&SSF_INACTIVE))
		Sprite3ZB(dc,tmpsp->p[4].x,tmpsp->p[4].y,0,img,arg);
	    case 2:
	      if (!(tmpsp->s[1].flags&SSF_INACTIVE))
		Sprite3ZB(dc,tmpsp->p[1].x,tmpsp->p[1].y,0,img,arg);
	      if (!(tmpsp->s[2].flags&SSF_INACTIVE))
		Sprite3ZB(dc,tmpsp->p[2].x,tmpsp->p[2].y,0,img,arg);
	    case 1:
	      Sprite3ZB(dc,tmpsp->p[0].x,tmpsp->p[0].y,0,img,arg);
	      break;
	    default:
	      Sprite3ZB(dc,tmpsp->p[0].x,tmpsp->p[0].y,0,$IB,"<Laser>",BI=6$,arg);
	      if (tmpsp->lasering && !tmpsp->laser_overheat) {
		draw_laser_line=TRUE;
		Snd(74);
	      }
	  }

	  ctrl_panel.laser_temperature=tmpsp->laser_temperature;

	  if (tmpsp->spacewalk_side) {
	    d=1.0-(tmpsp->spacewalk_timeout-tS)/SPACEWALK_TIME;
	    if (d>1.0) {
	      tmpsp->spacewalk_side=0;
	      ctrl_panel.spacewalk=FALSE;
	    } else {
	      if (d<0.5) {
		d=d*2;
		x=tmpsp->p[0].x*(1.0-d)+
		      tmpsp->p[tmpsp->spacewalk_side].x*(d);
		y=tmpsp->p[0].y*(1.0-d)+
		      tmpsp->p[tmpsp->spacewalk_side].y*(d);
	      } else {
		d=(d-0.5)*2;
		x=tmpsp->p[tmpsp->spacewalk_side].x*(1.0-d)+
		      tmpsp->p[0].x*(d);
		y=tmpsp->p[tmpsp->spacewalk_side].y*(1.0-d)+
		      tmpsp->p[0].y*(d);
	      }
	      Sprite3ZB(dc,x,y,0,$IB,"<spacewalk>",BI=4$,arg+0.75*Sin(tS*2));
	    }
	  } else {
	    if (ctrl_panel.spacewalk) {
	      if (tmpsp->spacewalk_side=Tweaked)
		tmpsp->spacewalk_timeout=tS+SPACEWALK_TIME;
	      else
		ctrl_panel.spacewalk=FALSE;
	    }
	  }
	  break;
	case ST_ENEMY2:
	  for (i=3;i<tmpsp->masses;i++) {
	    dc->color=PURPLE;
	    GrCircle(dc,tmpsp->p[i].x,tmpsp->p[i].y,tmpsp->p[i].radius);
	    GrFloodFill(dc,tmpsp->p[i].x,tmpsp->p[i].y+2,TRUE);
	    dc->color=WHITE;
	    GrCircle(dc,tmpsp->p[i].x,tmpsp->p[i].y,tmpsp->p[i].radius);
	  }
	case ST_ENEMY1:
	  D3DivEqu(D3Sub(&p1,&tmpsp->p[1].x,&tmpsp->p[0].x),2.0);
	  D3DivEqu(D3Sub(&p2,&tmpsp->p[2].x,&tmpsp->p[0].x),2.0);
	  D3Unit(D3Add(&p,&p1,&p2));
	  if (tS>tmpsp->reload_timeout)
	    img=$IB,"<gun_ready>",BI=2$;
	  else
	    img=$IB,"<gun_busy>",BI=3$;
	  arg=Arg(p.x,p.y);
	  Sprite3ZB(dc,tmpsp->p[0].x,tmpsp->p[0].y,0,img,arg);
	  arg=Arg(p1.x,p1.y);
	  Sprite3ZB(dc,tmpsp->p[0].x+p1.x,tmpsp->p[0].y+p1.y,0,
		$IB,"<EnemySide>",BI=5$,arg);
	  arg=Arg(p2.x,p2.y);
	  Sprite3ZB(dc,tmpsp->p[0].x+p2.x,tmpsp->p[0].y+p2.y,0,
		$IB,"<EnemySide>",BI=5$,arg);
	  break;
      }
      for (i=0;i<tmpsp->masses;i++) {
	dc->color=YELLOW;
	if (tmpsp->p[i].temperature>=1.0)
	  GrCircle(dc,tmpsp->p[i].x,tmpsp->p[i].y,
		tmpsp->p[i].temperature);
      }
    }
    else if (tmpsp->die_time<=tS<=tmpsp->die_timeout)
      for (j=0;j<tmpsp->masses;j++) {
	d=(tS-tmpsp->die_time)/(tmpsp->die_timeout-tmpsp->die_time);
	d=7*Sin(ã*d)*(6+j)+1;
	for (i=1;i<d;i++) {
	  if (i&1)
	    dc->color=YELLOW;
	  else
	    dc->color=LTRED;
	  GrCircle(dc,tmpsp->p[j].x,tmpsp->p[j].y,i);
	}
      }
    tmpsp=tmpsp->next;
  }

  tmps=shot_head.next;
  while (tmps!=&shot_head) {
    if (tmps->radius<1.0) {
      dc->color=LTGREEN;
      GrPlot(dc,tmps->p.x,tmps->p.y);
    } else {
      dc->color=YELLOW;
      GrCircle(dc,tmps->p.x,tmps->p.y,tmps->radius);
      if (tmps->radius>=2.0)
	GrFloodFill(dc,tmps->p.x,tmps->p.y,TRUE);
      dc->color=LTGREEN;
      GrCircle(dc,tmps->p.x,tmps->p.y,tmps->radius);
    }
    tmps=tmps->next;
  }

  if (human && draw_laser_line) {
    D3Sub(&p1,&human->p[1].x,&human->p[0].x);
    D3Sub(&p2,&human->p[2].x,&human->p[0].x);
    D3Unit(D3Add(&p,&p1,&p2));
    dc->color=LTBLUE;
    Line(dc,human->p[0].x-10*p.x,human->p[0].y-10*p.y,0,
	  human->p[0].x-800*p.x,human->p[0].y-800*p.y,0,&LaserPlot);
  }

  tmpmi=missile_head.next;
  while (tmpmi!=&missile_head) {
    if (tmpsp=tmpmi->target) {
      dc->color=LTRED;
      GrCircle(dc,tmpsp->p[0].x,tmpsp->p[0].y,10);
      GrPrint(dc,tmpsp->p[0].x+12,tmpsp->p[0].y-4,tmpmi->label);
    }
    tmpmi=tmpmi->next;
  }
}

U0 Explosion(MyMass *tmpm1,MyMass *tmpm2,F64 tons)
{
  MyMass *tmpm;
  CD3 p1;
  F64 d;

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    if (tmpm!=tmpm1 && tmpm!=tmpm2) {
      D3Sub(&p1,&tmpm->state->x,&tmpm1->state->x);
      d=D3NormSqr(&p1)-tmpm->radius*tmpm->radius;
      if (d<100.0*100.0) {
	if (d<1)
	  d=1;
	else
	  d=Sqrt(d);
	d=250000*tons/d`2;
	D3MulEqu(&p1,d);
	D3AddEqu(&tmpm->DstateDt->DxDt,&p1);
      }
    }
    tmpm=tmpm->next;
  }
}

Ship TargetGet(Missile *tmpmi)
{
  Ship *tmpsp,*res=NULL;
  F64 dd,best_dd=F64_MAX;
  I64 i;
  CD3 p,p1,p2;
  D3Unit(D3Sub(&p,&tmpmi->p_front.state->x,&tmpmi->p_back.state->x));
  tmpsp=ship_head.next;
  while (tmpsp!=&ship_head) {
    if (!tmpsp->exploding && tmpsp!=tmpmi->owner)
      for (i=0;i<tmpsp->masses;i++) {
	D3Sub(&p1,&tmpsp->p[i].state->x,&tmpmi->p_front.state->x);
	D3Unit(D3Copy(&p2,&p1));
	D3Cross(&p1,&p,&p2);
	if (D3Dot(&p,&p2)>0 && D3Norm(&p1)<=ã/16) {
	  dd=D3NormSqr(&p1);
	  if (dd<best_dd) {
	    best_dd=dd;
	    res=tmpsp;
	  }
	}
      }
    tmpsp=tmpsp->next;
  }
  return res;
}

U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{
  I64 i;
  F64 d,dd,dd2,spin,t_left,t_right,é_err,é_thrust,DéDt;
  CTask *task=ode->win_task;
  CD3 p,p1,p2;
  Ship *tmpsp;
  Missile *tmpmi;
  MyMass *tmpm,*tmpm1;

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    if (tmpm->type!=MT_SOLAR_FLARE && tmpm->type!=MT_ION) {
      d=tmpm->state->x;
      if (d-tmpm->radius<0)
	tmpm->DstateDt->DxDt+=Sqr(Sqr(Sqr(d-tmpm->radius)));
      if (d+tmpm->radius>task->pix_width)
	tmpm->DstateDt->DxDt-=Sqr(Sqr(Sqr((d+tmpm->radius)-task->pix_width)));
      d=tmpm->state->y;
      if (d-tmpm->radius<0)
	tmpm->DstateDt->DyDt+=Sqr(Sqr(Sqr(d-tmpm->radius)));
      if (d+tmpm->radius>task->pix_height)
	tmpm->DstateDt->DyDt-=Sqr(Sqr(Sqr((d+tmpm->radius)-task->pix_height)));
    }
    if (tmpm->type!=MT_ION && tmpm->type!=MT_ANTIMATTER_SPLAT) {
      tmpm1=ode->next_mass;
      while (tmpm1!=&ode->next_mass) {
	if (tmpm!=tmpm1) {
	  if (tmpm1->type==MT_ANTIMATTER_SPLAT) {
	    if (tmpm->type==MT_HUMAN_SHIP || tmpm->type==MT_ENEMY_SHIP) {
	      D3Sub(&p,&tmpm->state->x,&tmpm1->state->x);
	      dd=D3NormSqr(&p)+1;
	      if (dd<100000) {
		D3MulEqu(&p,100000/dd);
		D3AddEqu(&tmpm1->DstateDt->DxDt,&p);
	      }
	    }
	  } else if (tmpm1->type!=MT_ION) {
	    D3Sub(&p,&tmpm->state->x,&tmpm1->state->x);
	    dd=D3NormSqr(&p);
	    dd2=Sqr(tmpm->radius+tmpm1->radius);
	    if (dd<=dd2) {
	      d=Sqrt(dd)+0.0001;
	      D3MulEqu(&p,Sqr(Sqr(dd2-dd))/d);
	      D3AddEqu(&tmpm ->DstateDt->DxDt,&p);
	      D3SubEqu(&tmpm1->DstateDt->DxDt,&p);
	    }
	  }
	}
	tmpm1=tmpm1->next;
      }
    }
    tmpm=tmpm->next;
  }

  tmpsp=ship_head.next;
  while (tmpsp!=&ship_head) {
    if (tmpsp->exploding && tmpsp->die_time<=tS<=tmpsp->die_timeout)
      for (i=0;i<tmpsp->masses;i++)
	Explosion(&tmpsp->p[i],NULL,tmpsp->p[i].radius/250.0);
    switch (tmpsp->type) {
      case ST_HUMAN1:
	if (!tmpsp->exploding) {
	  if (tmpsp->spacewalk_side) {
	    t_left=0;
	    t_right=0;
	    d=1.0-(tmpsp->spacewalk_timeout-tS)/SPACEWALK_TIME;
	    if (0.485<d<0.515) {
	      D3Unit(D3Sub(&p,&tmpsp->p[2].state->x,&tmpsp->p[1].state->x));
	      if (tmpsp->spacewalk_side==3) {
		tmpsp->p[3].DstateDt->DxDt-=10*THRUST_MAX*p.x;
		tmpsp->p[3].DstateDt->DyDt-=10*THRUST_MAX*p.y;
		tmpsp->p[1].DstateDt->DxDt+=10*THRUST_MAX*p.x;
		tmpsp->p[1].DstateDt->DyDt+=10*THRUST_MAX*p.y;
	      } else {
		tmpsp->p[4].DstateDt->DxDt+=10*THRUST_MAX*p.x;
		tmpsp->p[4].DstateDt->DyDt+=10*THRUST_MAX*p.y;
		tmpsp->p[2].DstateDt->DxDt-=10*THRUST_MAX*p.x;
		tmpsp->p[2].DstateDt->DyDt-=10*THRUST_MAX*p.y;
	      }
	    }
	  } else {
	    if (d=D3Norm(D3Sub(&p1,&tmpsp->p[0].state->x,
		  &tmpsp->p[1].state->x))) {
	      D3Sub(&p2,&tmpsp->p[0].state->DxDt,&tmpsp->p[1].state->DxDt);
	      D3Cross(&p,&p1,&p2);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_left =Clamp(human_t_left+SPIN_GAIN*spin*human_antispin,
		  0,THRUST_MAX);

	    if (d=D3Norm(D3Sub(&p1,&tmpsp->p[0].state->x,
		  &tmpsp->p[2].state->x))) {
	      D3Sub(&p2,&tmpsp->p[0].state->DxDt,&tmpsp->p[2].state->DxDt);
	      D3Cross(&p,&p1,&p2);
	      spin=p.z/d;
	    } else
	      spin=0;
	    t_right=Clamp(human_t_right-SPIN_GAIN*spin*human_antispin,
		  0,THRUST_MAX);

	    D3Sub(&p1,&tmpsp->p[0].state->x,&tmpsp->p[1].state->x);
	    D3Sub(&p2,&tmpsp->p[0].state->x,&tmpsp->p[2].state->x);
	    D3Unit(D3Add(&p,&p1,&p2));
	    if (!(tmpsp->s[3].flags&SSF_INACTIVE)) {
	      D3Mul(&p1,t_left,&p);
	      D3AddEqu(&tmpsp->p[3].DstateDt->DxDt,&p1);
	    }
	    if (!(tmpsp->s[5].flags&SSF_INACTIVE)) {
	      D3Mul(&p2,t_right,&p);
	      D3AddEqu(&tmpsp->p[4].DstateDt->DxDt,&p2);
	    }
	  }
	}
	break;
    }
    tmpsp=tmpsp->next;
  }

  tmpmi=missile_head.next;
  while (tmpmi!=&missile_head) {
    if (tmpmi->active) {
      if (tmpmi->launched) {
	if (tmpmi->exploding)
	  Explosion(&tmpmi->p_front,&tmpmi->p_back,tmpmi->tons);
	else {
//Guide missile
	  if (tmpsp=tmpmi->target) {
	    D3Unit(D3Sub(&p,&tmpmi->p_front.state->x,
		  &tmpmi->p_back.state->x));
	    D3Sub(&p1,&tmpsp->p[0].state->x,&tmpmi->p_front.state->x);
	    d=D3Norm(&p1);
	    D3Unit(&p1);
	    é_err=D3Dot(&p,&p1);
	    D3Sub(&p1,&tmpmi->p_front.state->DxDt,&tmpmi->p_back.state->DxDt);
	    D3Cross(&p2,&p,&p1);
	    DéDt=D3Norm(&p2);
	    if (p2.z<0)
	      DéDt=-DéDt;
	    é_thrust=Clamp(200*(é_err+2*DéDt)/(d+200),-ã/8,ã/8);
	    p2.x=p.x*Cos(é_thrust)-p.y*Sin(é_thrust);
	    p2.y=p.y*Cos(é_thrust)+p.x*Sin(é_thrust);
	    p2.z=0;
	    D3AddEqu(&tmpmi->p_back.DstateDt->DxDt,D3MulEqu(&p2,THRUST_MAX));
	  }
	}
      } else
	tmpmi->target=TargetGet(tmpmi);
    } else
      tmpmi->target=NULL;
    tmpmi=tmpmi->next;
  }
}

U0 CheckDamage()
{
  I64 i,j,death_score;
  Ship *tmpsp,*tmpsp1;
  MyMass *tmpm,*tmpm1,*best_mass;
  CD3 p,p1,p2;
  F64 d,best_distance;
  Bool facing_sun=FALSE;

  if (human) {
    D3Sub(&p1,&human->p[1].x,&human->p[0].x);
    D3Sub(&p2,&human->p[2].x,&human->p[0].x);
    D3Add(&p,&p1,&p2);
    if (p.x>0)
      facing_sun=TRUE;
  }

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    if (tmpm->type==MT_ION) {
      if (facing_sun) {
	tmpm1=ode->next_mass;
	while (tmpm1!=&ode->next_mass) {
	  if (tmpm1->type==MT_HUMAN_SHIP) {
	    D3Sub(&p,&tmpm1->x,&tmpm->x);
	    if (D3NormSqr(&p)<Sqr(tmpm1->radius))
	      tmpm1->temperature+=3.0;
	  }
	  tmpm1=tmpm1->next;
	}
      }
    } else if (tmpm->type==MT_ANTIMATTER_SPLAT) {
      tmpm1=ode->next_mass;
      while (tmpm1!=&ode->next_mass) {
	if (tmpm1->type!=MT_ION && tmpm1->type!=MT_ANTIMATTER_SPLAT) {
	  D3Sub(&p,&tmpm1->x,&tmpm->x);
	  if (D3NormSqr(&p)<Sqr(tmpm1->radius))
	    tmpm1->temperature+=0.4;
	}
	tmpm1=tmpm1->next;
      }
    } else
      tmpm->temperature*=0.9;
    tmpm=tmpm->next;
  }

  if (human) {
    human->laser_temperature*=0.975;

    if (human->laser_overheat) {
      if (human->laser_temperature<LASER_THRESHOLD_TEMP)
	human->laser_overheat=FALSE;
    }
    if (!human->laser_overheat && human->lasering) {
      if (human->laser_temperature>=LASER_TEMP_MAX) {
	human->laser_overheat=TRUE;
	Snd;
      } else {
	human->laser_temperature+=1.0;
	D3Sub(&p1,&human->p[0].x,&human->p[1].x);
	D3Sub(&p2,&human->p[0].x,&human->p[2].x);
	D3Unit(D3Add(&p,&p1,&p2));
	p2.x=p.y;
	p2.y=-p.x;
	p2.z=0;
	best_mass=NULL;
	best_distance=F64_MAX;
	tmpm=ode->next_mass;
	while (tmpm!=&ode->next_mass) {
	  D3Sub(&p1,&human->p[0].x,&tmpm->x);
	  if (Abs(D3Dot(&p1,&p2))<tmpm->radius &&
		D3Dot(&p1,&p)<0.0) {
	    d=D3NormSqr(&p1);
	    if (d<best_distance) {
	      best_distance=d;
	      best_mass=tmpm;
	    }
	  }
	  tmpm=tmpm->next;
	}
	if (best_mass)
	  best_mass->temperature+=1.0;
      }
    }
  }

  tmpsp=ship_head.next;
  while (tmpsp!=&ship_head) {
    tmpsp1=tmpsp->next;
    death_score=0;
    switch (tmpsp->type) {
      case ST_HUMAN1:
	if (tmpsp->exploding) {
	  if (tS>tmpsp->die_timeout)  {
	    ShipDel(tmpsp);
	    human=NULL;
	  }
	} else
	  for (i=0;i<tmpsp->springs;i++) {
	    if (Abs(tmpsp->s[i].f)>tmpsp->s[i].strength) {
	      tmpsp->s[i].flags|=SSF_INACTIVE;
	      if (i==4)
		MissileDel(&tmpsp->missiles[0]);
	      else if (i==5)
		MissileDel(&tmpsp->missiles[1]);
	    }
	    if (tmpsp->s[i].flags&SSF_INACTIVE && i<3)
	      death_score++;
	  }
	break;
      default:
	if (tmpsp->exploding) {
	  if (tS>tmpsp->die_timeout) {
	    ShipDel(tmpsp);
	    score+=level;
	    if (score>best_score)
	      best_score=score;
	  }
	} else {
	  j=0;
	  for (i=0;i<tmpsp->springs;i++) {
	    if (tmpsp->s[i].flags&SSF_INACTIVE)
	      j++;
	    else if (Abs(tmpsp->s[i].f)>tmpsp->s[i].strength) {
	      tmpsp->s[i].flags|=SSF_INACTIVE;
	      j++;
	    }
	  }
	  if (j>1)
	    death_score++;
	}
    }
    if (!tmpsp->exploding) {
      for (i=0;i<tmpsp->masses;i++)
	if (tmpsp->p[i].temperature>MASS_TEMP_MAX)
	  death_score++;
      if (death_score) {
	tmpsp->exploding=TRUE;
	tmpsp->die_time=tS;
	tmpsp->die_timeout=tS+0.75;
	Noise(750,74,93);
	if (tmpsp->type==ST_HUMAN1)
	  game_over=TRUE;
      }
    }
    tmpsp=tmpsp1;
  }
}

//********************************** Shots

Shot *ShotNew(I64 type,CD3 *_p,CD3 *_v,F64 r,F64 fuse_time,
	CD3 *_p_gun_offset=NULL)
{
  Shot *tmps=CAlloc(sizeof(Shot));
  D3Copy(&tmps->p.x,_p);
  tmps->radius=r;
  tmps->splats=20*r;
  tmps->fuse_time=tS+fuse_time;
  tmps->p.mass=0.3*r*r*r;
  tmps->p.type=type;
  if (_p_gun_offset)
    D3AddEqu(&tmps->p.x,_p_gun_offset);
  D3Copy(&tmps->p.DxDt,_v);
  QueIns(&tmps->p,ode->last_mass);
  QueIns(tmps,shot_head.last);
}

U0 SolarFlares()
{
  CD3 p,v,p1,p2;
  CTask *task=ode->win_task;
  if (!alarm && t_solar_storm-2.0<tS<t_solar_storm+1.0) {
    Sweep(2000,74,93);
    alarm=TRUE;
  }
  if (t_solar_storm<tS) {  //If solar storm has arrived
    if (tS<t_solar_storm+5.0) { //If solar storm not over
      if (Rand<.1) {
	D3Equ(&p,-300,Rand*task->pix_height,0);
	D3Equ(&v,200.0,0,0);
	ShotNew(MT_SOLAR_FLARE,&p,&v,25,0.1);
      }
    } else {
      t_solar_storm=tS+25*Rand;  //Schedule next solar storm
      alarm=FALSE;
    }
  }
}

U0 FireOneGun(Ship *tmpsp,I64 n,F64 r,F64 fuse_time)
{
  I64 ona;
  CD3 p,v,p1,p2;
  Shot *tmps;
  D3Sub(&p1,&tmpsp->p[0].x,&tmpsp->p[1].x);
  D3Sub(&p2,&tmpsp->p[0].x,&tmpsp->p[2].x);
  D3Unit(D3Add(&p,&p1,&p2));
  D3MulEqu(D3Copy(&p1,&p),r+tmpsp->p[0].radius+5);
  D3AddEqu(D3MulEqu(D3Copy(&v,&p),1000/(r+1)),&tmpsp->p[n].DxDt);
  tmps=ShotNew(MT_ANTIMATTER_BALL,&tmpsp->p[n].x,&v,r,fuse_time,&p1);
  D3MulEqu(&p,tmps->p.mass/tmpsp->p[n].mass/100.0);
  D3SubEqu(&tmpsp->p[n].DxDt,&p);
  tmpsp->reload_timeout=tS+r/tmpsp->fire_rate;
  ona=Freq2Ona(500/r);
  Noise(100,ona,ona+12);
}

U0 FireOneMissile(Ship *tmpsp,I64 n)
{
  I64 i;
  Missile *tmpmi=&tmpsp->missiles[n];
  if (!tmpmi->launched && tmpmi->target) {
    tmpmi->fuse_time=tS+1.0;
    tmpmi->die_timeout=tmpmi->fuse_time+0.125;
    tmpmi->img=$IB,"<8>",BI=8$;
    for (i=1;i<5;i++)
      tmpmi->s[i].flags|=SSF_INACTIVE;
    tmpmi->launched=TRUE;
    Sweep(250,53,56);
  }
}

U0 HumanFireGunBegin()
{
  F64 r=3.0*ctrl_panel.shot_radius/CTRL_PANEL_RANGE+0.5,
	fuse_time=ToF64(ctrl_panel.fuse_time+1)/CTRL_PANEL_RANGE;
  if (human) {
    if (!human->exploding && !human->spacewalk_side && tS>human->reload_timeout)
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
      }
  }
}

U0 HumanFireMissileBegin(I64 n)
{
  if (human && !human->exploding &&
	!human->spacewalk_side && tS>human->reload_timeout)
    FireOneMissile(human,n);
}

U0 HumanFireLaserBegin()
{
  if (human && !human->exploding &&
	!human->spacewalk_side && tS>human->reload_timeout)
    human->lasering=TRUE;
}
U0 HumanFireLaserEnd()
{
  if (human && !human->exploding) {
    human->lasering=FALSE;
    Snd;
  }
}

U0 SplatNew(Shot *tmps,F64 die_time,F64 start,F64 end)
{
  MyMass *tmpm;
  F64 é=Arg(tmps->p.DxDt,tmps->p.DyDt);
  I64 i;
  for (i=0;i<tmps->splats;i++) {
    tmpm=CAlloc(sizeof(MyMass));
    D3Copy(&tmpm->x,&tmps->p.x);
    tmpm->radius=1;
    tmpm->mass=1;
    tmpm->die_timeout=tS+die_time;
    if (tmps->p.type==MT_SOLAR_FLARE)
      tmpm->type=MT_ION;
    else
      tmpm->type=MT_ANTIMATTER_SPLAT;
    D3Copy(&tmpm->DxDt,&tmps->p.DxDt);
    tmpm->DxDt+=50*Sqr(tmps->radius)*Rand*
	  Sin(start+é+(end-start)*i/tmps->splats);
    tmpm->DyDt+=50*Sqr(tmps->radius)*Rand*
	  Cos(start+é+(end-start)*i/tmps->splats);
    QueIns(tmpm,ode->last_mass);
  }
}

U0 ExpireShots()
{
  Shot *tmps=shot_head.next,*tmps1;
  while (tmps!=&shot_head) {
    tmps1=tmps->next;
    if (tS>tmps->fuse_time) {
      if (tmps->p.type==MT_SOLAR_FLARE)
	SplatNew(tmps,1.0,3*ã/8,5*ã/8);
      else
	SplatNew(tmps,.2,0,2*ã);
      QueRem(tmps);
      QueRem(&tmps->p);
      Free(tmps);
    }
    tmps=tmps1;
  }
}

U0 ExpireSplats()
{
  MyMass *tmpm,*tmpm1;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    tmpm1=tmpm->next;
    if ((tmpm->type==MT_ION || tmpm->type==MT_ANTIMATTER_SPLAT) &&
	  tS>tmpm->die_timeout) {
      QueRem(tmpm);
      Free(tmpm);
    }
    tmpm=tmpm1;
  }
}

U0 ExpireMissiles()
{
  I64 i;
  F64 dd,best_dd;
  Missile *tmpmi=missile_head.next,*tmpm1;
  while (tmpmi!=&missile_head) {
    tmpm1=tmpmi->next;
    if (tmpmi->launched) {
      best_dd=F64_MAX;
      if (tmpmi->target)
	for (i=0;i<tmpmi->target->masses;i++) {
	  dd=D3DistSqr(&tmpmi->p_front.x,&tmpmi->target->p[i].x);
	  if (dd<best_dd)
	    best_dd=dd;
	}
      if (!tmpmi->exploding && (best_dd<30*30 || tS>tmpmi->fuse_time)) {
	tmpmi->p_front.mass=10.0; //They go flying, if too light.
	tmpmi->p_back.mass =10.0;
	tmpmi->exploding=TRUE;
	Noise(50,93,105);
      } else if (tS>tmpmi->die_timeout)
	MissileDel(tmpmi);
    }
    tmpmi=tmpm1;
  }
}

//********************************** AI

U0 AI()
{
  CD3 p,p1,p2;
  Ship *tmpsp=ship_head.next;
  if (human && !human->exploding) {
    while (tmpsp!=&ship_head) {
      D3Sub(&p1,&tmpsp->p[0].x,&tmpsp->p[1].x);
      D3Sub(&p2,&tmpsp->p[0].x,&tmpsp->p[2].x);
      D3Add(&p,&p1,&p2);
      D3Sub(&p1,&human->p[0].x,&tmpsp->p[0].x);
      if (D3Dot(D3Unit(&p),D3Unit(&p1))>0.995 &&
	    tS>tmpsp->reload_timeout) {
	FireOneGun(tmpsp,0,1.5+.5,.4);
      }
      tmpsp=tmpsp->next;
    }
  }
}

//********************************** Init
U0 InitLevel()
{
  I64 i;
  MyMass *tmpm,*tmpm1;

  t_solar_storm=0;

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    tmpm1=tmpm->next;
    if (tmpm->type==MT_ION || tmpm->type==MT_ANTIMATTER_SPLAT) {
      QueRem(tmpm);
      Free(tmpm);
    }
    tmpm=tmpm1;
  }
  if (level==1)
    OneTimePopUp(&msg_flags,XMSGF_SOLAR_STORM,
	  "Face away from Sun in solar storm.\n");
  if (level==4)
    OneTimePopUp(&msg_flags,XMSGF_ANTISPIN,
	  "Press $$GREEN$$<CURSOR-DOWN>$$FG$$ for anti-spin stabilizer.\n");
  human=ShipNew(Fs->pix_width/2,Fs->pix_height/2,ST_HUMAN1);
  for (i=0;i<level+2;i++)
    PlaceShip(ST_ENEMY1);
  PlaceShip(ST_ENEMY2);
  show_level_msg=TRUE;
  ODEPause(ode);
}

U0 Init()
{
  I64 i;
  game_over=FALSE;
  score=0;
  level=1;

  QueInit(&ship_head);
  QueInit(&shot_head);
  QueInit(&missile_head);

  for (i=0;i<STARS_NUM;i++) {
    stars_x[i]=RandU16%GR_WIDTH;
    stars_y[i]=RandU16%GR_HEIGHT;
  }

  human_t_left=0;
  human_t_right=0;
  human_antispin=0;

  InitLevel;
}

//********************************** Main
U0 XCaliber()
{
  I64 ch,msg_code,arg1,arg2,sc;
  CCtrl *cp=CtrlPanelNew;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLACK<<4+WHITE;
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Game {"
	"  Restart(,'\n');"
	"  LevelUp(,'+');"
	"  LevelDown(,'-');"
	"}"
	"Play {"
	"  Fire(,CH_SPACE);"
	"  Thrust(,,SC_CURSOR_UP);"
	"  StopSpin(,,SC_CURSOR_DOWN);"
	"  Left(,,SC_CURSOR_LEFT);"
	"  Right(,,SC_CURSOR_RIGHT);"
	"  LeftMissile(,,SC_CURSOR_LEFT|SCF_CTRL);"
	"  RightMissile(,,SC_CURSOR_RIGHT|SCF_CTRL);"
	"  Spackwalk(,'w');"
	"  LongerFuse(,,SC_CURSOR_RIGHT|SCF_SHIFT);"
	"  ShorterFuse(,,SC_CURSOR_LEFT|SCF_SHIFT);"
	"  LargerShot(,,SC_CURSOR_UP|SCF_SHIFT);"
	"  SmallerShot(,,SC_CURSOR_DOWN|SCF_SHIFT);"
	"}"
	);
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  Fs->win_inhibit=WIG_TASK_DFT-WIF_SELF_FOCUS
	-WIF_SELF_BORDER-WIF_FOCUS_TASK_MENU-WIF_SELF_CTRLS;
  Fs->draw_it=&DrawIt;
  do {
    ode=ODENew(0,0.01,ODEF_HAS_MASSES);
    ode->derive=&MyDerivative;
    ode->min_tolerance=1e-9;
    ode->drag_v3=0.00001;
    Init;
    QueIns(ode,Fs->last_ode);
    ch=0;
    do {
      while (!game_over && !show_level_msg &&
	    (msg_code=ScanMsg(&arg1,&arg2,1<<MSG_KEY_DOWN|1<<MSG_KEY_UP))) {
	switch (msg_code) {
	  case MSG_KEY_DOWN:
	    ch=arg1; sc=arg2;
	    switch (ch) {
	      case 0:
		switch (sc.u8[0]) {
		  case SC_CURSOR_RIGHT:
		    if (sc&SCF_CTRL)
		      HumanFireMissileBegin(0);
		    else if (sc&SCF_SHIFT)
		      ctrl_panel.fuse_time+=2;
		    else
		      human_t_right=THRUST_MAX;
		    break;
		  case SC_CURSOR_LEFT:
		    if (sc&SCF_CTRL)
		      HumanFireMissileBegin(1);
		    else if (sc&SCF_SHIFT)
		      ctrl_panel.fuse_time-=2;
		    else
		      human_t_left =THRUST_MAX;
		    break;
		  case SC_CURSOR_UP:
		    if (sc&SCF_SHIFT)
		      ctrl_panel.shot_radius+=2;
		    else {
		      human_t_right=THRUST_MAX;
		      human_t_left =THRUST_MAX;
		    }
		    break;
		  case SC_CURSOR_DOWN:
		    if (sc&SCF_SHIFT)
		      ctrl_panel.shot_radius-=2;
		    else
		      human_antispin=ANTISPIN_MAX;
		    break;
		}
		break;
	      case CH_SPACE:
		if (level<4)
		  HumanFireGunBegin;
		else
		  HumanFireLaserBegin;
		break;
	      case 'w':
		ctrl_panel.spacewalk=TRUE;
		break;
	      case '+':
		level++;
		break;
	      case '-':
		level--;
		break;
	    }
	    break;
	  case MSG_KEY_UP:
	    ch=arg1; sc=arg2;
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
		if (level>=4)
		  HumanFireLaserEnd;
		break;
	    }
	    break;
	}
      }
      AI;
      SolarFlares;
      ExpireShots;
      ExpireSplats;
      ExpireMissiles;
      CheckDamage;
      Refresh; //msgs are only qued by winmgr
      if (show_level_msg) {
	ch=GetKey(&sc);
	if (ch=='\n')
	  ch=0;
	ODEPause(ode,OFF);
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
  RegWrite("TempleOS/XCaliber",
	"I64 best_score=%d;\n"
	"I64 msg_flags=%d;\n",best_score,msg_flags);
}
        ž      
    ÿÿÿÿ   ÿÿÿÿ
             
            
   ÿÿÿÿ   ýÿÿÿ
   ÿÿÿÿ      
               
   ÿÿÿÿ      
   þÿÿÿ   þÿÿÿ
                    ‚      øÿÿÿýÿÿÿ	      ÿÿÿÿÿÿ			ÿÿÿÿÿÿÿÿÿÿ			
	ÿÿÿÿÿÿÿÿ		



	ÿÿÿÿÿÿÿ			



	ÿÿÿÿÿÿÿÿÿÿ			
	ÿÿÿÿÿÿÿÿÿÿÿÿÿ			ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ	ÿÿÿÿÿÿÿ        ‚      øÿÿÿýÿÿÿ	      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        À      
ýÿÿÿüÿÿÿýÿÿÿ   
þÿÿÿ   ÿÿÿÿüÿÿÿ
þÿÿÿ   ýÿÿÿ   
þÿÿÿýÿÿÿ    ÿÿÿÿ
    ÿÿÿÿ   ýÿÿÿ
    ýÿÿÿ       
       ÿÿÿÿ   
ÿÿÿÿ   ýÿÿÿ   
    üÿÿÿ    ûÿÿÿ
              
ÿÿÿÿüÿÿÿ    úÿÿÿ        â         
    ýÿÿÿ      
   ÷ÿÿÿ    ÿÿÿÿ

   ùÿÿÿ
      

   ýÿÿÿþÿÿÿÿÿÿÿ
ÿÿÿÿýÿÿÿ   ÷ÿÿÿ
ÿÿÿÿþÿÿÿ         
úÿÿÿÿÿÿÿ      
         õÿÿÿ
   õÿÿÿúÿÿÿ       
   þÿÿÿ
       

          ùÿÿÿ
   ùÿÿÿ   þÿÿÿ        –      	   úÿÿÿûÿÿÿ    ÿÿÿÿÿÿÿÿ    ÿÿÿÿ       úÿÿÿ          úÿÿÿüÿÿÿ    þÿÿÿ       úÿÿÿ       
ýÿÿÿ   ùÿÿÿ   
ùÿÿÿ   ùÿÿÿÿÿÿÿ
ùÿÿÿÿÿÿÿýÿÿÿÿÿÿÿ        b      üÿÿÿþÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ        b      ÷ÿÿÿþÿÿÿ      ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ 