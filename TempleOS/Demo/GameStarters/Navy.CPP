#define GAME_SPEED_SCALE	0.1

F64 game_speed,
    launch_unit_x1,launch_unit_y1,launch_unit_x2,launch_unit_y2,launch_t,
    return_unit_x1,return_unit_y1,return_unit_x2,return_unit_y2,return_t,
    set_é_unit_x1,set_é_unit_y1,set_é_unit_x2,set_é_unit_y2,set_é_t;
CTask *main_task;

#define OT_CARRIER	0
#define OT_CRUISER	1
#define OT_FIGHTER	2
#define OT_NUM_TYPES	3

#define OF_SHIP		1
#define OF_ACTIVE	2
#define OF_RETURNING	4
#define OF_SHOOTING	8

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
} obj_root;

class Torpedo
{
  Torpedo *next,*last;
  Obj *target;
  F64 x,y,é,speed,timeout;
} torpedo_root;

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

#define NUM_PLAYERS	2

I64 num_carriers[NUM_PLAYERS]={2,3},
    num_cruisers[NUM_PLAYERS]={2,3},
    num_planes_per_squadron[NUM_PLAYERS]={6,5},
    num_squadrons_per_carrier[NUM_PLAYERS]={2,3},
    num_alive[NUM_PLAYERS],
    num_squadrons;

Obj *ObjFind(F64 x,F64 y,
	     I64 flag_mask=OF_ACTIVE|OF_RETURNING,I64 flag_val=OF_ACTIVE,
	     I64 type_mask=-1,I64 player_mask=-1,F64 *_d=NULL)
{
  Obj *tempo=obj_root.next,*best=NULL;
  F64 dd,best_dd=MAX_F64;
  while (tempo!=&obj_root) {
    if (tempo->flags&flag_mask==flag_val &&
	Bt(&type_mask,tempo->type)&&Bt(&player_mask,tempo->player)) {
      dd=Sqr(tempo->x-x)+Sqr(tempo->y-y);
      if (dd<best_dd) {
	best=tempo;
	best_dd=dd;
      }
    }
    tempo=tempo->next;
  }
  if (_d) *_d=Sqrt(best_dd);
  return best;
}

U0 ObjDel(Obj *tempo)
{
  if (tempo) {
    if (tT<tempo->death_time)
      tempo->flags&=~OF_ACTIVE;
    else {
      if (tempo->squadron>=0)
	SquadronRem(tempo);
      QueRem(tempo);
      if (tempo->squadron>=0)
	LBts(&squadrons[tempo->squadron].dead_mask,tempo->member_num);
      num_alive[tempo->player]--;
      Free(tempo);
    }
  }
}

$BG,1$


$SP,"<1>",1$




$SP,"<2>",2$


$SP,"<3>",3$



$SP,"<4>",4$




$SP,"<5>",5$


$SP,"<6>",6$
$BG$
U8 *imgs[NUM_PLAYERS][OT_NUM_TYPES]={{$IB,"<1>",1$,$IB,"<2>",2$,$IB,"<3>",3$},{$IB,"<4>",4$,$IB,"<5>",5$,$IB,"<6>",6$}};

U0 DrawIt(CTask *task,CDC *dc)
{
  F64 d,cur_time=tT;
  Obj *tempo;
  Torpedo *tempt;
  Bool old_preempt=Preempt(OFF);
  task->text_attr=BLUE<<4+WHITE;


  tempt=torpedo_root.next;
  while (tempt!=&torpedo_root) {
    dc->color=WHITE;
    GrPlot(dc,tempt->x,tempt->y);
    tempt=tempt->next;
  }

  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    if (tempo->flags&OF_ACTIVE && tempo->flags&OF_SHIP) {
      SpritePlotRotZ3b(dc,tempo->x,tempo->y,0,imgs[tempo->player][tempo->type],tempo->é+ã/2);
      if (Blink) {
	dc->color=BLACK;
	GrLine(dc,tempo->x+5,tempo->y,tempo->x+5+10,tempo->y);

	if (tempo->life_percent>0) {
	  if (tempo->life_percent<33)
	    dc->color=RED;
	  else if (tempo->life_percent<66)
	    dc->color=YELLOW;
	  else
	    dc->color=GREEN;
	  GrLine(dc,tempo->x+5,tempo->y,tempo->x+5+10*tempo->life_percent/100,tempo->y);
	}

	dc->color=BLACK;
	GrLine(dc,tempo->x+5,tempo->y+2,tempo->x+5+10,tempo->y+2);
	d=tempo->fuel*100/tempo->max_fuel;
	if (d>0) {
	  if (d<33)
	    dc->color=RED;
	  else if (d<66)
	    dc->color=YELLOW;
	  else
	    dc->color=GREEN;
	  GrLine(dc,tempo->x+5,tempo->y+2,tempo->x+5+10*d/100,tempo->y+2);
	}
      }
    }
    tempo=tempo->next;
  }

  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    if (tempo->flags&OF_ACTIVE && !(tempo->flags&OF_SHIP))
      SpritePlotRotZ3b(dc,tempo->x,tempo->y,0,imgs[tempo->player][tempo->type],tempo->é+ã/2);
    if (tempo->flags&OF_SHOOTING) {
      dc->color=LTRED;
      GrLine(dc,tempo->x,tempo->y,tempo->target_x,tempo->target_y,3);
    }
    tempo=tempo->next;
  }

  dc->flags|=DCF_TRANSFORMATION;
  if (cur_time<launch_t) {
    dc->color=LTGREEN;
    GrArrow3(dc,launch_unit_x1,launch_unit_y1,0,launch_unit_x2,launch_unit_y2,0);
  }
  if (cur_time<return_t) {
    dc->color=LTRED;
    GrArrow3(dc,return_unit_x1,return_unit_y1,0,return_unit_x2,return_unit_y2,0);
  }
  if (cur_time<set_é_t) {
    dc->color=YELLOW;
    GrArrow3(dc,set_é_unit_x1,set_é_unit_y1,0,set_é_unit_x2,set_é_unit_y2,0);
  }

  dc->color=YELLOW;
  GrPrintF(dc,-task->win_scroll_x,-task->win_scroll_y,"Game Speed: %5.2f",game_speed);
  dc->color=LTCYAN;
  GrPrintF(dc,-task->win_scroll_x,-task->win_scroll_y+	FONT_HEIGHT,"Player 1: %d",num_alive[0]);
  dc->color=LTPURPLE;
  GrPrintF(dc,-task->win_scroll_x,-task->win_scroll_y+2*FONT_HEIGHT,"Player 2: %d",num_alive[1]);

  if ((!num_alive[0]||!num_alive[1]) && Blink) {
    if (!num_alive[1]) {
      dc->color=LTGREEN;
      GrPutS(dc,task->win_pixel_width>>1 -(FONT_WIDTH*14)/2-task->win_scroll_x,
	      task->win_pixel_height>>1-FONT_HEIGHT/2-task->win_scroll_y,"Game Completed");
    } else {
      dc->color=LTRED;
      GrPutS(dc,task->win_pixel_width>>1 -(FONT_WIDTH*9)/2-task->win_scroll_x,
	      task->win_pixel_height>>1-FONT_HEIGHT/2-task->win_scroll_y,"Game Over");
    }
  }
  Preempt(old_preempt);
}

U0 Init()
{
  I64 i,fighter,ship,player,squadron,
      w=Fs->win_pixel_width,h=Fs->win_pixel_height;
  Squadron *temps;
  Obj *tempo,*tempo1;
  QueInit(&obj_root);
  QueInit(&torpedo_root);

  Fs->win_scroll_x=0;
  Fs->win_scroll_y=0;
  game_speed=1.0;
  launch_t=return_t=set_é_t=0;
  main_task=Fs;
  num_squadrons=0;
  for (player=0;player<NUM_PLAYERS;player++) {
    num_alive[player]=0;
    for (ship=0;ship<num_cruisers[player];ship++) {
      tempo1=CAlloc(sizeof(Obj));
      num_alive[player]++;
      tempo1->type=OT_CRUISER;	tempo1->player=player;
      tempo1->squadron=-1;	tempo1->member_num=ship;
      tempo1->flags=OF_ACTIVE|OF_SHIP;
      tempo1->x=0.8*w*(Rand-0.5)+w>>1; tempo1->y=0.8*h*(Rand-0.5)+h>>1;
      tempo1->host=NULL;
      tempo1->speed=35.0;	tempo1->turn_rate=2.5;
      tempo1->é=2*ã*(Rand-0.5);	tempo1->dé=0;
      tempo1->life_percent=100.0;
      tempo1->fuel=tempo1->max_fuel=100000;
      tempo1->fuel_burn_rate=100.0;
      tempo1->air_guns =5000;	tempo1->air_guns_range =30.0;
      tempo1->ship_guns=10000;	tempo1->ship_guns_range=30.0;
      tempo1->torpedos =tempo1->max_torpedos=0;
      tempo1->torpedos_range =0.0;
      tempo1->next_action_time=0;
      QueIns(tempo1,obj_root.last);
    }

    for (ship=0;ship<num_carriers[player];ship++) {
      tempo1=CAlloc(sizeof(Obj));
      num_alive[player]++;
      tempo1->type=OT_CARRIER;	tempo1->player=player;
      tempo1->squadron=-1;	tempo1->member_num=ship;
      tempo1->flags=OF_ACTIVE|OF_SHIP;
      tempo1->x=0.8*w*(Rand-0.5)+w>>1; tempo1->y=0.8*h*(Rand-0.5)+h>>1;
      tempo1->host=NULL;
      tempo1->speed=28.0;	tempo1->turn_rate=1.0;
      tempo1->é=2*ã*(Rand-0.5);	tempo1->dé=0;
      tempo1->life_percent=100.0;
      tempo1->fuel=tempo1->max_fuel=750000;
      tempo1->fuel_burn_rate=500.0;
      tempo1->air_guns =5000;	tempo1->air_guns_range =20.0;
      tempo1->ship_guns=2000;	tempo1->ship_guns_range=30.0;
      tempo1->torpedos =tempo1->max_torpedos=0;
      tempo1->torpedos_range =0.0;
      tempo1->next_action_time=0;
      QueIns(tempo1,obj_root.last);

      for (squadron=0;squadron<num_squadrons_per_carrier[player];squadron++,num_squadrons++) {
	for (fighter=0;fighter<num_planes_per_squadron[player];fighter++) {
	  tempo=CAlloc(sizeof(Obj));
	  num_alive[player]++;
	  tempo->type=OT_FIGHTER;	 tempo->player=player;
	  tempo->squadron=num_squadrons; tempo->member_num=fighter;
	  tempo->flags=0;
	  tempo->host=tempo1;
	  tempo->speed=300.0;		tempo->turn_rate=25.0;
	  tempo->life_percent=100.0;
	  tempo->fuel=tempo->max_fuel=1000;
	  tempo->fuel_burn_rate=1.0;
	  tempo->air_guns =35000;	tempo->air_guns_range =8.0;
	  tempo->ship_guns=0;		tempo->ship_guns_range=0.0;
	  tempo->torpedos =tempo->max_torpedos=1;
	  tempo->torpedos_range =20.0;
	  QueIns(tempo,obj_root.last);
	}
      }
    }
  }
  squadrons=CAlloc(num_squadrons*sizeof(Squadron));
  for (i=0,temps=squadrons;i<num_squadrons;i++,temps++) {
    temps->next_in_squadron=temps->last_in_squadron=temps;
    temps->squadron=i;
  }
  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    if (tempo->squadron>=0) {
      temps=&squadrons[tempo->squadron];
      temps->host  =tempo->host;
      temps->player=tempo->player;
      temps->total_mask=1<<num_planes_per_squadron[tempo->player]-1;
      SquadronIns(tempo,temps->last_in_squadron);
    }
    tempo=tempo->next;
  }
}

U0 CleanUp()
{
  QueDel(&obj_root,TRUE);
  QueDel(&torpedo_root,TRUE);
  Free(squadrons);
}

Obj *ObjLaunch(I64 player,I64 squadron=-1,Obj *host=NULL,F64 x=MAX_F64,F64 y=MAX_F64,F64 é=MAX_F64)
{
  Obj *tempo;
  F64 cur_time=tT;
  if (!host)
    host=ObjFind(x,y,,,1<<OT_CARRIER,1<<player);
  if (host && cur_time>host->next_action_time) {
    if (é==MAX_F64)
      é=Arg(x-host->x,y-host->y);
    tempo=obj_root.next;
    while (tempo!=&obj_root) {
      if (tempo->host==host && (squadron<0||tempo->squadron==squadron) && !(tempo->flags&OF_ACTIVE) &&
	(tempo->squadron<0||squadrons[tempo->squadron].action==SA_PARKED||squadrons[tempo->squadron].action==SA_LAUNCHING)) {
	if (tempo->fuel<=0.0) //When low on fuel, not zero, gets launched and captured.
	  LBts(&squadrons[tempo->squadron].dead_mask,tempo->member_num);
	else {
	  tempo->flags=tempo->flags&~OF_RETURNING|OF_ACTIVE;
	  tempo->é=host->é;
	  if (x==MAX_F64 || y==MAX_F64 || Sqr(x-host->x)+Sqr(y-host->y)>3*3)
	    tempo->dé=Wrap(é-tempo->é,-ã);
	  else
	    tempo->dé=0;
	  tempo->x=host->x;
	  tempo->y=host->y;
	  host->next_action_time=cur_time+0.25/game_speed;
	  return tempo;
	}
      }
      tempo=tempo->next;
    }
  }
  return NULL;
}

Squadron *SquadronLaunch(I64 player,F64 x=MAX_F64,F64 y=MAX_F64)
{
  Squadron *temps;
  Obj *tempo;
  if (tempo=ObjLaunch(player,,,x,y)) {
    if (player==0) {
      launch_unit_x1=tempo->x;
      launch_unit_y1=tempo->y;
      launch_unit_x2=ipx-main_task->win_pixel_left-main_task->win_scroll_x;
      launch_unit_y2=ipy-main_task->win_pixel_top -main_task->win_scroll_y;
      launch_t=tT+0.5;
    }
    if (tempo->squadron>=0) {
      temps=&squadrons[tempo->squadron];
      if (temps->action==SA_PARKED) {
	temps->action=SA_LAUNCHING;
	temps->é=tempo->é+tempo->dé;
      }
      return temps;
    }
  }
  return NULL;
}


Obj *ObjReturn(I64 player,F64 x,F64 y)
{
  Obj *tempo;
  if (tempo=ObjFind(x,y,OF_ACTIVE,OF_ACTIVE,1<<OT_FIGHTER,1<<player))
    tempo->flags|=OF_RETURNING;
  return tempo;
}

Squadron *SquadronReturn(I64 player,F64 x,F64 y)
{
  Squadron *temps;
  Obj *tempo;
  if (tempo=ObjReturn(player,x,y)) {
    if (player==0) {
      return_unit_x1=tempo->x;
      return_unit_y1=tempo->y;
      if (tempo->host) {
	return_unit_x2=tempo->host->x;
	return_unit_y2=tempo->host->y;
	return_t=tT+0.5;
      }
    }
    if (tempo->squadron>=0) {
      temps=&squadrons[tempo->squadron];
      if (temps->action==SA_FLYING)
	temps->action=SA_RETURNING;
      return temps;
    }
  }
  return NULL;
}

Obj *ObjSeté(I64 player,F64 x=MAX_F64,F64 y=MAX_F64,F64 é=MAX_F64)
{
  Obj *tempo;
  if ((tempo=ObjFind(x,y,,,,1<<player)) && tempo->flags&OF_ACTIVE && !(tempo->flags&OF_RETURNING)) {
    if (é==MAX_F64)
      é=Arg(x-tempo->x,y-tempo->y);
    tempo->dé+=Wrap(é-(tempo->é+tempo->dé),-ã);
    return tempo;
  }
  return NULL;
}

Squadron *SquadronSeté(I64 player,F64 x=MAX_F64,F64 y=MAX_F64,F64 é=MAX_F64)
{
  Squadron *temps;
  Obj *tempo;
  if (tempo=ObjSeté(player,x,y,é)) {
    if (player==0) {
      set_é_unit_x1=tempo->x;
      set_é_unit_y1=tempo->y;
      set_é_unit_x2=ipx-main_task->win_pixel_left-main_task->win_scroll_x;
      set_é_unit_y2=ipy-main_task->win_pixel_top -main_task->win_scroll_y;
      set_é_t=tT+0.5;
    }
    if (tempo->squadron>=0) {
      temps=&squadrons[tempo->squadron];
      if (temps->action==SA_FLYING) {
	temps->action=SA_SET_é;
	temps->é=tempo->é+tempo->dé;
      }
      return temps;
    }
  }
  return NULL;
}

U0 SquadronActions()
{
  I64 i,completed_mask;
  Obj *tempo;
  Squadron *temps;
  for (i=0,temps=squadrons;i<num_squadrons;i++,temps++) {
    completed_mask=0;
    switch (temps->action) {
      case SA_LAUNCHING:
	ObjLaunch(temps->player,i,temps->host,,,temps->é);
	tempo=temps->next_in_squadron;
	while (tempo!=temps) {
	  LBEqu(&completed_mask,tempo->member_num,tempo->flags&OF_ACTIVE);
	  tempo=tempo->next_in_squadron;
	}
	if (completed_mask|temps->dead_mask==temps->total_mask)
	  temps->action=SA_FLYING;
	break;
      case SA_FLYING:
	tempo=temps->next_in_squadron;
	while (tempo!=temps) {
	  LBEqu(&completed_mask,tempo->member_num,!(tempo->flags&OF_ACTIVE));
	  tempo=tempo->next_in_squadron;
	}
	if (completed_mask|temps->dead_mask==temps->total_mask)
	  temps->action=SA_PARKED;
	break;
      case SA_SET_é:
	tempo=temps->next_in_squadron;
	while (tempo!=temps) {
	  tempo->dé+=Wrap(temps->é-(tempo->é+tempo->dé),-ã);
	  tempo=tempo->next_in_squadron;
	}
	temps->action=SA_FLYING;
	break;
      case SA_RETURNING:
	tempo=temps->next_in_squadron;
	while (tempo!=temps) {
	  tempo->flags|=OF_RETURNING;
	  LBEqu(&completed_mask,tempo->member_num,!(tempo->flags&OF_ACTIVE));
	  tempo=tempo->next_in_squadron;
	}
	if (completed_mask|temps->dead_mask==temps->total_mask)
	  temps->action=SA_PARKED;
	break;
    }
    if (temps->dead_mask==temps->total_mask)
      temps->action=SA_DEAD;
  }
}

U0 AI(I64 player,F64 period)
{
  Obj *tempo;
  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    if (tempo->player==player) {
      if (tempo->type==OT_CARRIER && Rand<5*period)
	SquadronLaunch(player,tempo->x,tempo->y);
      if (tempo->flags&OF_ACTIVE && !(tempo->flags&OF_RETURNING) && Rand<10.0*period)
	SquadronSeté(player,tempo->x,tempo->y,tempo->é+pi/2*(Rand-0.5));
    }
    tempo=tempo->next;
  }
}

U0 Combat(F64 period)
{
  F64 d;
  Obj *tempo,*tempo1;
  Torpedo *tempt;
  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    tempo->flags&=~OF_SHOOTING;
    if (tempo->flags&OF_ACTIVE &&
      (tempo1=ObjFind(tempo->x,tempo->y,OF_ACTIVE,OF_ACTIVE,,1<<(tempo->player^1),&d))) {
      tempo->target_x=tempo1->x;
      tempo->target_y=tempo1->y;
      if (tempo1->flags&OF_SHIP) {
	if (tempo->torpedos && d<tempo->torpedos_range && Rand<125*period) {
	  tempo->torpedos--;
	  tempt=CAlloc(sizeof(Torpedo));
	  tempt->x=tempo->x;
	  tempt->y=tempo->y;
	  tempt->speed=100;
	  d/=tempt->speed*(GAME_SPEED_SCALE*game_speed);
	  tempo1->death_time=tempt->timeout=tT+d;
	  tempt->target=tempo1;
	  tempt->é=Arg(tempo1->x-tempo->x,tempo1->y-tempo->y);
	  QueIns(tempt,torpedo_root.last);
	  Sweep(2000,2000,300);
	} else if (tempo->ship_guns>0 && d<tempo->ship_guns_range) {
	  tempo->flags|=OF_SHOOTING;
	  if (Rand<125.0*period) {
	    tempo1->life_percent-=tempo->ship_guns*Rand*period;
	    if (Rand<10.0*period)
	      tempo1->fuel*=0.75*Rand+0.25;
	  }
	  Noise(100,75,200);
	}
      } else {
	if (tempo->air_guns>0 && d<tempo->air_guns_range) {
	  tempo->flags|=OF_SHOOTING;
	  if (Rand<125.0*period) {
	    tempo1->life_percent-=tempo->air_guns*Rand*period;
	    if (Rand<10.0*period)
	      tempo1->fuel*=0.75*Rand+0.25;
	  }
	  Noise(25,500,2000);
	}
      }
    }
    tempo=tempo->next;
  }
  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    tempo1=tempo->next;
    if (tempo->type==OT_FIGHTER &&
	(tempo->life_percent<=0.0 || tempo->flags&OF_ACTIVE && tempo->fuel<=0.0 ||
	 tempo->host && !(tempo->flags&OF_ACTIVE) && tempo->host->life_percent<=0.0))
      ObjDel(tempo);
    tempo=tempo1;
  }
  tempo=obj_root.next;
  while (tempo!=&obj_root) {
    tempo1=tempo->next;
    if (tempo->life_percent<=0.0)
      ObjDel(tempo);
    tempo=tempo1;
  }
}

#define ANIMATE_FREQ	50
U0 AnimateTask(I64)
{ //Preemption is off after Spawn
  Obj *tempo;
  Torpedo *tempt,*tempt1;
  F64 d,period;

  while (TRUE) {
    period=GAME_SPEED_SCALE*game_speed/ANIMATE_FREQ;
    SquadronActions;

    tempo=obj_root.next;
    while (tempo!=&obj_root) {
      if (tempo->flags&OF_ACTIVE && tempo->fuel>0) {
	if (tempo->dé) {
	  d=tempo->dé;
	  if (d> tempo->turn_rate*period) d= tempo->turn_rate*period;
	  if (d<-tempo->turn_rate*period) d=-tempo->turn_rate*period;
	  tempo->é +=d;
	  tempo->dé-=d;
	}
	tempo->x+=tempo->speed*Cos(tempo->é)*period*tempo->life_percent/100.0;
	tempo->y+=tempo->speed*Sin(tempo->é)*period*tempo->life_percent/100.0;
	tempo->fuel-=tempo->speed*tempo->fuel_burn_rate*period;
      }
      tempo=tempo->next;
    }

    tempt=torpedo_root.next;
    while (tempt!=&torpedo_root) {
      tempt1=tempt->next;
      if (tT>tempt->timeout) {
	tempo=tempt->target;
	if (Rand<0.333333) {
	  tempo->life_percent-=150*Rand*Rand;
	  if (Rand<0.333333)
	    tempo->fuel*=0.75*Rand+0.25;
	}
	QueRem(tempt);
	Free(tempt);
      } else {
	tempt->x+=tempt->speed*Cos(tempt->é)*period;
	tempt->y+=tempt->speed*Sin(tempt->é)*period;
      }
      tempt=tempt1;
    }

    tempo=obj_root.next;
    while (tempo!=&obj_root) {
      if (tempo->host && !(tempo->flags&OF_ACTIVE)) {
	tempo->x=tempo->host->x;
	tempo->y=tempo->host->y;
      }
      tempo=tempo->next;
    }

    tempo=obj_root.next;
    while (tempo!=&obj_root) {
      if (tempo->flags&OF_ACTIVE) {
	if (tempo->host) {
	  d=Sqrt(Sqr(tempo->x-tempo->host->x)+Sqr(tempo->y-tempo->host->y));
	  if (d<8 && tempo->max_fuel-tempo->fuel>30) {
	    tempo->life_percent=100.0;
	    if (tempo->host->fuel>0) {
	      d=tempo->max_fuel-tempo->fuel;
	      if (d>tempo->host->fuel) d=tempo->host->fuel;
	      tempo->host->fuel-=d;
	      tempo->fuel+=d;
	    }
	    tempo->torpedos=tempo->max_torpedos;
	    tempo->x=tempo->host->x;
	    tempo->y=tempo->host->y;
	    tempo->flags&=~OF_ACTIVE;
	  } else if (d>tempo->fuel-250)
	    tempo->flags|=OF_RETURNING;
	  if (tempo->flags&OF_RETURNING)
	    tempo->dé+=Wrap(Arg(tempo->host->x-tempo->x,tempo->host->y-tempo->y)-(tempo->é+tempo->dé),-ã);
	} else if (tempo->type==OT_CARRIER)
	  tempo->life_percent=Min(100.0,tempo->life_percent+2.5*period);
      }
      tempo=tempo->next;
    }

    AI(1,period);
    Combat(period);
    Sleep(1000/ANIMATE_FREQ);
  }
}


U0 Navy()
{
  I64 p1,p2;

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  PopUpOk("$$FG,GREEN$${Left-Mouse}$$FG$$\t\tChange Course\n"
	  "$$FG,GREEN$${Right-Mouse}$$FG$$\t\tLaunch Squadron\n"
	  "$$FG,GREEN$${Right-Double-Mouse}$$FG$$\tReturn Squadron\n"
	  "$$FG,GREEN$${Ctrl-Left Grab}$$FG$$\tScroll Screen\n");
  Preempt(OFF),
  WinBorder;
  WordStat;

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
  WinMax;
  Fs->win_inhibit|=WIF_IP_L|WIF_IP_R;
  Init;
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  Fs->animate_task=Spawn(&AnimateTask,NULL,"Animate",Fs);
  try {
    while (TRUE)
      switch(GetMsg(&p1,&p2,1<<MSG_KEY_DOWN|1<<MSG_IP_L_UP|1<<MSG_IP_R_UP|1<<MSG_IP_R_D_UP)) {
	case MSG_KEY_DOWN:
	  switch (p1) {
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
	case MSG_IP_L_UP:
	  SquadronSeté(0,p1,p2);
	  break;
	case MSG_IP_R_UP:
	  SquadronLaunch(0,p1,p2);
	  break;
	case MSG_IP_R_D_UP:
	  SquadronReturn(0,p1,p2);
	  break;
      }
nv_done:
    GetMsg(,,1<<MSG_KEY_UP);
  } catch
    CatchAll;
  MenuPop;
  SettingsPop;
  CleanUp;
}

Navy;
           r        &   úÿÿÿéÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ             r        &   úÿÿÿéÿÿÿÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿ  ÿÿÿ  ÿÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿ  ÿÿ  ÿÿ  ÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿ  ÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿ  ÿÿÿÿÿÿ  ÿÿÿÿÿÿ             b     	   %   üÿÿÿìÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿ                                                                                                                                       ÿ       ÿÿ       ÿÿÿ       ÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿ                  b     	   %   üÿÿÿìÿÿÿÿÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿÿ       ÿÿ                                                                                                                                       ÿ       ÿÿ       ÿÿÿ       ÿÿÿÿ       ÿÿÿÿÿ       ÿÿÿÿÿÿ       ÿÿÿÿÿÿÿ       ÿÿÿÿÿÿÿÿ                  0          þÿÿÿ       ÿÿÿÿ                              0          þÿÿÿ       ÿÿÿÿ                    