U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{
//The forces due to springs and drag are
  //automatically handled by the
  //ode code.  We can add new forces
  //here.
  CTask *task=ode->win_task;
  F64 d,dd;
  CD3 p,p2;
  MyMass *tempm1,*tempm2;

  //Collisions
  tempm1=ode->next_mass;
  while (tempm1!=&ode->next_mass) {
    tempm2=tempm1->next;
    while (tempm2!=&ode->next_mass) {
      D3Sub(&p,&tempm2->state->x,&tempm1->state->x);
      dd=D3NormSqr(&p);
      if (dd<=Sqr(tempm1->radius+tempm2->radius)) {
	d=Sqrt(dd)+0.0001;
	dd=10.0*Sqr(Sqr(Sqr(tempm1->radius+tempm2->radius)-dd));
	D3MulEqu(&p,dd/d);
	D3AddEqu(&tempm2->DstateDt->DxDt,&p);
	D3SubEqu(&tempm1->DstateDt->DxDt,&p);
      }
      tempm2=tempm2->next;
    }
    tempm1=tempm1->next;
  }

  tempm1=ode->next_mass;
  while (tempm1!=&ode->next_mass) {
    if (!(tempm1->flags&MSF_FIXED))
      tempm1->DstateDt->DyDt+=10.0*tempm1->mass; //Gravity
    tempm1=tempm1->next;
  }

  if (cursor_mass) {
    p2.x=ip.pos.x-task->pix_left-task->scroll_x;
    p2.y=ip.pos.y-task->pix_top-task->scroll_y;
    p2.z=0;
    D3Sub(&p,&p2,&cursor_mass->state->x);
    d=10.0*D3NormSqr(&p);
    D3MulEqu(&p,d);
    D3AddEqu(&cursor_mass->DstateDt->DxDt,&p);
  }
}
