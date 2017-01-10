U0 MyDerivative(CMathODE *ode,F64,COrder2D3 *,COrder2D3 *)
{
//The forces due to springs and drag are
  //automatically handled by the
  //ode code.  We can add new forces
  //here.
  CTask *task=ode->win_task;
  F64 d,dd;
  CD3 p,p2;
  MyMass *tmpm1,*tmpm2;

  //Collisions
  tmpm1=ode->next_mass;
  while (tmpm1!=&ode->next_mass) {
    tmpm2=tmpm1->next;
    while (tmpm2!=&ode->next_mass) {
      D3Sub(&p,&tmpm2->state->x,&tmpm1->state->x);
      dd=D3NormSqr(&p);
      if (dd<=Sqr(tmpm1->radius+tmpm2->radius)) {
	d=Sqrt(dd)+0.0001;
	dd=10.0*Sqr(Sqr(Sqr(tmpm1->radius+tmpm2->radius)-dd));
	D3MulEqu(&p,dd/d);
	D3AddEqu(&tmpm2->DstateDt->DxDt,&p);
	D3SubEqu(&tmpm1->DstateDt->DxDt,&p);
      }
      tmpm2=tmpm2->next;
    }
    tmpm1=tmpm1->next;
  }

  tmpm1=ode->next_mass;
  while (tmpm1!=&ode->next_mass) {
    if (!(tmpm1->flags&MSF_FIXED))
      tmpm1->DstateDt->DyDt+=10.0*tmpm1->mass; //Gravity
    tmpm1=tmpm1->next;
  }

  if (cursor_mass) {
    p2.x=ms.pos.x-task->pix_left-task->scroll_x;
    p2.y=ms.pos.y-task->pix_top-task->scroll_y;
    p2.z=0;
    D3Sub(&p,&p2,&cursor_mass->state->x);
    d=10.0*D3NormSqr(&p);
    D3MulEqu(&p,d);
    D3AddEqu(&cursor_mass->DstateDt->DxDt,&p);
  }
}
