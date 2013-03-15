/*
Alteration Ideas:
  * Add another fire phase
  * Add opportunity fire
  * Add overruns
  * Add close assaults
  * Add more unit types
  * Add artillery
  * Add aircraft
  * Add troop transport
  * Add land mines
  * Add buildings
  * Adjust terrain cost
  * Adjust terrain defense bonuses
  * Add bridges
  * Add amphibious scenareos
  * Add vehicle wreckage
  * Add scoring
  * Add game turn limit
*/

#define MAP_WIDTH		603
#define MAP_HEIGHT		452
#define MAX_UNITS		32
#define P1_TO_P2_ODDS		1.0
#define PLAYER_1_ARMOR_PERCENT	50
#define PLAYER_2_ARMOR_PERCENT	50

/*Constants are often faster when compiled.
This is an advantage of just-in-time compilation.
You can't change them when you start a
game over, however.  A down-side of this
is you can't use the <CTRL-SHIFT-L> auto-indent feature.
*/

#define HEX_SIDE	11
#exe {
  CTask *task;
  PopUpFile("/TempleOS/Demo/Graphics/RotateTank.CPP",FALSE,NULL,&task);
  F64 d;
  PopUpOk("This aspires to be a rip-off of\n"
	  "the Perfect General, but has tank\n"
	  "facing like the board game,\n"
	  "Stalin's Tanks.\n");
  if (!PopUpNoYes("Tournament Settings")) {
    d=PopUpRangeF64Log(300,2000,20,"%4f","Map Width In Pixels\n");
    DefinePrintF("MAP_WIDTH","%f",d);
    DefinePrintF("MAP_HEIGHT","%f",480.0/640.0*d);
    d=PopUpRangeF64Exp(8,512,2,"%3f","Player 2 Units\n");
    DefinePrintF("MAX_UNITS","%f",d);
    d=PopUpRangeF64(0.2,1.01,0.05,"%4.2f to 1.00","Player1:Player2 Odds\n");
    DefinePrintF("P1_TO_P2_ODDS","%4.2f",d);
    d=PopUpRangeF64(0,100,10,"%3f% %%","Player 1 Armor Percent\n");
    DefinePrintF("PLAYER_1_ARMOR_PERCENT","%f",d);
    d=PopUpRangeF64(0,100,10,"%3f% %%","Player 2 Armor Percent\n");
    DefinePrintF("PLAYER_2_ARMOR_PERCENT","%f",d);
  }
  DefinePrintF("DCOS","%12.9f",HEX_SIDE*Cos(60.0/180*ã));
  DefinePrintF("DSIN","%12.9f",HEX_SIDE*Sin(60.0/180*ã));
  DefinePrintF("HEX_RADIUS","%12.9f",DSIN+0.01); //slop
  d=PopUpRangeF64(0,100,25,"%3f% %%","Animation Delay\n");
  DefinePrintF("ANIMATION_DELAY","%5.3f",d/100.0);
  XTalk(task," ");
  TaskWait(task);
  Kill(task);
};


I64 map_cols=(MAP_WIDTH-DCOS)/(2*HEX_SIDE+2*DCOS),
   map_rows=ToI64((MAP_HEIGHT-DSIN)/DSIN)&~1,
   map_width=map_cols*(2*HEX_SIDE+2*DCOS)+DCOS,
   map_height=map_rows*DSIN+DSIN+1;

CDC *map;
U8 terrain[map_rows][map_cols];


//Centers of hexes
class Pt
{
  F64 x,y;
};
Pt hex_centers[map_rows][map_cols];

I64 show_visible_row,show_visible_col;
Bool roads[map_rows][map_cols],
       rivers[map_rows][map_cols],
       visible_map[map_rows][map_cols];

//Other options for PLAINS are WHITE or YELLOW
#define PLAINS		LTGREEN
#define TREES		GREEN
#define MOUNTAINS	DKGRAY

U8 movement_costs[16];
movement_costs[PLAINS]=2;
movement_costs[TREES]=6;
movement_costs[MOUNTAINS]=10;

//These are used to display a range circle when they player
//is firing.
F64 fire_radius,fire_radius_x,fire_radius_y;

//These display "phase", "turn" and "game over".
U8 msg_buf[136];
I64 msg_off_timeout; //Jiffies. Goes away after a time.

/*I got tricky by not defining a color
right away in these $LK,"CSprite","MN:CSprite"$s so they can
work for both players by setting dc->color
before drawing them.  I actually made these
graphics by defining a color in the <CTRL-R>
menu, drawing the unit and deleting the color.

I had to leave a gap between the tank tread
and body because of how it is rendered when rotated.
*/




$SP,"<1>",1$


$SP,"<2>",2$


//This is an infantry.

$SP,"<3>",3$

$SP,"<4>",4$


$SP,"<5>",5$



//Unit types
#define UT_INFANTRY	0
#define UT_ARTILLERY	1
#define UT_LT_TANK	2
#define UT_MD_TANK	3

class Unit
{
  U8 *img;
  I64 num,row,col,
     armored_attack,unarmored_attack,armor;
  I8 type,player,facing,movement,life,
     range,remaining_movement,accuracy;
  Bool visible[2],fired,infantry,indirect_fire,pad[3];
};

Unit units[2][MAX_UNITS];

// Bt(visible_unit_bitmap,p1+p0*MAX_UNITS)
U8 visible_unit_bitmap[2][MAX_UNITS*MAX_UNITS>>3];

#define PHASE_START	0
#define PHASE_PLOT	0
#define PHASE_PLOT0	0
#define PHASE_PLOT1	1
#define PHASE_MOVE	2
#define PHASE_MOVE0	2
#define PHASE_MOVE1	3
#define PHASE_FIRE	4
#define PHASE_FIRE0	4
#define PHASE_FIRE1	5
#define PHASE_END	6

I64 phase,cur_player,enemy_player,view_player,turn,cursor_row,cursor_col,alive_cnt[2],
   plot_routines[2],move_routines[2],fire_routines[2];


U0 Toward(I64 *_row,I64 *_col,I64 direction)
{
  switch (direction) {
    case 0:
      *_row-=2;
      break;
    case 1:
      if (*_row&1)
	*_col+=1;
      *_row-=1;
      break;
    case 2:
      if (*_row&1)
	*_col+=1;
      *_row+=1;
      break;
    case 3:
      *_row+=2;
      break;
    case 4:
      if (!(*_row&1))
	*_col-=1;
      *_row+=1;
      break;
    case 5:
      if (!(*_row&1))
	*_col-=1;
      *_row-=1;
      break;
  }
}

I64 FacingChg(I64 f1,I64 f2)
{
  I64 result=(f1+6-f2)%6;
  if (result>=3)
    return 6-result;
  else
    return result;
}

U0 RowCol2XY(F64 *_x,F64 *_y,I64 row,I64 col)
{
  Pt *c;
  row=LimitI64(row,0,map_rows);
  col=LimitI64(col,0,map_cols);
  c=&hex_centers[row][col];
  *_x=c->x;
  *_y=c->y;
}

U0 XY2RowCol(I64 *_row,I64 *_col,F64 x,F64 y)
{
  *_col=(x-DCOS/2)/(HEX_SIDE+DCOS);
  if (*_col&1)
    *_row=ToI64((y-DSIN)/(2*DSIN))*2+1;
  else
    *_row=ToI64(y/(2*DSIN))*2;
  *_col>>=1;
  *_row=LimitI64(*_row,0,map_rows-1);
  *_col=LimitI64(*_col,0,map_cols-1);
}

Unit *FindUnit(I64 row,I64 col)
{//Finds unit in a hexagon.
  I64 i,j;
  for (j=0;j<2;j++)
    for (i=0;i<MAX_UNITS;i++)
      if (units[j][i].life>0 &&
	  units[j][i].row==row &&
	  units[j][i].col==col)
	return &units[j][i];
  return NULL;
}

Bool CursorInWindow(CTask *task,I64 x,I64 y)
{
  if (0<=x+task->win_scroll_x<task->win_pixel_width &&
      0<=y+task->win_scroll_y<task->win_pixel_height)
    return TRUE;
  else
    return FALSE;
}

U0 UpdateCursor(CTask *task,I64 x,I64 y)
{
  if (CursorInWindow(task,x,y))
    XY2RowCol(&cursor_row,&cursor_col,x+task->horz_scroll.pos,y+task->vert_scroll.pos);
}

class LOSCtrl
{
  I64 r1,c1,r2,c2,distance;
};

Bool LOSPlot(LOSCtrl *l,I64 x,I64 y,I64 z)
{ //We got tricky and used z as the distance from the start of the line.
  I64 row,col;
  XY2RowCol(&row,&col,x,y);
  if ((row!=l->r1 || col!=l->c1) &&
      (row!=l->r2 || col!=l->c2) &&
      terrain[row][col]!=PLAINS) {
    if (terrain[l->r1][l->c1]==MOUNTAINS) {
      if (terrain[row][col]==MOUNTAINS || z>l->distance>>1)
	return FALSE;
    } else if (terrain[l->r2][l->c2]==MOUNTAINS) {
      if (terrain[row][col]==MOUNTAINS || z<=l->distance>>1)
	return FALSE;
    } else
      return FALSE;
  }
  return TRUE;
}

Bool LOS(I64 r1,I64 c1,I64 r2,I64 c2)
{
  F64 x1,y1,x2,y2;
  LOSCtrl l;
  RowCol2XY(&x1,&y1,r1,c1);
  RowCol2XY(&x2,&y2,r2,c2);
  l.r1=r1; l.c1=c1;
  l.r2=r2; l.c2=c2;
  l.distance=Sqrt(SqrI64(x1-x2)+SqrI64(y1-y2));
  return Line(&l,x1,y1,0,x2,y2,l.distance,&LOSPlot);
}

#define RV_ONE_FRIENDLY_UNIT	0
#define RV_UPDATE_FRIENDLY_UNIT	1
#define RV_FRIENDLY_UNIT_DIED	3
#define RV_ONE_ENEMY_UNIT	4
#define RV_ALL_UNITS		5

class MPCtrl1
{
  I64 mode,lo,hi;
  Unit *tempu;
};

U0 RVSetUp(I64 player)
{
  I64 i;
  Unit *ut0,*ut1;
  ut0=&units[player][0];
  ut1=&units[player^1][0];
  for (i=0;i<MAX_UNITS;i++,ut0++,ut1++) {
    LBtr(&ut1->visible[player],0);
    LBEqu(&ut0->visible[player],0,ut0->life>0);
  }
}

U0 RVMerge(I64 player)
{
  I64 i,j;
  Unit *ut1;
  U8 *dst,*src,*mask=CAlloc(MAX_UNITS>>3);
  src=&visible_unit_bitmap[player];
  for (j=0;j<MAX_UNITS;j++) { //p0
    dst=mask;
    for (i=0;i<MAX_UNITS>>3;i++) //p1
      *dst++|=*src++;
  }
  ut1=&units[player^1][0];
  for (j=0;j<MAX_UNITS;j++,ut1++)
    LBEqu(&ut1->visible[player],0,Bt(mask,j) && ut1->life>0);
  Free(mask);
}


Bool MPRecalcVisible(MPCtrl1 *job)
{
  Bool result=FALSE,seen;
  I64 i,j,row,col;
  F64 x1,y1,x2,y2,d,range;
  Unit *ut0,*ut1;
  ut0=&units[cur_player][job->lo];
  ut1=&units[enemy_player][job->lo];
  if (job->tempu) {
    row=job->tempu->row;
    col=job->tempu->col;
    range=job->tempu->range*2*HEX_RADIUS;
    range*=range;
  }
  switch (job->mode) {
    case RV_UPDATE_FRIENDLY_UNIT:
    case RV_ONE_FRIENDLY_UNIT:
      if (job->mode==RV_UPDATE_FRIENDLY_UNIT)
	range=MAX_F64;
      RowCol2XY(&x1,&y1,row,col);
      for (i=job->lo;i<job->hi;i++,ut1++) {
	seen=FALSE;
	if (ut1->life>0 &&
	    LOS(row,col,ut1->row,ut1->col)) {
	    RowCol2XY(&x2,&y2,ut1->row,ut1->col);
	  d=Sqr(x2-x1)+Sqr(y2-y1);
	  if (d<range) {
	    seen=TRUE;
	    LBts(&ut1->visible[cur_player],0);
	  }
	}
	if (job->mode==RV_UPDATE_FRIENDLY_UNIT)
	  LBEqu(&visible_unit_bitmap[cur_player],i+job->tempu->num*MAX_UNITS,seen);
      }
      break;
    case RV_ONE_ENEMY_UNIT:
      RowCol2XY(&x1,&y1,row,col);
      for (i=job->lo;i<job->hi;i++,ut1++)
	if (ut1->life>0 &&
	    LOS(row,col,ut1->row,ut1->col)) {
	  LBts(&visible_unit_bitmap[enemy_player],job->tempu->num+i*MAX_UNITS);
	  result=TRUE;
	} else
	  LBtr(&visible_unit_bitmap[enemy_player],job->tempu->num+i*MAX_UNITS);
      break;
    case RV_ALL_UNITS:
      ut0=&units[cur_player][0];
      for (i=0;i<MAX_UNITS;i++,ut0++)
	if (ut0->life>0) {
	  RowCol2XY(&x1,&y1,ut0->row,ut0->col);
	  ut1=&units[enemy_player][job->lo];
	  for (j=job->lo;j<job->hi;j++,ut1++) {
	    if (ut1->life>0 &&
	      LOS(ut0->row,ut0->col,ut1->row,ut1->col)) {
	      LBts(&ut1->visible[cur_player],0);
	      LBts(&visible_unit_bitmap[cur_player],j+i*MAX_UNITS);
	    } else
	      LBtr(&visible_unit_bitmap[cur_player],j+i*MAX_UNITS);
	  }
	} else
	  for (j=job->lo;j<job->hi;j++)
	    LBtr(&visible_unit_bitmap[cur_player],j+i*MAX_UNITS);
      ut0=&units[enemy_player][0];
      for (i=0;i<MAX_UNITS;i++,ut0++)
	if (ut0->life>0) {
	  RowCol2XY(&x1,&y1,ut0->row,ut0->col);
	  ut1=&units[cur_player][job->lo];
	  for (j=job->lo;j<job->hi;j++,ut1++) {
	    if (ut1->life>0 &&
	      LOS(ut0->row,ut0->col,ut1->row,ut1->col)) {
	      LBts(&ut1->visible[enemy_player],0);
	      LBts(&visible_unit_bitmap[enemy_player],j+i*MAX_UNITS);
	    } else
	      LBtr(&visible_unit_bitmap[enemy_player],j+i*MAX_UNITS);
	  }
	} else
	  for (j=job->lo;j<job->hi;j++)
	    LBtr(&visible_unit_bitmap[enemy_player],j+i*MAX_UNITS);
      break;
  }
  return result;
}

Bool RecalcVisible(I64 mode,Unit *tempu=NULL)
{
  I64 i,hi,k,cnt;
  Bool result;

/*The compiler doesn't go out of it's way
to know if something is constant.;-)  This
just compiles with the val at compile
time, an advantage of just-in-time over
AOT binaries.  TempleOS has a limited
stk size, so don't get in the habit.
$LK,"MAlloc","MN:MAlloc"$() would probably be the better choice.
*/
  MPCtrl1 job[mp_cnt];
  CMPCmd *cmd[mp_cnt];


  if (mode==RV_FRIENDLY_UNIT_DIED) {
    MemSet(&visible_unit_bitmap[enemy_player]+tempu->num*MAX_UNITS>>3,0,MAX_UNITS>>3);
    RVMerge(enemy_player);
    return 0; //return any value--don't care
  }

  cnt=mp_cnt; //cores
  hi=MAX_UNITS;
  if (mode==RV_ONE_ENEMY_UNIT) {
    for (hi--;hi>=0;hi--)
      if (units[enemy_player][hi].life>0)
	break;
    hi++;
  }
  k=hi;
  if (hi/mp_cnt<2)
    cnt=1;
  for (i=0;i<cnt;i++) {
    job[i].mode=mode;
    job[i].tempu=tempu;
    job[i].hi=k;
    k-=hi/cnt;
    if (k<0) k=0;
    if (i==cnt-1) k=0;
    job[i].lo=k;
  }


  for (i=cnt-1;i>0;i--)
    cmd[i]=MPJob(&MPRecalcVisible,&job[i],i,0);
  result=MPRecalcVisible(&job[0]);
  for (i=cnt-1;i>0;i--)
    if (MPJobResult(cmd[i]))
      result=TRUE;
  if (mode==RV_UPDATE_FRIENDLY_UNIT)
    RVMerge(cur_player);
  return result;
}

class MPCtrl2
{
  I64 lo,hi,row,col;
};

U0 MPRecalcVisibleMap(MPCtrl2 *job)
{
  I64 i,j;
  for (j=job->lo;j<job->hi;j++)
    for (i=0;i<map_cols;i++)
      if (LOS(job->row,job->col,j,i))
	visible_map[j][i]=TRUE;
      else
	visible_map[j][i]=FALSE;
}

U0 RecalcVisibleMap(I64 row,I64 col)
{
  I64 i,hi,k,cnt;
  MPCtrl2 job[mp_cnt];
  CMPCmd *cmd[mp_cnt];

  cnt=mp_cnt; //cores
  hi=map_rows;
  k=hi;
  if (hi/mp_cnt<2)
    cnt=1;
  for (i=0;i<cnt;i++) {
    job[i].row=row;
    job[i].col=col;
    job[i].hi=k;
    k-=hi/cnt;
    if (k<0) k=0;
    if (i==cnt-1) k=0;
    job[i].lo=k;
  }
  for (i=cnt-1;i>0;i--)
    cmd[i]=MPJob(&MPRecalcVisibleMap,&job[i],i,0);
  MPRecalcVisibleMap(&job[0]);
  for (i=cnt-1;i>0;i--)
    MPJobResult(cmd[i]);
}

I64 MoveCost(Unit *tempu,I64 r,I64 c,I64 facing)
{
  I64 result;
  if (tempu->infantry)
    result=0;
  else {
    result=FacingChg(facing,tempu->facing);
    if (result>0) result--;
  }
  if (roads[r][c] && roads[tempu->row][tempu->col])
    result+=1;
  else {
    if (tempu->infantry)
      result+=2;
    else {
      result+=movement_costs[terrain[r][c]];
      if (rivers[r][c])
	result=tempu->movement;
    }
  }
  return result;
}

I64 MoveOneHex(I64 *_row,I64 *_col,F64 x,F64 y)
{
  I64 direction,best_direction=-1,r,c;
  F64 d,best_d,x1,y1;
  RowCol2XY(&x1,&y1,*_row,*_col);
  best_d=Sqr(x1-x)+Sqr(y1-y);
  for (direction=0;direction<6;direction++) {
    r=*_row; c=*_col;
    Toward(&r,&c,direction);
    RowCol2XY(&x1,&y1,r,c);
    d=Sqr(x1-x)+Sqr(y1-y);
    if (0<=r<map_rows &&
	0<=c<map_cols &&
	d<best_d) {
      best_d=d;
      best_direction=direction;
    }
  }
  if (best_direction>=0) {
    Toward(_row,_col,best_direction);
    return best_direction;
  } else
    return -1;
}

Bool moving=FALSE;
I64 move_x,move_y;
F64 move_facing;
Unit *moving_unit;

Bool MovePlot(U0,I64 x,I64 y,I64)
{
  move_x=x; move_y=y;
  Sleep(5*ANIMATION_DELAY);
  return TRUE;
}

U0 MoveUnitAnimation(Unit *tempu,I64 r,I64 c,I64 facing)
{
  F64 x1,y1,x2,y2,f=facing*60.0*ã/180.0;
  moving_unit=tempu;
  RowCol2XY(&x1,&y1,tempu->row,tempu->col);
  move_x=x1; move_y=y1;
  moving=TRUE;
  if (tempu->infantry)
    Snd(300);
  else {
    move_facing=tempu->facing*60.0*ã/180.0;
    Snd(150);
    while (Wrap(f-move_facing,-ã)<=0) {
      move_facing-=0.03;
      Sleep(5*ANIMATION_DELAY);
    }
    while (Wrap(f-move_facing,-ã)>0) {
      move_facing+=0.03;
      Sleep(5*ANIMATION_DELAY);
    }
    Snd(100);
  }
  move_facing=f;
  RowCol2XY(&x2,&y2,r,c);
  Line(NULL,x1,y1,0,x2,y2,0,&MovePlot);
  Snd(0);
  moving_unit=NULL;
  moving=FALSE;
}
 
Bool MoveUnit(Unit *tempu,I64 x,I64 y)
{
  I64 r,c,r0=tempu->row,c0=tempu->col,i,facing;
  while (tempu->remaining_movement>0) {
    r=tempu->row;
    c=tempu->col;
    if ((facing=MoveOneHex(&r,&c,x,y))<0)
      break;
    else {
      i=MoveCost(tempu,r,c,facing);
      if (i>tempu->movement)
	i=tempu->movement;
      if (tempu->remaining_movement>=i && !FindUnit(r,c)) {
	MoveUnitAnimation(tempu,r,c,facing);
	tempu->facing=facing;
	tempu->remaining_movement-=i;
	tempu->row=r;
	tempu->col=c;
	RecalcVisible(RV_UPDATE_FRIENDLY_UNIT,tempu);
	LBEqu(&tempu->visible[enemy_player],0,RecalcVisible(RV_ONE_ENEMY_UNIT,tempu));
      } else
	break;
    }
  }
  if (tempu->row!=r0 || tempu->col!=c0)
    return TRUE;
  else
    return FALSE;
}

class IndirectOrders
{
  IndirectOrders *next,*last;
  Unit *attacker;
  I64 row,col;
} indirect_root;

U0 PlotShot(Unit *tempu,I64 row,I64 col)
{
  IndirectOrders *tempi;
  if (tempu->life<=0 || tempu->range<=0)
    return;
  tempu->fired=TRUE;
  tempi=CAlloc(sizeof(IndirectOrders));
  tempi->attacker=tempu;
  tempi->row=row;
  tempi->col=col;
  QueIns(tempi,indirect_root.last);
}

Bool firing=FALSE;
I64 fire_x,fire_y;
Unit *target_unit;
Bool target_hit;
Bool indirect_explosion=FALSE;
I64 indirect_row,indirect_col;

Bool FirePlot(U0,I64 x,I64 y,I64)
{
  fire_x=x; fire_y=y;
  firing=TRUE;
  Sleep(3*ANIMATION_DELAY);
  return TRUE;
}

U0 FireShot(Unit *tempu,Unit *target)
{
  I64 r,c,facing,
    t1=terrain[tempu->row][tempu->col],
    t2=terrain[target->row][target->col];
  F64 x1,y1,x2,y2,d,a,dammage=0,range_factor;
  Bool hit;

  if (tempu->life<=0 || target->life<=0 || tempu->range<=0)
    return;
  RowCol2XY(&x1,&y1,tempu->row,tempu->col);
  RowCol2XY(&x2,&y2,target->row,target->col);

  NewLine(2);

  d=100*Rand;
  "+%5.2f  Roll\n",d;
  d+=tempu->accuracy;
  "+%2d.00  Accuracy\n",tempu->accuracy;

  range_factor=Sqrt(Sqr(x2-x1)+Sqr(y2-y1))/(tempu->range*2*DSIN);
  "-%5.2f%% of Range\n",100*range_factor;
  d-=100*range_factor;
  if (t2==TREES) {
    "-30.00  Target in trees\n";
    d-=30;
  }
  if (t1==MOUNTAINS && t2!=MOUNTAINS) {
    "+30.00  Down from mountains\n";
    d+=30;
  }
  "_______\n";
  if (d>=0) {
    "+%5.2f  Hit\n",d;
    hit=TRUE;
  } else {
    "-%5.2f  Miss\n",-d;
    hit=FALSE;
  }

  target_unit=target;
  if (hit) {
    target_hit=TRUE;
    Noise(500*ANIMATION_DELAY,100,150);
    Sleep(500*ANIMATION_DELAY);
  } else {
    target_hit=FALSE;
    Noise(1000*ANIMATION_DELAY,750,1000);
    Sleep(1000*ANIMATION_DELAY);
  }

  if (hit)
    Line(NULL,x1,y1,0,x2,y2,0,&FirePlot);
  else {
    a=ã*2*Rand;
    d=(0.5-d/100)*HEX_SIDE;
    Line(NULL,x1,y1,0,x2+d*Cos(a),y2+d*Sin(a),0,&FirePlot);
  }
  firing=FALSE;
  tempu->fired=TRUE;

  if (hit) {
    r=target->row;c=target->col;
    if ((facing=MoveOneHex(&r,&c,x1,y1))>=0)
      facing=FacingChg(facing,target->facing);
    else
      facing=0;
    dammage=200.0*Rand;
    "Raw dammage roll out of 200: %3f\n",dammage;
    if (target->armor) {
      d=target->armor/100.0*(5-facing)/5.0;
      "Armor:%d-FacingAdjustment(%z) Yields:%5.2f Defense\n",
	target->armor,facing,"Front\0Side\0Rear\0",100*d;
      if (d>=0) {
	dammage*=(tempu->armored_attack/100.0)/d;
	"Armored Attack:%d\nRaw*Attack/Defense=%5.2f Dammage\n",
	  tempu->armored_attack,dammage;
      } else
	dammage=0;
    } else {
      d=1.0-range_factor;
      if (d>0) {
	"Range:%5.2f%% --> %5.2f%% Remaining\n",
	  100*range_factor,100*d;
	dammage*=(tempu->unarmored_attack/100.0)*d;
	"Unarmored Attack:%d\nRaw*Attack*Remaining=%5.2f Dammage\n",
	  tempu->unarmored_attack,dammage;
      } else
	dammage=0;
    }
    dammage=Round(dammage);
    if (dammage>0) {
      "Life:%3d - Dammage:%3f =",target->life,dammage;
      if (dammage>=target->life) {
	"Killed\n";
	Noise(1000*ANIMATION_DELAY,1000,4000);
	Sleep(1000*ANIMATION_DELAY);
	target->life=0;
	RecalcVisible(RV_FRIENDLY_UNIT_DIED,target);
	alive_cnt[target->player]--;
      } else {
	if (target->armor) {
	  if (dammage>0.6*target->life) {
	    target->movement=0;
	    "Immobilized\n";
	  } else
	    "No Penetration\n";
	} else {
	  target->life-=dammage;
	  "%d\n",target->life;
	}
      }
    }
  }
  while (snd_freq) //see $LK,"Snd","MN:Snd"$()
    Yield;
  target_unit=NULL;
}

I64 row_offsets[7]={-1,-2,-1,1,2,1,0};
I64 col_offsets_even[7]={-1, 0, 0,0,0,-1,0};
I64 col_offsets_odd [7]={ 0, 0, 1,1,0, 0,0};

U0 IndirectFireResolve()
{
  I64 i,r,c,facing;
  F64 x1,y1,x2,y2,d,dammage=0,range_factor;
  Unit *tempu,*target;
  Bool hit;
  IndirectOrders *tempi=indirect_root.next,*tempi1;
  while (tempi!=*indirect_root) {
    tempi1=tempi->next;
    tempu=tempi->attacker;
    RowCol2XY(&x1,&y1,tempu->row,tempu->col);
    RowCol2XY(&x2,&y2,tempi->row,tempi->col);

    NewLine(2);

    d=100*Rand;
    "+%5.2f  Roll\n",d;
    d+=tempu->accuracy;
    "+%2d.00  Accuracy\n",tempu->accuracy;

    range_factor=Sqrt(Sqr(x2-x1)+Sqr(y2-y1))/(tempu->range*2*DSIN);
    "-%5.2f%% of Range\n",100*range_factor;
    d-=100*range_factor;

    "_______\n";
    if (d>=0) {
      "+%5.2f  Hit\n",d;
      hit=TRUE;
    } else {
      "-%5.2f  Miss\n",-d;
      hit=FALSE;
    }

    if (hit) {
      Noise(500*ANIMATION_DELAY,100,150);
      Sleep(500*ANIMATION_DELAY);
    } else {
      Noise(1000*ANIMATION_DELAY,750,1000);
      Sleep(1000*ANIMATION_DELAY);
    }

    if (!hit) {
      i=RandU16%6;
      if (tempi->row&1)
	tempi->col+=col_offsets_odd[i];
      else
	tempi->col+=col_offsets_even[i];
      tempi->row+=row_offsets[i];
      RowCol2XY(&x2,&y2,tempi->row,tempi->col);
    }
    Line(NULL,x1,y1,0,x2,y2,0,&FirePlot);
    firing=FALSE;
    tempu->fired=TRUE;
    indirect_row=tempi->row;
    indirect_col=tempi->col;
    indirect_explosion=TRUE;
    for (i=0;i<7;i++) {
      if (tempi->row&1)
	c=tempi->col+col_offsets_odd[i];
      else
	c=tempi->col+col_offsets_even[i];
      r=tempi->row+row_offsets[i];
      if (0<=r<map_rows && 0<=c<map_cols &&
	(target=FindUnit(r,c))) {
	facing=1;
	dammage=200.0*Rand;
	"Raw dammage roll out of 200: %3f\n",dammage;
	if (target->armor) {
	  d=target->armor/100.0*(5-facing)/5.0;
	  "Armor:%d-FacingAdjustment(%z) Yields:%5.2f Defense\n",
	    target->armor,facing,"Front\0Side\0Rear\0",100*d;
	  if (d>=0) {
	    dammage*=(tempu->armored_attack/100.0)/d;
	    "Armored Attack:%d\nRaw*Attack/Defense=%5.2f Dammage\n",
	      tempu->armored_attack,dammage;
	  } else
	    dammage=0;
	} else {
	  d=1.0-range_factor;
	  if (d>0) {
	    "Range:%5.2f%% --> %5.2f%% Remaining\n",
	      100*range_factor,100*d;
	    dammage*=(tempu->unarmored_attack/100.0)*d;
	    "Unarmored Attack:%d\nRaw*Attack*Remaining=%5.2f Dammage\n",
	      tempu->unarmored_attack,dammage;
	  } else
	    dammage=0;
	}
	dammage=Round(dammage);
	if (dammage>0) {
	  "Life:%3d - Dammage:%3f =",target->life,dammage;
	  if (dammage>=target->life) {
	    "Killed\n";
	    target->life=0;
	    RecalcVisible(RV_FRIENDLY_UNIT_DIED,target);
	    alive_cnt[target->player]--;
	  } else {
	    if (target->armor) {
	      if (dammage>0.6*target->life) {
		target->movement=0;
		"Immobilized\n";
	      } else
		"No Penetration\n";
	    } else {
	      target->life-=dammage;
	      "%d\n",target->life;
	    }
	  }
	}
      }
    }
    Noise(2000*ANIMATION_DELAY,800,1000);
    Sleep(2000*ANIMATION_DELAY);
    while (snd_freq) //see $LK,"Snd","MN:Snd"$()
      Yield;
    indirect_explosion=FALSE;

    QueRem(tempi);
    Free(tempi);
    tempi=tempi1;
  }
}

U0 DrawHexes()
{
  F64 dx=2*HEX_SIDE+2*DCOS,dy=2*DSIN,
      x,y,x1,y1,x2,y2;
  I64 i,j;
  map->color=WHITE;
  GrRect(map,0,0,map->width,map->height);
  map->color=BLACK;
  y=0;
  for (j=0;j<map_rows;j+=2) {
    x=DCOS;
    GrLine(map,x,y,x-DCOS,y+DSIN);
    GrLine(map,x-DCOS,y+DSIN,x,y+2*DSIN);
    for (i=0;i<map_cols;i++) {
      x1=x; y1=y;
      x2=x1+HEX_SIDE; y2=y1;
      GrLine(map,x1,y1,x2,y2);
      x1=x2; y1=y2;
      x2+=DCOS; y2+=DSIN;
      GrLine(map,x1,y1,x2,y2);
      GrLine(map,x2,y2,x2-DCOS,y2+DSIN);
      x1=x2; y1=y2;
      x2+=HEX_SIDE;
      GrLine(map,x1,y1,x2,y2);
      GrLine(map,x2,y2,x2+DCOS,y2+DSIN);
      x1=x2; y1=y2;
      x2+=DCOS; y2-=DSIN;
      if (j || i<map_cols-1)
	GrLine(map,x1,y1,x2,y2);
      x+=dx;
    }
    y+=dy;
  }
  x=DCOS;
  for (i=0;i<map_cols;i++) {
    x1=x; y1=y;
    x2=x1+HEX_SIDE; y2=y1;
    GrLine(map,x1,y1,x2,y2);
    x1=x2; y1=y2;
    x2+=DCOS; y2+=DSIN;
    GrLine(map,x1,y1,x2,y2);
    x1=x2; y1=y2;
    x2+=HEX_SIDE;
    GrLine(map,x1,y1,x2,y2);
    x1=x2; y1=y2;
    x2+=DCOS; y2-=DSIN;
    GrLine(map,x1,y1,x2,y2);
    x+=dx;
  }
}

U0 MakeTerrain(U8 color,I64 cnt,I64 cluster_lo,I64 cluster_hi)
{
  I64 i,j,l,row,col;
  for (i=0;i<cnt;i++) {
    col=RandU32%map_cols;
    row=RandU32%map_rows;
    l=cluster_lo+RandU16%(cluster_hi-cluster_lo+1);
    for (j=0;j<l;j++) {
      terrain[row][col]=color;
      Toward(&row,&col,RandU16%6);
      col=LimitI64(col,0,map_cols-1);
      row=LimitI64(row,0,map_rows-1);
    }
  }
}

U0 MakeRivers()
{
  I64 i,row,col,direction;
  for (i=0;i<4;i++) {
    row=RandU32%map_rows;
    col=RandU32%map_cols;
    direction=RandU16%6;
    while (TRUE) {
      rivers[row][col]=TRUE;
      Toward(&row,&col,direction);
      if (!(0<=row<map_rows && 0<=col<map_cols))
	break;
      if (!(RandU16&3))
	direction=(direction+(7-RandU16%3))%6;
    }
  }
}

U0 MakeRoads()
{
  I64 i,row,col,direction;
  for (i=0;i<5;i++) {
    row=RandU32%map_rows;
    col=RandU32%map_cols;
    direction=RandU16%6;
    while (TRUE) {
      roads[row][col]=TRUE;
      Toward(&row,&col,direction);
      if (!(0<=row<map_rows && 0<=col<map_cols))
	break;
      if (!(RandU16%3))
	direction=(direction+(7-RandU16%3))%6;
    }
  }
}

U0 DrawTerrain()
{
  I64 i,j;
  F64 x,y;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      map->color=terrain[j][i];
      RowCol2XY(&x,&y,j,i);
      GrFloodFill(map,x,y);
    }
}

U0 DrawRivers()
{
  I64 i,j,k,r,c;
  F64 x1,y1,x2,y2;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      if (rivers[j][i]) {
	RowCol2XY(&x1,&y1,j,i);
	for (k=0;k<6;k++) {
	  r=j;c=i;
	  Toward(&r,&c,k);
	  if (0<=r<map_rows && 0<=c<map_cols &&
	    rivers[r][c]) {
	    RowCol2XY(&x2,&y2,r,c);
	    map->color=LTBLUE;
	    map->pen_width=4;
	    GrLine3(map,x1,y1,0,x2,y2,0);
	    map->color=BLUE;
	    map->pen_width=2;
	    GrLine3(map,x1,y1,0,x2,y2,0);
	  }
	}
      }
    }
}

U0 DrawRoads()
{
  I64 i,j,k,r,c;
  F64 x1,y1,x2,y2;
  map->color=RED;
  map->pen_width=3;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      if (roads[j][i]) {
	RowCol2XY(&x1,&y1,j,i);
	for (k=0;k<6;k++) {
	  r=j;c=i;
	  Toward(&r,&c,k);
	  if (0<=r<map_rows && 0<=c<map_cols &&
	    roads[r][c]) {
	    RowCol2XY(&x2,&y2,r,c);
	    GrLine3(map,x1,y1,0,x2,y2,0);
	  }
	}
      }
    }
}


U0 CalcHexCenters()
{
  I64 i,j;
  F64 x,y;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      x=(2*HEX_SIDE+2*DCOS)*i+HEX_SIDE/2+DCOS;
      if (j&1)
	x+=HEX_SIDE+DCOS;
      y=DSIN*(j+1);
      hex_centers[j][i].x=x;
      hex_centers[j][i].y=y;
    }
}

U0 DrawDots()
{
  I64 i,j;
  F64 x,y;
  map->color=BLACK;
  for (j=0;j<map_rows;j++)
    for (i=0;i<map_cols;i++) {
      RowCol2XY(&x,&y,j,i);
      GrPlot(map,x,y);
    }
}

U0 InitMap()
{
  CalcHexCenters;
  DrawHexes;
  MemSet(terrain,PLAINS,sizeof(terrain));
  MemSet(roads,FALSE,sizeof(roads));
  MemSet(rivers,FALSE,sizeof(rivers));
  MemSet(visible_map,FALSE,sizeof(visible_map));
  MakeTerrain(MOUNTAINS,0.03*map_cols*map_cols,5,35);
  MakeTerrain(TREES,0.03*map_cols*map_cols,5,35);
  DrawTerrain;
  MakeRivers;
  DrawRivers;
  MakeRoads;
  DrawRoads;
  DrawDots;
}

U0 InitUnits()
{
  I64 i,j,row,col,type;
  Unit *tempu;
  MemSet(units,0,sizeof(units));
  alive_cnt[0]=Round(MAX_UNITS*P1_TO_P2_ODDS);
  alive_cnt[1]=MAX_UNITS;
  for (j=0;j<2;j++)
    for (i=0;i<alive_cnt[j];i++) {
      tempu=&units[j][i];
      tempu->player=j;
      tempu->num=i;
      tempu->life=100;
      tempu->facing=RandU16%6;
      if (!j) {
	if (i>=Round(MAX_UNITS*P1_TO_P2_ODDS*PLAYER_1_ARMOR_PERCENT/100.0)) {
	  if (Round(i-MAX_UNITS*P1_TO_P2_ODDS*PLAYER_1_ARMOR_PERCENT/100.0)>=Round(0.85*(MAX_UNITS-MAX_UNITS*P1_TO_P2_ODDS*PLAYER_1_ARMOR_PERCENT/100.0)))
	    type=UT_ARTILLERY;
	  else
	    type=UT_INFANTRY;
	} else {
	  if (i>=Round(0.5*MAX_UNITS*P1_TO_P2_ODDS*PLAYER_1_ARMOR_PERCENT/100.0))
	    type=UT_MD_TANK;
	  else
	    type=UT_LT_TANK;
	}
      } else {
	if (i>=Round(MAX_UNITS*PLAYER_2_ARMOR_PERCENT/100.0)) {
	  if (Round(i-MAX_UNITS*PLAYER_2_ARMOR_PERCENT/100.0)>=Round(0.85*(MAX_UNITS-MAX_UNITS*PLAYER_2_ARMOR_PERCENT/100.0)))
	    type=UT_ARTILLERY;
	  else
	    type=UT_INFANTRY;
	} else {
	  if (i>=Round(0.5*MAX_UNITS*PLAYER_2_ARMOR_PERCENT/100.0))
	    type=UT_MD_TANK;
	  else
	    type=UT_LT_TANK;
	}
      }
      tempu->type=type;
      switch (type) {
	case UT_INFANTRY:
	  tempu->infantry=TRUE;
	  tempu->indirect_fire=FALSE;
	  tempu->armor	 =0;
	  tempu->armored_attack  =15;
	  tempu->unarmored_attack=180;
	  tempu->accuracy=45;
	  tempu->range	 =5;
	  tempu->movement=4;
	  tempu->img	 =$IB,"<3>",3$;
	  break;
	case UT_ARTILLERY:
	  tempu->infantry=TRUE;
	  tempu->indirect_fire=TRUE;
	  tempu->armor	 =0;
	  tempu->armored_attack  =75;
	  tempu->unarmored_attack=180;
	  tempu->accuracy=25;
	  tempu->range	 =20;
	  tempu->movement=2;
	  tempu->img	 =$IB,"<4>",4$;
	  break;
	case UT_LT_TANK:
	  tempu->infantry=FALSE;
	  tempu->indirect_fire=FALSE;
	  tempu->armor	 =30;
	  tempu->armored_attack  =40;
	  tempu->unarmored_attack=60;
	  tempu->accuracy=25;
	  tempu->range	 =8;
	  tempu->movement=24;
	  tempu->img	 =$IB,"<2>",2$;
	  break;
	case UT_MD_TANK:
	  tempu->infantry=FALSE;
	  tempu->indirect_fire=FALSE;
	  tempu->armor	 =60;
	  tempu->armored_attack  =60;
	  tempu->unarmored_attack=80;
	  tempu->accuracy=25;
	  tempu->range	 =12;
	  tempu->movement=16;
	  tempu->img	 =$IB,"<1>",1$;
	  break;
      }
      do {
	row=RandU32%map_rows;
	col=RandU32%(map_cols/3);
	if (j)
	  col+=2*map_cols/3;
      } while (FindUnit(row,col));
      tempu->row=row;
      tempu->col=col;
      LBts(&tempu->visible[cur_player],0);
    }
}

U0 NewTurn()
{
  I64 i,j;
  for (j=0;j<2;j++)
    for (i=0;i<MAX_UNITS;i++) {
      units[j][i].remaining_movement=units[j][i].movement;
      units[j][i].fired=FALSE;
    }
  phase=PHASE_START;
  moving_unit=NULL;

  SleepUntil(msg_off_timeout);
  msg_off_timeout=sys_jiffies+JIFFY_FREQ*2*ANIMATION_DELAY+1;
  Snd(1000);
  SPrintF(msg_buf,"Turn %d",++turn);
  RVSetUp(0);
  RVSetUp(1);
  RecalcVisible(RV_ALL_UNITS);
  cur_player=(turn&1)^1;
  enemy_player=cur_player^1;
}


U0 NewPhase()
{
  cur_player^=1;
  enemy_player=cur_player^1;
  if (++phase>=PHASE_END) {
    IndirectFireResolve;
    NewTurn;
  }
 
  if (phase&~1==PHASE_PLOT)
    Fs->border_attr=WHITE<<4+PURPLE;
  else if (phase&~1==PHASE_MOVE)
    Fs->border_attr=WHITE<<4+GREEN;
  else
    Fs->border_attr=WHITE<<4+RED;
  SleepUntil(msg_off_timeout);
  msg_off_timeout=sys_jiffies+JIFFY_FREQ*2*ANIMATION_DELAY+1;
  Snd(1000);
  switch (phase) {
    case PHASE_PLOT0:
    case PHASE_PLOT1:
      SPrintF(msg_buf,"Player %d Artillery Plot",cur_player+1);
      break;
    case PHASE_MOVE0:
    case PHASE_MOVE1:
      SPrintF(msg_buf,"Player %d Move",cur_player+1);
      break;
    case PHASE_FIRE0:
    case PHASE_FIRE1:
      SPrintF(msg_buf,"Player %d Fire",cur_player+1);
      break;
  }
}

U0 SetViewPlayer(I8 p)
{
  CMenuEntry *tempse;
  view_player=p;
  if (tempse=MenuEntryFind(Fs->cur_menu,"View/Player1"))
    tempse->checked= view_player==0;
  if (tempse=MenuEntryFind(Fs->cur_menu,"View/Player2"))
    tempse->checked= view_player==1;
}

U0 Init()
{
  moving_unit=NULL;
  InitMap;
  SetViewPlayer(cur_player=0);
  enemy_player=1;
  Fs->horz_scroll.pos=0;
  Fs->vert_scroll.pos=0;
  InitUnits;
  QueInit(&indirect_root);
  turn=0;
  fire_radius=0;
  show_visible_row=-1;
  show_visible_col=-1;
  *msg_buf=0;
  msg_off_timeout=0;
  phase=PHASE_END;
}

U0 CleanUp()
{
  QueDel(&indirect_root,TRUE);
}

#define T_TURN_OVER	0
#define T_GAME_OVER	1
#define T_NEW_GAME	2
#define T_EXIT_GAME	3

extern U0 DrawIt(CTask *task,CDC *dc);

U0 HandleChar(U8 ch)
{
  switch (ch) {
    case CH_ESC:
    case CH_SHIFT_ESC:
      if (Fs->draw_it==&DrawIt)
	throw(EXCEPT_LOCAL,T_EXIT_GAME);
      else
	Fs->draw_it=&DrawIt;
      break;
    case CH_SPACE:
      throw(EXCEPT_LOCAL,T_TURN_OVER);
    case '\n':
      throw(EXCEPT_LOCAL,T_NEW_GAME);
    case '1':
      SetViewPlayer(0);
      break;
    case '2':
      SetViewPlayer(1);
      break;
    case '3':
      if (Fs->draw_it==&DrawIt) {
	Fs->draw_it=Fs->next_settings->draw_it;
	View;
      }
      Fs->draw_it=&DrawIt;
      break;
  }
}

U0 CheckUser()
{
  U8 ch;
  if (!alive_cnt[0] || !alive_cnt[1])
    throw (EXCEPT_LOCAL,T_GAME_OVER);
  if (ch=ScanChar)
    HandleChar(ch);
}

U0 PickAI(U8 *dirname,I64 player)
{
  I64 i=0;
  U8 *st;
  CDirEntry *tempm,*tempm1,*tempm2;
  CDoc *doc=DocNew;
  Bool *old_silent=Silent(ON);
  st=MSPrintF("%s/*.CPP*",dirname);
  tempm=FilesFind(st);
  Free(st);
  tempm2=FilesFind("HOME/Tanks/*.CPP*");
  tempm1=tempm;
  Silent(old_silent);

  DocPrintF(doc,"Player %d Type\n\n",player+1);
  while (tempm1) {
    if (!(i++&3))
      DocPutS(doc,"\n");
    st=StrNew(tempm1->name);
    FileExtRem(st);
    tempm1->user_data=DocPrintF(doc,"$$MU-UL,\"%-10ts\",%d$$ ",st,tempm1);
    Free(st);
    tempm1=tempm1->next;
  }
  tempm1=tempm2;
  while (tempm1) {
    if (!(i++&3))
      DocPutS(doc,"\n");
    st=StrNew(tempm1->name);
    FileExtRem(st);
    tempm1->user_data=DocPrintF(doc,"$$MU-UL,\"%-10ts\",%d$$ ",st,tempm1);
    Free(st);
    tempm1=tempm1->next;
  }
  DocPutS(doc,"\n\n\nCreate your own AI in HOME/Tanks.");
  while ((tempm1=PopUpMenu(doc))<0);
  ExeFile(tempm1->full_name);
  DocDel(doc);
  CDirLstDel(tempm);
  CDirLstDel(tempm2);
  ExePrintF("plot_routines[%d]=&TanksPlot;move_routines[%d]=&TanksMove;fire_routines[%d]=&TanksFire;",player,player,player);
}


U0 DrawUnit(CTask *task,CDC *dc,Unit *tempu,I64 x,I64 y,F64 f)
{
  if (tempu->infantry)
    SpritePlot3(dc,x-task->horz_scroll.pos,y-task->vert_scroll.pos,0,tempu->img);
  else
    SpritePlotRotZ3b(dc,x-task->horz_scroll.pos,y-task->vert_scroll.pos,0,tempu->img,f);
}

U0 DrawUnits(CTask *task,CDC *dc)
{
  I64 i,j;
  F64 x,y;
  Unit *tempu;
  for (j=0;j<2;j++) {
    for (i=0;i<MAX_UNITS;i++) {
      tempu=&units[j][i];
      if (tempu==target_unit) {
	if (target_hit)
	  dc->color=RED;
	else
	  dc->color=GREEN;
      } else {
	if (j)
	  dc->color=LTPURPLE;
	else
	  dc->color=LTCYAN;
      }
      if (tempu->life>0 && Bt(&tempu->visible[view_player],0) && tempu!=moving_unit) {
	RowCol2XY(&x,&y,tempu->row,tempu->col);
	if (phase&~1==PHASE_MOVE && tempu->remaining_movement ||
	    (phase&~1==PHASE_PLOT&& tempu->indirect_fire||
	     phase&~1==PHASE_FIRE&&!tempu->indirect_fire) && !tempu->fired ||
	    Blink(5))
	  DrawUnit(task,dc,tempu,x,y,tempu->facing*60.0*ã/180.0);
      }
    }
  }
}

U0 DrawIt(CTask *task,CDC *dc)
{
  F64 x,y;
  I64 h,v,i,j,r,c;
  U8 buf[128];
  IndirectOrders *tempi;
  Bool old_preempt=Preempt(OFF);

  task->horz_scroll.min=0;
  task->horz_scroll.max=map_width-task->win_pixel_width;
  task->vert_scroll.min=-FONT_HEIGHT;
  task->vert_scroll.max=map_height-task->win_pixel_height;
  TaskDerivedValsUpdate(task);
  h=task->horz_scroll.pos;
  v=task->vert_scroll.pos;

  map->flags|=DCF_NO_TRANSPARENTS;
  GrBlot(dc,-h,-v,map);

  i=ipx-task->win_pixel_left-task->win_scroll_x;
  j=ipy-task->win_pixel_top -task->win_scroll_y;
  if (CursorInWindow(task,i,j))
    UpdateCursor(task,i,j);
  RowCol2XY(&x,&y,cursor_row,cursor_col);

//Roads require multiple cursor fills
  dc->color=YELLOW;
  c=terrain[cursor_row][cursor_col];
  for (i=-(HEX_SIDE+DCOS)/2;i<=(HEX_SIDE+DCOS)/2;i++) {
    if (GrPeek(dc,x+i-h,y-v)==c)
      GrFloodFill(dc,x+i-h,y-v);
    for (j=-HEX_SIDE/2;j<=HEX_SIDE/2;j++)
      if (GrPeek(dc,x+j-h,y+i-v)==c)
	GrFloodFill(dc,x+j-h,y+i-v);
  }

  DrawUnits(task,dc);
  if (firing) {
    dc->color=BLACK;
    GrCircle(dc,fire_x-h,fire_y-v,2);
  }
  if (indirect_explosion) {
    for (i=0;i<7;i++) {
      if (indirect_row&1)
	c=indirect_col+col_offsets_odd[i];
      else
	c=indirect_col+col_offsets_even[i];
      r=indirect_row+row_offsets[i];
      if (0<=r<map_rows && 0<=c<map_cols) {
	RowCol2XY(&x,&y,r,c);
	for (j=0;j<10;j++)  {
	  if (j&1)
	    dc->color=LTRED;
	  else
	    dc->color=YELLOW;
	  GrCircle(dc,x+RandU16%HEX_SIDE-HEX_SIDE/2-h,y+RandU16%HEX_SIDE-HEX_SIDE/2-v,2);
	}
      }
    }
  }
  if (moving_unit && moving_unit->visible[view_player]) {
    dc->color=YELLOW;
    DrawUnit(task,dc,moving_unit,move_x,move_y,move_facing);
  }
  ProgressBarsRst;
  if (moving_unit) {
    if (ipy<GR_HEIGHT/2) {
      progress4_max=moving_unit->movement;
      progress4=moving_unit->remaining_movement;
    } else {
      progress1_max=moving_unit->movement;
      progress1=moving_unit->remaining_movement;
    }
  }
  if (fire_radius) {
    dc->color=YELLOW;
    GrCircle(dc,fire_radius_x-h,fire_radius_y-v,fire_radius-1);
    GrCircle(dc,fire_radius_x-h,fire_radius_y-v,fire_radius+1);
    dc->color=RED;
    GrCircle(dc,fire_radius_x-h,fire_radius_y-v,fire_radius);
  }
  if (Blink(10)) {
    tempi=indirect_root.next;
    while (tempi!=&indirect_root) {
      if (tempi->attacker->player==view_player) {
	RowCol2XY(&x,&y,tempi->row,tempi->col);
	SpritePlot3(dc,x-task->horz_scroll.pos,y-task->vert_scroll.pos,0,$IB,"<5>",5$);
      }
      tempi=tempi->next;
    }
  }
  if (Bt(key_down_bitmap,SC_SHIFT)) {
    if (show_visible_row!=cursor_row || show_visible_col!=cursor_col) {
      show_visible_row=cursor_row;
      show_visible_col=cursor_col;
      RecalcVisibleMap(show_visible_row,show_visible_col);
    }

    dc->color=LTGRAY;
    for (j=0;j<map_rows;j++)
      for (i=0;i<map_cols;i++)
	if (!visible_map[j][i]) {
	  RowCol2XY(&x,&y,j,i);
	  GrLine(dc,x-6-h,y-6-v,x+6-h,y+6-v);
	  GrLine(dc,x+6-h,y-6-v,x-6-h,y+6-v);
	  GrLine(dc,x-h,y-6-v,x-h,y+6-v);
	  GrLine(dc,x+6-h,y-v,x-6-h,y-v);
	}
  }
  if (i=StrLen(msg_buf)*FONT_WIDTH) {
    dc->color=BLACK;
    GrRect(dc,(task->win_pixel_width-i)>>1-10-task->win_scroll_x,
	       (task->win_pixel_height-FONT_HEIGHT)>>1-10-task->win_scroll_y,
	       i+20,FONT_HEIGHT+20);

    dc->color=YELLOW;
    GrRect(dc,(task->win_pixel_width-i)>>1-7-task->win_scroll_x,
	       (task->win_pixel_height-FONT_HEIGHT)>>1-7-task->win_scroll_y,
	       i+14,FONT_HEIGHT+14);

    dc->color=RED;
    GrPutS(dc,(task->win_pixel_width-i)>>1-task->win_scroll_x,
		(task->win_pixel_height-FONT_HEIGHT)>>1-task->win_scroll_y,
      msg_buf);
    if (msg_off_timeout) {
      if (msg_off_timeout-sys_jiffies<3*JIFFY_FREQ/2*ANIMATION_DELAY)
	Snd(0);
      if (sys_jiffies>msg_off_timeout)
	*msg_buf=0;
    }
  }

  dc->color=WHITE;
  GrRect(dc,-task->win_scroll_x,-task->win_scroll_y,(13+7+10+10)*FONT_WIDTH,FONT_HEIGHT);

  if (phase&~1==PHASE_PLOT) {
    dc->color=PURPLE;
    SPrintF(buf,"Turn:%2d Artillery",turn);
  } else if (phase&~1==PHASE_MOVE) {
    dc->color=GREEN;
    SPrintF(buf,"Turn:%2d Move",turn);
  } else {
    dc->color=RED;
    SPrintF(buf,"Turn:%2d Fire",turn);
  }
  GrPutS(dc,-task->win_scroll_x,-task->win_scroll_y,buf);

  SPrintF(buf,"%3d Units",alive_cnt[0]);
  if (cur_player || Blink)
    dc->color=CYAN;
  else
    dc->color=LTCYAN;
  GrPutS(dc,-task->win_scroll_x+(13+7)*FONT_WIDTH,-task->win_scroll_y,buf);

  SPrintF(buf,"%3d Units",alive_cnt[1]);
  if (!cur_player || Blink)
    dc->color=PURPLE;
  else
    dc->color=LTPURPLE;
  GrPutS(dc,-task->win_scroll_x+(13+7+10)*FONT_WIDTH,-task->win_scroll_y,buf);
  Preempt(old_preempt);
}

U0 TaskEndCB()
{
  Snd(0);
  progress4=progress4_max=progress1=progress1_max=0;
  Exit;
}

U0 Tanks()
{
  I64 result,ch;
  map=DCNew(DCT_COLOR8,MAP_WIDTH,MAP_HEIGHT);

  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->win_inhibit|=WIF_DBL_CLICK|WIF_IP_L|WIF_IP_R;

  MenuPush(
"File {"
"  Abort(,CH_SHIFT_ESC);"
"  Exit(,CH_ESC);"
"}"
"Play {"
"  EndPhase(,CH_SPACE);"
"  Restart(,'\n');"
"}"
"View {"
"  Player1(,'1');"
"  Player2(,'2');"
"  ShowOdds(,'3');"
"  LOS(,0,SCF_SHIFT);"
"}"
);

  WinMax;
  WordStat;
  WinBorder(ON);
  Preempt(OFF);
  Init;
  PickAI("/TempleOS/Apps/Tanks/AIs",0);
  PickAI("/TempleOS/Apps/Tanks/AIs",1);

  PopUpOk("Left-click  to move or fire units.\n"
	"$$FG,GREEN$$<SPACE BAR>$$FG$$ or right-click to end phase.\n"
	"$$FG,GREEN$$<SHIFT>\t   $$FG$$ to show line-of-sight.\n"
	"$$FG,GREEN$$<ENTER>\t   $$FG$$ to start new game.\n"
	"$$FG,GREEN$$  1\t   $$FG$$ Player 1 view.\n"
	"$$FG,GREEN$$  2\t   $$FG$$ Player 2 view.\n"
	"$$FG,GREEN$$  3\t   $$FG$$ View odds calculations.");
  Fs->task_end_cb=&TaskEndCB; //<CTRL-ALT-X>
  DocCursor;
  DocClear;
  Fs->draw_it=&DrawIt;
  "$$FG,GREEN$$<SHIFT-ESC>$$FG$$ to return to game.\n";

  NewPhase;
  try {
    do {
      try {
	result=T_EXIT_GAME;
	if (phase&~1==PHASE_PLOT)
	  CallInd(plot_routines[cur_player]);
	else if (phase&~1==PHASE_MOVE)
	  CallInd(move_routines[cur_player]);
	else
	  CallInd(fire_routines[cur_player]);
      } catch {
	if (Fs->except_argc==2 && Fs->except_argv[0]==EXCEPT_LOCAL) {
	  Fs->catch_except=TRUE;
	  result=Fs->except_argv[1];
	}
      }
      switch (result) {
	case T_TURN_OVER:
	  NewPhase;
	  break;
	case T_GAME_OVER:
	  while (TRUE) {
	    msg_off_timeout=0;
	    StrCpy(msg_buf,"Game Over");
	    Snd(0);
	    ch=GetChar(NULL,FALSE);
	    if (ch=='\n') {
	      Init;
	      NewPhase;
	      break;
	    } else if (ch==CH_ESC || ch==CH_SHIFT_ESC) {
	      result=T_EXIT_GAME;
	      break;
	    } else if (ch=='1')
	      SetViewPlayer(0);
	    else if (ch=='2')
	      SetViewPlayer(1);
	  }
	  break;
	case T_NEW_GAME:
	  CleanUp;
	  Init;
	  NewPhase;
	  break;
      }
    } while (result!=T_EXIT_GAME);
  } catch
    CatchAll;
  ProgressBarsRst;

  SettingsPop;
  MenuPop;
  DCDel(map);
  CleanUp;
}
           L         ÿÿÿÿüÿÿÿ          úÿÿÿ      üÿÿÿúÿÿÿþÿÿÿ              øÿÿÿ                 üÿÿÿüÿÿÿ          ûÿÿÿûÿÿÿ   ûÿÿÿ   ûÿÿÿ            ûÿÿÿ   ûÿÿÿ   ûÿÿÿûÿÿÿüÿÿÿüÿÿÿ         üÿÿÿüÿÿÿ              G      ÿÿÿÿýÿÿÿ          üÿÿÿ      þÿÿÿüÿÿÿþÿÿÿ              úÿÿÿ           Ï      þÿÿÿþÿÿÿ      ÿÿÿÿýÿÿÿ   ýÿÿÿÿÿÿÿüÿÿÿ   üÿÿÿ     üÿÿÿ       ýÿÿÿ         þÿÿÿûÿÿÿ   ûÿÿÿ   ýÿÿÿ   üÿÿÿþÿÿÿýÿÿÿþÿÿÿüÿÿÿýÿÿÿýÿÿÿýÿÿÿ      ýÿÿÿ                  ÿÿÿÿ   ÿÿÿÿ              <                         þÿÿÿ       þÿÿÿ            