U0 SetSnap(CSnap *s,I64 i=0,F64 d=0)
{
  if (i) {
    s->x=i;
    s->y=i;
    s->z=i;
  }
  if (d) {
    s->dx=d;
    s->dy=d;
    s->dz=d;
  }
}

CSnap *SnapCur()
{
  static CSnap s;
  Bool old_preempt=Preempt(OFF);
  if (sys_focus_task && TaskValidate(sys_focus_task) &&
      sys_focus_task->win_pixel_left<=ipx_presnap<=sys_focus_task->win_pixel_right &&
      sys_focus_task->win_pixel_top <=ipy_presnap<=sys_focus_task->win_pixel_bottom)
    MemCpy(&s,&sys_focus_task->snap,sizeof(CSnap));
  else
    SetSnap(&s,1,1.0);
  Preempt(old_preempt);
  return &s;
}

U0 GetSnap(CSnap *_s)
{
  CSnap *s=SnapCur;
  MemCpy(_s,s,sizeof(CSnap));
}

U0 IPVarsUpdate(I64 x,I64 y,I64 z,Bool l,Bool c,Bool r)
{
  CSnap s;
  GetSnap(&s);
  ipx_presnap=ToI64(ipx_scale*x)+ipx_offset;
  ipx=RoundI64(ipx_presnap,s.x);
  if (sys_focus_task)
    ipx+=(sys_focus_task->win_pixel_left+sys_focus_task->win_scroll_x)%s.x;

  ipy_presnap=ToI64(ipy_scale*y)+ipy_offset;
  ipy=RoundI64(ipy_presnap,s.y);
  if (sys_focus_task)
    ipy+=(sys_focus_task->win_pixel_top+sys_focus_task->win_scroll_y)%s.y;

  ipz_presnap=ToI64(ipz_scale*z)+ipz_offset;
  ipz=RoundI64(ipz_presnap,s.z);
  if (sys_focus_task)
    ipz+=sys_focus_task->win_scroll_y%s.z;

  ipx=LimitI64(ipx,0,gr_width-1);
  ipy=LimitI64(ipy,0,gr_height-1);
  ip_throttle=LimitI64(ip_throttle,MIN_I32,MAX_I32);
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
  SetSnap(&ip_snap_last,1,1.0);
}
