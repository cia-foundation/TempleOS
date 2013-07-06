public I64 Circle(U8 *aux_data,
		I64 cx,I64 cy,I64 cz,
		I64 radius,
		Bool (*fp_plot)(U8 *aux,I64 x,I64 y,I64 z),
		I64 step=1,
		F64 start_radians=0,
		F64 len_radians=2*ã)
{
  I64 i,j,len=Ceil(len_radians*radius),
      x,y,x1,y1,s1,s2,c,result=0;
  F64 t;
  if (radius<=0||!step) return 0;
  t=1.0/radius;
  c=1<<SCALE1*Cos(t);
  if (step<0) {
    step=-step;
    s2=1<<SCALE1*Sin(t);
    s1=-s2;
  } else {
    s1=1<<SCALE1*Sin(t);
    s2=-s1;
  }
  if (start_radians) {
    x=radius*Cos(start_radians);
    y=-radius*Sin(start_radians);
  } else {
    x=radius;
    y=0;
  }
  x<<=SCALE2;
  y<<=SCALE2;
  for (i=0;i<=len;i+=step) {
    if ((*fp_plot)(aux_data,cx+x>>SCALE2,cy+y>>SCALE2,cz))
      result++;
    for (j=0;j<step;j++) {
      x1=(c*x+s1*y)>>SCALE1;
      y1=(s2*x+c*y)>>SCALE1;
      x=x1; y=y1;
    }
  }
  return result;
}

U0 Main()
{
  I64 i;
  CDC *dc=DCAlias;
  for (i=0;i<100;i++)
    Circle(dc,100,100,0,i*5,&GrLinePlot,1,0,2*ã);
  PressAKey;
  DCFill;
  DCDel(dc);
}

Main;