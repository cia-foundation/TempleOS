#define VR_ONE_FRIENDLY_UNIT	0
#define VR_UPDATE_FRIENDLY_UNIT	1
#define VR_FRIENDLY_UNIT_DIED	3
#define VR_ONE_ENEMY_UNIT	4
#define VR_ALL_UNITS		5

class MPCtrl1
{
  I64 mode,lo,hi;
  Unit *tmpu;
};

class MPCtrl2
{
  I64 lo,hi,row,col;
};


U0 VRSetUp(I64 player)
{
  I64 i;
  Unit *ut0,*ut1;
  ut0=&units[player][0];
  ut1=&units[player^1][0];
  for (i=0;i<UNITS_NUM;i++,ut0++,ut1++) {
    LBtr(&ut1->vis[player],0);
    LBEqu(&ut0->vis[player],0,ut0->life>0);
  }
}

U0 VRMerge(I64 player)
{
  I64 i,j;
  Unit *ut1;
  U8 *dst,*src,*mask=CAlloc((UNITS_NUM+7)>>3);
  for (j=0;j<UNITS_NUM;j++) {//p0
    src=&vis_unit_bitmap[player][(((UNITS_NUM+7)&~7)*j)>>3];
    dst=mask;
    for (i=0;i<(UNITS_NUM+7)>>3;i++) //player1
      *dst++|=*src++;
  }
  ut1=&units[player^1][0];
  for (j=0;j<UNITS_NUM;j++,ut1++)
    LBEqu(&ut1->vis[player],0,Bt(mask,j) && ut1->life>0);
  Free(mask);
}

Bool MPVisRecalc(MPCtrl1 *job)
{
  Bool res=FALSE,seen;
  I64 i,j,row,col;
  F64 x1,y1,x2,y2,dd,range;
  Unit *ut0,*ut1;
  ut0=&units[cur_player][job->lo];
  ut1=&units[enemy_player][job->lo];
  if (job->tmpu) {
    row=job->tmpu->row;
    col=job->tmpu->col;
    range=job->tmpu->range*2*HEX_RADIUS;
    range*=range;
  }
  switch (job->mode) {
    case VR_UPDATE_FRIENDLY_UNIT:
    case VR_ONE_FRIENDLY_UNIT:
      if (job->mode==VR_UPDATE_FRIENDLY_UNIT)
	range=F64_MAX;
      RowCol2XY(&x1,&y1,row,col);
      for (i=job->lo;i<job->hi;i++,ut1++) {
	seen=FALSE;
	if (ut1->life>0 &&
	      LOS(row,col,ut1->row,ut1->col)) {
	  RowCol2XY(&x2,&y2,ut1->row,ut1->col);
	  dd=Sqr(x2-x1)+Sqr(y2-y1);
	  if (dd<range) {
	    seen=TRUE;
	    LBts(&ut1->vis[cur_player],0);
	  }
	}
	if (job->mode==VR_UPDATE_FRIENDLY_UNIT)
	  LBEqu(&vis_unit_bitmap[cur_player],
		i+job->tmpu->num*((UNITS_NUM+7)&~7),seen);
      }
      break;
    case VR_ONE_ENEMY_UNIT:
      RowCol2XY(&x1,&y1,row,col);
      for (i=job->lo;i<job->hi;i++,ut1++)
	if (ut1->life>0 &&
	      LOS(row,col,ut1->row,ut1->col)) {
	  LBts(&vis_unit_bitmap[enemy_player],
		job->tmpu->num+i*((UNITS_NUM+7)&~7));
	  res=TRUE;
	} else
	  LBtr(&vis_unit_bitmap[enemy_player],
		job->tmpu->num+i*((UNITS_NUM+7)&~7));
      break;
    case VR_ALL_UNITS:
      ut0=&units[cur_player][0];
      for (i=0;i<UNITS_NUM;i++,ut0++)
	if (ut0->life>0) {
	  RowCol2XY(&x1,&y1,ut0->row,ut0->col);
	  ut1=&units[enemy_player][job->lo];
	  for (j=job->lo;j<job->hi;j++,ut1++) {
	    if (ut1->life>0 &&
		  LOS(ut0->row,ut0->col,ut1->row,ut1->col)) {
	      LBts(&ut1->vis[cur_player],0);
	      LBts(&vis_unit_bitmap[cur_player],j+i*((UNITS_NUM+7)&~7));
	    } else
	      LBtr(&vis_unit_bitmap[cur_player],j+i*((UNITS_NUM+7)&~7));
	  }
	} else
	  for (j=job->lo;j<job->hi;j++)
	    LBtr(&vis_unit_bitmap[cur_player],j+i*((UNITS_NUM+7)&~7));
      ut0=&units[enemy_player][0];
      for (i=0;i<UNITS_NUM;i++,ut0++)
	if (ut0->life>0) {
	  RowCol2XY(&x1,&y1,ut0->row,ut0->col);
	  ut1=&units[cur_player][job->lo];
	  for (j=job->lo;j<job->hi;j++,ut1++) {
	    if (ut1->life>0 &&
		  LOS(ut0->row,ut0->col,ut1->row,ut1->col)) {
	      LBts(&ut1->vis[enemy_player],0);
	      LBts(&vis_unit_bitmap[enemy_player],j+i*((UNITS_NUM+7)&~7));
	    } else
	      LBtr(&vis_unit_bitmap[enemy_player],j+i*((UNITS_NUM+7)&~7));
	  }
	} else
	  for (j=job->lo;j<job->hi;j++)
	    LBtr(&vis_unit_bitmap[enemy_player],j+i*((UNITS_NUM+7)&~7));
      break;
  }
  return res;
}

Bool VisRecalc(I64 mode,Unit *tmpu=NULL)
{
  I64 i,hi,k,cnt;
  Bool res;
/*The compiler doesn't go out of it's way
to know if something is const.;-)  This
just compiles with the val at compile
time, an advantage of just-in-time over
AOT binaries.  TempleOS has a limited
stk size, so don't get in the habit.
$LK,"MAlloc",A="MN:MAlloc"$() would probably be the better choice.
*/
  MPCtrl1 job[mp_cnt];
  CJob *cmd[mp_cnt];

  if (mode==VR_FRIENDLY_UNIT_DIED) {
    MemSet((&vis_unit_bitmap[enemy_player])(U8 *)+
	  (tmpu->num*((UNITS_NUM+7)&~7))>>3,0,(UNITS_NUM+7)>>3);
    VRMerge(enemy_player);
    return 0; //Return any value--don't care
  }

  cnt=mp_cnt; //Cores
  hi=UNITS_NUM;
  if (mode==VR_ONE_ENEMY_UNIT) {
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
    job[i].tmpu=tmpu;
    job[i].hi=k;
    k-=hi/cnt;
    if (k<0) k=0;
    if (i==cnt-1) k=0;
    job[i].lo=k;
  }

  res=FALSE;
  for (i=0;i<cnt;i++)
    cmd[i]=JobQue(&MPVisRecalc,&job[i],i,0);
  for (i=0;i<cnt;i++)
    if (JobResGet(cmd[i]))
      res=TRUE;
  if (mode==VR_UPDATE_FRIENDLY_UNIT)
    VRMerge(cur_player);
  return res;
}

U0 MPVisRecalcMap(MPCtrl2 *job)
{
  I64 i,j;
  for (j=job->lo;j<job->hi;j++)
    for (i=0;i<map_cols;i++)
      if (LOS(job->row,job->col,j,i))
	vis_map[j][i]=TRUE;
      else
	vis_map[j][i]=FALSE;
}

U0 VisRecalcMap(I64 row,I64 col)
{
  I64 i,hi,k,cnt;
  MPCtrl2 job[mp_cnt];
  CJob *cmd[mp_cnt];

  cnt=mp_cnt; //Cores
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
  for (i=0;i<cnt;i++)
    cmd[i]=JobQue(&MPVisRecalcMap,&job[i],i,0);
  for (i=0;i<cnt;i++)
    JobResGet(cmd[i]);
}
