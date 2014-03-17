U0 GridInit()
{
  ip_grid.x=ip_grid.y=ip_grid.z=8;
  ip_grid.x_offset=ip_grid.y_offset=ip_grid.z_offset=0;
  ip_grid.x_speed =ip_grid.y_speed =ip_grid.z_speed =1;
  ip_grid.show=ip_grid.snap=FALSE;
}

U0 IPVarsUpdate(I64 x,I64 y,I64 z,Bool l,Bool c,Bool r)
{
  ipx_presnap=ToI64(ipx_scale*x)+ipx_offset;
  ipy_presnap=ToI64(ipy_scale*y)+ipy_offset;
  ipz_presnap=ToI64(ipz_scale*z)+ipz_offset;
  if (ip_grid.snap) {
    ipx=Trunc(ipx_presnap/ip_grid.x)*ip_grid.x+ip_grid.x_offset;
    ipy=Trunc(ipy_presnap/ip_grid.y)*ip_grid.y+ip_grid.y_offset;
    ipz=Trunc(ipz_presnap/ip_grid.z)*ip_grid.z+ip_grid.z_offset;
  } else {
    ipx=ipx_presnap;
    ipy=ipy_presnap;
    ipz=ipz_presnap;
  }

  ipx=ClampI64(ipx,0,gr_width-1);
  ipy=ClampI64(ipy,0,gr_height-1);
  ip_throttle=ClampI64(ip_throttle,MIN_I32,MAX_I32);
  iptx=ipx/FONT_WIDTH;
  if (iptx>=text_cols) {
    iptx=text_cols-1;
    ipx=text_cols*FONT_WIDTH-1;
  }
  ipty=ipy/FONT_HEIGHT;
  if (ipty>=text_rows) {
    ipty=text_rows-1;
    ipy=text_rows*FONT_HEIGHT-1;
  }
  ip_lb=l;
  ip_cb=c;
  ip_rb=r;
  LBEqu(&sys_cur_scan_code,SCf_IP_L_DOWN,ip_lb);
  LBEqu(&sys_cur_scan_code,SCf_IP_R_DOWN,ip_rb);
}

U0 SetIP(I64 x=MAX_I64,I64 y=MAX_I64,I64 z=MAX_I64,I64 l=MAX_I64,I64 r=MAX_I64,I64 c=MAX_I64)
{ //Note: Generates a message.
  Bool old_preempt=Preempt(OFF);
  if (!(0<=x<gr_width))
    x=ipx;
  if (!(0<=y<gr_height))
    y=ipy;
  if (z==MAX_I64)
    z=ipz;

  if (!(FALSE<=l<=TRUE))
    l=ip_lb;
  if (!(FALSE<=c<=TRUE))
    c=ip_cb;
  if (!(FALSE<=r<=TRUE))
    r=ip_rb;

  x=(x-ipx_offset)/ipx_scale;
  y=(y-ipy_offset)/ipy_scale;
  z=(z-ipz_offset)/ipz_scale;
  IPVarsUpdate(x,y,z,l,c,r);
  switch (ip_dev) {
    case IP_MOUSE:
      SetMouseIP(x,y,z,l,c,r);
      break;
  }
  Preempt(old_preempt);
}

U0 IPVarsInit()
{
  ipx=0;
  ipy=0;
  ipz=0;
  ip_throttle=0;
  ip_throttle_step=(MAX_I32+1)>>3;
  ipx_offset=0;
  ipy_offset=0;
  ipz_offset=0;
  ipx_scale=1.0;
  ipy_scale=1.0;
  ipz_scale=1.0;
  ip_speed=0;
  ip_dev=IP_NULL;
  iptx=ipx/FONT_WIDTH;
  ipty=ipy/FONT_HEIGHT;
  ip_on=TRUE;
  ip_lb_last=FALSE;
  ip_rb_last=FALSE;
  ip_lb=FALSE;
  ip_cb=FALSE;
  ip_rb=FALSE;
  ip_has_wheel=FALSE;
  ip_last_move_time=tS;
  GridInit;
}
