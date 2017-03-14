#define GAME_SPEED_SCALE	0.1

F64 game_speed,
    launch_unit_x1,launch_unit_y1,launch_unit_x2,launch_unit_y2,launch_t,
    return_unit_x1,return_unit_y1,return_unit_x2,return_unit_y2,return_t,
    set_é_unit_x1,set_é_unit_y1,set_é_unit_x2,set_é_unit_y2,set_é_t,
    next_noise;
CTask *main_task;

#define OT_CARRIER	0
#define OT_CRUISER	1
#define OT_FIGHTER	2
#define OT_TYPES_NUM	3

#define OF_SHIP		1
#define OF_ACTIVE	2
#define OF_RETURNING	4
#define OF_SHOOTING	8

#define FIRE_WIDTH	56
#define FIRE_HEIGHT	16
#define FIRE_X_SCALE	0.5
#define FIRE_Y_SCALE	0.5

#define FIRE_COLORS	16
U8 fire_colors[FIRE_COLORS]={
YELLOW,YELLOW,LTRED,YELLOW, BLACK,YELLOW,RED,YELLOW,
YELLOW,BLACK,YELLOW,LTRED, RED,YELLOW,DKGRAY,YELLOW};

class Obj
{
  Obj *next,*last;
  Obj *next_in_squadron,*last_in_squadron;
  Obj *host;
  U8  player,type;
  U16 flags;
  I16 squadron,member_num;
  F64 x,y,é,dé,
	speed,turn_rate,
	life_percent,
	target_x,target_y,
	ship_guns,ship_guns_range,
	air_guns,air_guns_range,
	fuel,max_fuel,fuel_burn_rate,
	death_time,next_action_time;
  I32 torpedos,max_torpedos;
  F64 torpedos_range;
  U8  fire[(FIRE_WIDTH*FIRE_HEIGHT+7)/8];
} obj_head;

class Torpedo
{
  Torpedo *next,*last;
  Obj *target;
  F64 x,y,é,speed,timeout;
} torpedo_head;

#define SA_PARKED	0
#define SA_LAUNCHING	1
#define SA_FLYING	2
#define SA_SET_é	3
#define SA_RETURNING	4
#define SA_DEAD		5

class Squadron : Obj
{
  I64 action,dead_mask,total_mask;
} *squadrons;

U0 SquadronIns(Obj *o,Obj *pred)
{
  Obj *succ=pred->next_in_squadron;
  o->next_in_squadron=succ;
  o->last_in_squadron=pred;
  pred->next_in_squadron=o;
  succ->last_in_squadron=o;
}

U0 SquadronRem(Obj *o)
{
  Obj *pred=o->last_in_squadron,*succ=o->next_in_squadron;
  pred->next_in_squadron=succ;
  succ->last_in_squadron=pred;
}

#define PLAYERS_NUM	2

I64 num_carriers[PLAYERS_NUM]={2,3},
    num_cruisers[PLAYERS_NUM]={2,3},
    num_planes_per_squadron[PLAYERS_NUM]={6,5},
    num_squadrons_per_carrier[PLAYERS_NUM]={2,3},
    num_alive[PLAYERS_NUM],
    num_squadrons;

Obj *ObjFind(F64 x,F64 y,
	     I64 flag_mask=OF_ACTIVE|OF_RETURNING,I64 flag_val=OF_ACTIVE,
	     I64 type_mask=-1,I64 player_mask=-1,F64 *_d=NULL)
{
  Obj *tmpo=obj_head.next,*best=NULL;
  F64 dd,best_dd=F64_MAX;
  while (tmpo!=&obj_head) {
    if (tmpo->flags&flag_mask==flag_val &&
	  Bt(&type_mask,tmpo->type)&&Bt(&player_mask,tmpo->player)) {
      dd=Sqr(tmpo->x-x)+Sqr(tmpo->y-y);
      if (dd<best_dd) {
	best=tmpo;
	best_dd=dd;
      }
    }
    tmpo=tmpo->next;
  }
  if (_d) *_d=Sqrt(best_dd);
  return best;
}

U0 ObjDel(Obj *tmpo)
{
  if (tmpo) {
    if (tS<tmpo->death_time)
      tmpo->flags&=~OF_ACTIVE;
    else {
      if (tmpo->squadron>=0)
	SquadronRem(tmpo);
      QueRem(tmpo);
      if (tmpo->squadron>=0)
	LBts(&squadrons[tmpo->squadron].dead_mask,tmpo->member_num);
      num_alive[tmpo->player]--;
      Free(tmpo);
    }
  }
}
$BG,1$


$SP,"<1>",BI=1$




$SP,"<2>",BI=2$


$SP,"<3>",BI=3$



$SP,"<4>",BI=4$




$SP,"<5>",BI=5$


$SP,"<6>",BI=6$
$BG$
U8 *imgs[PLAYERS_NUM][OT_TYPES_NUM]={{$IB,"<1>",BI=1$,$IB,"<2>",BI=2$,$IB,"<3>",BI=3$},{$IB,"<4>",BI=4$,$IB,"<5>",BI=5$,$IB,"<6>",BI=6$}};

U0 DrawIt(CTask *task,CDC *dc)
{
  I64 i,j,k;
  F64 d,cur_time=tS;
  Obj *tmpo;
  Torpedo *tmpt;

  tmpt=torpedo_head.next;
  while (tmpt!=&torpedo_head) {
    dc->color=WHITE;
    GrPlot(dc,tmpt->x,tmpt->y);
    tmpt=tmpt->next;
  }

  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    if (tmpo->flags&OF_ACTIVE && tmpo->flags&OF_SHIP) {
      Sprite3ZB(dc,tmpo->x,tmpo->y,0,
	    imgs[tmpo->player][tmpo->type],tmpo->é+ã/2);

      k=0;
      for (j=0;j<FIRE_HEIGHT;j++)
	for (i=0;i<FIRE_WIDTH;i++)
	  if (Bt(tmpo->fire,k++)) {
	    dc->color=fire_colors[ToI64(k+10*tS)&(FIRE_COLORS-1)];
	    GrPlot(dc,
		  tmpo->x+FIRE_X_SCALE*(i-FIRE_WIDTH /2+(11*tS+i)%1.7)
	    *Cos(tmpo->é)-
		  FIRE_Y_SCALE*(j-FIRE_HEIGHT/2+(12*tS+j)%1.7)*Sin(tmpo->é),
		  tmpo->y+FIRE_Y_SCALE*(j-FIRE_HEIGHT/2+(19*tS+j)%1.7)
	    *Cos(tmpo->é)+
		  FIRE_X_SCALE*(i-FIRE_WIDTH /2+(13*tS+i)%1.7)*Sin(tmpo->é));
	  }
      if (Blink) {
	dc->color=BLACK;
	GrLine(dc,tmpo->x+5,tmpo->y,tmpo->x+5+10,tmpo->y);

	if (tmpo->life_percent>0) {
	  if (tmpo->life_percent<33)
	    dc->color=RED;
	  else if (tmpo->life_percent<66)
	    dc->color=YELLOW;
	  else
	    dc->color=GREEN;
	  GrLine(dc,tmpo->x+5,tmpo->y,
		tmpo->x+5+10*tmpo->life_percent/100,tmpo->y);
	}

	dc->color=BLACK;
	GrLine(dc,tmpo->x+5,tmpo->y+2,tmpo->x+5+10,tmpo->y+2);
	d=tmpo->fuel*100/tmpo->max_fuel;
	if (d>0) {
	  if (d<33)
	    dc->color=RED;
	  else if (d<66)
	    dc->color=YELLOW;
	  else
	    dc->color=GREEN;
	  GrLine(dc,tmpo->x+5,tmpo->y+2,tmpo->x+5+10*d/100,tmpo->y+2);
	}
      }
    }
    tmpo=tmpo->next;
  }

  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    if (tmpo->flags&OF_ACTIVE && !(tmpo->flags&OF_SHIP))
      Sprite3ZB(dc,tmpo->x,tmpo->y,0,
	    imgs[tmpo->player][tmpo->type],tmpo->é+ã/2);
    if (tmpo->flags&OF_SHOOTING) {
      dc->color=LTRED;
      GrLine(dc,tmpo->x,tmpo->y,tmpo->target_x,tmpo->target_y,3);
    }
    tmpo=tmpo->next;
  }

  dc->flags|=DCF_TRANSFORMATION;
  if (cur_time<launch_t) {
    dc->color=LTGREEN;
    GrArrow3(dc,launch_unit_x1,launch_unit_y1,0,
	  launch_unit_x2,launch_unit_y2,0);
  }
  if (cur_time<return_t) {
    dc->color=LTRED;
    GrArrow3(dc,return_unit_x1,return_unit_y1,0,
	  return_unit_x2,return_unit_y2,0);
  }
  if (cur_time<set_é_t) {
    dc->color=YELLOW;
    GrArrow3(dc,set_é_unit_x1,set_é_unit_y1,0,
	  set_é_unit_x2,set_é_unit_y2,0);
  }

  dc->color=YELLOW;
  GrPrint(dc,-task->scroll_x,-task->scroll_y,
	"Game Speed: %5.2f",game_speed);
  dc->color=LTCYAN;
  GrPrint(dc,-task->scroll_x,-task->scroll_y+FONT_HEIGHT,
	"Player 1: %d",num_alive[0]);
  dc->color=LTPURPLE;
  GrPrint(dc,-task->scroll_x,-task->scroll_y+2*FONT_HEIGHT,
	"Player 2: %d",num_alive[1]);

  if ((!num_alive[0]||!num_alive[1]) && Blink) {
    if (!num_alive[1]) {
      dc->color=LTGREEN;
      GrPrint(dc,task->pix_width>>1 -(FONT_WIDTH*14)/2-task->scroll_x,
	    task->pix_height>>1-FONT_HEIGHT/2-task->scroll_y,"Game Completed");
    } else {
      dc->color=LTRED;
      GrPrint(dc,task->pix_width>>1 -(FONT_WIDTH*9)/2-task->scroll_x,
	    task->pix_height>>1-FONT_HEIGHT/2-task->scroll_y,"Game Over");
    }
  }
}

U0 Init()
{
  I64 i,fighter,ship,player,squadron,
	w=Fs->pix_width,h=Fs->pix_height;
  Squadron *tmps;
  Obj *tmpo,*tmpo1;
  QueInit(&obj_head);
  QueInit(&torpedo_head);

  next_noise=tS;
  Fs->scroll_x=0;
  Fs->scroll_y=0;
  game_speed=1.0;
  launch_t=return_t=set_é_t=0;
  main_task=Fs;
  num_squadrons=0;
  for (player=0;player<PLAYERS_NUM;player++) {
    num_alive[player]=0;
    for (ship=0;ship<num_cruisers[player];ship++) {
      tmpo1=CAlloc(sizeof(Obj));
      num_alive[player]++;
      tmpo1->type=OT_CRUISER;	tmpo1->player=player;
      tmpo1->squadron=-1;	tmpo1->member_num=ship;
      tmpo1->flags=OF_ACTIVE|OF_SHIP;
      tmpo1->x=0.8*w*(Rand-0.5)+w>>1; tmpo1->y=0.8*h*(Rand-0.5)+h>>1;
      tmpo1->host=NULL;
      tmpo1->speed=35.0;	tmpo1->turn_rate=2.5;
      tmpo1->é=2*ã*(Rand-0.5);	tmpo1->dé=0;
      tmpo1->life_percent=100.0;
      tmpo1->fuel=tmpo1->max_fuel=100000;
      tmpo1->fuel_burn_rate=100.0;
      tmpo1->air_guns =5000;	tmpo1->air_guns_range =30.0;
      tmpo1->ship_guns=10000;	tmpo1->ship_guns_range=30.0;
      tmpo1->torpedos =tmpo1->max_torpedos=0;
      tmpo1->torpedos_range =0.0;
      tmpo1->next_action_time=0;
      QueIns(tmpo1,obj_head.last);
    }

    for (ship=0;ship<num_carriers[player];ship++) {
      tmpo1=CAlloc(sizeof(Obj));
      num_alive[player]++;
      tmpo1->type=OT_CARRIER;	tmpo1->player=player;
      tmpo1->squadron=-1;	tmpo1->member_num=ship;
      tmpo1->flags=OF_ACTIVE|OF_SHIP;
      tmpo1->x=0.8*w*(Rand-0.5)+w>>1; tmpo1->y=0.8*h*(Rand-0.5)+h>>1;
      tmpo1->host=NULL;
      tmpo1->speed=28.0;	tmpo1->turn_rate=1.0;
      tmpo1->é=2*ã*(Rand-0.5);	tmpo1->dé=0;
      tmpo1->life_percent=100.0;
      tmpo1->fuel=tmpo1->max_fuel=750000;
      tmpo1->fuel_burn_rate=500.0;
      tmpo1->air_guns =5000;	tmpo1->air_guns_range =20.0;
      tmpo1->ship_guns=2000;	tmpo1->ship_guns_range=30.0;
      tmpo1->torpedos =tmpo1->max_torpedos=0;
      tmpo1->torpedos_range =0.0;
      tmpo1->next_action_time=0;
      QueIns(tmpo1,obj_head.last);

      for (squadron=0;squadron<num_squadrons_per_carrier[player];
	    squadron++,num_squadrons++) {
	for (fighter=0;fighter<num_planes_per_squadron[player];fighter++) {
	  tmpo=CAlloc(sizeof(Obj));
	  num_alive[player]++;
	  tmpo->type=OT_FIGHTER;	 tmpo->player=player;
	  tmpo->squadron=num_squadrons; tmpo->member_num=fighter;
	  tmpo->flags=0;
	  tmpo->host=tmpo1;
	  tmpo->speed=300.0;		tmpo->turn_rate=25.0;
	  tmpo->life_percent=100.0;
	  tmpo->fuel=tmpo->max_fuel=1000;
	  tmpo->fuel_burn_rate=1.0;
	  tmpo->air_guns =35000;	tmpo->air_guns_range =8.0;
	  tmpo->ship_guns=0;		tmpo->ship_guns_range=0.0;
	  tmpo->torpedos =tmpo->max_torpedos=1;
	  tmpo->torpedos_range =20.0;
	  QueIns(tmpo,obj_head.last);
	}
      }
    }
  }
  squadrons=CAlloc(num_squadrons*sizeof(Squadron));
  for (i=0,tmps=squadrons;i<num_squadrons;i++,tmps++) {
    tmps->next_in_squadron=tmps->last_in_squadron=tmps;
    tmps->squadron=i;
  }
  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    if (tmpo->squadron>=0) {
      tmps=&squadrons[tmpo->squadron];
      tmps->host  =tmpo->host;
      tmps->player=tmpo->player;
      tmps->total_mask=1<<num_planes_per_squadron[tmpo->player]-1;
      SquadronIns(tmpo,tmps->last_in_squadron);
    }
    tmpo=tmpo->next;
  }
}

U0 CleanUp()
{
  QueDel(&obj_head,TRUE);
  QueDel(&torpedo_head,TRUE);
  Free(squadrons);
}

Obj *ObjLaunch(I64 player,I64 squadron=-1,
	Obj *host=NULL,F64 x=F64_MAX,F64 y=F64_MAX,F64 é=F64_MAX)
{
  Obj *tmpo;
  F64 cur_time=tS;
  if (!host)
    host=ObjFind(x,y,,,1<<OT_CARRIER,1<<player);
  if (host && cur_time>host->next_action_time) {
    if (é==F64_MAX)
      é=Arg(x-host->x,y-host->y);
    tmpo=obj_head.next;
    while (tmpo!=&obj_head) {
      if (tmpo->host==host && (squadron<0||tmpo->squadron==squadron) &&
	    !(tmpo->flags&OF_ACTIVE) &&
	    (tmpo->squadron<0||squadrons[tmpo->squadron].action==SA_PARKED||
	    squadrons[tmpo->squadron].action==SA_LAUNCHING)) {
	if (tmpo->fuel<=0.0)
//When low on fuel, not zero, gets launched and captured.
	  LBts(&squadrons[tmpo->squadron].dead_mask,tmpo->member_num);
	else {
	  tmpo->flags=tmpo->flags&~OF_RETURNING|OF_ACTIVE;
	  tmpo->é=host->é;
	  if (x==F64_MAX || y==F64_MAX || Sqr(x-host->x)+Sqr(y-host->y)>3*3)
	    tmpo->dé=Wrap(é-tmpo->é,-ã);
	  else
	    tmpo->dé=0;
	  tmpo->x=host->x;
	  tmpo->y=host->y;
	  host->next_action_time=cur_time+0.25/game_speed;
	  return tmpo;
	}
      }
      tmpo=tmpo->next;
    }
  }
  return NULL;
}

Squadron *SquadronLaunch(I64 player,F64 x=F64_MAX,F64 y=F64_MAX)
{
  Squadron *tmps;
  Obj *tmpo;
  if (tmpo=ObjLaunch(player,,,x,y)) {
    if (player==0) {
      launch_unit_x1=tmpo->x;
      launch_unit_y1=tmpo->y;
      launch_unit_x2=ms.pos.x-main_task->pix_left-main_task->scroll_x;
      launch_unit_y2=ms.pos.y-main_task->pix_top -main_task->scroll_y;
      launch_t=tS+0.5;
    }
    if (tmpo->squadron>=0) {
      tmps=&squadrons[tmpo->squadron];
      if (tmps->action==SA_PARKED) {
	tmps->action=SA_LAUNCHING;
	tmps->é=tmpo->é+tmpo->dé;
      }
      return tmps;
    }
  }
  return NULL;
}

Obj *ObjReturn(I64 player,F64 x,F64 y)
{
  Obj *tmpo;
  if (tmpo=ObjFind(x,y,OF_ACTIVE,OF_ACTIVE,1<<OT_FIGHTER,1<<player))
    tmpo->flags|=OF_RETURNING;
  return tmpo;
}

Squadron *SquadronReturn(I64 player,F64 x,F64 y)
{
  Squadron *tmps;
  Obj *tmpo;
  if (tmpo=ObjReturn(player,x,y)) {
    if (player==0) {
      return_unit_x1=tmpo->x;
      return_unit_y1=tmpo->y;
      if (tmpo->host) {
	return_unit_x2=tmpo->host->x;
	return_unit_y2=tmpo->host->y;
	return_t=tS+0.5;
      }
    }
    if (tmpo->squadron>=0) {
      tmps=&squadrons[tmpo->squadron];
      if (tmps->action==SA_FLYING)
	tmps->action=SA_RETURNING;
      return tmps;
    }
  }
  return NULL;
}

Obj *ObjSeté(I64 player,F64 x=F64_MAX,F64 y=F64_MAX,F64 é=F64_MAX)
{
  Obj *tmpo;
  if ((tmpo=ObjFind(x,y,,,,1<<player)) && tmpo->flags&OF_ACTIVE &&
	!(tmpo->flags&OF_RETURNING)) {
    if (é==F64_MAX)
      é=Arg(x-tmpo->x,y-tmpo->y);
    tmpo->dé+=Wrap(é-(tmpo->é+tmpo->dé),-ã);
    return tmpo;
  }
  return NULL;
}

Squadron *SquadronSeté(I64 player,F64 x=F64_MAX,F64 y=F64_MAX,F64 é=F64_MAX)
{
  Squadron *tmps;
  Obj *tmpo;
  if (tmpo=ObjSeté(player,x,y,é)) {
    if (player==0) {
      set_é_unit_x1=tmpo->x;
      set_é_unit_y1=tmpo->y;
      set_é_unit_x2=ms.pos.x-main_task->pix_left-main_task->scroll_x;
      set_é_unit_y2=ms.pos.y-main_task->pix_top -main_task->scroll_y;
      set_é_t=tS+0.5;
    }
    if (tmpo->squadron>=0) {
      tmps=&squadrons[tmpo->squadron];
      if (tmps->action==SA_FLYING) {
	tmps->action=SA_SET_é;
	tmps->é=tmpo->é+tmpo->dé;
      }
      return tmps;
    }
  }
  return NULL;
}

U0 SquadronActions()
{
  I64 i,completed_mask;
  Obj *tmpo;
  Squadron *tmps;
  for (i=0,tmps=squadrons;i<num_squadrons;i++,tmps++) {
    completed_mask=0;
    switch (tmps->action) {
      case SA_LAUNCHING:
	ObjLaunch(tmps->player,i,tmps->host,,,tmps->é);
	tmpo=tmps->next_in_squadron;
	while (tmpo!=tmps) {
	  LBEqu(&completed_mask,tmpo->member_num,tmpo->flags&OF_ACTIVE);
	  tmpo=tmpo->next_in_squadron;
	}
	if (completed_mask|tmps->dead_mask==tmps->total_mask)
	  tmps->action=SA_FLYING;
	break;
      case SA_FLYING:
	tmpo=tmps->next_in_squadron;
	while (tmpo!=tmps) {
	  LBEqu(&completed_mask,tmpo->member_num,!(tmpo->flags&OF_ACTIVE));
	  tmpo=tmpo->next_in_squadron;
	}
	if (completed_mask|tmps->dead_mask==tmps->total_mask)
	  tmps->action=SA_PARKED;
	break;
      case SA_SET_é:
	tmpo=tmps->next_in_squadron;
	while (tmpo!=tmps) {
	  tmpo->dé+=Wrap(tmps->é-(tmpo->é+tmpo->dé),-ã);
	  tmpo=tmpo->next_in_squadron;
	}
	tmps->action=SA_FLYING;
	break;
      case SA_RETURNING:
	tmpo=tmps->next_in_squadron;
	while (tmpo!=tmps) {
	  tmpo->flags|=OF_RETURNING;
	  LBEqu(&completed_mask,tmpo->member_num,!(tmpo->flags&OF_ACTIVE));
	  tmpo=tmpo->next_in_squadron;
	}
	if (completed_mask|tmps->dead_mask==tmps->total_mask)
	  tmps->action=SA_PARKED;
	break;
    }
    if (tmps->dead_mask==tmps->total_mask)
      tmps->action=SA_DEAD;
  }
}

U0 AI(I64 player,F64 period)
{
  Obj *tmpo;
  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    if (tmpo->player==player) {
      if (tmpo->type==OT_CARRIER && Rand<5*period)
	SquadronLaunch(player,tmpo->x,tmpo->y);
      if (tmpo->flags&OF_ACTIVE && !(tmpo->flags&OF_RETURNING) &&
	    Rand<10.0*period)
	SquadronSeté(player,tmpo->x,tmpo->y,tmpo->é+pi/2*(Rand-0.5));
    }
    tmpo=tmpo->next;
  }
}

U0 ShipDamage(Obj *tmpo,F64 d)
{
  I64 i,x=Rand*FIRE_WIDTH,y=Rand*FIRE_HEIGHT;
  tmpo->life_percent-=d;
  while (d>0) {
    if (!Bts(tmpo->fire,y*FIRE_WIDTH+x))
      d-=500.0/(FIRE_WIDTH*FIRE_HEIGHT);
    else
      d-=25.0/(FIRE_WIDTH*FIRE_HEIGHT);
    i=RandI16&7;
    x+=gr_x_offsets[i];
    y+=gr_y_offsets[i];
    while (x>=FIRE_WIDTH)
      x-=FIRE_WIDTH;
    while (x<0)
      x+=FIRE_WIDTH;
    while (y>=FIRE_HEIGHT)
      y-=FIRE_HEIGHT;
    while (y<0)
      y+=FIRE_HEIGHT;
  }
}

U0 ShipFix(Obj *tmpo,F64 d)
{
  tmpo->life_percent+=d;
  if (tmpo->life_percent>=100.0) {
    tmpo->life_percent=100.0;
    MemSet(tmpo->fire,0,sizeof(Obj.fire));
    return;
  }
  while (d>0)
    if (Btr(tmpo->fire,FIRE_WIDTH*FIRE_HEIGHT*Rand))
      d-=400.0/(FIRE_WIDTH*FIRE_HEIGHT);
    else
      d-=20.0/(FIRE_WIDTH*FIRE_HEIGHT);
}

U0 Combat(F64 period)
{
  F64 d;
  Obj *tmpo,*tmpo1;
  Torpedo *tmpt;
  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    tmpo->flags&=~OF_SHOOTING;
    if (tmpo->flags&OF_ACTIVE &&
	  (tmpo1=ObjFind(tmpo->x,tmpo->y,OF_ACTIVE,OF_ACTIVE,,
	  1<<(tmpo->player^1),&d))) {
      tmpo->target_x=tmpo1->x;
      tmpo->target_y=tmpo1->y;
      if (tmpo1->flags&OF_SHIP) {
	if (tmpo->torpedos && d<tmpo->torpedos_range && Rand<125*period) {
	  tmpo->torpedos--;
	  tmpt=CAlloc(sizeof(Torpedo));
	  tmpt->x=tmpo->x;
	  tmpt->y=tmpo->y;
	  tmpt->speed=100;
	  d/=tmpt->speed*(GAME_SPEED_SCALE*game_speed);
	  tmpo1->death_time=tmpt->timeout=tS+d;
	  tmpt->target=tmpo1;
	  tmpt->é=Arg(tmpo1->x-tmpo->x,tmpo1->y-tmpo->y);
	  QueIns(tmpt,torpedo_head.last);
	  Sweep(2000,86,53);
	} else if (tmpo->ship_guns>0 && d<tmpo->ship_guns_range) {
	  tmpo->flags|=OF_SHOOTING;
	  if (Rand<125.0*period) {
	    ShipDamage(tmpo1,tmpo->ship_guns*Rand*period);
	    if (Rand<10.0*period)
	      tmpo1->fuel*=0.75*Rand+0.25;
	  }
	  if (tS>next_noise) {
	    Noise(100,29,46);
	    next_noise=tS+0.1;
	  }
	}
      } else {
	if (tmpo->air_guns>0 && d<tmpo->air_guns_range) {
	  tmpo->flags|=OF_SHOOTING;
	  if (Rand<125.0*period) {
	    tmpo1->life_percent-=tmpo->air_guns*Rand*period;
	    if (Rand<10.0*period)
	      tmpo1->fuel*=0.75*Rand+0.25;
	  }
	  if (tS>next_noise) {
	    Noise(25,62,86);
	    next_noise=tS+0.025;
	  }
	}
      }
    }
    tmpo=tmpo->next;
  }
  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    tmpo1=tmpo->next;
    if (tmpo->type==OT_FIGHTER &&
	  (tmpo->life_percent<=0.0 || tmpo->flags&OF_ACTIVE &&
	  tmpo->fuel<=0.0 || tmpo->host &&
	  !(tmpo->flags&OF_ACTIVE) && tmpo->host->life_percent<=0.0))
      ObjDel(tmpo);
    tmpo=tmpo1;
  }
  tmpo=obj_head.next;
  while (tmpo!=&obj_head) {
    tmpo1=tmpo->next;
    if (tmpo->life_percent<=0.0)
      ObjDel(tmpo);
    tmpo=tmpo1;
  }
}

#define ANIMATE_FREQ	50
U0 AnimateTask(I64)
{
  Obj *tmpo;
  Torpedo *tmpt,*tmpt1;
  F64 d,period;

  while (TRUE) {
    period=GAME_SPEED_SCALE*game_speed/ANIMATE_FREQ;
    SquadronActions;

    tmpo=obj_head.next;
    while (tmpo!=&obj_head) {
      if (tmpo->flags&OF_ACTIVE && tmpo->fuel>0) {
	if (tmpo->dé) {
	  d=tmpo->dé;
	  if (d> tmpo->turn_rate*period) d= tmpo->turn_rate*period;
	  if (d<-tmpo->turn_rate*period) d=-tmpo->turn_rate*period;
	  tmpo->é +=d;
	  tmpo->dé-=d;
	}
	tmpo->x+=tmpo->speed*Cos(tmpo->é)*period*tmpo->life_percent/100.0;
	tmpo->y+=tmpo->speed*Sin(tmpo->é)*period*tmpo->life_percent/100.0;
	tmpo->fuel-=tmpo->speed*tmpo->fuel_burn_rate*period;
      }
      tmpo=tmpo->next;
    }

    tmpt=torpedo_head.next;
    while (tmpt!=&torpedo_head) {
      tmpt1=tmpt->next;
      if (tS>tmpt->timeout) {
	tmpo=tmpt->target;
	if (Rand<0.333333) {
	  ShipDamage(tmpo,150*Rand*Rand);
	  if (Rand<0.333333)
	    tmpo->fuel*=0.75*Rand+0.25;
	}
	QueRem(tmpt);
	Free(tmpt);
      } else {
	tmpt->x+=tmpt->speed*Cos(tmpt->é)*period;
	tmpt->y+=tmpt->speed*Sin(tmpt->é)*period;
      }
      tmpt=tmpt1;
    }

    tmpo=obj_head.next;
    while (tmpo!=&obj_head) {
      if (tmpo->host && !(tmpo->flags&OF_ACTIVE)) {
	tmpo->x=tmpo->host->x;
	tmpo->y=tmpo->host->y;
      }
      tmpo=tmpo->next;
    }

    tmpo=obj_head.next;
    while (tmpo!=&obj_head) {
      if (tmpo->flags&OF_ACTIVE) {
	if (tmpo->host) {
	  d=Sqrt(Sqr(tmpo->x-tmpo->host->x)+Sqr(tmpo->y-tmpo->host->y));
	  if (d<8 && tmpo->max_fuel-tmpo->fuel>30) {
	    tmpo->life_percent=100.0;
	    if (tmpo->host->fuel>0) {
	      d=tmpo->max_fuel-tmpo->fuel;
	      if (d>tmpo->host->fuel) d=tmpo->host->fuel;
	      tmpo->host->fuel-=d;
	      tmpo->fuel+=d;
	    }
	    tmpo->torpedos=tmpo->max_torpedos;
	    tmpo->x=tmpo->host->x;
	    tmpo->y=tmpo->host->y;
	    tmpo->flags&=~OF_ACTIVE;
	  } else if (d>tmpo->fuel-250)
	    tmpo->flags|=OF_RETURNING;
	  if (tmpo->flags&OF_RETURNING)
	    tmpo->dé+=Wrap(Arg(tmpo->host->x-tmpo->x,
		  tmpo->host->y-tmpo->y)-(tmpo->é+tmpo->dé),-ã);
	} else if (tmpo->type==OT_CARRIER)
	  ShipFix(tmpo,2.5*period);
      }
      tmpo=tmpo->next;
    }

    AI(1,period);
    Combat(period);
    Sleep(1000/ANIMATE_FREQ);
  }
}

U0 FlatTops()
{
  I64 arg1,arg2;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->text_attr=BLUE<<4+WHITE;
  AutoComplete;
  WinBorder;
  WinMax;
  DocCursor;
  DocClear;
  PopUpOk("$$GREEN$${Left-Mouse}$$FG$$\t\tChange Course\n"
	"$$GREEN$${Right-Mouse}$$FG$$\t\tLaunch Squadron\n"
	"$$GREEN$${Right-Double-Mouse}$$FG$$\tReturn Squadron\n"
	"$$GREEN$${Ctrl-Left Grab}$$FG$$\tScroll Scrn\n");
  MenuPush(
	"File {"
	"  Abort(,CH_SHIFT_ESC);"
	"  Exit(,CH_ESC);"
	"}"
	"Play {"
	"  Restart(,'\n');"
	"  Faster(,'+');"
	"  Slower(,'-');"
	"}"
	);
  Fs->win_inhibit|=WIF_SELF_MS_L|WIF_SELF_MS_R;
  Init;
  Fs->draw_it=&DrawIt;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",,Fs);
  try {
    while (TRUE)
      switch (GetMsg(&arg1,&arg2,
	    1<<MSG_KEY_DOWN|1<<MSG_MS_L_UP|1<<MSG_MS_R_UP|1<<MSG_MS_R_D_UP)) {
	case MSG_KEY_DOWN:
	  switch (arg1) {
	    case '\n':
	      CleanUp;
	      Init;
	      break;
	    case CH_ESC:
	    case CH_SHIFT_ESC:
	      goto nv_done;
	    case '+':
	      game_speed*=1.5;
	      break;
	    case '-':
	      game_speed/=1.5;
	      break;
	  }
	  break;
	case MSG_MS_L_UP:
	  SquadronSeté(0,arg1,arg2);
	  break;
	case MSG_MS_R_UP:
	  SquadronLaunch(0,arg1,arg2);
	  break;
	case MSG_MS_R_D_UP:
	  SquadronReturn(0,arg1,arg2);
	  break;
      }
nv_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    PutExcept;
  SettingsPop;
  CleanUp;
  MenuPop;
}

FlatTops;

















//Maybe use this in the future	$SP,"<7>",BI=7$
















        r     úÿÿÿéÿÿÿ   &   ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ          r     úÿÿÿéÿÿÿ   &   ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ          b     üÿÿÿìÿÿÿ	   %   ÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿ                                                                                                                                       ÿ       ÿÿ       ÿÿÿ       ÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿ               b     üÿÿÿìÿÿÿ	   %   ÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿ                                                                                                                                       ÿ       ÿÿ       ÿÿÿ       ÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿ               Z      þÿÿÿüÿÿÿ   	   ÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿ ÿ ÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿ         Z      þÿÿÿüÿÿÿ   	   ÿÿÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿ ÿ ÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿÿÿ ÿÿÿ         ã         <   ,         ,  çÿÿÿ    }   âÿÿÿ    i   Øÿÿÿ    U   Øÿÿÿ    U   âÿÿÿ        Éÿÿÿ    ÔþÿÿÉÿÿÿ    ÔþÿÿF           F       d          íþÿÿÎÿÿÿ    ÿÿÿÎÿÿÿ    ÿÿÿâÿÿÿ    íþÿÿâÿÿÿ    íþÿÿÎÿÿÿ   ÿÿÿÎÿÿÿ   ÿÿÿâÿÿÿ   íþÿÿâÿÿÿ   i   âÿÿÿ    ,     öÿÿÿ,  çÿÿÿöÿÿÿ}   âÿÿÿöÿÿÿi   ØÿÿÿöÿÿÿU   ØÿÿÿöÿÿÿU   âÿÿÿöÿÿÿ    ÉÿÿÿöÿÿÿÔþÿÿÉÿÿÿöÿÿÿÔþÿÿF   öÿÿÿ    F   öÿÿÿd      öÿÿÿi   âÿÿÿöÿÿÿ                                                                                          	         	                                    
            	      
      
            
            
            
                                                                                                                                                                                                                                                                                                                                                   
                   
      	         
         	               	                                                                                                                                                ,         ,  ìÿÿÿ       Øÿÿÿ    ÔþÿÿÝÿÿÿ    Ôþÿÿ<          2       Þþÿÿ-   âÿÿÿÞþÿÿâÿÿÿâÿÿÿ   Ýÿÿÿâÿÿÿ   -   âÿÿÿ'     âÿÿÿ'  ñÿÿÿâÿÿÿ                     	      	               	   
                                                      	               	            
   	      
                
          
                                  