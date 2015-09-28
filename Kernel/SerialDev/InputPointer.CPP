U0 GridInit()
{//Init input pointer grid struct. See $LK,"::/Demo/Graphics/Grid.CPP"$.
  ip_grid.x=ip_grid.y=ip_grid.z=8;
  ip_grid.x_offset=ip_grid.y_offset=ip_grid.z_offset=0;
  ip_grid.x_speed =ip_grid.y_speed =ip_grid.z_speed =1;
  ip_grid.show=ip_grid.snap=FALSE;
}

U0 IPVarsUpdate(I64 x,I64 y,I64 z,Bool l,Bool r)
{
  ip.presnap.x=ToI64(ip.scale.x*x)+ip.offset.x;
  ip.presnap.y=ToI64(ip.scale.y*y)+ip.offset.y;
  ip.presnap.z=ToI64(ip.scale.z*z)+ip.offset.z;
  if (ip_grid.snap) {
    ip.pos.x=Trunc(ip.presnap.x/ip_grid.x)*ip_grid.x+ip_grid.x_offset;
    ip.pos.y=Trunc(ip.presnap.y/ip_grid.y)*ip_grid.y+ip_grid.y_offset;
    ip.pos.z=Trunc(ip.presnap.z/ip_grid.z)*ip_grid.z+ip_grid.z_offset;
  } else {
    ip.pos.x=ip.presnap.x;
    ip.pos.y=ip.presnap.y;
    ip.pos.z=ip.presnap.z;
  }

  ip.pos.x=ClampI64(ip.pos.x,0,GR_WIDTH-1);
  ip.pos.y=ClampI64(ip.pos.y,0,GR_HEIGHT-1);
  ip.throttle=ClampI64(ip.throttle,MIN_I32,MAX_I32);
  ip.pos_text.x=ip.pos.x/FONT_WIDTH;
  if (ip.pos_text.x>=text.cols) {
    ip.pos_text.x=text.cols-1;
    ip.pos.x=text.cols*FONT_WIDTH-1;
  }
  ip.pos_text.y=ip.pos.y/FONT_HEIGHT;
  if (ip.pos_text.y>=text.rows) {
    ip.pos_text.y=text.rows-1;
    ip.pos.y=text.rows*FONT_HEIGHT-1;
  }
  ip.lb=l;
  ip.rb=r;
  LBEqu(&kbd.scan_code,SCf_IP_L_DOWN,ip.lb);
  LBEqu(&kbd.scan_code,SCf_IP_R_DOWN,ip.rb);
}

U0 IPSet(I64 x=MAX_I64,I64 y=MAX_I64,I64 z=MAX_I64,I64 l=MAX_I64,I64 r=MAX_I64)
{//Note: Generates a message. See $LK,"IPSet",A="FF:::/Demo/Games/Zing.CPP,IPSet"$().
  if (!(0<=x<GR_WIDTH))
    x=ip.pos.x;
  if (!(0<=y<GR_HEIGHT))
    y=ip.pos.y;
  if (z==MAX_I64)
    z=ip.pos.z;

  if (!(FALSE<=l<=TRUE))
    l=ip.lb;
  if (!(FALSE<=r<=TRUE))
    r=ip.rb;

  x=(x-ip.offset.x)/ip.scale.x;
  y=(y-ip.offset.y)/ip.scale.y;
  z=(z-ip.offset.z)/ip.scale.z;
  IPVarsUpdate(x,y,z,l,r);
  switch (ip.dev) {
    case IP_MOUSE:
      MouseIPSet(x,y,z,l,r);
      break;
  }
}

U0 IPVarsInit()
{
  MemSet(&ip,0,sizeof(CIPStateGlbls));
  MemSet(&ip_last,0,sizeof(CIPStateGlbls));
  ip.throttle=0;
  ip.throttle_step=(MAX_I32+1)>>3;
  ip.offset.x=ip.offset.y=ip.offset.z=0;
  ip.scale.x=ip.scale.y=ip.scale.z=1.0;
  ip.dev=IP_NULL;
  ip.pos_text.x=ip.pos_text.y=ip.pos_text.z=0;
  ip.has_wheel=FALSE;
  ip.show=TRUE;
  ip.speed=0;
  ip.timestamp=GetTSC;
  ip.dbl_time=0.350;
  GridInit;
}
