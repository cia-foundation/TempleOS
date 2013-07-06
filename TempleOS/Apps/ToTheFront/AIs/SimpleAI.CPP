Unit *FindNearestUnit(I64 row,I64 col,I64 player,Bool in_LOS,F64 range=-1)
{
  I64 i;
  F64 d,best_d=MAX_F64,x1,y1,x2,y2;
  Unit *best=NULL;
//$LK,"Sqrt","MN:Sqrt"$() is slow, so work with squared distances.
  if (range<0)
    range=MAX_F64;
  else
    range*=range;
  RowCol2XY(&x1,&y1,row,col);
  for (i=0;i<MAX_UNITS;i++)
    if (units[player][i].life>0) {
      if (!in_LOS || LOS(row,col,units[player][i].row,units[player][i].col)) {
	RowCol2XY(&x2,&y2,units[player][i].row,units[player][i].col);
	d=Sqr(x2-x1)+Sqr(y2-y1);
	if (d<=range && d<best_d) {
	  best=&units[player][i];
	  best_d=d;
	}
      }
    }
  return best;
}

U0 TanksPlot()
{
  Unit *target,*tempu;
  I64 i;
  for (i=0;i<MAX_UNITS;i++) {
    CheckUser;
    tempu=&units[cur_player][i];
    if (tempu->life>0 && tempu->indirect_fire &&
	(target=FindNearestUnit(tempu->row,tempu->col,enemy_player,TRUE,tempu->range*2*HEX_RADIUS)))
      PlotShot(tempu,target->row,target->col);
  }
  throw(EXCEPT_LOCAL,T_PHASE_OVER);
}

U0 TanksMove()
{
  Unit *target,*tempu;
  I64 i;
  F64 x,y;
  for (i=0;i<MAX_UNITS;i++) {
    CheckUser;
    tempu=&units[cur_player][i];
    if (tempu->life>0) {
      //This cheats because it violates Line-of-Sight
      if (target=FindNearestUnit(tempu->row,tempu->col,enemy_player,FALSE)) {
	RowCol2XY(&x,&y,target->row,target->col);
	if (!MoveUnit(tempu,x,y)) {
	  RowCol2XY(&x,&y,tempu->row,tempu->col);
	  MoveUnit(tempu,x+RandI16,y+RandI16);
	}
      }
    }
  }
  throw(EXCEPT_LOCAL,T_PHASE_OVER);
}

U0 TanksFire()
{
  Unit *target,*tempu;
  I64 i;
  for (i=0;i<MAX_UNITS;i++) {
    CheckUser;
    tempu=&units[cur_player][i];
    if (tempu->life>0 && !tempu->indirect_fire &&
	(target=FindNearestUnit(tempu->row,tempu->col,enemy_player,TRUE,tempu->range*2*HEX_RADIUS))) {
      FireShot(tempu,target);
      Sleep(250*ANIMATION_DELAY);
    }
  }
  throw(EXCEPT_LOCAL,T_PHASE_OVER);
}

