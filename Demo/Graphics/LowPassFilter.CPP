#define STEP_SIZE	400

U0 LowPassFilterDemo()
{
  F64 y,y1,t,dt;
  CDC *dc=DCAlias;

  WinBorder;
  WinMax;
  DocClear;

  dc->color=BLUE;
  GrLine(dc,0,0,0,STEP_SIZE);
  GrLine(dc,0,STEP_SIZE,GR_WIDTH/2,STEP_SIZE);
  GrLine(dc,GR_WIDTH/2,STEP_SIZE,GR_WIDTH/2,0);
  GrLine(dc,GR_WIDTH/2,0,GR_WIDTH,0);
  PressAKey;

  dc->color=CYAN;
  t=0;
  y=0.0;
  while (t<GR_WIDTH) {
    dt=1.0;
    if (t<GR_WIDTH/2)
      y1=STEP_SIZE-STEP_SIZE*Exp(-0.01*t);
    else
      y1=STEP_SIZE*Exp(-0.01*(t-GR_WIDTH/2));
    GrLine(dc,t,y,t+dt,y1);
    y=y1;
    t+=dt;
  }
  PressAKey;

  dc->color=BLACK;
  t=0;
  y=0.0;
  while (t<GR_WIDTH) {
    dt=1.0;
    if (t<GR_WIDTH/2)
      y1=0.99*y+.01*STEP_SIZE;
    else
      y1=0.99*y;
    GrLine(dc,t,y,t+dt,y1);
    y=y1;
    t+=dt;
  }
  PressAKey;

  dc->color=RED;
  t=0;
  y=0.0;
  while (t<GR_WIDTH) {
    dt=Rand/10;
    if (t<GR_WIDTH/2)
      y1=LowPass1(.01,y,STEP_SIZE,dt);
    else
      y1=LowPass1(.01,y,0,dt);
    GrLine(dc,t,y,t+dt,y1);
    y=y1;
    t+=dt;
  }
  PressAKey;

  dc->color=GREEN;
  t=0;
  y=0.0;
  while (t<GR_WIDTH) {
    dt=Rand/10;
    if (t<GR_WIDTH/2)
      y1=LowPass1(.1,y,STEP_SIZE,dt);
    else
      y1=LowPass1(.1,y,0,dt);
    GrLine(dc,t,y,t+dt,y1);
    y=y1;
    t+=dt;
  }
  PressAKey;

  DCDel(dc);
  DCFill;
}

LowPassFilterDemo;
