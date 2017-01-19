Unit *UnitNearestFind(I64 row,I64 col,I64 player,Bool in_LOS,F64 range=-1)
{
  I64 i;
  F64 dd,best_dd=F64_MAX,x1,y1,x2,y2;
  Unit *best=NULL;
//$LK,"Sqrt",A="MN:Sqrt"$() is slow, so work with squared distances.
  if (range<0)
    range=F64_MAX;
  else
    range*=range;
  RowCol2XY(&x1,&y1,row,col);
  for (i=0;i<UNITS_NUM;i++)
    if (units[player][i].life>0) {
      if (!in_LOS || LOS(row,col,units[player][i].row,units[player][i].col)) {
	RowCol2XY(&x2,&y2,units[player][i].row,units[player][i].col);
	dd=Sqr(x2-x1)+Sqr(y2-y1);
	if (dd<=range && dd<best_dd) {
	  best=&units[player][i];
	  best_dd=dd;
	}
      }
    }
  return best;
}

U0 PlayerIndirect()
{
  Unit *target,*tmpu;
  I64 i;
  for (i=0;i<UNITS_NUM;i++) {
    UserChk;
    tmpu=&units[cur_player][i];
    if (tmpu->life>0 && tmpu->indirect_fire &&
	  (target=UnitNearestFind(tmpu->row,tmpu->col,enemy_player,TRUE,
	  tmpu->range*2*HEX_RADIUS)))
      IndirectAdd(tmpu,target->row,target->col);
  }
  throw('PhaseOvr',TRUE);
}

U0 PlayerMove()
{
  Unit *target,*tmpu;
  I64 i;
  F64 x,y;
  for (i=0;i<UNITS_NUM;i++) {
    UserChk;
    tmpu=&units[cur_player][i];
    if (tmpu->life>0) {
//Cheats because it violates Line-of-Sight
      if (target=UnitNearestFind(tmpu->row,tmpu->col,enemy_player,FALSE)) {
	RowCol2XY(&x,&y,target->row,target->col);
	if (!UnitMove(tmpu,x,y)) {
	  RowCol2XY(&x,&y,tmpu->row,tmpu->col);
	  UnitMove(tmpu,x+RandI16,y+RandI16);
	}
      }
    }
  }
  throw('PhaseOvr',TRUE);
}

U0 PlayerDirect()
{
  Unit *target,*tmpu;
  I64 i;
  for (i=0;i<UNITS_NUM;i++) {
    UserChk;
    tmpu=&units[cur_player][i];
    if (tmpu->life>0 && !tmpu->indirect_fire &&
	  (target=UnitNearestFind(tmpu->row,tmpu->col,enemy_player,TRUE,
	  tmpu->range*2*HEX_RADIUS))) {
      UnitDirectFire(tmpu,target);
      Sleep(250*animation_delay);
    }
  }
  throw('PhaseOvr',TRUE);
}

