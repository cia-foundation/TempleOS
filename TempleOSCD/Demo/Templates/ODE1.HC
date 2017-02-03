class MyMass:CMass
{
  F64 radius;
};

class MySpring:CSpring
{
};

CMathODE *ode=NULL;

U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{
  MyMass *tmpm1,*tmpm2;

  tmpm1=ode->next_mass;
  while (tmpm1!=&ode->next_mass) {
    tmpm2=tmpm1->next;
    while (tmpm2!=&ode->next_mass) {
      tmpm2=tmpm2->next;
    }
    tmpm1=tmpm1->next;
  }
}

U0 Init()
{
  ode=ODENew(0,1e-4,ODEF_HAS_MASSES);
  ode->derive=&MyDerivative;
  ode->drag_v2=0.002;
  ode->drag_v3=0.00001;
  ode->acceleration_limit=5e3;

  QueIns(ode,Fs->last_ode);
}

U0 CleanUp()
{
  QueRem(ode);
  QueDel(&ode->next_mass,TRUE);
  QueDel(&ode->next_spring,TRUE);
  ODEDel(ode);
}
