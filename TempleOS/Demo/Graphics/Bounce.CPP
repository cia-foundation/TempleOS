//This uses $LK,"fixed-point-arithmetic","FI:::/TempleOS/Demo/Lectures/FixedPoint.CPP"$.

I64 x[16],y[16],dx[16],dy[16];

U0 Init()
{
  I64 i;
  F64 é;
  MemSet(x,0,sizeof(x));
  MemSet(y,0,sizeof(y));
  for (i=0;i<16;i++) {
    é=Rand*2*ã;
    dx[i]=MAX_I32*Cos(é);
    dy[i]=MAX_I32*Sin(é);
  }
}
 
U0 Bounce()
{
  CDC *dc=DCAlias;
  I64 i,msg_code,p1,update;
  Init;
  try { //Catch <CTRL-ALT-C>
    while (TRUE) {
      update=win_updates;
      do for (i=0;i<16;i++) {
	  dc->color=i;
	  GrPlot(dc,x[i].i32[1],y[i].i32[1]);
	  x[i]+=dx[i];
	  y[i]+=dy[i];
	  if (!(0<=x[i]<Fs->win_pixel_width<<32)) {
	    x[i]-=dx[i];
	    dx[i]=-dx[i];
	  }
	  if (!(0<=y[i]<Fs->win_pixel_height<<32)) {
	    y[i]-=dy[i];
	    dy[i]=-dy[i];
	  }
	}
      while (update==win_updates);
      while (msg_code=ScanMsg(&p1,NULL,1<<MSG_KEY_DOWN|1<<MSG_RESIZE|1<<MSG_MOVE))
	if (msg_code==MSG_KEY_DOWN) {
	  if (p1==CH_SHIFT_ESC || p1==CH_ESC)
	    goto bc_done;
	  else
	    Init;
	} else
	  DCFill(dc);
    }
bc_done:
  } catch
    CatchAll;
  DCFill(dc);
  DCDel(dc);
}
 
Bounce;
