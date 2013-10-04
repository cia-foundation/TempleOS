#help_index "Math/CMathODE;CMathODE"
#help_file "::/TempleOS/Doc/ODE"

//See $LK-A,"::/TempleOS/Doc/Acknowledgements.TXT"$.

F64 LowPass1(F64 a,F64 y0,F64 y,F64 dt=1.0)
{ //First order low pass filter
  dt=Exp(-a*dt);
  return y0*dt+y*(1.0-dt);
}

U0 ODERstPtrs(CMathODE *ode)
{
  I64 s=ode->n_internal*sizeof(F64);
  F64 *ptr=ode->array_base;

  ode->state_internal=ptr;
  ptr(I64)+=s;
  ode->state_scale=ptr;
  ptr(I64)+=s;
  ode->DstateDt=ptr;
  ptr(I64)+=s;
  ode->initial_state=ptr;
  ptr(I64)+=s;
  ode->temp0=ptr;
  ptr(I64)+=s;
  ode->temp1=ptr;
  ptr(I64)+=s;
  ode->temp2=ptr;
  ptr(I64)+=s;
  ode->temp3=ptr;
  ptr(I64)+=s;
  ode->temp4=ptr;
  ptr(I64)+=s;
  ode->temp5=ptr;
  ptr(I64)+=s;
  ode->temp6=ptr;
  ptr(I64)+=s;
  ode->temp7=ptr;
}

public CMathODE *ODENew(I64 n,F64 max_tolerance=1e-6,I64 flags=0)
{
//See $LK,"flags","MN:ODEF_HAS_MASSES"$.

//The tolerance is not precise.
//You can min_tolerance and it will
//dynamically adjust tolerance to utilize
//the CPU.

  I64 s=n*sizeof(F64);
  CMathODE *ode=MAlloc(sizeof(CMathODE));

  ode->t=0;
  ode->t_scale=1.0;
  ode->base_t=0;
  ode->flags=flags;
  ode->n_internal=ode->n=n;
  ode->h=1e-6;
  ode->h_min=1e-64;
  ode->h_max=1e32;
  ode->max_tolerance=ode->min_tolerance=ode->tolerance_internal=max_tolerance;
  ode->win_task=ode->mem_task=Fs;
  ode->derivative=NULL;
  QueInit(&ode->next_mass);
  QueInit(&ode->next_spring);
  ode->acceleration_limit=ode->drag_v=ode->drag_v2=ode->drag_v3=0;

  ode->state=CAlloc(s);
  ode->array_base=MAlloc(12*s);
  ODERstPtrs(ode);
  ode->ode_signature=ODE_SIGNATURE_VAL;
  return ode;
}

public U0 ODEDel(CMathODE *ode)
{
  if (!ode) return;
  ode->ode_signature=0;
  Free(ode->state);
  Free(ode->array_base);
  Free(ode);
}

public I64 ODESize(CMathODE *ode)
{
  if (!ode)
    return 0;
  else
    return MSize2(ode->state)+MSize2(ode->array_base)+MSize2(ode);
}

U0 ODESetMassesPtrs(CMathODE *ode,F64 *state,F64 *DstateDt)
{
  COrder2D3 *ptr1=state(F64 *)+ode->n,
	   *ptr2=DstateDt(F64 *)+ode->n;
  CMass *tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    tempm->state=ptr1++;
    tempm->DstateDt=ptr2++;
    tempm=tempm->next;
  }
}

U0 ODEState2Internal(CMathODE *ode)
{
  CMass *tempm;
  F64 *old_array_base;
  I64 mass_cnt;

  if (ode->flags&ODEF_HAS_MASSES) {
    mass_cnt=0;
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      mass_cnt++;
      tempm=tempm->next;
    }
    old_array_base=ode->array_base;
    ode->n_internal=ode->n+6*mass_cnt;
    ode->array_base=MAlloc(12*ode->n_internal*sizeof(F64),ode->mem_task);
    Free(old_array_base);
    ODERstPtrs(ode);

    ODESetMassesPtrs(ode,ode->state_internal,ode->state_internal);
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      MemCpy(tempm->state,&tempm->saved_state,sizeof(COrder2D3));
      tempm=tempm->next;
    }
  }
  MemCpy(ode->state_internal,ode->state,ode->n*sizeof(F64));
}

U0 ODEInternal2State(CMathODE *ode)
{
  CMass *tempm;
  MemCpy(ode->state,ode->state_internal,ode->n*sizeof(F64));
  if (ode->flags&ODEF_HAS_MASSES) {
    ODESetMassesPtrs(ode,ode->state_internal,ode->state_internal);
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      MemCpy(&tempm->saved_state,tempm->state,sizeof(COrder2D3));
      tempm=tempm->next;
    }
  }
}

public U0 ODERenum(CMathODE *ode)
{
  I64 i;
  CSpring *temps;
  CMass *tempm;

  i=0;
  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    tempm->num=i++;
    tempm=tempm->next;
  }

  i=0;
  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    temps->num=i++;
    temps->end1_num=temps->end1->num;
    temps->end2_num=temps->end2->num;
    temps=temps->next;
  }
}

public CMass *MassFind(CMathODE *ode,F64 x,F64 y,F64 z=0)
{
  CMass *tempm,*best_mass=NULL;
  F64 d,best_d=MAX_F64;

  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    d=Sqr(tempm->x-x)+Sqr(tempm->y-y)+Sqr(tempm->z-z);
    if (d<best_d) {
      best_d=d;
      best_mass=tempm;
    }
    tempm=tempm->next;
  }
  return best_mass;
}

public CSpring *SpringFind(CMathODE *ode,F64 x,F64 y,F64 z=0)
{
  CSpring *temps,*best_spring=NULL;
  F64 d,best_d=MAX_F64;

  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    d=Sqr((temps->end1->x+temps->end2->x)/2-x)+
      Sqr((temps->end1->y+temps->end2->y)/2-y)+
      Sqr((temps->end1->z+temps->end2->z)/2-z);
    if (d<best_d) {
      best_d=d;
      best_spring=temps;
    }
    temps=temps->next;
  }
  return best_spring;
}

public U0 MassOrSpringFind(CMathODE *ode,CMass **result_mass,CSpring **result_spring,
  F64 x,F64 y,F64 z=0)
{
  CMass   *tempm,*best_mass=NULL;
  CSpring *temps,*best_spring=NULL;
  F64 d,best_d=MAX_F64;

  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    d=Sqr(tempm->x-x)+Sqr(tempm->y-y)+Sqr(tempm->z-z);
    if (d<best_d) {
      best_d=d;
      best_mass=tempm;
    }
    tempm=tempm->next;
  }

  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    d=Sqr((temps->end1->x+temps->end2->x)/2-x)+
      Sqr((temps->end1->y+temps->end2->y)/2-y)+
      Sqr((temps->end1->z+temps->end2->z)/2-z);
    if (d<best_d) {
      best_d=d;
      best_spring=temps;
      best_mass=NULL;
    }
    temps=temps->next;
  }
  if (result_mass)   *result_mass  =best_mass;
  if (result_spring) *result_spring=best_spring;
}

public CMass *MassFindNum(CMathODE *ode,I64 num)
{
  CMass *tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    if (tempm->num==num)
      return tempm;
    tempm=tempm->next;
  }
  return NULL;
}

public U0 ODERstInactive(CMathODE *ode)
{
  CMass *tempm;
  CSpring *temps;
  tempm=ode->next_mass;
  while (tempm!=&ode->next_mass) {
    tempm->flags&=~MSF_INACTIVE;
    tempm=tempm->next;
  }
  temps=ode->next_spring;
  while (temps!=&ode->next_spring) {
    temps->flags&=~SSF_INACTIVE;
    temps=temps->next;
  }
}

U0 ODECalcSprings(CMathODE *ode)
{
  CSpring *temps=ode->next_spring;
  CMass *e1,*e2;
  F64 d;
  CD3 p;
  while (temps!=&ode->next_spring) {
    if (temps->flags&SSF_INACTIVE) {
      temps->displacement=0;
      temps->f=0;
    } else {
      e1=temps->end1;
      e2=temps->end2;
      d=D3Norm(D3Sub(&e2->state->x,&e1->state->x,&p));
      temps->displacement=d-temps->rest_len;
      temps->f=temps->displacement*temps->const;
      if (temps->f>0 && temps->flags&SSF_NO_TENSION)
	temps->f=0;
      else if (temps->f<0 && temps->flags&SSF_NO_COMPRESSION)
	temps->f=0;
      if (d>0) {
	D3MulEqu(&p,temps->f/d);
	D3AddEqu(&e1->DstateDt->DxDt,&p);
	D3SubEqu(&e2->DstateDt->DxDt,&p);
      }
    }
    temps=temps->next;
  }
}

U0 ODECalcDrag(CMathODE *ode)
{
  CMass *tempm;
  F64 d,dd;
  CD3 p;
  if (ode->drag_v || ode->drag_v2 || ode->drag_v3) {
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      if (!(tempm->flags & MSF_INACTIVE) &&
	  tempm->drag_profile_factor &&
	  (dd=D3SqrNorm(&tempm->state->DxDt))) {
	d=ode->drag_v;
	if (ode->drag_v2)
	  d+=ode->drag_v2*Sqrt(dd);
	if (ode->drag_v3)
	  d+=dd*ode->drag_v3;
	D3SubEqu(&tempm->DstateDt->DxDt,
	  D3Mul(d*tempm->drag_profile_factor,&tempm->state->DxDt,&p));
      }
      tempm=tempm->next;
    }
  }
}

U0 ODEApplyAccelerationLimit(CMathODE *ode)
{
  CMass *tempm;
  F64 d;
  if (ode->acceleration_limit) {
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      if (!(tempm->flags & MSF_INACTIVE) &&
	  (d=D3Norm(&tempm->DstateDt->DxDt))>ode->acceleration_limit)
	D3MulEqu(&tempm->DstateDt->DxDt,ode->acceleration_limit/d);
      tempm=tempm->next;
    }
  }
}

U0 ODECallDerivative(CMathODE *ode,F64 t,F64 *state,F64 *DstateDt)
{
  CMass *tempm;
  if (ode->flags&ODEF_HAS_MASSES) {
    ODESetMassesPtrs(ode,state,DstateDt);
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      if (!(tempm->flags&MSF_INACTIVE)) {
	D3Zero(&tempm->DstateDt->DxDt);
	D3Copy(&tempm->DstateDt->x,&tempm->state->DxDt);
      }
      tempm=tempm->next;
    }
    ODECalcSprings(ode);
    ODECalcDrag(ode);
    (*ode->derivative)(ode,t,state,DstateDt);
    tempm=ode->next_mass;
    while (tempm!=&ode->next_mass) {
      if (!(tempm->flags&MSF_INACTIVE)) {
	if (tempm->flags&MSF_FIXED) {
	  D3Zero(&tempm->DstateDt->DxDt);
	  D3Zero(&tempm->DstateDt->x);
	} else if (tempm->mass)
	  D3DivEqu(&tempm->DstateDt->DxDt,tempm->mass);
      }
      tempm=tempm->next;
    }
    ODEApplyAccelerationLimit(ode);
  } else
    (*ode->derivative)(ode,t,state,DstateDt);
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

  dym =ode->temp0;
  dyt =ode->temp1;
  yt  =ode->temp2;
  DstateDt=ode->temp3;
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
  F64 h=ode->h,*state=ode->state_internal,*DstateDt=ode->DstateDt,*ak2,*ak3,*ak4,*ak5,*ak6,*tempstate,*stateerr,*outstate;

  ak2=ode->temp0;
  ak3=ode->temp1;
  ak4=ode->temp2;
  ak5=ode->temp3;
  ak6=ode->temp4;
  tempstate=ode->temp5;
  outstate=ode->temp6;
  stateerr=ode->temp7;

  for (i=0;i<n;i++)
    tempstate[i]=state[i]+ODEb21*h*DstateDt[i];
  ODECallDerivative(ode,ode->t+ODEa2*h,tempstate,ak2);
  for (i=0;i<n;i++)
    tempstate[i]=state[i]+h*(ODEb31*DstateDt[i]+ODEb32*ak2[i]);
  ODECallDerivative(ode,ode->t+ODEa3*h,tempstate,ak3);
  for (i=0;i<n;i++)
    tempstate[i]=state[i]+h*(ODEb41*DstateDt[i]+ODEb42*ak2[i]+ODEb43*ak3[i]);
  ODECallDerivative(ode,ode->t+ODEa4*h,tempstate,ak4);
  for (i=0;i<n;i++)
    tempstate[i]=state[i]+h*(ODEb51*DstateDt[i]+ODEb52*ak2[i]+ODEb53*ak3[i]+ODEb54*ak4[i]);
  ODECallDerivative(ode,ode->t+ODEa5*h,tempstate,ak5);
  for (i=0;i<n;i++)
    tempstate[i]=state[i]+h*(ODEb61*DstateDt[i]+ODEb62*ak2[i]+ODEb63*ak3[i]+ODEb64*ak4[i]+ODEb65*ak5[i]);
  ODECallDerivative(ode,ode->t+ODEa6*h,tempstate,ak6);
  for (i=0;i<n;i++)
    outstate[i]=state[i]+h*(ODEc1*DstateDt[i]+ODEc3*ak3[i]+ODEc4*ak4[i]+ODEc6*ak6[i]);
  for (i=0;i<n;i++)
    stateerr[i]=h*(ODEdc1*DstateDt[i]+ODEdc3*ak3[i]+ODEdc4*ak4[i]+ODEdc5*ak5[i]+ODEdc6*ak6[i]);
}


#define SAFETY 0.9
#define PGROW  (-0.2)
#define PSHRNK (-0.25)
#define ERRCON 1.89e-4


U0 ODERK5OneStep(CMathODE *ode)
{
  I64 i;
  F64 errmax,temp,*tempstate=ode->temp6,*stateerr=ode->temp7;
  while (TRUE) {
    ode->h=Limit(ode->h,ode->h_min,ode->h_max);
    ODECashKarp(ode);
    errmax=0.0;
    for (i=0;i<ode->n_internal;i++) {
      temp=Abs(stateerr[i]/ode->state_scale[i]);
      if (temp>errmax)
	errmax=temp;
    }
    errmax/=ode->tolerance_internal;
    if (errmax<=1.0 || ode->h==ode->h_min) break;
    temp=ode->h*SAFETY*errmax`PSHRNK;
    if (temp<0.1*ode->h)
      ode->h*=0.1;
    else
      ode->h=temp;
  }
  ode->t+=ode->h;
  if (errmax>ERRCON)
    ode->h*=SAFETY*errmax`PGROW;
  else
    ode->h*=5.0;
  ode->h=Limit(ode->h,ode->h_min,ode->h_max);
  MemCpy(ode->state_internal,tempstate,sizeof(F64)*ode->n_internal);
}

F64 ode_allocated_factor=0.75;

U0 ODEsUpdate(CTask *task)
{
/*
This routine is called by the $LK,"window mgr","FF:::/TempleOS/Adam/Gr/GrScreen.CPP,ODEsUpdate"$ on a continuous
basis to allow real-time simulation.  It is intended
to provide results good enough for games.  It uses a runge-kutta
integrator which is a better algorithm than doing it with Euler.

It is adaptive step-sized, so it slows down when an important
event is taking place to improve accuracy, but in my implementation
it has a timeout.

*/
  I64 i;
  F64 d,start_time,timeout_time,t_desired,t_initial,interpolation;
  CMathODE *ode;
  Bool old_suspend;

  if (task->next_ode==&task->next_ode)
    task->last_ode_time=0;
  else {
    //See $LK,"GrUpdateWins","MN:GrUpdateWins"$()
    if (!Bt(&task->win_inhibit,WIf_ODE)) {
      //TODO: Being preempted here woulds be bad
      if (task!=Fs)
	old_suspend=Suspend(task);
      //We will not pick a time limit based on
      //how busy the CPU is, what percent of the
      //last refresh cycle was spent on ODE's
      //and what the refresh cycle rate was.
      start_time=tT;
      d=1.0/win_actual_refresh;
      timeout_time=start_time+
      (task->last_ode_time/d+0.1)/(last_refresh_ode_time/d+0.1)*
      ode_allocated_factor*d;
      ode=task->next_ode;
      while (ode!=&task->next_ode) {
	if (ode->ode_signature==ODE_SIGNATURE_VAL) {
	  t_initial=ode->t;
	  d=tT;
	  if (!(ode->flags&ODEF_STARTED)) {
	    ode->base_t=d;
	    ode->flags|=ODEF_STARTED;
	  }
	  d-=ode->base_t+t_initial;
	  t_desired=ode->t_scale*d+t_initial;
	  if (ode->flags&ODEF_PAUSED)
	    ode->base_t+=t_desired-ode->t; //Slip
	  else if (ode->derivative) {
	    ODEState2Internal(ode);
	    MemCpy(ode->initial_state,ode->state_internal,ode->n_internal*sizeof(F64));
	    while (ode->t<t_desired) {
	      ode->h_max=t_desired-ode->t;
	      ODECallDerivative(ode,ode->t,ode->state_internal,ode->DstateDt);
	      for (i=0;i<ode->n_internal;i++)
		ode->state_scale[i]=Abs(ode->state_internal[i])+
		Abs(ode->DstateDt[i]*ode->h)+ode->tolerance_internal;
	      ODERK5OneStep(ode);
	      if (tT>timeout_time) {
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
	  }
	  ode->base_t+=(1.0-ode->t_scale)*d;
	}
	ode=ode->next;
      }

      //Now, we will dynamically adjust tolerances.

      //We will regulate the tolerances
      //to fill the time we decided was
      //okay to devote to ODE's.
      //Since we might have multiple ODE's
      //active we scale them by the same factor.

      //This algorithm is probably not stable or very good, but it's something.

      //Target is 75% of allocated time.
      d=(tT-start_time)/(timeout_time-start_time)-0.75;

      ode=task->next_ode;
      while (ode!=&task->next_ode) {
	if (ode->ode_signature==ODE_SIGNATURE_VAL &&
	!(ode->flags&ODEF_PAUSED) && ode->derivative) {
	  if (ode->min_tolerance!=ode->max_tolerance) {
	    if (d>0)
	      ode->tolerance_internal*=10.0`d;
	    else
	      ode->tolerance_internal*=2.0`d;
	  }
	  ode->tolerance_internal=Limit(ode->tolerance_internal,ode->min_tolerance,ode->max_tolerance);
	}
	ode=ode->next;
      }
      refresh_ode_time+=task->last_ode_time=tT-start_time;
      //See $LK,"GrUpdateWins","MN:GrUpdateWins"$()
      if (task!=Fs)
	Suspend(task,old_suspend);
    }
  }
}

#help_index ""
