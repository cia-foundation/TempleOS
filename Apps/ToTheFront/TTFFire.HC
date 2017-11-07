U0 AttackHeader(Unit *tmpu,U8 *st,Unit *target)
{
  I64 i=9+StrLen(st);
  if (target) {
    i+=9;
    if (target->armor)
      i+=8;
    else
      i+=10;
  }
  '\n\n';
  "$$BLACK$$%h*c$$FG$$\n",i,'-';
  if (tmpu->player)
    "$$PURPLE$$Player 2$$FG$$ ";
  else
    "$$CYAN$$Player 1$$FG$$ ";
  "%s",st;
  if (target) {
    if (target->player)
      " $$PURPLE$$Player 2";
    else
      " $$CYAN$$Player 1";
    if (target->armor)
      " Armored";
    else
      " Unarmored";
    '$$FG$$';
  }
  '\n';
  "$$BLACK$$%h*c$$FG$$\n",i,'-';
}

F64 HitDamage(Unit *tmpu,Unit *target,I64 facing=1,F64 range_factor=0)
{
  F64 d,res=200.0*Rand;
  "\nRoll Out of 200\t\t:%6.2f Damage\n",res;
  if (target->armor) {
    d=target->armor/100.0*(5-facing)/5.0;
    if (d>=0) {
      "Armor Attack\t\t:%6.2f\n",ToF64(tmpu->armored_attack);
      res*=(tmpu->armored_attack/100.0)/d;
    } else
      res=0;
    "Armor(%z) Defense\t:%6.2f\n",facing,
	  "Front\0FrontSide\0RearSide\0Rear\0",100*d;
  } else {
    d=1.0-range_factor;
    if (d>0) {
      "Unarmored Attack\t:%6.2f\n",ToF64(tmpu->unarmored_attack);
      "Range Adjust\t\t:%6.2f%%\n",100*d;
      res*=(tmpu->unarmored_attack/100.0)*d;
    } else
      res=0;
  }
  "Attack/Defense Adjusted\t:%6.2f Damage\n",res;
  return Round(res);
}

Bool DamageDo(Unit *target,F64 damage)
{
  if (damage>0) {
    if (target->armor)
      "Armor Hit Score %6.2f\t:",damage;
    else
      "%3d Life - %3f Damage\t=",target->life,damage;
    if (damage>=target->life) {
      "$$RED$$Killed$$FG$$\n";
      Noise(1000*animation_delay,74,98);
      Sleep(1000*animation_delay);
      target->life=0;
      VisRecalc(VR_FRIENDLY_UNIT_DIED,target);
      alive_cnt[target->player]--;
      return TRUE;
    } else {
      if (target->armor) {
	if (damage>0.6*target->life) {
	  target->movement=0;
	  "$$RED$$Immobilized$$FG$$\n";
	} else
	  "$$GREEN$$No Penetration$$FG$$\n";
      } else {
	target->life-=damage;
	"$$RED$$%6.2f Life$$FG$$\n",ToF64(target->life);
      }
      return FALSE;
    }
  } else
    return FALSE;
}

U0 IndirectAdd(Unit *tmpu,I64 row,I64 col)
{
  IndirectOrders *tmpi;
  if (tmpu->life<=0 || tmpu->range<=0)
    return;
  tmpu->fired=TRUE;
  tmpi=CAlloc(sizeof(IndirectOrders));
  tmpi->attacker=tmpu;
  tmpi->row=row;
  tmpi->col=col;
  QueIns(tmpi,indirect_head.last);
}

Bool BulletPlot(U0,I64 x,I64 y,I64)
{
  fire_x=x; fire_y=y;
  firing=TRUE;
  Sleep(3*animation_delay);
  return TRUE;
}

U0 UnitDirectFire(Unit *tmpu,Unit *target)
{
  I64 r,c,facing,
	t1=terrain[tmpu->row][tmpu->col],t2=terrain[target->row][target->col];
  F64 x1,y1,x2,y2,d,a,range_factor;
  if (tmpu->life<=0 || target->life<=0 || tmpu->range<=0)
    return;
  AttackHeader(tmpu,"DirectFire",target);
  RowCol2XY(&x1,&y1,tmpu->row,tmpu->col);
  RowCol2XY(&x2,&y2,target->row,target->col);
  d=100*Rand;
  "+%5.2f  Roll\n",d;
  d+=tmpu->accuracy;
  "+%2d.00  Accuracy\n",tmpu->accuracy;

  range_factor=Sqrt(Sqr(x2-x1)+Sqr(y2-y1))/(tmpu->range*2*DSIN);
  "-%5.2f%% of Range\n",100*range_factor;
  d-=100*range_factor;
  if (t2==TREES) {
    "-30.00  Target in Trees Penalty\n";
    d-=30;
  }
  if (t1==MOUNTAINS && t2!=MOUNTAINS) {
    "+30.00  High Ground Bonus\n";
    d+=30;
  }
  "_______\n";
  target_unit=target;
  if (d>=0) {
    "+%5.2f  Hit\n",d;
    target_hit=TRUE;
    Noise(500*animation_delay,34,41);
    Sleep(500*animation_delay);
    Line(NULL,x1,y1,0,x2,y2,0,&BulletPlot);
  } else {
    "-%5.2f  Miss\n",-d;
    target_hit=FALSE;
    Noise(1000*animation_delay,69,74);
    Sleep(1000*animation_delay);
    a=ã*2*Rand;
    d=(0.5-d/100)*HEX_SIDE;
    Line(NULL,x1,y1,0,x2+d*Cos(a),y2+d*Sin(a),0,&BulletPlot);
  }
  firing=FALSE;
  tmpu->fired=TRUE;
  if (target_hit) {
    r=target->row;c=target->col;
    if ((facing=HexMoveOne(&r,&c,x1,y1))>=0)
      facing=FacingChg(facing,target->facing);
    else
      facing=0;
    DamageDo(target,HitDamage(tmpu,target,facing,range_factor));
  }
  while (scrncast.ona) //see $LK,"Snd",A="MN:Snd"$()
    Yield;
  target_unit=NULL;
}

Bool HexOccupy(Bool overrun,Unit *tmpu,Unit *target)
{
  I64 t2=terrain[target->row][target->col];
  F64 damage;
  if (tmpu->life<=0 || target->life<=0)
    return FALSE;
  if (overrun)
    AttackHeader(tmpu,"OverRun",target);
  else
    AttackHeader(tmpu,"CloseAssault",target);
  Noise(500*animation_delay,34,41);
  Sleep(500*animation_delay);
  tmpu->fired=TRUE;
  target->fired=TRUE;
  damage=HitDamage(tmpu,target);
  if (overrun) {
    damage*=2.0;
    "x2 OverRun Bonus\t=%6.2f Damage\n",damage;
    if (t2!=PLAINS) {
      damage/=2.0;
      "/2 Terrain Penalty\t=%6.2f Damage\n",damage;
    }
  } else {
    damage*=3.0;
    "x3 CloseAssault Bonus\t=%6.2f Damage\n",damage;
  }
  if (DamageDo(target,Round(damage))) {
    "$$RED$$Success$$FG$$\n";
    while (scrncast.ona) //see $LK,"Snd",A="MN:Snd"$()
      Yield;
    return TRUE;
  } else {
    tmpu->life=0;
    VisRecalc(VR_FRIENDLY_UNIT_DIED,tmpu);
    alive_cnt[tmpu->player]--;
    "$$RED$$Failure$$FG$$\n";
    while (scrncast.ona) //see $LK,"Snd",A="MN:Snd"$()
      Yield;
    return FALSE;
  }
}

U0 IndirectResolveAll()
{
  I64 i,r,c;
  F64 x1,y1,x2,y2,d,range_factor;
  Unit *tmpu,*target;
  IndirectOrders *tmpi=indirect_head.next,*tmpi1;
  while (tmpi!=*indirect_head) {
    tmpi1=tmpi->next;
    tmpu=tmpi->attacker;
    AttackHeader(tmpu,"IndirectFire",NULL);
    RowCol2XY(&x1,&y1,tmpu->row,tmpu->col);
    RowCol2XY(&x2,&y2,tmpi->row,tmpi->col);
    d=100*Rand;
    "+%5.2f  Roll\n",d;
    d+=tmpu->accuracy;
    "+%2d.00  Accuracy\n",tmpu->accuracy;
    range_factor=Sqrt(Sqr(x2-x1)+Sqr(y2-y1))/(tmpu->range*2*DSIN);
    "-%5.2f%% of Range\n",100*range_factor;
    d-=100*range_factor;
    '_______\n';


    if (d>=0) {
      "+%5.2f  Hit\n",d;
      Noise(500*animation_delay,34,41);
      Sleep(500*animation_delay);
    } else {
      "-%5.2f  Miss\n",-d;
      Noise(1000*animation_delay,69,74);
      Sleep(1000*animation_delay);
      i=RandU16%6;
      if (tmpi->row&1)
	tmpi->col+=col_offsets_odd[i];
      else
	tmpi->col+=col_offsets_even[i];
      tmpi->row+=row_offsets[i];
      RowCol2XY(&x2,&y2,tmpi->row,tmpi->col);
    }

    Line(NULL,x1,y1,0,x2,y2,0,&BulletPlot);
    firing=FALSE;
    tmpu->fired=TRUE;
    indirect_row=tmpi->row;
    indirect_col=tmpi->col;
    indirect_explosion=TRUE;
    for (i=0;i<7;i++) {
      if (tmpi->row&1)
	c=tmpi->col+col_offsets_odd[i];
      else
	c=tmpi->col+col_offsets_even[i];
      r=tmpi->row+row_offsets[i];
      if (0<=r<map_rows && 0<=c<map_cols && (target=UnitFind(r,c))) {
	AttackHeader(tmpu,"IndirectFire",target);
	DamageDo(target,HitDamage(tmpu,target));
      }
    }
    Noise(2000*animation_delay,70,74);
    Sleep(2000*animation_delay);
    while (scrncast.ona) //see $LK,"Snd",A="MN:Snd"$()
      Yield;
    indirect_explosion=FALSE;

    QueRem(tmpi);
    Free(tmpi);
    tmpi=tmpi1;
  }
}
