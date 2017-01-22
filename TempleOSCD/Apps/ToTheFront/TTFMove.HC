U8 movement_costs[16];
movement_costs[PLAINS]=2;
movement_costs[TREES]=6;
movement_costs[MOUNTAINS]=10;

I64 HexMoveOneCost(Unit *tmpu,I64 r,I64 c,I64 facing)
{
  I64 res;
  if (tmpu->infantry)
    res=0;
  else {
    res=FacingChg(facing,tmpu->facing);
    if (res>0) res--;
  }
  if (roads[r][c] && roads[tmpu->row][tmpu->col])
    res+=1;
  else {
    if (tmpu->infantry)
      res+=2;
    else {
      res+=movement_costs[terrain[r][c]];
      if (rivers[r][c])
	res=tmpu->movement;
    }
  }
  return res;
}

I64 HexMoveOne(I64 *_row,I64 *_col,F64 x,F64 y)
{
  I64 direction,best_direction=-1,r,c;
  F64 dd,best_dd,x1,y1;
  RowCol2XY(&x1,&y1,*_row,*_col);
  best_dd=Sqr(x1-x)+Sqr(y1-y);
  for (direction=0;direction<6;direction++) {
    r=*_row; c=*_col;
    Toward(&r,&c,direction);
    RowCol2XY(&x1,&y1,r,c);
    dd=Sqr(x1-x)+Sqr(y1-y);
    if (0<=r<map_rows && 0<=c<map_cols && dd<best_dd) {
      best_dd=dd;
      best_direction=direction;
    }
  }
  if (best_direction>=0) {
    Toward(_row,_col,best_direction);
    return best_direction;
  } else
    return -1;
}

Bool UnitMovePlot(U0,I64 x,I64 y,I64)
{
  move_x=x; move_y=y;
  Sleep(5*animation_delay);
  return TRUE;
}

U0 UnitMoveAnimation(Unit *tmpu,I64 r,I64 c,I64 facing)
{
  F64 x1,y1,x2,y2,f=facing*60.0*ã/180.0;
  moving_unit=tmpu;
  RowCol2XY(&x1,&y1,tmpu->row,tmpu->col);
  move_x=x1; move_y=y1;
  moving=TRUE;
  if (tmpu->infantry)
    Snd(53);
  else {
    move_facing=tmpu->facing*60.0*ã/180.0;
    Snd(41);
    while (Wrap(f-move_facing,-ã)<=0) {
      move_facing-=0.03;
      Sleep(5*animation_delay);
    }
    while (Wrap(f-move_facing,-ã)>0) {
      move_facing+=0.03;
      Sleep(5*animation_delay);
    }
    Snd(34);
  }
  move_facing=f;
  RowCol2XY(&x2,&y2,r,c);
  Line(NULL,x1,y1,0,x2,y2,0,&UnitMovePlot);
  Snd;
  moving_unit=NULL;
  moving=FALSE;
}
 
Bool UnitMove(Unit *tmpu,I64 x,I64 y)
{
  Unit *target;
  I64 r,c,r0=tmpu->row,c0=tmpu->col,i,facing;
  while (tmpu->remaining_movement>0) {
    r=tmpu->row;
    c=tmpu->col;
    if ((facing=HexMoveOne(&r,&c,x,y))<0)
      break;
    else {
      i=HexMoveOneCost(tmpu,r,c,facing);
      if (i>tmpu->movement)
	i=tmpu->movement;
      if (!tmpu->fired && tmpu->remaining_movement>=i &&
	    tmpu->remaining_movement>=tmpu->movement>>1 &&
	    (target=UnitFind(r,c)) && target->player!=tmpu->player &&
	    tmpu->infantry!=target->infantry) {
	if (!HexOccupy(ToBool(target->infantry),tmpu,target)) {
	  tmpu=NULL;
	  break;
	}
	i=tmpu->remaining_movement;
      }
      if (tmpu->remaining_movement>=i && !UnitFind(r,c)) {
	UnitMoveAnimation(tmpu,r,c,facing);
	tmpu->facing=facing;
	tmpu->remaining_movement-=i;
	tmpu->row=r;
	tmpu->col=c;
	VisRecalc(VR_UPDATE_FRIENDLY_UNIT,tmpu);
	LBEqu(&tmpu->vis[enemy_player],0,
	      VisRecalc(VR_ONE_ENEMY_UNIT,tmpu));
      } else
	break;
    }
  }
  if (!tmpu || tmpu->row!=r0 || tmpu->col!=c0)
    return TRUE;
  else
    return FALSE;
}
