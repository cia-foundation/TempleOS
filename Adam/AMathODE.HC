#help_index "Math/ODE"
#help_file "::/Doc/ODE"

//See $LK,"::/Doc/Credits.DD"$.

F64 LowPass1(F64 a,F64 y0,F64 y,F64 dt=1.0)
{//First order low pass filter
  dt=Exp(-a*dt);
  return y0*dt+y*(1.0-dt);
}

U0 ODERstPtrs(CMathODE *ode)
{
  I64 s=ode->n_internal*sizeof(F64);
  F64 *ptr=ode->array_base;
  ode->state_internal=ptr;	ptr(I64)+=s;
  ode->state_scale=ptr;		ptr(I64)+=s;
  ode->DstateDt=ptr;		ptr(I64)+=s;
  ode->initial_state=ptr;	ptr(I64)+=s;
  ode->tmp0=ptr;	ptr(I64)+=s;
  ode->tmp1=ptr;	ptr(I64)+=s;
  ode->tmp2=ptr;	ptr(I64)+=s;
  ode->tmp3=ptr;	ptr(I64)+=s;
  ode->tmp4=ptr;	ptr(I64)+=s;
  ode->tmp5=ptr;	ptr(I64)+=s;
  ode->tmp6=ptr;	ptr(I64)+=s;
  ode->tmp7=ptr;
}

public CMathODE *ODENew(I64 n,F64 max_tolerance=1e-6,I64 flags=0)
{//Make differential equation ctrl struct. See $LK,"flags",A="MN:ODEF_HAS_MASSES"$.
  //The tolerance is not precise.
  //You can min_tolerance and it will
  //dynamically adjust tolerance to utilize
  //the CPU.
  I64 s=n*sizeof(F64);
  CMathODE *ode=CAlloc(sizeof(CMathODE));
  ode->t_scale=1.0;
  ode->flags=flags;
  ode->n_internal=ode->n=n;
  ode->h=1e-6;
  ode->h_min=1e-64;
  ode->h_max=1e32;
  ode->max_tolerance=ode->min_tolerance=ode->tolerance_internal=max_tolerance;
  ode->win_task=ode->mem_task=Fs;
  QueInit(&ode->next_mass);
  QueInit(&ode->next_spring);
  ode->state=CAlloc(s);
  ode->array_base=MAlloc(12*s);
  ODERstPtrs(ode);
  return ode;
}


public Bool ODEPause(CMathODE *ode,Bool val=ON)
{//Pause ODE.
  Bool res;
  if (!ode) return OFF;
  res=LBEqu(&ode->flags,ODEf_PAUSED,val);
  if (val)
    while (Bt(&ode->flags,ODEf_BUSY))
      Yield;
  return res;
}

public U0 ODEDel(CMathODE *ode)
{//Free ODE node, but not masses or springs.
  I64 i;
  if (!ode) return;
  ODEPause(ode);
  Free(ode->state);
  Free(ode->array_base);
  if (ode->slave_tasks) {
    for (i=0;i<mp_cnt;i++)
      Kill(ode->slave_tasks[i]);
    Free(ode->slave_tasks);
  }
  Free(ode);
}

public I64 ODESize(CMathODE *ode)
{//Mem size of ode ctrl, but not masses and springs.
  if (!ode)
    return 0;
  else
    return MSize2(ode->state)+MSize2(ode->array_base)+MSize2(ode);
}

U0 ODESetMassesPtrs(CMathODE *ode,F64 *state,F64 *DstateDt)
{
  COrder2D3 *ptr1=state(F64 *)+ode->n,
	*ptr2=DstateDt(F64 *)+ode->n;
  CMass *tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    tmpm->state=ptr1++;
    tmpm->DstateDt=ptr2++;
    tmpm=tmpm->next;
  }
}

U0 ODEState2Internal(CMathODE *ode)
{
  CMass *tmpm;
  F64 *old_array_base;
  I64 mass_cnt;

  if (ode->flags&ODEF_HAS_MASSES) {
    mass_cnt=0;
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      mass_cnt++;
      tmpm=tmpm->next;
    }
    old_array_base=ode->array_base;
    ode->n_internal=ode->n+6*mass_cnt;
    ode->array_base=MAlloc(12*ode->n_internal*sizeof(F64),ode->mem_task);
    Free(old_array_base);
    ODERstPtrs(ode);

    ODESetMassesPtrs(ode,ode->state_internal,ode->state_internal);
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      MemCpy(tmpm->state,&tmpm->saved_state,sizeof(COrder2D3));
      tmpm=tmpm->next;
    }
  }
  MemCpy(ode->state_internal,ode->state,ode->n*sizeof(F64));
}

U0 ODEInternal2State(CMathODE *ode)
{
  CMass *tmpm;
  MemCpy(ode->state,ode->state_internal,ode->n*sizeof(F64));
  if (ode->flags&ODEF_HAS_MASSES) {
    ODESetMassesPtrs(ode,ode->state_internal,ode->state_internal);
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      MemCpy(&tmpm->saved_state,tmpm->state,sizeof(COrder2D3));
      tmpm=tmpm->next;
    }
  }
}

public U0 ODERenum(CMathODE *ode)
{//Renumber masses and springs.
  I64 i;
  CSpring *tmps;
  CMass *tmpm;

  i=0;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    tmpm->num=i++;
    tmpm=tmpm->next;
  }

  i=0;
  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    tmps->num=i++;
    tmps->end1_num=tmps->end1->num;
    tmps->end2_num=tmps->end2->num;
    tmps=tmps->next;
  }
}

public CMass *MassFind(CMathODE *ode,F64 x,F64 y,F64 z=0)
{//Search for mass nearest to x,y,z.
  CMass *tmpm,*best_mass=NULL;
  F64 dd,best_dd=F64_MAX;

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    dd=Sqr(tmpm->x-x)+Sqr(tmpm->y-y)+Sqr(tmpm->z-z);
    if (dd<best_dd) {
      best_dd=dd;
      best_mass=tmpm;
    }
    tmpm=tmpm->next;
  }
  return best_mass;
}

public CSpring *SpringFind(CMathODE *ode,F64 x,F64 y,F64 z=0)
{//Find spring midpoint nearest x,y,z.
  CSpring *tmps,*best_spring=NULL;
  F64 dd,best_dd=F64_MAX;

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    dd=Sqr((tmps->end1->x+tmps->end2->x)/2-x)+
	  Sqr((tmps->end1->y+tmps->end2->y)/2-y)+
	  Sqr((tmps->end1->z+tmps->end2->z)/2-z);
    if (dd<best_dd) {
      best_dd=dd;
      best_spring=tmps;
    }
    tmps=tmps->next;
  }
  return best_spring;
}

public U0 MassOrSpringFind(
	CMathODE *ode,CMass **res_mass,CSpring **res_spring,
	F64 x,F64 y,F64 z=0)
{//Find spring or mass nearest x,y,z.
  CMass   *tmpm,*best_mass=NULL;
  CSpring *tmps,*best_spring=NULL;
  F64 dd,best_dd=F64_MAX;

  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    dd=Sqr(tmpm->x-x)+Sqr(tmpm->y-y)+Sqr(tmpm->z-z);
    if (dd<best_dd) {
      best_dd=dd;
      best_mass=tmpm;
    }
    tmpm=tmpm->next;
  }

  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    dd=Sqr((tmps->end1->x+tmps->end2->x)/2-x)+
	  Sqr((tmps->end1->y+tmps->end2->y)/2-y)+
	  Sqr((tmps->end1->z+tmps->end2->z)/2-z);
    if (dd<best_dd) {
      best_dd=dd;
      best_spring=tmps;
      best_mass=NULL;
    }
    tmps=tmps->next;
  }
  if (res_mass)   *res_mass  =best_mass;
  if (res_spring) *res_spring=best_spring;
}

public CMass *MassFindNum(CMathODE *ode,I64 num)
{//Return mass number N.
  CMass *tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    if (tmpm->num==num)
      return tmpm;
    tmpm=tmpm->next;
  }
  return NULL;
}

public U0 ODERstInactive(CMathODE *ode)
{//Set all masses and springs to ACTIVE for new trial.
  CMass *tmpm;
  CSpring *tmps;
  tmpm=ode->next_mass;
  while (tmpm!=&ode->next_mass) {
    tmpm->flags&=~MSF_INACTIVE;
    tmpm=tmpm->next;
  }
  tmps=ode->next_spring;
  while (tmps!=&ode->next_spring) {
    tmps->flags&=~SSF_INACTIVE;
    tmps=tmps->next;
  }
}

U0 ODECalcSprings(CMathODE *ode)
{
  CSpring *tmps=ode->next_spring;
  CMass *e1,*e2;
  F64 d;
  CD3 p;
  while (tmps!=&ode->next_spring) {
    if (tmps->flags&SSF_INACTIVE) {
      tmps->displacement=0;
      tmps->f=0;
    } else {
      e1=tmps->end1;
      e2=tmps->end2;
      d=D3Norm(D3Sub(&p,&e2->state->x,&e1->state->x));
      tmps->displacement=d-tmps->rest_len;
      tmps->f=tmps->displacement*tmps->const;
      if (tmps->f>0 && tmps->flags&SSF_NO_TENSION)
	tmps->f=0;
      else if (tmps->f<0 && tmps->flags&SSF_NO_COMPRESSION)
	tmps->f=0;
      if (d>0) {
	D3MulEqu(&p,tmps->f/d);
	D3AddEqu(&e1->DstateDt->DxDt,&p);
	D3SubEqu(&e2->DstateDt->DxDt,&p);
      }
    }
    tmps=tmps->next;
  }
}

U0 ODECalcDrag(CMathODE *ode)
{
  CMass *tmpm;
  F64 d,dd;
  CD3 p;
  if (ode->drag_v || ode->drag_v2 || ode->drag_v3) {
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      if (!(tmpm->flags & MSF_INACTIVE) &&
	    tmpm->drag_profile_factor &&
	    (dd=D3NormSqr(&tmpm->state->DxDt))) {
	d=ode->drag_v;
	if (ode->drag_v2)
	  d+=ode->drag_v2*Sqrt(dd);
	if (ode->drag_v3)
	  d+=dd*ode->drag_v3;
	D3SubEqu(&tmpm->DstateDt->DxDt,
	      D3Mul(&p,d*tmpm->drag_profile_factor,&tmpm->state->DxDt));
      }
      tmpm=tmpm->next;
    }
  }
}

U0 ODEApplyAccelerationLimit(CMathODE *ode)
{
  CMass *tmpm;
  F64 d;
  if (ode->acceleration_limit) {
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      if (!(tmpm->flags & MSF_INACTIVE) &&
	    (d=D3Norm(&tmpm->DstateDt->DxDt))>ode->acceleration_limit)
	D3MulEqu(&tmpm->DstateDt->DxDt,ode->acceleration_limit/d);
      tmpm=tmpm->next;
    }
  }
}

U0 ODEMPTask(CMathODE *ode)
{
  while (TRUE) {
    while (!Bt(&ode->mp_not_done_flags,Gs->num))
      Yield;
    if (ode->mp_derive)
      (*ode->mp_derive)(ode,ode->mp_t,
	    Gs->num,ode->mp_state,ode->mp_DstateDt);
    LBtr(&ode->mp_not_done_flags,Gs->num);
  }
}

U0 ODEMPWake(CMathODE *ode)
{
  I64 i;
  if (!ode->slave_tasks) {
    ode->slave_tasks=CAlloc(mp_cnt*sizeof(CTask *));
    for (i=0;i<mp_cnt;i++)
      ode->slave_tasks[i]=Spawn(&ODEMPTask,ode,"ODE Slave",i);
  }
  for (i=0;i<mp_cnt;i++) {
    Suspend(ode->slave_tasks[i],FALSE);
    MPInt(I_WAKE,i);
  }
}

U0 ODEMPSleep(CMathODE *ode)
{
  I64 i;
  if (ode->slave_tasks) {
    while (ode->mp_not_done_flags)
      Yield;
    for (i=0;i<mp_cnt;i++)
      Suspend(ode->slave_tasks[i]);
  }
}

U0 ODECallMPDerivative(CMathODE *ode,F64 t,F64 *state,F64 *DstateDt)
{
  ode->mp_t=t;
  ode->mp_state=state;
  ode->mp_DstateDt=DstateDt;
  ode->mp_not_done_flags=1<<mp_cnt-1;
  do Yield;
  while (ode->mp_not_done_flags);
}

U0 ODECallDerivative(CMathODE *ode,F64 t,F64 *state,F64 *DstateDt)
{
  CMass *tmpm;
  if (ode->flags&ODEF_HAS_MASSES) {
    ODESetMassesPtrs(ode,state,DstateDt);
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      if (!(tmpm->flags&MSF_INACTIVE)) {
	D3Zero(&tmpm->DstateDt->DxDt);
	D3Copy(&tmpm->DstateDt->x,&tmpm->state->DxDt);
      }
      tmpm=tmpm->next;
    }
    ODECalcSprings(ode);
    ODECalcDrag(ode);
    if (ode->mp_derive)
      ODECallMPDerivative(ode,t,state,DstateDt);
    if (ode->derive)
      (*ode->derive)(ode,t,state,DstateDt);
    tmpm=ode->next_mass;
    while (tmpm!=&ode->next_mass) {
      if (!(tmpm->flags&MSF_INACTIVE)) {
	if (tmpm->flags&MSF_FIXED) {
	  D3Zero(&tmpm->DstateDt->DxDt);
	  D3Zero(&tmpm->DstateDt->x);
	} else if (tmpm->mass)
	  D3DivEqu(&tmpm->DstateDt->DxDt,tmpm->mass);
      }
      tmpm=tmpm->next;
    }
    ODEApplyAccelerationLimit(ode);
  } else {
    if (ode->mp_derive)
      ODECallMPDerivative(ode,t,state,DstateDt);
    if (ode->derive)
      (*ode->derive)(ode,t,state,DstateDt);
  }
}

U0 ODEOneStep(CMathODE *ode)
{
  I64 i;
  ODECallDerivative(ode,ode->t,ode->state_internal,ode->DstateDt);
  for (i=0;i<ode->n_internal;i++)
    ode->state_internal[i]+=ode->h*ode->DstateDt[i];
  ode->t+=ode->h;
}

U0 ODERK4OneStep(CMathODE *ode)
{
  I64 i,n=ode->n_internal;
  F64 xh,hh,h6,*dym,*dyt,*yt,*DstateDt;

  dym =ode->tmp0;
  dyt =ode->tmp1;
  yt  =ode->tmp2;
  DstateDt=ode->tmp3;
  hh  =0.5*ode->h;
  h6  =ode->h / 6.0;
  xh  =ode->t + hh;

  ODECallDerivative(ode,ode->t,ode->state_internal,ode->DstateDt);
  for (i=0;i<n;i++)
    yt[i]=ode->state_internal[i]+hh*DstateDt[i];
  ODECallDerivative(ode,xh,yt,dyt);
  for (i=0;i<n;i++)
    yt[i]=ode->state_internal[i]+hh*dyt[i];
  ODECallDerivative(ode,xh,yt,dym);
  for (i=0;i<n;i++) {
    yt[i]=ode->state_internal[i]+ode->h*dym[i];
    dym[i]+=dyt[i];
  }
  ode->t+=ode->h;
  ODECallDerivative(ode,ode->t,yt,dyt);
  for (i=0;i<n;i++)
    ode->state_internal[i]+=h6*(DstateDt[i]+dyt[i]+2.0*dym[i]);
}

#define ODEa2 0.2
#define ODEa3 0.3
#define ODEa4 0.6
#define ODEa5 1.0
#define ODEa6 0.875
#define ODEb21 0.2
#define ODEb31 (3.0/40.0)
#define ODEb32 (9.0/40.0)
#define ODEb41 0.3
#define ODEb42 (-0.9)
#define ODEb43 1.2
#define ODEb51 (-11.0/54.0)
#define ODEb52 2.5
#define ODEb53 (-70.0/27.0)
#define ODEb54 (35.0/27.0)
#define ODEb61 (1631.0/55296.0)
#define ODEb62 (175.0/512.0)
#define ODEb63 (575.0/13824.0)
#define ODEb64 (44275.0/110592.0)
#define ODEb65 (253.0/4096.0)
#define ODEc1  (37.0/378.0)
#define ODEc3  (250.0/621.0)
#define ODEc4  (125.0/594.0)
#define ODEc6  (512.0/1771.0)
#define ODEdc1 (37.0/378.0-2825.0/27648.0)
#define ODEdc3 (250.0/621.0-18575.0/48384.0)
#define ODEdc4 (125.0/594.0-13525.0/55296.0)
#define ODEdc5 (-277.0/14336.0)
#define ODEdc6 (512.0/1771.0-0.25)

U0 ODECashKarp(CMathODE *ode)
{
  I64 i,n=ode->n_internal;
  F64 h=ode->h,*state=ode->state_internal,
	*DstateDt=ode->DstateDt,*ak2,*ak3,*ak4,*ak5,*ak6,
	*tmpstate,*stateerr,*outstate;

  ak2=ode->tmp0;
  ak3=ode->tmp1;
  ak4=ode->tmp2;
  ak5=ode->tmp3;
  ak6=ode->tmp4;
  tmpstate=ode->tmp5;
  outstate=ode->tmp6;
  stateerr=ode->tmp7;

  for (i=0;i<n;i++)
    tmpstate[i]=state[i]+ODEb21*h*DstateDt[i];
  ODECallDerivative(ode,ode->t+ODEa2*h,tmpstate,ak2);
  for (i=0;i<n;i++)
    tmpstate[i]=state[i]+h*(ODEb31*DstateDt[i]+ODEb32*ak2[i]);
  ODECallDerivative(ode,ode->t+ODEa3*h,tmpstate,ak3);
  for (i=0;i<n;i++)
    tmpstate[i]=state[i]+h*(ODEb41*DstateDt[i]+ODEb42*ak2[i]+ODEb43*ak3[i]);
  ODECallDerivative(ode,ode->t+ODEa4*h,tmpstate,ak4);
  for (i=0;i<n;i++)
    tmpstate[i]=state[i]+h*(ODEb51*DstateDt[i]+
	  ODEb52*ak2[i]+ODEb53*ak3[i]+ODEb54*ak4[i]);
  ODECallDerivative(ode,ode->t+ODEa5*h,tmpstate,ak5);
  for (i=0;i<n;i++)
    tmpstate[i]=state[i]+h*(ODEb61*DstateDt[i]+
	  ODEb62*ak2[i]+ODEb63*ak3[i]+ODEb64*ak4[i]+ODEb65*ak5[i]);
  ODECallDerivative(ode,ode->t+ODEa6*h,tmpstate,ak6);

  for (i=0;i<n;i++)
    outstate[i]=state[i]+h*(ODEc1*DstateDt[i]+
	  ODEc3*ak3[i]+ODEc4*ak4[i]+ODEc6*ak6[i]);
  for (i=0;i<n;i++)
    stateerr[i]=h*(ODEdc1*DstateDt[i]+ODEdc3*ak3[i]+
	  ODEdc4*ak4[i]+ODEdc5*ak5[i]+ODEdc6*ak6[i]);
}

#define SAFETY 0.9
#define PGROW  (-0.2)
#define PSHRNK (-0.25)
#define ERRCON 1.89e-4

U0 ODERK5OneStep(CMathODE *ode)
{
  I64 i;
  F64 errmax,tmp,*tmpstate=ode->tmp6,*stateerr=ode->tmp7;
  while (TRUE) {
    ode->h=Clamp(ode->h,ode->h_min,ode->h_max);
    ODECashKarp(ode);
    errmax=0.0;
    for (i=0;i<ode->n_internal;i++) {
      tmp=Abs(stateerr[i]/ode->state_scale[i]);
      if (tmp>errmax)
	errmax=tmp;
    }
    errmax/=ode->tolerance_internal;
    if (errmax<=1.0 || ode->h==ode->h_min) break;
    tmp=ode->h*SAFETY*errmax`PSHRNK;
    if (tmp<0.1*ode->h)
      ode->h*=0.1;
    else
      ode->h=tmp;
  }
  ode->t+=ode->h;
  if (errmax>ERRCON)
    ode->h*=SAFETY*errmax`PGROW;
  else
    ode->h*=5.0;
  ode->h=Clamp(ode->h,ode->h_min,ode->h_max);
  MemCpy(ode->state_internal,tmpstate,sizeof(F64)*ode->n_internal);
}

F64 ode_alloced_factor=0.75;

U0 ODEsUpdate(CTask *task)
{/* This routine is called by the $LK,"window mgr",A="FF:::/Adam/Gr/GrScrn.HC,ODEsUpdate"$on a continuous
basis to allow real-time simulation.  It is intended
to provide ress good enough for games.  It uses a runge-kutta
integrator which is a better algorithm than doing it with Euler.

It is adaptive step-sized, so it slows down when an important
event is taking place to improve accuracy, but in my implementation
it has a timeout.
*/
  I64 i;
  F64 d,start_time,timeout_time,t_desired,t_initial,interpolation;
  CMathODE *ode;

  if (task->next_ode==&task->next_ode)
    task->last_ode_time=0;
  else if (!Bt(&task->win_inhibit,WIf_SELF_ODE)) {
//See $LK,"GrUpdateTasks",A="MN:GrUpdateTasks"$() and $LK,"GrUpdateTaskODEs",A="MN:GrUpdateTaskODEs"$().
    //We will not pick a time limit based on
    //how busy the CPU is, what percent of the
    //last refresh cycle was spent on ODE's
    //and what the refresh cycle rate was.
    start_time=tS;
    d=1.0/winmgr.fps;
    timeout_time=start_time+
	  (task->last_ode_time/d+0.1)/(winmgr.last_ode_time/d+0.1)*
	  ode_alloced_factor*d;
    ode=task->next_ode;
    while (ode!=&task->next_ode) {
      t_initial=ode->t;
      d=tS;
      if (!(ode->flags&ODEF_STARTED)) {
	ode->base_t=d;
	ode->flags|=ODEF_STARTED;
      }
      d-=ode->base_t+t_initial;
      t_desired=ode->t_scale*d+t_initial;
      if (ode->flags&ODEF_PAUSED)
	ode->base_t+=t_desired-ode->t; //Slip
      else {
	ode->flags|=ODEF_BUSY;
	if (ode->flags&ODEF_PAUSED)
	  ode->base_t+=t_desired-ode->t; //Slip
	else {
	  if (ode->derive || ode->mp_derive) {
	    if (ode->mp_derive)
	      ODEMPWake(ode);
	    ODEState2Internal(ode);
	    MemCpy(ode->initial_state,ode->state_internal,
		  ode->n_internal*sizeof(F64));
	    while (ode->t<t_desired) {
	      ode->h_max=t_desired-ode->t;
	      ODECallDerivative(ode,ode->t,ode->state_internal,ode->DstateDt);
	      for (i=0;i<ode->n_internal;i++)
		ode->state_scale[i]=Abs(ode->state_internal[i])+
		      Abs(ode->DstateDt[i]*ode->h)+ode->tolerance_internal;
	      ODERK5OneStep(ode);
	      if (tS>timeout_time) {
		ode->base_t+=t_desired-ode->t; //Slip
		goto ode_done;

	      }
	    }

	    //Interpolate if end time was not exact.
	    if (ode->t!=t_desired) {
	      if (interpolation=ode->t-t_initial) {
		interpolation=(t_desired-t_initial)/interpolation;
		if (interpolation!=1.0)
		  for (i=0;i<ode->n_internal;i++)
		    ode->state_internal[i]=(ode->state_internal[i]-
			  ode->initial_state[i])*interpolation+
			  ode->initial_state[i];
	      }
	      ode->t=t_desired;
	    }
ode_done:
	    ODEInternal2State(ode);

	    //Convenience call to set vals
	    ODECallDerivative(ode,ode->t,ode->state_internal,ode->DstateDt);

	    if (ode->mp_derive)
	      ODEMPSleep(ode);
	  }
	}
	ode->flags&=~ODEF_BUSY;
      }
      ode->base_t+=(1.0-ode->t_scale)*d;
      ode=ode->next;
    }

    //Now, we will dynamically adjust tolerances.

    //We will regulate the tolerances
    //to fill the time we decided was
    //okay to devote to ODE's.
    //Since we might have multiple ODE's
    //active we scale them by the same factor.

    //This algorithm is probably not stable or very good, but it's something.

    //Target is 75% of alloced time.
    d=(tS-start_time)/(timeout_time-start_time)-0.75;

    ode=task->next_ode;
    while (ode!=&task->next_ode) {
      if (!(ode->flags&ODEF_PAUSED) && ode->derive) {
	if (ode->min_tolerance!=ode->max_tolerance) {
	  if (d>0)
	    ode->tolerance_internal*=10.0`d;
	  else
	    ode->tolerance_internal*=2.0`d;
	}
	ode->tolerance_internal=Clamp(ode->tolerance_internal,
	      ode->min_tolerance,ode->max_tolerance);
      }
      ode=ode->next;
    }
    winmgr.ode_time+=task->last_ode_time=tS-start_time;
  }
}
